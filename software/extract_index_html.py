#!/usr/bin/env python3

import os
import re
import argparse
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('elf_file')
    parser.add_argument('index_html_file')

    args = parser.parse_args()

    sections = subprocess.check_output(['pio', 'pkg', 'exec', 'xtensa-esp32-elf-readelf', '--', '-S', args.elf_file], encoding='utf-8')

    flash_rodata_offset = None

    for section in sections.split('\n'):
        m = re.search(r'\s\.flash\.rodata\s+PROGBITS\s+([A-Fa-f0-9]+)\s', section)

        if m == None:
            continue

        flash_rodata_offset = int(m.group(1), 16)

    symbols = subprocess.check_output(['pio', 'pkg', 'exec', 'xtensa-esp32-elf-objdump', '--', '-t', args.elf_file], encoding='utf-8')

    index_html_data_offset = None
    index_html_data_length = None

    for symbol in symbols.split('\n'):
        m = re.match(r'^([A-Fa-f0-9]+)\s.*\s([A-Fa-f0-9]+)\s+index_html_data$', symbol)

        if m == None:
            continue

        index_html_data_offset = int(m.group(1), 16) - flash_rodata_offset
        index_html_data_length = int(m.group(2), 16)

    with tempfile.NamedTemporaryFile() as t:
        subprocess.check_call(['pio', 'pkg', 'exec', 'xtensa-esp32-elf-objcopy', '--', args.elf_file, '/dev/null', '--dump-section', '.flash.rodata=' + t.name])
        flash_rodata = t.read()

    with open(args.index_html_file + '.tmp', 'wb') as f:
        f.write(flash_rodata[index_html_data_offset:index_html_data_offset + index_html_data_length])
        f.flush()

    os.rename(args.index_html_file + '.tmp', args.index_html_file)


if __name__ == '__main__':
    main()
