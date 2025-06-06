import sys
import os
import json
import io
import configparser
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

directory = os.path.normpath(os.path.dirname(__file__))


def make_signature_path(path):
    if os.path.isabs(path):
        return path

    return os.path.join('.', os.path.relpath(os.path.join(directory, '../../../signature', path)))


def make_signature_keys_path(path):
    if os.path.isabs(path):
        return path

    return os.path.join('.', os.path.relpath(os.path.join(directory, '../../../signature/keys', path)))


metadata_json = os.getenv('PLATFORMIO_METADATA')

if metadata_json == None:
    print('$PLATFORMIO_METADATA not set')
    sys.exit(-1)

metadata = json.loads(metadata_json)

if len(metadata['signature_preset']) == 0:
    publisher_literal = ''
    sodium_public_key = b''
else:
    config = configparser.ConfigParser()
    config.read(make_signature_path('config.ini'))

    try:
        preset = dict(config['preset:' + metadata['signature_preset']])
    except KeyError:
        print(f"Preset {metadata['signature_preset']} is unknown, maybe the signature data is outdated")
        sys.exit(-1)

    if 'extends' in preset:
        extends = preset['extends']

        for key in config[extends]:
            if key not in preset:
                preset[key] = config.get(extends, key)

    publisher = preset['publisher']
    publisher_bytes = publisher.encode('utf-8')

    if len(publisher_bytes) < 1 or len(publisher_bytes) > 63:
        print(f'Signature publisher UTF-8 length is out of range: {repr(publisher)}')
        sys.exit(-1)

    print(f'Embedding sodium public key for {repr(publisher)}')

    publisher_literal = ''

    for c in publisher:
        n = ord(c)

        if n <= 0x7f:
            publisher_literal += json.dumps(c)[1:-1]
        elif n <= 0xffff:
            publisher_literal += f'\\u{n:04x}'
        else:
            publisher_literal += f'\\U{n:08x}'

    sodium_public_key_path = make_signature_keys_path(preset['sodium_public_key_path'])

    with open(sodium_public_key_path, 'r', encoding='utf-8') as f:
        sodium_public_key_json = json.loads(f.read())

    sodium_public_key = bytes.fromhex(sodium_public_key_json['sodium_public_key'])

h_path = 'signature_verify.embedded.h'

try:
    os.remove(h_path)
except FileNotFoundError:
    pass

with open(h_path + '.tmp', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated\n\n')
    f.write('extern const char *signature_publisher;\n\n')
    f.write('extern const unsigned char signature_sodium_public_key_data[];\n\n')
    f.write(f'#define signature_sodium_public_key_length {len(sodium_public_key)}\n\n')

os.replace(h_path + '.tmp', h_path)

cpp_path = 'signature_verify.embedded.cpp'

try:
    os.remove(cpp_path)
except FileNotFoundError:
    pass

with open(cpp_path + '.tmp', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated\n\n')
    f.write(f'const char *signature_publisher = "{publisher_literal}";\n\n')
    f.write('extern const unsigned char signature_sodium_public_key_data[] = {\n')

    sodium_public_key_io = io.BytesIO(sodium_public_key)
    b = sodium_public_key_io.read(12)

    while len(b) != 0:
        # read first to prevent trailing , after last byte
        next_b = sodium_public_key_io.read(12)
        f.write('    ' + ', '.join(['0x{:02x}'.format(x) for x in b]) + (',\n' if len(next_b) != 0 else '\n'))
        b = next_b

    f.write('};\n')

os.replace(cpp_path + '.tmp', cpp_path)
