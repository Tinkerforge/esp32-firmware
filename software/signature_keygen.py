#!/usr/bin/python3 -u

import os
import sys
import ctypes
import ctypes.util


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


def main():
    libsodium = load_libsodium()

    if libsodium == None:
        return 1

    directory = os.path.dirname(__file__)
    public_key_filename = os.path.relpath(os.path.join(directory, 'signature_public_key.bin'))
    secret_key_filename = os.path.relpath(os.path.join(directory, 'signature_secret_key.bin'))

    if os.path.exists(public_key_filename):
        print(f'error: public key {public_key_filename} already exists')
        return 1

    if os.path.exists(secret_key_filename):
        print(f'error: secret key {secret_key_filename} already exists')
        return 1

    crypto_sign_PUBLICKEYBYTES = libsodium.crypto_sign_publickeybytes()
    crypto_sign_SECRETKEYBYTES = libsodium.crypto_sign_secretkeybytes()

    public_key_buffer = ctypes.create_string_buffer(crypto_sign_PUBLICKEYBYTES)
    secret_key_buffer = ctypes.create_string_buffer(crypto_sign_SECRETKEYBYTES)

    if libsodium.crypto_sign_keypair(public_key_buffer, secret_key_buffer) < 0:
        print('error: crypto_sign_keypair failed')
        return 1

    try:
        with open(public_key_filename, 'wb') as f:
            f.write(public_key_buffer.raw)
    except Exception as e:
        print(f'error: could not write public key {public_key_filename}: {e}')
        return 1

    try:
        with open(secret_key_filename, 'wb') as f:
            f.write(secret_key_buffer.raw)
    except Exception as e:
        print(f'error: could not write secret key {secret_key_filename}: {e}')
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
