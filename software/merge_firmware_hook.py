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

with open(env.subst(f'$BUILD_DIR{os.sep}firmware_basename'), 'r', encoding='utf-8') as f:
    firmware_basename = f.read().strip()

product_id = env.GetProjectOption('custom_product_id')

env.AddPostAction(
    f"$BUILD_DIR{os.sep}${{PROGNAME}}.elf",
    env.VerboseAction(lambda **kwargs: os.makedirs("build", exist_ok=True), "Ensuring build directory exists")
)

env.AddPostAction(
    f"$BUILD_DIR{os.sep}${{PROGNAME}}.elf",
    env.VerboseAction(lambda **kwargs: os.makedirs("build_latest", exist_ok=True), "Ensuring build_latest directory exists")
)

if env.GetProjectOption("custom_autoclean_build_dir", default="false") == "true": # Option is a string, not a Python boolean.
    firmware_name = product_id + "_firmware"

    def delete_old(fwname):
        fileList = glob.glob(f'build{os.sep}{fwname}*')
        for filePath in fileList:
            try:
                os.remove(filePath)
            except:
                pass
        return None

    env.AddPostAction(
        f"$BUILD_DIR{os.sep}${{PROGNAME}}.elf",
        env.VerboseAction(lambda **kwargs: delete_old(firmware_name),
                          "Cleaning old {}*".format(firmware_name))
    )

def copy2(src, dst): # hide shutil.copy2 return value
    shutil.copy2(src, dst)
    return None

env.AddPostAction(
    f"$BUILD_DIR{os.sep}${{PROGNAME}}.elf",
    env.VerboseAction(lambda env, **kwargs: copy2(env.subst(f"$BUILD_DIR{os.sep}${{PROGNAME}}.elf"),
                                                  f"build{os.sep}{firmware_basename}.elf"),
                      f"Copying $BUILD_DIR{os.sep}${{PROGNAME}}.elf -> build{os.sep}{firmware_basename}.elf")
)

if sys.platform == 'win32':
    symlink = lambda src, dst: copy2(os.path.join('build', src), dst)
else:
    symlink = os.symlink

def remove_and_symlink(remove_pattern, src_path, dst_path):
    for path in glob.glob(remove_pattern):
        try:
            os.remove(path)
        except:
            pass

    return symlink(src_path, dst_path)

env.AddPostAction(
    f"$BUILD_DIR{os.sep}${{PROGNAME}}.elf",
    env.VerboseAction(lambda env, **kwargs: remove_and_symlink(f"build_latest{os.sep}{product_id}_firmware*.elf",
                                                               f"..{os.sep}build{os.sep}{firmware_basename}.elf",
                                                               f"build_latest{os.sep}{firmware_basename}.elf"),
                      f"Symlinking build{os.sep}{firmware_basename}.elf -> build_latest{os.sep}{firmware_basename}.elf")
)

env.AddPostAction(
    f"$BUILD_DIR{os.sep}${{PROGNAME}}.elf",
    env.VerboseAction(lambda env, **kwargs: symlink(f"{firmware_basename}.elf",
                                                    f"build{os.sep}{product_id}_firmware_latest.elf"),
                      f"Symlinking build{os.sep}{firmware_basename}.elf -> build{os.sep}{product_id}_firmware_latest.elf")
)

env.AddPostAction(
    f"$BUILD_DIR{os.sep}${{PROGNAME}}.elf",
    env.VerboseAction(lambda env, **kwargs: symlink(f"{firmware_basename}.elf",
                                                    f"build{os.sep}firmware_latest.elf"),
                      f"Symlinking build{os.sep}{firmware_basename}.elf -> build{os.sep}firmware_latest.elf")
)

def check_call(*args): # hide subprocess.check_call return value
    subprocess.check_call(args)
    return None

env.AddPostAction(
    f"$BUILD_DIR{os.sep}${{PROGNAME}}.bin",
    env.VerboseAction(lambda env, **kwargs: check_call(
        env.subst('$PYTHONEXE'),
        "-u",
        env.subst(f"$PROJECT_PACKAGES_DIR{os.sep}tool-esptoolpy{os.sep}esptool.py"),
        "--chip", "esp32",
        "merge-bin",
        "-o", env.subst(f"$BUILD_DIR{os.sep}${{PROGNAME}}_merged.bin"),
        "--target-offset", "0x1000",
        "0x1000", env.subst(f"$BUILD_DIR{os.sep}bootloader.bin"),
        "0x8000", env.subst(f"$BUILD_DIR{os.sep}partitions.bin"),
        "0xd000", env.subst(f"$BUILD_DIR{os.sep}firmware_info.bin"),
        "0xe000", "boot_app0.bin",
        "0x10000", env.subst(f"$BUILD_DIR{os.sep}${{PROGNAME}}.bin")
    ), "Merging {0}.bin -> {0}_merged.bin".format(env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")))
)

signature_preset = env.GetProjectOption('custom_signature_preset')

if len(signature_preset) > 0:
    env.AddPostAction(
        f"$BUILD_DIR{os.sep}${{PROGNAME}}.bin",
        env.VerboseAction(lambda env, **kwargs: check_call(
            env.subst('$PYTHONEXE'),
            "-u",
            env.subst(f"$PROJECT_DIR{os.sep}signature{os.sep}sign.py"),
            signature_preset,
            env.subst(f"$BUILD_DIR{os.sep}${{PROGNAME}}_merged.bin"),
            f"build{os.sep}{firmware_basename}_merged.bin"
        ), f"Signing $BUILD_DIR{os.sep}${{PROGNAME}}_merged.bin -> build{os.sep}{firmware_basename}_merged.bin")
    )
else:
    env.AddPostAction(
        f"$BUILD_DIR{os.sep}${{PROGNAME}}.bin",
        env.VerboseAction(lambda env, **kwargs: copy2(env.subst(f"$BUILD_DIR{os.sep}${{PROGNAME}}_merged.bin"),
                                                      f"build{os.sep}{firmware_basename}_merged.bin"),
                          f"Copying $BUILD_DIR{os.sep}${{PROGNAME}}_merged.bin -> build{os.sep}{firmware_basename}_merged.bin")
    )

env.AddPostAction(
    f"$BUILD_DIR{os.sep}${{PROGNAME}}.bin",
    env.VerboseAction(lambda env, **kwargs: remove_and_symlink(f"build_latest{os.sep}{product_id}_firmware*_merged.bin",
                                                               f"..{os.sep}build{os.sep}{firmware_basename}_merged.bin",
                                                               f"build_latest{os.sep}{firmware_basename}_merged.bin"),
                      f"Symlinking build{os.sep}{firmware_basename}_merged.bin -> build_latest{os.sep}{firmware_basename}_merged.bin")
)

env.AddPostAction(
    f"$BUILD_DIR{os.sep}${{PROGNAME}}.bin",
    env.VerboseAction(lambda env, **kwargs: symlink(f"{firmware_basename}_merged.bin",
                                                    f"build{os.sep}{product_id}_firmware_latest_merged.bin"),
                      f"Symlinking build{os.sep}{firmware_basename}_merged.bin -> build{os.sep}{product_id}_firmware_latest_merged.bin")
)

env.AddPostAction(
    f"$BUILD_DIR{os.sep}${{PROGNAME}}.bin",
    env.VerboseAction(lambda env, **kwargs: symlink(f"{firmware_basename}_merged.bin",
                                                    f"build{os.sep}firmware_latest_merged.bin"),
                      f"Symlinking build{os.sep}{firmware_basename}_merged.bin -> build{os.sep}firmware_latest_merged.bin")
)
