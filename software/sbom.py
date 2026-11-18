#!/usr/bin/env -S uv run --script

# Generates a CycloneDX SBOM for a firmware build.
#
# PlatformIO loads this file as post extra script. After each firmware build
# it writes build/<firmware_basename>.cdx.json. It can also be run manually:
#
#   ./sbom.py <pio env> [-o output.cdx.json]
#
# Sources of information:
# - $BUILD_DIR/metadata.json: product, version and git information (pio_hooks.py)
# - $BUILD_DIR/sbom_inputs.json: libraries found by PlatformIO's LDF and the
#   web interface's npm packages, written after each build
# - $BUILD_DIR/firmware.map: archives and objects that were actually linked
# - packages/esp32-arduino-libs#*: versions of ESP-IDF and its managed components
# - ESP-IDF's own SBOM information about its third-party components
#   (.gitmodules sbom-* entries and sbom.yml manifests), downloaded from
#   GitHub once per ESP-IDF version and cached in .pio/sbom/

import os
import re
import sys
import json
import glob
import uuid
import base64
import hashlib
import shutil
import zipfile
import argparse
import traceback
import subprocess
import urllib.error
import urllib.request
from datetime import datetime, timezone

INPUTS_FILENAME = 'sbom_inputs.json'

TINKERFORGE = {
    'name': 'Tinkerforge GmbH',
    'url': ['https://www.tinkerforge.com'],
}

ESPRESSIF = {
    'name': 'Espressif Systems (Shanghai) CO LTD',
    'url': ['https://www.espressif.com'],
}

FIRMWARE_LICENSE = 'LGPL-2.1-or-later'

# ESP-IDF archives that contain third-party code. The path refers to an
# entry of the "submodules" or "manifests" dict of sbom_data/esp-idf-*.json.
IDF_THIRD_PARTY = [
    # archives, kind, path, fallback name, license
    (['freertos'], 'manifest', 'components/freertos/FreeRTOS-Kernel/sbom.yml', 'freertos', 'MIT'),
    (['lwip'], 'submodule', 'components/lwip/lwip', 'lwip', 'BSD-3-Clause'),
    (['mbedtls', 'mbedtls_2', 'mbedcrypto', 'mbedx509', 'p256m', 'everest'], 'submodule', 'components/mbedtls/mbedtls', 'mbedtls', 'Apache-2.0 OR GPL-2.0-or-later'),
    (['mqtt'], 'submodule', 'components/mqtt/esp-mqtt', 'esp-mqtt', 'Apache-2.0'),
    (['wpa_supplicant'], 'manifest', 'components/wpa_supplicant/sbom.yml', 'wpa_supplicant', 'BSD-3-Clause'),
    (['http_parser'], 'manifest', 'components/http_parser/sbom.yml', 'http_parser', 'MIT'),
    (['spiffs'], 'submodule', 'components/spiffs/spiffs', 'spiffs', 'MIT'),
    (['json'], 'submodule', 'components/json/cJSON', 'cJSON', 'MIT'),
    (['protobuf-c'], 'submodule', 'components/protobuf-c/protobuf-c', 'protobuf-c', 'BSD-2-Clause'),
    (['fatfs'], 'manifest', 'components/fatfs/sbom.yml', 'FatFs', 'FatFs License'),
    (['rt'], 'manifest', 'components/rt/sbom.yml', 'FreeRTOS-Plus-POSIX', 'MIT'),
    (['console'], 'manifest', 'components/console/argtable3/sbom.yml', 'argtable3', 'BSD-3-Clause'),
    (['heap'], 'submodule', 'components/heap/tlsf', 'tlsf', 'BSD-3-Clause'),
    (['core', 'net80211', 'pp', 'mesh', 'smartconfig', 'espnow', 'wapi'], 'submodule', 'components/esp_wifi/lib', 'esp32-wifi-lib', 'Apache-2.0'),
    (['phy', 'rtc'], 'submodule', 'components/esp_phy/lib', 'esp-phy-lib', 'Apache-2.0'),
    (['coexist'], 'submodule', 'components/esp_coex/lib', 'esp-coex-lib', 'Apache-2.0'),
    (['unity'], 'submodule', 'components/unity/unity', 'unity', 'MIT'),
    (['cmock'], 'submodule', 'components/cmock/CMock', 'CMock', 'MIT'),
]

# ESP-IDF managed components (from the component registry) that are part of the esp32-arduino-libs packages
IDF_MANAGED_COMPONENTS = {
    'espressif/mdns': {
        'supplier': ESPRESSIF,
        'license': 'Apache-2.0',
        'url': 'https://github.com/espressif/esp-protocols/tree/master/components/mdns',
        'description': 'mDNS service for ESP-IDF',
    },
    'espressif/libsodium': {
        'supplier': ESPRESSIF,
        'license': 'ISC',
        'url': 'https://github.com/espressif/idf-extra-components/tree/master/libsodium',
        'description': 'libsodium port to ESP-IDF',
        'upstream': {
            # component version -> upstream version
            'versions': {'1.0.22~1': '1.0.22'},
            'name': 'libsodium',
            'supplier': {'name': 'Frank Denis'},
            'license': 'ISC',
            'url': 'https://github.com/jedisct1/libsodium',
            'purl': 'pkg:github/jedisct1/libsodium@{}',
            'cpe': 'cpe:2.3:a:jedisct1:libsodium:{}:*:*:*:*:*:*:*',
        },
    },
    'joltwallet/littlefs': {
        'supplier': {'name': 'Brian Pugh'},
        'license': 'MIT',
        'url': 'https://github.com/joltwallet/esp_littlefs',
        'description': 'LittleFS port to ESP-IDF',
        'upstream': {
            'versions': {'1.23.1': '2.11.3'},
            'name': 'littlefs',
            'supplier': {'name': 'littlefs-project'},
            'license': 'BSD-3-Clause',
            'url': 'https://github.com/littlefs-project/littlefs',
            'purl': 'pkg:github/littlefs-project/littlefs@v{}',
            'cpe': 'cpe:2.3:a:littlefs_project:littlefs:{}:*:*:*:*:*:*:*',
        },
    },
}

# Libraries from lib_deps that are forks of third-party projects
KNOWN_LIB_UPSTREAMS = {
    'ArduinoJson': {
        'license': 'MIT',
        'supplier': {'name': 'Benoit Blanchon'},
        'url': 'https://github.com/bblanchon/ArduinoJson',
        'purl': 'pkg:github/bblanchon/arduinojson@v{}',
        'cpe': 'cpe:2.3:a:arduinojson:arduinojson:{}:*:*:*:*:*:*:*',
    },
    'strict_variant': {
        'license': 'BSL-1.0',
        'supplier': {'name': 'Chris Beck'},
        'url': 'https://github.com/cbeck88/strict-variant',
    },
    'WireGuard-ESP32': {
        'license': 'BSD-3-Clause',
        'supplier': {'name': 'Kenta Ida'},
        'url': 'https://github.com/ciniml/WireGuard-ESP32-Arduino',
    },
    'libcbv2g': {
        'license': 'Apache-2.0',
        'supplier': {'name': 'EVerest'},
        'url': 'https://github.com/EVerest/libcbv2g',
    },
}

# Bricklet firmwares embedded into the firmware (src/modules/*/bricklet_<name>_firmware_<version>.zbin)
BRICKLET_LICENSE = 'LGPL-2.1-or-later'

BRICKLIB2 = {
    'name': 'bricklib2',
    'description': 'Common code of all Bricklets with XMC co-processor. Not pinned by the Bricklet repositories, the exact revision is unknown',
    'url': 'https://github.com/Tinkerforge/bricklib2',
}

BRICKLET_THIRD_PARTY = {
    'xmclib': {
        'name': 'XMC Peripheral Library',
        'version': '2.1.16',
        'description': 'Infineon XMC1000 peripheral drivers (bricklib2/xmclib/XMCLib)',
        'supplier': {'name': 'Infineon Technologies AG'},
        'license': 'BSD-3-Clause',
        'url': 'https://github.com/Infineon/mtb-xmclib-cat3',
    },
    'cmsis': {
        'name': 'CMSIS-Core',
        'version': '5.0.1',
        'description': 'Cortex-M0 core headers, shipped with the XMC Peripheral Library (bricklib2/xmclib/CMSIS)',
        'supplier': {'name': 'Arm Limited'},
        'license': 'Apache-2.0',
        'url': 'https://github.com/ARM-software/CMSIS_5',
    },
    'littlefs': {
        'name': 'littlefs',
        'version': '2.9.3',
        'description': 'File system for the SD card (bricklib2/warp/wem/littlefs)',
        'supplier': {'name': 'littlefs-project'},
        'license': 'BSD-3-Clause',
        'url': 'https://github.com/littlefs-project/littlefs',
        'purl': 'pkg:github/littlefs-project/littlefs@v{}',
        'cpe': 'cpe:2.3:a:littlefs_project:littlefs:{}:*:*:*:*:*:*:*',
    },
    'qrcode': {
        'name': 'QRCode',
        'description': 'QR code generator (src/qrcode)',
        'supplier': {'name': 'Richard Moore'},
        'license': 'MIT',
        'url': 'https://github.com/ricmoo/QRCode',
    },
    'nxp-nfc-library': {
        'name': 'NXP-NCI NfcLibrary',
        'description': 'NXP NCI and NDEF library for the PN7150, modified by Tinkerforge (src/NfcLibrary)',
        'supplier': {'name': 'NXP Semiconductors'},
        'license': 'NXP proprietary license',
        'patched': True,
    },
}

BRICKLET_FIRMWARES = {
    # zbin name: repository, third-party code besides xmclib and cmsis
    'evse': ('evse-bricklet', []),
    'evse_v2': ('evse-v2-bricklet', []),
    'nfc': ('nfc-bricklet', ['nxp-nfc-library']),
    'real_time_clock_v2': ('real-time-clock-v2-bricklet', []),
    'rs485': ('rs485-bricklet', []),
    'warp_energy_manager': ('warp-energy-manager-bricklet', ['littlefs']),
    'warp_energy_manager_v2': ('warp-energy-manager-v2-bricklet', ['littlefs']),
    'warp_esp32_ethernet_v2_co': ('warp-esp32-ethernet-v2-co-bricklet', []),
    'warp_front_panel': ('warp-front-panel-bricklet', ['qrcode']),
}

# Data embedded into the firmware that is generated from third-party assets
EMBEDDED_DATA = [
    {
        'name': 'warp-front-panel-flash-map',
        'description': 'Fonts and icons for the WARP Front Panel Bricklet, written to its flash by the firmware '
                       '(src/modules/front_panel/flash_map.bin.xz, generated by warp-front-panel-bricklet/software/flash/make_flash_map.py)',
        'source': 'src/modules/front_panel/generated/flash_map_xz.embedded.cpp',
        'file': 'src/modules/front_panel/flash_map.bin.xz',
        'url': 'https://github.com/Tinkerforge/warp-front-panel-bricklet/tree/master/software/flash',
        'third_party': [
            {
                'name': 'DejaVu Sans Mono',
                'description': 'Font, rendered into bitmaps',
                'supplier': {'name': 'DejaVu fonts team'},
                'license': 'Bitstream-Vera',
                'url': 'https://dejavu-fonts.github.io/',
            },
            {
                'name': 'Feather',
                'description': 'Icons, rendered into bitmaps',
                'supplier': {'name': 'Cole Bemis'},
                'license': 'MIT',
                'url': 'https://github.com/feathericons/feather',
            },
        ],
    },
]

# Third-party code that is copied into this repository. A component is only
# added if one of its paths was compiled into the firmware or bundled into
# the web interface.
VENDORED = [
    {
        'name': 'xz-embedded',
        'description': 'XZ decompressor (src/tools/libxz)',
        'supplier': {'name': 'Lasse Collin'},
        'license': '0BSD',
        'url': 'https://tukaani.org/xz/embedded.html',
        'paths': ['src/tools/libxz/'],
    },
    {
        'name': 'musl',
        'version': '1.1.23',
        'description': 'timegm() implementation (src/musl_libc_timegm.cpp)',
        'supplier': {'name': 'musl libc'},
        'license': 'MIT',
        'url': 'https://musl.libc.org/',
        'cpe': 'cpe:2.3:a:musl-libc:musl:{}:*:*:*:*:*:*:*',
        'paths': ['src/musl_libc_timegm.cpp'],
    },
    {
        'name': 'ESPAsyncWebServer',
        'description': 'Digest authentication, stripped down (src/digest_auth.cpp)',
        'supplier': {'name': 'Hristo Gochkov'},
        'license': 'LGPL-2.1-or-later',
        'url': 'https://github.com/me-no-dev/ESPAsyncWebServer',
        'paths': ['src/digest_auth.cpp'],
    },
    {
        'name': 'esp_websocket_client',
        'version': '1.4.0',
        'description': 'WebSocket client, patched (src/tools/tf_websocket_client.c)',
        'supplier': ESPRESSIF,
        'license': 'Apache-2.0',
        'url': 'https://github.com/espressif/esp-protocols/tree/master/components/esp_websocket_client',
        'purl': 'pkg:github/espressif/esp-protocols@websocket-v{}#components/esp_websocket_client',
        'patched': True,
        'paths': ['src/tools/tf_websocket_client.c'],
    },
    {
        'name': 'PDFGen',
        'version': '8a7298e7fa934b6ab1c21c876f9474756a44931d',
        'description': 'PDF generator, modified (src/modules/charge_tracker/pdfgen.cpp)',
        'supplier': {'name': 'Andre Renaud'},
        'license': 'Unlicense',
        'url': 'https://github.com/AndreRenaud/PDFGen',
        'purl': 'pkg:github/andrerenaud/pdfgen@{}',
        'patched': True,
        'paths': ['src/modules/charge_tracker/pdfgen.cpp'],
    },
    {
        'name': 'coreMQTT',
        'version': '2.1.0',
        'description': 'MQTT topic filter matching (src/modules/mqtt/matchTopicFilter.cpp)',
        'supplier': {'name': 'Amazon.com, Inc.'},
        'license': 'MIT',
        'url': 'https://github.com/FreeRTOS/coreMQTT',
        'purl': 'pkg:github/freertos/coremqtt@v{}',
        'paths': ['src/modules/mqtt/matchTopicFilter.cpp'],
    },
    {
        'name': 'wireguard-tools',
        'description': 'WireGuard key generation in JavaScript (web/src/modules/wireguard/wireguard.js)',
        'supplier': {'name': 'Jason A. Donenfeld'},
        'license': 'GPL-2.0-only',
        'url': 'https://git.zx2c4.com/wireguard-tools/tree/contrib/keygen-html/wireguard.js',
        'paths': ['web/src/modules/wireguard/wireguard.js', 'web/src_tfpp/modules/wireguard/wireguard.js'],
    },
]

KNOWN_SPDX_IDS = {
    '0BSD', 'Apache-2.0', 'BSD-2-Clause', 'BSD-3-Clause', 'BSL-1.0', 'CC0-1.0', 'GPL-2.0-only', 'GPL-2.0-or-later',
    'GPL-3.0-only', 'GPL-3.0-or-later', 'ISC', 'LGPL-2.1-only', 'LGPL-2.1-or-later', 'LGPL-3.0-only',
    'LGPL-3.0-or-later', 'MIT', 'MPL-2.0', 'Unlicense', 'Zlib', 'Bitstream-Vera',
}

DEPRECATED_SPDX_IDS = {
    'Apache 2.0': 'Apache-2.0',
    'GPL-2.0': 'GPL-2.0-only',
    'GPL-2.0+': 'GPL-2.0-or-later',
    'LGPL-2.1': 'LGPL-2.1-only',
    'LGPL-2.1+': 'LGPL-2.1-or-later',
}


class SbomError(Exception):
    pass


def warn(message):
    print(f'SBOM warning: {message}', file=sys.stderr)


def read_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        return json.loads(f.read())


def read_text(path):
    with open(path, 'r', encoding='utf-8') as f:
        return f.read()


def is_under(path, root):
    return root != None and os.path.commonpath([path, root]) == root


def data_hashes(data):
    return [
        {'alg': 'SHA-256', 'content': hashlib.sha256(data).hexdigest()},
        {'alg': 'SHA-512', 'content': hashlib.sha512(data).hexdigest()},
    ]


def file_hashes(path):
    sha256 = hashlib.sha256()
    sha512 = hashlib.sha512()

    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(1 << 20), b''):
            sha256.update(chunk)
            sha512.update(chunk)

    return [
        {'alg': 'SHA-256', 'content': sha256.hexdigest()},
        {'alg': 'SHA-512', 'content': sha512.hexdigest()},
    ]


def license_choice(license):
    if license == None:
        return None

    license = DEPRECATED_SPDX_IDS.get(license.strip(), license.strip())

    if license in KNOWN_SPDX_IDS:
        return [{'license': {'id': license}}]

    if re.search(r'\s(OR|AND|WITH)\s', license) != None:
        return [{'expression': license}]

    return [{'license': {'name': license}}]


def parse_sbom_entity(value):
    # ESP-IDF manifests use the SPDX style "Organization: name <email>" or "Person: name"
    if value == None:
        return None

    value = re.sub(r'^(Organization|Person):\s*', '', value).strip()
    m = re.fullmatch(r'(.*?)\s*<([^>]*)>', value)

    if m == None:
        return {'name': value}

    if m.group(2).startswith('http'):
        return {'name': m.group(1), 'url': [m.group(2)]}

    return {'name': m.group(1), 'contact': [{'email': m.group(2)}]}


def normalize_git_url(url):
    if url == None:
        return None

    m = re.fullmatch(r'git@([^:]+):(.+)', url)

    if m != None:
        url = f'https://{m.group(1)}/{m.group(2)}'

    if url.endswith('.git'):
        url = url[:-len('.git')]

    return url.rstrip('/')


def github_purl(url, version):
    m = re.fullmatch(r'https://github\.com/([^/]+)/([^/]+)', normalize_git_url(url) or '')

    if m == None:
        return None

    return f'pkg:github/{m.group(1).lower()}/{m.group(2).lower()}@{version}'


def git(path, *args):
    result = subprocess.run(['git', '-C', path] + list(args), capture_output=True, encoding='utf-8')

    if result.returncode != 0:
        return None

    return result.stdout.strip()


def git_info(path):
    # Only accept the repository if path is its root, not some parent repository
    toplevel = git(path, 'rev-parse', '--show-toplevel')

    if toplevel == None or os.path.realpath(toplevel) != os.path.realpath(path):
        return None

    return {
        'url': normalize_git_url(git(path, 'config', '--get', 'remote.origin.url')),
        'commit': git(path, 'rev-parse', 'HEAD'),
        'dirty': len(git(path, 'status', '--porcelain', '--untracked-files=no') or '') > 0,
    }


def bom_ref(name, version):
    return name if version == None else f'{name}@{version}'


def make_component(ref, name, version=None, type='library', supplier=None, manufacturer=None, description=None,
                   license=None, hashes=None, purl=None, cpe=None, url=None, vcs=None, pedigree=None, properties=None, group=None):
    component = {
        'type': type,
        'bom-ref': ref,
        'supplier': supplier,
        'manufacturer': manufacturer,
        'group': group,
        'name': name,
        'version': version,
        'description': description,
        'hashes': hashes,
        'licenses': license_choice(license),
        'cpe': cpe.format(version) if cpe != None and version != None else None,
        'purl': purl,
        'externalReferences': [],
        'pedigree': pedigree,
        'properties': [{'name': key, 'value': str(value)} for key, value in (properties or {}).items() if value != None],
    }

    if url != None:
        component['externalReferences'].append({'type': 'website', 'url': url})

    if vcs != None:
        component['externalReferences'].append({'type': 'vcs', 'url': vcs})

    return {key: value for key, value in component.items() if value != None and value != []}


def parse_map(map_path, project_dir):
    archives = set()
    objects = set()

    for token in read_text(map_path).split():
        if token.endswith(')') and '.a(' in token:
            archives.add(os.path.normpath(os.path.join(project_dir, token[:token.index('.a(') + 2])))
        elif token.endswith('.o') and '(' not in token:
            objects.add(os.path.normpath(os.path.join(project_dir, token)))

    return archives, objects


def find_parent(path, predicate):
    while True:
        parent = os.path.dirname(path)

        if parent == path:
            return None

        path = parent

        if predicate(path):
            return path


def archive_name(path):
    name = os.path.basename(path)

    if name.startswith('lib'):
        name = name[len('lib'):]

    return name[:-len('.a')]


def parse_dependencies_lock(path):
    # Minimal parser for the "dependencies" section of ESP-IDF's dependencies.lock
    versions = {}
    current = None

    for line in read_text(path).splitlines():
        m = re.fullmatch(r'  (\S+):', line)

        if m != None:
            current = m.group(1)
            continue

        m = re.fullmatch(r'    version: (.+)', line)

        if m != None and current != None:
            versions[current] = m.group(1).strip().strip('\'"')

        if not line.startswith(' '):
            current = None

    return versions


def parse_versions_txt(path):
    versions = {}

    for line in read_text(path).splitlines():
        if ':' in line:
            key, value = line.split(':', 1)
            versions[key.strip()] = value.strip()

    return versions


def fetch_url(url):
    headers = {'User-Agent': 'esp32-firmware-sbom'}
    token = os.environ.get('GITHUB_TOKEN')

    if token and url.startswith('https://api.github.com/'):
        headers['Authorization'] = f'Bearer {token}'

    try:
        with urllib.request.urlopen(urllib.request.Request(url, headers=headers), timeout=60) as response:
            return response.read().decode('utf-8')
    except urllib.error.HTTPError as e:
        if e.code == 404:
            return None

        raise SbomError(f'Could not download {url}: {e} (set GITHUB_TOKEN if the GitHub API rate limit was exceeded)')
    except urllib.error.URLError as e:
        raise SbomError(f'Could not download {url}: {e}')


def parse_sbom_manifest(text):
    # Minimal parser for ESP-IDF's sbom.yml files, only reads top-level scalars like "version: '1.2.3'"
    manifest = {}

    for line in text.splitlines():
        m = re.fullmatch(r'([A-Za-z][\w-]*):\s*(.*?)\s*', line)

        if m == None or len(m.group(2)) == 0:
            continue

        value = m.group(2)

        if len(value) >= 2 and value[0] == value[-1] and value[0] in '\'"':
            value = value[1:-1]

        manifest[m.group(1)] = value

    return manifest


def parse_gitmodules(text):
    submodules = {}
    current = None

    for line in text.splitlines():
        line = line.strip()

        if len(line) == 0 or line.startswith('#'):
            continue

        m = re.fullmatch(r'\[submodule "(.+)"\]', line)

        if m != None:
            current = {}
            submodules[m.group(1)] = current
            continue

        if current == None or '=' not in line:
            continue

        key, value = [x.strip() for x in line.split('=', 1)]

        if key.startswith('sbom-'):
            current.setdefault('sbom', {}).setdefault(key[len('sbom-'):], value)  # first sbom-cpe wins
        else:
            current[key] = value

    return submodules


def download_idf_data(idf_tag):
    raw_url = f'https://raw.githubusercontent.com/espressif/esp-idf/{idf_tag}'
    tree = json.loads(fetch_url(f'https://api.github.com/repos/espressif/esp-idf/git/trees/{idf_tag}?recursive=1') or 'null')

    if tree == None or tree.get('truncated'):
        raise SbomError(f'Could not get file list of ESP-IDF {idf_tag}')

    submodule_commits = {entry['path']: entry['sha'] for entry in tree['tree'] if entry['type'] == 'commit'}
    gitmodules = {info.get('path', path): info for path, info in parse_gitmodules(fetch_url(f'{raw_url}/.gitmodules') or '').items()}
    data = {'submodules': {}, 'manifests': {}}

    for _, kind, path, _, _ in IDF_THIRD_PARTY + [(None, 'manifest', 'components/newlib/sbom_newlibc.yml', None, None)]:
        if kind == 'manifest':
            manifest = fetch_url(f'{raw_url}/{path}')

            if manifest != None:
                data['manifests'][path] = parse_sbom_manifest(manifest)

            continue

        info = gitmodules.get(path, {})
        url = normalize_git_url(info.get('url', ''))

        if url.startswith('../../'):
            url = 'https://github.com/' + url[len('../../'):]

        commit = submodule_commits.get(path)
        sbom = info.get('sbom')

        if sbom == None and commit != None and url.startswith('https://github.com/'):
            # Submodules forked into the espressif namespace carry their own manifest
            manifest = fetch_url(f'https://raw.githubusercontent.com/{url[len("https://github.com/"):]}/{commit}/sbom.yml')

            if manifest != None:
                sbom = parse_sbom_manifest(manifest)

        data['submodules'][path] = {'url': url, 'commit': commit, 'sbom': sbom or {}}

    return data


def get_idf_data(project_dir, idf_tag):
    cache_path = os.path.join(project_dir, '.pio', 'sbom', f'esp-idf-{idf_tag}.json')

    try:
        return read_json(cache_path)
    except FileNotFoundError:
        pass

    print(f'Downloading SBOM information of ESP-IDF {idf_tag}')

    data = download_idf_data(idf_tag)

    os.makedirs(os.path.dirname(cache_path), exist_ok=True)

    with open(cache_path + '.tmp', 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=4)

    os.replace(cache_path + '.tmp', cache_path)

    return data


class SbomBuilder:
    def __init__(self, project_dir, build_dir):
        self.project_dir = project_dir
        self.build_dir = build_dir
        self.metadata = read_json(os.path.join(build_dir, 'metadata.json'))

        try:
            self.inputs = read_json(os.path.join(build_dir, INPUTS_FILENAME))
        except FileNotFoundError:
            raise SbomError(f'{os.path.join(build_dir, INPUTS_FILENAME)} not found, build the environment first')

        self.components = {}
        self.dependencies = {}
        self.self_built_libs = False
        self.tools = []
        self.root_ref = 'firmware'
        self.firmware_commit = self.metadata.get('git_commit')
        self.firmware_url = normalize_git_url(self.metadata.get('git_url')) or 'https://github.com/Tinkerforge/esp32-firmware'

    def add(self, component, depended_on_by=None):
        ref = component['bom-ref']

        if ref not in self.components:
            self.components[ref] = component
            self.dependencies.setdefault(ref, [])

        if depended_on_by != None:
            self.depend(depended_on_by, ref)

        return ref

    def depend(self, ref, dependency_ref):
        dependencies = self.dependencies.setdefault(ref, [])

        if dependency_ref not in dependencies and dependency_ref != ref:
            dependencies.append(dependency_ref)

    def repo_file_url(self, relative_path):
        if self.firmware_commit == None:
            return None

        return f'{self.firmware_url}/blob/{self.firmware_commit}/software/{relative_path}'

    def patches_pedigree(self, patch_dir_relative, notes):
        patch_dir = os.path.join(self.project_dir, patch_dir_relative)
        patches = []

        for path in sorted(glob.glob(os.path.join(patch_dir, '*'))):
            if not os.path.isfile(path):
                continue

            url = self.repo_file_url(os.path.relpath(path, self.project_dir))
            patch = {'type': 'unofficial'}

            if url != None:
                patch['diff'] = {'url': url}

            patches.append(patch)

        if self.self_built_libs:
            notes += '. The ESP-IDF libraries are self-built and may contain changes beyond these patches'

        if len(patches) == 0:
            return {'notes': notes} if self.self_built_libs else None

        return {'patches': patches, 'notes': notes}

    def generate(self):
        map_path = os.path.join(self.build_dir, 'firmware.map')

        if not os.path.exists(map_path):
            raise SbomError(f'{map_path} not found, build the environment first')

        archives, objects = parse_map(map_path, self.project_dir)

        self.src_dir = os.path.join(self.build_dir, 'src')
        self.compiled_sources = set()

        for obj in objects:
            if is_under(obj, self.src_dir):
                self.compiled_sources.add(os.path.join('src', os.path.relpath(obj, self.src_dir))[:-len('.o')])

        self.add_root()

        libs_package_dir = None
        toolchain_dir = None
        idf_archives = set()
        toolchain_archives = set()

        for archive in sorted(archives):
            if is_under(archive, self.build_dir):
                continue  # framework and LDF libraries are handled below

            if libs_package_dir == None:
                libs_package_dir = find_parent(archive, lambda p: os.path.exists(os.path.join(p, 'versions.txt')) and os.path.isdir(os.path.join(p, 'esp32')))

            if toolchain_dir == None:
                toolchain_dir = find_parent(archive, lambda p: os.path.basename(p).startswith('toolchain-') and os.path.exists(os.path.join(p, 'package.json')))

            if is_under(archive, libs_package_dir):
                idf_archives.add(archive_name(archive))
            elif is_under(archive, toolchain_dir):
                toolchain_archives.add(archive_name(archive))
            else:
                warn(f'Unknown archive {archive}')

        if libs_package_dir == None:
            raise SbomError('Could not find esp32-arduino-libs package in linker map')

        idf_ref = self.add_esp32_arduino_libs(libs_package_dir, idf_archives)
        self.add_arduino_esp32(idf_ref)
        self.add_toolchain(toolchain_dir, toolchain_archives)
        self.add_ldf_libs()
        self.add_vendored()
        self.add_embedded_firmwares()
        self.add_embedded_data()
        self.add_qca_firmware()
        self.add_tzdata()
        self.add_web_interface()

        return self.bom()

    def add_root(self):
        metadata = self.metadata
        firmware_basename = metadata['firmware_basename']
        distributions = []

        for path in sorted(glob.glob(os.path.join(self.project_dir, 'build', f'{glob.escape(firmware_basename)}_*.bin'))):
            distributions.append({
                'type': 'distribution',
                'url': os.path.basename(path),
                'hashes': file_hashes(path),
            })

        version = metadata.get('version_full_str')

        if version == None:
            version = '.'.join(metadata['version'][:3])

            if metadata['version'][3] != '255':
                version += f'-beta.{metadata["version"][3]}'

        root = make_component(self.root_ref, f'{metadata["firmware_info_product_name"]} Firmware', version,
                              type='firmware',
                              supplier=TINKERFORGE,
                              manufacturer=TINKERFORGE,
                              description=f'Firmware for {metadata["firmware_info_product_name"]} ({metadata["product_id"]})',
                              license=FIRMWARE_LICENSE,
                              purl=github_purl(self.firmware_url, self.firmware_commit) if self.firmware_commit != None else None,
                              vcs=self.firmware_url,
                              properties={
                                  'tf:product_id': metadata['product_id'],
                                  'tf:firmware_info_product_id': metadata.get('firmware_info_product_id'),
                                  'tf:pio_env': metadata.get('pio_env'),
                                  'tf:firmware_basename': firmware_basename,
                                  'tf:build_timestamp': metadata['build_timestamp'],
                                  'tf:git_commit': self.firmware_commit,
                                  'tf:git_dirty': metadata.get('git_dirty'),
                                  'tf:signature_preset': metadata.get('signature_preset') or None,
                              })

        root.setdefault('externalReferences', []).extend(distributions)

        if len(distributions) > 0:
            # The first image is the one that is flashed via the web interface
            primary = next((d for d in distributions if d['url'].endswith(('_merged.bin', '_ota.bin'))), distributions[0])
            root['hashes'] = primary['hashes']

        self.root = root
        self.dependencies[self.root_ref] = []

    def add_esp32_arduino_libs(self, package_dir, idf_archives):
        try:
            versions = parse_versions_txt(os.path.join(package_dir, 'versions.txt'))
            lock = parse_dependencies_lock(os.path.join(package_dir, 'esp32', 'dependencies.lock'))
        except FileNotFoundError as e:
            raise SbomError(f'ESP-IDF libraries in {package_dir} have no version information: {e}')

        try:
            tinkerforge_json = read_json(os.path.join(package_dir, 'tinkerforge.json'))
        except FileNotFoundError:
            tinkerforge_json = {}

        package_name = os.path.basename(package_dir)  # esp32-arduino-libs#<branch>_<commit>
        m = re.fullmatch(r'(.+?)#(.+)_([0-9a-f]{40})', package_name)

        self.self_built_libs = m == None

        if m != None:
            base, branch, commit = m.groups()
            url = normalize_git_url(tinkerforge_json.get('url', f'https://github.com/Tinkerforge/{base}'))

            libs_ref = self.add(make_component(f'{base}@{commit}', base, commit,
                                               supplier=TINKERFORGE,
                                               description='Precompiled ESP-IDF libraries, built with esp32-arduino-lib-builder and patches from esp32-firmware/software/patches/lib-builder',
                                               purl=github_purl(url, commit),
                                               vcs=url,
                                               properties={'tf:branch': branch} | {f'tf:versions:{key}': value for key, value in versions.items()}),
                                depended_on_by=self.root_ref)
        else:
            # Self-built libraries (e.g. debug builds of the lib-builder output) are not published,
            # so describe them by the versions they were built from
            warn(f'ESP-IDF libraries in {package_dir} are self-built, the SBOM describes them by their versions.txt')

            libs_ref = self.add(make_component('esp32-arduino-libs@self-built', 'esp32-arduino-libs',
                                               supplier=TINKERFORGE,
                                               description='Self-built ESP-IDF libraries, built with esp32-arduino-lib-builder. Not a published package, '
                                                           'the exact build configuration and patches are unknown',
                                               properties={'tf:self-built': 'True'} | {f'tf:versions:{key}': value for key, value in versions.items()}),
                                depended_on_by=self.root_ref)

        # ESP-IDF
        idf_version = lock.get('idf')

        if idf_version == None:
            raise SbomError(f'Could not find ESP-IDF version in {package_dir}')

        idf_tag = f'v{idf_version}'
        idf_data = get_idf_data(self.project_dir, idf_tag)

        managed_archives = {name for name in idf_archives if '__' in name}
        idf_component_archives = sorted(idf_archives - managed_archives)

        idf_ref = self.add(make_component(f'esp-idf@{idf_tag}', 'esp-idf', idf_version,
                                          type='framework',
                                          supplier=ESPRESSIF,
                                          manufacturer=ESPRESSIF,
                                          description='Espressif IoT Development Framework',
                                          license='Apache-2.0',
                                          purl=f'pkg:github/espressif/esp-idf@{idf_tag}',
                                          cpe='cpe:2.3:a:espressif:esp-idf:{}:*:*:*:*:*:*:*',
                                          url='https://github.com/espressif/esp-idf',
                                          pedigree=self.patches_pedigree(os.path.join('patches', 'lib-builder', 'esp-idf'), 'Patched by Tinkerforge'),
                                          properties={
                                              'tf:built-from': versions.get('esp-idf'),
                                              'tf:linked-archives': ', '.join(idf_component_archives),
                                          }),
                           depended_on_by=libs_ref)

        for archive_names, kind, path, fallback_name, license in IDF_THIRD_PARTY:
            linked = sorted(set(archive_names) & idf_archives)

            if len(linked) == 0:
                continue

            if kind == 'submodule':
                entry = idf_data['submodules'].get(path, {})
                info = entry.get('sbom', {})
                commit = entry.get('commit')
                url = normalize_git_url(info.get('url', entry.get('url')))
                purl = github_purl(url, commit) if commit != None and url != None else None
                version = info.get('version', commit)
            else:
                info = idf_data['manifests'].get(path, {})
                commit = None
                url = info.get('url')
                purl = None
                version = info.get('version')

            if len(info) == 0 and commit == None:
                warn(f'No ESP-IDF SBOM data for {path}')

            cpe = info.get('cpe')

            if isinstance(cpe, list):
                cpe = cpe[0]

            patch_dir = os.path.join('patches', 'lib-builder', 'esp-idf', path)
            name = info.get('name', fallback_name)

            self.add(make_component(bom_ref(f'esp-idf/{path}', version), name, version,
                                    supplier=parse_sbom_entity(info.get('supplier')) or ESPRESSIF,
                                    manufacturer=parse_sbom_entity(info.get('originator')),
                                    description=info.get('description'),
                                    license=license,
                                    purl=purl,
                                    cpe=cpe,
                                    url=url,
                                    pedigree=self.patches_pedigree(patch_dir, 'Patched by Tinkerforge'),
                                    properties={
                                        'tf:esp-idf-path': path,
                                        'tf:git_commit': commit,
                                        'tf:linked-archives': ', '.join(linked),
                                    }),
                     depended_on_by=idf_ref)

        # Managed components
        for archive in sorted(managed_archives):
            name = archive.replace('__', '/', 1)
            version = lock.get(name)
            info = IDF_MANAGED_COMPONENTS.get(name)

            if info == None:
                warn(f'Unknown ESP-IDF managed component {name}')
                info = {}

            namespace, short_name = name.split('/', 1)
            patch_dir = os.path.join('patches', 'lib-builder', 'managed_components', archive)
            ref = self.add(make_component(bom_ref(name, version), short_name, version,
                                          group=namespace,
                                          supplier=info.get('supplier'),
                                          description=info.get('description'),
                                          license=info.get('license'),
                                          url=info.get('url'),
                                          pedigree=self.patches_pedigree(patch_dir, 'Patched by Tinkerforge'),
                                          properties={'tf:esp-component-registry': f'https://components.espressif.com/components/{name}'}),
                           depended_on_by=libs_ref)

            upstream = info.get('upstream')

            if upstream != None:
                upstream_version = upstream['versions'].get(version)

                if upstream_version == None:
                    warn(f'Unknown upstream version of ESP-IDF managed component {name} {version}, update IDF_MANAGED_COMPONENTS in sbom.py')

                self.add(make_component(bom_ref(upstream['name'], upstream_version), upstream['name'], upstream_version,
                                        supplier=upstream['supplier'],
                                        license=upstream['license'],
                                        purl=upstream['purl'].format(upstream_version) if upstream_version != None else None,
                                        cpe=upstream.get('cpe'),
                                        url=upstream['url']),
                         depended_on_by=ref)

        self.idf_data = idf_data

        return idf_ref

    def add_arduino_esp32(self, idf_ref):
        framework_dir = self.inputs['framework_dir']
        package_json = read_json(os.path.join(framework_dir, 'package.json'))
        version = package_json['version']
        info = git_info(framework_dir) or {}
        commit = info.get('commit')
        url = info.get('url') or 'https://github.com/espressif/arduino-esp32'
        libraries_dir = os.path.join(framework_dir, 'libraries')
        libraries = sorted(lib['name'] for lib in self.inputs['libs'] if is_under(lib['path'], libraries_dir))
        upstream = None

        if not url.startswith('https://github.com/espressif/'):
            upstream = {'ancestors': [make_component(f'arduino-esp32-upstream@{version}', 'arduino-esp32', version,
                                                     supplier=ESPRESSIF,
                                                     license='LGPL-2.1-or-later',
                                                     purl=f'pkg:github/espressif/arduino-esp32@{version}',
                                                     url='https://github.com/espressif/arduino-esp32')],
                        'notes': 'Fork maintained by Tinkerforge'}

        self.arduino_ref = self.add(make_component(f'arduino-esp32@{commit or version}', 'arduino-esp32', version,
                                                   type='framework',
                                                   supplier=TINKERFORGE if 'Tinkerforge' in url else ESPRESSIF,
                                                   manufacturer=ESPRESSIF,
                                                   description='Arduino core for the ESP32',
                                                   license='LGPL-2.1-or-later',
                                                   purl=github_purl(url, commit) if commit != None else None,
                                                   vcs=url,
                                                   pedigree=upstream,
                                                   properties={
                                                       'tf:git_commit': commit,
                                                       'tf:git_dirty': info.get('dirty'),
                                                       'tf:arduino-libraries': ', '.join(libraries),
                                                   }),
                                    depended_on_by=self.root_ref)

        self.depend(self.arduino_ref, idf_ref)

    def add_toolchain(self, toolchain_dir, toolchain_archives):
        if toolchain_dir == None:
            warn('Could not find toolchain in linker map')
            return

        package_json = read_json(os.path.join(toolchain_dir, 'package.json'))
        toolchain_version = package_json['version']
        gcc_version = toolchain_version.split('+')[0]

        self.tools.append(make_component(f'{package_json["name"]}@{toolchain_version}', package_json['name'], toolchain_version,
                                         type='application',
                                         supplier=ESPRESSIF,
                                         description=package_json.get('description'),
                                         license=package_json.get('license'),
                                         url=normalize_git_url(package_json.get('repository', {}).get('url'))))

        newlib_archives = sorted(toolchain_archives & {'c', 'm', 'g'})

        if len(newlib_archives) > 0:
            newlib_version = None

            for path in glob.glob(os.path.join(toolchain_dir, '*', 'include', '_newlib_version.h')):
                m = re.search(r'#define\s+_NEWLIB_VERSION\s+"([^"]+)"', read_text(path))

                if m != None:
                    newlib_version = m.group(1)

            info = self.idf_data['manifests'].get('components/newlib/sbom_newlibc.yml', {})

            self.add(make_component(bom_ref('newlib', newlib_version), 'newlib', newlib_version,
                                    supplier=parse_sbom_entity(info.get('supplier')) or ESPRESSIF,
                                    manufacturer=parse_sbom_entity(info.get('originator')),
                                    description='C standard library, part of the toolchain',
                                    license='Newlib License (various BSD-style licenses)',
                                    cpe=info.get('cpe', 'cpe:2.3:a:newlib_project:newlib:{}:*:*:*:*:*:*:*'),
                                    url='https://github.com/espressif/newlib-esp32',
                                    properties={'tf:linked-archives': ', '.join(newlib_archives)}),
                     depended_on_by=self.root_ref)

        gcc_archives = sorted(toolchain_archives & {'gcc', 'stdc++', 'supc++', 'gcov', 'atomic'})

        if len(gcc_archives) > 0:
            self.add(make_component(f'gcc@{gcc_version}', 'gcc', gcc_version,
                                    supplier=ESPRESSIF,
                                    manufacturer={'name': 'Free Software Foundation'},
                                    description='GCC runtime libraries, part of the toolchain',
                                    license='GPL-3.0-or-later WITH GCC-exception-3.1',
                                    cpe='cpe:2.3:a:gnu:gcc:{}:*:*:*:*:*:*:*',
                                    url='https://github.com/espressif/gcc',
                                    properties={'tf:linked-archives': ', '.join(gcc_archives)}),
                     depended_on_by=self.root_ref)

        unknown = toolchain_archives - set(newlib_archives) - set(gcc_archives)

        if len(unknown) > 0:
            warn(f'Unknown toolchain archives: {", ".join(sorted(unknown))}')

    def add_ldf_libs(self):
        framework_dir = self.inputs['framework_dir']
        refs = {}
        urls = {}

        for lib in self.inputs['libs']:
            if is_under(lib['path'], framework_dir):
                refs[lib['path']] = self.arduino_ref
                continue

            path = lib['path']
            name = lib['name']
            manifest = {}

            try:
                manifest = read_json(os.path.join(path, 'library.json'))
            except FileNotFoundError:
                try:
                    for line in read_text(os.path.join(path, 'library.properties')).splitlines():
                        if '=' in line:
                            key, value = line.split('=', 1)
                            manifest[key.strip()] = value.strip()
                except FileNotFoundError:
                    pass

            info = git_info(path) or {}
            commit = info.get('commit')
            url = info.get('url')
            upstream = KNOWN_LIB_UPSTREAMS.get(name)
            manifest_version = lib.get('version') or manifest.get('version')
            is_tinkerforge = url != None and '/Tinkerforge/' in url
            license = manifest.get('license')
            pedigree = None

            if is_tinkerforge and upstream == None and license == 'LGPL-2.1':
                license = 'LGPL-2.1-or-later'  # source files say "version 2.1 of the License, or (at your option) any later version"

            if upstream != None:
                version = manifest_version
                license = upstream['license']
                ancestor_purl = upstream['purl'].format(manifest_version) if 'purl' in upstream else github_purl(upstream['url'], manifest_version)
                pedigree = {
                    'ancestors': [make_component(f'{name}-upstream@{manifest_version}', name, manifest_version,
                                                 supplier=upstream['supplier'],
                                                 license=upstream['license'],
                                                 purl=ancestor_purl,
                                                 cpe=upstream.get('cpe'),
                                                 url=upstream['url'])],
                    'notes': 'Fork maintained by Tinkerforge' if is_tinkerforge else None,
                }
                pedigree = {key: value for key, value in pedigree.items() if value != None}
            else:
                version = commit or manifest_version

            if commit == None:
                warn(f'Library {name} at {path} is not a git checkout, version is taken from its manifest')

            ref = self.add(make_component(f'{name}@{commit or manifest_version}', name, version,
                                          supplier=TINKERFORGE if is_tinkerforge else (upstream or {}).get('supplier'),
                                          manufacturer=(upstream or {}).get('supplier'),
                                          description=manifest.get('description') if manifest.get('description') not in (None, 'TBD') else manifest.get('sentence'),
                                          license=license,
                                          purl=github_purl(url, commit) if commit != None and url != None else None,
                                          cpe=upstream.get('cpe') if upstream != None else None,
                                          vcs=url,
                                          pedigree=pedigree,
                                          properties={
                                              'tf:git_commit': commit,
                                              'tf:git_dirty': info.get('dirty'),
                                              'tf:library-manifest-version': manifest_version,
                                          }))
            refs[path] = ref
            urls[path] = url

        depended_on = set()

        for lib in self.inputs['libs']:
            ref = refs[lib['path']]

            for dependency_path in lib['dependencies']:
                dependency_ref = refs.get(dependency_path)

                if dependency_ref != None and dependency_ref != ref:
                    self.depend(ref, dependency_ref)
                    depended_on.add(dependency_ref)

        # Libraries listed in lib_deps are direct dependencies of the firmware
        direct_names = set()
        direct_urls = set()
        direct_paths = set()

        for spec in self.inputs.get('lib_deps', []):
            spec = spec.strip()

            if spec.startswith('symlink://'):
                direct_paths.add(os.path.realpath(os.path.join(self.project_dir, spec[len('symlink://'):])))
            elif '://' in spec:
                direct_urls.add(normalize_git_url(spec.split('#', 1)[0]).lower())
            else:
                direct_names.add(re.split(r'[\s@=]', spec, maxsplit=1)[0].split('/')[-1])

        for lib in self.inputs['libs']:
            ref = refs[lib['path']]
            url = (urls.get(lib['path']) or '').lower()

            if ref not in depended_on or lib['name'] in direct_names or url in direct_urls or lib['path'] in direct_paths:
                self.depend(self.root_ref, ref)

    def is_used(self, paths):
        web_inputs = self.inputs['web']['inputs']

        for path in paths:
            if path.startswith('web/'):
                if any(x == path or x.startswith(path) for x in web_inputs):
                    return True
            elif any(x == path or x.startswith(path) for x in self.compiled_sources):
                return True

        return False

    def add_vendored(self):
        for vendored in VENDORED:
            if not self.is_used(vendored['paths']):
                continue

            version = vendored.get('version')
            pedigree = None

            if vendored.get('patched'):
                pedigree = {'notes': 'Modified by Tinkerforge'}

            self.add(make_component(bom_ref(f'vendored/{vendored["name"]}', version), vendored['name'], version,
                                    supplier=vendored['supplier'],
                                    description=vendored['description'],
                                    license=vendored['license'],
                                    purl=vendored['purl'].format(version) if 'purl' in vendored else None,
                                    cpe=vendored.get('cpe'),
                                    url=vendored['url'],
                                    pedigree=pedigree,
                                    properties={'tf:source-paths': ', '.join(vendored['paths'])}),
                     depended_on_by=self.root_ref)

    def add_embedded_firmwares(self):
        modules = set()

        for source in self.compiled_sources:
            m = re.fullmatch(r'src/modules/([^/]+)/generated/[^/]+_firmware_bin\.embedded\.cpp', source)

            if m != None:
                modules.add(m.group(1))

        for module in sorted(modules):
            for path in sorted(glob.glob(os.path.join(self.project_dir, 'src', 'modules', module, '*.zbin'))):
                # Same naming scheme as in util.embed_bricklet_firmware_bin
                m = re.fullmatch(r'bricklet_(.+)_firmware_(\d+)_(\d+)_(\d+)(?:_beta(\d+))?\.zbin', os.path.basename(path))

                if m == None:
                    warn(f'Unexpected bricklet firmware file name {path}')
                    continue

                zbin_name = m.group(1)
                version = '.'.join(m.group(2, 3, 4))
                beta = m.group(5)
                name = f'{zbin_name.replace("_", "-")}-bricklet-firmware'

                if beta != None:
                    version += f'-beta.{beta}'

                # Only the firmware part of the zbin is embedded, not the bootloader
                with zipfile.ZipFile(path) as zf:
                    firmware = zf.read(f'{zbin_name.replace("_", "-")}-bricklet-firmware.bin')

                repo, third_party = BRICKLET_FIRMWARES.get(zbin_name, (None, None))
                url = f'https://github.com/Tinkerforge/{repo}' if repo != None else None
                tag = f'v{version}' if beta == None else None

                if repo == None:
                    warn(f'Unknown bricklet firmware {zbin_name}, add it to BRICKLET_FIRMWARES in sbom.py')

                ref = self.add(make_component(f'{name}@{version}', name, version,
                                              type='firmware',
                                              supplier=TINKERFORGE,
                                              manufacturer=TINKERFORGE,
                                              description=f'Bricklet firmware, embedded to update the Bricklet ({os.path.relpath(path, self.project_dir)})',
                                              license=BRICKLET_LICENSE if repo != None else None,
                                              hashes=data_hashes(firmware),
                                              purl=github_purl(url, tag) if url != None and tag != None else None,
                                              vcs=url,
                                              properties={'tf:git_tag': tag if repo != None else None}),
                               depended_on_by=self.root_ref)

                if repo == None:
                    continue

                self.depend(ref, self.add(make_component(BRICKLIB2['name'], BRICKLIB2['name'],
                                                         supplier=TINKERFORGE,
                                                         description=BRICKLIB2['description'],
                                                         license=BRICKLET_LICENSE,
                                                         purl='pkg:github/tinkerforge/bricklib2',
                                                         vcs=BRICKLIB2['url'])))

                for key in ['xmclib', 'cmsis'] + third_party:
                    self.depend(ref, self.add_third_party(f'bricklet/{key}', BRICKLET_THIRD_PARTY[key]))

    def add_third_party(self, ref_prefix, info):
        version = info.get('version')

        return self.add(make_component(bom_ref(ref_prefix, version), info['name'], version,
                                       type=info.get('type', 'library'),
                                       supplier=info['supplier'],
                                       description=info['description'],
                                       license=info['license'],
                                       purl=info['purl'].format(version) if 'purl' in info else None,
                                       cpe=info.get('cpe'),
                                       url=info.get('url'),
                                       pedigree={'notes': 'Modified by Tinkerforge'} if info.get('patched') else None))

    def add_embedded_data(self):
        for data in EMBEDDED_DATA:
            if data['source'] not in self.compiled_sources:
                continue

            ref = self.add(make_component(f'data/{data["name"]}', data['name'],
                                          type='data',
                                          supplier=TINKERFORGE,
                                          manufacturer=TINKERFORGE,
                                          description=data['description'],
                                          hashes=file_hashes(os.path.join(self.project_dir, data['file'])),
                                          url=data['url']),
                           depended_on_by=self.root_ref)

            for info in data['third_party']:
                self.depend(ref, self.add_third_party(f'data/{data["name"]}/{info["name"]}', info | {'type': 'data'}))

    def add_qca_firmware(self):
        # The proprietary QCA PLC modem firmware is only embedded if it is present
        # in src/modules/iso15118/qca_firmware/ (not part of the public repository).
        source = 'src/modules/iso15118/generated/qca_firmware_nvm.embedded.cpp'

        if source not in self.compiled_sources:
            return

        firmware_dir = os.path.join(self.project_dir, 'src', 'modules', 'iso15118', 'qca_firmware')

        for pattern, name, description, supplier, license in [
            ('*.nvm', 'qca-plc-modem-firmware', 'Firmware for the Qualcomm QCA7006 PLC modem, written to the modem by the firmware '
                                                '(src/modules/iso15118/qca_firmware/)', {'name': 'Qualcomm Technologies, Inc.'}, 'Qualcomm proprietary license'),
            ('*.pib', 'qca-plc-modem-pib', 'Parameter Information Block (PIB) for the QCA7006 PLC modem, written to the modem by the firmware '
                                           '(src/modules/iso15118/qca_firmware/)', TINKERFORGE, None),
        ]:
            paths = sorted(glob.glob(os.path.join(firmware_dir, pattern)))

            if len(paths) != 1:
                warn(f'Expected exactly one {pattern} file in {firmware_dir}, found {len(paths)}')
                continue

            self.add(make_component(f'data/{name}', name,
                                    os.path.splitext(os.path.basename(paths[0]))[0],
                                    type='firmware' if pattern == '*.nvm' else 'data',
                                    supplier=supplier,
                                    description=description,
                                    license=license,
                                    hashes=file_hashes(paths[0])),
                     depended_on_by=self.root_ref)

    def add_tzdata(self):
        source = 'src/modules/ntp/timezones.c'

        if source not in self.compiled_sources:
            return

        m = re.match(r'/\*\s*\n(\d{4}[a-z]);', read_text(os.path.join(self.project_dir, source)))

        if m == None:
            warn(f'Could not find tzdata version in {source}')

        version = m.group(1) if m != None else None

        self.add(make_component(bom_ref('tzdata', version), 'tzdata', version,
                                type='data',
                                supplier={'name': 'IANA', 'url': ['https://www.iana.org/time-zones']},
                                description=f'Time zone database, converted to POSIX TZ strings ({source})',
                                license='Public Domain',
                                url='https://www.iana.org/time-zones',
                                properties={'tf:source-paths': source}),
                 depended_on_by=self.root_ref)

    def add_web_interface(self):
        web_dir = os.path.join(self.project_dir, 'web')
        web = self.inputs['web']
        metadata = self.metadata
        lock = read_json(os.path.join(web_dir, 'package-lock.json'))
        lock_packages = lock['packages']
        package_dirs = [os.path.normpath(os.path.join(self.project_dir, x)) for x in web['package_dirs']]

        if web['source'] != 'esbuild-metafile':
            warn('web/build/meta.json not found, listing all production npm dependencies instead of the bundled ones')

            for key, info in lock_packages.items():
                if key.startswith('node_modules/') and not info.get('dev') and not info.get('devOptional'):
                    package_dirs.append(os.path.join(web_dir, key))

        web_ref = self.add(make_component('web-interface', f'{metadata["firmware_info_product_name"]} Web Interface', self.root['version'],
                                          type='application',
                                          supplier=TINKERFORGE,
                                          manufacturer=TINKERFORGE,
                                          description='Web interface, embedded into the firmware',
                                          license=FIRMWARE_LICENSE,
                                          properties={'tf:bundled-package-source': web['source']}),
                           depended_on_by=self.root_ref)

        refs = {}

        for package_dir in sorted(set(package_dirs)):
            key = os.path.relpath(package_dir, web_dir).replace(os.sep, '/')
            lock_info = lock_packages.get(key, {})

            try:
                package_json = read_json(os.path.join(package_dir, 'package.json'))
            except FileNotFoundError:
                warn(f'{package_dir}/package.json not found')
                package_json = {}

            name = package_json.get('name', lock_info.get('name', key.split('node_modules/')[-1]))
            version = package_json.get('version', lock_info.get('version'))
            license = package_json.get('license', lock_info.get('license'))

            if isinstance(license, dict):
                license = license.get('type')

            hashes = None
            integrity = lock_info.get('integrity')

            if integrity != None and integrity.startswith('sha512-'):
                hashes = [{'alg': 'SHA-512', 'content': base64.b64decode(integrity[len('sha512-'):]).hex()}]

            repository = package_json.get('repository')

            if isinstance(repository, dict):
                repository = repository.get('url')

            if isinstance(repository, str):
                repository = re.sub(r'^git\+', '', repository)
                repository = re.sub(r'^github:', 'https://github.com/', repository)

                if re.fullmatch(r'[\w.-]+/[\w.-]+', repository):
                    repository = f'https://github.com/{repository}'

                repository = normalize_git_url(repository)

            author = package_json.get('author')

            if isinstance(author, dict):
                author = author.get('name')

            purl_name = name.replace('@', '%40', 1) if name.startswith('@') else name
            ref = f'npm:{name}@{version}'

            component = make_component(ref, name, version,
                                       supplier={'name': author} if isinstance(author, str) and len(author) > 0 else None,
                                       description=package_json.get('description'),
                                       license=license,
                                       hashes=hashes,
                                       purl=f'pkg:npm/{purl_name}@{version}',
                                       url=package_json.get('homepage'),
                                       vcs=repository if isinstance(repository, str) and repository.startswith('http') else None)

            if 'resolved' in lock_info:
                component.setdefault('externalReferences', []).append({'type': 'distribution', 'url': lock_info['resolved']})

            refs[key] = self.add(component)

        # Dependency graph between the bundled packages, resolved like Node.js does
        depended_on = set()

        for key, ref in refs.items():
            lock_info = lock_packages.get(key, {})

            for dependency in list(lock_info.get('dependencies', {})) + list(lock_info.get('peerDependencies', {})):
                parts = key.split('/')
                dependency_ref = None

                while True:
                    candidate = '/'.join(parts + ['node_modules', dependency]) if len(parts) > 0 else f'node_modules/{dependency}'

                    if candidate in lock_packages:
                        dependency_ref = refs.get(candidate)
                        break

                    if len(parts) == 0:
                        break

                    # Go up to the next node_modules directory
                    i = len(parts) - 1 - parts[::-1].index('node_modules')
                    parts = parts[:i]

                if dependency_ref != None and dependency_ref != ref:
                    self.depend(ref, dependency_ref)
                    depended_on.add(dependency_ref)

        direct = set(lock_packages.get('', {}).get('dependencies', {}))

        for key, ref in refs.items():
            if ref not in depended_on or key.split('node_modules/')[-1] in direct and key.count('node_modules/') == 1:
                self.depend(web_ref, ref)

        # Vendored code in the web interface is handled by add_vendored, attach it to the web interface instead of the firmware
        for vendored in VENDORED:
            ref = bom_ref(f'vendored/{vendored["name"]}', vendored.get('version'))

            if ref in self.components and all(path.startswith('web/') for path in vendored['paths']):
                self.dependencies[self.root_ref].remove(ref)
                self.depend(web_ref, ref)

    def bom(self):
        metadata = self.metadata
        timestamp = datetime.fromtimestamp(metadata['build_timestamp'], timezone.utc).strftime('%Y-%m-%dT%H:%M:%SZ')
        platform = self.inputs.get('platform')

        if platform != None:
            platform_json = {}

            try:
                platform_json = read_json(os.path.join(platform['dir'], 'platform.json'))
            except (FileNotFoundError, KeyError, TypeError):
                pass

            self.tools.insert(0, make_component(bom_ref(f'platform-{platform["name"]}', platform['version']), f'platform-{platform["name"]}', platform['version'],
                                                type='application',
                                                description='PlatformIO development platform',
                                                license=platform_json.get('license'),
                                                url=normalize_git_url(platform_json.get('repository', {}).get('url'))))

        self.tools.insert(0, make_component('esp32-firmware-sbom', 'esp32-firmware sbom.py', None,
                                            type='application',
                                            supplier=TINKERFORGE,
                                            description='SBOM generator of the esp32-firmware build system'))

        # Generate the serial number from the firmware name, so that rebuilding the SBOM of a build results in the same serial number
        serial_number = uuid.uuid5(uuid.NAMESPACE_URL, f'{self.firmware_url}/{metadata["firmware_basename"]}')

        components = sorted(self.components.values(), key=lambda c: (c['bom-ref']))
        known_refs = {self.root_ref} | set(self.components)
        dependencies = []

        for ref in [self.root_ref] + [c['bom-ref'] for c in components]:
            dependencies.append({'ref': ref, 'dependsOn': [x for x in self.dependencies.get(ref, []) if x in known_refs]})

        return {
            '$schema': 'http://cyclonedx.org/schema/bom-1.6.schema.json',
            'bomFormat': 'CycloneDX',
            'specVersion': '1.6',
            'serialNumber': f'urn:uuid:{serial_number}',
            'version': 1,
            'metadata': {
                'timestamp': timestamp,
                'lifecycles': [{'phase': 'build'}],
                'tools': {'components': self.tools},
                'component': self.root,
                'manufacturer': TINKERFORGE,
                'supplier': TINKERFORGE,
            },
            'components': components,
            'dependencies': dependencies,
        }


def generate(project_dir, build_dir, output_path=None):
    build_dir = os.path.abspath(build_dir)
    builder = SbomBuilder(project_dir, build_dir)
    bom = builder.generate()

    if output_path == None:
        output_path = os.path.join(project_dir, 'build', f'{builder.metadata["firmware_basename"]}.cdx.json')

    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)

    with open(output_path + '.tmp', 'w', encoding='utf-8') as f:
        json.dump(bom, f, indent=2)
        f.write('\n')

    os.replace(output_path + '.tmp', output_path)

    if is_under(os.path.abspath(output_path), project_dir):
        output_path_display = os.path.relpath(os.path.abspath(output_path), project_dir)
    else:
        output_path_display = output_path

    print(f'SBOM written to {output_path_display} ({len(bom["components"])} components)')

    return output_path


def main():
    parser = argparse.ArgumentParser(description='Generate a CycloneDX SBOM for an already built PlatformIO environment')
    parser.add_argument('env', help='PlatformIO environment, e.g. warp4')
    parser.add_argument('-o', '--output', help='output path, default: build/<firmware_basename>.cdx.json')
    args = parser.parse_args()

    project_dir = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.join(project_dir, '.pio', 'build', args.env)

    if not os.path.exists(os.path.join(build_dir, 'metadata.json')):
        print(f'Error: {build_dir} contains no build. Build the environment first, '
              'PlatformIO only keeps the build directory of the most recently built environment', file=sys.stderr)
        sys.exit(1)

    try:
        generate(project_dir, build_dir, args.output)
    except (SbomError, FileNotFoundError) as e:
        print(f'Error: {e}', file=sys.stderr)
        sys.exit(1)


def find_node_package_dir(path):
    parts = path.split(os.sep)

    if 'node_modules' not in parts:
        return None

    i = len(parts) - 1 - parts[::-1].index('node_modules')
    n = 2 if parts[i + 1].startswith('@') else 1

    return os.sep.join(parts[:i + 1 + n])


def collect_web_inputs(project_dir):
    web_dir = os.path.join(project_dir, 'web')
    package_dirs = set()
    inputs = []

    # The esbuild metafile lists every file that went into the JS bundle.
    # web/build always matches the embedded web interface: pio_hooks.py
    # rebuilds web/build whenever the embedded index.html is out of date.
    try:
        meta = read_json(os.path.join(web_dir, 'build', 'meta.json'))
    except FileNotFoundError:
        meta = None

    if meta != None:
        source = 'esbuild-metafile'
        src_tfpp_dir = os.path.join(web_dir, 'src_tfpp')

        for input_path in meta['inputs']:
            path = os.path.normpath(os.path.join(src_tfpp_dir, input_path))
            inputs.append(os.path.relpath(path, project_dir))
            package_dir = find_node_package_dir(path)

            if package_dir != None:
                package_dirs.add(package_dir)
    else:
        source = 'package-lock'

    # SCSS is compiled separately by sass, so collect node_modules references from the SCSS sources
    for scss_path in glob.glob(os.path.join(web_dir, 'src_tfpp', '**', '*.scss'), recursive=True):
        for line in read_text(scss_path).splitlines():
            line = line.strip()

            if line.startswith('//') or 'node_modules/' not in line:
                continue

            parts = line.split('node_modules/', 1)[1].split('/')
            name = '/'.join(parts[:2]) if parts[0].startswith('@') else parts[0]
            package_dir = os.path.join(web_dir, 'node_modules', name)

            if os.path.isdir(package_dir):
                package_dirs.add(os.path.normpath(package_dir))

    return {
        'source': source,
        'package_dirs': sorted(package_dirs),
        'inputs': sorted(inputs),
    }


def remove_build_latest(project_dir, metadata):
    build_latest_dir = os.path.join(project_dir, 'build_latest')

    for path in glob.glob(os.path.join(build_latest_dir, f'{glob.escape(metadata["product_id"])}_firmware*.cdx.json')):
        try:
            os.remove(path)
        except FileNotFoundError:
            pass


def update_build_latest(project_dir, metadata):
    # Same scheme as the .elf and .bin files in merge_firmware_hook.py: build_latest/ only contains the most recent files of each product
    filename = f'{metadata["firmware_basename"]}.cdx.json'
    build_latest_dir = os.path.join(project_dir, 'build_latest')

    os.makedirs(build_latest_dir, exist_ok=True)

    remove_build_latest(project_dir, metadata)

    if sys.platform == 'win32':
        shutil.copy2(os.path.join(project_dir, 'build', filename), os.path.join(build_latest_dir, filename))
    else:
        os.symlink(os.path.join('..', 'build', filename), os.path.join(build_latest_dir, filename))


def register_post_build_action(env):
    build_dir = env.subst('$BUILD_DIR')
    project_dir = env.subst('$PROJECT_DIR')

    try:
        metadata = read_json(os.path.join(build_dir, 'metadata.json'))
    except FileNotFoundError:
        return

    if env.GetProjectOption('custom_create_sbom', 'true') != 'true':
        remove_build_latest(project_dir, metadata)
        return

    def action(**kwargs):
        try:
            platform = env.PioPlatform()
            libs = []

            # Only PlatformIO knows which libraries the LDF resolved, so record them for the SBOM
            for lb in env.GetLibBuilders():
                if not lb.is_dependent:
                    continue

                libs.append({
                    'name': lb.name,
                    'version': lb.version,
                    'path': os.path.realpath(lb.path),
                    'dependencies': [os.path.realpath(dep.path) for dep in lb.depbuilders],
                })

            inputs = {
                'platform': {
                    'name': platform.name,
                    'version': platform.version,
                    'dir': platform.get_dir(),
                },
                'framework_dir': os.path.realpath(platform.get_package_dir('framework-arduinoespressif32')),
                'lib_deps': env.GetProjectOption('lib_deps', []),
                'libs': libs,
                'web': collect_web_inputs(project_dir),
            }

            with open(os.path.join(build_dir, INPUTS_FILENAME), 'w', encoding='utf-8') as f:
                json.dump(inputs, f, indent=4)

            generate(project_dir, build_dir)
            update_build_latest(project_dir, metadata)
        except SbomError as e:
            # Signed builds are release builds and must have an SBOM. Other builds, for example
            # with self-built ESP-IDF libraries or while offline, are built without one.
            if len(metadata['signature_preset']) > 0:
                print(f'Error: Could not generate SBOM: {e}', file=sys.stderr)
                raise

            # Don't leave the SBOM of a previous build next to this build's files
            remove_build_latest(project_dir, metadata)
            warn(f'Skipping SBOM: {e}')
        except:
            traceback.print_exc()
            raise

    # Registered as last post action of the .bin, so that all firmware images in build/ already exist
    env.AddPostAction(f'$BUILD_DIR{os.sep}${{PROGNAME}}.bin', env.VerboseAction(action, f'Generating SBOM build{os.sep}{metadata["firmware_basename"]}.cdx.json'))


if 'Import' in globals():  # loaded by PlatformIO as extra script
    env = None
    Import('env')  # noqa: F821
    register_post_build_action(env)
elif __name__ == '__main__':
    main()
