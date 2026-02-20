"""
PlatformIO custom upload script that uses esptool's fast reflashing (--diff-with).

Requires esptool >= 5.2. Falls back to standard full flash if esptool < 5.2.

Usage: Add to [base] in platformio.ini:
    extra_scripts = ... post:fast_reflash_upload.py

Then upload normally:
    pio run -e <env> -t upload

The script reads flash images and offsets dynamically from PlatformIO's
FLASH_EXTRA_IMAGES and ESP32_APP_OFFSET environment variables.

The script keeps a copy of the last-flashed binaries in
.pio/build/<target>/last_flashed/ and uses them as --diff-with references
on subsequent uploads.

Note on otadata (boot_app0.bin): This region is modified by the ESP32 at runtime
(OTA state tracking), so the on-device content will never match the original
boot_app0.bin. We always skip diffing for this region.
"""

Import("env")

import os
import sys
import shutil
import subprocess
import time
import re


# Minimum esptool version that supports --diff-with
MIN_ESPTOOL_VERSION = (5, 2, 0)

# Prefix for all print output to distinguish from esptool's native output
P = ">>>"


def parse_version(version_str):
    m = re.match(r'(\d+)\.(\d+)\.?(\d*)', version_str.strip())
    if m:
        return (int(m.group(1)), int(m.group(2)), int(m.group(3)) if m.group(3) else 0)
    return (0, 0, 0)


def get_esptool_version(python_exe, esptool_path):
    try:
        result = subprocess.run(
            [python_exe, esptool_path, "version"],
            capture_output=True, text=True, timeout=10
        )
        for line in result.stdout.strip().splitlines():
            line = line.strip()
            if line and line[0].isdigit():
                return parse_version(line)
    except Exception:
        pass
    return (0, 0, 0)


def get_esptool_version_module(python_exe):
    try:
        result = subprocess.run(
            [python_exe, "-c", "import esptool; print(esptool.__version__)"],
            capture_output=True, text=True, timeout=10
        )
        if result.returncode == 0:
            return parse_version(result.stdout.strip())
    except Exception:
        pass
    return (0, 0, 0)


def find_esptool(env):
    python_exe = env.subst("$PYTHONEXE")
    project_dir = env.subst("$PROJECT_DIR")

    # 1. PlatformIO-bundled esptool.py
    bundled = env.subst(f"$PROJECT_PACKAGES_DIR{os.sep}tool-esptoolpy{os.sep}esptool.py")
    if os.path.exists(bundled):
        ver = get_esptool_version(python_exe, bundled)
        if ver >= MIN_ESPTOOL_VERSION:
            return [python_exe, bundled], ver, True
        # Remember it for fallback
        bundled_cmd = [python_exe, bundled]
        bundled_ver = ver
    else:
        bundled_cmd = None
        bundled_ver = (0, 0, 0)

    # 2. esptool module in PlatformIO's Python
    ver = get_esptool_version_module(python_exe)
    if ver >= MIN_ESPTOOL_VERSION:
        return [python_exe, "-m", "esptool"], ver, True

    # 3. Project-local .venv or venv
    for venv_name in [".venv", "venv"]:
        venv_python = os.path.join(project_dir, venv_name, "bin", "python")
        if not os.path.exists(venv_python):
            venv_python = os.path.join(project_dir, venv_name, "Scripts", "python.exe")  # Windows
        if os.path.exists(venv_python):
            ver = get_esptool_version_module(venv_python)
            if ver >= MIN_ESPTOOL_VERSION:
                return [venv_python, "-m", "esptool"], ver, True

    # 4. esptool module in system Python
    for py_name in ["python3", "python"]:
        py_path = shutil.which(py_name)
        if not py_path or os.path.realpath(py_path) == os.path.realpath(python_exe):
            continue
        ver = get_esptool_version_module(py_path)
        if ver >= MIN_ESPTOOL_VERSION:
            return [py_path, "-m", "esptool"], ver, True

    # 5. esptool standalone binary on PATH
    for name in ["esptool", "esptool.py"]:
        path = shutil.which(name)
        if not path:
            continue
        try:
            result = subprocess.run(
                [path, "version"],
                capture_output=True, text=True, timeout=10
            )
            for line in result.stdout.strip().splitlines():
                line = line.strip()
                if line and line[0].isdigit():
                    ver = parse_version(line)
                    if ver >= MIN_ESPTOOL_VERSION:
                        return [path], ver, True
        except Exception:
            continue

    # No esptool >= 5.2 found. Fall back to bundled one.
    if bundled_cmd:
        return bundled_cmd, bundled_ver, False

    return None, (0, 0, 0), False


def format_version(ver):
    return f"{ver[0]}.{ver[1]}.{ver[2]}"


def get_binary_list(env):
    # Determines list of (offset, path, name, is_otadata) for all binaries to flash.
    # Reads dynamically from PlatformIO's FLASH_EXTRA_IMAGES (bootloader,
    # partitions, OTA data, etc.) and ESP32_APP_OFFSET (application firmware).
    images = []

    # Extra images populated by the framework builder (bootloader, partitions, OTA data)
    for offset, path in env.get("FLASH_EXTRA_IMAGES", []):
        resolved = env.subst(path)
        name = os.path.basename(resolved)
        # OTA data partition (boot_app0.bin) is modified at runtime by the ESP32
        is_otadata = "boot_app0" in name
        images.append((offset, resolved, name, is_otadata))

    # Application firmware
    app_offset = env.subst("$ESP32_APP_OFFSET")
    firmware_path = env.subst(os.path.join("$BUILD_DIR", "${PROGNAME}.bin"))
    images.append((app_offset, firmware_path, os.path.basename(firmware_path), False))

    return images


def get_upload_flags(env, port):
    # Extract esptool flags from PlatformIO's UPLOADERFLAGS.

    # Returns a list of flags up to (but not including) the first offset/path pair,
    # i.e. the global options and write-flash subcommand with its options.
    # The --port value is replaced with the resolved port.

    # UPLOADERFLAGS layout:
    #  [--chip, esp32, --port, "$UPLOAD_PORT", --baud, $UPLOAD_SPEED,
    #   --before, default-reset, --after, hard-reset,
    #   write-flash, -z, --flash-mode, dio, --flash-freq, 80m, --flash-size, detect,
    #   0x1000, bootloader.bin, 0x8000, partitions.bin, ...]

    # We stop before the first item that looks like a flash offset (hex number
    # not preceded by a flag).
    raw_flags = [env.subst(f).strip('"') for f in env.get("UPLOADERFLAGS", [])]

    # Walk through the flags, consuming flag-value pairs.
    # Stop when we hit a bare value (not preceded by a --flag) after write-flash options.
    flags = []
    found_write_flash = False
    i = 0
    while i < len(raw_flags):
        f = raw_flags[i]
        if not found_write_flash:
            flags.append(f)
            if f in ("write-flash", "write_flash"):
                found_write_flash = True
            i += 1
            continue
        # After write-flash: flags start with -, bare values are offset/path pairs
        if f.startswith("-"):
            flags.append(f)
            # Consume the flag's value if present
            if i + 1 < len(raw_flags) and not raw_flags[i + 1].startswith("-"):
                flags.append(raw_flags[i + 1])
                i += 2
            else:
                i += 1
        else:
            break  # first offset reached

    # Replace port with the resolved one (auto-detected or user-specified)
    for i, f in enumerate(flags):
        if f == "--port" and i + 1 < len(flags):
            flags[i + 1] = port
            break

    return flags


def do_fast_reflash(env, esptool_cmd, port, binaries, last_flashed_dir):
    # For OTA data (boot_app0.bin), we always use 'skip' because the
    # on-device content is modified by the ESP32 at runtime (OTA state tracking).
    upload_flags = get_upload_flags(env, port)

    flash_args = []
    diff_args = []
    has_any_diff = False

    for offset, path, name, is_otadata in binaries:
        if not os.path.exists(path):
            print(f"{P} WARNING: {path} not found, skipping")
            continue

        flash_args.extend([offset, path])

        last_path = os.path.join(last_flashed_dir, name)
        if is_otadata:
            # otadata is modified at runtime by ESP32, always skip diff
            diff_args.append("skip")
        elif os.path.exists(last_path):
            diff_args.append(last_path)
            has_any_diff = True
        else:
            diff_args.append("skip")

    cmd = list(esptool_cmd) + upload_flags + flash_args

    if has_any_diff:
        cmd += ["--diff-with"] + diff_args
    else:
        print(f"{P} No previous binaries for comparison -- full flash this time.")

    print(f"{P} Running: {' '.join(cmd)}")
    return subprocess.call(cmd)


def do_normal_flash(env, esptool_cmd, port, binaries):
    upload_flags = get_upload_flags(env, port)

    flash_args = []
    for offset, path, name, is_otadata in binaries:
        if os.path.exists(path):
            flash_args.extend([offset, path])
        else:
            print(f"{P} WARNING: {path} not found, skipping")

    cmd = list(esptool_cmd) + upload_flags + flash_args
    print(f"{P} Running: {' '.join(cmd)}")
    return subprocess.call(cmd)


def save_flashed_binaries(binaries, last_flashed_dir):
    os.makedirs(last_flashed_dir, exist_ok=True)
    for _offset, path, name, _is_otadata in binaries:
        if os.path.exists(path):
            shutil.copy2(path, os.path.join(last_flashed_dir, name))


def upload_callback(source, target, env):
    # Upload callback invoked by PlatformIO
    build_dir = env.subst("$BUILD_DIR")
    last_flashed_dir = os.path.join(build_dir, "last_flashed")

    print(f"{P} Fast Reflash Upload")

    # Resolve upload port
    port = env.subst("$UPLOAD_PORT")
    if not port or port.startswith("$"):
        # Auto-detect
        try:
            from platformio.device.finder import SerialPortFinder
            port = SerialPortFinder(
                board_config=env.BoardConfig(),
                upload_protocol="esptool",
            ).find()
        except Exception:
            print(f"{P} ERROR: No upload port specified and auto-detection failed.")
            print(f"{P} Set upload_port in platformio.ini or pass --upload-port.")
            return 1
    print(f"{P} Upload port: {port}")

    # Collect binaries
    binaries = get_binary_list(env)
    for offset, path, name, is_otadata in binaries:
        exists = os.path.exists(path)
        size_str = ""
        if exists:
            size_kb = os.path.getsize(path) / 1024
            size_str = f" ({size_kb:.0f} KB)"
        ota_tag = " [otadata, skip diff]" if is_otadata else ""
        print(f"{P} {offset}: {name}{size_str}{ota_tag}{'' if exists else ' (MISSING)'}")

    # Find best available esptool
    esptool_cmd, esptool_ver, supports_diff = find_esptool(env)
    if not esptool_cmd:
        print(f"{P} ERROR: No esptool found!")
        return 1

    ver_str = format_version(esptool_ver)
    min_ver_str = format_version(MIN_ESPTOOL_VERSION)

    if supports_diff:
        # Check if we actually have previous binaries
        has_prev = any(
            os.path.exists(os.path.join(last_flashed_dir, name))
            for _offset, _path, name, is_otadata in binaries
            if not is_otadata
        )
        if has_prev:
            print(f"{P} Using esptool {ver_str} with fast reflash (--diff-with)")
        else:
            print(f"{P} Using esptool {ver_str} -- first flash, no diff references yet")
    else:
        print(f"{P} esptool {ver_str} < {min_ver_str}, fast reflash not available")
        print(f"{P} Install esptool >= {min_ver_str} for fast reflash:")
        print(f"{P}   pip install 'esptool>={min_ver_str}'")
        print(f"{P} Falling back to standard full flash.")

    print()
    start_time = time.time()

    if supports_diff:
        ret = do_fast_reflash(env, esptool_cmd, port, binaries, last_flashed_dir)
    else:
        ret = do_normal_flash(env, esptool_cmd, port, binaries)

    elapsed = time.time() - start_time

    print()
    if ret == 0:
        print(f"{P} Upload completed in {elapsed:.1f}s")
        save_flashed_binaries(binaries, last_flashed_dir)
        print(f"{P} Saved binaries to {last_flashed_dir}")
    else:
        print(f"{P} Upload FAILED (exit code {ret}) after {elapsed:.1f}s")

    return ret


# Register the custom upload command with PlatformIO
env.Replace(UPLOADCMD=upload_callback)
