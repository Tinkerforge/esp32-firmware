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

    args = parser.parse_args()
    directory = os.path.dirname(__file__)
    public_key_filename = os.path.relpath(os.path.join(directory, 'sodium_public_key.bin'))

    try:
        with open(public_key_filename, 'rb') as f:
            public_key = f.read()
    except Exception as e:
        print(f'error: could not read public key {public_key_filename}: {e}')
        return 1

    crypto_sign_PUBLICKEYBYTES = libsodium.crypto_sign_publickeybytes()

    if len(public_key) != crypto_sign_PUBLICKEYBYTES:
        print('error: public key has wrong size')
        return 1

    try:
        with open(args.input_filename, 'rb') as f:
            input_data = bytearray(f.read())
    except Exception as e:
        print(f'error: could not read input {args.input_filename}: {e}')
        return

    crypto_sign_BYTES = libsodium.crypto_sign_bytes()
    assert(crypto_sign_BYTES == 64)

    firmware_info_offset = 0xd000 - 0x1000
    signature = bytes(input_data[firmware_info_offset - crypto_sign_BYTES:firmware_info_offset])
    input_data[firmware_info_offset - crypto_sign_BYTES:firmware_info_offset] = bytes([0xff] * crypto_sign_BYTES)

    state = CryptoSignState()

    if libsodium.crypto_sign_init(ctypes.byref(state)) < 0:
        print('error: crypto_sign_init failed')
        return 1

    if libsodium.crypto_sign_update(ctypes.byref(state), bytes(input_data), len(input_data)) < 0:
        print('error: crypto_sign_update failed')
        return 1

    if libsodium.crypto_sign_final_verify(ctypes.byref(state), signature, public_key) < 0:
        print('error: crypto_sign_final_verify failed')
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
