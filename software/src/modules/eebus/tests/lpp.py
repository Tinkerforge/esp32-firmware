#!/usr/bin/env -S uv run --group tests --script

# LPP (Limitation of Power Production) tests with an Energy Guard test peer, including the EEBUS LPP automation trigger.
#
# The device under test is the Controllable System (energy manager). The test peer connects to its SHIP server,
# announces the Energy Guard actor of LPP, binds to LoadControl and DeviceConfiguration and writes limits.
# Other trusted EEBUS peers are removed during the suite and restored afterwards.
#
# Every test starts with a connected and bound Energy Guard that controls the device without a limit
# ("unlimited/controlled"), with the failsafe values BASE_FAILSAFE and with two automation rules
# (production limited / not limited -> print) that did not fire yet, see setup().

import time
from urllib.error import HTTPError

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext
from software.src.modules.eebus.tests._common import *
from software.src.modules.eebus.tests._eebus_peer import (
    EebusPeer, LppEnergyGuard, energy_guard_layout, as_list,
    ERROR_NO_ERROR, ERROR_COMMAND_REJECTED,
)

TRIGGER_EEBUS_LPP = 27
ACTION_PRINT = 1

RULE_LIMITED = 0
RULE_NOT_LIMITED = 1

LPP_LAYOUT_USE_CASES = ("limitationOfPowerProduction",)

peer: EebusPeer | None = None
lpp: LppEnergyGuard | None = None
BASE_FAILSAFE = (5000, 2 * 3600)  # (power W, duration s)
removed_peers: list[dict] = []
original_failsafe: tuple[int, int] | None = None  # (power W, duration s), restored after the suite, as they are persistent
original_automation_config: dict | None = None


def lpp_state(tc: TestContext) -> dict:
    return tc.api("eebus/usecases")["lpp"]


def wait_for_lpp_state(tc: TestContext, state: int, timeout: float = 5.0) -> dict:
    def check():
        s = lpp_state(tc)
        tc.assert_eq(state, s["usecase_state"])
        return s
    return tc.wait_for(check, timeout=timeout, poll_delay=0.5)


def expect_ack(tc: TestContext, error_number: int):
    tc.assert_eq(ERROR_NO_ERROR, error_number)


def expect_nack(tc: TestContext, error_number: int, expected: int = ERROR_COMMAND_REJECTED):
    tc.assert_eq(expected, error_number)


# ---------------------------------------------------------------------------
# Automation
# ---------------------------------------------------------------------------

def get_last_run(tc: TestContext) -> list[int]:
    return tc.api("automation/state")["last_run"]


def set_rules(tc: TestContext):
    """Two fresh rules that did not fire yet: production limited -> print, not limited -> print."""
    tc.api("automation/config_update", {"tasks": []})
    tc.wait_for(lambda: tc.assert_eq([], get_last_run(tc)))
    tc.api("automation/config_update", {"tasks": [
        {
            "trigger": [TRIGGER_EEBUS_LPP, {"limited": True}],
            "action": [ACTION_PRINT, {"message": "LPP test: production limited"}],
            "delay": 0,
        },
        {
            "trigger": [TRIGGER_EEBUS_LPP, {"limited": False}],
            "action": [ACTION_PRINT, {"message": "LPP test: production not limited"}],
            "delay": 0,
        },
    ]})
    tc.wait_for(lambda: tc.assert_eq([0, 0], get_last_run(tc)))


def wait_fired(tc: TestContext, rule: int, since: int = 0, timeout: float = 5.0) -> int:
    def check() -> int:
        last_run = get_last_run(tc)[rule]
        tc.assert_gt(since, last_run)
        return last_run
    return tc.wait_for(check, timeout=timeout, poll_delay=0.5)


# ---------------------------------------------------------------------------
# Energy Guard
# ---------------------------------------------------------------------------

def connect_guard(tc: TestContext):
    global peer, lpp
    peer = start_test_peer(tc, energy_guard_layout(LPP_LAYOUT_USE_CASES))
    peer.discover()
    lpp = LppEnergyGuard(peer)
    lpp.setup()


def stop_guard(tc: TestContext, guard: EebusPeer | None):
    if guard is None:
        return
    try:
        stop_test_peer(tc, guard)
    except HTTPError:
        pass  # Already removed, e.g. by a reboot
    time.sleep(2)  # Let the device remove the bindings of the closed peer


def disconnect_guard(tc: TestContext):
    global peer, lpp
    stop_guard(tc, peer)
    peer = None
    lpp = None


def take_control(tc: TestContext, guard_lpp: LppEnergyGuard):
    """Heartbeat followed by a deactivated limit without duration: unlimited/controlled (LPP 2.2), also from failsafe."""
    since = len(guard_lpp.peer.messages)
    guard_lpp.peer.send_heartbeat()
    expect_ack(tc, guard_lpp.write_limit(0, active=False, duration="PT0S"))
    wait_for_lpp_state(tc, LPC_UNLIMITED_CONTROLLED)
    guard_lpp.wait_for_limit_notify(active=False, value=0, since=since)


def suite_setup(tc: TestContext):
    global removed_peers, original_failsafe, original_automation_config
    tc.set_test_timeout(120)

    enable_eebus(tc)
    if "lpp" not in tc.api("eebus/usecases"):
        tc.skip("LPP not available on this device")
    state = lpp_state(tc)
    original_failsafe = (state["failsafe_limit_power_w"], state["failsafe_limit_duration_s"])
    original_automation_config = tc.api("automation/config")
    removed_peers = isolate_eebus_peers(tc)


def setup(tc: TestContext):
    if not tc.api("eebus/config")["enable"]:
        set_eebus_enabled(tc, True)
        time.sleep(2)
    if peer is None or not peer.is_connected():
        disconnect_guard(tc)
        connect_guard(tc)
    peer.heartbeat_enabled = True
    take_control(tc, lpp)
    state = lpp_state(tc)
    if (state["failsafe_limit_power_w"], state["failsafe_limit_duration_s"]) != BASE_FAILSAFE:
        expect_ack(tc, lpp.write_failsafe(*BASE_FAILSAFE))
    set_rules(tc)


def restore_failsafe(tc: TestContext):
    """The failsafe values are stored persistently. Write the values from before the suite back."""
    if original_failsafe is None:
        return
    state = lpp_state(tc)
    if (state["failsafe_limit_power_w"], state["failsafe_limit_duration_s"]) == original_failsafe:
        return
    guard = start_test_peer(tc, energy_guard_layout(LPP_LAYOUT_USE_CASES))
    try:
        guard.discover()
        guard_lpp = LppEnergyGuard(guard)
        guard_lpp.setup()
        take_control(tc, guard_lpp)
        expect_ack(tc, guard_lpp.write_failsafe(*original_failsafe))
    finally:
        stop_test_peer(tc, guard)


def suite_teardown(tc: TestContext):
    disconnect_guard(tc)
    if not tc.api("eebus/config")["enable"]:
        set_eebus_enabled(tc, True)
        time.sleep(2)
    restore_failsafe(tc)
    restore_eebus_peers(tc, removed_peers)
    if original_automation_config is not None:
        tc.api("automation/config_update", original_automation_config)


# ---------------------------------------------------------------------------
# Use case
# ---------------------------------------------------------------------------

def test_limit_description(tc: TestContext):
    """The device describes a production active power limit that can be changed by the Energy Guard (LPP-011).
    Its value is <= 0 W (LPP-001)."""
    limit = lpp.read_limit()
    tc.assert_true(limit["isLimitChangeable"])
    tc.assert_le(0, limit["value"]["number"])


def test_failsafe_values(tc: TestContext):
    """Failsafe values are accepted in range and reported back (LPP-021, LPP-022). The failsafe limit is >= 0 W (LPP-010)."""
    since = len(peer.messages)
    expect_ack(tc, lpp.write_failsafe(5500, 3 * 3600))
    state = lpp_state(tc)
    tc.assert_eq(5500, state["failsafe_limit_power_w"])
    tc.assert_eq(3 * 3600, state["failsafe_limit_duration_s"])
    peer.wait_for(lambda m: m.classifier == "notify" and m.function == "deviceConfigurationKeyValueListData", since=since, what="failsafe notify")
    tc.assert_eq(5500, lpp.read_failsafe()[lpp.failsafe_power_key]["scaledNumber"]["number"])


def test_failsafe_power_negative(tc: TestContext):
    """A negative Failsafe Production Active Power Limit is rejected (LPP-010)."""
    expect_nack(tc, lpp.write_failsafe(power=-100))
    tc.assert_eq(BASE_FAILSAFE[0], lpp_state(tc)["failsafe_limit_power_w"])


def test_failsafe_duration_shared_with_lpc(tc: TestContext):
    """LPP 2.7.1: The Failsafe Duration Minimum is provided only once and used for LPC and LPP."""
    desc = peer.read(lpp.dc_client, lpp.dc_server, "deviceConfigurationKeyValueDescriptionListData")
    durations = [d for d in as_list(desc.get("deviceConfigurationKeyValueDescriptionData")) if d.get("keyName") == "failsafeDurationMinimum"]
    tc.assert_eq(1, len(durations))
    if "lpc" not in tc.api("eebus/usecases"):
        return
    try:
        expect_ack(tc, lpp.write_failsafe(duration=4 * 3600))
        usecases = tc.api("eebus/usecases")
        tc.assert_eq(4 * 3600, usecases["lpp"]["failsafe_limit_duration_s"])
        tc.assert_eq(4 * 3600, usecases["lpc"]["failsafe_limit_duration_s"])
    finally:
        expect_ack(tc, lpp.write_failsafe(duration=BASE_FAILSAFE[1]))


def test_positive_limit_rejected(tc: TestContext):
    """A positive production limit is rejected and the state is kept (LPP-001, LPP-003/1)."""
    expect_nack(tc, lpp.write_limit(1000, active=True))
    tc.assert_eq(LPC_UNLIMITED_CONTROLLED, lpp_state(tc)["usecase_state"])
    tc.assert_eq([0, 0], get_last_run(tc))


def test_limit(tc: TestContext):
    """An active limit: limited, notified with the negative value, reported as absolute value in the API."""
    since = len(peer.messages)
    expect_ack(tc, lpp.write_limit(-4200, active=True, duration=300))
    state = wait_for_lpp_state(tc, LPC_LIMITED)
    tc.assert_true(state["limit_active"])
    tc.assert_eq(4200, state["current_limit"])
    tc.assert_gt(280, state["outstanding_duration_s"])
    limit = lpp.wait_for_limit_notify(active=True, value=-4200, since=since)
    tc.assert_eq(-4200, limit["value"]["number"])


def test_constraints(tc: TestContext):
    """Scenario 4: The contractual production nominal maximum (LPP-042) is not known yet and therefore not reported.
    A device that is not an energy manager would report LPP-041, an energy manager never does."""
    characteristics = lpp.read_constraints()
    tc.assert_false("powerProductionNominalMax" in characteristics)
    tc.assert_false("contractualProductionNominalMax" in characteristics)
    tc.assert_eq(0, lpp_state(tc)["constraints_power_maximum"])


# ---------------------------------------------------------------------------
# Automation trigger
# ---------------------------------------------------------------------------

def test_trigger_registered_and_enabled(tc: TestContext):
    state = tc.api("automation/state")
    tc.assert_true(TRIGGER_EEBUS_LPP in state["registered_triggers"])
    tc.assert_true(TRIGGER_EEBUS_LPP in state["enabled_triggers"])


def test_trigger_on_limit_and_release(tc: TestContext):
    """Activating the limit fires "limited", deactivating it fires "not limited"."""
    expect_ack(tc, lpp.write_limit(-3000, active=True))
    wait_for_lpp_state(tc, LPC_LIMITED)
    limited_ts = wait_fired(tc, RULE_LIMITED)
    tc.assert_eq(0, get_last_run(tc)[RULE_NOT_LIMITED])

    expect_ack(tc, lpp.write_limit(-3000, active=False))
    wait_for_lpp_state(tc, LPC_UNLIMITED_CONTROLLED)
    wait_fired(tc, RULE_NOT_LIMITED)
    tc.assert_eq(limited_ts, get_last_run(tc)[RULE_LIMITED])


def test_trigger_no_refire_on_limit_change(tc: TestContext):
    """Changing the value of an active limit does not fire again."""
    expect_ack(tc, lpp.write_limit(-3000, active=True))
    limited_ts = wait_fired(tc, RULE_LIMITED)
    expect_ack(tc, lpp.write_limit(-6000, active=True))
    tc.wait_for(lambda: tc.assert_eq(6000, lpp_state(tc)["current_limit"]), timeout=3)
    time.sleep(2)
    tc.assert_eq([limited_ts, 0], get_last_run(tc))


def test_trigger_on_duration_expiry(tc: TestContext):
    """When the duration of the limit expires, "not limited" fires."""
    expect_ack(tc, lpp.write_limit(-3000, active=True, duration=5))
    wait_fired(tc, RULE_LIMITED)
    wait_for_lpp_state(tc, LPC_UNLIMITED_CONTROLLED, timeout=15)
    wait_fired(tc, RULE_NOT_LIMITED)


def test_trigger_disable_and_enable_eebus(tc: TestContext):
    """Disabling EEBUS while limited fires "not limited" and disables the trigger.
    Enabling EEBUS reports the initial state again and enables the trigger."""
    expect_ack(tc, lpp.write_limit(-3000, active=True))
    wait_fired(tc, RULE_LIMITED)

    set_eebus_enabled(tc, False)
    try:
        not_limited_ts = wait_fired(tc, RULE_NOT_LIMITED)
        tc.wait_for(lambda: tc.assert_false(TRIGGER_EEBUS_LPP in tc.api("automation/state")["enabled_triggers"]))
    finally:
        set_eebus_enabled(tc, True)
        # Disabling EEBUS closed the connection
        disconnect_guard(tc)

    tc.wait_for(lambda: tc.assert_true(TRIGGER_EEBUS_LPP in tc.api("automation/state")["enabled_triggers"]))
    # "init" with the failsafe limit BASE_FAILSAFE below the unknown nominal maximum, written by the Energy Guard: limited
    wait_for_lpp_state(tc, LPC_INIT)
    wait_fired(tc, RULE_LIMITED, since=not_limited_ts)


def test_trigger_failsafe(tc: TestContext):
    """The failsafe state counts as limited: Without heartbeat for 120 s the device enters failsafe (LPP-912) and the
    Failsafe Production Active Power Limit applies (LPP-901). Taking control again fires "not limited"."""
    tc.set_test_timeout(300)
    peer.send_heartbeat()
    peer.heartbeat_enabled = False
    state = wait_for_lpp_state(tc, LPC_FAILSAFE, timeout=150)
    tc.assert_eq(BASE_FAILSAFE[0], state["current_limit"])
    limited_ts = wait_fired(tc, RULE_LIMITED)
    tc.assert_eq(0, get_last_run(tc)[RULE_NOT_LIMITED])

    peer.heartbeat_enabled = True
    take_control(tc, lpp)
    wait_fired(tc, RULE_NOT_LIMITED, since=limited_ts)


if __name__ == "__main__":
    run_testsuite(locals())
