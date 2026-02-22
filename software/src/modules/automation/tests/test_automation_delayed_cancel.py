#!/usr/bin/env python3
"""Integration test for cancellation of pending delayed actions on config change.

When a trigger fires with delay > 0, the action is scheduled to run after the
delay.  If the automation config is changed (apply_config) while a delayed
action is still pending, the pending action MUST be cancelled.  This prevents
a use-after-free: the pending lambda holds a raw pointer into the old
config_in_use tree, which is freed when apply_config() replaces it.

These tests verify:
  - A pending delayed action is cancelled when config changes.
  - After cancellation, new rules from the updated config work correctly.
  - Multiple pending delayed actions are all cancelled on a single config change.
  - A delayed action that is NOT interrupted still fires normally (sanity check).

Usage:
    python3 test_automation_delayed_cancel.py <device-ip>
    python3 test_automation_delayed_cancel.py <device-ip> --verbose

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


# ---------------------------------------------------------------------------
# Rule builders
# ---------------------------------------------------------------------------

TRIGGER_HTTP = 18
ACTION_EM_RELAY_SWITCH = 13
HTTP_GET_POST_PUT = 4


def make_http_trigger(suffix):
    return [TRIGGER_HTTP, {
        "method": HTTP_GET_POST_PUT,
        "url_suffix": suffix,
        "payload": "",
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

def test_01_config_change_cancels_pending_delayed_action(host, verbose):
    """Fire trigger with 8s delay, change config at ~2s, old action must NOT fire."""
    print("  [01] Config change cancels pending delayed action ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    # Rule: HTTP trigger -> close relay 0 after 8s delay
    task = make_task(
        trigger=make_http_trigger("cancel_me"),
        action=make_relay_action(0, True),
        delay=8,
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Fire the trigger -- action is now pending (scheduled in ~8s)
    status, _ = send_trigger(host, "cancel_me")
    if status != 200:
        raise TestFailure(f"Trigger HTTP {status}")
    if verbose:
        print("    Trigger fired, delayed action pending (8s)")

    # Wait 2s, then change config to an empty rule set.
    # This calls apply_config() which must cancel the pending action.
    time.sleep(2)
    set_automation_config(host, [])
    if verbose:
        print("    Config changed to empty at ~2s, pending action should be cancelled")

    # Wait long enough for the original 8s delay to have expired
    # (wait another 8s from now = ~10s total since trigger).
    # The relay must remain open the entire time.
    for check in range(16):
        time.sleep(0.5)
        if get_relay_state(host, 0):
            raise TestFailure(
                f"Relay closed at ~{2 + 0.5 * (check + 1):.1f}s -- "
                "cancelled delayed action still fired!")

    if verbose:
        print("    Relay stayed open for 10s after trigger -- action was cancelled")

    print("    PASS")


def test_02_new_rules_work_after_cancellation(host, verbose):
    """After cancelling a pending action via config change, new rules work."""
    print("  [02] New rules work after cancellation ...")

    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    time.sleep(0.5)

    # Initial rule: close relay 0 after 8s delay
    old_task = make_task(
        trigger=make_http_trigger("old_rule"),
        action=make_relay_action(0, True),
        delay=8,
    )
    set_automation_config(host, [old_task])
    time.sleep(1)

    # Fire old trigger
    send_trigger(host, "old_rule")
    if verbose:
        print("    Old rule triggered (8s delay pending)")

    # After 2s, replace with a new rule: close relay 1 immediately
    time.sleep(2)
    new_task = make_task(
        trigger=make_http_trigger("new_rule"),
        action=make_relay_action(1, True),
        delay=0,
    )
    set_automation_config(host, [new_task])
    time.sleep(1)

    # Fire new trigger
    send_trigger(host, "new_rule")

    # New rule should work: relay 1 closes immediately
    if not wait_for_relay(host, 1, True, timeout=3):
        raise TestFailure("New rule did not fire after config change")

    # Old rule must be cancelled: relay 0 stays open
    time.sleep(8)
    if get_relay_state(host, 0):
        raise TestFailure("Old cancelled action still fired (relay 0 closed)")

    if verbose:
        print("    New rule worked, old action stayed cancelled")

    print("    PASS")


def test_03_multiple_pending_actions_all_cancelled(host, verbose):
    """Fire multiple delayed triggers, change config, ALL must be cancelled."""
    print("  [03] Multiple pending delayed actions all cancelled ...")

    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    time.sleep(0.5)

    # Two rules: both with long delays
    tasks = [
        make_task(
            trigger=make_http_trigger("multi_a"),
            action=make_relay_action(0, True),
            delay=8,
        ),
        make_task(
            trigger=make_http_trigger("multi_b"),
            action=make_relay_action(1, True),
            delay=10,
        ),
    ]
    set_automation_config(host, tasks)
    time.sleep(1)

    # Fire both triggers
    send_trigger(host, "multi_a")
    send_trigger(host, "multi_b")
    if verbose:
        print("    Both triggers fired (8s and 10s delays pending)")

    # After 2s, wipe config
    time.sleep(2)
    set_automation_config(host, [])
    if verbose:
        print("    Config cleared at ~2s")

    # Wait 12s total -- neither relay should close
    for check in range(20):
        time.sleep(0.5)
        r0 = get_relay_state(host, 0)
        r1 = get_relay_state(host, 1)
        if r0 or r1:
            raise TestFailure(
                f"Relay(s) closed at ~{2 + 0.5 * (check + 1):.1f}s: "
                f"relay0={r0}, relay1={r1} -- cancelled actions leaked!")

    if verbose:
        print("    Both relays stayed open for 12s -- all actions cancelled")

    print("    PASS")


def test_04_uninterrupted_delayed_action_still_fires(host, verbose):
    """Sanity check: delayed action fires normally when config is NOT changed."""
    print("  [04] Uninterrupted delayed action fires normally (sanity) ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_http_trigger("no_cancel"),
        action=make_relay_action(0, True),
        delay=3,
    )
    set_automation_config(host, [task])
    time.sleep(1)

    send_trigger(host, "no_cancel")

    # Should NOT fire within 1s
    time.sleep(1)
    if get_relay_state(host, 0):
        raise TestFailure("Relay closed too early (within 1s of 3s delay)")

    # Should fire by 5s
    if not wait_for_relay(host, 0, True, timeout=4):
        raise TestFailure("Delayed action did not fire (3s delay)")

    print("    PASS")


def test_05_rapid_config_changes_with_pending_actions(host, verbose):
    """Fire trigger, then rapidly change config 3 times -- no stale action fires."""
    print("  [05] Rapid config changes with pending actions ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    # Start with a rule that has 6s delay
    task = make_task(
        trigger=make_http_trigger("rapid"),
        action=make_relay_action(0, True),
        delay=6,
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Fire the trigger
    send_trigger(host, "rapid")
    if verbose:
        print("    Trigger fired (6s delay)")

    # Rapidly change config 3 times over ~3s.  Each change calls apply_config()
    # which should cancel any pending delayed action from the previous config.
    for i in range(3):
        time.sleep(1)
        # Each config is slightly different (different suffix) to force
        # a real config update, not a no-op.
        new_task = make_task(
            trigger=make_http_trigger(f"rapid_v{i}"),
            action=make_relay_action(0, True),
            delay=6,
        )
        set_automation_config(host, [new_task])
        if verbose:
            print(f"    Config change #{i + 1} at ~{i + 1}s")

    # Wait for the original delay to expire and some extra margin
    time.sleep(6)

    # Relay must still be open -- the original trigger's pending action
    # was cancelled by the first config change, and none of the new configs
    # had their triggers fired.
    if get_relay_state(host, 0):
        raise TestFailure(
            "Relay closed -- a stale delayed action survived rapid config changes")

    if verbose:
        print("    Relay stayed open through 3 rapid config changes")

    print("    PASS")


def test_06_cancel_then_retrigger_same_rule(host, verbose):
    """Cancel a delayed action via config re-apply, then re-trigger same rule."""
    print("  [06] Cancel then re-trigger same rule ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_http_trigger("refire"),
        action=make_relay_action(0, True),
        delay=6,
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Fire trigger (6s delay pending)
    send_trigger(host, "refire")
    if verbose:
        print("    First trigger fired (6s delay)")

    # After 2s, re-apply same config to cancel pending + reset cooldown
    time.sleep(2)
    set_automation_config(host, [task])
    time.sleep(1)

    # Relay must still be open (original action cancelled)
    if get_relay_state(host, 0):
        raise TestFailure("Relay closed before re-trigger")

    # Now fire the trigger again -- fresh delay starts
    send_trigger(host, "refire")
    if verbose:
        print("    Re-triggered after cancel (6s delay)")

    # Should NOT fire within 3s
    time.sleep(3)
    if get_relay_state(host, 0):
        raise TestFailure("Relay closed too early after re-trigger (3s of 6s delay)")

    # Should fire by 6s + margin
    if not wait_for_relay(host, 0, True, timeout=5):
        raise TestFailure("Re-triggered delayed action did not fire")

    if verbose:
        print("    Re-triggered action fired correctly after full delay")

    print("    PASS")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

ALL_TESTS = [
    test_01_config_change_cancels_pending_delayed_action,
    test_02_new_rules_work_after_cancellation,
    test_03_multiple_pending_actions_all_cancelled,
    test_04_uninterrupted_delayed_action_still_fires,
    test_05_rapid_config_changes_with_pending_actions,
    test_06_cancel_then_retrigger_same_rule,
]


def main():
    parser = argparse.ArgumentParser(
        description="Integration test: delayed action cancellation on config change.",
        epilog=(
            "Example:\n"
            "  python3 test_automation_delayed_cancel.py 192.168.0.47\n"
            "  python3 test_automation_delayed_cancel.py 192.168.0.47 --verbose\n"
            "  python3 test_automation_delayed_cancel.py 192.168.0.47 --test 03"
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

    print("=== Delayed Action Cancellation Test ===")
    print(f"Device: {host}\n")

    try:
        state = get_json(host, "energy_manager/state")
        print(f"EM state: relays={state['relays']}")
    except Exception as e:
        print(f"ERROR: Cannot connect to device: {e}")
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
