#!/usr/bin/python3 -u

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'provisioning')

import re
import subprocess
import getpass
from provisioning.provision_common.provision_common import *

BASE58 = '123456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ'


def main():
    while True:
        qr_code = getpass.getpass(green('Scan ESP32 label: ')).strip()

        if len(qr_code) == 0:
            print('Aborted!')
            return

        m = re.fullmatch('WIFI:S:((?:esp32|warp|warp2|warp3|wem|seb)-[{0}]{{3,6}});T:WPA;P:([{0}]{{4}}-[{0}]{{4}}-[{0}]{{4}}-[{0}]{{4}});;'.format(BASE58), qr_code)

        if m == None:
            print('Invalid ESP32 label: {0}'.format(qr_code))
            continue

        ssid = m.group(1)
        passphrase = m.group(2)

        print('SSID:', ssid)
        print('Passphrase:', passphrase)

        subprocess.check_call(['./print-esp32-label.py', ssid, passphrase])


if __name__ == '__main__':
    main()
