#!/usr/bin/env -S uv run --script

import shutil
import subprocess
import sys
import os
import tinkerforge_util as tfutil

if len(sys.argv) != 3:
    print("Usage: {} path/to/esp32-arduino-lib-builder path/to/defconfig".format(sys.argv[0]))
    sys.exit(1)

root = sys.argv[1]

if not os.path.isfile(os.path.join(root, "configs", "defconfig.esp32")):
    print("defconfig not found. Is the lib-builder root correct?")
    sys.exit(1)

shutil.move(os.path.join(root, "configs", "defconfig.esp32"), os.path.join(root, "configs", "defconfig.esp32vanilla"))
try:
    shutil.copy(os.path.join(sys.argv[2]), os.path.join(root, "configs", "defconfig.esp32"))
    with tfutil.ChangedDirectory(os.path.join(root)):
        subprocess.call(['bash', '-c', '. esp-idf/export.sh && ./build.sh -s -t esp32'], stdin=sys.stdin, stdout=sys.stdout, stderr=sys.stderr)

finally:
    shutil.move(os.path.join(root, "configs", "defconfig.esp32vanilla"), os.path.join(root, "configs", "defconfig.esp32"))
