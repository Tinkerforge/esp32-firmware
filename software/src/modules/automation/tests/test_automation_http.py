#!/usr/bin/env python3
"""Integration test for automation with HTTP triggers.

Tests that HTTP triggers fire automation actions correctly. The HTTP
trigger is the most directly testable trigger mechanism: send a request
to /automation_trigger/<suffix> and the action executes immediately.

Usage:
    python3 test_automation_http.py <device-ip>
    python3 test_automation_http.py <device-ip> --verbose

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


def get_text(host, path):
    """GET a plain-text endpoint."""
    url = f"http://{host}/{path}"
    req = urllib.request.Request(url)
    with urllib.request.urlopen(req, timeout=5) as resp:
        return resp.read().decode("utf-8", errors="replace")


def send_trigger(host, suffix, method="GET", payload=None):
    """Send an HTTP request to /automation_trigger/<suffix>.

    Returns (status_code, response_body).
    """
    url = f"http://{host}/automation_trigger/{suffix}"
    data = payload.encode("utf-8") if payload else None
    req = urllib.request.Request(url, data=data, method=method)
    if data:
        req.add_header("Content-Type", "text/plain")
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

def get_relay_state(host, index=0):
    state = get_json(host, "energy_manager/state")
    return state["relays"][index]



def set_relay_direct(host, index, closed):
    payload = [255, 255, 255, 255]
    payload[2 + index] = 1 if closed else 0
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


def wait_for_relay(host, index, expected, timeout=15, poll_interval=0.5):
    deadline = time.time() + timeout
    while time.time() < deadline:
        actual = get_relay_state(host, index)
        if actual == expected:
            return True
        time.sleep(poll_interval)
    return False



# ---------------------------------------------------------------------------
# Automation rule builders
# ---------------------------------------------------------------------------

# Trigger IDs
TRIGGER_HTTP = 18

# Action IDs
ACTION_PRINT = 1
ACTION_EM_RELAY_SWITCH = 13

# HTTP methods
HTTP_GET = 0
HTTP_POST = 1
HTTP_PUT = 2
HTTP_POST_PUT = 3
HTTP_GET_POST_PUT = 4


def make_http_trigger(url_suffix, method=HTTP_GET_POST_PUT, payload=""):
    return [TRIGGER_HTTP, {
        "method": method,
        "url_suffix": url_suffix,
        "payload": payload,
    }]


def make_relay_action(index, closed):
    return [ACTION_EM_RELAY_SWITCH, {
        "index": index,
        "closed": closed,
    }]



def make_print_action(message):
    return [ACTION_PRINT, {
        "message": message,
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


# ---------------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------------

def test_01_http_get_closes_relay(host, verbose):
    """HTTP GET trigger closes relay 0."""
    print("  [01] HTTP GET trigger -> close relay 0 ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_http_trigger("test_relay_close", method=HTTP_GET),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    status, body = send_trigger(host, "test_relay_close", method="GET")
    if status != 200:
        raise TestFailure(f"Trigger returned HTTP {status}: {body}")

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close. State: {get_relay_state(host, 0)}")

    print("    PASS")


def test_02_http_get_opens_relay(host, verbose):
    """HTTP GET trigger opens relay 0."""
    print("  [02] HTTP GET trigger -> open relay 0 ...")

    # Relay should be closed from test_01
    assert get_relay_state(host, 0), "Relay 0 should be closed"

    task = make_task(
        trigger=make_http_trigger("test_relay_open", method=HTTP_GET),
        action=make_relay_action(index=0, closed=False),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    status, body = send_trigger(host, "test_relay_open", method="GET")
    if status != 200:
        raise TestFailure(f"Trigger returned HTTP {status}: {body}")

    if not wait_for_relay(host, 0, False, timeout=15):
        raise TestFailure(
            f"Relay 0 did not open. State: {get_relay_state(host, 0)}")

    print("    PASS")


def test_03_http_post_trigger(host, verbose):
    """HTTP POST trigger closes relay 0."""
    print("  [03] HTTP POST trigger -> close relay 0 ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_http_trigger("test_post", method=HTTP_POST),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # POST should match
    status, body = send_trigger(host, "test_post", method="POST")
    if status != 200:
        raise TestFailure(f"POST trigger returned HTTP {status}: {body}")

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close. State: {get_relay_state(host, 0)}")

    print("    PASS")


def test_04_wrong_method_rejected(host, verbose):
    """HTTP trigger with wrong method returns 405 and does not fire."""
    print("  [04] Wrong HTTP method -> 405, relay stays open ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    # Rule expects GET only
    task = make_task(
        trigger=make_http_trigger("test_method", method=HTTP_GET),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Send POST instead of GET
    status, body = send_trigger(host, "test_method", method="POST")
    if status != 405:
        raise TestFailure(f"Expected HTTP 405 but got {status}: {body}")

    time.sleep(1)
    if get_relay_state(host, 0):
        raise TestFailure("Relay 0 closed but wrong method was used")

    print("    PASS")


def test_05_wrong_suffix_rejected(host, verbose):
    """HTTP trigger with wrong URL suffix returns 404."""
    print("  [05] Wrong URL suffix -> 404 ...")

    task = make_task(
        trigger=make_http_trigger("correct_suffix"),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    status, body = send_trigger(host, "wrong_suffix", method="GET")
    if status != 404:
        raise TestFailure(f"Expected HTTP 404 but got {status}: {body}")

    print("    PASS")


def test_06_payload_matching(host, verbose):
    """HTTP trigger with payload matching."""
    print("  [06] Payload matching -> correct payload fires, wrong rejected ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_http_trigger("test_payload", method=HTTP_POST, payload="secret"),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Wrong payload should be rejected (422)
    status, body = send_trigger(host, "test_payload", method="POST", payload="wrong")
    if status != 422:
        raise TestFailure(f"Wrong payload: expected HTTP 422 but got {status}: {body}")

    time.sleep(1)
    if get_relay_state(host, 0):
        raise TestFailure("Relay closed with wrong payload")

    # Correct payload should work
    status, body = send_trigger(host, "test_payload", method="POST", payload="secret")
    if status != 200:
        raise TestFailure(f"Correct payload: HTTP {status}: {body}")

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close with correct payload. "
            f"State: {get_relay_state(host, 0)}")

    print("    PASS")


def test_07_print_action(host, verbose):
    """HTTP trigger fires Print action, message appears in event log."""
    print("  [07] HTTP trigger -> print message ...")

    marker = f"AUTOTEST_{int(time.time())}"

    task = make_task(
        trigger=make_http_trigger("test_print"),
        action=make_print_action(marker),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    status, body = send_trigger(host, "test_print", method="GET")
    if status != 200:
        raise TestFailure(f"Trigger returned HTTP {status}: {body}")

    time.sleep(1)

    # Check event log for the marker string
    log_text = get_text(host, "event_log")
    if marker not in log_text:
        if verbose:
            # Print last 500 chars of log for debugging
            print(f"    Event log tail: ...{log_text[-500:]}")
        raise TestFailure(f"Marker '{marker}' not found in event log")

    print("    PASS")


def test_08_multiple_http_rules(host, verbose):
    """Multiple HTTP triggers with different suffixes, each controlling a relay."""
    print("  [08] Multiple HTTP triggers: different suffixes ...")

    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    time.sleep(0.5)

    tasks = [
        make_task(
            trigger=make_http_trigger("relay0_close"),
            action=make_relay_action(index=0, closed=True),
        ),
        make_task(
            trigger=make_http_trigger("relay1_close"),
            action=make_relay_action(index=1, closed=True),
        ),
    ]
    set_automation_config(host, tasks)
    time.sleep(1)

    # Fire only relay0 trigger
    status, _ = send_trigger(host, "relay0_close", method="GET")
    if status != 200:
        raise TestFailure(f"relay0_close trigger: HTTP {status}")

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure("Relay 0 did not close")

    # Relay 1 should still be open
    time.sleep(1)
    if get_relay_state(host, 1):
        raise TestFailure("Relay 1 closed but only relay0_close was triggered")

    # Now fire relay1 trigger
    status, _ = send_trigger(host, "relay1_close", method="GET")
    if status != 200:
        raise TestFailure(f"relay1_close trigger: HTTP {status}")

    if not wait_for_relay(host, 1, True, timeout=15):
        raise TestFailure("Relay 1 did not close")

    print("    PASS")


def test_09_put_method(host, verbose):
    """HTTP PUT trigger works correctly."""
    print("  [09] HTTP PUT trigger -> close relay 0 ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_http_trigger("test_put", method=HTTP_PUT),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    status, body = send_trigger(host, "test_put", method="PUT")
    if status != 200:
        raise TestFailure(f"PUT trigger returned HTTP {status}: {body}")

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close. State: {get_relay_state(host, 0)}")

    print("    PASS")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

ALL_TESTS = [
    test_01_http_get_closes_relay,
    test_02_http_get_opens_relay,
    test_03_http_post_trigger,
    test_04_wrong_method_rejected,
    test_05_wrong_suffix_rejected,
    test_06_payload_matching,
    test_07_print_action,
    test_08_multiple_http_rules,
    test_09_put_method,
]


def main():
    parser = argparse.ArgumentParser(
        description="Integration test for automation HTTP triggers.",
        epilog=(
            "Example:\n"
            "  python3 test_automation_http.py 192.168.0.33\n"
            "  python3 test_automation_http.py 192.168.0.33 --verbose\n"
            "  python3 test_automation_http.py 192.168.0.33 --test 06"
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

    print(f"=== Automation HTTP Trigger Test ===")
    print(f"Device: {host}\n")

    try:
        state = get_json(host, "energy_manager/state")
        print(f"EM state: relays={state['relays']}")
    except Exception as e:
        print(f"ERROR: Cannot connect to device: {e}")
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

    for label, fn in [
        ("relay 0", lambda: set_relay_direct(host, 0, False)),
        ("relay 1", lambda: set_relay_direct(host, 1, False)),
    ]:
        try:
            fn()
        except Exception as e:
            if verbose:
                print(f"  WARNING: Failed to reset {label}: {e}")

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
