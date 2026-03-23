#!/usr/bin/env -S uv run --group tests --script

# Tests for meter value automation triggers

import time

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite
else:
    import tinkerforge_util as tfutil

    tfutil.create_parent_module(__file__, "software")
    from software.test_runner.test_context import run_testsuite, TestContext

TRIGGER_METER_VALUE = 24
ACTION_PRINT = 1

CMP_GT  = 0  # >
CMP_LT  = 1  # <
CMP_GTE = 2  # >=
CMP_LTE = 3  # <=
CMP_EQ  = 4  # ==
CMP_NEQ = 5  # !=

METER_CLASS_API = 4
METER_SLOT = 4

MVID_POWER_ACTIVE_L_SUM = 74  # PowerActiveLSumImExDiff (W)
MVID_VOLTAGE_L1N = 1          # VoltageL1N (V)

METER_VALUE_IDS = [MVID_POWER_ACTIVE_L_SUM, MVID_VOLTAGE_L1N]


_original_automation_config = None
_original_meter_config = None
_marker_counter = 0


def _make_marker(label="METER"):
    global _marker_counter
    _marker_counter += 1
    return f"{label}_{int(time.time())}_{_marker_counter}"


def _reboot_and_wait(tc: TestContext):
    try:
        tc.api("reboot", None)
    except Exception:
        pass
    time.sleep(5)
    for _ in range(60):
        try:
            tc.http_request("GET", f"meters/{METER_SLOT}/values", timeout=2, parse=True)
            return
        except Exception:
            time.sleep(1)
    tc.fail("Device did not come back after reboot")


def _push_meter_values(tc: TestContext, power, voltage=230.0):
    tc.api(f"meters/{METER_SLOT}/update", [power, voltage])


def _get_event_log(tc: TestContext) -> str:
    return tc.http_request("GET", "event_log").decode("utf-8", errors="replace")


def _make_meter_trigger(comparator, threshold, hysteresis=0.0, value_id=MVID_POWER_ACTIVE_L_SUM):
    return [TRIGGER_METER_VALUE, {
        "meter_slot": METER_SLOT,
        "value_id": value_id,
        "comparator": comparator,
        "threshold": threshold,
        "hysteresis": hysteresis,
    }]


def _make_print_action(message):
    return [ACTION_PRINT, {"message": message}]


def _make_task(trigger, action, delay=0):
    return {
        "trigger": trigger,
        "action": action,
        "delay": delay,
    }


def _set_automation(tc: TestContext, tasks):
    tc.api("automation/config_update", {"tasks": tasks})


def suite_setup(tc: TestContext):
    tc.set_test_timeout(120)

    global _original_automation_config, _original_meter_config
    _original_automation_config = tc.api("automation/config")
    _original_meter_config = tc.api(f"meters/{METER_SLOT}/config")

    tc.api(
        f"meters/{METER_SLOT}/config_update",
        [
            METER_CLASS_API,
            {
                "display_name": "Automation Test",
                "location": 0,
                "value_ids": METER_VALUE_IDS,
            },
        ],
    )

    _reboot_and_wait(tc)

    # Verify meter is available
    value_ids = tc.api(f"meters/{METER_SLOT}/value_ids")
    tc.assert_eq(METER_VALUE_IDS, value_ids)


def test_gt_fires(tc: TestContext):
    """Power > 100 W -> print action fires."""
    marker = _make_marker("GT")
    task = _make_task(
        trigger=_make_meter_trigger(CMP_GT, 100.0),
        action=_make_print_action(marker),
    )
    _set_automation(tc, [task])
    time.sleep(1)

    _push_meter_values(tc, 200.0)

    tc.wait_for(lambda: tc.assert_true(marker in _get_event_log(tc)), timeout=15)


def test_lt_fires(tc: TestContext):
    """Power < 100 W -> print action fires."""
    marker = _make_marker("LT")
    task = _make_task(
        trigger=_make_meter_trigger(CMP_LT, 100.0),
        action=_make_print_action(marker),
    )
    _set_automation(tc, [task])
    time.sleep(1)

    _push_meter_values(tc, 10.0)

    tc.wait_for(lambda: tc.assert_true(marker in _get_event_log(tc)), timeout=15)


def test_gte_exact_boundary(tc: TestContext):
    """Power >= 42 W with value exactly at 42 -> fires."""
    marker = _make_marker("GTE")
    task = _make_task(
        trigger=_make_meter_trigger(CMP_GTE, 42.0),
        action=_make_print_action(marker),
    )
    _set_automation(tc, [task])
    time.sleep(1)

    _push_meter_values(tc, 42.0)

    tc.wait_for(lambda: tc.assert_true(marker in _get_event_log(tc)), timeout=15)


def test_lte_exact_boundary(tc: TestContext):
    """Power <= 42 W with value exactly at 42 -> fires."""
    marker = _make_marker("LTE")
    task = _make_task(
        trigger=_make_meter_trigger(CMP_LTE, 42.0),
        action=_make_print_action(marker),
    )
    _set_automation(tc, [task])
    time.sleep(1)

    _push_meter_values(tc, 42.0)

    tc.wait_for(lambda: tc.assert_true(marker in _get_event_log(tc)), timeout=15)


def test_eq_within_hysteresis(tc: TestContext):
    """Power == 100 W within hysteresis band of 5 W -> fires.

    Value 103 W: |103 - 100| = 3 <= 5 hysteresis.
    """
    marker = _make_marker("EQ")
    task = _make_task(
        trigger=_make_meter_trigger(CMP_EQ, 100.0, hysteresis=5.0),
        action=_make_print_action(marker),
    )
    _set_automation(tc, [task])
    time.sleep(1)

    _push_meter_values(tc, 103.0)

    tc.wait_for(lambda: tc.assert_true(marker in _get_event_log(tc)), timeout=15)


def test_neq_outside_hysteresis(tc: TestContext):
    """Power != 100 W outside hysteresis band of 5 W -> fires.

    Value 200 W: |200 - 100| = 100 > 5 hysteresis.
    """
    marker = _make_marker("NEQ")
    task = _make_task(
        trigger=_make_meter_trigger(CMP_NEQ, 100.0, hysteresis=5.0),
        action=_make_print_action(marker),
    )
    _set_automation(tc, [task])
    time.sleep(1)

    _push_meter_values(tc, 200.0)

    tc.wait_for(lambda: tc.assert_true(marker in _get_event_log(tc)), timeout=15)


def test_condition_not_met(tc: TestContext):
    """Power > 500 W but actual power is 100 W -> does not fire."""
    marker = _make_marker("NOTMET")
    task = _make_task(
        trigger=_make_meter_trigger(CMP_GT, 500.0),
        action=_make_print_action(marker),
    )
    _set_automation(tc, [task])
    time.sleep(1)

    _push_meter_values(tc, 100.0)
    time.sleep(5)

    tc.assert_false(marker in _get_event_log(tc))


def test_eq_outside_hysteresis_no_fire(tc: TestContext):
    """Power == 100 W with hysteresis 5, but value is 200 W -> does not fire.

    |200 - 100| = 100 > 5, so == does not match.
    """
    # Push a value outside the hysteresis band before setting up the rule
    _push_meter_values(tc, 200.0)

    marker = _make_marker("EQ_NO")
    task = _make_task(
        trigger=_make_meter_trigger(CMP_EQ, 100.0, hysteresis=5.0),
        action=_make_print_action(marker),
    )
    _set_automation(tc, [task])
    time.sleep(1)

    _push_meter_values(tc, 200.0)
    time.sleep(5)

    tc.assert_false(marker in _get_event_log(tc))


def test_edge_no_retrigger(tc: TestContext):
    """Edge detection: trigger fires once, does not re-trigger while
    value remains above threshold (within hysteresis reset band).

    Trigger: > 100 W, hysteresis 20 W.
    Reset condition: value < (100 - 20) = 80 W.
    """
    marker = _make_marker("EDGE1")
    task = _make_task(
        trigger=_make_meter_trigger(CMP_GT, 100.0, hysteresis=20.0),
        action=_make_print_action(marker),
    )
    _set_automation(tc, [task])
    time.sleep(1)

    # First push above threshold -> should fire once
    _push_meter_values(tc, 200.0)
    tc.wait_for(lambda: tc.assert_true(marker in _get_event_log(tc)), timeout=15)
    tc.assert_eq(1, _get_event_log(tc).count(marker))

    # Push another value still above threshold and above reset band -> must NOT fire again
    _push_meter_values(tc, 150.0)
    time.sleep(5)

    tc.assert_eq(1, _get_event_log(tc).count(marker))


def test_edge_hysteresis_retrigger(tc: TestContext):
    """Edge detection: value drops below hysteresis reset band,
    then rises above threshold again -> fires a second time.

    Trigger: > 100 W, hysteresis 20 W.
    Reset when value < 80 W.
    """
    tc.set_test_timeout(45)

    marker = _make_marker("EDGE2")
    task = _make_task(
        trigger=_make_meter_trigger(CMP_GT, 100.0, hysteresis=20.0),
        action=_make_print_action(marker),
    )
    _set_automation(tc, [task])
    time.sleep(1)

    # Step 1: Push above threshold -> fires (count = 1)
    _push_meter_values(tc, 200.0)
    tc.wait_for(lambda: tc.assert_eq(1, _get_event_log(tc).count(marker)), timeout=15)

    # Step 2: Push below reset band (< 80 W) -> resets trigger state
    _push_meter_values(tc, 50.0)
    time.sleep(3)

    # Step 3: Push above threshold again -> fires again (count = 2)
    _push_meter_values(tc, 200.0)
    tc.wait_for(lambda: tc.assert_eq(2, _get_event_log(tc).count(marker)), timeout=15)


def test_modify_rule(tc: TestContext):
    """Modify meter rule at runtime: change the print message.

    config_update resets trigger edge state, so the modified rule
    can fire immediately on the next value push.
    """
    marker_a = _make_marker("MOD_A")
    task = _make_task(
        trigger=_make_meter_trigger(CMP_GT, 100.0),
        action=_make_print_action(marker_a),
    )
    _set_automation(tc, [task])
    time.sleep(1)

    _push_meter_values(tc, 200.0)
    tc.wait_for(lambda: tc.assert_true(marker_a in _get_event_log(tc)), timeout=15)

    # Modify rule: different message (config_update resets edge state)
    marker_b = _make_marker("MOD_B")
    task = _make_task(
        trigger=_make_meter_trigger(CMP_GT, 100.0),
        action=_make_print_action(marker_b),
    )
    _set_automation(tc, [task])
    time.sleep(1)

    _push_meter_values(tc, 300.0)
    tc.wait_for(lambda: tc.assert_true(marker_b in _get_event_log(tc)), timeout=15)

    # marker_a must not have fired a second time
    tc.assert_eq(1, _get_event_log(tc).count(marker_a))


def test_multiple_rules(tc: TestContext):
    """Two meter rules with different messages: both fire."""
    marker_1 = _make_marker("MULTI_1")
    marker_2 = _make_marker("MULTI_2")
    tasks = [
        _make_task(
            trigger=_make_meter_trigger(CMP_GT, 100.0),
            action=_make_print_action(marker_1),
        ),
        _make_task(
            trigger=_make_meter_trigger(CMP_GT, 100.0),
            action=_make_print_action(marker_2),
        ),
    ]
    _set_automation(tc, tasks)
    time.sleep(1)

    _push_meter_values(tc, 200.0)

    tc.wait_for(lambda: tc.assert_true(marker_1 in _get_event_log(tc)), timeout=15)
    tc.wait_for(lambda: tc.assert_true(marker_2 in _get_event_log(tc)), timeout=15)


def test_second_value_id(tc: TestContext):
    """Trigger on VoltageL1N (second value_id) instead of power."""
    marker = _make_marker("VOLT")
    task = _make_task(
        trigger=_make_meter_trigger(CMP_GT, 200.0, value_id=MVID_VOLTAGE_L1N),
        action=_make_print_action(marker),
    )
    _set_automation(tc, [task])
    time.sleep(1)

    # Push: power=0, voltage=240 V (> 200 V threshold)
    _push_meter_values(tc, 0.0, voltage=240.0)

    tc.wait_for(lambda: tc.assert_true(marker in _get_event_log(tc)), timeout=15)


def test_delete_rules(tc: TestContext):
    """Delete all rules: config becomes empty."""
    _set_automation(tc, [])
    time.sleep(1)

    config = tc.api("automation/config")
    tc.assert_eq(0, len(config.get("tasks", [None])))


def suite_teardown(tc: TestContext):
    global _original_automation_config, _original_meter_config

    # Restore automation config
    if _original_automation_config is not None:
        try:
            tc.api("automation/config_update", _original_automation_config)
        except Exception:
            pass
        _original_automation_config = None

    # Restore meter config
    if _original_meter_config is not None:
        try:
            tc.api(f"meters/{METER_SLOT}/config_update", _original_meter_config)
        except Exception:
            pass
        _original_meter_config = None


if __name__ == "__main__":
    run_testsuite(locals())
