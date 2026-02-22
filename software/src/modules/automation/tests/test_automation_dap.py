#!/usr/bin/env python3
"""Integration test for automation with day-ahead price triggers.

Tests that day-ahead price triggers fire automation actions correctly.
Pushes price data via the HTTP API, and verifies that the trigger fires
when the current price crosses the configured threshold.

Usage:
    python3 test_automation_dap.py <device-ip>
    python3 test_automation_dap.py <device-ip> --verbose

Prerequisites on the device:
    - Energy Manager v2 with relays
    - NTP must be synced (needed to determine the "current" price slot)
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
        if get_relay_state(host, index) == expected:
            return True
        time.sleep(poll_interval)
    return False


# ---------------------------------------------------------------------------
# DAP helpers
# ---------------------------------------------------------------------------

def ensure_dap_push_mode(host):
    """Enable day_ahead_prices in Push mode."""
    config = get_json(host, "day_ahead_prices/config")
    needs_update = (
        not config.get("enable", False) or
        config.get("source", 0) != 1  # Push = 1
    )
    if needs_update:
        config["enable"] = True
        config["source"] = 1
        status, body = put_json(host, "day_ahead_prices/config_update", config)
        if status != 200:
            raise RuntimeError(
                f"Failed to enable DAP push mode: HTTP {status}: {body}")
        time.sleep(1)
    return config


def push_prices(host, prices, resolution=60):
    """Push price data.  Prices are in ct/1000 per kWh.

    Aligns first_date to UTC midnight today.
    resolution: 15 or 60 minutes.
    """
    now = int(time.time())
    midnight = now - now % 86400
    first_date_minutes = midnight // 60
    resolution_enum = 0 if resolution == 15 else 1

    payload = {
        "first_date": first_date_minutes,
        "resolution": resolution_enum,
        "prices": prices,
    }
    status, body = put_json(host, "day_ahead_prices/prices_update", payload)
    if status != 200:
        raise RuntimeError(f"prices_update failed: HTTP {status}: {body}")


def get_dap_state(host):
    return get_json(host, "day_ahead_prices/state")


def make_hourly_prices(count, default_price):
    """Create a list of hourly prices all set to default_price."""
    return [default_price] * count


def set_current_hour_price(prices, price_value, resolution=60):
    """Set the price for the current hour slot.

    Returns the modified prices list and the index that was set.
    """
    now = int(time.time())
    midnight = now - now % 86400
    seconds_since_midnight = now - midnight
    if resolution == 60:
        index = seconds_since_midnight // 3600
    else:
        index = seconds_since_midnight // 900
    if index < len(prices):
        prices[index] = price_value
    return prices, index


# ---------------------------------------------------------------------------
# Automation rule builders
# ---------------------------------------------------------------------------

TRIGGER_DAP_NOW = 20
ACTION_EM_RELAY_SWITCH = 13


def make_dap_trigger_absolute(comparison, value_ct):
    """Build a DAP trigger with absolute threshold.

    comparison: 0 = greater, 1 = less
    value_ct:   threshold in cents (e.g. 5 = 5 ct/kWh)
    """
    return [TRIGGER_DAP_NOW, {
        "type": 1,       # absolute
        "comparison": comparison,
        "value": value_ct,
    }]


def make_dap_trigger_average(comparison, value_pct):
    """Build a DAP trigger relative to today's average.

    comparison: 0 = greater, 1 = less
    value_pct:  percentage of average (e.g. 120 = 120%)
    """
    return [TRIGGER_DAP_NOW, {
        "type": 0,       # relative to average
        "comparison": comparison,
        "value": value_pct,
    }]


def make_relay_action(index, closed):
    return [ACTION_EM_RELAY_SWITCH, {
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


# We need unique prices each push to trigger change detection.
_price_offset = [0]


def next_price_offset():
    """Return a small incrementing offset to ensure price values change."""
    _price_offset[0] += 1
    return _price_offset[0]


# ---------------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------------

def test_01_price_gt_absolute_closes_relay(host, verbose):
    """DAP price > 5 ct/kWh (absolute) -> close relay 0.

    Push prices where the current slot is 8 ct/kWh = 8000 ct/1000.
    Threshold is 5 ct = 5*1000 = 5000 ct/1000.
    8000 > 5000, so trigger fires.
    """
    print("  [01] DAP price > 5ct (absolute) -> close relay 0 ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_dap_trigger_absolute(comparison=0, value_ct=5),  # > 5 ct
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Push 48h of prices at 3 ct, but current hour at 8 ct
    offset = next_price_offset()
    prices = make_hourly_prices(48, 3000 + offset)
    prices, idx = set_current_hour_price(prices, 8000 + offset)
    if verbose:
        print(f"    Current hour index: {idx}, price: {prices[idx]} ct/1000")

    push_prices(host, prices)
    time.sleep(2)

    if verbose:
        dap_state = get_dap_state(host)
        print(f"    DAP state: current_price={dap_state.get('current_price')}")

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close. State: {get_relay_state(host, 0)}")

    print("    PASS")


def test_02_price_lt_absolute_closes_relay(host, verbose):
    """DAP price < 10 ct/kWh (absolute) -> close relay 0.

    Push current slot at 3 ct/kWh. Threshold is 10 ct.
    3000 < 10000, so trigger fires.
    """
    print("  [02] DAP price < 10ct (absolute) -> close relay 0 ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_dap_trigger_absolute(comparison=1, value_ct=10),  # < 10 ct
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    offset = next_price_offset()
    prices = make_hourly_prices(48, 5000 + offset)
    prices, idx = set_current_hour_price(prices, 3000 + offset)
    if verbose:
        print(f"    Current hour index: {idx}, price: {prices[idx]} ct/1000")

    push_prices(host, prices)
    time.sleep(2)

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close. State: {get_relay_state(host, 0)}")

    print("    PASS")


def test_03_price_condition_not_met(host, verbose):
    """DAP price > 15 ct (absolute) -> close relay 0. But price is only 5 ct.

    5000 is NOT > 15000, so trigger should not fire.
    """
    print("  [03] DAP condition not met: relay stays open ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_dap_trigger_absolute(comparison=0, value_ct=15),  # > 15 ct
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    offset = next_price_offset()
    prices = make_hourly_prices(48, 5000 + offset)
    prices, idx = set_current_hour_price(prices, 5000 + offset)
    if verbose:
        print(f"    Current hour index: {idx}, price: {prices[idx]} ct/1000")

    push_prices(host, prices)
    time.sleep(3)

    if get_relay_state(host, 0):
        raise TestFailure("Relay 0 closed but price was below threshold")

    print("    PASS")


def test_04_modify_dap_rule(host, verbose):
    """Modify DAP rule at runtime.

    First: price > 5ct -> close relay.
    Then change to: price > 5ct -> open relay.
    """
    print("  [04] Modify DAP rule at runtime ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    # First rule: close relay
    task = make_task(
        trigger=make_dap_trigger_absolute(comparison=0, value_ct=5),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    offset = next_price_offset()
    prices = make_hourly_prices(48, 3000 + offset)
    prices, idx = set_current_hour_price(prices, 8000 + offset)
    push_prices(host, prices)
    time.sleep(2)

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure("Relay 0 did not close with first rule")

    # Modify rule: open relay
    task = make_task(
        trigger=make_dap_trigger_absolute(comparison=0, value_ct=5),
        action=make_relay_action(index=0, closed=False),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Push different price to re-trigger
    offset = next_price_offset()
    prices = make_hourly_prices(48, 3000 + offset)
    prices, idx = set_current_hour_price(prices, 9000 + offset)
    push_prices(host, prices)
    time.sleep(2)

    if not wait_for_relay(host, 0, False, timeout=15):
        raise TestFailure(
            f"Relay 0 did not open after rule modification. "
            f"State: {get_relay_state(host, 0)}")

    print("    PASS")


def test_05_price_avg_trigger(host, verbose):
    """DAP price > 120% of average -> close relay 0.

    Push 48h prices: most slots at 5 ct, current slot at 8 ct.
    Average will be ~5 ct. 120% of 5 ct = 6 ct. Current 8 ct > 6 ct.
    """
    print("  [05] DAP price > 120% avg -> close relay 0 ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_dap_trigger_average(comparison=0, value_pct=120),  # > 120% avg
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    offset = next_price_offset()
    prices = make_hourly_prices(48, 5000 + offset)
    # Set current hour to 8 ct (significantly above average of ~5 ct)
    prices, idx = set_current_hour_price(prices, 8000 + offset)
    if verbose:
        avg = sum(prices[:24]) / 24
        print(f"    Today avg: {avg:.0f} ct/1000, current: {prices[idx]} ct/1000")
        print(f"    120% of avg: {avg * 1.2:.0f} ct/1000")

    push_prices(host, prices)
    time.sleep(2)

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close. State: {get_relay_state(host, 0)}")

    print("    PASS")


def test_06_price_below_avg_no_fire(host, verbose):
    """DAP price > 120% avg but price is actually below average.

    All prices at 5 ct. Current price IS the average, so NOT > 120%.
    """
    print("  [06] DAP price at avg, trigger > 120% avg: no fire ...")

    set_relay_direct(host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_dap_trigger_average(comparison=0, value_pct=120),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # All prices identical => current = average, NOT > 120% of average
    offset = next_price_offset()
    prices = make_hourly_prices(48, 5000 + offset)
    push_prices(host, prices)
    time.sleep(3)

    if get_relay_state(host, 0):
        raise TestFailure("Relay 0 closed but price was not above 120% of average")

    print("    PASS")


def test_07_multiple_dap_rules(host, verbose):
    """Two DAP rules: one for each relay.

    Rule 1: price > 3ct -> close relay 0
    Rule 2: price > 3ct -> close relay 1
    """
    print("  [07] Multiple DAP rules: both relays close ...")

    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    time.sleep(0.5)

    tasks = [
        make_task(
            trigger=make_dap_trigger_absolute(comparison=0, value_ct=3),
            action=make_relay_action(index=0, closed=True),
        ),
        make_task(
            trigger=make_dap_trigger_absolute(comparison=0, value_ct=3),
            action=make_relay_action(index=1, closed=True),
        ),
    ]
    set_automation_config(host, tasks)
    time.sleep(1)

    offset = next_price_offset()
    prices = make_hourly_prices(48, 2000 + offset)
    prices, idx = set_current_hour_price(prices, 7000 + offset)
    push_prices(host, prices)
    time.sleep(2)

    if not wait_for_relay(host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close. State: {get_relay_state(host, 0)}")
    if not wait_for_relay(host, 1, True, timeout=15):
        raise TestFailure(
            f"Relay 1 did not close. State: {get_relay_state(host, 1)}")

    print("    PASS")


def test_08_delete_dap_rules(host, verbose):
    """Delete DAP rules: config becomes empty."""
    print("  [08] Delete DAP rules: config becomes empty ...")

    set_automation_config(host, [])
    time.sleep(1)

    config = get_automation_config(host)
    if len(config.get("tasks", [None])) != 0:
        raise TestFailure(
            f"Expected empty tasks, got: {json.dumps(config)}")

    set_relay_direct(host, 0, False)
    set_relay_direct(host, 1, False)
    time.sleep(1)

    if get_relay_state(host, 0) or get_relay_state(host, 1):
        raise TestFailure("Relays should be open after clearing rules")

    print("    PASS")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

ALL_TESTS = [
    test_01_price_gt_absolute_closes_relay,
    test_02_price_lt_absolute_closes_relay,
    test_03_price_condition_not_met,
    test_04_modify_dap_rule,
    test_05_price_avg_trigger,
    test_06_price_below_avg_no_fire,
    test_07_multiple_dap_rules,
    test_08_delete_dap_rules,
]


def main():
    parser = argparse.ArgumentParser(
        description="Integration test for automation day-ahead price triggers.",
        epilog=(
            "Example:\n"
            "  python3 test_automation_dap.py 192.168.0.33\n"
            "  python3 test_automation_dap.py 192.168.0.33 --verbose\n"
            "  python3 test_automation_dap.py 192.168.0.33 --test 05"
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

    print(f"=== Automation Day-Ahead Price Trigger Test ===")
    print(f"Device: {host}\n")

    try:
        state = get_json(host, "energy_manager/state")
        print(f"EM state: relays={state['relays']}")
    except Exception as e:
        print(f"ERROR: Cannot connect to device: {e}")
        sys.exit(1)

    # Check NTP sync
    try:
        ntp_state = get_json(host, "ntp/state")
        synced = ntp_state.get("synced", False)
        print(f"NTP synced: {synced}")
        if not synced:
            print("WARNING: NTP not synced. DAP current price depends on accurate time.")
    except Exception:
        print("WARNING: Could not check NTP state.")

    # Enable DAP push mode
    print("Configuring day_ahead_prices for push mode ...")
    try:
        ensure_dap_push_mode(host)
    except Exception as e:
        print(f"ERROR: {e}")
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
