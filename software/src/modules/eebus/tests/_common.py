import time
import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import MeterValueID

METER_CLASS_API = 4
METER_LOCATION_CHARGER = 2
METER_LOCATION_GRID = 4

EEBUS_NO_VALUE = -2147483648

# MeterValueID constants matching the mvids[] array in eebus.cpp.
METER_VALUE_IDS = [
    MeterValueID.CurrentL1ImExDiff,
    MeterValueID.CurrentL2ImExDiff,
    MeterValueID.CurrentL3ImExDiff,
    MeterValueID.PowerActiveL1ImExDiff,
    MeterValueID.PowerActiveL2ImExDiff,
    MeterValueID.PowerActiveL3ImExDiff,
    MeterValueID.PowerActiveLSumImExDiff,
    MeterValueID.EnergyActiveLSumImport,
    MeterValueID.EnergyActiveLSumExport,
    MeterValueID.VoltageL1N,
    MeterValueID.VoltageL2N,
    MeterValueID.VoltageL3N,
    MeterValueID.VoltageL1L2,
    MeterValueID.VoltageL2L3,
    MeterValueID.VoltageL3L1,
    MeterValueID.FrequencyLAvg,
]

# Test values matching METER_VALUE_IDS order.
METER_VALUES = [
    10.0,
    11.0,
    9.0,    # Current per phase (A)
    2300.0,
    2530.0,
    2070.0, # Power per phase (W)
    6900.0, # Total power (W)
    100.0,  # Energy import (kWh)
    50.0,   # Energy export (kWh)
    230.0,
    231.0,
    229.0,  # Phase-neutral voltage (V)
    400.0,
    399.0,
    401.0,  # Phase-phase voltage (V)
    50.0,   # Frequency (Hz)
]

VOLTAGE_ONLY_IDS = [MeterValueID.VoltageL1N, MeterValueID.VoltageL2N, MeterValueID.VoltageL3N]
VOLTAGE_ONLY_VALUES = [230.0, 231.0, 229.0]

def enable_eebus(tc):
    config = tc.api("eebus/config")
    config["enable"] = True
    tc.api("eebus/config_update", config)
    time.sleep(1)
