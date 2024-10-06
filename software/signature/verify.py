#!/usr/bin/python3 -u

import os
import sys
import ctypes
import ctypes.util
import argparse
import subprocess
import json
import configparser
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
            raise Exception('Cannot find libsodium library')

    if libsodium.sodium_init() < 0:
        raise Exception('libsodium sodium_init failed')

    return libsodium


def main():
    libsodium = load_libsodium()

    parser = argparse.ArgumentParser()
    parser.add_argument('signature_name')
    parser.add_argument('input_path')

    args = parser.parse_args()

    config = configparser.ConfigParser()
    config.read(make_path('config.ini'))
    config = config['preset:' + config['signature:' + args.signature_name]['preset']]

    try:
        with open(args.input_path, 'rb') as f:
            input_data = bytearray(f.read())
    except Exception as e:
        raise Exception(f'Could not read input from {args.input_path}: {e}')

    if not config.getboolean('gpg_sign'):
        print('Skipping GPG verify')
    else:
        gpg_public_key_path = make_keys_path(config['gpg_public_key_path'])

        try:
            subprocess.check_call([
                'gpgv',
                '--keyring', gpg_public_key_path,
                args.input_path + '.sha256.asc', args.input_path + '.sha256',
            ])
        except:
            raise Exception('Could not verify GPG signature')

        print('GPG signature is valid')

        try:
            with open(args.input_path + '.sha256', 'r') as f:
                expected_sha256sum = f.read().split(' ')[0]
        except Exception as e:
            raise Exception(f'Could not read checksum from {args.input_path}.sha256: {e}')

        actual_sha256sum = hashlib.sha256(input_data).hexdigest()

        if actual_sha256sum != expected_sha256sum:
            raise Exception(f'Checksum mismatch: {repr(actual_sha256sum)} != {repr(expected_sha256sum)}')

        print('Checksum is matching')

    sodium_public_key_path = make_keys_path(config['sodium_public_key_path'])

    with open(sodium_public_key_path, 'r', encoding='utf-8') as f:
        sodium_public_key_json = json.loads(f.read())

    sodium_public_key = bytes.fromhex(sodium_public_key_json['sodium_public_key'])

    crypto_sign_PUBLICKEYBYTES = libsodium.crypto_sign_publickeybytes()

    if len(sodium_public_key) != crypto_sign_PUBLICKEYBYTES:
        raise Exception('Sodium public key has wrong size')

    crypto_sign_BYTES = libsodium.crypto_sign_bytes()
    assert(crypto_sign_BYTES == 64)

    firmware_info_offset = 0xd000 - 0x1000
    signature_info_offset = firmware_info_offset - 0x1000

    signature_info = bytearray(input_data[signature_info_offset:signature_info_offset + 0x1000])

    if signature_info[0:7] == bytes([0xff] * 7):
        raise Exception('Input file is not sodium signed')

    signature = bytes(signature_info[72:72 + crypto_sign_BYTES])
    signature_info[72:72 + crypto_sign_BYTES] = bytes([0x55] * crypto_sign_BYTES)

    if signature_info[4092:4096] != crc32(signature_info[0:4092]).to_bytes(4, byteorder='little'):
        raise Exception('Signature info is malformed')

    if signature_info[0:7] != bytes.fromhex('E6210F21EC1217'):
        raise Exception('Signature info is malformed')

    actual_publisher = signature_info[8:72].decode('utf-8').rstrip('\0')

    if actual_publisher != config['publisher']:
        raise Exception(f'Publisher mismatch: {repr(actual_publisher)} != {repr(config["publisher"])}')

    input_data[signature_info_offset + 72:signature_info_offset + 72 + crypto_sign_BYTES] = bytes([0x55] * crypto_sign_BYTES)

    state = CryptoSignState()

    if libsodium.crypto_sign_init(ctypes.byref(state)) < 0:
        raise Exception('libsodium crypto_sign_init failed')

    if libsodium.crypto_sign_update(ctypes.byref(state), bytes(input_data), len(input_data)) < 0:
        raise Exception('libsodium crypto_sign_update failed')

    if libsodium.crypto_sign_final_verify(ctypes.byref(state), signature, sodium_public_key) < 0:
        raise Exception(f'Sodium signature by {repr(actual_publisher)} is NOT valid')

    print(f'Sodium signature by {repr(actual_publisher)} is valid')


if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print(f'Error: {e}')
        sys.exit(1)
