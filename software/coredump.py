#!/usr/bin/env python3

from esp_coredump import CoreDump
import base64
import string
import json
import argparse
import os, sys

def get_tf_coredump_json(coredump_path: str):
    tf_coredump_json = get_tf_coredump_data(coredump_path)[28:]
    tf_coredump_json = tf_coredump_json[:tf_coredump_json.rfind("___tf_coredump_data_end___")]
    return tf_coredump_json

def get_tf_coredump_data(coredump_path: str):
    printable = set(string.printable)
    found_str = ""
    with open(coredump_path, "rb") as file:
        while True:
            data = file.read(1024*4)
            if not data:
                break
            for char in data:
                char = chr(char)
                if char in printable:
                    found_str += char
                elif found_str.find("___tf_coredump_data_end___") != -1:
                    return found_str
                else:
                    found_str = ""


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("path")
    args = parser.parse_args(sys.argv[1:])
    path = args.path

    with open(path) as file:
        file_str = file.read()
        core_dump_b64 = file_str[file_str.rfind("___CORE_DUMP_START___\n\n") + 60:]
        core_dump = base64.b64decode(core_dump_b64)
        with open("/tmp/coredump.elf", "wb") as core_dump_file:
            core_dump_file.write(core_dump)

        tf_coredump_json = get_tf_coredump_json("/tmp/coredump.elf")
        tf_coredump_data = json.loads(tf_coredump_json)
        firmware_path = "build/" + tf_coredump_data['firmware_file_name'] + ".elf"
        if os.path.exists(firmware_path):
            coredump = CoreDump(chip='esp32', core="/tmp/coredump.elf", core_format='elf', prog=firmware_path)
            coredump.dbg_corefile()
        else:
            print("Firmware {} not found".format(tf_coredump_data['firmware_file_name'] + ".elf"))