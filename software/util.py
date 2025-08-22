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
from dataclasses import dataclass, InitVar, field
from collections.abc import Callable

import tinkerforge_util as tfutil

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

def embed_data_internal(data, cpp_path, h_path, var_name, var_type, var_size_type):
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

        write_nl = False
        if ('int' in var_type and var_type.endswith('_t')) or ('int' in var_size_type and var_size_type.endswith('_t')):
            f.write('#include <stdint.h>\n')
            write_nl = True
        if var_type == 'size_t' or var_size_type == 'size_t':
            f.write('#include <stddef.h>\n')
            write_nl = True
        if var_type == 'ssize_t' or var_size_type == 'ssize_t':
            f.write('#include <sys/types.h>\n')
            write_nl = True

        if write_nl:
            f.write('\n')

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

        f.write('};\n\n')
        f.write('extern const {0} {1}_length = {2};\n'.format(var_size_type, var_name, written))

    os.replace(cpp_path + '.tmp', cpp_path)

    with open(h_path + '.tmp', 'w', encoding='utf-8') as f:
        f.write(f'// WARNING: This file is generated by util.py, probably from prepare.py in this directory\n\n')
        f.write('#pragma once\n\n')

        write_nl = False
        if ('int' in var_type and var_type.endswith('_t')) or ('int' in var_size_type and var_size_type.endswith('_t')):
            f.write('#include <stdint.h>\n')
            write_nl = True
        if var_type == 'size_t' or var_size_type == 'size_t':
            f.write('#include <stddef.h>\n')
            write_nl = True
        if var_type == 'ssize_t' or var_size_type == 'ssize_t':
            f.write('#include <sys/types.h>\n')
            write_nl = True

        if write_nl:
            f.write('\n')

        f.write('extern const {0} {1}_data[];\n'.format(var_type, var_name))
        f.write('extern const {0} {1}_length;\n'.format(var_size_type, var_name))

    os.replace(h_path + '.tmp', h_path)

def embed_data(data, dst_dir, var_name, var_type, var_size_type='size_t'):
    cpp_path = os.path.join(dst_dir, var_name + '.embedded.cpp')
    h_path = os.path.join(dst_dir, var_name + '.embedded.h')

    embed_data_internal(data, cpp_path, h_path, var_name, var_type, var_size_type)

def embed_data_with_digest(data, dst_dir, var_name, var_type, var_size_type='size_t', data_filter=lambda data: data, env=None):
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
        embed_data_internal(data_filter(data), cpp_path, h_path, var_name, var_type, var_size_type)
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

    embed_data_with_digest(fw, '.', firmware_name + '_bricklet_firmware_bin', 'uint8_t', 'size_t', env=env)

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

        if not os.path.isdir(module_path):
            print(f"Front-end module {module_name} from custom_frontend_modules does not exist", file=sys.stderr)
            sys.exit(-1)

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

#region enum generator
@dataclass
class EnumValue:
    name: NameFlavors
    number: int
    comment: str

def generate_enum(source_file_name: str,
                  backend_module: NameFlavors,
                  enum_name: NameFlavors,
                  underlying_type: str,
                  values: list[EnumValue],
                  comment: str | None):
    mod_path = os.path.join('src', 'modules', backend_module.under)

    enum_values = [f'    {v.name.camel} = {v.number},{v.comment}\n' for v in values]
    enum_cases= [f'    case {enum_name.camel}::{v.name.camel}: return "{v.name.space}";\n' for v in values]
    value_number_min_name = min(values, key=lambda v: v.number).name
    value_number_max_name = max(values, key=lambda v: v.number).name

    enum_raw_values = {e.name.space: e.number for e in values}

    try:
        with open(os.path.join(mod_path, source_file_name + '.previous'), 'r', encoding='utf-8') as f:
            enum_previous_raw_values = json.loads(f.read())
    except FileNotFoundError:
        enum_previous_raw_values = None

    if enum_previous_raw_values != None:
        for value_name, value_number in enum_raw_values.items():
            if value_name in enum_previous_raw_values and enum_previous_raw_values[value_name] != value_number:
                print(f'Error: Invalid change to value "{value_name}" in enum file "{source_file_name}" in backend {mod_path}')
                sys.exit(1)

    tfutil.write_file_if_different(os.path.join(mod_path, source_file_name + '.previous'), json.dumps(enum_raw_values))

    enum_header = ""
    enum_header += f'// WARNING: This file is generated from "{source_file_name}" by pio_hooks.py\n\n'
    enum_header += '#include <stdint.h>\n\n'
    enum_header += '#pragma once\n\n'
    if comment:
        enum_header += comment
    enum_header += f'enum class {enum_name.camel} : {underlying_type} {{\n'
    enum_header += ''.join(enum_values)
    enum_header += '\n'
    enum_header += f'    _min = {value_number_min_name.camel},\n'
    enum_header += f'    _max = {value_number_max_name.camel},\n'
    enum_header += '};\n\n'
    enum_header += f'#define {enum_name.upper}_COUNT {len(values)}\n\n'
    enum_header += f'const char *get_{enum_name.under}_name({enum_name.camel} value);\n'

    tfutil.write_file_if_different(os.path.join(mod_path, enum_name.under + '.enum.h'), enum_header)

    enum_source = ""
    enum_source += f'// WARNING: This file is generated from "{source_file_name}" by pio_hooks.py\n\n'
    enum_source += f'#include "{enum_name.under}.enum.h"\n\n'
    enum_source += f'const char *get_{enum_name.under}_name({enum_name.camel} value)\n'
    enum_source += '{\n'
    enum_source += '    switch (value) {\n'
    enum_source += ''.join(enum_cases)
    enum_source += '    default: return "Unknown";\n'
    enum_source += '    }\n'
    enum_source += '}\n'

    tfutil.write_file_if_different(os.path.join(mod_path, enum_name.under + '.enum.cpp'), enum_source)

    frontend_source = ""
    frontend_source += f'// WARNING: This file is generated from "{source_file_name}" by pio_hooks.py\n\n'
    frontend_source += f'export const enum {enum_name.camel} {{\n'
    frontend_source += ''.join(enum_values)
    frontend_source += '\n'
    frontend_source += f'    _min = {value_number_min_name.camel},\n'
    frontend_source += f'    _max = {value_number_max_name.camel},\n'
    frontend_source += '}\n'

    frontend_mod_path = os.path.join('web', 'src', 'modules', backend_module.under)
    if os.path.exists(frontend_mod_path) and os.path.isdir(frontend_mod_path):
        tfutil.write_file_if_different(os.path.join(frontend_mod_path, enum_name.under + '.enum.ts'), frontend_source)
#endregion

#region union generator

@dataclass
class Type:
    name: str
    size: int
    config: str
    config_update: Callable[[str], str]
    ts_type: str

class Types:
    Bool  = Type('bool',  1, "Config::Bool(false)",  lambda x: f"updateBool({x})", "boolean")

    U8  = Type('uint8_t',  1, "Config::Uint8(0)",  lambda x: f"updateUint({x})", "number")
    U16 = Type('uint16_t', 2, "Config::Uint16(0)", lambda x: f"updateUint({x})", "number")
    U32 = Type('uint32_t', 4, "Config::Uint32(0)", lambda x: f"updateUint({x})", "number")
    U64 = Type('uint64_t', 8, "Config::Uint53(0)", lambda x: f"updateUint53({x})", "number")

    S8  = Type('int8_t',  1, "Config::Int8(0)",  lambda x: f"updateInt({x})", "number")
    S16 = Type('int16_t', 2, "Config::Int16(0)", lambda x: f"updateInt({x})", "number")
    S32 = Type('int32_t', 4, "Config::Int32(0)", lambda x: f"updateInt({x})", "number")
    S64 = Type('int64_t', 8, "Config::Int52(0)", lambda x: f"updateInt52({x})", "number")

    Micros  = Type('micros_t',  8, "Config::Int52(0)", lambda x: f"updateInt52({x}.as<int64_t>())", "number")
    Millis  = Type('millis_t',  8, "Config::Int52(0)", lambda x: f"updateInt52({x}.as<int64_t>())", "number")
    Seconds = Type('seconds_t', 8, "Config::Int52(0)", lambda x: f"updateInt52({x}.as<int64_t>())", "number")

@dataclass
class Member:
    name: NameFlavors = field(init=False)
    _name_str: InitVar[str]
    type: Type

    def __post_init__(self, _name_str):
        self.name = FlavoredName(_name_str).get()

    def get_decl(self):
        return f"{self.type.name} {self.name.under}"

    def get_config(self):
        return f'{self.type.config}'

    def get_conf_write(self, variant_name, member_index, member_count):
        src = f'this->{variant_name.under}.{self.name.under}'

        if member_count == 1 and member_index == 0:
            return f"target->get()->{self.type.config_update(src)};"
        return f"target->get()->get({member_index})->{self.type.config_update(src)};"

    def get_ts_type(self):
        return f'{self.name.under}: {self.type.ts_type}'

@dataclass
class Variant:
    name: NameFlavors = field(init=False)
    _name_str: InitVar[str]

    members: list[Member] = field(default_factory=list)

    def __post_init__(self, _name_str):
        self.name = FlavoredName(_name_str).get()

    def get_union_member(self):
        if len(self.members) == 0:
            return ""
        return f"struct {{{" ".join(("[[gnu::packed]] " if m.type.size > 1 else "") + m.get_decl() + ";" for m in self.members)} }} {self.name.under};"

    def get_factory_fn_signature(self, union_name):
        return f"static {union_name.camel} {self.name.camel}({", ".join(m.get_decl() for m in self.members)});"

    def get_factory_fn_impl(self, union_name):
        if len(self.members) == 0:
            init = "._empty = 0"
        else:
            init = f".{self.name.under} = {{" + \
                 ", ".join(f".{m.name.under} = {m.name.under}" for m in self.members) + \
                 "}"

        return f"{union_name.camel} {union_name.camel}::{self.name.camel}({", ".join(m.get_decl() for m in self.members)}) {{ return {union_name.camel}{{{init}, .tag = {union_name.camel}Tag::{self.name.camel}}}; }}"

    def get_conf_union_prototype(self, union_name):
        if len(self.members) == 0:
            m = "*Config::Null()"
        else:
            m = ", ".join(m.get_config() for m in self.members)

        if len(self.members) > 1:
            m = f"Config::Tuple({{{m}}})"

        return f"{{{union_name.camel}Tag::{self.name.camel}, {m}}}"

    def get_conf_write(self, union_name):
        return f"""case {union_name.camel}Tag::{self.name.camel}:
            {"\n            ".join(m.get_conf_write(self.name, i, len(self.members)) for i, m in enumerate(self.members))}
            break;"""

    def size(self):
        return sum(m.type.size for m in self.members)

    def get_ts_type(self, union_name):
        m = ", ".join(m.get_ts_type() for m in self.members)
        if len(self.members) > 1:
            m = f"[{m}]"
        if len(self.members) > 0:
            m = f", {m}"

        return f"[ {union_name.camel}Tag.{self.name.camel}{m} ]"

@dataclass
class Union:
    name: NameFlavors = field(init=False)
    _name_str: InitVar[str]

    expected_size: int
    variants: list[Variant]

    def __post_init__(self, _name_str):
        self.name = FlavoredName(_name_str).get()

    def get_struct(self):
        max_variant_size = max(v.size() for v in self.variants)
        padding_size = self.expected_size - max_variant_size - 1 # tag takes one byte
        if padding_size < 0:
            print(f"{self.name.space}: {max_variant_size=} >= {self.expected_size=} (tag requires one byte extra)")
            sys.exit(1)
        padding = f'uint8_t _pad[{padding_size}]{{}};'

        return f"""#pragma once

#include <stdint.h>
#include <TFTools/Micros.h>

#include "{self.name.under}_tag.enum.h"
#include "config.h"

struct {self.name.camel} {{
    union {{
        {"uint8_t _empty;" if any(len(v.members) == 0 for v in self.variants) else ""}
        {"\n        ".join(x for x in [v.get_union_member() for v in self.variants] if len(x) > 0)}
    }};
    {self.name.camel}Tag tag;
    {padding}

    {"\n    ".join(v.get_factory_fn_signature(self.name) for v in self.variants)}

    static const ConfUnionPrototype<{self.name.camel}Tag> *getUnionPrototypes();
    static size_t getUnionPrototypeCount();
    static Config getUnion();

    void writeToConfig(Config *target);
}};

static_assert(sizeof({self.name.camel}) == {self.expected_size});
"""

    def get_impl(self):
        return f"""
#include "{self.name.under}.union.h"

{"\n".join(v.get_factory_fn_impl(self.name) for v in self.variants)}

const ConfUnionPrototype<{self.name.camel}Tag> *{self.name.camel}::getUnionPrototypes() {{
    static const ConfUnionPrototype<{self.name.camel}Tag> result[{self.name.upper}_TAG_COUNT] = {{
        {",\n        ".join(v.get_conf_union_prototype(self.name) for v in self.variants)}
    }};
    return result;
}}

size_t {self.name.camel}::getUnionPrototypeCount() {{ return {self.name.upper}_TAG_COUNT; }}

Config {self.name.camel}::getUnion() {{ return Config::Union<{self.name.camel}Tag>(*Config::Null(), {self.name.camel}Tag::None, {self.name.camel}::getUnionPrototypes(), {self.name.camel}::getUnionPrototypeCount()); }}

void {self.name.camel}::writeToConfig(Config *target) {{
    if (target->getTag<{self.name.camel}Tag>() != this->tag)
        target->changeUnionVariant(this->tag);

    switch (this->tag) {{
        {"\n        ".join(v.get_conf_write(self.name) for v in self.variants)}
    }}
}}
"""

    def get_ts_type(self):
        return f"""import {{ {self.name.camel}Tag }} from "./{self.name.under}_tag.enum";

export type {self.name.camel} =
    {" |\n    ".join(v.get_ts_type(self.name) for v in self.variants)};
"""

    def generate(self, module_name: NameFlavors):
        # Generate union tag enum
        generate_enum(
            f'{self.name.space} Tag.uint8.enum',
            module_name,
            FlavoredName(self.name.space + " Tag").get(),
            'uint8_t',
            [EnumValue(v.name, i, '') for i, v in enumerate(self.variants)],
            None)

        # Generate backend union definition and implementation
        with tfutil.ChangedDirectory(os.path.join('src', 'modules', module_name.under)):
            tfutil.write_file_if_different(self.name.under + ".union.h", self.get_struct())
            tfutil.write_file_if_different(self.name.under + ".union.cpp", self.get_impl())

        # Generate frontend API type for union
        frontend_mod_path = os.path.join('web', 'src', 'modules', module_name.under)
        if os.path.exists(frontend_mod_path) and os.path.isdir(frontend_mod_path):
            tfutil.write_file_if_different(os.path.join(frontend_mod_path, self.name.under + '.union.ts'), self.get_ts_type())

#endregion

import importlib.util
import sys

def import_from_path(module_name, file_path):
    spec = importlib.util.spec_from_file_location(module_name, file_path)
    if spec is None or spec.loader is None:
        raise Exception(f"Failed to import spec from file locaton {module_name=} {file_path=}")

    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    spec.loader.exec_module(module)
    return module
