#!/usr/bin/env -S uv run --group tests --script

# End-to-end tests for the evcc integration (evcc's tinkerforge-warp-ws charger template).
#
# evcc runs in a Docker container (host network) on the test machine and connects to the DUT via HTTP and WebSocket.
# Tests that need a vehicle use the testbox (--brickd) and are skipped without it.
# Tests that control charging need an EVSE and are skipped on devices without one (e.g. an ESP32 Ethernet Brick).
#
# Requires Docker. The image can be overridden with the EVCC_TEST_IMAGE environment variable.
# Set EVCC_TEST_KEEP=1 to keep the evcc container running after the suite for debugging.

import os
import time
from urllib.error import HTTPError

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext, MeterValueID
from software.test_runner.docker_container import docker_available
from software.src.modules.evse_common.tests._evcc import Evcc, format_state

METER_CLASS_NONE = 0
METER_CLASS_API = 4
METER_LOCATION_CHARGER = 2

# Value IDs evcc reads from the configured meter (warp.DefaultSchema in evcc).
METER_VALUE_IDS = [
    MeterValueID.PowerActiveLSumImExDiff,
    MeterValueID.EnergyActiveLSumImport,
    MeterValueID.CurrentL1ImExSum,
    MeterValueID.CurrentL2ImExSum,
    MeterValueID.CurrentL3ImExSum,
    MeterValueID.VoltageL1N,
    MeterValueID.VoltageL2N,
    MeterValueID.VoltageL3N,
]
METER_VALUES = [4321.0, 123.456, 6.1, 6.2, 6.3, 230.1, 231.2, 229.3]

# IEC 61851 states as reported in evse/state
IEC_A = 0
IEC_B = 1
IEC_C = 2

EVCC_INTERVAL = 5
# evcc applies changes in its control loop; allow a few cycles.
EVCC_TIMEOUT = 6 * EVCC_INTERVAL

evcc: Evcc | None = None
meter_slot = -1
has_evse = False

original_meter_config: list | None = None
original_external_current: int | None = None
original_phase_auto_switch: bool | None = None


# ---- Helpers -----------------------------------------------------------------

def get_evcc() -> Evcc:
    assert evcc is not None
    return evcc


def find_free_meter_slot(tc: TestContext) -> int:
    slot = 0
    while True:
        try:
            cfg = tc.api(f"meters/{slot}/config")
        except HTTPError as e:
            if e.code == 404:
                return -1
            raise
        if cfg[0] == METER_CLASS_NONE:
            return slot
        slot += 1


def wait_loadpoint(tc: TestContext, check, *, timeout: float = EVCC_TIMEOUT) -> dict:
    """Waits until check(loadpoint) does not raise an AssertionError."""
    def fn() -> dict:
        lp = get_evcc().loadpoint()
        try:
            check(lp)
        except AssertionError as e:
            raise AssertionError(f"{e}\nevcc loadpoint: {format_state(lp)}")
        return lp

    return tc.wait_for(fn, timeout=timeout, poll_delay=1)


def wait_external_current(tc: TestContext, current: int) -> None:
    tc.wait_for(lambda: tc.assert_eq(current, tc.api("evse/external_current")["current"]), timeout=EVCC_TIMEOUT, poll_delay=1)


def assert_no_evcc_errors(tc: TestContext) -> None:
    problems = get_evcc().log_problems()
    if problems:
        raise AssertionError("evcc logged errors:\n" + "\n".join(problems))


def require_evse(tc: TestContext) -> None:
    if not has_evse:
        tc.skip("No EVSE")


def connect_vehicle(tc: TestContext, cp: str, iec_state: int) -> None:
    require_evse(tc)
    tc.get_testbox().set_cp(cp)
    tc.wait_for(lambda: tc.assert_eq(iec_state, tc.api("evse/state")["iec61851_state"]), timeout=10)


# ---- Suite -------------------------------------------------------------------

def suite_setup(tc: TestContext) -> None:
    global evcc, meter_slot, has_evse, original_meter_config, original_external_current, original_phase_auto_switch

    if not docker_available():
        tc.skip("Docker is not available")

    if not tc.device_type().is_warp():
        tc.skip("Not a WARP charger")

    tc.set_test_timeout(10 * 60)

    # Without an EVSE (e.g. an ESP32 Ethernet Brick running a charger firmware) the EVSE API exists, but commands have no effect.
    has_evse = bool(tc.api("info/modules").get("evse_common", False))

    meter_slot = find_free_meter_slot(tc)
    if meter_slot < 0:
        tc.skip("No free meter slot")

    original_meter_config = tc.api(f"meters/{meter_slot}/config")
    original_external_current = tc.api("evse/external_current")["current"]
    try:
        original_phase_auto_switch = tc.api("evse/phase_auto_switch")["enabled"]
    except HTTPError as e:
        if e.code != 404:
            raise

    tc.api(f"meters/{meter_slot}/config_update", [
        METER_CLASS_API,
        {
            "display_name": "evcc Test",
            "location": METER_LOCATION_CHARGER,
            "excluded": False,
            "value_ids": METER_VALUE_IDS,
        },
    ])
    tc.reboot()

    tc.api(f"meters/{meter_slot}/update", METER_VALUES)

    evcc = Evcc(tc.find_free_port(17070), f"http://{tc._esp_host}", meter_slot, interval=EVCC_INTERVAL)
    evcc.start()


def setup(tc: TestContext) -> None:
    get_evcc().set_mode("off")
    get_evcc().set_max_current(16)
    get_evcc().mark_log()


def teardown(tc: TestContext) -> None:
    if has_evse and tc._brickd_host is not None:
        tc.get_testbox().set_cp("A")
        tc.wait_for(lambda: tc.assert_eq(IEC_A, tc.api("evse/state")["iec61851_state"]), timeout=10)


def test_charger_detected(tc: TestContext) -> None:
    lp = wait_loadpoint(tc, lambda lp: tc.assert_false(lp["connected"]))
    tc.assert_eq("Test", lp["title"])

    # evcc disconnects from the charger if it can't initialize it; wait for one more control cycle.
    time.sleep(2 * EVCC_INTERVAL)
    tc.assert_true(get_evcc().container.running())
    assert_no_evcc_errors(tc)


def test_meter_values(tc: TestContext) -> None:
    for values in (METER_VALUES, [v * 1.5 for v in METER_VALUES]):
        tc.api(f"meters/{meter_slot}/update", values)

        def check(lp: dict) -> None:
            tc.assert_epsilon(values[0], 0.5, lp["chargePower"])
            tc.assert_epsilon(values[1], 0.001, lp["chargeTotalImport"])
            tc.assert_true(lp.get("chargeCurrents"))
            tc.assert_true(lp.get("chargeVoltages"))
            for i in range(3):
                tc.assert_epsilon(values[2 + i], 0.01, lp["chargeCurrents"][i])
                tc.assert_epsilon(values[5 + i], 0.1, lp["chargeVoltages"][i])

        wait_loadpoint(tc, check)

    assert_no_evcc_errors(tc)


def test_reconnect_after_reboot(tc: TestContext) -> None:
    tc.api(f"meters/{meter_slot}/update", METER_VALUES)
    wait_loadpoint(tc, lambda lp: tc.assert_epsilon(METER_VALUES[0], 0.5, lp["chargePower"]))

    tc.reboot()

    # evcc reconnects the WebSocket with an exponential backoff of up to 30 s.
    values = [v * 2 for v in METER_VALUES]
    tc.api(f"meters/{meter_slot}/update", values)

    def check(lp: dict) -> None:
        tc.assert_epsilon(values[0], 0.5, lp["chargePower"])

    # Push the values again in case the API meter was not ready yet.
    def push_and_check(lp: dict) -> None:
        tc.api(f"meters/{meter_slot}/update", values)
        check(lp)

    wait_loadpoint(tc, push_and_check, timeout=90)

    # Losing the connection is logged as error; don't check the log here.


def test_phase_auto_switch_disabled(tc: TestContext) -> None:
    if original_phase_auto_switch is None or tc.device_type().is_warp(2):
        tc.skip("No automatic phase switching")

    tc.api("evse/phase_auto_switch_update", {"enabled": True})
    if not tc.api("evse/phase_auto_switch")["enabled"]:
        tc.skip("Automatic phase switching can't be enabled on this device")

    # evcc disables it when it connects to the charger.
    get_evcc().restart()
    tc.wait_for(lambda: tc.assert_false(tc.api("evse/phase_auto_switch")["enabled"]), timeout=EVCC_TIMEOUT, poll_delay=1)

    assert_no_evcc_errors(tc)


def test_vehicle_status(tc: TestContext) -> None:
    connect_vehicle(tc, "B", IEC_B)
    wait_loadpoint(tc, lambda lp: (tc.assert_true(lp["connected"]), tc.assert_false(lp["charging"])))

    get_evcc().set_mode("now")
    connect_vehicle(tc, "C", IEC_C)
    wait_loadpoint(tc, lambda lp: (tc.assert_true(lp["connected"]), tc.assert_true(lp["charging"])))

    connect_vehicle(tc, "A", IEC_A)
    wait_loadpoint(tc, lambda lp: (tc.assert_false(lp["connected"]), tc.assert_false(lp["charging"])))

    assert_no_evcc_errors(tc)


def test_mode_controls_charging(tc: TestContext) -> None:
    connect_vehicle(tc, "B", IEC_B)
    wait_loadpoint(tc, lambda lp: tc.assert_true(lp["connected"]))

    get_evcc().set_mode("now")
    wait_external_current(tc, 16000)
    wait_loadpoint(tc, lambda lp: tc.assert_true(lp["enabled"]))

    get_evcc().set_mode("off")
    wait_external_current(tc, 0)
    wait_loadpoint(tc, lambda lp: tc.assert_false(lp["enabled"]))

    assert_no_evcc_errors(tc)


def test_max_current(tc: TestContext) -> None:
    # The EVSE only enters state C if charging is allowed.
    connect_vehicle(tc, "B", IEC_B)
    get_evcc().set_mode("now")
    wait_external_current(tc, 16000)
    connect_vehicle(tc, "C", IEC_C)

    for current in (10, 6, 13):
        get_evcc().set_max_current(current)
        wait_external_current(tc, current * 1000)
        wait_loadpoint(tc, lambda lp: tc.assert_epsilon(current, 0.01, lp["offeredCurrent"]))

    assert_no_evcc_errors(tc)


def suite_teardown(tc: TestContext) -> None:
    global evcc

    if evcc is not None:
        if os.environ.get("EVCC_TEST_KEEP") == "1":
            tc.dbg(f"Keeping evcc container running at {evcc.base_url}")
        else:
            evcc.stop()
        evcc = None

    if original_phase_auto_switch is not None:
        tc.api("evse/phase_auto_switch_update", {"enabled": original_phase_auto_switch})

    if original_external_current is not None:
        tc.api("evse/external_current_update", {"current": original_external_current})

    if original_meter_config is not None:
        tc.api(f"meters/{meter_slot}/config_update", original_meter_config)
        tc.reboot()


if __name__ == "__main__":
    run_testsuite(locals())
