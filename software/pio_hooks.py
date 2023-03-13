Import("env")

import sys

if sys.hexversion < 0x3060000:
    print('Error: Python >= 3.6 required')
    sys.exit(1)

from collections import namedtuple
import functools
import os
import shutil
import subprocess
import time
import re
import json
import glob
import io
import gzip
from base64 import b64encode
from zlib import crc32
import util

from hyphenations import hyphenations, allowed_missing

NameFlavors = namedtuple('NameFlavors', 'space lower camel headless under upper dash camel_abbrv lower_no_space camel_constant_safe')

class FlavoredName(object):
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

def specialize_template(template_filename, destination_filename, replacements, check_completeness=True, remove_template=False):
    lines = []
    replaced = set()

    with open(template_filename, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            for key in replacements:
                replaced_line = line.replace(key, replacements[key])

                if replaced_line != line:
                    replaced.add(key)

                line = replaced_line

            lines.append(line)

    if check_completeness and replaced != set(replacements.keys()):
        raise Exception('Not all replacements for {0} have been applied. Missing are {1}'.format(template_filename, ', '.join(set(replacements.keys() - replaced))))

    with open(destination_filename, 'w', encoding='utf-8') as f:
        f.writelines(lines)

    if remove_template:
        os.remove(template_filename)

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

def collect_translation(path, override=False):
    translation = {}

    for translation_path in glob.glob(os.path.join(path, 'translation_*.json')):
        m = re.match(r'translation_([a-z]+){0}\.json'.format('_override' if override else ''), os.path.split(translation_path)[-1])

        if m == None:
            continue

        language = m.group(1)

        with open(translation_path, 'r', encoding='utf-8') as f:
            try:
                translation[language] = json.loads(f.read())
            except:
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

def hyphenate(s):
    # Replace longest words first. This prevents replacing parts of longer words.
    for word in sorted(re.split('\W+', s.replace("&shy;", "")), key=lambda x: len(x), reverse=True):
        for l, r in hyphenations:
            if word == l:
                s = s.replace(l, r)
                break
        else:
            if len(word) > HYPHENATE_THRESHOLD:
                missing_hyphenations.append(word)

    return s

def hyphenate_translation(translation, parent_key=None):
    if parent_key == None:
        parent_key = []

    return {key: (hyphenate(value) if isinstance(value, str) else hyphenate_translation(value, parent_key=parent_key + [key])) for key, value in translation.items()}

def main():
    if env.IsCleanTarget():
        return

    subprocess.check_call([env.subst('$PYTHONEXE'), "-u", "update_packages.py"])

    # Add build flags
    timestamp = int(time.time())
    name = env.GetProjectOption("custom_name")
    host_prefix = env.GetProjectOption("custom_host_prefix")
    display_name = env.GetProjectOption("custom_display_name")
    manual_url = env.GetProjectOption("custom_manual_url")
    apidoc_url = env.GetProjectOption("custom_apidoc_url")
    firmware_url = env.GetProjectOption("custom_firmware_url")
    require_firmware_info = env.GetProjectOption("custom_require_firmware_info")
    build_flags = env.GetProjectOption("build_flags")
    frontend_debug = env.GetProjectOption("custom_frontend_debug") == "true"
    web_only = env.GetProjectOption("custom_web_only") == "true"

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

    build_src_filter = ['+<*>']

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

    with open(os.path.join('src', 'build.h'), 'w', encoding='utf-8') as f:
        f.write('#pragma once\n')
        f.write('#include <stdint.h>\n')
        f.write('#define OLDEST_VERSION_MAJOR {}\n'.format(oldest_version[0]))
        f.write('#define OLDEST_VERSION_MINOR {}\n'.format(oldest_version[1]))
        f.write('#define OLDEST_VERSION_PATCH {}\n'.format(oldest_version[2]))
        f.write('#define BUILD_VERSION_MAJOR {}\n'.format(version[0]))
        f.write('#define BUILD_VERSION_MINOR {}\n'.format(version[1]))
        f.write('#define BUILD_VERSION_PATCH {}\n'.format(version[2]))
        f.write('#define BUILD_VERSION_STRING "{}.{}.{}"\n'.format(*version))
        f.write('#define BUILD_HOST_PREFIX "{}"\n'.format(host_prefix))
        f.write('#define BUILD_NAME_{}\n'.format(name.upper()))
        f.write('#define BUILD_DISPLAY_NAME "{}"\n'.format(display_name))
        f.write('#define BUILD_REQUIRE_FIRMWARE_INFO {}\n'.format(require_firmware_info))
        f.write('uint32_t build_timestamp(void);\n')
        f.write('const char *build_timestamp_hex_str(void);\n')
        f.write('const char *build_version_full_str(void);\n')
        f.write('const char *build_info_str(void);\n')

    with open(os.path.join('src', 'build.cpp'), 'w', encoding='utf-8') as f:
        f.write('#include "build.h"\n')
        f.write('uint32_t build_timestamp(void) {{ return {}; }}\n'.format(timestamp))
        f.write('const char *build_timestamp_hex_str(void) {{ return "{:x}"; }}\n'.format(timestamp))
        f.write('const char *build_version_full_str(void) {{ return "{}.{}.{}-{:x}"; }}\n'.format(*version, timestamp))
        f.write('const char *build_info_str(void) {{ return "git url: {}, git branch: {}, git commit id: {}"; }}\n'.format(git_url, branch_name, git_commit_id))

    firmware_basename = '{}_firmware{}_{}_{:x}{}'.format(
        name,
        "-WITH-WIFI-PASSPHRASE-DO-NOT-DISTRIBUTE" if not_for_distribution else "",
        '_'.join(version),
        timestamp,
        dirty_suffix,
    )

    with open(os.path.join(env.subst('$BUILD_DIR'), 'firmware_basename'), 'w', encoding='utf-8') as f:
        f.write(firmware_basename)

    # Handle backend modules
    excluded_backend_modules = list(os.listdir('src/modules'))
    backend_modules = [FlavoredName(x).get() for x in env.GetProjectOption("custom_backend_modules").splitlines()]
    for backend_module in backend_modules:
        mod_path = os.path.join('src', 'modules', backend_module.under)

        if not os.path.exists(mod_path) or not os.path.isdir(mod_path):
            print("Backend module {} not found.".format(backend_module.space, mod_path))

        excluded_backend_modules.remove(backend_module.under)

        if os.path.exists(os.path.join(mod_path, "prepare.py")):
            print('Preparing backend module:', backend_module.space)

            environ = dict(os.environ)
            environ['PLATFORMIO_PROJECT_DIR'] = env.subst('$PROJECT_DIR')
            environ['PLATFORMIO_BUILD_DIR'] = env.subst('$BUILD_DIR')

            with ChangedDirectory(mod_path):
                subprocess.check_call([env.subst('$PYTHONEXE'), "-u", "prepare.py"], env=environ)

    if build_src_filter != None:
        for excluded_backend_module in excluded_backend_modules:
            build_src_filter.append('-<modules/{0}/*>'.format(excluded_backend_module))

        env.Replace(SRC_FILTER=[' '.join(build_src_filter)])

    specialize_template("main.cpp.template", os.path.join("src", "main.cpp"), {
        '{{{module_includes}}}': '\n'.join(['#include "modules/{0}/{0}.h"'.format(x.under) for x in backend_modules]),
        '{{{module_decls}}}': '\n'.join(['{} {};'.format(x.camel, x.under) for x in backend_modules]),
        '{{{module_pre_setup}}}': '\n    '.join(['{}.pre_setup();'.format(x.under) for x in backend_modules]),
        '{{{module_setup}}}': '\n    '.join(['{}.setup();'.format(x.under) for x in backend_modules]),
        '{{{module_register_urls}}}': '\n    '.join(['{}.register_urls();'.format(x.under) for x in backend_modules]),
        '{{{module_loop}}}': '\n    '.join(['{}.loop();'.format(x.under) for x in backend_modules]),
        '{{{display_name}}}': display_name,
        '{{{display_name_upper}}}': display_name.upper(),
        '{{{module_init_config}}}': ',\n        '.join('{{"{0}", Config::Bool({0}.initialized)}}'.format(x.under) for x in backend_modules if not x.under.startswith("hidden_"))
    })


    all_mods = []
    for existing_backend_module in os.listdir(os.path.join('src', 'modules')):
        if not os.path.isdir(os.path.join('src', 'modules', existing_backend_module)):
            continue

        all_mods.append(existing_backend_module.upper())

    backend_mods_upper = [x.upper for x in backend_modules]

    specialize_template("modules.h.template", os.path.join("src", "modules.h"), {
        '{{{module_includes}}}': '\n'.join(['#include "modules/{0}/{0}.h"'.format(x.under) for x in backend_modules]),
        '{{{module_defines}}}': '\n'.join(['#define MODULE_{}_AVAILABLE() {}'.format(x, "1" if x in backend_mods_upper else "0") for x in all_mods]),
        '{{{module_extern_decls}}}': '\n'.join(['extern {} {};'.format(x.camel, x.under) for x in backend_modules]),
    })

    # Handle frontend modules
    navbar_entries = []
    content_entries = []
    status_entries = []
    main_ts_entries = []
    pre_scss_paths = []
    post_scss_paths = []
    frontend_modules = [FlavoredName(x).get() for x in env.GetProjectOption("custom_frontend_modules").splitlines()]
    translation = collect_translation('web')

    # API
    api_imports = []
    api_config_map_entries = []
    api_cache_entries = []
    module_counter = 0
    exported_interface_pattern = re.compile("export interface ([A-Za-z0-9$_]+)")
    exported_type_pattern = re.compile("export type ([A-Za-z0-9$_]+)")
    api_path_pattern = re.compile("//APIPath:([^\n]*)\n")

    favicon_path = None
    logo_path = None
    branding_path = None

    for frontend_module in frontend_modules:
        mod_path = os.path.join('web', 'src', 'modules', frontend_module.under)

        if not os.path.exists(mod_path) or not os.path.isdir(mod_path):
            print("Error: Frontend module {} not found.".format(frontend_module.space, mod_path))
            sys.exit(1)

        potential_favicon_path = os.path.join(mod_path, 'favicon.png')

        if os.path.exists(potential_favicon_path):
            if favicon_path != None:
                print('Error: Favicon path collision ' + potential_favicon_path + ' vs ' + favicon_path)
                sys.exit(1)

            favicon_path = potential_favicon_path

        potential_logo_path = os.path.join(mod_path, 'logo.png')

        if os.path.exists(potential_logo_path):
            if logo_path != None:
                print('Error: Logo path collision ' + potential_logo_path + ' vs ' + logo_path)
                sys.exit(1)

            logo_path = potential_logo_path

        potential_branding_path = os.path.join(mod_path, 'branding.ts')

        if os.path.exists(potential_branding_path):
            if branding_path != None:
                print('Error: Branding path collision ' + potential_branding_path + ' vs ' + branding_path)
                sys.exit(1)

            branding_path = potential_branding_path

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
                content = f.read()

            api_path = frontend_module.under + "/"

            api_match = api_path_pattern.match(content)
            if api_match is not None:
                api_path = api_match.group(1).strip()

            api_exports = exported_interface_pattern.findall(content) + exported_type_pattern.findall(content)
            if len(api_exports) != 0:
                api_module = "module_{}".format(module_counter)
                module_counter += 1

                api_imports.append("import * as {} from '../modules/{}/api';".format(api_module, frontend_module.under))

                api_config_map_entries += ["'{}{}': {}.{},".format(api_path, x, api_module, x) for x in api_exports]
                api_config_map_entries += ["'{}{}_modified': ConfigModified,".format(api_path, x, api_module, x) for x in api_exports]
                api_cache_entries += ["'{}{}': null as any,".format(api_path, x) for x in api_exports]
                api_cache_entries += ["'{}{}_modified': null as any,".format(api_path, x) for x in api_exports]

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
    if len(missing_hyphenations) > 0:
        print("Missing hyphenations detected. Add those to hyphenations.py!")
        for x in missing_hyphenations:
            print("    {}".format(x))

    for path in glob.glob(os.path.join('web', 'src', 'ts', 'translation_*.ts')):
        os.remove(path)

    if len(translation) == 0:
        print('Error: Translation missing')
        sys.exit(1)

    with open(os.path.join('web', 'src', 'ts', 'translation.json'), 'w', encoding='utf-8') as f:
        data = json.dumps(translation, indent=4, ensure_ascii=False)
        data = data.replace('{{{display_name}}}', display_name)
        data = data.replace('{{{manual_url}}}', manual_url)
        data = data.replace('{{{apidoc_url}}}', apidoc_url)
        data = data.replace('{{{firmware_url}}}', firmware_url)

        f.write(data)

    if favicon_path == None:
        print('Error: Favicon missing')
        sys.exit(1)

    with open(favicon_path, 'rb') as f:
        favicon = b64encode(f.read()).decode('ascii')

    if logo_path == None:
        print('Error: Logo missing')
        sys.exit(1)

    if branding_path == None:
        print('Error: Branding missing')
        sys.exit(1)

    with open(logo_path, 'rb') as f:
        logo_base64 = b64encode(f.read()).decode('ascii')

    with open(branding_path, 'r', encoding='utf-8') as f:
        branding = f.read()

    specialize_template(os.path.join("web", "index.html.template"), os.path.join("web", "src", "index.html"), {
        '{{{favicon}}}': favicon,
        '{{{logo_base64}}}': logo_base64,
        '{{{navbar}}}': '\n                        '.join(navbar_entries),
        '{{{content}}}': '\n                    '.join(content_entries),
        '{{{status}}}': '\n                            '.join(status_entries)
    })

    specialize_template(os.path.join("web", "main.ts.template"), os.path.join("web", "src", "main.ts"), {
        '{{{module_imports}}}': '\n'.join(['import * as {0} from "./modules/{0}/main";'.format(x) for x in main_ts_entries]),
        '{{{modules}}}': ', '.join([x for x in main_ts_entries]),
        '{{{preact_debug}}}': 'import "preact/debug";' if frontend_debug else ''
    })

    specialize_template(os.path.join("web", "main.scss.template"), os.path.join("web", "src", "main.scss"), {
        '{{{module_pre_imports}}}': '\n'.join(['@import "{0}";'.format(x.replace('\\', '/')) for x in pre_scss_paths]),
        '{{{module_post_imports}}}': '\n'.join(['@import "{0}";'.format(x.replace('\\', '/')) for x in post_scss_paths])
    })

    specialize_template(os.path.join("web", "api_defs.ts.template"), os.path.join("web", "src", "ts", "api_defs.ts"), {
        '{{{imports}}}': '\n'.join(api_imports),
        '{{{module_interface}}}': ',\n    '.join('{}: boolean'.format(x.under) for x in backend_modules),
        '{{{config_map_entries}}}': '\n    '.join(api_config_map_entries),
        '{{{api_cache_entries}}}': '\n    '.join(api_cache_entries),
    })

    specialize_template(os.path.join("web", "branding.ts.template"), os.path.join("web", "src", "ts", "branding.ts"), {
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
            elif type_only:
                output += ['string,\n']
            else:
                string = '"{0}"'.format(value.replace('"', '\\"'))

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

    specialize_template(os.path.join("web", "translation.tsx.template"), os.path.join("web", "src", "ts", "translation.tsx"), {
        '{{{translation}}}': translation_str,
    })

    # Check translation completeness
    print('Checking translation completeness')

    with ChangedDirectory('web'):
        subprocess.check_call([env.subst('$PYTHONEXE'), "-u", "check_translation_completeness.py"] + [x.under for x in frontend_modules])

    # Check translation override completeness
    print('Checking translation override completeness')

    with ChangedDirectory('web'):
        subprocess.check_call([env.subst('$PYTHONEXE'), "-u", "check_override_completeness.py"])

    # Generate web interface
    print('Checking web interface dependencies')

    node_modules_src_paths = ['web/package-lock.json']

    # FIXME: Scons runs this script using exec(), resulting in __file__ being not available
    #node_modules_src_paths.append(__file__)

    node_modules_needs_update, node_modules_reason, node_modules_digest = util.check_digest(node_modules_src_paths, [], 'web', 'node_modules', env=env)
    node_modules_digest_paths = util.get_digest_paths('web', 'node_modules', env=env)

    if not node_modules_needs_update and os.path.exists('web/node_modules/tinkerforge.marker'):
        print('Web interface dependencies are up-to-date')
    else:
        if not os.path.exists('web/node_modules/tinkerforge.marker'):
            node_modules_reason = 'marker file missing'

        print('Web interface dependencies are not up-to-date ({0}), updating now'.format(node_modules_reason))

        util.remove_digest('web', 'node_modules', env=env)

        rmtree_tries = 10

        while rmtree_tries > 0:
            try:
                shutil.rmtree('web/node_modules')
                break
            except FileNotFoundError:
                break
            except:
                # on windows for some unknown reason sometimes a directory stays
                # or becomes non-empty during the shutil.rmtree call and and
                # cannot be removed anymore. if that happens jus try again
                time.sleep(0.5)

                rmtree_tries -= 1

                if rmtree_tries == 0:
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

    print('Checking web interface')

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
        print('Web interface is up-to-date')
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
            subprocess.check_call([env.subst('$PYTHONEXE'), "-u", "build.py"] + ([] if not frontend_debug else ['--js-source-map', '--css-source-map']))

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

        util.embed_data(gzip.compress(html_bytes), 'src', 'index_html', 'char')
        util.store_digest(index_html_digest, 'src', 'index_html', env=env)

    print("Checking HTML ID usage")
    with ChangedDirectory('web'):
        subprocess.check_call([env.subst('$PYTHONEXE'), "-u", "check_id_usage.py"] + [x.under for x in frontend_modules])

    if web_only:
        print('Stopping build after web')
        sys.exit(0)

main()
