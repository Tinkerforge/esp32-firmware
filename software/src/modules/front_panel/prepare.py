import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

util.embed_bricklet_firmware_bin()

with open('flash_map.bin.xz', 'rb') as f:
    util.embed_data_with_digest(f.read(), '.', 'flash_map_xz', 'uint8_t', 'size_t')
