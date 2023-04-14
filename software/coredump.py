#!/usr/bin/env python3

import base64
import string
import json
import argparse
import os, sys
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

if __name__ == '__main__':
    gdb = find_gdb()
    if gdb is None:
        print("Can't find xtensa-esp32-elf-gdb or pio.")
        print("Make sure that one of them is available in your $PATH.")
        sys.exit(-1)

    parser = argparse.ArgumentParser()
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
            os.system("{} -q --batch -ex 'set style enabled on' -ex 'bt full' {} {}".format(gdb, firmware_path, core_dump_path))
        else:
            print("Firmware {} not found".format(elf_name))
    finally:
        if os.path.exists(core_dump_path):
            os.remove(core_dump_path)
