import sys

if sys.hexversion < 0x3060000:
    print('Python >= 3.6 required')
    sys.exit(1)

import os
import json
import shutil
from urllib.request import urlretrieve
from zipfile import ZipFile

VERSION = '2.0.0'

print('Updating packages')

with open('packages/config.json', 'r') as f:
    config_json = json.loads(f.read())

config = {}

for c in config_json:
    config[c['base'] + '#' + c['branch'] + '_' + c['commit']] = c

for name in sorted(os.listdir('packages')):
    if name == 'config.json':
        continue

    if name.endswith('-dev'):
        print('Ignoring {0}'.format(name))
        continue

    if name not in config:
        package_path = os.path.join('packages', name)

        if not os.path.isdir(package_path):
            print('Removing {0}'.format(name))
            os.remove(package_path)
        else:
            package_json_path = os.path.join('packages', name, 'package.json')

            try:
                with open(os.path.join(package_json_path), 'r') as f:
                    package_json = f.read()
            except FileNotFoundError:
                package_json = None

            shutil.rmtree(package_path)

            if package_json == None:
                print('Removing {0}'.format(name))
            else:
                print('Clearing {0}'.format(name))
                os.makedirs(package_path)

                with open(package_json_path, 'w') as f:
                    f.write(package_json)

        continue

    tinkerforge_json_path = os.path.join('packages', name, 'tinkerforge.json')

    base = config[name]['base']
    branch = config[name]['branch']
    commit = config[name]['commit']
    url = config[name]['url']

    try:
        with open(tinkerforge_json_path, 'r') as f:
            tinkerforge_json = json.loads(f.read())

            if tinkerforge_json.get('version') == VERSION:
                print('Skipping {0}'.format(name))
                continue
    except FileNotFoundError:
        pass

    zip_path = os.path.join('packages', '{0}.zip'.format(name))

    if os.path.exists(zip_path):
        os.remove(zip_path)

    print('Downloading {0}'.format(name))

    try:
        os.remove(zip_path + '.tmp')
    except FileNotFoundError:
        pass

    urlretrieve('{0}/archive/{1}.zip'.format(url, commit), zip_path + '.tmp')
    os.rename(zip_path + '.tmp', zip_path)

    print('Unpacking {0}'.format(name))

    try:
        shutil.rmtree(os.path.join('packages', name))
    except FileNotFoundError:
        pass

    with ZipFile(zip_path) as zf:
        prefix_zip = base + '-' + commit + '/'
        prefix_fs = base + '#' + branch + '_' + commit + '/'

        for n in zf.namelist():
            if not n.startswith(prefix_zip):
                print('Error: {0} has malformed entry {1}. Expected prefix {2}'.format(name, n, prefix_zip))
                sys.exit(1)

            with zf.open(n) as f:
                data = f.read()

            path = os.path.join('packages', n.replace(prefix_zip, prefix_fs))
            path_dir, path_file = os.path.split(path)

            os.makedirs(path_dir, exist_ok=True)

            if len(path_file) == 0:
                continue

            with open(path, 'wb') as f:
                f.write(data)

    with open(os.path.join('packages', name, '.gitignore'), 'w') as f:
        f.write('*\n!package.json\n')

    with open(tinkerforge_json_path, 'w') as f:
        f.write(json.dumps({'version': VERSION}))

    os.remove(zip_path)
