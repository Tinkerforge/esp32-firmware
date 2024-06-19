#!/usr/bin/python3 -u

import os
import sys
import ctypes
import ctypes.util
import argparse
import re
import json


def load_libsodium():
    libsodium_path = ctypes.util.find_library('sodium')

    if libsodium_path != None:
        libsodium = ctypes.cdll.LoadLibrary(libsodium_path)
    else:
        directory = os.path.dirname(__file__)

        for extension in ['so', 'dll', 'dylib']:
            try:
                libsodium = ctypes.cdll.LoadLibrary(os.path.join(directory, f'libsodium.{extension}'))
            except:
                continue

            break
        else:
            print('error: cannot find libsodium library')
            return None

    if libsodium.sodium_init() < 0:
        print('error: sodium_init failed')
        return None

    return libsodium


def publisher_type(value):
    # limit to 63 to ensure NUL-termination
    if re.match(r'^[A-Za-z0-9,\. _-]{1,63}$', value) == None:
        raise argparse.ArgumentTypeError('publisher is malformed')

    return value


def main():
    libsodium = load_libsodium()

    if libsodium == None:
        return 1

    parser = argparse.ArgumentParser()
    parser.add_argument('--force', action='store_true')
    parser.add_argument('publisher', type=publisher_type)

    args = parser.parse_args()

    directory = os.path.dirname(__file__)
    public_key_filename = os.path.relpath(os.path.join(directory, 'signature_public_key.json'))
    secret_key_filename = os.path.relpath(os.path.join(directory, 'signature_secret_key.json'))

    if os.path.exists(public_key_filename):
        if args.force:
            try:
                os.remove(public_key_filename)
            except Exception as e:
                print(f'error: could not remove old public key file {public_key_filename}: {e}')
                return 1
        else:
            print(f'error: public key file {public_key_filename} already exists')
            return 1

    if os.path.exists(secret_key_filename):
        if args.force:
            try:
                os.remove(secret_key_filename)
            except Exception as e:
                print(f'error: could not remove old secret key file {secret_key_filename}: {e}')
                return 1
        else:
            print(f'error: secret key file {secret_key_filename} already exists')
            return 1

    crypto_sign_PUBLICKEYBYTES = libsodium.crypto_sign_publickeybytes()
    crypto_sign_SECRETKEYBYTES = libsodium.crypto_sign_secretkeybytes()

    public_key_buffer = ctypes.create_string_buffer(crypto_sign_PUBLICKEYBYTES)
    secret_key_buffer = ctypes.create_string_buffer(crypto_sign_SECRETKEYBYTES)

    if libsodium.crypto_sign_keypair(public_key_buffer, secret_key_buffer) < 0:
        print('error: crypto_sign_keypair failed')
        return 1

    public_key_json = json.dumps({'publisher': args.publisher, 'public_key': public_key_buffer.raw.hex()})

    try:
        with open(public_key_filename + '.tmp', 'w') as f:
            f.write(public_key_json + '\n')
    except Exception as e:
        print(f'error: could not write public key to {public_key_filename}.tmp: {e}')
        return 1

    try:
        os.replace(public_key_filename + '.tmp', public_key_filename)
    except Exception as e:
        print(f'error: could not rename public key from {public_key_filename}.tmp to {public_key_filename}: {e}')
        return 1

    secret_key_json = json.dumps({'publisher': args.publisher, 'secret_key': secret_key_buffer.raw.hex()})

    try:
        with open(secret_key_filename + '.tmp', 'w') as f:
            f.write(secret_key_json + '\n')
    except Exception as e:
        print(f'error: could not write secret key to {secret_key_filename}.tmp: {e}')
        return 1

    try:
        os.replace(secret_key_filename + '.tmp', secret_key_filename)
    except Exception as e:
        print(f'error: could not rename secret key from {secret_key_filename}.tmp to {secret_key_filename}: {e}')
        return 1

    print(f'success: signature keys created for {args.publisher}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
