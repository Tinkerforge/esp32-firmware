#!/usr/bin/python3 -u

import sys
import os

if sys.hexversion < 0x3000000:
    # Debian: python[2] -> python3
    sys.exit(os.system('{}3 {}'.format(sys.executable.rstrip('23'), __file__)))

from urllib.request import urlretrieve
from zipfile import ZipFile

PACKAGES = [
    ('arduino-esp32', 'warp-1.3.1', 'https://github.com/Tinkerforge/arduino-esp32'),
    ('arduino-esp32', 'warp2-1.1.1', 'https://github.com/Tinkerforge/arduino-esp32'),
]

packages_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'packages')

os.makedirs(packages_dir, exist_ok=True)

for package in PACKAGES:
    base, branch, url = package
    name = '{0}-{1}'.format(base, branch)
    marker_path = os.path.join(packages_dir, name, 'tinkerforge.marker')

    if os.path.exists(marker_path):
        print('skipping {0}'.format(name))
        continue

    zip_path = os.path.join(packages_dir, '{0}.zip'.format(name))

    if not os.path.exists(zip_path):
        print('downloading {0}'.format(name))

        try:
            os.remove(zip_path + '.tmp')
        except FileNotFoundError:
            pass

        urlretrieve('{0}/archive/refs/heads/{1}.zip'.format(url, branch), zip_path + '.tmp')

        os.rename(zip_path + '.tmp', zip_path)

    print('unpacking {0}'.format(name))

    with ZipFile(zip_path) as f:
        prefix = name + '/'

        for n in f.namelist():
            if not n.startswith(prefix):
                print('error: {0} has malformed entry {1}'.format(name, n))
                sys.exit(1)

        f.extractall(packages_dir)

    with open(marker_path, 'w') as f:
        pass
