import time

METER_CLASS_API = 4
METER_LOCATION_CHARGER = 2
METER_LOCATION_GRID = 4

EEBUS_NO_VALUE = -2147483648

# MeterValueID constants matching the mvids[] array in eebus.cpp.
METER_VALUE_IDS = [
    14,  # CurrentL1ImExDiff (A)
    18,  # CurrentL2ImExDiff (A)
    22,  # CurrentL3ImExDiff (A)
    39,  # PowerActiveL1ImExDiff (W)
    48,  # PowerActiveL2ImExDiff (W)
    57,  # PowerActiveL3ImExDiff (W)
    74,  # PowerActiveLSumImExDiff (W)
    209, # EnergyActiveLSumImport (kWh)
    211, # EnergyActiveLSumExport (kWh)
    1,   # VoltageL1N (V)
    2,   # VoltageL2N (V)
    3,   # VoltageL3N (V)
    4,   # VoltageL1L2 (V)
    5,   # VoltageL2L3 (V)
    6,   # VoltageL3L1 (V)
    364, # FrequencyLAvg (Hz)
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

VOLTAGE_ONLY_IDS = [1, 2, 3] # VoltageL1N, VoltageL2N, VoltageL3N
VOLTAGE_ONLY_VALUES = [230.0, 231.0, 229.0]

def reboot_and_wait(tc):
    try:
        tc.api("reboot", None)
    except Exception:
        pass
    time.sleep(5)
    for _ in range(60):
        try:
            tc.http_request("GET", "eebus/state", timeout=2, parse=True)
            return
        except Exception:
            time.sleep(1)
    tc.fail("Device did not come back after reboot")

def enable_eebus(tc):
    config = tc.api("eebus/config")
    config["enable"] = True
    tc.api("eebus/config_update", config)
    time.sleep(1)
