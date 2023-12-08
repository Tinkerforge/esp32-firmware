#!/usr/bin/python3 -u

import os
import argparse
import re
import tempfile
import shlex

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('debug_protocol')

    args = parser.parse_args()
    found_header = False
    csv_fd, csv_path = tempfile.mkstemp(prefix=os.path.split(args.debug_protocol)[-1] + '-')

    with open(args.debug_protocol, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            if not found_header and re.match(r'^millis,(STATE)?,.*$', line.strip()) != None:
                found_header = True

            if found_header:
                if len(line.strip()) == 0:
                    break

                os.write(csv_fd, line.encode('utf-8'))

    os.close(csv_fd)
    os.system(f'libreoffice --calc {shlex.quote(csv_path)} --infilter="CSV:44"')

if __name__ == '__main__':
    main()
