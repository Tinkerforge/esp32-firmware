#!/usr/bin/env -S uv run --active --script

import re
import subprocess
import getpass
from tinkerforge_util.colored import green
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'provisioning')

from provisioning.provision_common.provision_common import *
from provisioning.provision_common.provision_common.zbase32 import ZBASE32

BASE58 = '123456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ'


def main():
    while True:
        qr_code = getpass.getpass(green('Scan ESP32 label: ')).strip()

        if len(qr_code) == 0:
            print('Aborted!')
            return

        m = re.fullmatch('WIFI:S:((?:esp32|warp|warp2|warp3|warp4|wem|wem2|seb|wallbox)-(?:[{0}]{{3,6}}|[{1}]{{3,7}}));T:WPA;P:([{0}]{{4}}-[{0}]{{4}}-[{0}]{{4}}-[{0}]{{4}});;'.format(BASE58, ZBASE32), qr_code)

        if m == None:
            print('Invalid ESP32 label: {0}'.format(qr_code))
            continue

        ssid = m.group(1)
        passphrase = m.group(2)

        print('SSID:', ssid)

        subprocess.check_call(['./print-esp32-label.py', ssid, passphrase])


if __name__ == '__main__':
    main()
