import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

util.file_to_embedded_ts('underconstruction.gif')
