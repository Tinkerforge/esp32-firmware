#!/usr/bin/env -S uv run --group tests --script

# MPC notification rate tests with a Monitoring Appliance test peer.
#
# A charger meter updates all its values about once per second. The device coalesces them into one notification and
# sends at most one every 5 seconds (one notification per value and second overloaded the SMA Sunny Home Manager 2.0).
#
# The charger meter slot is reconfigured as an API meter (reboot), restored afterwards (reboot).
# Other trusted EEBUS peers are removed during the suite and restored afterwards.

import time

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext
from software.src.modules.eebus.tests._common import *
from software.src.modules.eebus.tests._eebus_peer import (
    EebusPeer, Layout, EntitySpec, FeatureSpec, UseCaseSpec, HEARTBEAT_FUNCTIONS, as_list, ERROR_NO_ERROR,
)

NOTIFY_INTERVAL_S = 5

peer: EebusPeer | None = None
removed_peers: list[dict] = []
original_meter_config = None
meter_slot = 0
client: dict | None = None
server: dict | None = None


def monitoring_appliance_layout() -> Layout:
    e = [1]
    return Layout(
        device_type="EnergyManagementSystem",
        entities=[EntitySpec(e, "CEM")],
        features=[
            FeatureSpec(e, 1, "Measurement", "client"),
            FeatureSpec(e, 2, "ElectricalConnection", "client"),
            FeatureSpec(e, 3, "DeviceDiagnosis", "server", dict(HEARTBEAT_FUNCTIONS)),
        ],
        use_cases=[UseCaseSpec(e, "MonitoringAppliance", "monitoringOfPowerConsumption")],
    )


def push_meter(tc: TestContext, total_power_w: float):
    values = list(METER_VALUES)
    values[6] = total_power_w
    tc.api(f"meters/{meter_slot}/update", values)


def notifies(since: int) -> list:
    return [m for m in peer.received("notify", "measurementListData", since) if m.source.get("feature") == server["feature"]]


def total_power_of(msg) -> int | None:
    for d in as_list(msg.data.get("measurementData")):
        if d.get("measurementId") == total_power_id:
            return d["value"]["number"] * 10 ** d["value"].get("scale", 0)
    return None


total_power_id: int | None = None


def suite_setup(tc: TestContext):
    global peer, removed_peers, original_meter_config, meter_slot, client, server, total_power_id
    tc.set_test_timeout(180)

    try:
        meter_slot = tc.api("evse/meter_config").get("slot", 0)
    except Exception:
        tc.skip("evse/meter_config not available; MPC requires a charger")

    original_meter_config = tc.api(f"meters/{meter_slot}/config")
    tc.api(f"meters/{meter_slot}/config_update", [METER_CLASS_API, {"display_name": "MPC Notify Test Meter", "location": METER_LOCATION_CHARGER, "excluded": False, "value_ids": METER_VALUE_IDS}])
    tc.reboot()
    enable_eebus(tc)
    if "mpc" not in tc.api("eebus/usecases"):
        tc.skip("MPC not available on this device")

    # MPC activates with the first power value
    push_meter(tc, 1000)
    tc.wait_for(lambda: tc.assert_true(tc.api("eebus/usecases")["mpc"]["active"]), timeout=10)

    removed_peers = isolate_eebus_peers(tc)
    peer = start_test_peer(tc, monitoring_appliance_layout())
    peer.discover()
    uc = peer.find_remote_use_case("MonitoredUnit", "monitoringOfPowerConsumption")
    tc.assert_true(uc is not None)
    entity = uc.get("address", {}).get("entity")
    client = peer.local([1], 1)
    server = peer.find_remote_feature("Measurement", "server", entity)
    tc.assert_eq(ERROR_NO_ERROR, peer.subscribe(client, server, "Measurement"))

    desc = peer.read(client, server, "measurementDescriptionListData")
    for d in as_list(desc.get("measurementDescriptionData")):
        if d.get("scopeType") == "acPowerTotal":
            total_power_id = d.get("measurementId")
    tc.assert_true(total_power_id is not None)


def suite_teardown(tc: TestContext):
    stop_test_peer(tc, peer)
    restore_eebus_peers(tc, removed_peers)
    if original_meter_config is not None:
        tc.api(f"meters/{meter_slot}/config_update", original_meter_config)
        tc.reboot()


def test_coalesced_and_rate_limited(tc: TestContext):
    """Meter updates every second for 20 s: at most one notification per 5 s instead of five per second."""
    time.sleep(NOTIFY_INTERVAL_S + 1)  # Let a pending notification of the setup pass
    since = len(peer.messages)
    t0 = time.monotonic()
    power = 2000
    while time.monotonic() - t0 < 20:
        power += 10
        push_meter(tc, power)
        time.sleep(1)
    time.sleep(NOTIFY_INTERVAL_S + 1)
    msgs = notifies(since)
    tc.dbg(f"{len(msgs)} measurementListData notifications in {time.monotonic() - t0:.1f} s")
    # 26 s with one notification per 5 s, the first one immediately: 6 at most. Before: 5 per meter update, i.e. 100.
    tc.assert_true(2 <= len(msgs) <= 6)
    gaps = [b.time - a.time for a, b in zip(msgs, msgs[1:])]
    tc.assert_true(all(g >= NOTIFY_INTERVAL_S - 0.5 for g in gaps))
    # The last notification has the last value: Nothing is lost by coalescing
    tc.assert_eq(power, total_power_of(msgs[-1]))


def test_single_update_notified_promptly(tc: TestContext):
    """After a quiet period, a single change is notified at once (not only after the interval)."""
    time.sleep(NOTIFY_INTERVAL_S + 1)
    since = len(peer.messages)
    push_meter(tc, 4321)
    t0 = time.monotonic()
    msg = peer.wait_for(lambda m: m.classifier == "notify" and m.function == "measurementListData" and total_power_of(m) == 4321, since=since, timeout=5, what="notify with new power")
    tc.assert_true(msg.time - t0 < 2)
    time.sleep(1)
    tc.assert_eq(1, len(notifies(since)))


if __name__ == "__main__":
    run_testsuite(locals())
