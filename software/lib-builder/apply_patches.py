import os
import sys
import subprocess

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
    print("Usage: {} path/to/esp32-arduino-lib-builder path/to/patches".format(sys.argv[0]))
    sys.exit(1)

root_dir = sys.argv[1]
patch_dir = sys.argv[2]

for root, dirs, files in os.walk(patch_dir):
    if len(files) == 0:
        continue
    dir_to_apply_in = os.path.relpath(root, start=patch_dir)
    files = [os.path.abspath(os.path.join(root, x)) for x in files]
    with ChangedDirectory(os.path.join(root_dir, dir_to_apply_in)):
        for patch in sorted(files):
            print("Calling", *["git", "am", patch], "in", os.path.join(root_dir, dir_to_apply_in))
            subprocess.call(["git", "am", patch], stdin=sys.stdin, stdout=sys.stdout, stderr=sys.stderr)
    #print(os.path.relpath(root, start=patch_dir), dirs, files)

