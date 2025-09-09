import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.util import *

#from ....util import *

spec = Union("Three Phase Decision", 20, variants=[
    Variant('None'),
    Variant('Yes Welcome Charge Until', [
        Member('Timestamp', Types.Timestamp)
    ]),
    Variant('No Phase Minimum', [
        Member('Timestamp', Types.Timestamp),
        Member('Required mA', Types.S32),
        Member('Min mA', Types.S32),
        Member('Phase', Types.U8),
    ]),
    Variant('No Phase Improvement', [
        Member('Timestamp', Types.Timestamp),
        Member('Allocable mA', Types.S32),
        Member('Available mA', Types.S32),
        Member('Phase', Types.U8),
    ]),
    Variant('Yes Improves Spread'),
    Variant('No Forced 1p Until', [
        Member('Timestamp', Types.Timestamp)
    ]),
    Variant('No Fixed 1p'),
    Variant('Yes Unknown Rot Switchable'),
    Variant('No Hysteresis Blocked Until', [
        Member('Timestamp', Types.Timestamp)
    ]),
    Variant('No Phase Switch Blocked Until', [
        Member('Timestamp', Types.Timestamp)
    ]),
    Variant('Yes Waking Up'),
    Variant('Yes Normal'),
])
