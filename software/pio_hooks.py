Import("env")

import sys

if sys.hexversion < 0x3060000:
    print('Error: Python >= 3.6 required')
    sys.exit(1)

import configparser
import os
import shutil
import subprocess
import time
import re
import json
import glob
import pathlib
from base64 import b64encode
from zlib import crc32
from collections import namedtuple
import tinkerforge_util as tfutil
import util
from hyphenations import hyphenations, allowed_missing
from web.tfpp import tfpp

FrontendComponent = namedtuple('FrontendComponent', 'module component mode')
FrontendStatusComponent = namedtuple('FrontendStatusComponent', 'module component')

def check_call(*args, **kwargs):
    try:
        subprocess.check_call(*args, **kwargs)
    except Exception as e:
        print(f'Error: Command failed: {e}')
        sys.exit(1)

def get_changelog_version(product_id):
    path = f'changelog_{product_id}.txt'
    versions = []

    with open(path, 'r', encoding='utf-8') as f:
        for i, line in enumerate(f.readlines()):
            line = line.rstrip()

            if len(line) == 0:
                continue

            if re.match(r'^(?:- [A-Z0-9\(]|  ([A-Za-z0-9\(\"]|--hide-payload)).*$', line) != None:
                continue

            m = re.match(r'^(?:<unknown>|20[0-9]{2}-[0-9]{2}-[0-9]{2}): ([0-9]+)\.([0-9]+)\.([0-9]+)(?:-beta\.([0-9]+))? \((?:<unknown>|[a-f0-9]+)\)$', line)

            if m == None:
                raise Exception('Invalid line {} in {}: {}'.format(i + 1, path, line))

            version = (int(m.group(1)), int(m.group(2)), int(m.group(3)), int(m.group(4)) if m.group(4) != None else 255)

            if version[0] not in [0, 1, 2]:
                raise Exception('Invalid major version in {}: {}'.format(path, version))

            if len(versions) > 0 and (version[2] < 90 and versions[-1][2] < 90) or (version[2] >= 90 and versions[-1][2] >= 90):
                if versions[-1] >= version:
                    raise Exception('Invalid version order in {}: {} -> {}'.format(path, versions[-1], version))

                if versions[-1][0] == version[0] and versions[-1][1] == version[1] and versions[-1][2] + 1 != version[2]:
                    raise Exception('Invalid version jump in {}: {} -> {}'.format(path, versions[-1], version))

                if versions[-1][0] == version[0] and versions[-1][1] != version[1] and versions[-1][1] + 1 != version[1]:
                    raise Exception('Invalid version jump in {}: {} -> {}'.format(path, versions[-1], version))

                if versions[-1][1] != version[1] and version[2] != 0:
                    raise Exception('Invalid version jump in {}: {} -> {}'.format(path, versions[-1], version))

                if versions[-1][0] != version[0] and (version[1] != 0 or version[2] != 0):
                    raise Exception('Invalid version jump in {}: {} -> {}'.format(path, versions[-1], version))

                # FIXME: validate optional beta part

            versions.append(version)

    if len(versions) == 0:
        raise Exception('No version found in {}'.format(path))

    version_oldest = (str(versions[0][0]), str(versions[0][1]), str(versions[0][2]), str(versions[0][3]))
    version = (str(versions[-1][0]), str(versions[-1][1]), str(versions[-1][2]), str(versions[-1][3]))
    return version_oldest, version

def write_firmware_info(product_id, product_name, version, build_timestamp):
    print(f'Firmware info: {product_id}, {product_name}, {".".join(version[:3]) + ("-beta." + version[3] if version[3] != "255" else "")}, {build_timestamp:08X}')

    buf = bytearray([0xFF] * 4096)

    # 7121CE12F0126E
    # tink er for ge
    buf[0:7] = bytes.fromhex("7121CE12F0126E") # magic
    buf[7] = 0x03 # firmware info version, note: a new version has to be backwards compatible
    product_name_bytes = product_name.encode("utf-8") # max 60 bytes

    if len(product_name_bytes) > 60:
        raise Exception('product_name is longer than 60 bytes')

    buf[8:8 + len(product_name_bytes)] = product_name_bytes
    buf[8 + len(product_name_bytes):68] = bytes(60 - len(product_name_bytes))
    buf[68] = 0x00 # 0 byte to make sure string is terminated. also pads the version, so that the build timestamp will be 4-byte aligned
    buf[69] = int(version[0])
    buf[70] = int(version[1])
    buf[71] = int(version[2])
    buf[72:76] = build_timestamp.to_bytes(4, byteorder='little')
    buf[76] = int(version[3]) # since firmware info version 2
    product_id_bytes = product_id.encode("utf-8") # max 30 bytes

    if len(product_id_bytes) > 60:
        raise Exception('product_id is longer than 60 bytes')

    buf[77:77 + len(product_id_bytes)] = product_id_bytes # since firmware info version 3
    buf[77 + len(product_id_bytes):137] = bytes(60 - len(product_id_bytes))
    buf[137] = 0x00 # 0 byte to make sure string is terminated
    buf[4092:4096] = crc32(buf[0:4092]).to_bytes(4, byteorder='little')

    pathlib.Path(env.subst('$BUILD_DIR'), 'firmware_info.bin').write_bytes(buf)

def generate_module_dependencies(info_path, module, modules, all_modules_upper):
    def get_and_check_duplicates(config, config_key):
        m = config['Dependencies'].get(config_key, "")
        if m is None:
            return None

        m = m.splitlines()

        if '*' in m:
            if len(m) > 1:
                print(f"List of '{config_key}' modules for module '{module_name}' contains * and other entries. Use either * or a specifix list.", file=sys.stderr)
                sys.exit(1)
            m = [x.space for x in modules if x.space != module_name]

        s = set(m)
        if len(s) != len(m):
            print(f"List of '{config_key}' modules for module '{module_name}' contains duplicates.", file=sys.stderr)
            m = sorted(s)
        return m

    if module:
        module_name = module.space
    else:
        module_name = f'[{info_path}]'

    has_dependencies = False
    wants_module_list = False
    fail = False
    dep_modules = []
    all_optional_modules_upper = []

    if os.path.exists(info_path):
        config = configparser.ConfigParser(inline_comment_prefixes=('#',';'))
        config.read(info_path)
        if config.has_section('Dependencies'):
            has_dependencies = True
            wants_module_list = config['Dependencies'].getboolean('ModuleList', False)
            required_modules = []
            available_optional_modules = []

            known_keys = set(['requires', 'optional', 'conflicts', 'after', 'before', 'modulelist'])
            unknown_keys = set(config['Dependencies'].keys()).difference(known_keys)
            if len(unknown_keys) > 0:
                print(f"Dependency error: '{module.under}/module.ini contains unknown keys {unknown_keys}  ", file=sys.stderr)
                fail = True

            requires = get_and_check_duplicates(config, 'Requires')
            for req_name in requires:
                req_module, _ = find_module_space(modules, req_name)
                if not req_module:
                    if '_'.join(req_name.split(' ')).upper() in all_modules_upper:
                        print(f"Dependency error: Module '{module_name}' requires module '{req_name}', which is available but not enabled for this environment.", file=sys.stderr)
                    else:
                        print(f"Dependency error: Module '{module_name}' requires module '{req_name}', which does not exist.", file=sys.stderr)
                    fail = True
                required_modules.append(req_module)

            optional = get_and_check_duplicates(config, 'Optional')
            if optional is not None:
                for opt_name in optional:
                    if opt_name == module_name:
                        print(f"Dependency error: Module '{module_name}' cannot list itself as optional.", file=sys.stderr)
                        fail = True
                    opt_name_upper = '_'.join(opt_name.split(' ')).upper()
                    opt_module, _ = find_module_space(modules, opt_name)
                    if not opt_module:
                        if opt_name_upper not in all_modules_upper:
                            print(f"Dependency error: Optional module '{opt_name}' wanted by module '{module_name}' does not exist.", file=sys.stderr)
                            fail = True
                    else:
                        if opt_module in required_modules:
                            print(f"Dependency error: Optional module '{opt_name}' wanted by module '{module_name}' is already listed as required.", file=sys.stderr)
                            fail = True
                        available_optional_modules.append(opt_module)
                    all_optional_modules_upper.append(opt_name_upper)

            conflicts = get_and_check_duplicates(config, 'Conflicts')
            if conflicts is not None:
                for conflict_name in conflicts:
                    if conflict_name == module_name:
                        print(f"Dependency error: Module '{module_name}' cannot list itself as conflicting.", file=sys.stderr)
                        fail = True
                    conflict_module, index = find_module_space(modules, conflict_name)
                    if index < 0:
                        if '_'.join(conflict_name.split(' ')).upper() not in all_modules_upper:
                            print(f"Dependency error: Module '{conflict_name}' in 'Conflicts' list of module '{module_name}' does not exist.", file=sys.stderr)
                            fail = True
                    elif conflict_module:
                        print(f"Dependency error: Module '{module_name}' conflicts with module '{conflict_name}'.", file=sys.stderr)
                        fail = True

            if module:
                cur_module_index = modules.index(module)

            after = get_and_check_duplicates(config, 'After')
            if after is not None:
                for after_name in after:
                    if after_name == module_name:
                        print(f"Dependency error: Module '{module_name}' cannot require to be loaded after itself.", file=sys.stderr)
                        fail = True
                    _, index = find_module_space(modules, after_name)
                    if index < 0:
                        if '_'.join(after_name.split(' ')).upper() not in all_modules_upper:
                            print(f"Dependency error: Module '{after_name}' in 'After' list of module '{module_name}' does not exist.", file=sys.stderr)
                            fail = True
                    elif index > cur_module_index:
                        print(f"Dependency error: Module '{module_name}' must be loaded after module '{after_name}'.", file=sys.stderr)
                        fail = True

            before = get_and_check_duplicates(config, 'Before')
            if before is not None:
                for before_name in before:
                    if before_name == module_name:
                        print(f"Dependency error: Module '{module_name}' cannot require to be loaded before itself.", file=sys.stderr)
                        fail = True
                    _, index = find_module_space(modules, before_name)
                    if index < 0:
                        if '_'.join(before_name.split(' ')).upper() not in all_modules_upper:
                            print(f"Dependency error: Module '{before_name}' in 'Before' list of module '{module_name}' does not exist.", file=sys.stderr)
                            fail = True
                    elif index < cur_module_index:
                        print(f"Dependency error: Module '{module_name}' must be loaded before module '{before_name}'.", file=sys.stderr)
                        fail = True

            dep_modules = required_modules + available_optional_modules

    if fail:
        sys.exit(1)

    return has_dependencies, wants_module_list, dep_modules, all_optional_modules_upper

def generate_backend_module_dependencies_header(info_path, header_path_prefix, backend_module, backend_modules, all_backend_modules_upper, backend_module_instance_names):
    if backend_module:
        module_name = backend_module.space
    else:
        module_name = f'[{info_path}]'

    has_dependencies, wants_module_list, dep_modules, all_optional_modules_upper = generate_module_dependencies(info_path, backend_module, backend_modules, all_backend_modules_upper)

    if has_dependencies:
        backend_modules_upper = [x.upper for x in backend_modules]

        defines  = ''.join(['#define MODULE_{}_AVAILABLE() {}\n'.format(x, "1" if x in backend_modules_upper else "0") for x in all_optional_modules_upper])
        undefs  = ''.join(['#undef MODULE_{}_AVAILABLE\n'.format(x) for x in all_optional_modules_upper])
        includes = ''.join([f'#include "modules/{x.under}/{x.under}.h"\n' for x in dep_modules])
        decls    = ''.join([f'extern {x.camel} {backend_module_instance_names[x.space]};\n' for x in dep_modules])

        available_h_content  = f'// WARNING: This file is generated from "{info_path}" by pio_hooks.py\n\n'
        available_h_content += '#ifdef MODULE_DEPENDENCIES_H_INCLUDED\n'
        available_h_content += '#error "Any module available header (transitively?) included after module_dependencies.h include! Swap order!"\n'
        available_h_content += '#endif\n'
        available_h_content += '#ifdef MODULE_AVAILABLE_H\n'
        available_h_content += '#error "Overlapping module available header includes! include module_available_end.h at the end of headers including module_available.h!"\n'
        available_h_content += '#endif\n'
        available_h_content += '#define MODULE_AVAILABLE_H\n'

        available_end_h_content = f'// WARNING: This file is generated from "{info_path}" by pio_hooks.py\n\n'
        available_end_h_content += '#undef MODULE_AVAILABLE_H\n'

        dependencies_h_content  = f'// WARNING: This file is generated from "{info_path}" by pio_hooks.py\n\n'
        dependencies_h_content += '#if __INCLUDE_LEVEL__ > 1\n'
        dependencies_h_content += f'#error "Don\'t include {os.path.split(header_path_prefix)[-1]}dependencies.h in headers, only in sources! Use {os.path.split(header_path_prefix)[-1]}available.h in headers if you want to check whether a module is compiled in"\n'
        dependencies_h_content += '#endif\n\n'

        if defines:
            available_h_content += '\n' + defines
        if undefs:
            available_end_h_content += '\n' + undefs
        if includes:
            dependencies_h_content += '\n' + includes
        if decls:
            dependencies_h_content += '\n' + decls

        dependencies_h_content += f'\n\n#include "{os.path.split(header_path_prefix)[-1]}available.h"\n'
        dependencies_h_content += f'#define MODULE_DEPENDENCIES_H_INCLUDED\n'

        if wants_module_list:
            dependencies_h_content += '\n'
            dependencies_h_content += '#include "config.h"\n'
            dependencies_h_content += 'extern Config modules;\n'

        tfutil.write_file_if_different(header_path_prefix + 'available.h', available_h_content)
        tfutil.write_file_if_different(header_path_prefix + 'dependencies.h', dependencies_h_content)
        tfutil.write_file_if_different(header_path_prefix + 'available_end.h', available_end_h_content)
    else:
        try:
            os.remove(header_path_prefix + 'available.h')
        except FileNotFoundError:
            pass

        try:
            os.remove(header_path_prefix + 'dependencies.h')
        except FileNotFoundError:
            pass

def generate_frontend_module_available_file(info_path, file_path_prefix, frontend_module, frontend_modules, all_frontend_modules_upper):
    if frontend_module:
        module_name = frontend_module.space
    else:
        module_name = f'[{info_path}]'

    has_dependencies, wants_module_list, dep_modules, all_optional_modules_upper = generate_module_dependencies(info_path, frontend_module, frontend_modules, all_frontend_modules_upper)

    if has_dependencies:
        frontend_modules_upper = [x.upper for x in frontend_modules]
        defines = ''.join(['//#define MODULE_{}_AVAILABLE {}\n'.format(x, "1" if x in frontend_modules_upper else "0") for x in all_optional_modules_upper])
        available_inc_content  = f'// WARNING: This file is generated from "{info_path}" by pio_hooks.py\n'

        if defines:
            available_inc_content += '\n' + defines

        tfutil.write_file_if_different(file_path_prefix + 'available.inc', available_inc_content)
    else:
        try:
            os.remove(file_path_prefix + 'available.inc')
        except FileNotFoundError:
            pass

def update_translation(translation, update, override=False, parent_key=None):
    if parent_key == None:
        parent_key = []

    assert isinstance(translation, dict), '.'.join(parent_key)
    assert isinstance(update, dict), '.'.join(parent_key)

    for key, value in update.items():
        if not isinstance(value, dict) or key not in translation:
            if override:
                try:
                    if translation[key] != None:
                        pass #raise Exception('.'.join(parent_key + [key]) + ' cannot override non-null translation')
                except KeyError:
                    print('Ignoring unused translation override', '.'.join(parent_key + [key]))
                    continue
            elif key in translation:
                raise Exception('.'.join(parent_key + [key]) + ' cannot replace existing translation')

            translation[key] = value
        else:
            update_translation(translation[key], value, override=override, parent_key=parent_key + [key])

# Header that is expected to be found in translation_*.tsx files
TSX_HEADER = """/** @jsxImportSource preact */
import { h } from "preact";
let x = """

# Additional lines that are allowed in translation_*.tsx files
TSX_ADDITIONAL_HEADER_LINES = [
    'import { toLocaleFixed } from "../../ts/util";',
    'import { __ } from "../../ts/translation";',
    'import { __ } from "./src/ts/translation";',
    'import { removeUnicodeHacks } from "../../ts/translation";',
    'import { removeUnicodeHacks } from "./src/ts/translation";',
    'import { __, removeUnicodeHacks } from "../../ts/translation";',
    'import { __, removeUnicodeHacks } from "./src/ts/translation";',
    'import * as options from "../../options";',
    'import * as options from "./src/options";',
]

TSX_LINE_COMMENT_PATTERN = re.compile(r'^[ \t]*//.*$', re.MULTILINE)

TSX_FRAGMENT_PATTERN = re.compile(r'<>.*?</>', re.MULTILINE | re.DOTALL)
TSX_FUNCTION_PATTERN = re.compile(r'/\*[SF]FN\*/.*?/\*NF\*/', re.MULTILINE | re.DOTALL)
TSX_BACKTICK_PATTERN = re.compile(r'`.*?`', re.MULTILINE | re.DOTALL)

TSX_FUNCTION_ARGS_PATTERN = re.compile(r'FN\*/\s*\(([^\)]*)\)', re.MULTILINE | re.DOTALL)

TSX_JSON_REPLACEMENTS = [
    # Escape nested fragments in functions
    ('<>', '***START_FRAGMENT***'),
    ('</>', '***END_FRAGMENT***'),

    # Escape backticks
    ('`', '***BACKTICK***')
]

def tsx_to_json(match):
    s = match.group(0)

    for old, new in TSX_JSON_REPLACEMENTS:
        s = s.replace(old, new)

    return json.dumps(s)

def json_to_tsx(s):
    for old, new in TSX_JSON_REPLACEMENTS:
        s = s.replace(new, old)

    return s

def collect_translation(path, override=False):
    translation = {}

    tsxs = glob.glob(os.path.join(path, 'translation_*.tsx'))
    jsons = glob.glob(os.path.join(path, 'translation_*.json'))
    if len(tsxs) > 0 and len(jsons) > 0:
        print(f"Found translation_*.tsx and translation_*.json in same module ({path})! Use either tsx or json translations!")
        sys.exit(1)

    for translation_path in tsxs + jsons:
        m = re.match(r'translation_([a-z]+){0}\.(tsx|json)'.format('_override' if override else ''), os.path.split(translation_path)[-1])

        if m == None:
            continue

        language = m.group(1)
        is_tsx = m.group(2) == "tsx"

        with open(translation_path, 'r', encoding='utf-8') as f:
            content = f.read()
            if is_tsx:
                for x in TSX_ADDITIONAL_HEADER_LINES:
                    content = content.replace(x + "\n", "", 1)
                content = content.replace(TSX_HEADER, "", 1)
                content = re.sub(TSX_LINE_COMMENT_PATTERN, "", content)
                content = re.sub(TSX_FUNCTION_PATTERN, tsx_to_json, content)
                content = re.sub(TSX_FRAGMENT_PATTERN, tsx_to_json, content)
                content = re.sub(TSX_BACKTICK_PATTERN, tsx_to_json, content)
            try:
                translation[language] = json.loads(content)
            except:
                with open("/tmp/out.json", "w", encoding='utf-8') as f:
                    f.write(content)
                print('JSON error in', translation_path)
                raise

    return translation

def check_translation(translation, parent_key=None):
    if parent_key == None:
        parent_key = []

    assert isinstance(translation, dict), '.'.join(parent_key)

    for key, value in translation.items():
        assert isinstance(value, (dict, str)), '.'.join(parent_key + [key]) + ' has unexpected type ' + type(value).__name__

        if isinstance(value, dict):
            check_translation(value, parent_key=parent_key + [key])

HYPHENATE_THRESHOLD = 9

missing_hyphenations = {}

def should_be_hyphenated(x):
    return x not in allowed_missing and not x.startswith("***START_FRAGMENT***") and not x.endswith("***END_FRAGMENT***")

def hyphenate(s, key, lang):
    if '\u00AD' in s:
        print("Found unicode soft hyphen in translation value {}: {}".format(key, s.replace('\u00AD', "___HERE___")))
        sys.exit(1)

    if '&shy;' in s:
        print("Found HTML entity soft hyphen in translation value {}: {}".format(key, s))
        sys.exit(1)

    # s could be a string or fragment function. We don't want to hyphenate the translation keys in __() calls used in those.
    # To prevent re.split(r'\W+', s) from seeing the key components (for example in __("power_manager.automation.foo") "automation" should not be hyphenated)
    # "escape" them with a string that is probably not used in keys but does not form a word boundary: 'ÄÖÜÄÖÜ'
    s = re.sub(r'__\("([^"]+)"\)', lambda match: f'__("{match.group(1).replace(".", "ÄÖÜÄÖÜ")}")', s)

    def repl(m: re.Match):
        word = m.group(0)

        for l, r in hyphenations:
            if word == l:
                return r
        else:
            is_too_long = len(word) > HYPHENATE_THRESHOLD
            is_camel_case = re.search(r'[a-z][A-Z]', word) is not None
            is_snake_case = "_" in word
            is_escaped = 'ÄÖÜÄÖÜ' in word
            if is_too_long and not is_camel_case and not is_snake_case and not is_escaped and should_be_hyphenated(word):
                missing_hyphenation = missing_hyphenations.setdefault(lang, [])

                if word not in missing_hyphenation:
                    missing_hyphenation.append(word)
            return word

    s = re.sub(r'\w+', repl, s)

    # Reverse escaping of translation keys.
    s = re.sub(r'__\("([^"]+)"\)', lambda match: f'__("{match.group(1).replace("ÄÖÜÄÖÜ", ".")}")', s)

    return s

def hyphenate_translation(translation, parent_key=None, lang=None):
    if parent_key == None:
        parent_key = []

    return {key: (hyphenate(value, key, lang if lang is not None else key) if isinstance(value, str) else hyphenate_translation(value, parent_key + [key], lang if lang is not None else key)) for key, value in translation.items()}

def repair_rtc_dir():
    path = os.path.abspath("src/modules/rtc")

    for filename in ["real_time_clock_v2_bricklet_firmware_bin.digest",
                     "real_time_clock_v2_bricklet_firmware_bin.embedded.cpp",
                     "real_time_clock_v2_bricklet_firmware_bin.embedded.h"]:
        try:
            os.remove(os.path.join(path, filename))
        except FileNotFoundError:
            pass

def repair_firmware_update_dir():
    path = os.path.abspath("src/modules/firmware_update")

    for filename in ["recovery_html.digest",
                     "recovery_html.embedded.cpp",
                     "recovery_html.embedded.h",
                     "signature_public_key.embedded.cpp",
                     "signature_public_key.embedded.h"]:
        try:
            os.remove(os.path.join(path, filename))
        except FileNotFoundError:
            pass

def repair_meters_modbus_tcp_dir():
    path = os.path.abspath("src/modules/meters_modbus_tcp")

    for filename in ["modbus_register_type.enum.cpp",
                     "modbus_register_type.enum.h",
                     "modbus_register_address_mode.enum.cpp",
                     "modbus_register_address_mode.enum.h",
                     "Modbus Value Type.uint8.enum",
                     "modbus_value_type.enum.cpp",
                     "modbus_value_type.enum.h"]:
        try:
            os.remove(os.path.join(path, filename))
        except FileNotFoundError:
            pass

    path = os.path.abspath("web/src/modules/meters_modbus_tcp")

    for filename in ["modbus_register_type.enum.ts",
                     "modbus_register_address_mode.enum.ts",
                     "modbus_value_type.enum.ts"]:
        try:
            os.remove(os.path.join(path, filename))
        except FileNotFoundError:
            pass

def repair_meters_sun_spec_dir():
    path = os.path.abspath("src/modules/meters_sun_spec")

    for filename in ["sun_spec_model_id.cpp",
                     "sun_spec_model_id.h"]:
        try:
            os.remove(os.path.join(path, filename))
        except FileNotFoundError:
            pass

def find_module_space(modules, name_space):
    index = 0
    for module in modules:
        if module.space == name_space:
            return module, index
        index += 1

    name_upper = '_'.join(name_space.split(' ')).upper()
    for module in modules:
        if module.upper == name_upper:
            print(f"Dependency error: Encountered incorrectly capitalized module '{name_space}'", file=sys.stderr)
            sys.exit(1)

    return None, -1

def preprocess_web(frontend_modules_under):
    with tfutil.ChangedDirectory('web'):
        try:
            shutil.rmtree('src_tfpp')
        except FileNotFoundError:
            pass

        for root, dirs, files in os.walk('src', followlinks=True):
            for name in files:
                src_path = pathlib.Path(root) / name

                if src_path.parts[:2] == ('src', 'modules') and src_path.parts[2] not in frontend_modules_under:
                    continue

                src_tfpp_path = pathlib.Path('src_tfpp', *src_path.parts[1:])

                if src_path.suffix in ['.ts', '.tsx']:
                    try:
                        tfpp(src_path, src_tfpp_path)
                    except Exception as e:
                        print(f'Error: {e}', file=sys.stderr)
                        exit(42)
                else:
                    src_tfpp_path.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(src_path, src_tfpp_path)

def build_web(js_source_map, css_source_map, no_minify):
    with tfutil.ChangedDirectory(pathlib.Path('web', 'src_tfpp')):
        js_analyze = False

        build_dir = pathlib.Path('..', 'build')

        html_minifier_terser_options = [
            '--collapse-boolean-attributes',
            '--collapse-inline-tag-whitespace',
            '--collapse-whitespace',
            '--conservative-collapse',
            '--decode-entities',
            '--no-include-auto-generated-tags',
            '--minify-urls',
            '--prevent-attributes-escaping',
            '--process-conditional-comments',
            '--remove-attribute-quotes',
            '--remove-comments',
            '--remove-empty-attributes',
            '--remove-optional-tags',
            '--remove-redundant-attributes',
            '--remove-script-type-attributes',
            '--remove-style-link-type-attributes',
            '--sort-attributes',
            '--sort-class-name',
            '--trim-custom-fragments',
            '--use-short-doctype'
        ]

        try:
            shutil.rmtree(str(build_dir))
        except FileNotFoundError:
            pass

        build_dir.mkdir(parents=True)

        print('tsc...')
        check_call([
            'npx',
            'tsc',
            '--build',
            'tsconfig.json'
        ], shell=sys.platform == 'win32')

        print('esbuild...')
        esbuild_args = [
            'npx',
            'esbuild',
            'main.tsx',
            f'--metafile={build_dir / "meta.json"}',
            '--bundle',
            '--target=es6',
            '--alias:argon2-browser=../node_modules/argon2-browser/dist/argon2-bundled.min.js',
            '--alias:jquery=../node_modules/jquery/dist/jquery.slim.min',
            f'--outfile={build_dir / "bundle.min.js"}'
        ]

        if js_analyze:
            esbuild_args += ['--analyze']

        if js_source_map:
            esbuild_args += ['--sourcemap=inline']

        if not no_minify:
            esbuild_args += ['--minify']

        check_call(esbuild_args, shell=sys.platform == 'win32')

        print('sass...')
        scss_args = [
            'npx',
            'sass',
            '--silence-deprecation',
            'color-functions,import,global-builtin,abs-percent,mixed-decls'  # still used by bootstrap 4
        ]

        if not css_source_map:
            scss_args += ['--no-source-map']

        scss_args += [
            'main.scss',
            str(build_dir / 'main.css')
        ]

        check_call(scss_args, shell=sys.platform == 'win32')

        print('postcss...')
        check_call([
            'npx',
            'postcss',
            str(build_dir / 'main.css'),
            '-o',
            str(build_dir / 'main.min.css')
        ], shell=sys.platform == 'win32')

        if css_source_map:
            css_src = (build_dir / 'main.min.css').read_text(encoding='utf-8')
            css_map = b64encode((build_dir / 'main.min.css.map').read_bytes()).decode('ascii')

            (build_dir / 'main.min.css.map').unlink()

            css_src = css_src.replace('sourceMappingURL=main.min.css.map', f'sourceMappingURL=data:text/json;base64,{css_map}')

            (build_dir / 'main.min.css').write_text(css_src, encoding='utf-8')

        print('html-minifier-terser...')
        check_call([
            'npx',
            'html-minifier-terser'] +
            html_minifier_terser_options + [
            '-o',
            str(build_dir / 'index.min.html'),
            'index.html'
        ], shell=sys.platform == 'win32')

def main():
    if env.IsCleanTarget():
        return

    # Enable this for class_size script
    #env.Append(CXXFLAGS=["-fdump-lang-class"])

    repair_rtc_dir()
    repair_firmware_update_dir()
    repair_meters_modbus_tcp_dir()
    repair_meters_sun_spec_dir()

    check_call([env.subst('$PYTHONEXE'), "-u", "update_packages.py"])

    build_flags = env.GetProjectOption("build_flags")
    product_id = env.GetProjectOption("custom_product_id", None)
    options = env.GetProjectOption("custom_options", "")

    old_style_options = {}

    for key in ['name', 'manufacturer', 'manufacturer_full', 'manufacturer_user_agent', 'config_type', 'host_prefix', 'display_name', 'display_name_user_agent_override',
                'manual_url', 'apidoc_url', 'doc_base_url', 'firmware_url', 'firmware_update_url', 'remote_access_host', 'support_email', 'day_ahead_price_api_url',
                'solar_forecast_api_url', 'require_firmware_info', 'local_meter_default_display_name']:
        value = env.GetProjectOption('custom_' + key, None)

        if value != None:
            old_style_options[key] = value

    if len(old_style_options) > 0:
        print('Warning: Use of old-style custom_* options detected! Trying to auto-convert to new-style custom_options!')
        print('  ' + '\n  '.join([f'{key} = {value}' for key, value in old_style_options.items()]))

        product_id = old_style_options['name']
        del old_style_options['name']

        old_style_options.pop('manufacturer_user_agent', None)

        old_style_options['hostname_prefix'] = old_style_options['host_prefix']
        del old_style_options['host_prefix']

        old_style_options['product_name'] = old_style_options['display_name']
        del old_style_options['display_name']

        old_style_options.pop('display_name_user_agent_override', None)

        old_style_options['mqtt_api_doc_url'] = old_style_options['apidoc_url']
        del old_style_options['apidoc_url']

        old_style_options['warp_doc_base_url'] = old_style_options['doc_base_url']
        del old_style_options['doc_base_url']

        old_style_options['firmware_update_latest_url'] = old_style_options['firmware_url']
        del old_style_options['firmware_url']

        if 'firmware_update_url' in old_style_options:
            old_style_options['firmware_update_update_url'] = old_style_options['firmware_update_url']
            del old_style_options['firmware_update_url']

        if 'remote_access_host' not in old_style_options:
            old_style_options['remote_access_host'] = 'my.warp-charger.com'

    for key, value in old_style_options.items():
        if key not in ('require_firmware_info',):
            value = json.dumps(value)

        options += '\n' + key + ' = ' + value

    old_style_build_flags = {}

    for key in ['FIRMWARE_UPDATE_ENABLE_ROLLBACK', 'METERS_SLOTS', 'METERS_SLOT_0_DEFAULT_CLASS', 'POWER_MANAGER_DEFAULT_METER_SLOT', 'CHARGE_TRACKER_PDF_LOGO']:
        for build_flag in build_flags:
            if key in build_flag:
                old_style_build_flags[key.lower()] = build_flag.split(key + '=')[-1]

    if len(old_style_build_flags) > 0:
        print('Warning: Use of old-style build_flags detected! Trying to auto-convert to new-style custom_options!')
        print('  ' + '\n  '.join([f'{key} = {value}' for key, value in old_style_build_flags.items()]))

        if 'charge_tracker_pdf_logo' not in old_style_build_flags:
            old_style_build_flags['charge_tracker_pdf_logo'] = 'CHARGE_TRACKER_PDF_LOGO_WARP'

        for key, value in old_style_build_flags.items():
            if key == 'meters_slots':
                key = 'meters_max_slots'

            options += '\n' + key + ' = ' + value

    web_build_flags = env.GetProjectOption("custom_web_build_flags", "")
    old_style_web_build_flags = {}

    for key_value in web_build_flags.split('\n'):
        if len(key_value) == 0:
            continue

        key, value = key_value.split('=', 1)
        old_style_web_build_flags[key.strip().lower()] = value.strip()

    if len(old_style_web_build_flags) > 0:
        print('Warning: Use of old-style web_build_flags detected! Trying to auto-convert to new-style custom_options!')
        print('  ' + '\n  '.join([f'{key} = {value}' for key, value in old_style_web_build_flags.items()]))

        for key, value in old_style_web_build_flags.items():
            if key == 'meters_slots':
                key = 'meters_max_slots'

            options += '\n' + key + ' = ' + value

    build_timestamp = int(time.time())
    branding = env.GetProjectOption("custom_branding")
    frontend_debug = env.GetProjectOption("custom_frontend_debug") == "true"
    web_only = env.GetProjectOption("custom_web_only") == "true"
    prepare_only = "-DPREPARE_ONLY" in build_flags
    signature_preset = env.GetProjectOption("custom_signature_preset")
    monitor_speed = env.GetProjectOption("monitor_speed")
    nightly = "-DNIGHTLY" in build_flags

    if sys.platform.startswith('linux'):
        firmware_elf_symlink = f'build/{product_id}_firmware_latest.elf'
        firmware_bin_symlink = f'build/{product_id}_firmware_latest_merged.bin'

        try:
            os.remove(firmware_elf_symlink)
        except FileNotFoundError:
            pass

        try:
            os.remove(firmware_bin_symlink)
        except FileNotFoundError:
            pass

        try:
            os.remove('build/firmware_latest.elf')
        except FileNotFoundError:
            pass

        try:
            os.remove('build/firmware_latest_merged.bin')
        except FileNotFoundError:
            pass

    is_release = len(subprocess.run(["git", "tag", "--contains", "HEAD"], check=True, capture_output=True).stdout) > 0
    is_dirty = len(subprocess.run(["git", "diff", "HEAD"], check=True, capture_output=True).stdout) > 0
    dirty_suffix = ""
    git_url = subprocess.run(["git", "config", "--get", "remote.origin.url"], check=True, capture_output=True).stdout.decode("utf-8").strip()
    git_commit_id = subprocess.run(["git", "rev-parse", "--short=15", "HEAD"], check=True, capture_output=True).stdout.decode("utf-8").strip()
    branch_name = subprocess.run(["git", "branch", "--show-current"], check=True, capture_output=True).stdout.decode("utf-8").strip()

    if is_dirty or not is_release:
        if branch_name == "master":
            dirty_suffix = '_' + git_commit_id
        else:
            dirty_suffix = '_' + git_commit_id + "_" + branch_name.replace("_", "-")

    try:
        version_oldest, version = get_changelog_version(product_id)
    except Exception as e:
        print('Error: Could not get changelog version: {0}'.format(e))
        sys.exit(1)

    build_src_filter = ['+<*>', '-<empty.cpp>']

    if not os.path.isdir("build"):
        os.makedirs("build")

    # Open <project_name>_wifi.json (example: esp32_ethernet_wifi.json) for custom default WIFI configuration.
    # If default_wifi.json is available it is used for all projects instead of <project_name>_wifi.json.
    # If no *_wifi.json is available the default as defined in Wifi::setup in the wifi.c is used.
    not_for_distribution = False
    is_from_default_wifi_json = False
    try:
        with open(os.path.join('default_wifi.json'), 'r', encoding='utf-8') as f:
            custom_wifi = json.loads(f.read())
            is_from_default_wifi_json = True
    except FileNotFoundError:
        try:
            with open(os.path.join(product_id + '_wifi.json'), 'r', encoding='utf-8') as f:
                custom_wifi = json.loads(f.read())
        except FileNotFoundError:
            custom_wifi = {}

    if 'mqtt_enable' in custom_wifi:
        build_flags.append('-DDEFAULT_MQTT_ENABLE={0}'.format(custom_wifi['mqtt_enable']))

    if 'mqtt_broker_host' in custom_wifi:
        build_flags.append('-DDEFAULT_MQTT_BROKER_HOST=\\"{0}\\"'.format(custom_wifi['mqtt_broker_host']))

    if 'mqtt_broker_port' in custom_wifi:
        build_flags.append('-DDEFAULT_MQTT_BROKER_PORT={0}'.format(custom_wifi['mqtt_broker_port']))

    if 'mqtt_broker_username' in custom_wifi:
        build_flags.append('-DDEFAULT_MQTT_BROKER_USERNAME=\\"{0}\\"'.format(custom_wifi['mqtt_broker_username']))

    if 'mqtt_broker_password' in custom_wifi:
        build_flags.append('-DDEFAULT_MQTT_BROKER_PASSWORD=\\"{0}\\"'.format(custom_wifi['mqtt_broker_password']))

    if 'ap_enable' in custom_wifi:
        build_flags.append('-DDEFAULT_WIFI_AP_ENABLE={0}'.format(custom_wifi['ap_enable']))

    if 'ap_fallback_only' in custom_wifi:
        build_flags.append('-DDEFAULT_WIFI_AP_FALLBACK_ONLY={0}'.format(custom_wifi['ap_fallback_only']))

    if 'ap_ssid' in custom_wifi:
        build_flags.append('-DDEFAULT_WIFI_AP_SSID="\\"{0}\\""'.format(custom_wifi['ap_ssid']))

    if 'ap_passphrase' in custom_wifi:
        # If password comes from the default_wifi.json it is not for distribution and the file is renamed accordingly
        not_for_distribution = is_from_default_wifi_json
        build_flags.append('-DDEFAULT_WIFI_AP_PASSPHRASE="\\"{0}\\""'.format(custom_wifi['ap_passphrase']))

    if 'sta_enable' in custom_wifi:
        build_flags.append('-DDEFAULT_WIFI_STA_ENABLE={0}'.format(custom_wifi['sta_enable']))

    if 'sta_ssid' in custom_wifi:
        build_flags.append('-DDEFAULT_WIFI_STA_SSID="\\"{0}\\""'.format(custom_wifi['sta_ssid']))

    if 'sta_passphrase' in custom_wifi:
        # If password comes from the default_wifi.json it is not for distribution and the file is renamed accordingly
        not_for_distribution = is_from_default_wifi_json
        build_flags.append('-DDEFAULT_WIFI_STA_PASSPHRASE="\\"{0}\\""'.format(custom_wifi['sta_passphrase']))

    if 'debug_fs_enable' in custom_wifi:
        # Force not for distribution if file system access is enabled.
        # This is too dangerous to distribute, as one can access for example stored wifi passphrases
        # via /debug/fs/config/wifi_sta_config
        not_for_distribution = True
        build_flags.append('-DDEBUG_FS_ENABLE="\\"{0}\\""'.format(custom_wifi['debug_fs_enable']))

    if 'w_error' in custom_wifi and custom_wifi['w_error']:
        build_flags.append('-Werror')

    env.Replace(BUILD_FLAGS=build_flags)

    build_lines = []
    build_lines.append('#pragma once')
    build_lines.append('#include <stdint.h>')
    build_lines.append('#define BUILD_VERSION_OLDEST_MAJOR {}'.format(version_oldest[0]))
    build_lines.append('#define BUILD_VERSION_OLDEST_MINOR {}'.format(version_oldest[1]))
    build_lines.append('#define BUILD_VERSION_OLDEST_PATCH {}'.format(version_oldest[2]))
    build_lines.append('#define BUILD_VERSION_OLDEST_BETA {}'.format(version_oldest[3]))
    build_lines.append('#define BUILD_VERSION_MAJOR {}'.format(version[0]))
    build_lines.append('#define BUILD_VERSION_MINOR {}'.format(version[1]))
    build_lines.append('#define BUILD_VERSION_PATCH {}'.format(version[2]))
    build_lines.append('#define BUILD_VERSION_BETA {}'.format(version[3]))
    build_lines.append('#define BUILD_VERSION_STRING "{}.{}.{}"'.format(*version))
    build_lines.append('#define BUILD_MONITOR_SPEED {}'.format(monitor_speed))
    build_lines.append('#define BUILD_IS_SIGNED() {}'.format("1" if len(signature_preset) > 0 else "0"))
    build_lines.append('uint32_t build_timestamp();')
    build_lines.append('const char *build_timestamp_hex_str();')
    build_lines.append('const char *build_version_full_str();')
    build_lines.append('const char *build_version_full_str_upper();')
    build_lines.append('const char *build_info_str();')
    build_lines.append('const char *build_filename_str();')
    build_lines.append('const char *build_commit_id_str();')
    build_lines.append('#define BUILD_CUSTOM_APP_DESC_MAGIC 0xBCDE6543')
    build_lines.append('#define BUILD_CUSTOM_APP_DESC_VERSION 1')
    build_lines.append('typedef struct {')
    build_lines.append('    uint32_t magic; // BUILD_CUSTOM_APP_DESC_MAGIC')
    build_lines.append('    uint8_t version; // BUILD_CUSTOM_APP_DESC_VERSION')
    build_lines.append('    uint8_t padding[3];')
    build_lines.append('    uint8_t fw_version_major;')
    build_lines.append('    uint8_t fw_version_minor;')
    build_lines.append('    uint8_t fw_version_patch;')
    build_lines.append('    uint8_t fw_version_beta;')
    build_lines.append('    uint32_t fw_build_timestamp;')
    build_lines.append('} build_custom_app_desc_t;')
    tfutil.write_file_if_different(os.path.join('src', 'build.h'), '\n'.join(build_lines))

    firmware_basename = '{}_firmware{}{}{}_{}_{:x}{}'.format(
        product_id,
        "-UNSIGNED" if len(signature_preset) == 0 else "",
        "-NIGHTLY" if nightly else "",
        "-WITH-WIFI-PASSPHRASE-DO-NOT-DISTRIBUTE" if not_for_distribution else "",
        "{}_{}_{}{}".format(*version[:3], f"_beta_{version[3]}" if version[3] != "255" else ""),
        build_timestamp,
        dirty_suffix,
    )

    version_full_str = "{}.{}.{}{}+{:x}".format(*version[:3], f"-beta.{version[3]}" if version[3] != "255" else "", build_timestamp)

    build_lines = []
    build_lines.append('#include "build.h"')
    build_lines.append('uint32_t build_timestamp() {{ return {}; }}'.format(build_timestamp))
    build_lines.append('const char *build_timestamp_hex_str() {{ return "{:x}"; }}'.format(build_timestamp))
    build_lines.append('const char *build_version_full_str() {{ return "{}"; }}'.format(version_full_str))
    build_lines.append('const char *build_version_full_str_upper() {{ return "{}"; }}'.format(version_full_str.upper()))
    build_lines.append('const char *build_info_str() {{ return "git url: {}, git branch: {}, git commit id: {}"; }}'.format(git_url, branch_name, git_commit_id))
    build_lines.append('const char *build_filename_str() {{ return "{}"; }}'.format(firmware_basename))
    build_lines.append('const char *build_commit_id_str() {{ return "{}"; }}'.format(git_commit_id))
    build_lines.append('static_assert(sizeof(build_custom_app_desc_t) == 16, "build_custom_app_desc_t has wrong size");')
    build_lines.append('extern const __attribute__((section(".rodata_custom_desc"))) build_custom_app_desc_t build_custom_app_desc = {')
    build_lines.append('    BUILD_CUSTOM_APP_DESC_MAGIC,')
    build_lines.append('    BUILD_CUSTOM_APP_DESC_VERSION,')
    build_lines.append('    {0, 0, 0},')
    build_lines.append('    BUILD_VERSION_MAJOR,')
    build_lines.append('    BUILD_VERSION_MINOR,')
    build_lines.append('    BUILD_VERSION_PATCH,')
    build_lines.append('    BUILD_VERSION_BETA,')
    build_lines.append('    {},'.format(build_timestamp))
    build_lines.append('};')
    tfutil.write_file_if_different(os.path.join('src', 'build.cpp'), '\n'.join(build_lines))
    del build_lines

    with open(os.path.join(env.subst('$BUILD_DIR'), 'firmware_basename'), 'w', encoding='utf-8') as f:
        f.write(firmware_basename)

    frontend_modules = [util.FlavoredName(x).get() for x in env.GetProjectOption("custom_frontend_modules").splitlines()]

    if nightly:
        frontend_modules.append(util.FlavoredName("Nightly").get())
        frontend_modules.append(util.FlavoredName("Debug").get())

    branding_module = util.FlavoredName('Branding ' + branding).get()
    frontend_modules.append(branding_module)
    branding_mod_path = os.path.join('web', 'src', 'modules', branding_module.under)

    if not os.path.exists(branding_mod_path):
        print(f'Error: Branding module {branding} Branding missing')
        sys.exit(1)

    for filename in ['branding.ts', 'logo.png', 'favicon.png']:
        if not os.path.exists(os.path.join(branding_mod_path, filename)):
            print(f'Error: Branding module {branding} Branding does not contain {filename}')
            sys.exit(1)

    frontend_components = []
    for entry in env.GetProjectOption("custom_frontend_components").splitlines():
        m = re.match(r'^\s*([^|\$]+)(?:\s*\|\s*([^|\$]+))?(?:\s*\$\s*(Open|Close))?\s*$', entry)

        if m == None:
            print('Error: Invalid custom_frontend_components entry:', entry)
            sys.exit(1)

        module = util.FlavoredName(m.group(1).strip()).get()

        if module not in frontend_modules:
            print('Error: Unknown module in custom_frontend_components entry:', module.space)
            sys.exit(1)

        if m.group(2) != None:
            component = util.FlavoredName(m.group(2).strip()).get()
        else:
            component = module

        mode = m.group(3)

        frontend_components.append(FrontendComponent(module, component, mode))

    if nightly:
        module = util.FlavoredName("Debug").get()
        component = module
        frontend_components.append(FrontendComponent(module, component, None))

    frontend_status_components = []
    for entry in env.GetProjectOption("custom_frontend_status_components").splitlines():
        parts = [x.strip() for x in entry.split('|')]

        if len(parts) == 1:
            module = util.FlavoredName(parts[0]).get()
            component = util.FlavoredName(parts[0] + ' Status').get()
        elif len(parts) == 2:
            module = util.FlavoredName(parts[0]).get()
            component = util.FlavoredName(parts[1]).get()
        else:
            print('Error: Invalid custom_frontend_status_components entry:', entry)
            sys.exit(1)

        if module not in frontend_modules:
            print('Error: Unknown module in custom_frontend_status_components entry:', module.space)
            sys.exit(1)

        frontend_status_components.append(FrontendStatusComponent(module, component))

    metadata = json.dumps({
        'product_id': product_id,
        'signature_preset': signature_preset,
        'frontend_modules': [frontend_module.under for frontend_module in frontend_modules],
        'branding_mod_path': os.path.abspath(branding_mod_path),
    }, separators=(',', ':'))

    # Handle backend modules
    excluded_backend_modules = list(os.listdir('src/modules'))
    # The order 'Task Scheduler', 'Event Log', 'API' must be exactly this one:
    # API::setup migrates the config and expects that tasks can be scheduled and the logger is initialized
    backend_modules = [util.FlavoredName(x).get() for x in ['Task Scheduler', 'Event Log', 'API', 'Web Server', 'Rtc'] + env.GetProjectOption("custom_backend_modules").splitlines()]

    if nightly:
        backend_modules.append(util.FlavoredName("Debug").get())

    with tfutil.ChangedDirectory('src'):
        excluded_bindings = [pathlib.PurePath(x).as_posix() for x in glob.glob('bindings/brick_*') + glob.glob('bindings/bricklet_*')]

    excluded_bindings.remove('bindings/bricklet_unknown.h')
    excluded_bindings.remove('bindings/bricklet_unknown.c')

    def include_bindings(path):
        if not path.endswith('.h') and not path.endswith('.c') and not path.endswith('.cpp'):
            return

        with open(path, 'r', encoding='utf-8') as f:
            for line in f.readlines():
                m = p.match(line)

                if m != None:
                    binding = m.group(1)

                    if binding + '.h' in excluded_bindings:
                        excluded_bindings.remove(binding + '.h')

                    if binding + '.c' in excluded_bindings:
                        excluded_bindings.remove(binding + '.c')

    p = re.compile(r'#\s*include\s+[<"](bindings/brick(?:let)?_.*)\.h[>"]')

    for backend_module in backend_modules:
        mod_path = os.path.join('src', 'modules', backend_module.under)

        if not os.path.exists(mod_path) or not os.path.isdir(mod_path):
            print("Backend module '{}' not found.".format(backend_module.space))
            sys.exit(1)

        for root, dirs, files in os.walk(mod_path, followlinks=True):
            for filename in files:
                include_bindings(os.path.join(root, filename))

        excluded_backend_modules.remove(backend_module.under)

        if os.path.exists(os.path.join(mod_path, "prepare.py")):
            util.log('Preparing backend module:', backend_module.space)

            environ = dict(os.environ)
            environ['PLATFORMIO_PROJECT_DIR'] = env.subst('$PROJECT_DIR')
            environ['PLATFORMIO_BUILD_DIR'] = env.subst('$BUILD_DIR')
            environ['PLATFORMIO_METADATA'] = metadata

            with tfutil.ChangedDirectory(mod_path):
                check_call([env.subst('$PYTHONEXE'), "-u", "prepare.py"], env=environ)

    for root, dirs, files in os.walk('src', followlinks=True):
        root_path = pathlib.PurePath(root)
        root_parents = [root_path] + list(root_path.parents)

        if pathlib.PurePath('src', 'bindings') in root_parents or pathlib.PurePath('src', 'modules') in root_parents:
            continue

        for filename in files:
            include_bindings(os.path.join(root, filename))

    for excluded_backend_module in excluded_backend_modules:
        build_src_filter.append('-<modules/{0}/*>'.format(excluded_backend_module))

    for excluded_binding in excluded_bindings:
        build_src_filter.append('-<{0}>'.format(excluded_binding))

    env.Replace(SRC_FILTER=[' '.join(build_src_filter)])

    all_backend_modules_upper = []
    for existing_backend_module in os.listdir(os.path.join('src', 'modules')):
        if not os.path.isdir(os.path.join('src', 'modules', existing_backend_module)):
            continue

        all_backend_modules_upper.append(existing_backend_module.upper())

    identifier_backlist = ["system"]

    tfutil.specialize_template("modules.cpp.template", os.path.join("src", "modules.cpp"), {
        '{{{imodule_extern_decls}}}': '\n'.join([f'extern IModule *const {x.under}_imodule;' for x in backend_modules]),
        '{{{imodules_count}}}': str(len(backend_modules)),
        '{{{imodules_array}}}': '\n    '.join([f'{{&{x.under}_imodule, "{x.camel}"}},' for x in backend_modules]),
        '{{{module_init_config}}}': ',\n        '.join(f'{{"{x.under}", Config::Bool({x.under}_imodule->initialized)}}' for x in backend_modules if not x.under.startswith("hidden_")),
    })

    util.log("Generating module_dependencies.h from module.ini", flush=True)

    backend_module_instance_names = {}

    for backend_module in backend_modules:
        mod_path = os.path.join('src', 'modules', backend_module.under)
        info_path = os.path.join(mod_path, 'module.ini')
        instance_name = backend_module.under

        backend_module_instance_names[backend_module.space] = backend_module.under

        if not os.path.exists(info_path):
            print(f'Warning: {backend_module.under} has no module.ini file')
        else:
            config = configparser.ConfigParser(inline_comment_prefixes=('#',';'))
            config.read(info_path)

            if config.has_section('Common'):
                known_keys = set(['instancename'])
                unknown_keys = set(config['Common'].keys()).difference(known_keys)

                if len(unknown_keys) > 0:
                    print(f"Module error: '{backend_module.under}/module.ini contains unknown keys {unknown_keys}  ", file=sys.stderr)
                    sys.exit(1)

                instance_name = config['Common'].get('InstanceName', instance_name)

        if instance_name in identifier_backlist:
            instance_name += '_'

        backend_module_instance_names[backend_module.space] = instance_name

        with open(os.path.join(mod_path, 'module.cpp'), 'w', encoding='utf-8') as f:
            f.write(f'// WARNING: This file is generated from "{info_path}" by pio_hooks.py\n\n')
            f.write(f'#include "{backend_module.under}.h"\n\n')
            f.write(f'{backend_module.camel} {instance_name};\n\n')
            f.write('// Enforce that all back-end modules implement the IModule interface. If you receive\n')
            f.write("// an error like \"cannot convert 'MyModule*' to 'IModule*' in initialization\", you\n")
            f.write('// have to add the IModule interface to your back-end module\'s class declaration:\n')
            f.write('//\n')
            f.write('// class MyModule final : public IModule\n')
            f.write('// {\n')
            f.write('//     // content here\n')
            f.write('// }\n')
            # To get global constants that are usable in other compilation units, they must be
            # declared extern. Otherwise, they will be optimized away before reaching the linker.
            f.write(f'extern IModule *const {backend_module.under}_imodule = &{instance_name};\n')

    for backend_module in backend_modules:
        mod_path = os.path.join('src', 'modules', backend_module.under)
        info_path = os.path.join(mod_path, 'module.ini')
        header_path_prefix = os.path.join(mod_path, 'module_')

        generate_backend_module_dependencies_header(info_path, header_path_prefix, backend_module, backend_modules, all_backend_modules_upper, backend_module_instance_names)

    generate_backend_module_dependencies_header('src/main_dependencies.ini', 'src/main_', None, backend_modules, all_backend_modules_upper, backend_module_instance_names)

    # Generate options
    options_value = {}
    options_origin = {}

    def collect_options(module_type, mod_path_prefix, module):
        mod_path = os.path.join(mod_path_prefix, module.under)

        if not os.path.exists(mod_path) or not os.path.isdir(mod_path):
            print(f"{module_type.capitalize()} module {module.space} not found")
        else:
            info_path = os.path.join(mod_path, 'module.ini')

            if os.path.exists(info_path):
                config = configparser.ConfigParser(inline_comment_prefixes=('#',';'))
                config.read(info_path)

                if config.has_section('Options'):
                    for key in config['Options']:
                        value = config['Options'][key]

                        if len(value) == 0:
                            print(f"Option {key} in {module_type} module {module.under} has empty value", file=sys.stderr)
                            sys.exit(1)

                        if key in options_origin:
                            print(f"{module_type.capitalize()} module {module.under} redefines option {key} already defined in {options_origin[key]}", file=sys.stderr)
                            sys.exit(1)

                        options_value[key] = value
                        options_origin[key] = f'{module_type} module {module.under}'

    for backend_module in backend_modules:
        collect_options('backend', 'src/modules', backend_module)

    for frontend_module in frontend_modules:
        collect_options('frontend', 'web/src/modules', frontend_module)

    if len(options) > 0:
        for key_value in options.split('\n'):
            if len(key_value) == 0:
                continue

            if '=' not in key_value:
                print(f"Option {repr(key_value)} in environment is not <key>=<value>", file=sys.stderr)
                sys.exit(1)

            key, value = key_value.split('=', 1)
            key = key.strip()
            value = value.strip()

            if len(key) == 0:
                print(f"Option {key} in environment has empty key", file=sys.stderr)
                sys.exit(1)

            if len(value) == 0:
                print(f"Option {key} in environment has empty value", file=sys.stderr)
                sys.exit(1)

            options_value[key] = value
            options_origin[key] = 'environment'

    assert 'product_id' not in options_value

    options_value['product_id'] = json.dumps(product_id)
    options_origin['product_id'] = 'pio_hooks.py'

    for suffix in ['warp', 'warp2', 'warp3', 'warp4', 'energy_manager', 'energy_manager_v2', 'smart_energy_broker', 'eltako']:
        key = f'product_id_is_{suffix}'

        assert key not in options_value

        options_value[key] = '1' if product_id == suffix else '0'
        options_origin[key] = 'pio_hooks.py'

    options_value['product_id_is_warp_any'] = '1' if product_id in ['warp', 'warp2', 'warp3', 'warp4'] else '0'
    options_origin['product_id_is_warp_any'] = 'pio_hooks.py'

    assert 'manufacturer_upper' not in options_value

    options_value['manufacturer_upper'] = options_value['manufacturer'].upper()
    options_origin['manufacturer_upper'] = 'pio_hooks.py'

    manufacturer_user_agent = json.loads(options_value['manufacturer']).replace("²", "2").replace(" ", "_")
    manufacturer_user_agent.encode('ascii')

    assert re.match(r'^[ !#$%&\'\*+\.\^_`\|~0-9a-zA-Z-]+$', manufacturer_user_agent), manufacturer_user_agent
    assert 'manufacturer_user_agent' not in options_value

    options_value['manufacturer_user_agent'] = json.dumps(manufacturer_user_agent)
    options_origin['manufacturer_user_agent'] = 'pio_hooks.py'

    assert 'product_name_upper' not in options_value

    options_value['product_name_upper'] = options_value['product_name'].upper()
    options_origin['product_name_upper'] = 'pio_hooks.py'

    product_name_user_agent = json.loads(options_value['product_name']).replace("/", " ").replace(" ", "_")
    product_name_user_agent.encode('ascii')

    assert re.match(r'^[ !#$%&\'\*+\.\^_`\|~0-9a-zA-Z-]+$', product_name_user_agent), product_name_user_agent
    assert 'product_name_user_agent' not in options_value

    options_value['product_name_user_agent'] = json.dumps(product_name_user_agent)
    options_origin['product_name_user_agent'] = 'pio_hooks.py'

    options_h = []
    options_ts = []
    options_inc = []
    options_key_ljust = max([len(key) for key in options_value.keys()])
    options_value_ljust = max([len(key) for key in options_value.values()])

    print('Options:')

    for key, value in sorted(options_value.items()):
        print(f'  {key.ljust(options_key_ljust)} = {str(value).ljust(options_value_ljust)} // from {options_origin[key]}')

        options_h.append(f'// from {options_origin[key]}')
        options_h.append(f'#define OPTIONS_{key.upper()}() {value}\n')

        if value in ['0', '1']:
            options_inc.append(f'// from {options_origin[key]}')
            options_inc.append(f'//#define OPTIONS_{key.upper()} {value}\n')

        if re.match(r'^(-?\d+|".*")$', value) == None:
            value = json.dumps(value)

        options_ts.append(f'// from {options_origin[key]}')
        options_ts.append(f'export const {key.upper()} = {value};\n')

    tfutil.write_file_if_different(os.path.join('src', 'options.h'), '// WARNING: This file is generated by pio_hooks.py\n\n' + '\n'.join(options_h))
    tfutil.write_file_if_different(os.path.join('web', 'src', 'options.ts'), '// WARNING: This file is generated by pio_hooks.py\n\n' + '\n'.join(options_ts))
    tfutil.write_file_if_different(os.path.join('web', 'src', 'options.inc'), '// WARNING: This file is generated by pio_hooks.py\n\n' + '\n'.join(options_inc))

    firmware_info_product_id_override = json.loads(options_value['firmware_info_product_id_override'])

    if len(firmware_info_product_id_override) > 0:
        firmware_info_product_id = firmware_info_product_id_override
    else:
        firmware_info_product_id = product_id

    firmware_info_product_name_override = json.loads(options_value['firmware_info_product_name_override'])

    if len(firmware_info_product_name_override) > 0:
        firmware_info_product_name = firmware_info_product_name_override
    else:
        firmware_info_product_name = json.loads(options_value['product_name'])

    write_firmware_info(firmware_info_product_id, firmware_info_product_name, version, build_timestamp)

    # Handle frontend modules
    main_ts_entries = []
    pre_scss_paths = []
    post_scss_paths = []
    translation = collect_translation('web')

    # API
    api_imports = []
    api_config_map_entries = []
    api_cache_entries = []
    module_counter = 0
    exported_interface_pattern = re.compile("export interface ([A-Za-z0-9$_]+)")
    exported_type_pattern = re.compile("export type ([A-Za-z0-9$_]+)")
    api_path_pattern = re.compile("//APIPath:([^\n]*)\n")

    for frontend_module in frontend_modules:
        mod_path = os.path.join('web', 'src', 'modules', frontend_module.under)

        if os.path.exists(os.path.join(mod_path, "prepare.py")):
            util.log('Preparing frontend module:', frontend_module.space)

            environ = dict(os.environ)
            environ['PLATFORMIO_PROJECT_DIR'] = env.subst('$PROJECT_DIR')
            environ['PLATFORMIO_BUILD_DIR'] = env.subst('$BUILD_DIR')
            environ['PLATFORMIO_METADATA'] = metadata

            with tfutil.ChangedDirectory(mod_path):
                check_call([env.subst('$PYTHONEXE'), "-u", "prepare.py"], env=environ)

        if os.path.exists(os.path.join(mod_path, 'main.ts')) or os.path.exists(os.path.join(mod_path, 'main.tsx')):
            main_ts_entries.append(frontend_module.under)

        if os.path.exists(os.path.join(mod_path, 'api.ts')):
            with open(os.path.join(mod_path, 'api.ts'), 'r', encoding='utf-8') as f:
                content = f.readlines()

            api_path = frontend_module.under + "/"

            found_api_exports = False

            for line in content:
                match = api_path_pattern.match(line)
                if match is not None:
                    api_path = match.group(1).strip()
                    continue

                match = exported_interface_pattern.match(line)
                if match is None:
                    match = exported_type_pattern.match(line)
                    if match is None:
                        continue

                found_api_exports = True

                type_ = match.group(1)
                api_suffix = type_.split("___")[0]

                api_config_map_entries.append("'{}{}': module_{}.{},".format(api_path, api_suffix, module_counter, type_))
                api_cache_entries.append("'{}{}': undefined as any,".format(api_path, api_suffix))
                api_cache_entries.append("'{}{}_modified': undefined as any,".format(api_path, api_suffix))

            if found_api_exports:
                api_module = "module_{}".format(module_counter)
                module_counter += 1

                api_imports.append("import * as {} from '../modules/{}/api';".format(api_module, frontend_module.under))

        for phase, scss_paths in [('pre', pre_scss_paths), ('post', post_scss_paths)]:
            scss_path = os.path.join(mod_path, phase + '.scss')

            if os.path.exists(scss_path):
                scss_paths.append(os.path.relpath(scss_path, 'web/src'))

        update_translation(translation, collect_translation(mod_path))
        update_translation(translation, collect_translation(mod_path, override=True), override=True)

    check_translation(translation)
    translation = hyphenate_translation(translation)

    all_frontend_modules_upper = []
    for existing_frontend_module in os.listdir(os.path.join('web', 'src', 'modules')):
        if not os.path.isdir(os.path.join('web', 'src', 'modules', existing_frontend_module)):
            continue

        all_frontend_modules_upper.append(existing_frontend_module.upper())

    for frontend_module in frontend_modules:
        mod_path = os.path.join('web', 'src', 'modules', frontend_module.under)
        info_path = os.path.join(mod_path, 'module.ini')
        file_path_prefix = os.path.join(mod_path, 'module_')

        generate_frontend_module_available_file(info_path, file_path_prefix, frontend_module, frontend_modules, all_frontend_modules_upper)

    global missing_hyphenations
    if len(missing_hyphenations) > 0:
        print("Missing hyphenations detected. Add those to hyphenations.py!")
        dicts = None
        try:
            from pyphen import Pyphen
            langs = {
                'de': 'de_DE',
                'en': 'en_US'
            }
            dicts = {k: Pyphen(lang=langs[k]) for k in missing_hyphenations.keys()}
        except ImportError:
            print("Pyphen not installed. Will not print hyphenation suggestions.")

        for lang, lst in missing_hyphenations.items():
            print("  {}".format(lang))
            for x in lst:
                if dicts is None:
                    print("    {}".format(x))
                else:
                    print('    "{}",'.format(dicts[lang].inserted(x)))

    for path in glob.glob(os.path.join('web', 'src', 'ts', 'translation_*.ts')):
        os.remove(path)

    if len(translation) == 0:
        print('Error: Translation missing')
        sys.exit(1)

    tfutil.write_file_if_different(os.path.join('web', 'src', 'ts', 'translation.json'), json.dumps(translation, indent=4, ensure_ascii=False))

    with open(os.path.join(branding_mod_path, 'favicon.png'), 'rb') as f:
        favicon = b64encode(f.read()).decode('ascii')

    with open(os.path.join(branding_mod_path, 'logo.png'), 'rb') as f:
        logo_base64 = b64encode(f.read()).decode('ascii')

    with open(os.path.join(branding_mod_path, 'branding.ts'), 'r', encoding='utf-8') as f:
        branding_ts = f.read()

    with open(os.path.join(branding_mod_path, 'pre.scss'), 'r', encoding='utf-8') as f:
        color = f.read().split('\n')[1].split(' ')[1]
        if color.endswith(';'):
            color = color[:-1]

    tfutil.specialize_template(os.path.join("web", "index.html.template"), os.path.join("web", "src", "index.html"), {
        '{{{favicon}}}': favicon,
        '{{{theme_color}}}': color
    })

    navbar = []
    navbar_nesting = 0
    navbar_group = None
    navbar_mapping = []

    for frontend_component in frontend_components:
        if frontend_component[2] == 'Open':
            navbar.append(f'<{frontend_component[1].camel}Navbar group_ref={{this.{frontend_component[1].under}_ref}}>')
            navbar_nesting += 1
            navbar_group = frontend_component
        elif frontend_component[2] == 'Close':
            navbar.append(f'</{frontend_component[1].camel}Navbar>')
            navbar_nesting -= 1
            navbar_group = None
        else:
            navbar.append(f'{"   " * navbar_nesting}<{frontend_component[1].camel}Navbar />')

            if navbar_group != None:
                navbar_mapping.append((frontend_component[1].under, f'{navbar_group[1].under}_ref'))

    def has_status_component(x):
        return (x.component.space + " Status") in [y.component.space for y in frontend_status_components]

    def has_subpage_component(x):
        return (x.component.space.endswith(" Status") and x.component.space.removesuffix(" Status") in [y.component.space for y in frontend_components])

    tfutil.specialize_template(os.path.join("web", "app.tsx.template"), os.path.join("web", "src", "app.tsx"), {
        '{{{logo_base64}}}': logo_base64,
        '{{{navbar_imports}}}': '\n'.join([f'import {{ {x.component.camel}Navbar }} from "./modules/{x.module.under}/main";' for x in frontend_components if x.mode != 'Close']),
        '{{{navbar}}}': '\n                                    '.join(navbar),
        '{{{navbar_refs}}}': '\n    '.join([f'{x.component.under}_ref = createRef();' for x in frontend_components if x.mode == 'Open']),
        '{{{navbar_refs_mapping}}}': '\n            '.join([f'{repr(x[0])}: this.{x[1]},' for x in navbar_mapping]),
        '{{{content_imports}}}': '\n'.join([f'import {{ {x.component.camel} }} from "./modules/{x.module.under}/main";' for x in frontend_components if x.mode == None]),
        '{{{content}}}': '\n                            '.join([f'<{x.component.camel}{f" status_ref={{this.{x.component.under}_status_ref}}" if has_status_component(x) else ""} />' for x in frontend_components if x.mode == None]),
        '{{{status_imports}}}': '\n'.join([f'import {{ {x.component.camel} }} from "./modules/{x.module.under}/main";' for x in frontend_status_components]),
        '{{{status}}}': '\n                                '.join([f'<{x.component.camel}{f" ref={{this.{x.component.under}_ref}}" if has_subpage_component(x) else "" } />' for x in frontend_status_components]),
        '{{{status_refs}}}': '\n    '.join([f'{x.component.under}_ref = createRef();' for x in frontend_status_components if has_subpage_component(x)]),
    })

    tfutil.specialize_template(os.path.join("web", "main.tsx.template"), os.path.join("web", "src", "main.tsx"), {
        '{{{module_imports}}}': '\n'.join(['import * as {0} from "./modules/{0}/main";'.format(x) for x in main_ts_entries]),
        '{{{modules}}}': ', '.join([x for x in main_ts_entries]),
        '{{{preact_debug}}}': 'import "preact/debug";' if frontend_debug else ''
    })

    tfutil.specialize_template(os.path.join("web", "main.scss.template"), os.path.join("web", "src", "main.scss"), {
        '{{{module_pre_imports}}}': '\n'.join(['@import "{0}";'.format(x.replace('\\', '/')) for x in pre_scss_paths]),
        '{{{module_post_imports}}}': '\n'.join(['@import "{0}";'.format(x.replace('\\', '/')) for x in post_scss_paths])
    })

    tfutil.specialize_template(os.path.join("web", "api_defs.ts.template"), os.path.join("web", "src", "ts", "api_defs.ts"), {
        '{{{imports}}}': '\n'.join(api_imports),
        '{{{module_interface}}}': ',\n    '.join('{}: boolean'.format(x.under) for x in backend_modules),
        '{{{config_map_entries}}}': '\n    '.join(api_config_map_entries),
        '{{{api_cache_entries}}}': '\n    '.join(api_cache_entries),
    })

    tfutil.specialize_template(os.path.join("web", "branding.ts.template"), os.path.join("web", "src", "ts", "branding.ts"), {
        '{{{logo_base64}}}': logo_base64,
        '{{{branding}}}': branding_ts,
    })

    translation_str = ''

    def format_translation(language, subtranslation, type_only, indent):
        output = ['{\n']

        assert isinstance(subtranslation, (dict, str)), type(subtranslation)

        for key, value in sorted(subtranslation.items()):
            output += [indent + '    ', key, ': ']

            if isinstance(value, dict):
                output += format_translation(language, value, type_only, indent + '    ')
            else:
                is_fragment = value.startswith("***START_FRAGMENT***") and value.endswith("***END_FRAGMENT***")
                is_string_function = value.startswith("/*SFN*/") and value.endswith("/*NF*/")
                is_fragment_function = value.startswith("/*FFN*/") and value.endswith("/*NF*/")
                is_backtick = value.startswith("***BACKTICK***") and value.endswith("***BACKTICK***")
                is_string = not is_fragment and not is_string_function and not is_fragment_function and not is_backtick

                if type_only:
                    if is_fragment:
                        output += ['VNode,\n']
                    elif is_string_function or is_fragment_function:
                        args = re.search(TSX_FUNCTION_ARGS_PATTERN, value).group(1)
                        result = "string" if is_string_function else "VNode"
                        output += ['({}) => {},\n'.format(args, result)]
                    else:
                        output += ['string,\n']
                else:
                    if is_fragment:
                        string = json_to_tsx(value)
                    elif is_string_function or is_fragment_function:
                        # removeprefix/suffix are new in Python 3.9. We have to support 3.8.
                        string = json_to_tsx(value)#[len("/*SFN*/"):-len("/*NF*/")]
                    elif is_backtick:
                        string = json_to_tsx(value)
                    else:
                        string = '"{0}"'.format(value.replace('"', '\\"'))

                    if is_string or is_string_function:
                        if match := re.search(r"<[^>]*>", value):
                            print("Found HTML tag {} in non-fragment value {}".format(match.group(0), value))

                    subtranslation[key] = string
                    output += [string, ',\n']

        output += [indent, '}']

        if len(indent) > 0:
            output += [',\n']

        return output

    def replace_placeholders_in_placeholders(language, output):
        replacement_made = True

        def lookup_translation(language, match):
            nonlocal replacement_made
            replacement_made = True
            path: list = match.group(1).split(".")
            tr = translation[language]

            while len(path) > 0:
                tr = tr[path.pop(0)]

            return tr

        while replacement_made:
            replacement_made = False
            for i, x in enumerate(output):
                x = re.sub(r'__\("([^"]+)"\)(?=[^\(])', lambda match: lookup_translation(language, match), x)
                # Simplify "a" + "b" to "ab"
                x = x.replace('" + "', '')

                # Simplify <>{"abc"}</> to abc
                x = re.sub(r'{<>\"([^"{]*)\"</>}', r'\g<1>', x)

                # Simplify {<>abc</>} to abc
                x = re.sub(r'{<>([^<{]*)</>}', r'\g<1>', x)

                # Simplify {"a"} to a
                x = re.sub(r'{\"([^"]*)\"}', r'\g<1>', x)

                output[i] = x
        return output

    translation_str += 'type Translation = ' + ''.join(format_translation('en', translation['en'], True, '')) + '\n\n'

    for language in sorted(translation):
        formatted = format_translation(language, translation[language], False, '')
        formatted = replace_placeholders_in_placeholders(language, formatted)
        translation_str += 'const translation_{0}: Translation = {1} as const\n\n'.format(language, ''.join(formatted))

    translation_str += 'const translation: {[index: string]: Translation} = {\n'

    for language in sorted(translation):
        translation_str += '    "{0}": translation_{0},\n'.format(language)

    translation_str += '} as const\n'

    tfutil.specialize_template(os.path.join("web", "translation.tsx.template"), os.path.join("web", "src", "ts", "translation.tsx"), {
        '{{{translation}}}': translation_str,
    })

    # Generate enums
    for backend_module in backend_modules:
        mod_path = os.path.join('src', 'modules', backend_module.under)

        if not os.path.exists(mod_path) or not os.path.isdir(mod_path):
            print("Backend module {} not found.".format(backend_module.space))
        else:
            for filename in os.listdir(mod_path):
                if not filename.endswith(".enum"):
                    continue

                filename_parts = filename.split('.')

                if len(filename_parts) != 3:
                    print('Error: Invalid enum file "{}" in backend {}'.format(filename, mod_path))
                    sys.exit(1)

                try:
                    with open(os.path.join(mod_path, filename + '.previous'), 'r', encoding='utf-8') as f:
                        enum_previous_raw_values = json.loads(f.read())
                except FileNotFoundError:
                    enum_previous_raw_values = None

                enum_comments = []
                enum_name = util.FlavoredName(filename_parts[0]).get()
                enum_raw_values = {}
                enum_values = []
                enum_cases = []
                value_number = -1
                value_number_min = None
                value_number_max = None
                value_count = 0

                with open(os.path.join(mod_path, filename), 'r', encoding='utf-8') as f:
                    for line in f.readlines():
                        line = line.strip()

                        if len(line) == 0:
                            continue

                        m = re.match(r'^(?:(#).*|//\s*(.*)|([A-Za-z][A-Za-z0-9 ]+?)?\s*(?:=\s*(\d+))?\s*(?://\s*(.*))?)$', line)

                        if m == None:
                            print(f'Error: Malformed line enum file "{filename}" in backend {mod_path}: {line}')
                            sys.exit(1)

                        file_comment = m.group(1)

                        if file_comment != None:
                            continue

                        enum_comment = m.group(2)

                        if enum_comment != None:
                            enum_comments.append(f'// {enum_comment}\n')
                            continue

                        value_name = util.FlavoredName(m.group(3)).get()

                        if m.group(4) != None:
                            value_number = int(m.group(4))
                        else:
                            value_number += 1

                        value_comment = m.group(5)

                        if value_comment == None:
                            value_comment = ''
                        else:
                            value_comment = ' // ' + value_comment

                        if value_number_min == None:
                            value_number_min = value_number
                        else:
                            value_number_min = min(value_number_min, value_number)

                        if value_number_max == None:
                            value_number_max = value_number
                        else:
                            value_number_max = max(value_number_max, value_number)

                        value_count += 1

                        if value_name.space in enum_raw_values:
                            print(f'Error: Duplicate value "{value_name.space}" in enum file "{filename}" in backend {mod_path}')
                            sys.exit(1)

                        enum_raw_values[value_name.space] = value_number
                        enum_values.append(f'    {value_name.camel} = {value_number},{value_comment}\n')
                        enum_cases.append(f'    case {enum_name.camel}::{value_name.camel}: return "{value_name.space}";\n')

                if enum_previous_raw_values != None:
                    for value_name, value_number in enum_raw_values.items():
                        if value_name in enum_previous_raw_values and enum_previous_raw_values[value_name] != value_number:
                            print(f'Error: Invalid change to value "{value_name}" in enum file "{filename}" in backend {mod_path}')
                            sys.exit(1)

                tfutil.write_file_if_different(os.path.join(mod_path, filename + '.previous'), json.dumps(enum_raw_values))

                with open(os.path.join(mod_path, enum_name.under + '.enum.h'), 'w', encoding='utf-8') as f:
                    f.write(f'// WARNING: This file is generated from "{filename}" by pio_hooks.py\n\n')
                    f.write('#include <stdint.h>\n\n')
                    f.write('#pragma once\n\n')
                    f.write(''.join(enum_comments))
                    f.write(f'enum class {enum_name.camel} : {filename_parts[1]}_t {{\n')
                    f.write(f'    _min = {value_number_min},\n')
                    f.write(''.join(enum_values))
                    f.write(f'    _max = {value_number_max},\n')
                    f.write('};\n\n')
                    f.write(f'#define {enum_name.upper}_COUNT {value_count}\n\n')
                    f.write(f'const char *get_{enum_name.under}_name({enum_name.camel} value);\n')

                with open(os.path.join(mod_path, enum_name.under + '.enum.cpp'), 'w', encoding='utf-8') as f:
                    f.write(f'// WARNING: This file is generated from "{filename}" by pio_hooks.py\n\n')
                    f.write(f'#include "{enum_name.under}.enum.h"\n\n')
                    f.write(f'const char *get_{enum_name.under}_name({enum_name.camel} value)\n')
                    f.write('{\n')
                    f.write('    switch (value) {\n')
                    f.write(''.join(enum_cases))
                    f.write('    default: return "Unknown";\n')
                    f.write('    }\n')
                    f.write('}\n')

                frontend_mod_path = os.path.join('web', 'src', 'modules', backend_module.under)

                if os.path.exists(frontend_mod_path) and os.path.isdir(frontend_mod_path):
                    with open(os.path.join(frontend_mod_path, enum_name.under + '.enum.ts'), 'w', encoding='utf-8') as f:
                        f.write(f'// WARNING: This file is generated from "{filename}" by pio_hooks.py\n\n')
                        f.write(f'export const enum {enum_name.camel} {{\n')
                        f.write(f'    _min = {value_number_min},\n')
                        f.write(''.join(enum_values))
                        f.write(f'    _max = {value_number_max},\n')
                        f.write('}\n')

    # Preprocessing web interface
    util.log('Preprocessing web interface')

    preprocess_web([x.under for x in frontend_modules])

    # Check translation completeness
    util.log('Checking translation completeness')

    with tfutil.ChangedDirectory('web'):
        check_call([env.subst('$PYTHONEXE'), "-u", "check_translation_completeness.py"] + [x.under for x in frontend_modules])

    # Check translation override completeness
    util.log('Checking translation override completeness')

    with tfutil.ChangedDirectory('web'):
        check_call([env.subst('$PYTHONEXE'), "-u", "check_override_completeness.py"])

    if prepare_only:
        print("Stopping build after prepare")
        sys.exit(0)

    # Generate web interface
    util.log('Checking web interface dependencies')

    node_modules_src_paths = ['web/package-lock.json']

    # FIXME: Scons runs this script using exec(), resulting in __file__ being not available
    #node_modules_src_paths.append(__file__)

    node_modules_needs_update, node_modules_reason, node_modules_digest = util.check_digest(node_modules_src_paths, [], 'web', 'node_modules', env=env)
    node_modules_digest_paths = util.get_digest_paths('web', 'node_modules', env=env)

    if not node_modules_needs_update and os.path.exists('web/node_modules/tinkerforge.marker'):
        util.log('Web interface dependencies are up-to-date')
    else:
        if not os.path.exists('web/node_modules/tinkerforge.marker'):
            node_modules_reason = 'marker file missing'

        print('Web interface dependencies are not up-to-date ({0}), updating now'.format(node_modules_reason))

        util.remove_digest('web', 'node_modules', env=env)

        def clear_directory(path):
            if not os.path.exists(path):
                return

            with os.scandir(path) as entries:
                for entry in entries:
                    if entry.is_dir() and not entry.is_symlink():
                        shutil.rmtree(entry.path)
                    else:
                        os.remove(entry.path)

            if len(os.listdir(path)) == 0:
                return

            raise Exception('Failed to clear ' + path)

        # On Windows, for some unknown reason, sometimes a directory
        # stays or becomes non-empty during the shutil.rmtree call and
        # cannot be removed anymore. If that happens, just try again.
        #
        # Only clear (not remove) the node_modules directory,
        # in case it is a mount point, soft-, hardlink or in some other way special.
        attempts = 10
        for i in range(attempts):
            try:
                clear_directory('web/node_modules')
                break
            except:
                time.sleep(0.5)

                if i == attempts - 1:
                    raise

        with tfutil.ChangedDirectory('web'):
            npm_version = subprocess.check_output(['npm', '--version'], shell=sys.platform == 'win32', encoding='utf-8').strip()

            m = re.fullmatch(r'(\d+)\.\d+\.\d+', npm_version)

            if m == None:
                print('Error: npm version has unexpected format: {0}'.format(npm_version))
                sys.exit(1)

            if int(m.group(1)) < 8:
                print('Error: npm >= 8 required, found npm {0}'.format(npm_version))
                sys.exit(1)

            check_call(['npm', 'ci'], shell=sys.platform == 'win32')

        with open('web/node_modules/tinkerforge.marker', 'wb') as f:
            pass

        util.store_digest(node_modules_digest, 'web', 'node_modules', env=env)

    util.log('Checking web interface')

    index_html_src_paths = []
    index_html_src_datas = []

    for filename in sorted(os.listdir('web')):
        path = os.path.join('web', filename)

        if os.path.isfile(path):
            index_html_src_paths.append(path)

    for root, dirs, files in sorted(os.walk('web/src', followlinks=True)):
        for filename in sorted(files):
            index_html_src_paths.append(os.path.join(root, filename))

    index_html_src_paths += node_modules_digest_paths

    for frontend_module in frontend_modules: # ensure changes to the frontend modules change the digest
        index_html_src_datas.append(frontend_module.under.encode('utf-8'))

    # FIXME: Scons runs this script using exec(), resulting in __file__ being not available
    #index_html_src_paths.append(__file__)

    index_html_needs_update, index_html_reason, index_html_digest = util.check_digest(index_html_src_paths, index_html_src_datas, 'src', 'index_html', env=env)

    if not index_html_needs_update and os.path.exists('src/index_html.embedded.h') and os.path.exists('src/index_html.embedded.cpp'):
        util.log('Web interface is up-to-date')
    else:
        if not os.path.exists('src/index_html.embedded.h') or not os.path.exists('src/index_html.embedded.cpp'):
            index_html_reason = 'embedded file missing'

        print('Web interface is not up-to-date ({0}), building now'.format(index_html_reason))

        util.remove_digest('src', 'index_html', env=env)

        try:
            shutil.rmtree('web/build')
        except FileNotFoundError:
            pass

        build_py_args = []

        js_source_map = False
        css_source_map = False
        no_minify = False

        if frontend_debug:
            js_source_map = True
            css_source_map = True
            no_minify = True

        build_web(js_source_map, css_source_map, no_minify)

        with open('web/build/main.min.css', 'r', encoding='utf-8') as f:
            css = f.read()

        with open('web/build/bundle.min.js', 'r', encoding='utf-8') as f:
            js = f.read()

        with open('web/build/index.min.html', 'r', encoding='utf-8') as f:
            html = f.read()

        html = html.replace('<link href=css/main.css rel=stylesheet>', '<style rel=stylesheet>{0}</style>'.format(css))
        html = html.replace('<script src=js/bundle.js></script>', '<script>{0}</script>'.format(js))
        html_bytes = html.encode('utf-8')

        with open('web/build/index.standalone.html', 'wb') as f:
            f.write(html_bytes)

        util.embed_data(util.gzip_compress(html_bytes), 'src', 'index_html', 'char', 'ssize_t')
        util.store_digest(index_html_digest, 'src', 'index_html', env=env)

    if web_only:
        print('Stopping build after web')
        sys.exit(0)

main()
