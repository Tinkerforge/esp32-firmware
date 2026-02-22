#!/usr/bin/env python3
"""Integration test for automation delay / cooldown functionality.

The ``delay`` field on each automation task specifies a delay in seconds
(0-86400).  When a trigger matches:
  - The action is scheduled to run after ``delay`` seconds.
  - The same rule cannot re-fire until the delay has elapsed (cooldown).
  - A config change (apply_config) resets all cooldown timers.

Usage:
    python3 test_automation_delay.py <device-ip>
    python3 test_automation_delay.py <device-ip> --verbose

Prerequisites on the device:
    - Energy Manager v2 with relays
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


def send_trigger(host, suffix, method="GET", payload=None):
    url = f"http://{host}/automation_trigger/{suffix}"
    data = payload.encode("utf-8") if payload else None
    req = urllib.request.Request(url, data=data, method=method)
    if data:
        req.add_header("Content-Type", "text/plain")
    try:
        with urllib.request.urlopen(req, timeout=10) as resp:
            return resp.status, resp.read().decode("utf-8", errors="replace")
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode("utf-8", errors="replace")


# ---------------------------------------------------------------------------
# Device state helpers
# ---------------------------------------------------------------------------

def get_relay_state(host, index=0):
    return get_json(host, "energy_manager/state")["relays"][index]


def set_relay_direct(host, index, closed):
    payload = [255, 255, 255, 255]
    payload[2 + index] = 1 if closed else 0
    status, body = put_json(host, "energy_manager/outputs_update", payload)
    if status != 200:
        raise RuntimeError(f"outputs_update failed: HTTP {status}: {body}")


def set_automation_config(host, tasks):
    status, body = put_json(host, "automation/config_update",
                            {"tasks": tasks})
    if status != 200:
        raise RuntimeError(
            f"automation/config_update failed: HTTP {status}: {body}")


def get_automation_config(host):
    return get_json(host, "automation/config")


def wait_for_relay(host, index, expected, timeout=15, poll_interval=0.5):
    deadline = time.time() + timeout
    while time.time() < deadline:
        if get_relay_state(host, index) == expected:
            return True
        time.sleep(poll_interval)
    return False


def ensure_temperatures_push_mode(host):
    config = get_json(host, "temperatures/config")
    if not config.get("enable", False) or config.get("source", 0) != 1:
        config["enable"] = True
        config["source"] = 1
        status, body = put_json(host, "temperatures/config_update", config)
        if status != 200:
            raise RuntimeError(
                f"temperatures push mode failed: HTTP {status}: {body}")
        time.sleep(1)


_temp_counter = [300]


def next_temp():
    _temp_counter[0] += 10
    return _temp_counter[0]


def push_temperatures(host, value_c10):
    now = int(time.time())
    first_date = (now - now % 86400) // 60
    status, body = put_json(host, "temperatures/temperatures_update", {
        "first_date": first_date,
        "temperatures": [value_c10] * 48,
    })
    if status != 200:
        raise RuntimeError(
            f"temperatures_update failed: HTTP {status}: {body}")


# ---------------------------------------------------------------------------
# Rule builders
# ---------------------------------------------------------------------------

TRIGGER_HTTP = 18
TRIGGER_TEMPERATURE_NOW = 23
ACTION_EM_RELAY_SWITCH = 13
HTTP_GET_POST_PUT = 4


def make_http_trigger(suffix):
    return [TRIGGER_HTTP, {
        "method": HTTP_GET_POST_PUT,
        "url_suffix": suffix,
        "payload": "",
    }]


def make_temp_trigger(comparison, value_c10):
    return [TRIGGER_TEMPERATURE_NOW, {
        "type": 0,
        "comparison": comparison,
        "value": value_c10,
    }]


def make_relay_action(index, closed):
    return [ACTION_EM_RELAY_SWITCH, {
        "index": index,
        "closed": closed,
    }]


def make_task(trigger, action, delay=0):
    return {"trigger": trigger, "action": action, "delay": delay}


# ---------------------------------------------------------------------------
# Test helpers
# ---------------------------------------------------------------------------

class TestFailure(Exception):
    pass


# ---------------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------------

def test_01_delay_defers_action(host, verbose):
    """HTTP trigger with 3s delay: relay must NOT close at 1s, MUST close by 5s."""
    print("  [01] Delay defers action (3s) ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_http_trigger("delay_3s"),
        action=make_relay_action(0, True),
        delay=3,
    )
    set_automation_config(host, [task])
    time.sleep(1)

    status, _ = send_trigger(host, "delay_3s")
    if status != 200:
        raise TestFailure(f"Trigger HTTP {status}")

    # After ~1s the relay must still be open
    time.sleep(1)
    if get_relay_state(host, 0):
        raise TestFailure("Relay closed too early (within 1s of a 3s delay)")

    # By ~5s it must be closed
    if not wait_for_relay(host, 0, True, timeout=5):
        raise TestFailure("Relay did not close after 3s delay")

    print("    PASS")


def test_02_zero_delay_immediate(host, verbose):
    """HTTP trigger with 0s delay: relay closes immediately."""
    print("  [02] Zero delay fires immediately ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_http_trigger("delay_0"),
        action=make_relay_action(0, True),
        delay=0,
    )
    set_automation_config(host, [task])
    time.sleep(1)

    status, _ = send_trigger(host, "delay_0")
    if status != 200:
        raise TestFailure(f"Trigger HTTP {status}")

    if not wait_for_relay(host, 0, True, timeout=3):
        raise TestFailure("Relay did not close with zero delay")

    print("    PASS")


def test_03_cooldown_suppresses_retrigger(host, verbose):
    """During cooldown period, re-triggering the same rule is suppressed."""
    print("  [03] Cooldown suppresses re-trigger ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    # Delay=4s  -> action fires at ~4s, cooldown until ~4s
    task = make_task(
        trigger=make_http_trigger("cooldown"),
        action=make_relay_action(0, True),
        delay=4,
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # First trigger
    send_trigger(host, "cooldown")
    # Wait for action to fire and relay to close
    if not wait_for_relay(host, 0, True, timeout=8):
        raise TestFailure("Relay did not close after first trigger")

    # Open relay directly, then immediately re-trigger
    set_relay_direct(host, 0, False)
    time.sleep(0.5)
    send_trigger(host, "cooldown")

    # The rule is still in cooldown (last_run was set to now+4s on first
    # fire, action ran at ~4s, but the cooldown check is deadline_elapsed
    # on that same timestamp).  The re-trigger should be suppressed.
    time.sleep(2)
    if get_relay_state(host, 0):
        raise TestFailure("Relay closed during cooldown -- should be suppressed")

    # Eventually cooldown expires and a new trigger should work.
    # Wait a bit longer (total ~6s from first trigger should be enough for
    # the 4s cooldown to expire) then re-trigger.
    time.sleep(4)
    send_trigger(host, "cooldown")
    if not wait_for_relay(host, 0, True, timeout=8):
        raise TestFailure("Relay did not close after cooldown expired")

    print("    PASS")


def test_04_two_rules_different_delays(host, verbose):
    """Two rules: 0s delay and 3s delay fire at correct times."""
    print("  [04] Two rules with different delays ...")

    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    time.sleep(0.5)

    tasks = [
        make_task(
            trigger=make_http_trigger("fast"),
            action=make_relay_action(0, True),
            delay=0,
        ),
        make_task(
            trigger=make_http_trigger("slow"),
            action=make_relay_action(1, True),
            delay=3,
        ),
    ]
    set_automation_config(host, tasks)
    time.sleep(1)

    # Fire both triggers at the same time
    send_trigger(host, "fast")
    send_trigger(host, "slow")

    # Relay 0 should close immediately
    if not wait_for_relay(host, 0, True, timeout=3):
        raise TestFailure("Relay 0 (0s delay) did not close immediately")

    # Relay 1 should still be open after 1s
    time.sleep(1)
    if get_relay_state(host, 1):
        raise TestFailure("Relay 1 (3s delay) closed too early")

    # Relay 1 should close by ~5s
    if not wait_for_relay(host, 1, True, timeout=5):
        raise TestFailure("Relay 1 (3s delay) did not close in time")

    print("    PASS")


def test_05_delay_with_temperature_trigger(host, verbose):
    """Temperature trigger with 3s delay."""
    print("  [05] Temperature trigger with 3s delay ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_temp_trigger(comparison=0, value_c10=-100),  # > -10 C
        action=make_relay_action(0, True),
        delay=3,
    )
    set_automation_config(host, [task])
    time.sleep(1)

    push_temperatures(host, next_temp())

    # Must not fire within 1s
    time.sleep(1)
    if get_relay_state(host, 0):
        raise TestFailure("Relay closed too early (within 1s of 3s delay)")

    # Must fire by 5s
    if not wait_for_relay(host, 0, True, timeout=5):
        raise TestFailure("Relay did not close after 3s delay on temp trigger")

    print("    PASS")


def test_06_config_change_resets_cooldown(host, verbose):
    """Changing config resets all cooldown timers."""
    print("  [06] Config change resets cooldown ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    # Rule with 5s delay
    task = make_task(
        trigger=make_http_trigger("reset_cd"),
        action=make_relay_action(0, True),
        delay=5,
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Fire the trigger -> action scheduled in 5s, cooldown set
    send_trigger(host, "reset_cd")
    if not wait_for_relay(host, 0, True, timeout=10):
        raise TestFailure("Initial trigger did not fire")

    # Now relay is closed; open it and re-apply the SAME config.
    # This should reset cooldown so re-triggering works immediately.
    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    # Re-apply config (same rules) to reset cooldown
    set_automation_config(host, [task])
    time.sleep(1)

    send_trigger(host, "reset_cd")
    # With cooldown reset, the 5s delay starts fresh
    if not wait_for_relay(host, 0, True, timeout=10):
        raise TestFailure("Trigger did not fire after config reset")

    print("    PASS")


def test_07_delay_no_early_fire(host, verbose):
    """Rule with 6s delay: verify no fire at 3s, fire by 8s."""
    print("  [07] Longer delay (6s): no early fire ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_http_trigger("delay_6s"),
        action=make_relay_action(0, True),
        delay=6,
    )
    set_automation_config(host, [task])
    time.sleep(1)

    send_trigger(host, "delay_6s")

    # Check at 3s -- must still be open
    time.sleep(3)
    if get_relay_state(host, 0):
        raise TestFailure("Relay closed at 3s (delay is 6s)")

    # Must close by 8s total (2s margin)
    if not wait_for_relay(host, 0, True, timeout=5):
        raise TestFailure("Relay did not close after 6s delay")

    print("    PASS")


def test_08_modify_delay_at_runtime(host, verbose):
    """Change delay from 5s to 0s at runtime, second trigger fires immediately."""
    print("  [08] Modify delay 5s -> 0s at runtime ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    # Start with 5s delay
    task = make_task(
        trigger=make_http_trigger("mod_delay"),
        action=make_relay_action(0, True),
        delay=5,
    )
    set_automation_config(host, [task])
    time.sleep(1)

    send_trigger(host, "mod_delay")
    if not wait_for_relay(host, 0, True, timeout=10):
        raise TestFailure("First trigger (5s delay) did not fire")

    # Reset relay and change delay to 0
    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task["delay"] = 0
    set_automation_config(host, [task])
    time.sleep(1)

    send_trigger(host, "mod_delay")
    # With 0s delay it should be near-instant
    if not wait_for_relay(host, 0, True, timeout=3):
        raise TestFailure("Trigger with 0s delay did not fire immediately")

    print("    PASS")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

ALL_TESTS = [
    test_01_delay_defers_action,
    test_02_zero_delay_immediate,
    test_03_cooldown_suppresses_retrigger,
    test_04_two_rules_different_delays,
    test_05_delay_with_temperature_trigger,
    test_06_config_change_resets_cooldown,
    test_07_delay_no_early_fire,
    test_08_modify_delay_at_runtime,
]


def main():
    parser = argparse.ArgumentParser(
        description="Integration test for automation delay / cooldown.",
        epilog=(
            "Example:\n"
            "  python3 test_automation_delay.py 192.168.0.33\n"
            "  python3 test_automation_delay.py 192.168.0.33 --verbose\n"
            "  python3 test_automation_delay.py 192.168.0.33 --test 03"
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

    print("=== Automation Delay / Cooldown Test ===")
    print(f"Device: {host}\n")

    try:
        state = get_json(host, "energy_manager/state")
        print(f"EM state: relays={state['relays']}")
    except Exception as e:
        print(f"ERROR: Cannot connect to device: {e}")
        sys.exit(1)

    # Temperatures push mode needed for test_05
    print("Configuring temperatures for push mode ...")
    try:
        ensure_temperatures_push_mode(host)
    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(1)

    original_config = get_automation_config(host)
    if verbose:
        print(f"Original config: {json.dumps(original_config, indent=2)}")

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
        print(f"  WARNING: Failed to restore config: {e}")
    try:
        set_relay_direct(host, 0, False)
        set_relay_direct(host, 1, False)
    except Exception as e:
        print(f"  WARNING: Failed to reset relays: {e}")

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
