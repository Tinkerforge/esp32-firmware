#!/usr/bin/env python3

import base64
import string
import json
import argparse
import os, sys
import tempfile

try:
    from esp_coredump import CoreDump
except:
    print("Failed to import esp_coredump. Install ESP-IDF and (on POSIX) run source /path/to/esp-idf/export.sh")
    sys.exit(-1)

PREFIX = b"___tf_coredump_data_start___"
SUFFIX = b"___tf_coredump_data_end___"

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
    parser = argparse.ArgumentParser()
    parser.add_argument("path")
    args = parser.parse_args(sys.argv[1:])
    path = args.path

    with open(path) as file:
        file_str = file.read()
        core_dump_b64 = file_str[file_str.rfind("___CORE_DUMP_START___\n\n") + 60:]
        core_dump = base64.b64decode(core_dump_b64)

        with open(core_dump_path, "wb") as f:
            f.write(core_dump)

        try:
            tf_coredump_json = get_tf_coredump_data(core_dump_path)
            tf_coredump_data = json.loads(tf_coredump_json)

            firmware_path = "build/" + tf_coredump_data['firmware_file_name'] + ".elf"
            if os.path.exists(firmware_path):
                coredump = CoreDump(chip='esp32', core=core_dump_path, core_format='elf', prog=firmware_path)
                coredump.dbg_corefile()
            else:
                print("Firmware {} not found".format(tf_coredump_data['firmware_file_name'] + ".elf"))
        finally:
            if os.path.exists(core_dump_path):
                os.remove(core_dump_path)
