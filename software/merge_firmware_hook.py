Import("env")

import sys

if sys.hexversion < 0x3060000:
    print('Python >= 3.6 required')
    sys.exit(1)

import glob
import os
import re
import shutil
import subprocess

with open(os.path.join(env.subst('$BUILD_DIR'), 'firmware_basename'), 'r', encoding='utf-8') as f:
    firmware_basename = f.read().strip()

partitions = env.GetProjectOption('board_build.partitions')
m = re.match('^default_([1-9][0-9]*MB)_coredump.csv$', partitions)
flash_size = m.group(1)

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(lambda **kwargs: os.makedirs("build", exist_ok=True), "Ensuring build dir exists")
)

if env.GetProjectOption("custom_autoclean_build_dir", default="false") == "true": # Option is a string, not a Python boolean.
    firmware_name = env.GetProjectOption("custom_name") + "_firmware"

    def delete_old(fwname):
        fileList = glob.glob('build/{}*'.format(fwname))
        for filePath in fileList:
            try:
                os.remove(filePath)
            except:
                pass
        return None

    env.AddPostAction(
        "$BUILD_DIR/${PROGNAME}.elf",
        env.VerboseAction(lambda **kwargs: delete_old(firmware_name),
                          "Cleaning old {}_*".format(firmware_name))
    )

def copy2(src, dst): # hide shutil.copy2 return value
    shutil.copy2(src, dst)
    return None

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(lambda env, **kwargs: copy2(env.subst("$BUILD_DIR/${PROGNAME}.elf"),
                                                  f"build/{firmware_basename}.elf"),
                      f"Copying $BUILD_DIR{os.sep}${{PROGNAME}}.elf -> build{os.sep}{firmware_basename}.elf")
)

if sys.platform == 'win32':
    symlink = lambda src, dst: copy2(os.path.join('build', src), dst)
else:
    symlink = os.symlink

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(lambda env, **kwargs: symlink(f"{firmware_basename}.elf",
                                                    f"build/{env.GetProjectOption('custom_name')}_firmware_latest.elf"),
                    f"Symlinking build/{firmware_basename}.elf -> build{os.sep}{env.GetProjectOption('custom_name')}_firmware_latest.elf")
)

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(lambda env, **kwargs: symlink(f"{firmware_basename}.elf",
                                                    f"build/firmware_latest.elf"),
                    f"Symlinking build/{firmware_basename}.elf -> build{os.sep}firmware_latest.elf")
)

def check_call(*args): # hide subprocess.check_call return value
    subprocess.check_call(args)
    return None

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.bin",
    env.VerboseAction(lambda env, **kwargs: check_call(
        env.subst('$PYTHONEXE'),
        "-u",
        env.subst("$PROJECT_PACKAGES_DIR/tool-esptoolpy/esptool.py"),
        "--chip", "esp32",
        "merge_bin",
        "-o", env.subst("$BUILD_DIR/${PROGNAME}_merged.bin"),
        "--target-offset", "0x1000",
        "0x1000", env.subst("$BUILD_DIR/bootloader.bin"),
        "0x8000", env.subst("$BUILD_DIR/partitions.bin"),
        "0xd000", env.subst("$BUILD_DIR/firmware_info.bin"),
        "0xe000", "boot_app0.bin",
        "0x10000", env.subst("$BUILD_DIR/${PROGNAME}.bin")
    ), "Merging {0}.bin -> {0}_merged.bin".format(env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")))
)

if env.GetProjectOption('custom_signed') == 'true':
    env.AddPostAction(
        "$BUILD_DIR/${PROGNAME}.bin",
        env.VerboseAction(lambda env, **kwargs: check_call(
            env.subst('$PYTHONEXE'),
            "-u",
            env.subst("$PROJECT_DIR/signature/sign.py"),
            env.GetProjectOption("custom_name"),
            env.subst("$BUILD_DIR/${PROGNAME}_merged.bin"),
            "build/{}_merged.bin".format(firmware_basename)
        ), f"Signing $BUILD_DIR{os.sep}${{PROGNAME}}_merged.bin -> build{os.sep}{{os.sep}}_merged.bin")
    )
else:
    env.AddPostAction(
        "$BUILD_DIR/${PROGNAME}.bin",
        env.VerboseAction(lambda env, **kwargs: copy2(env.subst("$BUILD_DIR/${PROGNAME}_merged.bin"),
                                                      f"build/{firmware_basename}_merged.bin"),
                          f"Copying $BUILD_DIR{os.sep}${{PROGNAME}}_merged.bin -> build{os.sep}{firmware_basename}_merged.bin")
    )

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.bin",
    env.VerboseAction(lambda env, **kwargs: symlink(f"{firmware_basename}_merged.bin",
                                                    f"build/{env.GetProjectOption('custom_name')}_firmware_latest_merged.bin"),
                        f"Symlinking build{os.sep}{firmware_basename}_merged.bin -> build{os.sep}{env.GetProjectOption('custom_name')}_firmware_latest_merged.bin")
)

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.bin",
    env.VerboseAction(lambda env, **kwargs: symlink(f"{firmware_basename}_merged.bin",
                                                    f"build/firmware_latest_merged.bin"),
                        f"Symlinking build{os.sep}{firmware_basename}_merged.bin -> build{os.sep}firmware_latest_merged.bin")
)
