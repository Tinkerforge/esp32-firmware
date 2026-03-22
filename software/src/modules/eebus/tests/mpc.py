#!/usr/bin/env -S uv run --group tests --script

# Tests for MPC (Monitoring of Power Consumption) use case.

import time

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite
    from ._common import *
else:
    import tinkerforge_util as tfutil

    tfutil.create_parent_module(__file__, "software")
    from software.test_runner.test_context import run_testsuite, TestContext
    from software.src.modules.eebus.tests._common import *



_original_meter_config = None
_meter_slot = 0


def suite_setup(tc: TestContext):
    global _original_meter_config, _meter_slot

    # Look up the charger meter slot from EVSE config
    try:
        evse_meter_config = tc.api("evse/meter_config")
        _meter_slot = evse_meter_config.get("slot", 0)
    except Exception:
        tc.skip("evse/meter_config not available; MPC requires EVSE mode")

    _original_meter_config = tc.api(f"meters/{_meter_slot}/config")

    # Configure the charger meter slot as an API meter
    tc.api(
        f"meters/{_meter_slot}/config_update",
        [
            METER_CLASS_API,
            {
                "display_name": "MPC Test Meter",
                "location": METER_LOCATION_CHARGER,
                "value_ids": METER_VALUE_IDS,
            },
        ],
    )

    reboot_and_wait(tc)
    enable_eebus(tc)


def test_all_values(tc: TestContext):
    """Push meter values and verify they appear in the MPC API state."""

    usecases = tc.api("eebus/usecases")
    if "monitoring_of_power_consumption" not in usecases:
        tc.skip("MPC use case not available on this device")

    tc.api(f"meters/{_meter_slot}/update", METER_VALUES)

    def check_mpc():
        usecases = tc.api("eebus/usecases")
        mpc = usecases["monitoring_of_power_consumption"]

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

    usecases = tc.api("eebus/usecases")
    if "monitoring_of_power_consumption" not in usecases:
        tc.skip("MPC use case not available on this device")

    tc.api(
        f"meters/{_meter_slot}/config_update",
        [
            METER_CLASS_API,
            {
                "display_name": "MPC Negative Test Meter",
                "location": METER_LOCATION_CHARGER,
                "value_ids": VOLTAGE_ONLY_IDS,
            },
        ],
    )

    reboot_and_wait(tc)
    enable_eebus(tc)

    tc.api(f"meters/{_meter_slot}/update", VOLTAGE_ONLY_VALUES)

    def check_mpc_not_activated():
        usecases = tc.api("eebus/usecases")
        mpc = usecases["monitoring_of_power_consumption"]

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
    global _original_meter_config
    if _original_meter_config is not None:
        tc.api(f"meters/{_meter_slot}/config_update", _original_meter_config)
        _original_meter_config = None


if __name__ == "__main__":
    run_testsuite(locals())
