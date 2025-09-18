import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.util import *

#from ....util import *

spec = Union("Zero Phase Decision", 20, variants=[
    Variant('None'),
    Variant('Yes Charge Mode Off'),
    Variant('Yes Waiting For Rotation', [
        Member('Next Rotation', Types.Uptime)
    ]),
    Variant('Yes Not Active'),
    Variant('Yes Rotated For B1'),
    Variant('Yes Rotated For Higher Prio'),
    Variant('Yes Phase Overload', [
        Member('Timestamp', Types.Uptime),
        Member('Overload mA', Types.S32),
        Member('Phase', Types.U8)
    ]),
    Variant('No Cloud Filter Blocks Until', [
        Member('Timestamp', Types.Uptime),
        Member('Max PV mA', Types.S32),
    ]),
    Variant('No Hysteresis Blocks Until', [
        Member('Timestamp', Types.Uptime)
    ]),
])
