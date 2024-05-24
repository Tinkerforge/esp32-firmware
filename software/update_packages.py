import sys

if sys.hexversion < 0x3060000:
    print('Python >= 3.6 required')
    sys.exit(1)

import os
import json
import shutil
import shlex
from urllib.request import urlretrieve
from zipfile import ZipFile

import util

VERSION = '2.1.0'

util.log('Updating packages')

with open('packages/config.json', 'r', encoding='utf-8') as f:
    config_json = json.loads(f.read())

config = {}

for c in config_json:
    config[c['base'] + '#' + c['branch'] + '_' + c['commit']] = c

for name in sorted(os.listdir('packages')):
    if name == 'config.json':
        continue

    package_path = os.path.join('packages', name)

    if not os.path.exists(package_path):
        continue  # directory content changed while listing the directory

    if name.endswith('-dev'):
        util.log('Ignoring {0}'.format(name))
        continue

    if name not in config:
        if not os.path.isdir(package_path):
            print('Removing {0}'.format(name))
            os.remove(package_path)
        else:
            package_json_path = os.path.join('packages', name, 'package.json')

            try:
                with open(os.path.join(package_json_path), 'r', encoding='utf-8') as f:
                    package_json = f.read()
            except FileNotFoundError:
                package_json = None

            shutil.rmtree(package_path)

            if package_json == None:
                print('Removing {0}'.format(name))
            else:
                util.log('Clearing {0}'.format(name))
                os.makedirs(package_path)

                with open(package_json_path, 'w', encoding='utf-8') as f:
                    f.write(package_json)

        continue

    tinkerforge_json_path = os.path.join('packages', name, 'tinkerforge.json')

    base = config[name]['base']
    branch = config[name]['branch']
    commit = config[name]['commit']
    url = config[name]['url']

    try:
        with open(tinkerforge_json_path, 'r', encoding='utf-8') as f:
            tinkerforge_json = json.loads(f.read())

            if tinkerforge_json.get('version') == VERSION and tinkerforge_json.get('url') == url:
                util.log('Skipping {0}'.format(name))
                continue
    except FileNotFoundError:
        pass

    zip_path = os.path.join('packages', '{0}.zip'.format(name))

    try:
        os.remove(zip_path)
    except FileNotFoundError:
        pass

    print('Downloading {0}'.format(name))

    try:
        os.remove(zip_path + '.tmp')
    except FileNotFoundError:
        pass

    zip_url = '{0}/archive/{1}.zip'.format(url, commit)

    try:
        urlretrieve(zip_url, zip_path + '.tmp')
    except Exception as e:
        print('Error while downloading {0}: {1}'.format(zip_url, e))
        print('Retrying with curl')

        command = shlex.join(['curl', '-s', '-L', zip_url, '-o', zip_path + '.tmp'])

        if sys.platform == 'win32':
            command = 'powershell.exe ' + command

        if os.system(command) != 0:
            print('Error while downloading {0} with curl'.format(zip_url))
            sys.exit(1)

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

    with open(os.path.join('packages', name, '.gitignore'), 'w', encoding='utf-8') as f:
        f.write('*\n!package.json\n')

    with open(tinkerforge_json_path, 'w', encoding='utf-8') as f:
        f.write(json.dumps({'version': VERSION, 'url': url}))

    os.remove(zip_path)
