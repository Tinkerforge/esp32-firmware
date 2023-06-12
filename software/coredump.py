#!/usr/bin/env python3

import base64
import string
import json
import argparse
import os, sys
import subprocess
import tempfile
from shutil import which

def find_gdb():
    path = which("xtensa-esp32-elf-gdb")
    if path is not None:
        return path

    path = which("pio")
    if path is not None:
        return path + " pkg exec xtensa-esp32-elf-gdb --"

    return None

PREFIX = b"___tf_coredump_info_start___"
SUFFIX = b"___tf_coredump_info_end___"

def get_tf_coredump_data(coredump_path: str):
    printable = set(string.printable)
    found_str = ""
    with open(coredump_path, "rb") as file:
        b = file.read()
        start = b.find(PREFIX)

        if start < 0:
            return None

        end = b.find(SUFFIX)

        if end < 0 or end < start:
            return None

        start += len(PREFIX)
        return b[start:end]

core_dump_path = os.path.join(tempfile.gettempdir(), "tf_coredump.elf")

def get_core_dump_from_debug_report(path):
    with open(path) as file:
        file_str = file.read()
        core_dump_start_pos = file_str.rfind("___CORE_DUMP_START___\n\n")
        if core_dump_start_pos < 0:
            print("Debug log doesn't contain a core dump")
            sys.exit(-1)

        core_dump_b64 = file_str[core_dump_start_pos + 60:]
        core_dump = base64.b64decode(core_dump_b64)

        with open(core_dump_path, "wb") as f:
            f.write(core_dump)

def download_core_dump(port):
    os.system("pio pkg exec esptool.py -- --port {} --chip esp32 --baud 921600 read_flash 0xff0014 0xFFEC {}".format(port, core_dump_path))

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

if __name__ == '__main__':
    gdb = find_gdb()
    if gdb is None:
        print("Can't find xtensa-esp32-elf-gdb or pio.")
        print("Make sure that one of them is available in your $PATH.")
        sys.exit(-1)

    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--interactive", action='store_true', help="Don't exit gdb immediately")

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("path", nargs='?', default=None)
    group.add_argument("-p", "--port")
    args = parser.parse_args(sys.argv[1:])

    if args.port:
        if len(args.port) < 6:
            port = "/dev/ttyUSB" + args.port
        else:
            port = args.port

    if args.path:
        get_core_dump_from_debug_report(args.path)
    if args.port:
        download_core_dump(port)


    try:
        tf_coredump_json = get_tf_coredump_data(core_dump_path)
        if not tf_coredump_json:
            print("Core dump in debug log has no TF coredump info.")
            sys.exit (-1)

        tf_coredump_data = json.loads(tf_coredump_json)
        print(tf_coredump_data)
        elf_name = tf_coredump_data['firmware_file_name'] + ".elf"
        script_path = os.path.dirname(os.path.realpath(__file__))

        firmware_path = os.path.join(script_path, "build", elf_name)
        if not os.path.exists(firmware_path):
            firmware_path = os.path.join(script_path, "..", "..", "warp-charger", "firmwares", elf_name)

        if os.path.exists(firmware_path):
            with tempfile.TemporaryDirectory(prefix="coredump-git-") as d:
                os.system(f"git clone --shared --no-checkout {script_path}/.. {d}")
                with ChangedDirectory(d):
                    os.system(f"git checkout --quiet {tf_coredump_data['firmware_commit_id']}")
                    commit_time = int(subprocess.check_output(['git', 'log', '-1', '--pretty=%at', tf_coredump_data['firmware_commit_id']]))
                    for (dirpath, dirnames, filenames) in os.walk('software/src'):
                        for filename in filenames:
                            os.utime(os.sep.join([dirpath, filename]), (commit_time, commit_time))

                os.system(f"{gdb} " +
                           ("-q --batch " if not args.interactive else "") +
                           "-iex 'set pagination off' " +
                          f"-iex 'directory {d}' " +
                          f"-iex 'set substitute-path src/ {d}/software/src' " +
                          f"-iex 'set substitute-path /home/erik/ {os.path.expanduser('~')}' " +
                           "-iex 'set style enabled on' " +
                           "-iex 'set print frame-info source-and-location' " +
                           "-ex 'echo =============================================================\n' " +
                           "-ex 'echo Run \"disassemble /s\" in interactive mode to analyze assembly.\n' " +
                           "-ex 'echo\n' " +
                           "-ex 'echo =============================================================\n' " +
                           "-ex 'echo ==================== Registers at crash =====================\n' " +
                           "-ex 'echo =============================================================\n' " +
                           "-ex 'info registers ps a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 a10 a11 a12 a13 a14 a15' " +
                           "-ex 'echo =============================================================\n' " +
                           "-ex 'echo =================== Backtrace starts here ===================\n' " +
                           "-ex 'echo =============================================================\n' " +
                           "-ex 'bt full' " +
                          f"{firmware_path} {core_dump_path}")
        else:
            print("Firmware {} not found".format(elf_name))
    finally:
        if os.path.exists(core_dump_path):
            os.remove(core_dump_path)
