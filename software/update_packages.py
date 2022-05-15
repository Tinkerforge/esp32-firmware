Import('env')

import sys

if sys.hexversion < 0x3060000:
    print('Python >= 3.6 required')
    sys.exit(1)

import os
from urllib.request import urlretrieve
from zipfile import ZipFile

PACKAGES = [
    ('arduino-esp32', 'warp-2.0.2', 'https://github.com/Tinkerforge/arduino-esp32'),
    ('arduino-esp32', 'warp2-2.0.2', 'https://github.com/Tinkerforge/arduino-esp32'),
]

print('Updating local packages')

os.makedirs('packages', exist_ok=True)

for package in PACKAGES:
    base, branch, url = package
    name = '{0}-{1}'.format(base, branch)
    marker_path = os.path.join('packages', name, 'tinkerforge.marker')

    if os.path.exists(marker_path):
        print('Skipping {0}'.format(name))
        continue

    zip_path = os.path.join('packages', '{0}.zip'.format(name))

    if not os.path.exists(zip_path):
        print('Downloading {0}'.format(name))

        try:
            os.remove(zip_path + '.tmp')
        except FileNotFoundError:
            pass

        urlretrieve('{0}/archive/refs/heads/{1}.zip'.format(url, branch), zip_path + '.tmp')

        os.rename(zip_path + '.tmp', zip_path)

    print('Unpacking {0}'.format(name))

    with ZipFile(zip_path) as f:
        prefix = name + '/'

        for n in f.namelist():
            if not n.startswith(prefix):
                print('Error: {0} has malformed entry {1}'.format(name, n))
                sys.exit(1)

        f.extractall('packages')

    with open(marker_path, 'w') as f:
        pass
