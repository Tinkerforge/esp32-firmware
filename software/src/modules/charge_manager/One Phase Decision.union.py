import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.util import *

#from ....util import *

spec = Union("One Phase Decision", 12, variants=[
    Variant('None'),
    Variant('Yes Welcome Charge Until', [
        Member('Timestamp', Types.Timestamp)
    ]),
    Variant('No Phase Minimum', [
        Member('Required mA', Types.U32),
        Member('Min mA', Types.U32),
        Member('Phase', Types.U8)
    ]),
    Variant('No PV Improvement'),
    Variant('No Phase Improvement'),
    Variant('Yes Improves Spread'),
    Variant('No Forced 3p Until', [
        Member('Timestamp', Types.Timestamp)
    ]),
    Variant('No Fixed 3p'),
    Variant('Yes Waking Up'),
    Variant('Yes Normal'),
    Variant('No Hysteresis Blocked Until', [
        Member('Timestamp', Types.Timestamp)
    ]),
])
