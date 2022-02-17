import os
import io
import re
from zipfile import ZipFile

def embed_file(src_file, dst_name, data_type):
    with open(dst_name + '.embedded.cpp', 'w') as f:
        f.write('// WARNING: This file is generated\n\n')
        f.write('#include <stddef.h>\n')
        f.write('#include <stdint.h>\n\n')
        f.write('extern const {0} {1}_data[] = {{\n'.format(data_type, dst_name))

        written = 0
        b = src_file.read(12)

        while len(b) != 0:
            # read first to prevent trailing , after last byte
            next_b = src_file.read(12)
            f.write('    ' + ', '.join(['0x{:02x}'.format(x) for x in b]) + (',\n' if len(next_b) != 0 else '\n'))
            written += len(b)
            b = next_b

        f.write('};\n\n')
        f.write('extern const size_t {0}_length = {1};\n'.format(dst_name, written))

    with open(dst_name + '.embedded.h', 'w') as f:
        f.write('// WARNING: This file is generated\n\n')
        f.write('#include <stddef.h>\n')
        f.write('#include <stdint.h>\n\n')
        f.write('extern const {0} {1}_data[];\n'.format(data_type, dst_name))
        f.write('extern const size_t {0}_length;\n'.format(dst_name))

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
        with zf.open('{}-bricklet-firmware.bin'.format(firmware_name.replace('_', '-')), 'r') as src_file:
            embed_file(src_file, firmware_name + '_bricklet_firmware_bin', 'uint8_t')
