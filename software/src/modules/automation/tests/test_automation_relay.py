#!/usr/bin/env python3
"""Integration test for automation save-without-restart.

Tests that automation rules can be created, modified, and deleted at runtime
without requiring a device reboot. Uses temperature triggers to switch EM
relays, verifying end-to-end automation behavior on a real Energy Manager.

Usage:
    python3 test_automation_relay.py <device-ip>
    python3 test_automation_relay.py <device-ip> --verbose

Prerequisites on the device:
    - Energy Manager v2 with at least one relay
    - temperatures module available
"""

import argparse
import json
import sys
import time
import urllib.request
import urllib.error


# ---------------------------------------------------------------------------
# Device communication helpers
# ---------------------------------------------------------------------------

def get_json(host, path):
    url = f"http://{host}/{path}"
    req = urllib.request.Request(url)
    with urllib.request.urlopen(req, timeout=5) as resp:
        return json.loads(resp.read())


def put_json(host, path, payload):
    url = f"http://{host}/{path}"
    data = json.dumps(payload).encode("utf-8")
    req = urllib.request.Request(url, data=data, method="PUT")
    req.add_header("Content-Type", "application/json")
    try:
        with urllib.request.urlopen(req, timeout=10) as resp:
            body = resp.read().decode("utf-8", errors="replace")
            return resp.status, body
    except urllib.error.HTTPError as e:
        body = e.read().decode("utf-8", errors="replace")
        return e.code, body


# ---------------------------------------------------------------------------
# Temperature push helpers
# ---------------------------------------------------------------------------

def midnight_today_utc_minutes():
    """UTC midnight of today, in minutes since epoch."""
    now = int(time.time())
    midnight = now - now % 86400
    return midnight // 60


def make_temperatures(current_temp_c10):
    """Build a temperatures_update payload.

    The firmware expects 47-49 int16 values (hourly temperatures for ~2 days)
    in units of degrees C * 10.  We fill them all with the same value so
    that the 'current' temperature (index derived from hour-of-day) matches
    our expectation regardless of when the test runs.

    current_temp_c10: temperature in degC*10  (e.g. 250 = 25.0 degC)
    """
    first_date = midnight_today_utc_minutes()
    temperatures = [current_temp_c10] * 48
    return {
        "first_date": first_date,
        "temperatures": temperatures,
    }


# ---------------------------------------------------------------------------
# Device state helpers
# ---------------------------------------------------------------------------

def get_relay_state(host, index=0):
    """Read relay state from energy_manager/state. Returns True (closed) or False (open)."""
    state = get_json(host, "energy_manager/state")
    relays = state["relays"]
    return relays[index]


def set_relay_direct(host, index, closed):
    """Directly set a relay via energy_manager/outputs_update.

    Payload is [sg0, sg1, relay0, relay1] where 255 = no change.
    """
    payload = [255, 255, 255, 255]
    payload[2 + index] = 1 if closed else 0
    status, body = put_json(host, "energy_manager/outputs_update", payload)
    if status != 200:
        raise RuntimeError(f"outputs_update failed: HTTP {status}: {body}")


def ensure_temperatures_push_mode(host):
    """Enable temperatures module in Push mode if not already configured."""
    config = get_json(host, "temperatures/config")
    needs_update = (
        not config.get("enable", False) or
        config.get("source", 0) != 1
    )
    if needs_update:
        config["enable"] = True
        config["source"] = 1  # Push
        status, body = put_json(host, "temperatures/config_update", config)
        if status != 200:
            raise RuntimeError(
                f"Failed to enable temperatures push mode: HTTP {status}: {body}")
        time.sleep(1)
    return config


def push_temperatures(host, current_temp_c10):
    """Push temperature data to the device."""
    payload = make_temperatures(current_temp_c10)
    status, body = put_json(host, "temperatures/temperatures_update", payload)
    if status != 200:
        raise RuntimeError(
            f"temperatures_update failed: HTTP {status}: {body}")


def set_automation_config(host, tasks):
    """PUT /automation/config_update with the given task list."""
    payload = {"tasks": tasks}
    status, body = put_json(host, "automation/config_update", payload)
    if status != 200:
        raise RuntimeError(
            f"automation/config_update failed: HTTP {status}: {body}")


def get_automation_config(host):
    """GET /automation/config."""
    return get_json(host, "automation/config")


# ---------------------------------------------------------------------------
# Automation rule builders
# ---------------------------------------------------------------------------

# Trigger IDs
TRIGGER_TEMPERATURE_NOW = 23

# Action IDs
ACTION_EM_RELAY_SWITCH = 13


def make_temp_trigger(comparison, value_c10, temp_type=0):
    """Build a temperature trigger union.

    comparison: 0 = greater than, 1 = less than
    value_c10:  threshold in degC * 10
    temp_type:  0 = current
    """
    return [TRIGGER_TEMPERATURE_NOW, {
        "type": temp_type,
        "comparison": comparison,
        "value": value_c10,
    }]


def make_relay_action(index, closed):
    """Build an EM relay switch action union."""
    return [ACTION_EM_RELAY_SWITCH, {
        "index": index,
        "closed": closed,
    }]


def make_task(trigger, action, delay=0):
    """Build a single automation task."""
    return {
        "trigger": trigger,
        "action": action,
        "delay": delay,
    }


# ---------------------------------------------------------------------------
# Test helpers
# ---------------------------------------------------------------------------

class TestFailure(Exception):
    pass


def wait_for_relay(host, index, expected, timeout=15, poll_interval=0.5):
    """Poll until relay reaches expected state or timeout."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        actual = get_relay_state(host, index)
        if actual == expected:
            return True
        time.sleep(poll_interval)
    return False


# ---------------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------------

def test_01_temp_gt_closes_relay(host, verbose):
    """Rule: temperature > -100 degC => close relay 0.

    Push 25 degC which is > -100, so relay 0 should close.
    """
    print("  [01] Temperature > -100 closes relay 0 ...")

    # Ensure relay starts open
    set_relay_direct(host, 0, False)
    time.sleep(0.5)
    assert not get_relay_state(host, 0), "Relay 0 should start open"

    # Create rule: temp > -10.0 degC => close relay 0
    task = make_task(
        trigger=make_temp_trigger(comparison=0, value_c10=-100),  # > -10.0 C
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    if verbose:
        print(f"    Config: {json.dumps(get_automation_config(host), indent=2)}")

    # Push temperature 25.0 degC (definitely > -10.0)
    # NOTE: Each test uses a unique temperature value because the trigger
    # has change-detection and only fires when values actually change.
    push_temperatures(host, 250)
    time.sleep(1)

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close. Current state: {get_relay_state(host, 0)}")

    print("    PASS")


def test_02_modify_rule_opens_relay(host, verbose):
    """Modify existing rule: temperature > -100 => open relay 0.

    Same trigger, but action changes to open. Push new temperature data
    to re-trigger the automation.
    """
    print("  [02] Modify rule: relay action changes to open ...")

    # Relay should currently be closed from test_01
    assert get_relay_state(host, 0), "Relay 0 should be closed from previous test"

    # Modify rule: same trigger, action = open relay 0
    task = make_task(
        trigger=make_temp_trigger(comparison=0, value_c10=-100),
        action=make_relay_action(index=0, closed=False),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Push a different temperature to trigger change detection
    push_temperatures(host, 260)
    time.sleep(1)

    if not wait_for_relay(host, 0, False, timeout=15):
        raise TestFailure(
            f"Relay 0 did not open. Current state: {get_relay_state(host, 0)}")

    print("    PASS")


def test_03_temp_lt_closes_relay(host, verbose):
    """Rule: temperature < 500 degC => close relay 0.

    Push 27 degC which is < 500, so relay 0 should close.
    """
    print("  [03] Temperature < 500 closes relay 0 ...")

    # Ensure relay starts open
    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    # Create rule: temp < 50.0 degC => close relay 0
    task = make_task(
        trigger=make_temp_trigger(comparison=1, value_c10=500),  # < 50.0 C
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Push 27.0 degC (< 50.0, different from test_02's 260)
    push_temperatures(host, 270)
    time.sleep(1)

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close. Current state: {get_relay_state(host, 0)}")

    print("    PASS")


def test_04_condition_not_met(host, verbose):
    """Rule: temperature > 300 degC => close relay 0.

    Push 28 degC which is NOT > 300, so relay should stay open.
    """
    print("  [04] Condition not met: relay stays open ...")

    # Reset relay to open
    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    # Create rule: temp > 30.0 degC => close relay 0
    task = make_task(
        trigger=make_temp_trigger(comparison=0, value_c10=300),  # > 30.0 C
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Push 28.0 degC (NOT > 30.0, different from test_03's 270)
    push_temperatures(host, 280)
    time.sleep(3)

    # Relay should still be open
    if get_relay_state(host, 0):
        raise TestFailure("Relay 0 closed but condition should not have been met")

    print("    PASS")


def test_05_multiple_rules(host, verbose):
    """Two rules: one for each relay.

    Rule 1: temp > -100 => close relay 0
    Rule 2: temp > -100 => close relay 1
    """
    print("  [05] Multiple rules: both relays close ...")

    # Reset both relays
    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    time.sleep(0.5)

    tasks = [
        make_task(
            trigger=make_temp_trigger(comparison=0, value_c10=-100),
            action=make_relay_action(index=0, closed=True),
        ),
        make_task(
            trigger=make_temp_trigger(comparison=0, value_c10=-100),
            action=make_relay_action(index=1, closed=True),
        ),
    ]
    set_automation_config(host, tasks)
    time.sleep(1)

    # Push 29.0 degC (different from test_04's 280)
    push_temperatures(host, 290)
    time.sleep(1)

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close. State: {get_relay_state(host, 0)}")
    if not wait_for_relay(host, 1, True, timeout=15):
        raise TestFailure(
            f"Relay 1 did not close. State: {get_relay_state(host, 1)}")

    print("    PASS")


def test_06_delete_rules(host, verbose):
    """Delete all rules: relays should be controllable directly again.

    After clearing automation config, manually open both relays.
    """
    print("  [06] Delete all rules: config becomes empty ...")

    set_automation_config(host, [])
    time.sleep(1)

    config = get_automation_config(host)
    if len(config.get("tasks", [None])) != 0:
        raise TestFailure(
            f"Expected empty tasks, got: {json.dumps(config)}")

    # Verify relays can be controlled directly
    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    time.sleep(1)

    if get_relay_state(host, 0):
        raise TestFailure("Relay 0 should be open after clearing rules")
    if get_relay_state(host, 1):
        raise TestFailure("Relay 1 should be open after clearing rules")

    print("    PASS")


def test_07_add_rule_after_clear(host, verbose):
    """Add a new rule after config was cleared (save-without-restart stress test).

    This specifically tests that apply_config() correctly rebuilds
    config_in_use and last_run after a clear+add cycle.
    """
    print("  [07] Add rule after clear: relay closes ...")

    # Relay should be open from test_06
    assert not get_relay_state(host, 0), "Relay 0 should be open"

    task = make_task(
        trigger=make_temp_trigger(comparison=0, value_c10=-100),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Push temperature to trigger (different from test_05's 290)
    push_temperatures(host, 300)
    time.sleep(1)

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close after re-adding rule. "
            f"State: {get_relay_state(host, 0)}")

    print("    PASS")


def test_08_rapid_config_changes(host, verbose):
    """Rapidly change automation config multiple times.

    Tests that apply_config() handles rapid successive calls without
    crashing or corrupting state.
    """
    print("  [08] Rapid config changes: no crash ...")

    for i in range(5):
        temp_value = -100 + i * 10
        closed = (i % 2 == 0)
        task = make_task(
            trigger=make_temp_trigger(comparison=0, value_c10=temp_value),
            action=make_relay_action(index=0, closed=closed),
        )
        set_automation_config(host, [task])
        time.sleep(0.3)

    # Give the device time to settle
    time.sleep(2)

    # Verify device is still responsive
    config = get_automation_config(host)
    if "tasks" not in config:
        raise TestFailure("Device not responding correctly after rapid changes")

    # Also verify state endpoint works
    state = get_json(host, "energy_manager/state")
    if "relays" not in state:
        raise TestFailure("energy_manager/state malformed after rapid changes")

    print("    PASS")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

ALL_TESTS = [
    test_01_temp_gt_closes_relay,
    test_02_modify_rule_opens_relay,
    test_03_temp_lt_closes_relay,
    test_04_condition_not_met,
    test_05_multiple_rules,
    test_06_delete_rules,
    test_07_add_rule_after_clear,
    test_08_rapid_config_changes,
]


def main():
    parser = argparse.ArgumentParser(
        description="Integration test for automation save-without-restart.",
        epilog=(
            "Example:\n"
            "  python3 test_automation_relay.py 192.168.0.33\n"
            "  python3 test_automation_relay.py 192.168.0.33 --verbose\n"
            "  python3 test_automation_relay.py 192.168.0.33 --test 03"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("host", nargs="?", default=None,
                        help="Device IP or hostname")
    parser.add_argument("--verbose", "-v", action="store_true",
                        help="Verbose output")
    parser.add_argument("--test", "-t", type=str, default=None,
                        help="Run only tests matching this substring")
    args = parser.parse_args()

    if args.host is None:
        parser.print_help()
        sys.exit(1)

    host = args.host
    verbose = args.verbose

    # Verify connectivity
    print(f"=== Automation Integration Test ===")
    print(f"Device: {host}\n")

    try:
        state = get_json(host, "energy_manager/state")
        print(f"EM state: relays={state['relays']}, "
              f"sg_ready={state['sg_ready_outputs']}")
    except Exception as e:
        print(f"ERROR: Cannot connect to device: {e}")
        sys.exit(1)

    # Ensure temperatures is in push mode
    print("Configuring temperatures for push mode ...")
    try:
        ensure_temperatures_push_mode(host)
    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(1)

    # Save original automation config to restore later
    original_config = get_automation_config(host)
    if verbose:
        print(f"Original automation config: {json.dumps(original_config, indent=2)}")

    print()

    # Select tests
    tests = ALL_TESTS
    if args.test:
        tests = [t for t in tests if args.test in t.__name__]
        if not tests:
            print(f"No tests matching '{args.test}'")
            sys.exit(1)

    # Run tests
    passed = 0
    failed = 0
    errors = []

    for test_fn in tests:
        try:
            test_fn(host, verbose)
            passed += 1
        except TestFailure as e:
            failed += 1
            errors.append((test_fn.__name__, str(e)))
            print(f"    FAIL: {e}")
        except Exception as e:
            failed += 1
            errors.append((test_fn.__name__, f"Unexpected error: {e}"))
            print(f"    ERROR: {e}")

    # Cleanup: restore original config and open relays
    print("\nCleaning up ...")
    try:
        set_automation_config(host, original_config.get("tasks", []))
    except Exception as e:
        print(f"  WARNING: Failed to restore automation config: {e}")

    try:
        set_relay_direct(host, 0, False)
        set_relay_direct(host, 1, False)
    except Exception as e:
        print(f"  WARNING: Failed to reset relays: {e}")

    # Summary
    print(f"\n{'='*50}")
    print(f"Results: {passed} passed, {failed} failed, {len(tests)} total")

    if errors:
        print("\nFailed tests:")
        for name, err in errors:
            print(f"  - {name}: {err}")
        sys.exit(1)
    else:
        print("\nAll tests passed!")


if __name__ == "__main__":
    main()
