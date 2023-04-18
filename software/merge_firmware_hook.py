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
                                                  "build/{}.elf".format(firmware_basename)),
                      "Copying $BUILD_DIR/${PROGNAME}.elf")
)

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.bin",
    env.VerboseAction(lambda env, **kwargs: copy2(env.subst("$BUILD_DIR/${PROGNAME}.bin"),
                                                  "build/{}.bin".format(firmware_basename)),
                      "Copying $BUILD_DIR/${PROGNAME}.bin")
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
        "-o", "build/{}_merged.bin".format(firmware_basename),
        "--flash_mode", "dio",
        "--flash_size", flash_size,
        "--flash_freq", "40m",
        "--target-offset", "0x1000",
        "0x1000", env.subst("$BUILD_DIR/bootloader.bin"),
        "0x8000", env.subst("$BUILD_DIR/partitions.bin"),
        "0xd000", env.subst("$BUILD_DIR/firmware_info.bin"),
        "0xe000", "boot_app0.bin",
        "0x10000", env.subst("$BUILD_DIR/${PROGNAME}.bin")
    ), "Merging firmware.bin")
)
