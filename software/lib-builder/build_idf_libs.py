#!/usr/bin/python3 -u

import shutil
import subprocess
import sys
import os

import create_defconfig
import merge_defconfig

# use "with ChangedDirectory('/path/to/abc')" instead of "os.chdir('/path/to/abc')"
class ChangedDirectory(object):
    def __init__(self, path):
        self.path = path
        self.previous_path = None

    def __enter__(self):
        self.previous_path = os.getcwd()
        os.chdir(self.path)

    def __exit__(self, type_, value, traceback):
        os.chdir(self.previous_path)

if len(sys.argv) != 3:
    print("Usage: {} path/to/esp32-arduino-lib-builder path/to/defconfig".format(sys.argv[0]))
    sys.exit(1)

root = sys.argv[1]

shutil.move(os.path.join(root, "configs", "defconfig.esp32"), os.path.join(root, "configs", "defconfig.esp32vanilla"))
try:
    shutil.copy(os.path.join(sys.argv[2]), os.path.join(root, "configs", "defconfig.esp32"))
    with ChangedDirectory(os.path.join(root)):
        subprocess.call("./build.sh -t esp32", shell=True, stdin=sys.stdin, stdout=sys.stdout, stderr=sys.stderr)

finally:
    shutil.move(os.path.join(root, "configs", "defconfig.esp32vanilla"), os.path.join(root, "configs", "defconfig.esp32"))
