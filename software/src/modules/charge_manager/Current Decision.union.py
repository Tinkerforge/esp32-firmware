import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.util import *

#from ....util import *

spec = Union("Current Decision", 4, variants=[
    Variant('None'),
    Variant('Enable Not Charging'),
    Variant('Phase Limit', [
        Member('Unknown Rotation', Types.Bool)
    ]),
    Variant('Requested'),
    Variant('Fair', [ # TODO split fair in fair pv (without unknown rot) and fair phase (with unknown rot)
        Member('Unknown Rotation', Types.Bool)
    ]),
    Variant('Guaranteed PV'),
    Variant('Left Over', [
        Member('Unknown Rotation', Types.Bool)
    ]),
])
