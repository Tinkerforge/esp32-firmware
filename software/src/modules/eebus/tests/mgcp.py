#!/usr/bin/env -S uv run --group tests --script

# Tests for MGCP (Monitoring of Grid Connection Point) use case.

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


def suite_setup(tc: TestContext):
    global _original_meter_config
    _original_meter_config = tc.api("meters/0/config")

    # Configure meter slot 0 as API meter with Grid location
    tc.api(
        "meters/0/config_update",
        [
            METER_CLASS_API,
            {
                "display_name": "MGCP Test Meter",
                "location": METER_LOCATION_GRID,
                "value_ids": METER_VALUE_IDS,
            },
        ],
    )

    tc.reboot()
    enable_eebus(tc)


def test_all_values(tc: TestContext):
    """Push meter values and verify they appear in the MGCP API state."""

    usecases = tc.api("eebus/usecases")
    if "monitoring_of_grid_connection_point" not in usecases:
        tc.skip("MGCP use case not available on this device")

    tc.api("meters/0/update", METER_VALUES)

    def check_mgcp():
        usecases = tc.api("eebus/usecases")
        mgcp = usecases["monitoring_of_grid_connection_point"]

        tc.assert_eq(6900, mgcp["total_power_w"])

        tc.assert_eq(100000, mgcp["energy_consumed_wh"])
        tc.assert_eq(50000, mgcp["energy_feed_in_wh"])

        tc.assert_eq(10000, mgcp["current_phase_1_ma"])
        tc.assert_eq(11000, mgcp["current_phase_2_ma"])
        tc.assert_eq(9000, mgcp["current_phase_3_ma"])

        tc.assert_eq(230, mgcp["voltage_phase_1_v"])
        tc.assert_eq(231, mgcp["voltage_phase_2_v"])
        tc.assert_eq(229, mgcp["voltage_phase_3_v"])

        tc.assert_eq(50000, mgcp["frequency_mhz"])

        tc.assert_eq(True, mgcp["active"])

    tc.wait_for(check_mgcp, timeout=10)


def test_no_activation_without_power(tc: TestContext):
    """A meter that only provides voltage (no power/energy IDs) must not activate MGCP.

    Reconfigure the meter with only voltage value_ids and reboot.
    After reboot only push voltage values and verify MGCP did *not* activate.
    MGCP requires power + energy_feed_in + energy_consumed to activate.
    """

    usecases = tc.api("eebus/usecases")
    if "monitoring_of_grid_connection_point" not in usecases:
        tc.skip("MGCP use case not available on this device")

    tc.api(
        "meters/0/config_update",
        [
            METER_CLASS_API,
            {
                "display_name": "MGCP Negative Test Meter",
                "location": METER_LOCATION_GRID,
                "value_ids": VOLTAGE_ONLY_IDS,
            },
        ],
    )

    tc.reboot()
    enable_eebus(tc)

    tc.api("meters/0/update", VOLTAGE_ONLY_VALUES)

    def check_mgcp_not_activated():
        usecases = tc.api("eebus/usecases")
        mgcp = usecases["monitoring_of_grid_connection_point"]

        tc.assert_eq(230, mgcp["voltage_phase_1_v"])
        tc.assert_eq(231, mgcp["voltage_phase_2_v"])
        tc.assert_eq(229, mgcp["voltage_phase_3_v"])

        tc.assert_eq(EEBUS_NO_VALUE, mgcp["total_power_w"])

        tc.assert_eq(False, mgcp["active"])

    tc.wait_for(check_mgcp_not_activated, timeout=15)


def suite_teardown(tc: TestContext):
    global _original_meter_config
    if _original_meter_config is not None:
        tc.api("meters/0/config_update", _original_meter_config)
        _original_meter_config = None


if __name__ == "__main__":
    run_testsuite(locals())
