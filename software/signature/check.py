#!/usr/bin/python3 -u

import os
import sys
import argparse
import configparser
import subprocess
import getpass

directory = os.path.normpath(os.path.dirname(__file__))


def make_path(path):
    if os.path.isabs(path):
        return path

    return os.path.join('.', os.path.relpath(os.path.join(directory, path)))


def make_keys_path(path):
    if os.path.isabs(path):
        return path

    return os.path.join('.', os.path.relpath(os.path.join(directory, 'keys', path)))


def keepassxc(preset, prefix, action, args, entry, password=None, input=None):
    path = make_keys_path(preset[prefix + '_path'])
    protection = preset[prefix + '_protection']
    full_args = ['keepassxc-cli', action]
    full_kwargs = {'stderr': subprocess.DEVNULL, 'encoding': 'utf-8'}
    full_input = None

    if protection == 'token':
        full_args += ['-q', '--no-password', '-y', f'2:{preset[prefix + "_token"]}']
    elif protection == 'keyfile':
        full_args += ['-q', '--no-password', '-k', make_keys_path(preset[prefix + '_keyfile'])]
    elif protection == 'password':
        assert password != None
        full_input = password + '\n'
    else:
        raise Exception(f'Invalid protection: {protection}')

    full_args += args + [path, entry]

    if input != None:
        if full_input != None:
            full_input += input
        else:
            full_input = input

    if full_input != None:
        full_kwargs['input'] = full_input

    try:
        return subprocess.check_output(full_args, **full_kwargs)
    except:
        return None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('preset')

    args = parser.parse_args()

    config = configparser.ConfigParser()
    config.read(make_path('config.ini'))

    try:
        preset = dict(config['preset:' + args.preset])
    except KeyError:
        raise Exception(f'Preset {args.preset} is unknown, maybe the signature data is outdated')

    if 'extends' in preset:
        extends = preset['extends']

        for key in config[extends]:
            if key not in preset:
                preset[key] = config.get(extends, key)

    sodium_secret_key_path = make_keys_path(preset['sodium_secret_key_path'])

    print(f'Reading sodium secret key entry from {sodium_secret_key_path}')

    if not os.path.exists(sodium_secret_key_path):
        raise Exception(f'Sodium secret key file {sodium_secret_key_path} is missing')

    sodium_secret_key_hex = None

    while sodium_secret_key_hex == None:
        sodium_secret_key_password = None

        if preset['sodium_secret_key_protection'] == 'password':
            print(f'Enter password for sodium secret key file {sodium_secret_key_path}:')

            try:
                sodium_secret_key_password = getpass.getpass(prompt='')
            except KeyboardInterrupt:
                raise Exception('Aborted')

        sodium_secret_key_hex = keepassxc(preset, 'sodium_secret_key', 'show', [], 'sodium_secret_key', password=sodium_secret_key_password)

        if sodium_secret_key_hex == None:
            message = f'Could not read sodium secret key entry from {sodium_secret_key_path}'

            if preset['sodium_secret_key_protection'] == 'password':
                print(message)
            else:
                raise Exception(message)

    if preset['gpg_sign'] != 'true':
        print('Skipping GPG keyring')
    else:
        gpg_keyring_passphrase_path = make_keys_path(preset['gpg_keyring_passphrase_path'])

        print(f'Reading GPG keyring passphrase entry from {gpg_keyring_passphrase_path}')

        if not os.path.exists(gpg_keyring_passphrase_path):
            raise Exception(f'GPG keyring passphrase file {gpg_keyring_passphrase_path} is missing')

        gpg_keyring_passphrase = None

        while gpg_keyring_passphrase == None:
            gpg_keyring_passphrase_password = None

            if preset['gpg_keyring_passphrase_protection'] == 'password':
                print(f'Enter password for GPG keyring passphrase file {gpg_keyring_passphrase_path}:')

                try:
                    gpg_keyring_passphrase_password = getpass.getpass(prompt='')
                except KeyboardInterrupt:
                    raise Exception('Aborted')

            gpg_keyring_passphrase = keepassxc(preset, 'gpg_keyring_passphrase', 'show', [], 'gpg_keyring_passphrase', password=gpg_keyring_passphrase_password)

            if gpg_keyring_passphrase == None:
                message = f'Could not read GPG keyring passphrase entry from {gpg_keyring_passphrase_path}'

                if preset['gpg_keyring_passphrase_protection'] == 'password':
                    print(message)
                else:
                    raise Exception(message)

        gpg_keyring_passphrase = gpg_keyring_passphrase.strip()

        gpg_keyring_path = make_keys_path(preset['gpg_keyring_path'])

        if not os.path.exists(gpg_keyring_path):
            raise Exception(f'GPG keyring file {gpg_keyring_path} is missing')

        try:
            subprocess.check_call([
                'gpg',
                '--pinentry-mode', 'loopback',
                '--no-default-keyring',
                '--keyring', gpg_keyring_path,
                '--passphrase', gpg_keyring_passphrase,
                '--list-secret-keys',
            ])
        except:
            raise Exception(f'Could not list secret key from GPG keyring')

    print('Check successful')


if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print(f'Error: {e}')
        sys.exit(1)
