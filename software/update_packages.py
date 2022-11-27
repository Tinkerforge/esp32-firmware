import sys

if sys.hexversion < 0x3060000:
    print('Python >= 3.6 required')
    sys.exit(1)

import os
import json
import shutil
from urllib.request import urlretrieve
from zipfile import ZipFile

VERSION = '1.0.0'

print('Updating packages')

with open('packages/config.json', 'r') as f:
    config_json = json.loads(f.read())

config = {}

for c in config_json:
    config[c['base'] + '#' + c['branch']] = c
    config[c['base'] + '-' + c['branch']] = c

    if c['commit'] != None:
        config[c['base'] + '#' + c['branch'] + '_' + c['commit']] = c
        config[c['base'] + '-' + c['branch'] + '_' + c['commit']] = c

for name in sorted(os.listdir('packages')):
    if name == 'config.json':
        continue

    if name.endswith('-dev'):
        print('Ignoring {0}'.format(name))
        continue

    if name not in config:
        print('Removing {0}'.format(name))

        path = os.path.join('packages', name)

        try:
            shutil.rmtree(path)
        except NotADirectoryError:
            os.remove(path)

        continue

    active = config[name]['active']

    if not active:
        package_path = os.path.join('packages', name)
        package_json_path = os.path.join('packages', name, 'package.json')

        with open(os.path.join(package_json_path), 'r') as f:
            package_json = f.read()

        shutil.rmtree(package_path)
        os.makedirs(package_path)

        with open(package_json_path, 'w') as f:
            f.write(package_json)

        with open(os.path.join(package_path, '.gitignore'), 'w') as f:
            if '#' in name:
                f.write('*\n!package.json\n')
            else:
                f.write('*\n')
    else:
        tinkerforge_json_path = os.path.join('packages', name, 'tinkerforge.json')

        try:
            with open(tinkerforge_json_path, 'r') as f:
                tinkerforge_json = json.loads(f.read())

                if tinkerforge_json.get('version') == VERSION:
                    print('Skipping {0}'.format(name))
                    continue
        except FileNotFoundError:
            pass

        base = config[name]['base']
        branch = config[name]['branch']
        commit = config[name]['commit']
        url = config[name]['url']
        zip_path = os.path.join('packages', '{0}.zip'.format(name))

        if os.path.exists(zip_path):
            os.remove(zip_path)

        print('Downloading {0}'.format(name))

        try:
            os.remove(zip_path + '.tmp')
        except FileNotFoundError:
            pass

        if commit == None:
            urlretrieve('{0}/archive/refs/heads/{1}.zip'.format(url, branch), zip_path + '.tmp')
        else:
            urlretrieve('{0}/archive/{1}.zip'.format(url, commit), zip_path + '.tmp')

        os.rename(zip_path + '.tmp', zip_path)

        print('Unpacking {0}'.format(name))

        try:
            shutil.rmtree(os.path.join('packages', name))
        except FileNotFoundError:
            pass

        with ZipFile(zip_path) as zf:
            if commit == None:
                prefix_zip = base + '-' + branch + '/'
                prefix_fs = base + '#' + branch + '/'
            else:
                prefix_zip = base + '-' + commit + '/'
                prefix_fs = base + '#' + branch + '_' + commit + '/'

            for n in zf.namelist():
                if not n.startswith(prefix_zip):
                    print('Error: {0} has malformed entry {1}'.format(name, n))
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
