import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.util import *

#from ....util import *

spec = Union("Zero Phase Decision", 8, variants=[
    Variant('None'),
    Variant('Yes Charge Mode Off'),
    Variant('Yes Waiting For Rotation'),
    Variant('Yes Not Active'),
    Variant('Yes Rotated For B1'),
    Variant('Yes Rotated For Higher Prio'),
    Variant('Yes Phase Overload', [
        Member('Overload mA', Types.U32),
        Member('Phase', Types.U8)
    ]),
    Variant('Yes Unknown'),
])
