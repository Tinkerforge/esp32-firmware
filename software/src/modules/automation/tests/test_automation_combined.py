#!/usr/bin/env python3
"""Integration test for combined / mixed automation scenarios.

Tests that multiple trigger types coexist in a single config, that one
trigger can fire multiple actions, that chain reactions work (action A
triggers trigger B), and various edge cases (max rules, empty config,
rapid config replacement).

Usage:
    python3 test_automation_combined.py <device-ip>
    python3 test_automation_combined.py <device-ip> --verbose

Prerequisites on the device:
    - Energy Manager v2 with relays
    - temperatures module available
    - day_ahead_prices module available
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
    url = f"http://{host}/{path}"
    req = urllib.request.Request(url)
    with urllib.request.urlopen(req, timeout=5) as resp:
        return resp.read().decode("utf-8", errors="replace")


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


_temp_counter = [500]


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


def ensure_dap_push_mode(host):
    config = get_json(host, "day_ahead_prices/config")
    if config.get("source", 0) != 1:
        config["source"] = 1
        status, body = put_json(host, "day_ahead_prices/config_update", config)
        if status != 200:
            raise RuntimeError(
                f"DAP push mode failed: HTTP {status}: {body}")
        time.sleep(1)


_dap_offset = [0]


def push_dap_prices(host, current_price_ct1000):
    """Push DAP prices with the given price for the current hour."""
    _dap_offset[0] += 1
    now = int(time.time())
    midnight = now - now % 86400
    first_date = midnight // 60
    # 24 hourly prices; set all to a baseline, override the current hour
    prices = [5000] * 24
    current_hour = (now - midnight) // 3600
    if 0 <= current_hour < 24:
        prices[current_hour] = current_price_ct1000 + _dap_offset[0]
    status, body = put_json(host, "day_ahead_prices/prices_update", {
        "first_date": first_date,
        "resolution": 1,
        "prices": prices,
    })
    if status != 200:
        raise RuntimeError(f"prices_update failed: HTTP {status}: {body}")


# ---------------------------------------------------------------------------
# Rule builders
# ---------------------------------------------------------------------------

TRIGGER_HTTP = 18
TRIGGER_TEMPERATURE_NOW = 23
TRIGGER_DAP_NOW = 20
TRIGGER_EM_RELAY_SWITCH = 21

ACTION_PRINT = 1
ACTION_EM_RELAY_SWITCH = 13

HTTP_GET_POST_PUT = 4


def make_http_trigger(suffix, method=HTTP_GET_POST_PUT, payload=""):
    return [TRIGGER_HTTP, {
        "method": method,
        "url_suffix": suffix,
        "payload": payload,
    }]


def make_temp_trigger(comparison, value_c10):
    return [TRIGGER_TEMPERATURE_NOW, {
        "type": 0,
        "comparison": comparison,
        "value": value_c10,
    }]


def make_dap_trigger_absolute(comparison, value_ct):
    return [TRIGGER_DAP_NOW, {
        "type": 1,
        "comparison": comparison,
        "value": value_ct,
    }]


def make_relay_trigger(index, closed):
    return [TRIGGER_EM_RELAY_SWITCH, {
        "index": index,
        "closed": closed,
    }]


def make_relay_action(index, closed):
    return [ACTION_EM_RELAY_SWITCH, {
        "index": index,
        "closed": closed,
    }]


def make_print_action(message):
    return [ACTION_PRINT, {"message": message}]


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

def test_01_http_and_temp_coexist(host, verbose):
    """HTTP trigger + temperature trigger in the same config both work."""
    print("  [01] HTTP + temperature triggers coexist ...")

    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    time.sleep(0.5)

    tasks = [
        make_task(
            trigger=make_http_trigger("coexist"),
            action=make_relay_action(0, True),
        ),
        make_task(
            trigger=make_temp_trigger(comparison=0, value_c10=-100),
            action=make_relay_action(1, True),
        ),
    ]
    set_automation_config(host, tasks)
    time.sleep(1)

    # Fire HTTP trigger -> relay 0 closes
    status, _ = send_trigger(host, "coexist")
    if status != 200:
        raise TestFailure(f"HTTP trigger returned {status}")

    if not wait_for_relay(host, 0, True, timeout=10):
        raise TestFailure("Relay 0 did not close via HTTP trigger")

    # Relay 1 should still be open (temp not pushed yet)
    if get_relay_state(host, 1):
        raise TestFailure("Relay 1 closed prematurely")

    # Push temperature -> relay 1 closes
    push_temperatures(host, next_temp())
    if not wait_for_relay(host, 1, True, timeout=15):
        raise TestFailure("Relay 1 did not close via temperature trigger")

    print("    PASS")


def test_02_one_trigger_two_actions(host, verbose):
    """Single HTTP suffix triggers two actions: close relay 0 and relay 1."""
    print("  [02] One trigger -> two actions (relay 0 + relay 1) ...")

    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    time.sleep(0.5)

    tasks = [
        make_task(
            trigger=make_http_trigger("both_relays"),
            action=make_relay_action(0, True),
        ),
        make_task(
            trigger=make_http_trigger("both_relays"),
            action=make_relay_action(1, True),
        ),
    ]
    set_automation_config(host, tasks)
    time.sleep(1)

    status, _ = send_trigger(host, "both_relays")
    if status != 200:
        raise TestFailure(f"HTTP trigger returned {status}")

    if not wait_for_relay(host, 0, True, timeout=10):
        raise TestFailure("Relay 0 did not close")
    if not wait_for_relay(host, 1, True, timeout=10):
        raise TestFailure("Relay 1 did not close")

    print("    PASS")


def test_03_chain_reaction(host, verbose):
    """HTTP trigger -> close relay 0 -> relay trigger -> close relay 1."""
    print("  [03] Chain: HTTP -> relay 0 close -> relay trigger -> relay 1 close ...")

    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    time.sleep(0.5)

    tasks = [
        # Step 1: HTTP trigger closes relay 0
        make_task(
            trigger=make_http_trigger("chain_start"),
            action=make_relay_action(0, True),
        ),
        # Step 2: relay 0 closing triggers relay 1 close
        make_task(
            trigger=make_relay_trigger(0, True),
            action=make_relay_action(1, True),
        ),
    ]
    set_automation_config(host, tasks)
    time.sleep(1)

    status, _ = send_trigger(host, "chain_start")
    if status != 200:
        raise TestFailure(f"HTTP trigger returned {status}")

    # Both relays should eventually close
    if not wait_for_relay(host, 0, True, timeout=10):
        raise TestFailure("Relay 0 did not close")
    if not wait_for_relay(host, 1, True, timeout=10):
        raise TestFailure("Relay 1 did not close (chain reaction failed)")

    print("    PASS")


def test_04_dap_and_http_mixed(host, verbose):
    """DAP trigger and HTTP trigger in same config, controlling different relays."""
    print("  [04] DAP + HTTP triggers mixed ...")

    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    time.sleep(0.5)

    # First push a low price (below threshold) so the DAP trigger condition
    # is clearly "not met" before we install the rule.
    push_dap_prices(host, 1000)  # 1ct, below 5ct threshold
    time.sleep(2)

    tasks = [
        make_task(
            trigger=make_http_trigger("dap_mix"),
            action=make_relay_action(0, True),
        ),
        make_task(
            trigger=make_dap_trigger_absolute(comparison=0, value_ct=5),
            action=make_relay_action(1, True),
        ),
    ]
    set_automation_config(host, tasks)
    time.sleep(1)

    # Fire HTTP -> relay 0
    status, _ = send_trigger(host, "dap_mix")
    if status != 200:
        raise TestFailure(f"HTTP trigger returned {status}")
    if not wait_for_relay(host, 0, True, timeout=10):
        raise TestFailure("Relay 0 did not close via HTTP")

    # Push high price -> relay 1 (price > 5ct)
    push_dap_prices(host, 10000)  # 10ct, well above threshold
    if not wait_for_relay(host, 1, True, timeout=15):
        raise TestFailure("Relay 1 did not close via DAP trigger")

    print("    PASS")


def test_05_max_rules_print(host, verbose):
    """14 rules (max capacity): 14 HTTP triggers with Print actions."""
    print("  [05] Max rules (14): all fire ...")

    marker_base = f"MAX_{int(time.time())}"
    tasks = []
    for i in range(14):
        tasks.append(make_task(
            trigger=make_http_trigger(f"max_{i}"),
            action=make_print_action(f"{marker_base}_{i}"),
        ))
    set_automation_config(host, tasks)
    time.sleep(1)

    # Fire all 14 triggers
    for i in range(14):
        status, _ = send_trigger(host, f"max_{i}")
        if status != 200:
            raise TestFailure(f"Trigger max_{i} returned HTTP {status}")

    time.sleep(2)

    # Check event log for all markers
    log = get_text(host, "event_log")
    missing = []
    for i in range(14):
        if f"{marker_base}_{i}" not in log:
            missing.append(i)

    if missing:
        raise TestFailure(
            f"Missing markers in event log for rules: {missing}")

    print("    PASS")


def test_06_empty_config_no_handlers(host, verbose):
    """Empty config: HTTP triggers return 404 (no handlers registered)."""
    print("  [06] Empty config -> triggers return 404 ...")

    set_automation_config(host, [])
    time.sleep(1)

    status, _ = send_trigger(host, "nonexistent")
    if status != 404:
        raise TestFailure(f"Expected 404 for empty config, got {status}")

    # Verify config is actually empty
    config = get_automation_config(host)
    if config.get("tasks", []):
        raise TestFailure("Config not empty after setting empty tasks")

    print("    PASS")


def test_07_replace_config_old_stops(host, verbose):
    """Replace config: old trigger stops working, new trigger works."""
    print("  [07] Replace config: old trigger stops, new works ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    # Config A: suffix "old_trigger" closes relay 0
    set_automation_config(host, [make_task(
        trigger=make_http_trigger("old_trigger"),
        action=make_relay_action(0, True),
    )])
    time.sleep(1)

    # Verify old trigger works
    status, _ = send_trigger(host, "old_trigger")
    if status != 200:
        raise TestFailure(f"Old trigger returned {status}")
    if not wait_for_relay(host, 0, True, timeout=10):
        raise TestFailure("Relay did not close with old trigger")

    # Reset relay
    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    # Config B: replace with suffix "new_trigger"
    set_automation_config(host, [make_task(
        trigger=make_http_trigger("new_trigger"),
        action=make_relay_action(0, True),
    )])
    time.sleep(1)

    # Old trigger should now return 404
    status, _ = send_trigger(host, "old_trigger")
    if status != 404:
        raise TestFailure(
            f"Old trigger should return 404 after replace, got {status}")

    # Relay should still be open
    if get_relay_state(host, 0):
        raise TestFailure("Relay closed from old trigger after config replace")

    # New trigger should work
    status, _ = send_trigger(host, "new_trigger")
    if status != 200:
        raise TestFailure(f"New trigger returned {status}")
    if not wait_for_relay(host, 0, True, timeout=10):
        raise TestFailure("Relay did not close with new trigger")

    print("    PASS")


def test_08_same_action_different_triggers(host, verbose):
    """Two different trigger types (HTTP + temperature) both close relay 0."""
    print("  [08] Different triggers -> same action ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    tasks = [
        make_task(
            trigger=make_http_trigger("same_action"),
            action=make_relay_action(0, True),
        ),
        make_task(
            trigger=make_temp_trigger(comparison=0, value_c10=-100),
            action=make_relay_action(0, True),
        ),
    ]
    set_automation_config(host, tasks)
    time.sleep(1)

    # Fire HTTP trigger -> relay 0 closes
    send_trigger(host, "same_action")
    if not wait_for_relay(host, 0, True, timeout=10):
        raise TestFailure("Relay 0 did not close via HTTP trigger")

    # Open relay, then fire via temperature
    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    push_temperatures(host, next_temp())
    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure("Relay 0 did not close via temperature trigger")

    print("    PASS")


def test_09_rapid_config_replacement(host, verbose):
    """Rapidly replace config 5 times, then verify final config works."""
    print("  [09] Rapid config replacement (5x) ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    # Push 5 different configs rapidly
    for i in range(5):
        set_automation_config(host, [make_task(
            trigger=make_http_trigger(f"rapid_{i}"),
            action=make_relay_action(0, True),
        )])

    time.sleep(1)

    # Only the last config should be active
    for i in range(4):
        status, _ = send_trigger(host, f"rapid_{i}")
        if status != 404:
            raise TestFailure(
                f"Stale trigger rapid_{i} returned {status}, expected 404")

    # Final config should work
    status, _ = send_trigger(host, "rapid_4")
    if status != 200:
        raise TestFailure(f"Final trigger rapid_4 returned {status}")
    if not wait_for_relay(host, 0, True, timeout=10):
        raise TestFailure("Relay did not close with final rapid config")

    print("    PASS")


def test_10_reverse_chain(host, verbose):
    """Reverse chain: HTTP -> close relay 0 -> relay opens -> close relay 1.

    Tests an open-trigger chain: fire HTTP, close relay 0, then immediately
    open relay 0 -- the "relay 0 opens" trigger fires and closes relay 1.
    """
    print("  [10] Reverse chain: HTTP close -> direct open -> relay trigger ...")

    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    time.sleep(0.5)

    tasks = [
        # HTTP trigger closes relay 0
        make_task(
            trigger=make_http_trigger("rev_chain"),
            action=make_relay_action(0, True),
        ),
        # Relay 0 opening closes relay 1
        make_task(
            trigger=make_relay_trigger(0, False),
            action=make_relay_action(1, True),
        ),
    ]
    set_automation_config(host, tasks)
    time.sleep(1)

    # Fire HTTP -> relay 0 closes
    send_trigger(host, "rev_chain")
    if not wait_for_relay(host, 0, True, timeout=10):
        raise TestFailure("Relay 0 did not close via HTTP")

    # Now directly open relay 0 -> should trigger relay 1 close
    set_relay_direct(host, 0, False)
    if not wait_for_relay(host, 1, True, timeout=10):
        raise TestFailure("Relay 1 did not close after relay 0 opened")

    print("    PASS")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

ALL_TESTS = [
    test_01_http_and_temp_coexist,
    test_02_one_trigger_two_actions,
    test_03_chain_reaction,
    test_04_dap_and_http_mixed,
    test_05_max_rules_print,
    test_06_empty_config_no_handlers,
    test_07_replace_config_old_stops,
    test_08_same_action_different_triggers,
    test_09_rapid_config_replacement,
    test_10_reverse_chain,
]


def main():
    parser = argparse.ArgumentParser(
        description="Integration test for combined / mixed automation scenarios.",
        epilog=(
            "Example:\n"
            "  python3 test_automation_combined.py 192.168.0.33\n"
            "  python3 test_automation_combined.py 192.168.0.33 --verbose\n"
            "  python3 test_automation_combined.py 192.168.0.33 --test 05"
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

    print("=== Automation Combined / Mixed Trigger Test ===")
    print(f"Device: {host}\n")

    try:
        state = get_json(host, "energy_manager/state")
        print(f"EM state: relays={state['relays']}")
    except Exception as e:
        print(f"ERROR: Cannot connect to device: {e}")
        sys.exit(1)

    # Check NTP for DAP tests
    try:
        ntp = get_json(host, "ntp/state")
        print(f"NTP synced: {ntp.get('synced', False)}")
    except Exception:
        print("NTP state: unknown")

    print("Configuring temperatures for push mode ...")
    try:
        ensure_temperatures_push_mode(host)
    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(1)

    print("Configuring day_ahead_prices for push mode ...")
    try:
        ensure_dap_push_mode(host)
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
