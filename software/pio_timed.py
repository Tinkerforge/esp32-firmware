#!/usr/bin/env -S python3 -u

# USAGE: ./pio_timed.py run -e energy_manager -t upload && ./ff -s

import sys
import subprocess
import time
import datetime

begin = time.monotonic()

try:
    sys.exit(subprocess.run(['pio'] + sys.argv[1:]).returncode)
finally:
    end = time.monotonic()
    elasped = int(end - begin)

    print(f'ELASPED: {elasped} seconds')

    with open('pio_timed.log', 'a') as f:
        f.write(f'{datetime.datetime.now().isoformat()} {elasped}\n')

