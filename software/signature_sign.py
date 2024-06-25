#!/usr/bin/python3 -u

import os
import sys
import ctypes
import ctypes.util
import argparse
import json
from zlib import crc32


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
    parser.add_argument('--force', action='store_true')
    parser.add_argument('input_filename')
    parser.add_argument('output_filename')

    args = parser.parse_args()

    if os.path.exists(args.output_filename):
        if args.force:
            try:
                os.remove(args.output_filename)
            except Exception as e:
                print(f'error: could not remove old output file {args.output_filename}: {e}')
                return 1
        else:
            print(f'error: output file {args.output_filename} already exists')
            return 1

    directory = os.path.dirname(__file__)
    secret_key_filename = os.path.relpath(os.path.join(directory, 'signature_secret_key_v1.json'))

    try:
        with open(secret_key_filename, 'r', encoding='utf-8') as f:
            secret_key_json = json.loads(f.read())
    except Exception as e:
        print(f'error: could not read secret key from {secret_key_filename}: {e}')
        return 1

    publisher = secret_key_json['publisher']

    if not isinstance(publisher, str):
        print('error: publisher is not a string')
        return 1

    publisher_bytes = publisher.encode('utf-8')

    if len(publisher_bytes) < 1 or len(publisher_bytes) > 63:
        print('error: publisher UTF-8 length is out of range')
        return 1

    secret_key_hex = secret_key_json['secret_key']

    if not isinstance(secret_key_hex, str):
        print('error: secret key is not a string')
        return 1

    try:
        secret_key = bytes.fromhex(secret_key_hex)
    except:
        print('error: secret key is malformed')
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
        print(f'error: could not read input from {args.input_filename}: {e}')
        return 1

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
        print('error: crypto_sign_update failed')
        return 1

    signature_buffer = ctypes.create_string_buffer(crypto_sign_BYTES)

    if libsodium.crypto_sign_final_create(ctypes.byref(state), signature_buffer, 0, secret_key) < 0:
        print('error: crypto_sign_final_create failed')
        return 1

    input_data[signature_info_offset + 72:signature_info_offset + 72 + crypto_sign_BYTES] = signature_buffer.raw

    try:
        with open(args.output_filename + '.tmp', 'wb') as f:
            f.write(input_data)
    except Exception as e:
        print(f'error: could not write output to {args.output_filename}.tmp: {e}')
        return 1

    try:
        os.replace(args.output_filename + '.tmp', args.output_filename)
    except Exception as e:
        print(f'error: could not rename output from {args.output_filename}.tmp to {args.output_filename}: {e}')
        return 1

    print(f'success: published by {repr(publisher)}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
