#!/usr/bin/env -S uv run --group tests --script

# LPC (Limitation of Power Consumption) tests with an Energy Guard test peer.
#
# The device under test is the Controllable System. The test peer connects to its SHIP server,
# announces the Energy Guard actor, binds to LoadControl and DeviceConfiguration and writes limits.
# Other trusted EEBUS peers are removed during the suite and restored afterwards.
#
# The tests depend on each other and run in order.

import time
from urllib.error import HTTPError

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext
from software.src.modules.eebus.tests._common import *
from software.src.modules.eebus.tests._eebus_peer import (
    EebusPeer, LpcEnergyGuard, energy_guard_layout, as_list,
    ERROR_NO_ERROR, ERROR_COMMAND_REJECTED, ERROR_BINDING_REQUIRED,
)

peer: EebusPeer | None = None
lpc: LpcEnergyGuard | None = None
removed_peers: list[dict] = []
p14a_config = None


def p14a(tc: TestContext) -> dict | None:
    if p14a_config is None:
        return None
    return tc.api("p14a_enwg/state")


def expect_p14a(tc: TestContext, active: bool, limit_w: int = 0):
    state = p14a(tc)
    if state is None:
        return
    tc.assert_eq(active, state["active"])
    if active:
        tc.assert_eq(limit_w, state["limit_w"])


def expect_ack(tc: TestContext, error_number: int):
    tc.assert_eq(ERROR_NO_ERROR, error_number)


def expect_nack(tc: TestContext, error_number: int, expected: int = ERROR_COMMAND_REJECTED):
    tc.assert_eq(expected, error_number)


def suite_setup(tc: TestContext):
    global peer, lpc, removed_peers, p14a_config
    tc.set_test_timeout(120)

    enable_eebus(tc)
    if "lpc" not in tc.api("eebus/usecases"):
        tc.skip("LPC not available on this device")
    removed_peers = isolate_eebus_peers(tc)

    try:
        p14a_config = tc.api("p14a_enwg/config")
    except HTTPError:
        p14a_config = None
    if p14a_config is not None:
        tc.api("p14a_enwg/config_update", p14a_config | {"enable": True, "source": [P14A_SOURCE_EEBUS, None], "limit_charger": True})

    peer = start_test_peer(tc, energy_guard_layout())
    peer.discover()
    lpc = LpcEnergyGuard(peer)
    lpc.setup()


def suite_teardown(tc: TestContext):
    stop_test_peer(tc, peer)
    restore_eebus_peers(tc, removed_peers)
    if p14a_config is not None:
        tc.api("p14a_enwg/config_update", p14a_config)


def test_heartbeat_subscription(tc: TestContext):
    """The device subscribes to the heartbeat of the Energy Guard's own entity and reads it (LPC 3.4.3, IG-LPC 3.7)."""
    sub = peer.wait_for_heartbeat_subscription()
    tc.assert_eq([1], sub["serverAddress"]["entity"])
    tc.assert_eq(3, sub["serverAddress"]["feature"])
    tc.assert_eq("DeviceDiagnosis", sub["serverFeatureType"])
    tc.assert_true(peer.received("read", "deviceDiagnosisHeartbeatData"))


def test_limit_description(tc: TestContext):
    """The device describes a consumption active power limit that can be changed by the Energy Guard."""
    tc.assert_true(lpc.limit_id is not None)
    limit = lpc.read_limit()
    tc.assert_true(limit["isLimitChangeable"])


def test_controlled(tc: TestContext):
    """Heartbeat followed by a deactivated limit takes control: unlimited/controlled (LPC 2.2)."""
    peer.send_heartbeat()
    expect_ack(tc, lpc.write_limit(0, active=False))
    wait_for_lpc_state(tc, LPC_UNLIMITED_CONTROLLED)
    tc.wait_for(lambda: expect_p14a(tc, False), timeout=3)


def test_failsafe_values(tc: TestContext):
    """Failsafe values are accepted in range and reported back (LPC-021, LPC-022)."""
    since = len(peer.messages)
    expect_ack(tc, lpc.write_failsafe(5000, 2 * 3600))
    state = lpc_state(tc)
    tc.assert_eq(5000, state["failsafe_limit_power_w"])
    tc.assert_eq(7200, state["failsafe_limit_duration_s"])
    peer.wait_for(lambda m: m.classifier == "notify" and m.function == "deviceConfigurationKeyValueListData", since=since, what="failsafe notify")
    tc.assert_eq(5000, lpc.read_failsafe()[lpc.failsafe_power_key]["scaledNumber"]["number"])


def test_failsafe_duration_below_min(tc: TestContext):
    """A failsafe duration below 2 h is rejected and ignored (IG-LPC 3.1)."""
    expect_nack(tc, lpc.write_failsafe(duration=3600))
    tc.assert_eq(7200, lpc_state(tc)["failsafe_limit_duration_s"])


def test_failsafe_duration_above_max(tc: TestContext):
    """A failsafe duration above 24 h is rejected and the maximum is used instead (LPC-022/4, LPC-022/5)."""
    expect_nack(tc, lpc.write_failsafe(duration=25 * 3600))
    tc.assert_eq(86400, lpc_state(tc)["failsafe_limit_duration_s"])
    expect_ack(tc, lpc.write_failsafe(duration=2 * 3600))
    tc.assert_eq(7200, lpc_state(tc)["failsafe_limit_duration_s"])


def test_failsafe_power_negative(tc: TestContext):
    """A negative failsafe consumption limit is rejected (IG-LPC 3.6)."""
    expect_nack(tc, lpc.write_failsafe(power=-100))
    tc.assert_eq(5000, lpc_state(tc)["failsafe_limit_power_w"])


def test_limit(tc: TestContext):
    """An active limit with duration: limited, notified to subscribers and applied to §14a."""
    since = len(peer.messages)
    expect_ack(tc, lpc.write_limit(4200, active=True, duration=300))
    state = wait_for_lpc_state(tc, LPC_LIMITED)
    tc.assert_true(state["limit_active"])
    tc.assert_eq(4200, state["current_limit"])
    tc.assert_gt(280, state["outstanding_duration_s"])
    notify = peer.wait_for(lambda m: m.classifier == "notify" and m.function == "loadControlLimitListData", since=since, what="limit notify")
    limit = [d for d in as_list(notify.data.get("loadControlLimitData")) if d.get("limitId") == lpc.limit_id][0]
    tc.assert_true(limit["isLimitActive"])
    tc.assert_eq(4200, limit["value"]["number"])
    tc.assert_true("endTime" in limit.get("timePeriod", {}))
    tc.wait_for(lambda: expect_p14a(tc, True, 4200), timeout=3)


def test_duration_zero(tc: TestContext):
    """An activated limit with duration PT0S deactivates the limit and is acknowledged (IG-LPC 2.2, 2.16)."""
    expect_ack(tc, lpc.write_limit(4200, active=True, duration="PT0S"))
    state = wait_for_lpc_state(tc, LPC_UNLIMITED_CONTROLLED)
    tc.assert_false(state["limit_active"])
    tc.wait_for(lambda: expect_p14a(tc, False), timeout=3)


def test_no_duration(tc: TestContext):
    """An active limit without duration is valid until further notice."""
    expect_ack(tc, lpc.write_limit(4300, active=True))
    state = wait_for_lpc_state(tc, LPC_LIMITED)
    tc.assert_eq(4300, state["current_limit"])
    tc.assert_eq(0, state["outstanding_duration_s"])


def test_delete_duration(tc: TestContext):
    """A partial delete of the timePeriod removes the duration (LPC 3.4.1.4)."""
    expect_ack(tc, lpc.write_limit(4400, active=True, duration=600))
    tc.assert_gt(580, wait_for_lpc_state(tc, LPC_LIMITED)["outstanding_duration_s"])
    expect_ack(tc, lpc.write_limit(4500, active=True, delete_duration=True))
    state = wait_for_lpc_state(tc, LPC_LIMITED)
    tc.assert_eq(4500, state["current_limit"])
    tc.assert_eq(0, state["outstanding_duration_s"])


def test_partial_write_value_only(tc: TestContext):
    """A partial write that only contains the value keeps the activation state."""
    expect_ack(tc, lpc.write_limit(4600))
    state = wait_for_lpc_state(tc, LPC_LIMITED)
    tc.assert_true(state["limit_active"])
    tc.assert_eq(4600, state["current_limit"])


def test_negative_limit(tc: TestContext):
    """A negative consumption limit is rejected and the old limit is kept (LPC-003/1)."""
    expect_nack(tc, lpc.write_limit(-100, active=True, duration=300))
    state = wait_for_lpc_state(tc, LPC_LIMITED)
    tc.assert_eq(4600, state["current_limit"])


def test_unknown_limit_id(tc: TestContext):
    """A write on an unknown limitId is rejected."""
    expect_nack(tc, lpc.write_limit(1000, active=True, limit_id=99))
    tc.assert_eq(4600, lpc_state(tc)["current_limit"])


def test_write_without_binding(tc: TestContext):
    """A write from a feature without binding is rejected with BindingRequired."""
    unbound = peer.local([1], 4)
    error = peer.write(unbound, lpc.lc_server, "loadControlLimitListData", {"loadControlLimitData": [{"limitId": lpc.limit_id, "isLimitActive": False}]})
    expect_nack(tc, error, ERROR_BINDING_REQUIRED)
    tc.assert_eq(LPC_LIMITED, lpc_state(tc)["usecase_state"])


def test_limit_duration_expires(tc: TestContext):
    """When the duration expires the limit is deactivated and the state is unlimited/controlled."""
    expect_ack(tc, lpc.write_limit(4600, active=True, duration=10))
    wait_for_lpc_state(tc, LPC_LIMITED)
    since = len(peer.messages)
    state = wait_for_lpc_state(tc, LPC_UNLIMITED_CONTROLLED, timeout=20)
    tc.assert_false(state["limit_active"])
    notify = peer.wait_for(lambda m: m.classifier == "notify" and m.function == "loadControlLimitListData", since=since, what="limit notify")
    limit = [d for d in as_list(notify.data.get("loadControlLimitData")) if d.get("limitId") == lpc.limit_id][0]
    tc.assert_false(limit["isLimitActive"])


def test_heartbeat_loss(tc: TestContext):
    """Without heartbeat for 120 s the device enters failsafe and applies the failsafe limit (LPC 2.2, IG-LPC 3.7).
    The device polls the heartbeat while no notifications arrive; the unchanged (stale) heartbeat must not count."""
    tc.set_test_timeout(300)
    peer.send_heartbeat()
    # The duration of this limit expires during failsafe (failsafe after 120 s), see test_leave_failsafe_with_limit
    expect_ack(tc, lpc.write_limit(4200, active=True, duration=125))
    peer.heartbeat_enabled = False
    t0 = time.monotonic()
    since = len(peer.messages)

    time.sleep(max(0, t0 + 100 - time.monotonic()))
    tc.assert_eq(LPC_LIMITED, lpc_state(tc)["usecase_state"])
    # IG-LPC 3.7: After 75 s without notification the device reads the heartbeat
    tc.assert_true(peer.received("read", "deviceDiagnosisHeartbeatData", since))

    time.sleep(max(0, t0 + 125 - time.monotonic()))
    state = wait_for_lpc_state(tc, LPC_FAILSAFE, timeout=10)
    tc.assert_eq(5000, state["current_limit"])
    tc.wait_for(lambda: expect_p14a(tc, True, 5000), timeout=3)


def test_failsafe_write_without_heartbeat(tc: TestContext):
    """In failsafe a write without a heartbeat within 60 s is rejected and the state is kept (IG-LPC 2.14)."""
    expect_nack(tc, lpc.write_limit(0, active=False))
    wait_for_lpc_state(tc, LPC_FAILSAFE)


def test_failsafe_values_in_failsafe(tc: TestContext):
    """Failsafe values are only evaluated after heartbeat and limit (IG-LPC 2.11)."""
    expect_nack(tc, lpc.write_failsafe(power=6000))
    tc.assert_eq(5000, lpc_state(tc)["failsafe_limit_power_w"])


def test_heartbeat_alone_keeps_failsafe(tc: TestContext):
    """A heartbeat alone does not leave failsafe."""
    peer.send_heartbeat()
    time.sleep(2)
    tc.assert_eq(LPC_FAILSAFE, lpc_state(tc)["usecase_state"])


def test_leave_failsafe_with_limit(tc: TestContext):
    """Heartbeat followed by an activated limit without duration leaves failsafe to "limited" (LPC-919).
    The duration of the limit before failsafe expired during failsafe and must not deactivate the new limit."""
    peer.send_heartbeat()
    expect_ack(tc, lpc.write_limit(4300, active=True))
    state = wait_for_lpc_state(tc, LPC_LIMITED)
    tc.assert_eq(4300, state["current_limit"])
    tc.assert_eq(0, state["outstanding_duration_s"])
    tc.wait_for(lambda: expect_p14a(tc, True, 4300), timeout=3)


def test_leave_failsafe(tc: TestContext):
    """Deactivating the limit after leaving failsafe."""
    peer.heartbeat_enabled = True
    expect_ack(tc, lpc.write_limit(0, active=False))
    wait_for_lpc_state(tc, LPC_UNLIMITED_CONTROLLED)
    tc.wait_for(lambda: expect_p14a(tc, False), timeout=3)


def test_disable_eebus_removes_limit(tc: TestContext):
    """Disabling EEBUS removes an active EEBUS limit from §14a. Runs last, as it disconnects the peer."""
    if p14a_config is None:
        tc.skip("p14a_enwg not available")
    expect_ack(tc, lpc.write_limit(4400, active=True))
    wait_for_lpc_state(tc, LPC_LIMITED)
    tc.wait_for(lambda: expect_p14a(tc, True, 4400), timeout=3)
    set_eebus_enabled(tc, False)
    try:
        tc.wait_for(lambda: expect_p14a(tc, False), timeout=5)
    finally:
        set_eebus_enabled(tc, True)
        time.sleep(2)


if __name__ == "__main__":
    run_testsuite(locals())
