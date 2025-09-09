import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.util import *

#from ....util import *

spec = Union("Global Decision", 16, variants=[
    Variant('None'),
    Variant('PV Excess Overloaded Hysteresis Blocks Until', [
        Member('Timestamp', Types.Timestamp),
        Member('Overload mA', Types.S32),
    ]),
    Variant('Hysteresis Elapses At', [
        Member('Timestamp', Types.Timestamp)
    ]),
    Variant('Next Rotation At', [
        Member('Timestamp', Types.Timestamp)
    ]),
])
