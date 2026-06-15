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
    print("bootloader.bin for Debug module not found. Re-run build when done.", file=sys.stderr)

util.embed_data_with_digest(bootloader_data, 'generated', 'embedded_bootloader', 'uint8_t', 'size_t')

partition_table_path = f"{build_dir}{os.sep}partitions.bin"
partition_table_data = b''

try:
    with open(partition_table_path, 'rb') as f:
        partition_table_data = f.read()
except FileNotFoundError:
    print("partitions.bin for Debug module not found. Re-run build when done.", file=sys.stderr)

for i in range(len(partition_table_data), 0, -1):
    if partition_table_data[i - 1] != 0xFF:
        partition_table_length = (i + 32 + 31) & ~31 # At least 32 0xFF bytes up to the next multiple of 32
        partition_table_data = partition_table_data[:partition_table_length]
        break

util.embed_data_with_digest(partition_table_data, 'generated', 'embedded_partition_table', 'uint8_t', 'size_t')
