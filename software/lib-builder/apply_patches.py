#!/usr/bin/env -S uv run --script

import os
import sys
import subprocess
import tinkerforge_util as tfutil

if len(sys.argv) < 3 or len(sys.argv) > 4:
    print("Usage: {} path/to/esp32-arduino-lib-builder path/to/patches [number of first to apply]".format(sys.argv[0]))
    sys.exit(1)

root_dir = sys.argv[1]
patch_dir = sys.argv[2]
skip = int(sys.argv[3]) if len(sys.argv) > 3 else -1

for root, dirs, files in os.walk(patch_dir):
    if len(files) == 0:
        continue
    dir_to_apply_in = os.path.relpath(root, start=patch_dir)
    files = [os.path.abspath(os.path.join(root, x)) for x in files]
    with tfutil.ChangedDirectory(os.path.join(root_dir, dir_to_apply_in)):
        for patch in sorted(files):
            i = int(os.path.basename(patch).split("-")[0])
            if i < skip:
                print("Skipping", patch)
                continue
            if (os.path.splitext(patch)[1] == ".rawpatch"):
                print(f"Calling patch --no-backup-if-mismatch < {patch} in {os.path.join(root_dir, dir_to_apply_in)}")
                subprocess.check_call(f"patch --no-backup-if-mismatch < {patch}", stdin=sys.stdin, stdout=sys.stdout, stderr=sys.stderr, shell=True)
            else:
                print("Calling", *["git", "am", patch], "in", os.path.join(root_dir, dir_to_apply_in))
                subprocess.check_call(["git", "am", patch], stdin=sys.stdin, stdout=sys.stdout, stderr=sys.stderr)
    #print(os.path.relpath(root, start=patch_dir), dirs, files)
