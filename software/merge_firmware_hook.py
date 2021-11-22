Import("env", "projenv")

import os
import re

with open(os.path.join('src', 'firmware_basename'), 'r') as f:
    firmware_basename = f.read().strip()

partitions = env.GetProjectOption('board_build.partitions')
m = re.match('^default_([1-9][0-9]*MB)_coredump.csv$', partitions)
flash_size = m.group(1)

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction("mkdir -p build", "Ensuring build dir exists")
)

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([
        "cp", "$BUILD_DIR/${PROGNAME}.elf", "build/{}.elf".format(firmware_basename)
    ]), "Copying $BUILD_DIR/${PROGNAME}.elf")
)

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.bin",
    env.VerboseAction(" ".join([
        "cp", "$BUILD_DIR/${PROGNAME}.bin", "build/{}.bin".format(firmware_basename)
    ]), "Copying $BUILD_DIR/${PROGNAME}.bin")
)

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.bin",
    env.VerboseAction(" ".join([
        "python3", "merge_bins.py",
        "0x1000", "bootloader_dio_40m.bin",
        "0x8000", "$BUILD_DIR/partitions.bin",
        "0xd000", "build/fw_info.bin",
        "0xe000", "boot_app0.bin",
        "0x10000", "$BUILD_DIR/${PROGNAME}.bin",
        "build/{}_merged.bin".format(firmware_basename)
    ]), "Merging firmware.bin")
)
