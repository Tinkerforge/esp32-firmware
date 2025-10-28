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
except KeyboardInterrupt:
    sys.exit(1)
finally:
    end = time.monotonic()
    elapsed = int(end - begin)

    print(f'ELAPSED: {datetime.timedelta(seconds=elapsed)}')

    with open('pio_timed.log', 'a') as f:
        f.write(f'{datetime.datetime.now().isoformat()} {elapsed} {" ".join(args)}\n')

if monitor:
    try:
        sys.exit(subprocess.run(['./ff', '-s']).returncode)
    except KeyboardInterrupt:
        sys.exit(1)
