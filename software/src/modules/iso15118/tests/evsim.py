#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import threading

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from _common import EVSim, IsoTestEnvironment
from software.src.modules.meters.generated.meter_value_id import MeterValueID
from software.test_runner.test_context import TestContext, run_testsuite

environment = None

ISO15118_METER_SLOT = 1
EV_DATA_SOURCE_ISO20 = 4
EV_SOC = 42
EV_CAPACITY_WH = 24000
EV_POWER_W = 6900


def suite_setup(tc: TestContext):
    global environment
    environment = IsoTestEnvironment(tc)
    environment.start()


def setup(tc: TestContext):
    assert environment is not None
    environment.reset_session()


def suite_teardown(tc: TestContext):
    if environment is not None:
        environment.stop()


def test_iso20_ac_session(tc: TestContext):
    tc.set_test_timeout(180)
    assert environment is not None

    simulator = EVSim(
        environment.iface,
        charge_loop_cycles=1,
        iso20_soc=EV_SOC,
        iso20_capacity_wh=EV_CAPACITY_WH,
        iso20_power_w=EV_POWER_W,
        pause_after_charge_loop=True,
    )
    simulator_error = []

    def run_simulator():
        try:
            simulator.run(timeout=150)
        except Exception as e:  # noqa: BLE001 - propagate the worker failure on the test thread
            simulator_error.append(e)

    thread = threading.Thread(target=run_simulator, daemon=True)
    thread.start()

    try:
        if not simulator.charge_loop_reached.wait(timeout=120):
            if simulator_error:
                raise simulator_error[0]
            tc.fail("EVSim did not reach an ISO 15118-20 AC ChargeLoop")

        def check_live_state():
            iso20 = tc.api("iso15118/state_iso20")
            tc.assert_eq(14, iso20["state"])
            tc.assert_search(r"^[0-9A-F]{16}$", iso20["session_id"])
            tc.assert_eq("WMIV1234567890ABCDEX", iso20["evcc_id"])
            tc.assert_eq(EV_SOC, iso20["soc"])
            tc.assert_eq(EV_CAPACITY_WH, iso20["energy_capacity"])
            tc.assert_eq(EV_POWER_W, iso20["active_power"])

            ev = tc.api("ev/state")
            tc.assert_eq(EV_SOC, ev["soc"])
            tc.assert_eq(EV_CAPACITY_WH / 1000, ev["capacity"])

            meter_config = tc.api(f"meters/{ISO15118_METER_SLOT}/config")
            tc.assert_eq(9, meter_config[0])
            meter_state = tc.api(f"meters/{ISO15118_METER_SLOT}/state")
            tc.assert_eq(EV_DATA_SOURCE_ISO20, meter_state["source"])
            value_ids = tc.api(f"meters/{ISO15118_METER_SLOT}/value_ids")
            values = tc.api(f"meters/{ISO15118_METER_SLOT}/values")
            meter_values = dict(zip(value_ids, values))
            tc.assert_eq(EV_SOC, meter_values[MeterValueID.StateOfCharge])
            tc.assert_eq(EV_CAPACITY_WH / 1000, meter_values[MeterValueID.Capacity])
            tc.assert_eq(EV_POWER_W, meter_values[MeterValueID.PowerDCChaDisSum])

        tc.wait_for(check_live_state, timeout=5)
    finally:
        simulator.resume_charge_loop()
        thread.join(timeout=30)

    tc.assert_false(thread.is_alive())
    if simulator_error:
        raise simulator_error[0]

    state = tc.api("iso15118/state_common")
    tc.assert_eq("urn:iso:std:iso:15118:-20:AC", state["protocol"])
    tc.assert_eq(["urn:iso:std:iso:15118:-20:AC"], state["supported_protocols"])

    def check_session_stopped():
        iso20 = tc.api("iso15118/state_iso20")
        tc.assert_eq(11, iso20["state"])
        tc.assert_eq(EV_SOC, iso20["soc"])
        tc.assert_eq(EV_CAPACITY_WH, iso20["energy_capacity"])
        tc.assert_eq(EV_POWER_W, iso20["active_power"])

    tc.wait_for(check_session_stopped, timeout=5)

    # SessionStop ends HLC but does not represent a physical unplug. EV and meter
    # values remain available until State A, which debug mode cannot inject.
    ev = tc.api("ev/state")
    tc.assert_eq(EV_SOC, ev["soc"])
    tc.assert_eq(EV_CAPACITY_WH / 1000, ev["capacity"])
    meter_state = tc.api(f"meters/{ISO15118_METER_SLOT}/state")
    tc.assert_eq(EV_DATA_SOURCE_ISO20, meter_state["source"])
    value_ids = tc.api(f"meters/{ISO15118_METER_SLOT}/value_ids")
    values = tc.api(f"meters/{ISO15118_METER_SLOT}/values")
    meter_values = dict(zip(value_ids, values))
    tc.assert_eq(EV_SOC, meter_values[MeterValueID.StateOfCharge])
    tc.assert_eq(EV_CAPACITY_WH / 1000, meter_values[MeterValueID.Capacity])
    tc.assert_eq(EV_POWER_W, meter_values[MeterValueID.PowerDCChaDisSum])


if __name__ == "__main__":
    run_testsuite(locals())
