#!/usr/bin/python3 -u

import os
import json
from urllib.request import urlretrieve

def choose(title, choices):
    answer = None

    while answer == None:
        answer = input(title + ' ').strip().lower()

        if answer not in choices:
            answer = None
            print('Warning: Invalid answer!')

    return answer

def get(title, default):
    answer = None

    while answer == None:
        answer = input(title + ' ').strip()

        if len(answer) == 0:
            if default != None:
                answer = default
            else:
                answer = None
                print('Warning: Invalid answer!')

    return answer

def make_absolute_path(path):
    return os.path.join(os.path.dirname(os.path.realpath(__file__)), path)

def download_package_json(package_name, url, commit):
    package_path = make_absolute_path(os.path.join('packages', package_name))
    package_json_url = f'{url}/raw/{commit}/package.json'
    package_json_path = os.path.join(package_path, 'package.json')

    print(f'Downloading {package_json_url}')

    os.makedirs(package_path, exist_ok=True)
    urlretrieve(package_json_url, package_json_path + '.tmp')
    os.replace(package_json_path + '.tmp', package_json_path)

def write_config_json(config_json):
    with open(make_absolute_path(os.path.join('packages', 'config.json')), 'w', encoding='utf-8') as f:
        f.write(json.dumps(config_json, indent=4) + '\n')

def main():
    with open(make_absolute_path(os.path.join('packages', 'config.json')), 'r', encoding='utf-8') as f:
        config_json = json.loads(f.read())

    exisiting_package_names = set()

    for index, config in enumerate(config_json):
        exisiting_package_names.add(f"{config['base']}#{config['branch']}_{config['commit']}")

        print(f'Package {index}')
        print(f" Base:   {config['base']}")
        print(f" Branch: {config['branch']}")
        print(f" Commit: {config['commit']}")
        print(f" URL:    {config['url']}")
        print()

    answer = choose('[A]dd, [M]odify or [R]emove package?', ['a', 'm', 'r'])

    if answer == 'a':
        print(f'Package {len(config_json)}')

        base = get(' Base (default: arduino-esp32):', 'arduino-esp32')
        branch = get(' Branch:', None)
        commit = get(' Commit:', None)
        package_name = f'{base}#{branch}_{commit}'

        if package_name in exisiting_package_names:
            print('Error: Package already exists')
            return

        url = get(f' Base (default: https://github.com/Tinkerforge/{base}):', f'https://github.com/Tinkerforge/{base}').rstrip('/')

        download_package_json(package_name, url, commit)

        config_json.append({
            'base': base,
            'branch': branch,
            'commit': commit,
            'url': url,
        })

        write_config_json(config_json)

        print(f'Added package {package_name}')
        print()
        print('Next (for a git clone):')
        print(' Commit software/packages/config.json')
        print(f' Commit software/packages/{package_name}/package.json')
        print(' Modify platform_packages option in platformio.ini')
    elif answer == 'm':
        if len(config_json) == 0:
            print('No packages to modify')
            return

        index = int(get(f'Package [0..{len(config_json) - 1}]:', None))
        config = config_json[index]
        old_package_name = f"{config['base']}#{config['branch']}_{config['commit']}"
        old_package_path = make_absolute_path(os.path.join('packages', old_package_name))

        try:
            os.remove(os.path.join(old_package_path, 'tinkerforge.json'))
        except FileNotFoundError:
            pass

        config['base'] = get(f" Base (default: {config['base']}):", config['base'])
        config['branch'] = get(f" Branch (default: {config['branch']}):", config['branch'])
        config['commit'] = get(f" Commit (default: {config['commit']}):", config['commit'])
        config['url'] = get(f" URL (default: {config['url']}):", config['url'])
        new_package_name = f"{config['base']}#{config['branch']}_{config['commit']}"

        download_package_json(new_package_name, config['url'], config['commit'])
        write_config_json(config_json)

        print(f'Modified package {old_package_name}')
        print()
        print('Next (for a git clone):')
        print(' Commit software/packages/config.json')

        if old_package_name != new_package_name:
            print(f' Commit software/packages/{new_package_name}/package.json')

        print(' Modify platform_packages option in platformio.ini')
    elif answer == 'r':
        if len(config_json) == 0:
            print('No packages to remove')
            return

        index = int(get(f'Package [0..{len(config_json) - 1}]:', None))
        config = config_json[index]
        package_name = f"{config['base']}#{config['branch']}_{config['commit']}"

        config_json.pop(index)
        write_config_json(config_json)

        print(f'Removed package {package_name}')
        print()
        print('Next (for a git clone):')
        print(' Commit software/packages/config.json')
        print(' Modify platform_packages option in platformio.ini')

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Aborted')
