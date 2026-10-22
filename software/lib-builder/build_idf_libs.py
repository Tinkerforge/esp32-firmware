#!/usr/bin/env -S /bin/sh -c 'uv run --project "$(dirname "$0")" --script "$0" "$@"'

import shutil
import subprocess
import sys
import os
import tinkerforge_util as tfutil

if len(sys.argv) != 3:
    print("Usage: {} path/to/esp32-arduino-lib-builder path/to/defconfig".format(sys.argv[0]))
    sys.exit(1)

root = sys.argv[1]

if not os.path.isfile(os.path.join(root, "configs", "defconfig.esp32")):
    print("defconfig not found. Is the lib-builder root correct?")
    sys.exit(1)

shutil.move(os.path.join(root, "configs", "defconfig.esp32"), os.path.join(root, "configs", "defconfig.esp32vanilla"))
try:
    shutil.copy(os.path.join(sys.argv[2]), os.path.join(root, "configs", "defconfig.esp32"))
    with tfutil.ChangedDirectory(os.path.join(root)):
        env = os.environ

        # Drop uv environment
        env['PATH'] = ':'.join(path for path in env['PATH'].split(':') if not path.startswith(env['VIRTUAL_ENV']))
        del env['UV']
        del env['UV_RUN_RECURSION_DEPTH']
        del env['VIRTUAL_ENV']

        # Update embedded certificate bundle because the repository version is usually outdated.
        # Use curl with ETags to avoid bothering the source too much.
        curl_path = shutil.which('curl')
        if not curl_path:
            print("Not updating embedded certificate bundle: System curl not available", file=sys.stderr)
        else:
            bundle_dir  = os.path.join('esp-idf', 'components', 'mbedtls', 'esp_crt_bundle')
            etag_path   = os.path.join(bundle_dir, 'cacrt_all_etag.txt')
            bundle_path = os.path.join(bundle_dir, 'cacrt_all.pem')

            subprocess.check_call([curl_path, '-sS', '--etag-compare', etag_path, '--etag-save', etag_path, '-o', bundle_path, 'https://curl.se/ca/cacert.pem'], stdin=sys.stdin, stdout=sys.stdout, stderr=sys.stderr, env=env)

        subprocess.call(['bash', '-c', '. esp-idf/export.sh && ./build.sh -s -t esp32'], stdin=sys.stdin, stdout=sys.stdout, stderr=sys.stderr, env=env)

finally:
    shutil.move(os.path.join(root, "configs", "defconfig.esp32vanilla"), os.path.join(root, "configs", "defconfig.esp32"))
