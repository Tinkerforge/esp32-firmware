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

def main():
    with open(make_absolute_path(os.path.join('packages', 'config.json')), 'r') as f:
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

    answer = choose('[A]dd or [R]emove package?', ['a', 'r'])

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
        package_path = make_absolute_path(os.path.join('packages', package_name))
        package_json_url = f'{url}/raw/{commit}/package.json'
        package_json_path = os.path.join(package_path, 'package.json')

        print(f'Downloading {package_json_url}')

        os.makedirs(package_path, exist_ok=True)
        urlretrieve(package_json_url, package_json_path + '.tmp')
        os.replace(package_json_path + '.tmp', package_json_path)

        config_json.append({
            'base': base,
            'branch': branch,
            'commit': commit,
            'url': url,
        })

        with open(make_absolute_path(os.path.join('packages', 'config.json')), 'w') as f:
            f.write(json.dumps(config_json, indent=4) + '\n')

        print(f'Added package {package_name}')
    elif answer == 'r':
        index = int(get(f'Package [0..{len(config_json) - 1}]:', None))
        config = config_json[index]
        package_name = f"{config['base']}#{config['branch']}_{config['commit']}"

        config_json.pop(index)

        with open(make_absolute_path(os.path.join('packages', 'config.json')), 'w') as f:
            f.write(json.dumps(config_json, indent=4) + '\n')

        print(f'Removed package {package_name}')

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Aborted')
