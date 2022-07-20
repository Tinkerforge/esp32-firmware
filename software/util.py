import os
import sys
import io
import re
import hashlib
from zipfile import ZipFile
import json
import re

def get_digest_paths(dst_dir, var_name, env=None):
    if env is not None:
        project_dir = env.subst('$PROJECT_DIR')
    else:
        project_dir = os.getenv('PLATFORMIO_PROJECT_DIR')

        if project_dir == None:
            print('$PLATFORMIO_PROJECT_DIR not set')
            sys.exit(-1)

    if env is not None:
        build_dir = env.subst('$BUILD_DIR')
    else:
        build_dir = os.getenv('PLATFORMIO_BUILD_DIR')

        if build_dir == None:
            print('$PLATFORMIO_BUILD_DIR not set')
            sys.exit(-1)

    digest1_path = os.path.realpath(os.path.join(build_dir, os.path.relpath(os.path.join(os.getcwd(), dst_dir), project_dir).replace('\\', '/').replace('/', ',') + ',' + var_name + '.digest'))
    digest2_path = os.path.realpath(os.path.join(os.getcwd(), dst_dir, var_name + '.digest'))

    return digest1_path, digest2_path

def check_digest(src_paths, src_datas, dst_dir, var_name, env=None):
    # read old digests
    digest1_path, digest2_path = get_digest_paths(dst_dir, var_name, env=env)

    try:
        with open(digest1_path, 'r', encoding='utf-8') as f:
            old_digest1 = f.read().strip()
    except FileNotFoundError:
        old_digest1 = None

    try:
        with open(digest2_path, 'r', encoding='utf-8') as f:
            old_digest2 = f.read().strip()
    except FileNotFoundError:
        old_digest2 = None

    # caclulate new digest
    h = hashlib.sha256()

    with open(__file__, 'rb') as f:
        h.update(f.read())

    for src_path in src_paths:
        with open(src_path, 'rb') as f:
            h.update(f.read())

    for src_data in src_datas:
        h.update(src_data)

    new_digest = h.hexdigest()

    # check digests
    needs_update = old_digest1 != new_digest or old_digest2 != new_digest

    if not needs_update:
        reason = None
    else:
        if old_digest1 == None or old_digest2 == None:
            reason = 'digest file missing'
        elif old_digest1 != old_digest2:
            reason = 'digest out-of-sync'
        elif old_digest1 != new_digest:
            reason = 'digest out-of-date'
        else:
            reason = 'unknown'

    return needs_update, reason, new_digest

def remove_digest(dst_dir, var_name, env=None):
    for digest_path in get_digest_paths(dst_dir, var_name, env=env):
        try:
            os.remove(digest_path)
        except FileNotFoundError:
            pass

def store_digest(digest, dst_dir, var_name, env=None):
    for digest_path in get_digest_paths(dst_dir, var_name, env=env):
        with open(digest_path + '.tmp', 'w', encoding='utf-8') as f:
            f.write(digest)

        os.replace(digest_path + '.tmp', digest_path)

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

def embed_data_with_digest(data, dst_dir, var_name, var_type, data_filter=lambda data: data, env=None):
    needs_update, reason, new_digest = check_digest([], [data], dst_dir, var_name, env=env)
    cpp_path = os.path.join(dst_dir, var_name + '.embedded.cpp')
    h_path = os.path.join(dst_dir, var_name + '.embedded.h')

    if not needs_update and os.path.exists(cpp_path) and os.path.exists(h_path):
        print('Embedded {0} is up-to-date'.format(var_name))
    else:
        if not os.path.exists(cpp_path) or not os.path.exists(h_path):
            reason = 'embedded file missing'

        print('Embedding {0} ({1})'.format(var_name, reason))

        remove_digest(dst_dir, var_name)
        embed_data_internal(data_filter(data), cpp_path, h_path, var_name, var_type)
        store_digest(new_digest, dst_dir, var_name)

def embed_bricklet_firmware_bin(env=None):
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
            embed_data_with_digest(f.read(), '.', firmware_name + '_bricklet_firmware_bin', 'uint8_t', env=env)

def merge(left, right, path=[]):
    for key in right:
        if key in left:
            if isinstance(left[key], dict) and isinstance(right[key], dict):
                merge(left[key], right[key], path + [str(key)])
            else:
                print("Found ambiguous placeholder ", '.'.join(path + [key]))
        else:
            left[key] = right[key]
    return left

def parse_ts_file(path, name, used_placeholders, template_literals):
    with open(path, 'r') as f:
        content = f.read()

    placeholders = re.findall('__\(([^\)]*)', content)
    placeholders_unchecked = re.findall('translate_unchecked\(([^\)]*)', content)

    template_literal_keys = [x for x in placeholders_unchecked if x[0] == '`' and x[-1] == '`' and '${' in x and '}' in x]
    placeholders = [x for x in placeholders if x not in template_literal_keys]

    template_literals.update({x[1:-1]: [] for x in template_literal_keys})

    incorrect_placeholders = [x for x in placeholders if not x[0] == '"' or not x[-1] == '"']
    if len(incorrect_placeholders) != 0:
        print("Found incorrectly quoted placeholders. Use \"\"!", incorrect_placeholders)

    used_placeholders += [x[1:-1] for x in placeholders]

def parse_ts_files(files):
    used_placeholders = []
    template_literals = {}

    for f in files:
        parse_ts_file(f, os.path.basename(f), used_placeholders, template_literals)

    return used_placeholders, template_literals

def get_nested_keys(d, path=""):
    r = []
    for k, v in d.items():
        if isinstance(v, dict) and len(v) > 0:
            r += get_nested_keys(v, path + "." + k if path != "" else k)
        elif isinstance(v, str):
            r.append(path + "." + k)
    return r

colors = {"off": "\x1b[00m",
          "blue": "\x1b[34m",
          "cyan": "\x1b[36m",
          "green": "\x1b[32m",
          "red": "\x1b[31m",
          "gray": "\x1b[90m"}

def red(s):
    return colors["red"]+s+colors["off"]

def green(s):
    return colors["green"]+s+colors["off"]

def gray(s):
    return colors['gray']+s+colors["off"]
