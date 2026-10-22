#!/usr/bin/env -S uv run --group tests --script

# Tests for MPC (Monitoring of Power Consumption) use case.

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext
from software.src.modules.eebus.tests._common import *

original_meter_config = None
meter_slot = 0


def suite_setup(tc: TestContext):
    global original_meter_config, meter_slot

    # Look up the charger meter slot from EVSE config
    try:
        evse_meter_config = tc.api("evse/meter_config")
        meter_slot = evse_meter_config.get("slot", 0)
    except Exception:
        tc.skip("evse/meter_config not available; MPC requires EVSE mode")

    original_meter_config = tc.api(f"meters/{meter_slot}/config")
    enable_eebus(tc)
    if "mpc" not in tc.api("eebus/usecases"):
        tc.skip("MPC use case not available on this device")


def test_all_values(tc: TestContext):
    """Push meter values and verify they appear in the MPC API state."""

    configure_api_meter(tc, meter_slot, "MPC Test Meter", METER_LOCATION_CHARGER, METER_VALUE_IDS)
    tc.api(f"meters/{meter_slot}/update", METER_VALUES)

    def check_mpc():
        usecases = tc.api("eebus/usecases")
        mpc = usecases["mpc"]

        tc.assert_eq(6900, mpc["total_power_w"])

        tc.assert_eq(2300, mpc["power_phase_1_w"])
        tc.assert_eq(2530, mpc["power_phase_2_w"])
        tc.assert_eq(2070, mpc["power_phase_3_w"])

        tc.assert_eq(100000, mpc["energy_consumed_wh"])
        tc.assert_eq(50000, mpc["energy_produced_wh"])

        tc.assert_eq(10000, mpc["current_phase_1_ma"])
        tc.assert_eq(11000, mpc["current_phase_2_ma"])
        tc.assert_eq(9000, mpc["current_phase_3_ma"])

        tc.assert_eq(230, mpc["voltage_phase_1_v"])
        tc.assert_eq(231, mpc["voltage_phase_2_v"])
        tc.assert_eq(229, mpc["voltage_phase_3_v"])

        tc.assert_eq(400, mpc["voltage_phase_1_2_v"])
        tc.assert_eq(399, mpc["voltage_phase_2_3_v"])
        tc.assert_eq(401, mpc["voltage_phase_3_1_v"])

        tc.assert_eq(50000, mpc["frequency_mhz"])

        tc.assert_eq(True, mpc["active"])

    tc.wait_for(check_mpc, timeout=10)


def test_no_activation_without_power(tc: TestContext):
    """A meter that only provides voltage (no power IDs) must not activate MPC.

    Reconfigure the meter with only voltage value_ids and reboot.
    After reboot only push voltage values and verify MPC did *not* activate.
    """

    configure_api_meter(tc, meter_slot, "MPC Test Meter", METER_LOCATION_CHARGER, VOLTAGE_ONLY_IDS)
    tc.api(f"meters/{meter_slot}/update", VOLTAGE_ONLY_VALUES)

    def check_mpc_not_activated():
        usecases = tc.api("eebus/usecases")
        mpc = usecases["mpc"]

        tc.assert_eq(230, mpc["voltage_phase_1_v"])
        tc.assert_eq(231, mpc["voltage_phase_2_v"])
        tc.assert_eq(229, mpc["voltage_phase_3_v"])

        tc.assert_eq(EEBUS_NO_VALUE, mpc["total_power_w"])

        tc.assert_eq(EEBUS_NO_VALUE, mpc["power_phase_1_w"])
        tc.assert_eq(EEBUS_NO_VALUE, mpc["power_phase_2_w"])
        tc.assert_eq(EEBUS_NO_VALUE, mpc["power_phase_3_w"])

        tc.assert_eq(False, mpc["active"])

    tc.wait_for(check_mpc_not_activated, timeout=15)


def suite_teardown(tc: TestContext):
    global original_meter_config
    if original_meter_config is not None:
        tc.api(f"meters/{meter_slot}/config_update", original_meter_config)
        original_meter_config = None
        tc.reboot()


if __name__ == "__main__":
    run_testsuite(locals())
