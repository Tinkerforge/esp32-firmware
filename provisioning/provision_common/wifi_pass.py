#!/bin/python3

import ctypes
import ctypes.util
import os
import sys
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'provisioning')

from provisioning.tinkerforge.ip_connection import base58encode
from provisioning.provision_common.zbase32 import zbase32encode

directory = os.path.normpath(os.path.dirname(__file__))

def make_path(path):
    if os.path.isabs(path):
        return path

    return os.path.join('.', os.path.relpath(os.path.join(directory, path)))


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


def calculate_passphrase(efuses_block3: bytes, factory_mac: bytes) -> str:
    libsodium = load_libsodium()
    master_key = bytearray(efuses_block3)

    # Fill reserved two bytes with two bytes from the factory MAC address.
    master_key[0] = factory_mac[3]
    master_key[1] = factory_mac[4]

    # Fill unusable MAC version byte with the last byte from the factory MAC address.
    master_key[23] = factory_mac[5]

    wifi_subkey = bytes(32)
    wifi_context = "wifipass".encode('ascii')

    libsodium.crypto_kdf_blake2b_derive_from_key(wifi_subkey, 32, 0, wifi_context, bytes(master_key))

    passphrase = ''

    for i in range(4):
        block_int = int.from_bytes(wifi_subkey[i * 8:i * 8 + 8], 'little') % (58 * 58 * 58 * 58)
        passphrase += base58encode(block_int).rjust(4, '1') + '-'

    return passphrase[0:-1]


def main():
    # reserved bytes: 0 1 23        v v                                           v
    efuses_block3 = bytes.fromhex("000057019061363267916c66747d54203ff746c611d2930050b4729052cc0200")
    factory_mac   = bytes.fromhex("a8032a316618")

    uid = zbase32encode(int.from_bytes(efuses_block3[28:32], 'little'))
    passphrase = calculate_passphrase(efuses_block3, factory_mac)

    print(f"UID:  {uid}")  # expected "fun1"
    print(f"Pass: {passphrase}")  # expected "1SH4-6BbE-xN56-6Ygf"

if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print(f'Error: {e}')
        sys.exit(1)
