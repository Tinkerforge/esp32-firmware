import argparse
import ctypes
import ctypes.util
import os
import subprocess
import sys
import tempfile


directory = os.path.normpath(os.path.dirname(__file__))


def make_path(path) -> str:
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


def load_libmbedtls():
    libmbedtls_path = ctypes.util.find_library('mbedtls')

    if libmbedtls_path != None:
        libmbedtls = ctypes.cdll.LoadLibrary(libmbedtls_path)
    else:
        for extension in ['so', 'dll', 'dylib']:
            try:
                libmbedtls = ctypes.cdll.LoadLibrary(make_path(f'libmbedtls.{extension}'))
            except:
                continue

            break
        else:
            raise Exception('Cannot find libmbedtls library')

    return libmbedtls


libsodium = load_libsodium()
libmbedtls = load_libmbedtls()


class BlakeCtrCtx(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ('key',               ctypes.c_uint8 * 32),
        ('subkey_id_counter', ctypes.c_uint32),
        ('call_counter',      ctypes.c_uint32),
    ]


class mbedtls_pk_context(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ('pk_info', ctypes.c_void_p),
        ('pk_ctx', ctypes.c_char_p),
        ('pad', ctypes.c_uint8 * 128), # Struct might have additional data that we don't care about.
    ]


def derive_subkey(libsodium, efuses_block3: bytes, factory_mac: bytes, context: str) -> bytes:
    master_key = bytearray(efuses_block3)

    # Fill reserved two bytes with two bytes from the factory MAC address.
    master_key[0] = factory_mac[3]
    master_key[1] = factory_mac[4]

    # Fill unusable MAC version byte with the last byte from the factory MAC address.
    master_key[23] = factory_mac[5]

    subkey = bytes(32)
    context_bytes = context.encode('ascii')

    libsodium.crypto_kdf_blake2b_derive_from_key(subkey, 32, 0, context_bytes, bytes(master_key))

    return subkey


def derive_encryption_key(libsodium, efuses_block3: bytes, factory_mac: bytes) -> bytes:
    return derive_subkey(libsodium, efuses_block3, factory_mac, 'flashenc')


def derive_rsa_prng_key(libsodium, efuses_block3: bytes, factory_mac: bytes) -> bytes:
    return derive_subkey(libsodium, efuses_block3, factory_mac, 'rsasgkey')


CTR_DRBG_FUNC = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t)

@CTR_DRBG_FUNC
def blake_ctr_drbg_for_rsa_key_func(ctx_ptr: int, output_ptr: int, len: int) -> int:
    # Can't raise any exceptions in C-style callbacks. Must return non-zero on error.
    try:
        global libsodium

        ctx    = ctypes.cast(ctypes.c_void_p(ctx_ptr   ), ctypes.POINTER(BlakeCtrCtx))
        output = ctypes.cast(ctypes.c_void_p(output_ptr), ctypes.POINTER(ctypes.c_uint8 * len)).contents

        context_bytes: bytes = 'rsa_drbg'.encode('ascii')

        if len % 64 != 0:
            print('Requested amount of randomness not a multiple of 64')
            return -1

        offset: int = 0

        while offset < len:
            rnd = bytes(64)

            err: int = libsodium.crypto_kdf_blake2b_derive_from_key(rnd, 64, ctx.contents.subkey_id_counter + 0, context_bytes, ctx.contents.key)

            if err != 0:
                print(f"crypto_kdf_blake2b_derive_from_key failed: {err}")
                return -1

            output[offset:offset+64] = rnd

            ctx.contents.subkey_id_counter += 1
            offset += 64

        return 0
    except BaseException as e:
        print(f"Exception in blake_ctr_drbg_for_rsa_key_func: {e}, {type(e)}")
        return -1


def generate_rsa_pem(libsodium, efuses_block3: bytes, factory_mac: bytes) -> str:
    blake_ctr_ctx = BlakeCtrCtx()
    libsodium.sodium_memzero(ctypes.byref(blake_ctr_ctx), ctypes.sizeof(blake_ctr_ctx))

    blake_ctr_ctx.key[:] = derive_rsa_prng_key(libsodium, efuses_block3, factory_mac)

    rsa_key_ctx = bytes(280) # sizeof(mbedtls_rsa_context) is 120 on Xtensa but is 280 on amd64.

    libmbedtls.mbedtls_rsa_init.argtypes = [ctypes.c_void_p]
    libmbedtls.mbedtls_rsa_init(rsa_key_ctx)

    KEY_SIZE: int = 3072
    EXPONENT: int = 65537

    libmbedtls.mbedtls_rsa_gen_key.argtypes = [ctypes.c_void_p, CTR_DRBG_FUNC, ctypes.POINTER(BlakeCtrCtx), ctypes.c_uint, ctypes.c_int]
    libmbedtls.mbedtls_rsa_gen_key.restype = ctypes.c_int

    err: int = libmbedtls.mbedtls_rsa_gen_key(rsa_key_ctx, blake_ctr_drbg_for_rsa_key_func, blake_ctr_ctx, KEY_SIZE, EXPONENT)

    if err != 0:
        raise Exception(f"mbedtls_rsa_gen_key failed: {err}")

    pk_ctx = mbedtls_pk_context()
    pk_ctx.pk_ctx = ctypes.c_char_p(rsa_key_ctx)

    libmbedtls.mbedtls_pk_info_from_type.restype = ctypes.c_void_p
    pk_ctx.pk_info = libmbedtls.mbedtls_pk_info_from_type(1) # MBEDTLS_PK_RSA is 1

    key_pem_buffer = bytes(3072)
    err = libmbedtls.mbedtls_pk_write_key_pem(ctypes.byref(pk_ctx), key_pem_buffer, len(key_pem_buffer))

    libmbedtls.mbedtls_rsa_free(rsa_key_ctx)

    if err != 0:
        raise Exception(f"mbedtls_pk_write_key_pem failed: {err}")

    return key_pem_buffer[:key_pem_buffer.find(0)].decode('ascii')


def sign(infile: str, outfile: str, efuses_block3: bytes, factory_mac: bytes) -> None:
    key_pem: str = generate_rsa_pem(libsodium, efuses_block3, factory_mac)

    with tempfile.NamedTemporaryFile(prefix='sbv2-', suffix='.pem', delete_on_close=False) as key_file:
        key_file.write(key_pem.encode('ascii'))
        key_file.flush()

        sign_cmd = [
            'uv',
            'run',
            'pio',
            'pkg',
            'exec',
            '-p', 'tool-esptoolpy',
            '--',
            'espsecure',
            'sign-data',
            infile,
            '-o', outfile,
            '-v', '2',
            '-k', key_file.name,
        ]

        try:
            subprocess.check_call(sign_cmd)
        except subprocess.CalledProcessError as e:
            print('espsecure call failed', file=sys.stderr)
            sys.exit(1)


def encrypt(infile: str, outfile: str, address: int, efuses_block3: bytes, factory_mac: bytes) -> None:
    encryption_key: bytes = derive_encryption_key(libsodium, efuses_block3, factory_mac)[::-1]

    with tempfile.NamedTemporaryFile(prefix='sbv2-', suffix='.bin', delete_on_close=False) as key_file:
        key_file.write(encryption_key)
        key_file.flush()

        encrypt_cmd = [
            'uv',
            'run',
            'pio',
            'pkg',
            'exec',
            '-p', 'tool-esptoolpy',
            '--',
            'espsecure',
            'encrypt-flash-data',
            infile,
            '-o', outfile,
            '-a', str(address),
            '-k', key_file.name,
        ]

        try:
            subprocess.check_call(encrypt_cmd)
        except subprocess.CalledProcessError as e:
            print('espsecure call failed', file=sys.stderr)
            sys.exit(1)


def sign_str(infile: str, outfile: str, efuses_block3: str, factory_mac: str) -> None:
    _efuses_block3 = bytes.fromhex(efuses_block3)
    _factory_mac   = bytes.fromhex(factory_mac.replace(":", ""))

    sign(infile, outfile, _efuses_block3, _factory_mac)


def encrypt_str(infile: str, outfile: str, address: str, efuses_block3: str, factory_mac: str) -> None:
    _efuses_block3 = bytes.fromhex(efuses_block3)
    _factory_mac   = bytes.fromhex(factory_mac.replace(":", ""))
    _address       = int(address, 0)

    encrypt(infile, outfile, _address, _efuses_block3, _factory_mac)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='secure_boot', description='Generate Secure Boot v2 signatures and encrypt data')
    parser.add_argument('infile')
    parser.add_argument('outfile')
    parser.add_argument('efuses_block3')
    parser.add_argument('factory_mac')

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("-s", "--sign", action="store_true")
    group.add_argument("-e", "--encryption_address")

    args = parser.parse_args(sys.argv[1:])

    if args.sign:
        sign_str(args.infile, args.outfile, args.efuses_block3, args.factory_mac)
    elif args.encryption_address is not None:
        encrypt_str(args.infile, args.outfile, args.encryption_address, args.efuses_block3, args.factory_mac)
    else:
        raise Exception("Invalid mode")
