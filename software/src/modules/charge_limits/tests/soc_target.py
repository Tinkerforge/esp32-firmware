#!/usr/bin/env -S uv run --group tests --script

# Simple tests for charge_limits SoC target API

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

saved_config = None


def suite_setup(tc: TestContext):
    global saved_config
    saved_config = tc.api("charge_limits/default_limits")


def test_default_limits_has_soc_field(tc: TestContext):
    config = tc.api("charge_limits/default_limits")

    if tc.device_type().is_warp(4):
        tc.assert_in(config, "soc_target_pct")
    else:
        tc.assert_not("soc_target_pct" in config)


def test_set_default_soc_target(tc: TestContext):
    if not tc.device_type().is_warp(4):
        tc.skip("SoC target is WARP4-only")

    config = tc.api("charge_limits/default_limits")
    config["soc_target_pct"] = 80
    tc.api("charge_limits/default_limits_update", config)

    updated = tc.api("charge_limits/default_limits")
    tc.assert_eq(80, updated["soc_target_pct"])


def test_active_limits_has_soc_field(tc: TestContext):
    active = tc.api("charge_limits/active_limits")

    if tc.device_type().is_warp(4):
        tc.assert_in(active, "soc_target_pct")
    else:
        tc.assert_not("soc_target_pct" in active)


def test_override_soc_updates_active_limits(tc: TestContext):
    if not tc.device_type().is_warp(4):
        tc.skip("SoC target is WARP4-only")

    tc.api("charge_limits/override_soc", {"soc_target_pct": 75})

    def _check_set():
        active = tc.api("charge_limits/active_limits")
        tc.assert_eq(75, active["soc_target_pct"])

    tc.wait_for(_check_set)

    # Disable by setting to 0
    tc.api("charge_limits/override_soc", {"soc_target_pct": 0})

    def _check_reset():
        active = tc.api("charge_limits/active_limits")
        tc.assert_eq(0, active["soc_target_pct"])

    tc.wait_for(_check_reset)


def test_state_has_soc_fields(tc: TestContext):
    state = tc.api("charge_limits/state")

    if tc.device_type().is_warp(4):
        tc.assert_in(state, "soc_target_pct")
        tc.assert_in(state, "current_soc_pct")
    else:
        tc.assert_not("soc_target_pct" in state)
        tc.assert_not("current_soc_pct" in state)


def test_override_soc_updates_state(tc: TestContext):
    if not tc.device_type().is_warp(4):
        tc.skip("SoC target is WARP4-only")

    tc.api("charge_limits/override_soc", {"soc_target_pct": 90})

    def _check():
        state = tc.api("charge_limits/state")
        tc.assert_eq(90, state["soc_target_pct"])

    tc.wait_for(_check)


def suite_teardown(tc: TestContext):
    if saved_config is not None:
        tc.api("charge_limits/default_limits_update", saved_config)


if __name__ == "__main__":
    run_testsuite(locals())
