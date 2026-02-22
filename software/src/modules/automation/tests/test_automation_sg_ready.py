#!/usr/bin/env python3
"""Integration test for automation with SG Ready outputs and cross-module triggers.

Tests SG Ready actions, EMRelaySwitch triggers (relay state change triggers
another action), and EMSGReadySwitch triggers (SG Ready state change
triggers another action).

Usage:
    python3 test_automation_sg_ready.py <device-ip>
    python3 test_automation_sg_ready.py <device-ip> --verbose

Prerequisites on the device:
    - Energy Manager v2 with relays and SG Ready outputs
    - If the heating module is controlling SG Ready outputs, it will be
      temporarily disabled during the test and restored afterward
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
# Device state helpers
# ---------------------------------------------------------------------------

def get_em_state(host):
    return get_json(host, "energy_manager/state")


def get_relay_state(host, index=0):
    return get_em_state(host)["relays"][index]


def get_sg_ready_state(host, index=0):
    return get_em_state(host)["sg_ready_outputs"][index]


def set_relay_direct(host, index, closed):
    payload = [255, 255, 255, 255]
    payload[2 + index] = 1 if closed else 0
    status, body = put_json(host, "energy_manager/outputs_update", payload)
    if status != 200:
        raise RuntimeError(f"outputs_update failed: HTTP {status}: {body}")


def set_sg_ready_direct(host, index, closed):
    payload = [255, 255, 255, 255]
    payload[index] = 1 if closed else 0
    status, body = put_json(host, "energy_manager/outputs_update", payload)
    if status != 200:
        raise RuntimeError(f"outputs_update failed: HTTP {status}: {body}")


def set_automation_config(host, tasks):
    payload = {"tasks": tasks}
    status, body = put_json(host, "automation/config_update", payload)
    if status != 200:
        raise RuntimeError(
            f"automation/config_update failed: HTTP {status}: {body}")


def get_automation_config(host):
    return get_json(host, "automation/config")


def ensure_temperatures_push_mode(host):
    config = get_json(host, "temperatures/config")
    needs_update = (
        not config.get("enable", False) or
        config.get("source", 0) != 1
    )
    if needs_update:
        config["enable"] = True
        config["source"] = 1
        status, body = put_json(host, "temperatures/config_update", config)
        if status != 200:
            raise RuntimeError(
                f"Failed to enable temperatures push mode: HTTP {status}: {body}")
        time.sleep(1)


def push_temperatures(host, current_temp_c10):
    now = int(time.time())
    midnight = now - now % 86400
    first_date = midnight // 60
    payload = {
        "first_date": first_date,
        "temperatures": [current_temp_c10] * 48,
    }
    status, body = put_json(host, "temperatures/temperatures_update", payload)
    if status != 200:
        raise RuntimeError(
            f"temperatures_update failed: HTTP {status}: {body}")


def wait_for_relay(host, index, expected, timeout=15, poll_interval=0.5):
    deadline = time.time() + timeout
    while time.time() < deadline:
        if get_relay_state(host, index) == expected:
            return True
        time.sleep(poll_interval)
    return False


def wait_for_sg_ready(host, index, expected, timeout=15, poll_interval=0.5):
    deadline = time.time() + timeout
    while time.time() < deadline:
        if get_sg_ready_state(host, index) == expected:
            return True
        time.sleep(poll_interval)
    return False


def reset_all_outputs(host):
    """Open all relays and SG Ready outputs."""
    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    set_sg_ready_direct(host, 0, False)
    set_sg_ready_direct(host, 1, False)
    time.sleep(1)


# Fields in heating config that cause it to claim SG Ready outputs.
_HEATING_SGR_FIELDS = [
    "pv_excess_control", "yield_forecast", "p14enwg",
    "blocking", "extended",
]


def disable_heating_sgr_control(host, verbose):
    """Disable heating features that lock SG Ready outputs.

    Returns the original heating config (for restore), or None if no
    changes were needed.
    """
    try:
        config = get_json(host, "heating/config")
    except Exception:
        return None  # No heating module present

    # Check if any SG Ready-controlling feature is enabled
    active = [f for f in _HEATING_SGR_FIELDS if config.get(f, False)]
    if not active:
        return None  # Nothing to disable

    original = dict(config)
    for field in _HEATING_SGR_FIELDS:
        config[field] = False

    status, body = put_json(host, "heating/config_update", config)
    if status != 200:
        raise RuntimeError(
            f"Failed to disable heating SG Ready control: HTTP {status}: {body}")

    if verbose:
        print(f"  Disabled heating SG Ready fields: {active}")
    time.sleep(1)
    return original


def restore_heating_config(host, original_config, verbose):
    """Restore the original heating config."""
    if original_config is None:
        return
    status, body = put_json(host, "heating/config_update", original_config)
    if status != 200:
        print(f"  WARNING: Failed to restore heating config: HTTP {status}: {body}")
    elif verbose:
        print("  Restored original heating config")
    time.sleep(1)


# ---------------------------------------------------------------------------
# Automation rule builders
# ---------------------------------------------------------------------------

TRIGGER_TEMPERATURE_NOW = 23
TRIGGER_EM_RELAY_SWITCH = 21
TRIGGER_EM_SG_READY_SWITCH = 22

ACTION_EM_RELAY_SWITCH = 13
ACTION_EM_SG_READY_SWITCH = 16


def make_temp_trigger(comparison, value_c10, temp_type=0):
    return [TRIGGER_TEMPERATURE_NOW, {
        "type": temp_type,
        "comparison": comparison,
        "value": value_c10,
    }]


def make_relay_trigger(index, closed):
    """Trigger that fires when a relay changes to the given state."""
    return [TRIGGER_EM_RELAY_SWITCH, {
        "index": index,
        "closed": closed,
    }]


def make_sg_ready_trigger(index, closed):
    """Trigger that fires when an SG Ready output changes to the given state."""
    return [TRIGGER_EM_SG_READY_SWITCH, {
        "index": index,
        "closed": closed,
    }]


def make_relay_action(index, closed):
    return [ACTION_EM_RELAY_SWITCH, {
        "index": index,
        "closed": closed,
    }]


def make_sg_ready_action(index, closed):
    return [ACTION_EM_SG_READY_SWITCH, {
        "index": index,
        "closed": closed,
    }]


def make_task(trigger, action, delay=0):
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


# Unique temperature per test to avoid change-detection suppression.
_temp_counter = [200]


def next_temp():
    """Return a unique temperature value for each call."""
    _temp_counter[0] += 10
    return _temp_counter[0]


# ---------------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------------

def test_01_temp_closes_sg_ready(host, verbose):
    """Temperature trigger -> close SG Ready 0."""
    print("  [01] Temperature trigger -> close SG Ready 0 ...")

    reset_all_outputs(host)

    task = make_task(
        trigger=make_temp_trigger(comparison=0, value_c10=-100),  # > -10.0 C
        action=make_sg_ready_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    push_temperatures(host, next_temp())
    time.sleep(1)

    if not wait_for_sg_ready(host, 0, True, timeout=15):
        raise TestFailure(
            f"SG Ready 0 did not close. State: {get_sg_ready_state(host, 0)}")

    print("    PASS")


def test_02_temp_closes_sg_ready_1(host, verbose):
    """Temperature trigger -> close SG Ready 1."""
    print("  [02] Temperature trigger -> close SG Ready 1 ...")

    set_sg_ready_direct(host, 1, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_temp_trigger(comparison=0, value_c10=-100),
        action=make_sg_ready_action(index=1, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    push_temperatures(host, next_temp())
    time.sleep(1)

    if not wait_for_sg_ready(host, 1, True, timeout=15):
        raise TestFailure(
            f"SG Ready 1 did not close. State: {get_sg_ready_state(host, 1)}")

    print("    PASS")


def test_03_relay_trigger_sg_ready_action(host, verbose):
    """Relay state change triggers SG Ready action.

    Rule: when relay 0 closes -> close SG Ready 0.
    Activate by directly closing relay 0 via outputs_update.
    """
    print("  [03] Relay closes -> SG Ready 0 closes (cross-module) ...")

    reset_all_outputs(host)

    task = make_task(
        trigger=make_relay_trigger(index=0, closed=True),
        action=make_sg_ready_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Close relay 0 directly to fire the trigger
    set_relay_direct(host, 0, True)

    if not wait_for_sg_ready(host, 0, True, timeout=15):
        raise TestFailure(
            f"SG Ready 0 did not close after relay 0 closed. "
            f"State: {get_sg_ready_state(host, 0)}")

    print("    PASS")


def test_04_sg_ready_trigger_relay_action(host, verbose):
    """SG Ready state change triggers relay action.

    Rule: when SG Ready 0 closes -> close relay 1.
    Activate by directly closing SG Ready 0 via outputs_update.
    """
    print("  [04] SG Ready 0 closes -> relay 1 closes (cross-module) ...")

    reset_all_outputs(host)

    task = make_task(
        trigger=make_sg_ready_trigger(index=0, closed=True),
        action=make_relay_action(index=1, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Close SG Ready 0 directly
    set_sg_ready_direct(host, 0, True)

    if not wait_for_relay(host, 1, True, timeout=15):
        raise TestFailure(
            f"Relay 1 did not close after SG Ready 0 closed. "
            f"State: {get_relay_state(host, 1)}")

    print("    PASS")


def test_05_relay_open_trigger(host, verbose):
    """Relay opening triggers an action.

    Rule: when relay 0 opens -> close SG Ready 1.
    """
    print("  [05] Relay 0 opens -> SG Ready 1 closes ...")

    reset_all_outputs(host)
    # First close relay 0
    set_relay_direct(host, 0, True)
    time.sleep(1)

    task = make_task(
        trigger=make_relay_trigger(index=0, closed=False),
        action=make_sg_ready_action(index=1, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Now open relay 0 to fire the trigger
    set_relay_direct(host, 0, False)

    if not wait_for_sg_ready(host, 1, True, timeout=15):
        raise TestFailure(
            f"SG Ready 1 did not close after relay 0 opened. "
            f"State: {get_sg_ready_state(host, 1)}")

    print("    PASS")


def test_06_sg_ready_open_trigger(host, verbose):
    """SG Ready opening triggers an action.

    Rule: when SG Ready 1 opens -> close relay 0.
    """
    print("  [06] SG Ready 1 opens -> relay 0 closes ...")

    reset_all_outputs(host)
    # First close SG Ready 1
    set_sg_ready_direct(host, 1, True)
    time.sleep(1)

    task = make_task(
        trigger=make_sg_ready_trigger(index=1, closed=False),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Open SG Ready 1 to fire the trigger
    set_sg_ready_direct(host, 1, False)

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close after SG Ready 1 opened. "
            f"State: {get_relay_state(host, 0)}")

    print("    PASS")


def test_07_both_relays_and_sg_ready(host, verbose):
    """Four rules controlling all outputs.

    Rule 1: temp > -10 -> close relay 0
    Rule 2: temp > -10 -> close relay 1
    Rule 3: temp > -10 -> close SG Ready 0
    Rule 4: temp > -10 -> close SG Ready 1
    """
    print("  [07] Temperature closes all 4 outputs ...")

    reset_all_outputs(host)

    tasks = [
        make_task(
            trigger=make_temp_trigger(comparison=0, value_c10=-100),
            action=make_relay_action(index=0, closed=True),
        ),
        make_task(
            trigger=make_temp_trigger(comparison=0, value_c10=-100),
            action=make_relay_action(index=1, closed=True),
        ),
        make_task(
            trigger=make_temp_trigger(comparison=0, value_c10=-100),
            action=make_sg_ready_action(index=0, closed=True),
        ),
        make_task(
            trigger=make_temp_trigger(comparison=0, value_c10=-100),
            action=make_sg_ready_action(index=1, closed=True),
        ),
    ]
    set_automation_config(host, tasks)
    time.sleep(1)

    push_temperatures(host, next_temp())
    time.sleep(1)

    for idx in range(2):
        if not wait_for_relay(host, idx, True, timeout=15):
            raise TestFailure(
                f"Relay {idx} did not close. State: {get_relay_state(host, idx)}")
    for idx in range(2):
        if not wait_for_sg_ready(host, idx, True, timeout=15):
            raise TestFailure(
                f"SG Ready {idx} did not close. "
                f"State: {get_sg_ready_state(host, idx)}")

    print("    PASS")


def test_08_modify_sg_ready_rule(host, verbose):
    """Modify SG Ready rule at runtime: change from close to open."""
    print("  [08] Modify SG Ready rule: close -> open ...")

    reset_all_outputs(host)

    # First: close SG Ready 0 via temperature trigger
    task = make_task(
        trigger=make_temp_trigger(comparison=0, value_c10=-100),
        action=make_sg_ready_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    push_temperatures(host, next_temp())
    time.sleep(1)

    if not wait_for_sg_ready(host, 0, True, timeout=15):
        raise TestFailure("SG Ready 0 did not close initially")

    # Now modify: open SG Ready 0
    task = make_task(
        trigger=make_temp_trigger(comparison=0, value_c10=-100),
        action=make_sg_ready_action(index=0, closed=False),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    push_temperatures(host, next_temp())
    time.sleep(1)

    if not wait_for_sg_ready(host, 0, False, timeout=15):
        raise TestFailure(
            f"SG Ready 0 did not open after rule change. "
            f"State: {get_sg_ready_state(host, 0)}")

    print("    PASS")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

ALL_TESTS = [
    test_01_temp_closes_sg_ready,
    test_02_temp_closes_sg_ready_1,
    test_03_relay_trigger_sg_ready_action,
    test_04_sg_ready_trigger_relay_action,
    test_05_relay_open_trigger,
    test_06_sg_ready_open_trigger,
    test_07_both_relays_and_sg_ready,
    test_08_modify_sg_ready_rule,
]


def main():
    parser = argparse.ArgumentParser(
        description="Integration test for automation SG Ready and cross-module triggers.",
        epilog=(
            "Example:\n"
            "  python3 test_automation_sg_ready.py 192.168.0.33\n"
            "  python3 test_automation_sg_ready.py 192.168.0.33 --verbose\n"
            "  python3 test_automation_sg_ready.py 192.168.0.33 --test 04"
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

    print(f"=== Automation SG Ready / Cross-Module Test ===")
    print(f"Device: {host}\n")

    try:
        state = get_em_state(host)
        print(f"EM state: relays={state['relays']}, "
              f"sg_ready={state['sg_ready_outputs']}")
    except Exception as e:
        print(f"ERROR: Cannot connect to device: {e}")
        sys.exit(1)

    # Ensure temperatures module is in push mode
    print("Configuring temperatures for push mode ...")
    try:
        ensure_temperatures_push_mode(host)
    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(1)

    # Temporarily disable heating SG Ready control if active
    print("Checking heating module SG Ready control ...")
    original_heating = None
    try:
        original_heating = disable_heating_sgr_control(host, verbose)
        if original_heating:
            print("  Heating SG Ready control disabled for testing")
        else:
            print("  No heating SG Ready control active")
    except Exception as e:
        print(f"ERROR: Cannot disable heating SG Ready control: {e}")
        sys.exit(1)

    original_config = get_automation_config(host)
    if verbose:
        print(f"Original automation config: {json.dumps(original_config, indent=2)}")

    print()

    tests = ALL_TESTS
    if args.test:
        tests = [t for t in tests if args.test in t.__name__]
        if not tests:
            print(f"No tests matching '{args.test}'")
            sys.exit(1)

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

    # Cleanup
    print("\nCleaning up ...")
    try:
        set_automation_config(host, original_config.get("tasks", []))
    except Exception as e:
        print(f"  WARNING: Failed to restore automation config: {e}")

    try:
        reset_all_outputs(host)
    except Exception as e:
        print(f"  WARNING: Failed to reset outputs: {e}")

    restore_heating_config(host, original_heating, verbose)

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
