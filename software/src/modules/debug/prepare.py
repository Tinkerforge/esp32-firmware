import os
import sys

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

build_dir = util.get_env_metadata()['build_dir']
bootloader_path = f"{build_dir}{os.sep}bootloader.bin"
bootloader_data = b''

try:
    with open(bootloader_path, 'rb') as f:
        bootloader_data = f.read()
except FileNotFoundError:
    print("bootloader.bin not found. Re-run build when done.", file=sys.stderr)

util.embed_data_with_digest(bootloader_data, 'generated', 'embedded_bootloader', 'uint8_t', 'size_t')
