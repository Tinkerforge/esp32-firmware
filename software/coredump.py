#!/usr/bin/env python3

import base64
import string
import json
import argparse
import os, sys
import tempfile

has_esp_coredump = False
has_xtensa_gdb = False

try:
    from esp_coredump import CoreDump
    has_esp_coredump = True
except:
    from shutil import which
    has_xtensa_gdb = which("xtensa-esp32-elf-gdb") is not None

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

if __name__ == '__main__':
    if not (has_esp_coredump or has_xtensa_gdb):
        print("Failed to import esp_coredump and can't find xtensa-esp32-elf-gdb.")
        print("Install ESP-IDF and (on POSIX) run source /path/to/esp-idf/export.sh")
        print("or put 'xtensa-esp32-elf-gdb' in your PATH.")
        sys.exit(-1)

    parser = argparse.ArgumentParser()
    parser.add_argument("path")
    args = parser.parse_args(sys.argv[1:])
    path = args.path

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

        try:
            tf_coredump_json = get_tf_coredump_data(core_dump_path)
            tf_coredump_data = json.loads(tf_coredump_json)

            elf_name = tf_coredump_data['firmware_file_name'] + ".elf"
            script_path = os.path.dirname(os.path.realpath(__file__))

            firmware_path = os.path.join(script_path, "build", elf_name)
            if not os.path.exists(firmware_path):
                firmware_path = os.path.join(script_path, "..", "..", "warp-charger", "firmwares", elf_name)

            if os.path.exists(firmware_path):
                if has_esp_coredump:
                    coredump = CoreDump(chip='esp32', core=core_dump_path, core_format='elf', prog=firmware_path)
                    coredump.dbg_corefile()
                if has_xtensa_gdb:
                    os.system("xtensa-esp32-elf-gdb -q --batch -ex 'set style enabled on' -ex 'bt full' {} {}".format(firmware_path, core_dump_path))
            else:
                print("Firmware {} not found".format(elf_name))
        finally:
            if os.path.exists(core_dump_path):
                os.remove(core_dump_path)
