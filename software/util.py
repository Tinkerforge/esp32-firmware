import os
import io
import re
import hashlib
from zipfile import ZipFile

def embed_data_internal(data, cpp_path, h_path, var_name, var_type):
    try:
        os.remove(cpp_path)
    except FileNotFoundError:
        pass

    try:
        os.remove(h_path)
    except FileNotFoundError:
        pass

    with open(cpp_path + '.tmp', 'w', encoding='utf-8') as f:
        f.write('// WARNING: This file is generated\n\n')
        f.write('#include <stdint.h>\n\n')
        f.write('extern const {0} {1}_data[] = {{\n'.format(var_type, var_name))

        written = 0
        data_file = io.BytesIO(data)
        b = data_file.read(12)

        while len(b) != 0:
            # read first to prevent trailing , after last byte
            next_b = data_file.read(12)
            f.write('    ' + ', '.join(['0x{:02x}'.format(x) for x in b]) + (',\n' if len(next_b) != 0 else '\n'))
            written += len(b)
            b = next_b

        f.write('};\n')

    os.replace(cpp_path + '.tmp', cpp_path)

    with open(h_path + '.tmp', 'w', encoding='utf-8') as f:
        f.write('// WARNING: This file is generated\n\n')
        f.write('#include <stdint.h>\n\n')
        f.write('extern const {0} {1}_data[];\n\n'.format(var_type, var_name))
        f.write('#define {0}_length {1}\n'.format(var_name, written))

    os.replace(h_path + '.tmp', h_path)

def embed_data(data, dst_dir, var_name, var_type):
    cpp_path = os.path.join(dst_dir, var_name + '.embedded.cpp')
    h_path = os.path.join(dst_dir, var_name + '.embedded.h')

    embed_data_internal(data, cpp_path, h_path, var_name, var_type)

def embed_data_with_digest(data, dst_dir, var_name, var_type, data_filter=lambda data: data):
    project_dir = os.getenv('PLATFORMIO_PROJECT_DIR')

    if project_dir == None:
        print('$PLATFORMIO_PROJECT_DIR not set')
        sys.exit(-1)

    build_dir = os.getenv('PLATFORMIO_BUILD_DIR')

    if build_dir == None:
        print('$PLATFORMIO_BUILD_DIR not set')
        sys.exit(-1)

    digest_path = os.path.join(build_dir, os.path.relpath(os.getcwd(), project_dir).replace('\\', '/').replace('/', ',') + ',' + var_name + '.digest')
    cpp_path = os.path.join(dst_dir, var_name + '.embedded.cpp')
    h_path = os.path.join(dst_dir, var_name + '.embedded.h')

    try:
        with open(digest_path, 'r', encoding='utf-8') as f:
            old_digest = f.read().strip()
    except FileNotFoundError:
        old_digest = None

    with open(__file__, 'rb') as f:
        new_digest = hashlib.sha256(data + f.read()).hexdigest()

    if old_digest == new_digest and os.path.exists(cpp_path) and os.path.exists(h_path):
        print('Embedded {0} is up-to-date'.format(var_name))
        return

    if old_digest == None:
        reason = 'digest file missing'
    elif old_digest != new_digest:
        reason = 'digest mismatch'
    elif not os.path.exists(cpp_path) or not os.path.exists(h_path):
        reason = 'embedded file missing'
    else:
        reason = 'unknown'

    print('Embedding {0} ({1})'.format(var_name, reason))

    try:
        os.remove(digest_path)
    except FileNotFoundError:
        pass

    embed_data_internal(data_filter(data), cpp_path, h_path, var_name, var_type)

    with open(digest_path + '.tmp', 'w', encoding='utf-8') as f:
        f.write(new_digest)

    os.replace(digest_path + '.tmp', digest_path)

def embed_bricklet_firmware_bin():
    firmwares = [x for x in os.listdir('.') if x.endswith('.zbin') and x.startswith('bricklet_')]

    if len(firmwares) > 1:
        print('More than one firmware found in', os.getcwd())
        sys.exit(-1)

    if len(firmwares) == 0:
        print('No firmware found in', os.getcwd())
        sys.exit(-1)

    firmware = firmwares[0]
    m = re.fullmatch('bricklet_(.*)_firmware_\d+_\d+_\d+.zbin', firmware)

    if m == None:
        print('Firmware {} did not match naming schema'.format(firmware))
        sys.exit(-1)

    firmware_name = m.group(1)

    with ZipFile(firmware) as zf:
        with zf.open('{}-bricklet-firmware.bin'.format(firmware_name.replace('_', '-')), 'r') as f:
            embed_data_with_digest(f.read(), '.', firmware_name + '_bricklet_firmware_bin', 'uint8_t')
