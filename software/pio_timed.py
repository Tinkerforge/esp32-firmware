#!/usr/bin/env -S python3 -u

import sys
import subprocess
import time
import datetime

args = list(sys.argv[1:])
monitor = False
i = 0

while i < len(args):
    if args[i] == '-t' and i + 1 < len(args) and args[i + 1] == 'monitor':
        args.pop(i)
        args.pop(i)
        monitor = True
    else:
        i += 1

begin = time.monotonic()

try:
    return_code = subprocess.run(['pio'] + args).returncode

    if return_code != 0:
        sys.exit(return_code)
finally:
    end = time.monotonic()
    elasped = int(end - begin)

    print(f'ELASPED: {elasped} seconds')

    with open('pio_timed.log', 'a') as f:
        f.write(f'{datetime.datetime.now().isoformat()} {elasped}\n')

if monitor:
    sys.exit(subprocess.run(['./ff', '-s']).returncode)
