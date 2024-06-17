#!/usr/bin/python3 -u

import os
import sys
import ctypes
import ctypes.util
import argparse


class CryptoSignState(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ('state', ctypes.c_uint64 * 8),
        ('count', ctypes.c_uint64 * 2),
        ('buf', ctypes.c_uint8 * 128)
    ]


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

    parser = argparse.ArgumentParser()
    parser.add_argument('input_filename')
    parser.add_argument('output_filename')

    args = parser.parse_args()

    if os.path.exists(args.output_filename):
        print(f'error: output {args.output_filename} already exists')
        return 1

    directory = os.path.dirname(__file__)
    secret_key_filename = os.path.relpath(os.path.join(directory, 'sodium_secret_key.bin'))

    try:
        with open(secret_key_filename, 'rb') as f:
            secret_key = f.read()
    except Exception as e:
        print(f'error: could not read secret key {secret_key_filename}: {e}')
        return 1

    crypto_sign_SECRETKEYBYTES = libsodium.crypto_sign_secretkeybytes()

    if len(secret_key) != crypto_sign_SECRETKEYBYTES:
        print('error: secret key has wrong size')
        return 1

    state = CryptoSignState()

    if libsodium.crypto_sign_init(ctypes.byref(state)) < 0:
        print('error: crypto_sign_init failed')
        return 1

    try:
        with open(args.input_filename, 'rb') as f:
            input_data = bytearray(f.read())
    except Exception as e:
        print(f'error: could not read input {args.input_filename}: {e}')
        return 1

    if libsodium.crypto_sign_update(ctypes.byref(state), bytes(input_data), len(input_data)) < 0:
        print('error: crypto_sign_update failed')
        return 1

    crypto_sign_BYTES = libsodium.crypto_sign_bytes()
    assert(crypto_sign_BYTES == 64)

    signature_buffer = ctypes.create_string_buffer(crypto_sign_BYTES)

    if libsodium.crypto_sign_final_create(ctypes.byref(state), signature_buffer, 0, secret_key) < 0:
        print('error: crypto_sign_final_create failed')
        return 1

    firmware_info_offset = 0xd000 - 0x1000
    input_data[firmware_info_offset - crypto_sign_BYTES:firmware_info_offset] = signature_buffer.raw

    try:
        with open(args.output_filename, 'wb') as f:
            f.write(input_data)
    except Exception as e:
        print(f'error: could not write output {args.output_filename}: {e}')
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
