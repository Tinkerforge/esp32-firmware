import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

util.embed_bricklet_firmware_bin()
