import os
import sys
import importlib.util
import importlib.machinery
import json
import io

software_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))

def create_software_module():
    software_spec = importlib.util.spec_from_file_location('software', os.path.join(software_dir, '__init__.py'))
    software_module = importlib.util.module_from_spec(software_spec)

    software_spec.loader.exec_module(software_module)

    sys.modules['software'] = software_module

if 'software' not in sys.modules:
    create_software_module()

from software import util

try:
    with open('../../../signature_public_key_v1.json', 'r', encoding='utf-8') as f:
        signature_public_key_json = json.loads(f.read())

    signature_publisher = repr(signature_public_key_json['publisher'].encode('utf-8'))[2:-1].replace('"', '\\"')
    signature_public_key = bytes.fromhex(signature_public_key_json['public_key'])
except FileNotFoundError:
    signature_publisher = b''
    signature_public_key = b''

h_path = 'signature_public_key.embedded.h'

try:
    os.remove(h_path)
except FileNotFoundError:
    pass

with open(h_path + '.tmp', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated\n\n')
    f.write('extern const char *signature_publisher;\n\n')
    f.write('extern const unsigned char signature_public_key_data[];\n\n')
    f.write(f'#define signature_public_key_length {len(signature_public_key)}\n\n')

os.replace(h_path + '.tmp', h_path)

cpp_path = 'signature_public_key.embedded.cpp'

try:
    os.remove(cpp_path)
except FileNotFoundError:
    pass

with open(cpp_path + '.tmp', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated\n\n')
    f.write(f'const char *signature_publisher = "{signature_publisher}";\n\n')
    f.write('extern const unsigned char signature_public_key_data[] = {\n')

    public_key_io = io.BytesIO(signature_public_key)
    b = public_key_io.read(12)

    while len(b) != 0:
        # read first to prevent trailing , after last byte
        next_b = public_key_io.read(12)
        f.write('    ' + ', '.join(['0x{:02x}'.format(x) for x in b]) + (',\n' if len(next_b) != 0 else '\n'))
        b = next_b

    f.write('};\n')

os.replace(cpp_path + '.tmp', cpp_path)
