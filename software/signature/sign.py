#!/usr/bin/python3 -u

import os
import sys
import ctypes
import ctypes.util
import argparse
import configparser
import subprocess
import getpass
import hashlib
from zlib import crc32

directory = os.path.normpath(os.path.dirname(__file__))


class CryptoSignState(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ('state', ctypes.c_uint64 * 8),
        ('count', ctypes.c_uint64 * 2),
        ('buf', ctypes.c_uint8 * 128)
    ]


def make_path(path):
    if os.path.isabs(path):
        return path

    return os.path.join('.', os.path.relpath(os.path.join(directory, path)))


def make_keys_path(path):
    if os.path.isabs(path):
        return path

    return os.path.join('.', os.path.relpath(os.path.join(directory, 'keys', path)))


def load_libsodium():
    libsodium_path = ctypes.util.find_library('sodium')

    if libsodium_path != None:
        libsodium = ctypes.cdll.LoadLibrary(libsodium_path)
    else:
        for extension in ['so', 'dll', 'dylib']:
            try:
                libsodium = ctypes.cdll.LoadLibrary(make_path(f'libsodium.{extension}'))
            except:
                continue

            break
        else:
            raise Exception('cannot find libsodium library')

    if libsodium.sodium_init() < 0:
        raise Exception('libsodium sodium_init failed')

    return libsodium


def keepassxc(config, prefix, action, args, entry, password=None, input=None):
    path = make_keys_path(config[prefix + '_path'])
    protection = config[prefix + '_protection']
    full_args = ['keepassxc-cli', action]
    full_kwargs = {'stderr': subprocess.DEVNULL, 'encoding': 'utf-8'}
    full_input = None

    if protection == 'token':
        full_args += ['-q', '--no-password', '-y', f'2:{config[prefix + "_token"]}']
    elif protection == 'keyfile':
        full_args += ['-q', '--no-password', '-k', make_keys_path(config[prefix + '_keyfile'])]
    elif protection == 'password':
        assert password != None
        full_input = password + '\n'
    else:
        raise Exception(f'invalid protection: {protection}')

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
    libsodium = load_libsodium()

    parser = argparse.ArgumentParser()
    parser.add_argument('--force-overwrite', action='store_true')
    parser.add_argument('--sodium-secret-key-password', nargs='?')
    parser.add_argument('--gpg-keyring-passphrase-password', nargs='?')
    parser.add_argument('signature_name')
    parser.add_argument('input_path')
    parser.add_argument('output_path')

    args = parser.parse_args()

    if not os.path.exists(args.input_path):
        raise Exception(f'input file {args.input_path} is missing')

    if os.path.exists(args.output_path):
        if not args.force_overwrite:
            raise Exception(f'output file {args.output_path} already exists')

        if os.path.samefile(args.input_path, args.output_path):
            raise Exception(f'input file {args.input_path} and output file {args.output_path} are the same')

        print(f'removing existing output file {args.output_path} [--force-overwrite]')

        try:
            os.remove(args.output_path)
        except Exception as e:
            raise Exception(f'could not remove existing output file {args.output_path}: {e}')

    config = configparser.ConfigParser()
    config.read(make_path('config.ini'))
    config = config['preset:' + config['signature:' + args.signature_name]['preset']]

    publisher_bytes = config['publisher'].encode('utf-8')

    if len(publisher_bytes) < 1 or len(publisher_bytes) > 63:
        raise Exception('signature publisher UTF-8 length is out of range')

    publisher = repr(publisher_bytes)[2:-1].replace('"', '\\"')

    sodium_secret_key_path = make_keys_path(config['sodium_secret_key_path'])

    print(f'reading sodium secret key entry from {sodium_secret_key_path}')

    if not os.path.exists(sodium_secret_key_path):
        raise Exception(f'sodium secret key file {sodium_secret_key_path} is missing')

    sodium_secret_key_password = args.sodium_secret_key_password

    if config['sodium_secret_key_protection'] == 'password' and sodium_secret_key_password == None:
        print(f'enter password for sodium secret key file {sodium_secret_key_path}:')
        sodium_secret_key_password = getpass.getpass(prompt='')

    sodium_secret_key_hex = keepassxc(config, 'sodium_secret_key', 'show', ['-s', '-a', 'password'], 'sodium_secret_key', password=sodium_secret_key_password)

    if sodium_secret_key_hex == None:
        raise Exception(f'could not read sodium secret key entry from {sodium_secret_key_path}')

    sodium_secret_key = bytes.fromhex(sodium_secret_key_hex)

    crypto_sign_SECRETKEYBYTES = libsodium.crypto_sign_secretkeybytes()

    if len(sodium_secret_key) != crypto_sign_SECRETKEYBYTES:
        raise Exception('sodium secret key has wrong size')

    state = CryptoSignState()

    if libsodium.crypto_sign_init(ctypes.byref(state)) < 0:
        raise Exception('libsodium crypto_sign_init failed')

    try:
        with open(args.input_path, 'rb') as f:
            input_data = bytearray(f.read())
    except Exception as e:
        raise Exception(f'could not read input from {args.input_path}: {e}')

    crypto_sign_BYTES = libsodium.crypto_sign_bytes()
    assert(crypto_sign_BYTES == 64)

    firmware_info_offset = 0xd000 - 0x1000
    signature_info_offset = firmware_info_offset - 0x1000

    signature_info = bytearray([0xff] * 4096)
    signature_info[0:7] = bytes.fromhex('E6210F21EC1217')  # firmware info magic in reverse
    signature_info[7] = 0x01  # signature info version, note: a new version has to be backwards compatible
    signature_info[8:8 + len(publisher_bytes)] = publisher_bytes
    signature_info[8 + len(publisher_bytes):71] = bytes(63 - len(publisher_bytes))
    signature_info[71] = 0x00  # 0 byte to make sure string is terminated
    signature_info[72:72 + crypto_sign_BYTES] = bytes([0x55] * crypto_sign_BYTES)  # signature, will be inserted later
    signature_info[4092:4096] = crc32(signature_info[0:4092]).to_bytes(4, byteorder='little')

    input_data[signature_info_offset:signature_info_offset + len(signature_info)] = signature_info

    if libsodium.crypto_sign_update(ctypes.byref(state), bytes(input_data), len(input_data)) < 0:
        raise Exception('libsodium crypto_sign_update failed')

    signature_buffer = ctypes.create_string_buffer(crypto_sign_BYTES)

    if libsodium.crypto_sign_final_create(ctypes.byref(state), signature_buffer, 0, sodium_secret_key) < 0:
        raise Exception('libsodium crypto_sign_final_create failed')

    input_data[signature_info_offset + 72:signature_info_offset + 72 + crypto_sign_BYTES] = signature_buffer.raw

    try:
        with open(args.output_path + '.tmp', 'wb') as f:
            f.write(input_data)
    except Exception as e:
        raise Exception(f'could not write output to {args.output_path}.tmp: {e}')

    try:
        os.replace(args.output_path + '.tmp', args.output_path)
    except Exception as e:
        raise Exception(f'could not rename output file from {args.output_path}.tmp to {args.output_path}: {e}')

    if not config.getboolean('gpg_sign'):
        print('skipping GPG signature')
    else:
        sha256sum = hashlib.sha256(input_data).hexdigest()

        try:
            with open(args.output_path + '.sha256.tmp', 'w', encoding='utf-8') as f:
                f.write(f'{sha256sum}  {os.path.split(args.output_path)[-1]}\n')
        except Exception as e:
            raise Exception(f'could not write checksum to {args.output_path}.sha256.tmp: {e}')

        try:
            os.replace(args.output_path + '.sha256.tmp', args.output_path + '.sha256')
        except Exception as e:
            raise Exception(f'could not rename checksum file from {args.output_path}.tmp to {args.output_path}.sha256: {e}')

        gpg_keyring_passphrase_path = make_keys_path(config['gpg_keyring_passphrase_path'])

        print(f'reading GPG keyring passphrase entry from {gpg_keyring_passphrase_path}')

        if not os.path.exists(gpg_keyring_passphrase_path):
            raise Exception(f'GPG keyring passphrase file {gpg_keyring_passphrase_path} is missing')

        gpg_keyring_passphrase_password = args.gpg_keyring_passphrase_password

        if config['gpg_keyring_passphrase_protection'] == 'password' and gpg_keyring_passphrase_password == None:
            print(f'enter password for GPG keyring passphrase file {gpg_keyring_passphrase_path}:')
            gpg_keyring_passphrase_password = getpass.getpass(prompt='')

        gpg_keyring_passphrase = keepassxc(config, 'gpg_keyring_passphrase', 'show', ['-s', '-a', 'password'], 'gpg_keyring_passphrase', password=gpg_keyring_passphrase_password)

        if gpg_keyring_passphrase == None:
            raise Exception(f'could not read GPG keyring passphrase entry from {gpg_keyring_passphrase_path}')

        gpg_keyring_passphrase = gpg_keyring_passphrase.strip()

        gpg_keyring_path = make_keys_path(config['gpg_keyring_path'])

        if not os.path.exists(gpg_keyring_path):
            raise Exception(f'GPG keyring file {gpg_keyring_path} is missing')

        try:
            subprocess.check_call([
                'gpg',
                '--pinentry-mode', 'loopback',
                '--no-default-keyring',
                '--keyring', gpg_keyring_path,
                '--passphrase', gpg_keyring_passphrase,
                '--output', args.output_path + '.sha256.asc',
                '--armor',
                '--detach-sign', args.output_path + '.sha256',
            ])
        except:
            raise Exception(f'could not GPG sign {args.output_path}.sha256')

    print(f'successfully signed by {repr(publisher)}: {args.output_path}')

    # FIXME: force token unplugging now


if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print(f'error: {e}')
        sys.exit(1)
