import os
import sys
import io
import re
import hashlib
from zipfile import ZipFile
import binascii
import struct
import mimetypes
from base64 import b64encode
from collections import namedtuple
import functools
import json
import collections

NameFlavors = namedtuple('NameFlavors', 'space lower camel headless under upper dash camel_abbrv lower_no_space camel_constant_safe')

VERBOSE = False

def log(*args, **kwargs):
    if VERBOSE:
        print(*args, **kwargs)

class FlavoredName:
    def __init__(self, name):
        self.words = name.split(' ')
        self.cache = {}

    def get(self, skip=0, suffix=''):
        key = str(skip) + ',' + suffix

        try:
            return self.cache[key]
        except KeyError:
            if skip < 0:
                words = self.words[:skip]
            else:
                words = self.words[skip:]

            words[-1] += suffix

            self.cache[key] = NameFlavors(' '.join(words), # space
                                          ' '.join(words).lower(), # lower
                                          ''.join(words), # camel
                                          ''.join([words[0].lower()] + words[1:]), # headless
                                          '_'.join(words).lower(), # under
                                          '_'.join(words).upper(), # upper
                                          '-'.join(words).lower(), # dash
                                          ''.join([word.capitalize() for word in words]),  # camel_abbrv; like camel, but produces GetSpiTfp... instead of GetSPITFP...
                                          ''.join(words).lower(),
                                          # camel_constant_safe; inserts '_' between digit-words to disambiguate between 1,1ms and 11ms
                                          functools.reduce(lambda l, r: l + '_' + r if (l[-1].isdigit() and r[0].isdigit()) else l + r, words))

            return self.cache[key]

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

    # calculate new digest
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
        f.write('// WARNING: This file is generated by util.py, probably from prepare.py in this directory\n\n')

        if 'int' in var_type and var_type.endswith('_t'):
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
        f.write(f'// WARNING: This file is generated by util.py, probably from prepare.py in this directory\n\n')
        f.write('#pragma once\n\n')

        if 'int' in var_type and var_type.endswith('_t'):
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
        log('Embedded {0} is up-to-date'.format(var_name))
    else:
        if not os.path.exists(cpp_path) or not os.path.exists(h_path):
            reason = 'embedded file missing'

        print('Embedding {0} ({1})'.format(var_name, reason))

        remove_digest(dst_dir, var_name)
        embed_data_internal(data_filter(data), cpp_path, h_path, var_name, var_type)
        store_digest(new_digest, dst_dir, var_name)

def patch_beta_firmware(data, beta_version):
    data = bytearray(data)
    data[-10] = 200 + beta_version
    new_checksum = struct.pack('<I', binascii.crc32(data[:-4]) & 0xFFFFFFFF)
    data = data[:-4] + new_checksum
    return data

def embed_bricklet_firmware_bin(env=None):
    firmwares = [x for x in os.listdir('.') if x.endswith('.zbin') and x.startswith('bricklet_')]

    if len(firmwares) > 1:
        print('More than one firmware found in', os.getcwd())
        sys.exit(-1)

    if len(firmwares) == 0:
        print('No firmware found in', os.getcwd())
        sys.exit(-1)

    firmware = firmwares[0]
    m = re.fullmatch(r'bricklet_(.*)_firmware_\d+_\d+_\d+(?:_beta(\d+))?.zbin', firmware)

    if m == None:
        print('Firmware {} did not match naming schema'.format(firmware))
        sys.exit(-1)

    firmware_name = m.group(1)

    with ZipFile(firmware) as zf:
        with zf.open('{}-bricklet-firmware.bin'.format(firmware_name.replace('_', '-')), 'r') as f:
            fw = f.read()

    beta_version = m.group(2)
    if beta_version is not None:
        fw = patch_beta_firmware(fw, int(beta_version))

    embed_data_with_digest(fw, '.', firmware_name + '_bricklet_firmware_bin', 'uint8_t', env=env)

def gzip_compress(data):
    from shutil import which
    sevenz_path = which('7z') or which('7za')
    if sevenz_path:
        from subprocess import run
        result = run([sevenz_path, 'a', '-tgzip', '-mx=9', '-mfb=258', '-mpass=5', '-an', '-si', '-so'], input=data, capture_output=True)
        if result.returncode == 0:
            return result.stdout

    print("Using gzip! Install 7z or 7za to reduce the web interface size by ~ 5 %")
    from gzip import compress
    return compress(data)

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
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    placeholders = [x.strip() for x in re.findall(r'__\(([^\)]*)', content)]
    placeholders_unchecked = [x.strip() for x in re.findall(r'translate_unchecked\(([^\)]*)', content)]

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

def file_to_data_url(path):
    with open(path, 'rb') as f:
        data = b64encode(f.read()).decode('ascii')

    mimetypes.init()
    mtype, encoding = mimetypes.guess_type(path)
    if mtype is None:
        print("Failed to guess mimetype for", path)
        sys.exit(1)

    return "data:{};base64,{}".format(mtype, data)

def file_to_embedded_ts(path):
    data_url = file_to_data_url(path)
    basename = os.path.basename(path).replace(".", "_")
    path = path.replace(os.path.basename(path), basename)
    with open(path + ".embedded.ts", 'w', encoding='utf-8') as f:
        f.write('export let {} = "{}";'.format(basename, data_url))

FrontendPlugin = collections.namedtuple('FrontendPlugin', 'module_name import_name interface_names')

def find_frontend_plugins(host_module_name, plugin_name):
    host_module_name = FlavoredName(host_module_name).get()
    plugin_name = FlavoredName(plugin_name).get()

    project_dir = os.getenv('PLATFORMIO_PROJECT_DIR')

    if project_dir == None:
        print('$PLATFORMIO_PROJECT_DIR not set')
        sys.exit(-1)

    metadata_json = os.getenv('PLATFORMIO_METADATA')

    if metadata_json == None:
        print('$PLATFORMIO_METADATA not set')
        sys.exit(-1)

    metadata = json.loads(metadata_json)
    plugins = []
    plugin_file_base = 'plugin_' + host_module_name.under + '_' + plugin_name.under
    plugin_file_names = [plugin_file_base + '.ts', plugin_file_base + '.tsx']

    for module_name in metadata['frontend_modules']:
        module_path = os.path.join(project_dir, 'web', 'src', 'modules', module_name)

        for file_name in os.listdir(module_path):
            if file_name not in plugin_file_names:
                continue

            file_path = os.path.join(module_path, file_name)
            interface_names = []
            interface_re = re.compile(r'^export\s+(?:interface|type)\s+([A-Za-z0-9_]+{0}{1})[^A-Za-z0-9_]*$'.format(host_module_name.camel, plugin_name.camel))

            with open(file_path, 'r', encoding='utf-8') as f:
                for line in f:
                    m = interface_re.match(line.strip())

                    if m != None:
                        interface_names.append(m.group(1))

            plugins.append(FrontendPlugin(module_name, plugin_file_base, interface_names))

    return plugins
