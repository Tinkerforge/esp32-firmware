import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.util import *

#from ....util import *

spec = Union("Three Phase Decision", 20, require_stable_api=False, variants=[
    Variant('None'),
    Variant('Yes Switched To Fixed 3p'),
    Variant('Yes Normal'),
    Variant('Yes Unknown Rot Switchable'),
    Variant('Yes Welcome Charge Until', [
        Member('Timestamp', Types.Uptime)
    ]),
    Variant('Yes Waking Up'),
    Variant('No Phase Minimum', [
        Member('Timestamp', Types.Uptime),
        Member('Required mA', Types.S32),
        Member('Min mA', Types.S32),
        Member('Phase', Types.U8),
    ]),
    Variant('No Phase Improvement', [
        Member('Timestamp', Types.Uptime),
        Member('Allocable mA', Types.S32),
        Member('Available mA', Types.S32),
        Member('Phase', Types.U8),
    ]),
    Variant('No Forced 1p Until', [
        Member('Timestamp', Types.Uptime)
    ]),
    Variant('No Fixed 1p'),
    Variant('No Hysteresis Blocked Until', [
        Member('Timestamp', Types.Uptime)
    ]),
])
