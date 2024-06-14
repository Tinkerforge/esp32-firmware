#!/usr/bin/python3 -u

import os
import ctypes
import ctypes.util


def main():
    directory = os.path.dirname(__file__)
    public_key_filename = os.path.normpath(os.path.join(directory, 'sodium_public_key.bin'))
    secret_key_filename = os.path.normpath(os.path.join(directory, 'sodium_secret_key.bin'))

    if os.path.exists(public_key_filename):
        print(f'error: public key {public_key_filename} already exists')
        return

    if os.path.exists(secret_key_filename):
        print(f'error: secret key {secret_key_filename} already exists')
        return

    libsodium_path = ctypes.util.find_library('sodium')

    if libsodium_path != None:
        libsodium = ctypes.cdll.LoadLibrary(libsodium_path)
    else:
        for extension in ['so', 'dll', 'dylib']:
            try:
                libsodium = ctypes.cdll.LoadLibrary(os.path.join(directory, f'libsodium.{extension}'))
            except:
                continue

            break
        else:
            print('error: cannot find libsodium')
            return

    if libsodium.sodium_init() < 0:
        print('error: sodium_init failed')
        return

    crypto_sign_PUBLICKEYBYTES = libsodium.crypto_sign_publickeybytes()
    crypto_sign_SECRETKEYBYTES = libsodium.crypto_sign_secretkeybytes()

    public_key = ctypes.create_string_buffer(crypto_sign_PUBLICKEYBYTES)
    secret_key = ctypes.create_string_buffer(crypto_sign_SECRETKEYBYTES)

    if libsodium.crypto_sign_keypair(public_key, secret_key) < 0:
        print('error: crypto_sign_keypair failed')
        return

    with open(public_key_filename, 'wb') as f:
        f.write(public_key)

    with open(secret_key_filename, 'wb') as f:
        f.write(secret_key)


if __name__ == '__main__':
    main()
