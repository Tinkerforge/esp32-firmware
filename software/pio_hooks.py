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
from pathlib import PurePath
from base64 import b64encode
from zlib import crc32
import util

from hyphenations import hyphenations, allowed_missing

# use "with ChangedDirectory('/path/to/abc')" instead of "os.chdir('/path/to/abc')"
class ChangedDirectory(object):
    def __init__(self, path):
        self.path = path
        self.previous_path = None

    def __enter__(self):
        self.previous_path = os.getcwd()
        os.chdir(self.path)

    def __exit__(self, type_, value, traceback):
        os.chdir(self.previous_path)


def get_changelog_version(name):
    path = os.path.join('changelog_{}.txt'.format(name))
    versions = []

    with open(path, 'r', encoding='utf-8') as f:
        for i, line in enumerate(f.readlines()):
            line = line.rstrip()

            if len(line) == 0:
                continue

            if re.match(r'^(?:- [A-Z0-9\(]|  ([A-Za-z0-9\(\"]|--hide-payload)).*$', line) != None:
                continue

            m = re.match(r'^(?:<unknown>|20[0-9]{2}-[0-9]{2}-[0-9]{2}): ([0-9]+)\.([0-9]+)\.([0-9]+) \((?:<unknown>|[a-f0-9]+)\)$', line)

            if m == None:
                raise Exception('Invalid line {} in {}: {}'.format(i + 1, path, line))

            version = (int(m.group(1)), int(m.group(2)), int(m.group(3)))

            if version[0] not in [0, 1, 2]:
                raise Exception('Invalid major version in {}: {}'.format(path, version))

            if len(versions) > 0 and (version[2] < 90 and versions[-1][2] < 90) or  (version[2] >= 90 and versions[-1][2] >= 90):
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

            versions.append(version)

    if len(versions) == 0:
        raise Exception('No version found in {}'.format(path))

    oldest_version = (str(versions[0][0]), str(versions[0][1]), str(versions[0][2]))
    version = (str(versions[-1][0]), str(versions[-1][1]), str(versions[-1][2]))
    return oldest_version, version

def write_firmware_info(display_name, major, minor, patch, build_time):
    buf = bytearray([0xFF] * 4096)

    # 7121CE12F0126E
    # tink er for ge
    buf[0:7] = bytearray.fromhex("7121CE12F0126E") # magic
    buf[7] = 0x01 #firmware_info_version, note: a new version has to be backwards compatible

    name_bytes = display_name.encode("utf-8") # firmware name, max 60 chars
    buf[8:8 + len(name_bytes)] = name_bytes
    buf[8 + len(name_bytes):68] = bytes(60 - len(name_bytes))
    buf[68] = 0x00 # 0 byte to make sure string is terminated. also pads the fw version, so that the build date will be 4-byte aligned
    buf[69] = int(major)
    buf[70] = int(minor)
    buf[71] = int(patch)
    buf[72:76] = build_time.to_bytes(4, byteorder='little')
    buf[4092:4096] = crc32(buf[0:4092]).to_bytes(4, byteorder='little')

    with open(os.path.join(env.subst("$BUILD_DIR"), "firmware_info.bin"), "wb") as f:
        f.write(buf)

def generate_module_dependencies_header(info_path, header_path, backend_module, backend_modules, all_mods_upper):
    if backend_module:
        module_name = backend_module.space
    else:
        module_name = f'[{info_path}]'

    has_dependencies = False

    if os.path.exists(info_path):
        config = configparser.ConfigParser()
        config.read(info_path)
        if config.has_section('Dependencies'):
            has_dependencies = True
            required_mods = []
            available_optional_mods = []
            all_optional_mods_upper = []

            allow_nonexist = config['Dependencies'].getboolean('AllowNonexist', False)

            known_keys = set(['requires', 'optional', 'conflicts', 'after', 'before', 'modulelist'])
            unknown_keys = set(config['Dependencies'].keys()).difference(known_keys)
            if len(unknown_keys) > 0:
                print(f"Error: '{backend_module.under}/module.ini contains unknown keys {unknown_keys}  ", file=sys.stderr)
                sys.exit(1)

            requires = config['Dependencies'].get('Requires', "")
            requires = requires.splitlines()
            old_len = len(requires)
            requires = list(dict.fromkeys(requires))
            if len(requires) != old_len:
                print(f"List of required modules for module '{module_name}' contains duplicates.", file=sys.stderr)
            for req_name in requires:
                req_module, _ = find_backend_module_space(backend_modules, req_name)
                if not req_module:
                    if '_'.join(req_name.split(' ')).upper() in all_mods_upper:
                        print(f"Error: Module '{module_name}' requires module '{req_name}', which is available but not enabled for this environment.", file=sys.stderr)
                    else:
                        print(f"Error: Module '{module_name}' requires module '{req_name}', which does not exist.", file=sys.stderr)
                    sys.exit(1)
                required_mods.append(req_module)

            optional = config['Dependencies'].get('Optional')
            if optional is not None:
                optional = optional.splitlines()
                old_len = len(optional)
                optional = list(dict.fromkeys(optional))
                if len(optional) != old_len:
                    print(f"List of optional modules for module '{module_name}' contains duplicates.", file=sys.stderr)
                for opt_name in optional:
                    opt_name_upper = '_'.join(opt_name.split(' ')).upper()
                    opt_module, _ = find_backend_module_space(backend_modules, opt_name)
                    if not opt_module:
                        if not allow_nonexist and opt_name_upper not in all_mods_upper:
                            print(f"Error: Optional module '{opt_name}' wanted by module '{module_name}' does not exist.", file=sys.stderr)
                            sys.exit(1)
                    else:
                        if opt_module in required_mods:
                            print(f"Error: Optional module '{opt_name}' wanted by module '{module_name}' is already listed as required.", file=sys.stderr)
                            sys.exit(1)
                        available_optional_mods.append(opt_module)
                    all_optional_mods_upper.append(opt_name_upper)

            conflicts = config['Dependencies'].get('Conflicts')
            if conflicts is not None:
                conflicts = conflicts.splitlines()
                old_len = len(conflicts)
                conflicts = list(dict.fromkeys(conflicts))
                if len(conflicts) != old_len:
                    print(f"List of conflicting modules for module '{module_name}' contains duplicates.", file=sys.stderr)
                for conflict_name in conflicts:
                    conflict_module, _ = find_backend_module_space(backend_modules, conflict_name)
                    if conflict_module:
                        print(f"Error: Module '{module_name}' conflicts with module '{conflict_name}'.", file=sys.stderr)
                        sys.exit(1)

            if backend_module:
                cur_module_index = backend_modules.index(backend_module)

            after = config['Dependencies'].get('After')
            if after is not None:
                after = after.splitlines()
                old_len = len(after)
                after = list(dict.fromkeys(after))
                if len(after) != old_len:
                    print(f"List of 'After' modules for module '{module_name}' contains duplicates.", file=sys.stderr)
                for after_name in after:
                    _, index = find_backend_module_space(backend_modules, after_name)
                    if index < 0:
                        if not allow_nonexist and '_'.join(after_name.split(' ')).upper() not in all_mods_upper:
                            print(f"Error: Module '{after_name}' in 'After' list of module '{module_name}' does not exist.", file=sys.stderr)
                            sys.exit(1)
                    elif index > cur_module_index:
                        print(f"Error: Module '{module_name}' must be loaded after module '{after_name}'.", file=sys.stderr)
                        sys.exit(1)

            before = config['Dependencies'].get('Before')
            if before is not None:
                before = before.splitlines()
                old_len = len(before)
                before = list(dict.fromkeys(before))
                if len(before) != old_len:
                    print(f"List of 'Before' modules for module '{module_name}' contains duplicates.", file=sys.stderr)
                for before_name in before:
                    _, index = find_backend_module_space(backend_modules, before_name)
                    if index < 0:
                        if not allow_nonexist and '_'.join(before_name.split(' ')).upper() not in all_mods_upper:
                            print(f"Error: Module '{before_name}' in 'Before' list of module '{module_name}' does not exist.", file=sys.stderr)
                            sys.exit(1)
                    elif index < cur_module_index:
                        print(f"Error: Module '{module_name}' must be loaded before module '{before_name}'.", file=sys.stderr)
                        sys.exit(1)

            dep_mods = required_mods + available_optional_mods
            backend_mods_upper = [x.upper for x in backend_modules]

            defines  = ''.join(['#define MODULE_{}_AVAILABLE() {}\n'.format(x, "1" if x in backend_mods_upper else "0") for x in all_optional_mods_upper])
            includes = ''.join([f'#include "modules/{x.under}/{x.under}.h"\n' for x in dep_mods])
            decls    = ''.join([f'extern {x.camel} {x.under};\n' for x in dep_mods])

            header_content  = '// WARNING: This file is generated.\n\n'
            header_content += '#pragma once\n'

            if defines:
                header_content += '\n' + defines
            if includes:
                header_content += '\n' + includes
            if decls:
                header_content += '\n' + decls

            if config['Dependencies'].getboolean('ModuleList', False):
                header_content += '\n'
                header_content += '#include "config.h"\n'
                header_content += 'extern Config modules;\n'

            util.write_file_if_different(header_path, header_content)

    if not has_dependencies:
        try:
            os.remove(header_path)
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
                        raise Exception('.'.join(parent_key + [key]) + ' cannot override non-null translation')
                except KeyError:
                    print('Ignoring unused translation override', '.'.join(parent_key + [key]))
                    continue
            elif key in translation:
                raise Exception('.'.join(parent_key + [key]) + ' cannot replace existing translation')

            translation[key] = value
        else:
            update_translation(translation[key], value, override=override, parent_key=parent_key + [key])

TSX_HEADER = """/** @jsxImportSource preact */
import { h } from "preact";
let x = """

TSX_LINE_COMMENT_PATTERN = re.compile(r'^[ \t]*//.*$', re.MULTILINE)

TSX_FRAGMENT_PATTERN = re.compile(r'<>.*?</>', re.MULTILINE | re.DOTALL)
TSX_FUNCTION_PATTERN = re.compile(r'/\*[SF]FN\*/.*?/\*NF\*/', re.MULTILINE | re.DOTALL)

TSX_FUNCTION_ARGS_PATTERN = re.compile(r'FN\*/\s*\(([^\)]*)\)', re.MULTILINE | re.DOTALL)

TSX_JSON_REPLACEMENTS = [# Escape nested fragments in functions
    ('<>', '___START_FRAGMENT___'),
    ('</>', '___END_FRAGMENT___'),
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
                content = content.replace(TSX_HEADER, "", 1)
                content = re.sub(TSX_LINE_COMMENT_PATTERN, "", content)
                content = re.sub(TSX_FUNCTION_PATTERN,
                                 tsx_to_json,
                                 content)

                content = re.sub(TSX_FRAGMENT_PATTERN,
                                 tsx_to_json,
                                 content)
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

missing_hyphenations = []

def hyphenate(s, key):
    if '\u00AD' in s:
        print("Found unicode soft hyphen in translation value {}: {}".format(key, s.replace('\u00AD', "___HERE___")))
        sys.exit(1)

    # Replace longest words first. This prevents replacing parts of longer words.
    for word in sorted(re.split('\W+', s.replace("&shy;", "")), key=lambda x: len(x), reverse=True):
        for l, r in hyphenations:
            if word == l:
                s = s.replace(l, r)
                break
        else:
            is_too_long = len(word) > HYPHENATE_THRESHOLD
            is_camel_case = word[:1].islower() and not word[1:].islower()
            is_snake_case = "_" in word
            if is_too_long:# and not (is_camel_case or is_snake_case):
                missing_hyphenations.append(word)

    return s

def hyphenate_translation(translation, parent_key=None):
    if parent_key == None:
        parent_key = []

    return {key: (hyphenate(value, key) if isinstance(value, str) else hyphenate_translation(value, parent_key=parent_key + [key])) for key, value in translation.items()}

def repair_rtc_dir():
    path = os.path.abspath("src/modules/rtc")
    try:
        os.remove(path + "/real_time_clock_v2_bricklet_firmware_bin.digest")
        os.remove(path + "/real_time_clock_v2_bricklet_firmware_bin.embedded.cpp")
        os.remove(path + "/real_time_clock_v2_bricklet_firmware_bin.embedded.h")
    except:
        pass

def find_backend_module_space(backend_modules, name_space):
    index = 0
    for backend_module in backend_modules:
        if backend_module.space == name_space:
            return backend_module, index
        index += 1

    name_upper = '_'.join(name_space.split(' ')).upper()
    for backend_module in backend_modules:
        if backend_module.upper == name_upper:
            print(f"Error: Encountered incorrectly capitalized backend module '{name_space}'", file=sys.stderr)
            sys.exit(1)

    return None, -1

def find_branding_module(frontend_modules):
    branding_module = None

    for frontend_module in frontend_modules:
        mod_path = os.path.join('web', 'src', 'modules', frontend_module.under)

        potential_branding_path = os.path.join(mod_path, 'branding.ts')

        if os.path.exists(potential_branding_path):
            if branding_module != None:
                print('Error: Branding module collision ' + mod_path + ' vs ' + branding_module)
                sys.exit(1)

            branding_module = mod_path

    if branding_module is None:
        print('Error: No branding module selected')
        sys.exit(1)

    req_for_branding = ['branding.ts', 'logo.png', 'favicon.png']

    for f in req_for_branding:
        if not os.path.exists(os.path.join(branding_module, f)):
            print('Error: Branding module does not contain {}'.format(f))
            sys.exit(1)

    return branding_module

def main():
    if env.IsCleanTarget():
        return

    repair_rtc_dir()
    subprocess.check_call([env.subst('$PYTHONEXE'), "-u", "update_packages.py"])

    # Add build flags
    timestamp = int(time.time())
    name = env.GetProjectOption("custom_name")
    config_type = env.GetProjectOption("custom_config_type")
    host_prefix = env.GetProjectOption("custom_host_prefix")
    display_name = env.GetProjectOption("custom_display_name")
    manual_url = env.GetProjectOption("custom_manual_url")
    apidoc_url = env.GetProjectOption("custom_apidoc_url")
    firmware_url = env.GetProjectOption("custom_firmware_url")
    require_firmware_info = env.GetProjectOption("custom_require_firmware_info")
    build_flags = env.GetProjectOption("build_flags")
    frontend_debug = env.GetProjectOption("custom_frontend_debug") == "true"
    web_only = env.GetProjectOption("custom_web_only") == "true"
    web_build_flags = env.GetProjectOption("custom_web_build_flags")
    monitor_speed = env.GetProjectOption("monitor_speed")
    nightly = "-DNIGHTLY" in build_flags

    is_release = len(subprocess.run(["git", "tag", "--contains", "HEAD"], check=True, capture_output=True).stdout) > 0
    is_dirty = len(subprocess.run(["git", "diff"], check=True, capture_output=True).stdout) > 0
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
        oldest_version, version = get_changelog_version(name)
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
            with open(os.path.join(name + '_wifi.json'), 'r', encoding='utf-8') as f:
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

    env.Replace(BUILD_FLAGS=build_flags)

    write_firmware_info(display_name, *version, timestamp)

    build_lines = []
    build_lines.append('#pragma once')
    build_lines.append('#include <stdint.h>')
    build_lines.append('#define OLDEST_VERSION_MAJOR {}'.format(oldest_version[0]))
    build_lines.append('#define OLDEST_VERSION_MINOR {}'.format(oldest_version[1]))
    build_lines.append('#define OLDEST_VERSION_PATCH {}'.format(oldest_version[2]))
    build_lines.append('#define BUILD_VERSION_MAJOR {}'.format(version[0]))
    build_lines.append('#define BUILD_VERSION_MINOR {}'.format(version[1]))
    build_lines.append('#define BUILD_VERSION_PATCH {}'.format(version[2]))
    build_lines.append('#define BUILD_VERSION_STRING "{}.{}.{}"'.format(*version))
    build_lines.append('#define BUILD_HOST_PREFIX "{}"'.format(host_prefix))
    build_lines.append('#define BUILD_NAME_{}'.format(name.upper()))
    build_lines.append('#define BUILD_CONFIG_TYPE "{}"'.format(config_type))
    build_lines.append('#define BUILD_DISPLAY_NAME "{}"'.format(display_name))
    build_lines.append('#define BUILD_DISPLAY_NAME_UPPER "{}"'.format(display_name.upper()))
    build_lines.append('#define BUILD_REQUIRE_FIRMWARE_INFO {}'.format(require_firmware_info))
    build_lines.append('#define BUILD_MONITOR_SPEED {}'.format(monitor_speed))
    build_lines.append('uint32_t build_timestamp(void);')
    build_lines.append('const char *build_timestamp_hex_str(void);')
    build_lines.append('const char *build_version_full_str(void);')
    build_lines.append('const char *build_info_str(void);')
    build_lines.append('const char *build_filename_str(void);')
    build_lines.append('const char *build_commit_id_str(void);')
    util.write_file_if_different(os.path.join('src', 'build.h'), '\n'.join(build_lines))

    firmware_basename = '{}_firmware{}{}_{}_{:x}{}'.format(
        name,
        "-NIGHTLY" if nightly else "",
        "-WITH-WIFI-PASSPHRASE-DO-NOT-DISTRIBUTE" if not_for_distribution else "",
        '_'.join(version),
        timestamp,
        dirty_suffix,
    )

    build_lines = []
    build_lines.append('#include "build.h"')
    build_lines.append('uint32_t build_timestamp(void) {{ return {}; }}'.format(timestamp))
    build_lines.append('const char *build_timestamp_hex_str(void) {{ return "{:x}"; }}'.format(timestamp))
    build_lines.append('const char *build_version_full_str(void) {{ return "{}.{}.{}-{:x}"; }}'.format(*version, timestamp))
    build_lines.append('const char *build_info_str(void) {{ return "git url: {}, git branch: {}, git commit id: {}"; }}'.format(git_url, branch_name, git_commit_id))
    build_lines.append('const char *build_filename_str(void){{return "{}"; }}'.format(firmware_basename))
    build_lines.append('const char *build_commit_id_str(void){{return "{}"; }}'.format(git_commit_id))
    util.write_file_if_different(os.path.join('src', 'build.cpp'), '\n'.join(build_lines))
    del build_lines

    with open(os.path.join(env.subst('$BUILD_DIR'), 'firmware_basename'), 'w', encoding='utf-8') as f:
        f.write(firmware_basename)

    frontend_modules = [util.FlavoredName(x).get() for x in env.GetProjectOption("custom_frontend_modules").splitlines()]
    if nightly:
        frontend_modules.append(util.FlavoredName("Nightly").get())
        frontend_modules.append(util.FlavoredName("Debug").get())

    branding_module = find_branding_module(frontend_modules)

    metadata = json.dumps({
        'frontend_modules': [frontend_module.under for frontend_module in frontend_modules]
    }, separators=(',', ':'))

    web_build_lines = []
    for web_build_flag in web_build_flags.split('\n'):
        web_build_lines.append(f'export const {web_build_flag};')

    util.write_file_if_different(os.path.join('web', 'src', 'build.ts'), '\n'.join(web_build_lines))

    # Handle backend modules
    excluded_backend_modules = list(os.listdir('src/modules'))
    backend_modules = [util.FlavoredName(x).get() for x in env.GetProjectOption("custom_backend_modules").splitlines()]

    if nightly:
        backend_modules.append(util.FlavoredName("Debug").get())

    with ChangedDirectory('src'):
        excluded_bindings = [PurePath(x).as_posix() for x in glob.glob('bindings/brick_*') + glob.glob('bindings/bricklet_*')]

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
            print("Backend module {} not found.".format(backend_module.space, mod_path))

        for root, dirs, files in os.walk(mod_path):
            for name in files:
                include_bindings(os.path.join(root, name))

        excluded_backend_modules.remove(backend_module.under)

        if os.path.exists(os.path.join(mod_path, "prepare.py")):
            util.log('Preparing backend module:', backend_module.space)

            environ = dict(os.environ)
            environ['PLATFORMIO_PROJECT_DIR'] = env.subst('$PROJECT_DIR')
            environ['PLATFORMIO_BUILD_DIR'] = env.subst('$BUILD_DIR')
            environ['PLATFORMIO_METADATA'] = metadata

            abs_branding_module = os.path.abspath(branding_module)
            with ChangedDirectory(mod_path):
                subprocess.check_call([env.subst('$PYTHONEXE'), "-u", "prepare.py", abs_branding_module], env=environ)

    for root, dirs, files in os.walk('src'):
        root_path = PurePath(root)
        root_parents = [root_path] + list(root_path.parents)

        if PurePath('src', 'bindings') in root_parents or PurePath('src', 'modules') in root_parents:
            continue

        for name in files:
            include_bindings(os.path.join(root, name))

    for excluded_backend_module in excluded_backend_modules:
        build_src_filter.append('-<modules/{0}/*>'.format(excluded_backend_module))

    for excluded_binding in excluded_bindings:
        build_src_filter.append('-<{0}>'.format(excluded_binding))

    env.Replace(SRC_FILTER=[' '.join(build_src_filter)])

    all_mods = []
    for existing_backend_module in os.listdir(os.path.join('src', 'modules')):
        if not os.path.isdir(os.path.join('src', 'modules', existing_backend_module)):
            continue

        all_mods.append(existing_backend_module.upper())

    backend_mods_upper = [x.upper for x in backend_modules]

    util.specialize_template("modules.h.template", os.path.join("src", "modules.h"), {
        '{{{module_includes}}}': '\n'.join(['#include "modules/{0}/{0}.h"'.format(x.under) for x in backend_modules]),
        '{{{module_defines}}}': '\n'.join(['#define MODULE_{}_AVAILABLE() {}'.format(x, "1" if x in backend_mods_upper else "0") for x in all_mods]),
        '{{{module_extern_decls}}}': '\n'.join(['extern {} {};'.format(x.camel, x.under) for x in backend_modules]),
    })

    util.specialize_template("modules.cpp.template", os.path.join("src", "modules.cpp"), {
        '{{{module_decls}}}': '\n'.join(['{} {};'.format(x.camel, x.under) for x in backend_modules]),
        '{{{imodule_count}}}': str(len(backend_modules)),
        '{{{imodule_vector}}}': '\n    '.join(['imodules->push_back(&{});'.format(x.under) for x in backend_modules]),
        '{{{module_init_config}}}': ',\n        '.join('{{"{0}", Config::Bool({0}.initialized)}}'.format(x.under) for x in backend_modules if not x.under.startswith("hidden_")),
    })

    util.log("Generating module_dependencies.h from module.ini", flush=True)
    generate_module_dependencies_header('src/event_log_dependencies.ini', 'src/event_log_dependencies.h', None, backend_modules, all_mods)
    generate_module_dependencies_header('src/web_dependencies.ini', 'src/web_dependencies.h', None, backend_modules, all_mods)
    for backend_module in backend_modules:
        mod_path = os.path.join('src', 'modules', backend_module.under)
        info_path = os.path.join(mod_path, 'module.ini')
        header_path = os.path.join(mod_path, 'module_dependencies.h')
        generate_module_dependencies_header(info_path, header_path, backend_module, backend_modules, all_mods)

    # Handle frontend modules
    navbar_entries = []
    content_entries = []
    status_entries = []
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

            abs_branding_module = os.path.abspath(branding_module)
            with ChangedDirectory(mod_path):
                subprocess.check_call([env.subst('$PYTHONEXE'), "-u", "prepare.py", abs_branding_module], env=environ)

        if os.path.exists(os.path.join(mod_path, 'navbar.html')):
            with open(os.path.join(mod_path, 'navbar.html'), 'r', encoding='utf-8') as f:
                navbar_entries.append(f.read())

        if os.path.exists(os.path.join(mod_path, 'content.html')):
            with open(os.path.join(mod_path, 'content.html'), 'r', encoding='utf-8') as f:
                content_entries.append(f.read())

        if os.path.exists(os.path.join(mod_path, 'status.html')):
            with open(os.path.join(mod_path, 'status.html'), 'r', encoding='utf-8') as f:
                status_entries.append(f.read())

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
                api_cache_entries.append("'{}{}': null as any,".format(api_path, api_suffix))
                api_cache_entries.append("'{}{}_modified': null as any,".format(api_path, api_suffix))

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

    global missing_hyphenations
    missing_hyphenations = sorted(set(missing_hyphenations) - allowed_missing)
    missing_hyphenations = [x for x in missing_hyphenations if not x.startswith("___START_FRAGMENT___") and not x.endswith("___END_FRAGMENT___")]
    if len(missing_hyphenations) > 0:
        print("Missing hyphenations detected. Add those to hyphenations.py!")
        for x in missing_hyphenations:
            print("    {}".format(x))

    for path in glob.glob(os.path.join('web', 'src', 'ts', 'translation_*.ts')):
        os.remove(path)

    if len(translation) == 0:
        print('Error: Translation missing')
        sys.exit(1)

    translation_data = json.dumps(translation, indent=4, ensure_ascii=False)
    translation_data = translation_data.replace('{{{display_name}}}', display_name)
    translation_data = translation_data.replace('{{{manual_url}}}', manual_url)
    translation_data = translation_data.replace('{{{apidoc_url}}}', apidoc_url)
    translation_data = translation_data.replace('{{{firmware_url}}}', firmware_url)
    util.write_file_if_different(os.path.join('web', 'src', 'ts', 'translation.json'), translation_data)
    del translation_data

    with open(os.path.join(branding_module, 'favicon.png'), 'rb') as f:
        favicon = b64encode(f.read()).decode('ascii')

    with open(os.path.join(branding_module, 'logo.png'), 'rb') as f:
        logo_base64 = b64encode(f.read()).decode('ascii')

    with open(os.path.join(branding_module, 'branding.ts'), 'r', encoding='utf-8') as f:
        branding = f.read()

    with open(os.path.join(branding_module, 'pre.scss'), 'r', encoding='utf-8') as f:
        color = f.read().split('\n')[1].split(' ')[1]
        if color.endswith(';'):
            color = color[:-1]

    util.specialize_template(os.path.join("web", "index.html.template"), os.path.join("web", "src", "index.html"), {
        '{{{favicon}}}': favicon,
        '{{{logo_base64}}}': logo_base64,
        '{{{navbar}}}': '\n                        '.join(navbar_entries),
        '{{{content}}}': '\n                    '.join(content_entries),
        '{{{status}}}': '\n                            '.join(status_entries),
        '{{{theme_color}}}': color
    })

    util.specialize_template(os.path.join("web", "main.tsx.template"), os.path.join("web", "src", "main.tsx"), {
        '{{{module_imports}}}': '\n'.join(['import * as {0} from "./modules/{0}/main";'.format(x) for x in main_ts_entries]),
        '{{{modules}}}': ', '.join([x for x in main_ts_entries]),
        '{{{preact_debug}}}': 'import "preact/debug";' if frontend_debug else ''
    })

    util.specialize_template(os.path.join("web", "main.scss.template"), os.path.join("web", "src", "main.scss"), {
        '{{{module_pre_imports}}}': '\n'.join(['@import "{0}";'.format(x.replace('\\', '/')) for x in pre_scss_paths]),
        '{{{module_post_imports}}}': '\n'.join(['@import "{0}";'.format(x.replace('\\', '/')) for x in post_scss_paths])
    })

    util.specialize_template(os.path.join("web", "api_defs.ts.template"), os.path.join("web", "src", "ts", "api_defs.ts"), {
        '{{{imports}}}': '\n'.join(api_imports),
        '{{{module_interface}}}': ',\n    '.join('{}: boolean'.format(x.under) for x in backend_modules),
        '{{{config_map_entries}}}': '\n    '.join(api_config_map_entries),
        '{{{api_cache_entries}}}': '\n    '.join(api_cache_entries),
    })

    util.specialize_template(os.path.join("web", "branding.ts.template"), os.path.join("web", "src", "ts", "branding.ts"), {
        '{{{logo_base64}}}': logo_base64,
        '{{{branding}}}': branding,
    })

    translation_str = ''

    def format_translation(translation, type_only, indent):
        output = ['{\n']

        assert isinstance(translation, (dict, str)), type(translation)

        for key, value in sorted(translation.items()):
            output += [indent + '    ', key, ': ']

            if isinstance(value, dict):
                output += format_translation(value, type_only, indent + '    ')
            else:
                is_fragment = value.startswith("___START_FRAGMENT___") and value.endswith("___END_FRAGMENT___")
                is_string_function = value.startswith("/*SFN*/") and value.endswith("/*NF*/")
                is_fragment_function = value.startswith("/*FFN*/") and value.endswith("/*NF*/")
                is_string = not is_fragment and not is_string_function and not is_fragment_function

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
                    else:
                        string = '"{0}"'.format(value.replace('"', '\\"'))

                    if is_string or is_string_function:
                        if match := re.search(r"<[^>]*>", value):
                            print("Found HTML tag {} in non-fragment value {}".format(match.group(0), value))

                    if '{{{' in string:
                        string = string.replace('{{{display_name}}}', display_name)
                        string = string.replace('{{{manual_url}}}', manual_url)
                        string = string.replace('{{{apidoc_url}}}', apidoc_url)
                        string = string.replace('{{{firmware_url}}}', firmware_url)

                    output += [string, ',\n']

        output += [indent, '}']

        if len(indent) > 0:
            output += [',\n']

        return output

    translation_str += 'type Translation = ' + ''.join(format_translation(translation['en'], True, '')) + '\n\n'

    for language in sorted(translation):
        translation_str += 'const translation_{0}: Translation = {1} as const\n\n'.format(language, ''.join(format_translation(translation[language], False, '')))

    translation_str += 'const translation: {[index: string]: Translation} = {\n'

    for language in sorted(translation):
        translation_str += '    "{0}": translation_{0},\n'.format(language)

    translation_str += '} as const\n'

    util.specialize_template(os.path.join("web", "translation.tsx.template"), os.path.join("web", "src", "ts", "translation.tsx"), {
        '{{{translation}}}': translation_str,
    })

    # Check translation completeness
    util.log('Checking translation completeness')

    with ChangedDirectory('web'):
        subprocess.check_call([env.subst('$PYTHONEXE'), "-u", "check_translation_completeness.py"] + [x.under for x in frontend_modules])

    # Check translation override completeness
    util.log('Checking translation override completeness')

    with ChangedDirectory('web'):
        subprocess.check_call([env.subst('$PYTHONEXE'), "-u", "check_override_completeness.py"])

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

        with ChangedDirectory('web'):
            npm_version = subprocess.check_output(['npm', '--version'], shell=sys.platform == 'win32', encoding='utf-8').strip()

            m = re.fullmatch(r'(\d+)\.\d+\.\d+', npm_version)

            if m == None:
                print('Error: npm version has unexpected format: {0}'.format(npm_version))
                sys.exit(1)

            if int(m.group(1)) < 8:
                print('Error: npm >= 8 required, found npm {0}'.format(npm_version))
                sys.exit(1)

            subprocess.check_call(['npm', 'ci'], shell=sys.platform == 'win32')

        with open('web/node_modules/tinkerforge.marker', 'wb') as f:
            pass

        util.store_digest(node_modules_digest, 'web', 'node_modules', env=env)

    util.log('Checking web interface')

    index_html_src_paths = []
    index_html_src_datas = []

    for name in sorted(os.listdir('web')):
        path = os.path.join('web', name)

        if os.path.isfile(path):
            index_html_src_paths.append(path)

    for root, dirs, files in sorted(os.walk('web/src')):
        for name in sorted(files):
            index_html_src_paths.append(os.path.join(root, name))

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

        with ChangedDirectory('web'):
            try:
                subprocess.check_call([env.subst('$PYTHONEXE'), "-u", "build.py"] + ([] if not frontend_debug else ['--js-source-map', '--css-source-map']))
            except subprocess.CalledProcessError as e:
                if e.returncode != 42:
                    print(e, file=sys.stderr)
                sys.exit(1)

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

        util.embed_data(util.gzip_compress(html_bytes), 'src', 'index_html', 'char')
        util.store_digest(index_html_digest, 'src', 'index_html', env=env)

    util.log("Checking HTML ID usage")
    with ChangedDirectory('web'):
        subprocess.check_call([env.subst('$PYTHONEXE'), "-u", "check_id_usage.py"] + [x.under for x in frontend_modules])

    if web_only:
        print('Stopping build after web')
        sys.exit(0)

main()
