#!/usr/bin/env -S uv run --group tests --script

# Tests for the p9_eeg module: config validation, the mapping of limits to levels and the automation trigger with the API source.
# The relays are only switched if an Energy Manager Bricklet is present. These tests only check the API state.

import time
from urllib.error import HTTPError

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

SOURCE_EEBUS = 0
SOURCE_API = 1
RELAY_UNUSED = 255

TRIGGER_P9_EEG = 27
ACTION_PRINT = 1
RULE_LIMITED = 0
RULE_NOT_LIMITED = 1

original_config: dict | None = None
relay_count = 2


def make_config(enable: bool = True, source: int = SOURCE_API, inverter_power_w: int = 10000,
                relays: list[int] | None = None, default_level_pct: int = 100) -> dict:
    if relays is None:
        relays = [30, 0][:relay_count]
    return {
        "enable": enable,
        "source": source,
        "inverter_power_w": inverter_power_w,
        "relays": relays,
        "default_level_pct": default_level_pct,
    }


def pad_relays(*levels: int) -> list[int]:
    return list(levels) + [RELAY_UNUSED] * (relay_count - len(levels))


def expect_rejected(tc: TestContext, config: dict):
    try:
        tc.api("p9_eeg/config_update", config)
    except HTTPError as e:
        tc.assert_eq(400, e.code)
        return
    tc.fail(f"config was accepted: {config}")


def set_control(tc: TestContext, active: bool, limit_w: int):
    tc.api("p9_eeg/control_update", {"active": active, "limit_w": limit_w})


def wait_for_state(tc: TestContext, **expected) -> dict:
    def check() -> dict:
        state = tc.api("p9_eeg/state")
        for key, value in expected.items():
            tc.assert_eq(value, state[key])
        return state
    return tc.wait_for(check)


def suite_setup(tc: TestContext):
    global original_config, relay_count
    original_config = tc.api("p9_eeg/config")
    relay_count = len(original_config["relays"])


def suite_teardown(tc: TestContext):
    if original_config is not None:
        tc.api("p9_eeg/config_update", original_config)


def test_inverter_power_required(tc: TestContext):
    expect_rejected(tc, make_config(inverter_power_w=0))
    tc.api("p9_eeg/config_update", make_config(enable=False, inverter_power_w=0))


def test_relay_required(tc: TestContext):
    expect_rejected(tc, make_config(relays=[RELAY_UNUSED] * relay_count))


def test_invalid_relay_level(tc: TestContext):
    expect_rejected(tc, make_config(relays=pad_relays(40)))
    expect_rejected(tc, make_config(relays=pad_relays(100)))


def test_duplicate_relay_level(tc: TestContext):
    if relay_count < 2:
        tc.skip("Only one relay")
    expect_rejected(tc, make_config(relays=[30, 30]))


def test_default_level_must_be_available(tc: TestContext):
    expect_rejected(tc, make_config(relays=pad_relays(30), default_level_pct=60))
    expect_rejected(tc, make_config(default_level_pct=50))


def test_default_level_until_first_control(tc: TestContext):
    """After a reboot the API source has not provided a limit yet, the default level applies until the first control update."""
    tc.api("p9_eeg/config_update", make_config(default_level_pct=30))
    tc.reboot()
    wait_for_state(tc, default_active=True, level_pct=30, limit_active=False)

    set_control(tc, False, 0)
    wait_for_state(tc, default_active=False, level_pct=100, limit_active=False)


def test_level_mapping(tc: TestContext):
    """The highest level that complies with the limit is used. 10 kW inverter, levels 0 %, 30 % and 100 %."""
    if relay_count < 2:
        tc.skip("Only one relay")
    tc.api("p9_eeg/config_update", make_config(relays=[30, 0]))

    for limit_w, level in [(10000, 100), (9999, 30), (4200, 30), (3000, 30), (2999, 0), (0, 0)]:
        set_control(tc, True, limit_w)
        wait_for_state(tc, limit_active=True, limit_w=limit_w, level_pct=level, limit_reachable=True, default_active=False)

    set_control(tc, False, 0)
    wait_for_state(tc, limit_active=False, limit_w=0, level_pct=100)


def test_limit_not_reachable(tc: TestContext):
    """Below the lowest level the lowest level is used and reported as not reachable."""
    tc.api("p9_eeg/config_update", make_config(relays=pad_relays(60)))
    set_control(tc, True, 5000)
    wait_for_state(tc, level_pct=60, limit_reachable=False)
    set_control(tc, True, 6000)
    wait_for_state(tc, level_pct=60, limit_reachable=True)
    set_control(tc, False, 0)
    wait_for_state(tc, level_pct=100, limit_reachable=True)


def test_disable(tc: TestContext):
    tc.api("p9_eeg/config_update", make_config(relays=pad_relays(0)))
    set_control(tc, True, 0)
    wait_for_state(tc, level_pct=0)
    tc.api("p9_eeg/config_update", make_config(enable=False, relays=pad_relays(0)))
    wait_for_state(tc, limit_active=False, level_pct=100, default_active=False)


def test_eebus_source_without_eebus(tc: TestContext):
    """Without LPP information from EEBUS the default level applies."""
    eebus_config = tc.api("eebus/config")
    try:
        if eebus_config["enable"]:
            tc.api("eebus/config_update", {**eebus_config, "enable": False})
        tc.api("p9_eeg/config_update", make_config(source=SOURCE_EEBUS, relays=pad_relays(0), default_level_pct=0))
        wait_for_state(tc, default_active=True, level_pct=0)
    finally:
        if eebus_config["enable"]:
            tc.api("eebus/config_update", eebus_config)


def test_eebus_preconfigured_failsafe(tc: TestContext):
    """With source EEBUS the default level is the pre-configured LPP failsafe limit (LPP-021/1).
    A failsafe limit written by an Energy Guard has priority (LPP-903) and is stored persistently."""
    eebus_config = tc.api("eebus/config")
    if not eebus_config["enable"]:
        tc.api("eebus/config_update", {**eebus_config, "enable": True})
    try:
        tc.wait_for(lambda: tc.assert_true("lpp" in tc.api("eebus/usecases")), timeout=10)

        def lpp_failsafe_w() -> int:
            return tc.api("eebus/usecases")["lpp"]["failsafe_limit_power_w"]

        tc.api("p9_eeg/config_update", make_config(source=SOURCE_EEBUS, inverter_power_w=10001, relays=pad_relays(0), default_level_pct=0))
        time.sleep(1)
        if lpp_failsafe_w() != 0:
            tc.skip("The LPP failsafe limit was written by an Energy Guard")

        tc.api("p9_eeg/config_update", make_config(source=SOURCE_EEBUS, inverter_power_w=10001, relays=pad_relays(30), default_level_pct=30))
        # Rounded up, so that the failsafe limit maps back to 30 %
        tc.wait_for(lambda: tc.assert_eq(3001, lpp_failsafe_w()))

        tc.api("p9_eeg/config_update", make_config(source=SOURCE_EEBUS, inverter_power_w=10001, relays=pad_relays(30), default_level_pct=100))
        tc.wait_for(lambda: tc.assert_ne(3001, lpp_failsafe_w()))

        tc.api("p9_eeg/config_update", make_config(source=SOURCE_API, inverter_power_w=10001, relays=pad_relays(0), default_level_pct=0))
        tc.wait_for(lambda: tc.assert_ne(0, lpp_failsafe_w()))
    finally:
        if not eebus_config["enable"]:
            tc.api("eebus/config_update", eebus_config)


def test_trigger_api_source(tc: TestContext):
    """The automation trigger fires when §9 EEG limits the feed-in (level below 100 %), also with the API source."""
    automation_config = tc.api("automation/config")

    def last_run() -> list[int]:
        return tc.api("automation/state")["last_run"]

    def wait_fired(rule: int, since: int = 0) -> int:
        def check() -> int:
            ts = last_run()[rule]
            tc.assert_gt(since, ts)
            return ts
        return tc.wait_for(check)

    try:
        tc.api("p9_eeg/config_update", make_config())
        set_control(tc, False, 0)
        wait_for_state(tc, level_pct=100, default_active=False)

        tc.api("automation/config_update", {"tasks": []})
        tc.wait_for(lambda: tc.assert_eq([], last_run()))
        tc.api("automation/config_update", {"tasks": [
            {"trigger": [TRIGGER_P9_EEG, {"limited": True}], "action": [ACTION_PRINT, {"message": "p9_eeg test: limited"}], "delay": 0},
            {"trigger": [TRIGGER_P9_EEG, {"limited": False}], "action": [ACTION_PRINT, {"message": "p9_eeg test: not limited"}], "delay": 0},
        ]})
        tc.wait_for(lambda: tc.assert_eq([0, 0], last_run()))

        # A limit above the rated power keeps 100 % and does not limit
        set_control(tc, True, 20000)
        wait_for_state(tc, limit_active=True, level_pct=100)
        time.sleep(1)
        tc.assert_eq([0, 0], last_run())

        set_control(tc, True, 4200)
        limited_ts = wait_fired(RULE_LIMITED)
        tc.assert_eq(0, last_run()[RULE_NOT_LIMITED])

        # Another level below 100 % does not fire again
        set_control(tc, True, 0)
        wait_for_state(tc, limit_w=0)
        time.sleep(1)
        tc.assert_eq([limited_ts, 0], last_run())

        set_control(tc, False, 0)
        wait_fired(RULE_NOT_LIMITED)
        tc.assert_eq(limited_ts, last_run()[RULE_LIMITED])
    finally:
        tc.api("automation/config_update", automation_config)


if __name__ == "__main__":
    run_testsuite(locals())
