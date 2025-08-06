import sys

if sys.hexversion < 0x3060000:
    print('Python >= 3.6 required')
    sys.exit(1)

import glob
import configparser
import subprocess


def main():
    for ini_path in sorted(glob.glob('*.ini')):
        if ini_path.endswith('platformio.ini'):
            continue

        config = configparser.ConfigParser()
        config.read(ini_path)

        for section in config.sections():
            if not section.startswith('env:'):
                continue

            env_name = section[4:]
            args = ['pio', 'run', '-e', env_name, '-j', '2']

            subprocess.check_call(args + ['-t', 'clean'])
            subprocess.check_call(args)


if __name__ == '__main__':
    main()
