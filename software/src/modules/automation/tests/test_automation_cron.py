#!/usr/bin/env python3
"""Integration test for cron-triggered automation rules.

Tests that cron triggers fire at the correct device-local minute and that
non-matching cron triggers do not fire.  Uses the device's own NTP clock
(via /ntp/state) and timezone (via /ntp/config) to compute target minutes
in local time, because the firmware's cron scheduler uses localtime_r().

Usage:
    python3 test_automation_cron.py <device-ip>
    python3 test_automation_cron.py <device-ip> --verbose

Prerequisites on the device:
    - Energy Manager v2 with at least one relay
    - NTP must be synced (cron triggers require RTC sync)

Runtime: approximately 1-2 minutes (waits for the next minute boundary).
"""

import argparse
import json
import sys
import time
import urllib.request
import urllib.error
from datetime import datetime, timezone
from zoneinfo import ZoneInfo


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
    """Read relay state from energy_manager/state."""
    state = get_json(host, "energy_manager/state")
    return state["relays"][index]


def set_relay_direct(host, index, closed):
    """Directly set a relay via energy_manager/outputs_update."""
    payload = [255, 255, 255, 255]
    payload[2 + index] = 1 if closed else 0
    status, body = put_json(host, "energy_manager/outputs_update", payload)
    if status != 200:
        raise RuntimeError(f"outputs_update failed: HTTP {status}: {body}")


def wait_for_relay(host, index, expected, timeout=15, poll_interval=0.5):
    """Poll until relay reaches expected state or timeout."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        if get_relay_state(host, index) == expected:
            return True
        time.sleep(poll_interval)
    return False


def set_automation_config(host, tasks):
    """PUT /automation/config_update with the given task list."""
    status, body = put_json(host, "automation/config_update",
                            {"tasks": tasks})
    if status != 200:
        raise RuntimeError(
            f"automation/config_update failed: HTTP {status}: {body}")


def get_automation_config(host):
    """GET /automation/config."""
    return get_json(host, "automation/config")


# ---------------------------------------------------------------------------
# Device clock helpers
# ---------------------------------------------------------------------------

def get_device_time(host):
    """Return the device's current local time as (hour, minute, epoch_mins) tuple.

    Uses /ntp/state (``time`` in minutes since epoch) and /ntp/config
    (``timezone`` as IANA zone name) to compute the device's local time.
    The cron trigger uses localtime_r() which converts to local time, so
    we must match that.
    """
    ntp = get_json(host, "ntp/state")
    if not ntp.get("synced", False):
        raise RuntimeError("NTP is not synced on the device; cron tests "
                           "require a synced clock")
    minutes_since_epoch = ntp["time"]

    # Get the device timezone from NTP config
    ntp_config = get_json(host, "ntp/config")
    tz_name = ntp_config.get("timezone", "UTC")
    try:
        tz = ZoneInfo(tz_name)
    except KeyError:
        raise RuntimeError(f"Unknown timezone '{tz_name}' on device")

    # Convert UTC epoch minutes to local time
    utc_seconds = minutes_since_epoch * 60
    dt_utc = datetime.fromtimestamp(utc_seconds, tz=timezone.utc)
    dt_local = dt_utc.astimezone(tz)

    return dt_local.hour, dt_local.minute, minutes_since_epoch


# ---------------------------------------------------------------------------
# Rule builders
# ---------------------------------------------------------------------------

TRIGGER_CRON = 1
ACTION_EM_RELAY_SWITCH = 13


def make_cron_trigger(minute=-1, hour=-1, mday=-1, wday=-1):
    """Build a cron trigger union.

    -1 = wildcard (match any).
    """
    return [TRIGGER_CRON, {
        "mday": mday,
        "wday": wday,
        "hour": hour,
        "minute": minute,
    }]


def make_relay_action(index, closed):
    """Build an EM relay switch action union."""
    return [ACTION_EM_RELAY_SWITCH, {
        "index": index,
        "closed": closed,
    }]


def make_task(trigger, action, delay=0):
    """Build a single automation task."""
    return {"trigger": trigger, "action": action, "delay": delay}


# ---------------------------------------------------------------------------
# Test helpers
# ---------------------------------------------------------------------------

class TestFailure(Exception):
    pass


# ---------------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------------

def test_01_cron_fires_at_next_minute(host, verbose):
    """Cron trigger at the upcoming minute closes relay 0.

    Reads the device clock, computes the next minute boundary, sets up a
    cron rule for that exact minute/hour, then waits for the relay to close.
    """
    print("  [01] Cron trigger fires at next device minute ...")

    # Ensure relay starts open
    set_relay_direct(host, 0, False)
    time.sleep(0.5)
    assert not get_relay_state(host, 0), "Relay 0 should start open"

    # Read device clock and compute the target minute (in local time)
    hour, minute, epoch_mins = get_device_time(host)
    # Target: 2 minutes from now in local time.  We use +2 because the
    # current reported minute may be almost over, so +1 could already be
    # passing; +2 gives a full minute of headroom.
    # Compute target local time by adding 2 minutes.
    ntp_config = get_json(host, "ntp/config")
    tz = ZoneInfo(ntp_config.get("timezone", "UTC"))
    target_utc_secs = (epoch_mins + 2) * 60
    dt_target = datetime.fromtimestamp(target_utc_secs, tz=timezone.utc).astimezone(tz)
    target_hour = dt_target.hour
    target_minute = dt_target.minute

    target_epoch_mins = epoch_mins + 2

    if verbose:
        print(f"    Device local time: {hour:02d}:{minute:02d} "
              f"(epoch_mins={epoch_mins})")
        print(f"    Target local time: {target_hour:02d}:{target_minute:02d} "
              f"(epoch_mins={target_epoch_mins})")

    # Create cron rule: at target_hour:target_minute -> close relay 0
    task = make_task(
        trigger=make_cron_trigger(minute=target_minute, hour=target_hour),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    if verbose:
        print(f"    Config: {json.dumps(get_automation_config(host), indent=2)}")

    # Wait for the cron to fire.  The target is ~2 minutes from now.
    # We poll for up to 150 seconds (2.5 min) to account for any timing skew.
    print(f"    Waiting for cron at device local time "
          f"{target_hour:02d}:{target_minute:02d} ...")

    if not wait_for_relay(host, 0, True, timeout=150, poll_interval=1.0):
        # Re-read device time for diagnostics
        h2, m2, em2 = get_device_time(host)
        raise TestFailure(
            f"Relay 0 did not close. Device local time now: {h2:02d}:{m2:02d} "
            f"(epoch_mins={em2}), target was "
            f"{target_hour:02d}:{target_minute:02d} "
            f"(epoch_mins={target_epoch_mins})")

    print("    PASS")


def test_02_cron_wrong_minute_no_fire(host, verbose):
    """Cron trigger at a non-matching minute does NOT fire.

    Sets up a cron rule for a minute far from the current device time
    and verifies the relay stays open after waiting 30 seconds.
    """
    print("  [02] Cron at wrong minute does not fire ...")

    # Ensure relay starts open
    set_relay_direct(host, 0, False)
    time.sleep(0.5)
    assert not get_relay_state(host, 0), "Relay 0 should start open"

    # Read device clock
    hour, minute, epoch_mins = get_device_time(host)

    # Pick a minute that is 30 minutes away from now (won't fire within
    # our 30-second observation window)
    wrong_minute = (minute + 30) % 60
    # If the wrong minute happens to land on the current hour boundary
    # in an unfortunate way, also shift the hour to be safe
    wrong_hour = (hour + 12) % 24  # 12 hours away

    if verbose:
        print(f"    Device local time: {hour:02d}:{minute:02d}")
        print(f"    Wrong target: {wrong_hour:02d}:{wrong_minute:02d}")

    task = make_task(
        trigger=make_cron_trigger(minute=wrong_minute, hour=wrong_hour),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(host, [task])
    time.sleep(1)

    # Wait 30 seconds -- relay should NOT close
    print("    Waiting 30s to confirm relay stays open ...")
    time.sleep(30)

    if get_relay_state(host, 0):
        raise TestFailure(
            f"Relay 0 closed but cron was set for "
            f"{wrong_hour:02d}:{wrong_minute:02d}, device time is "
            f"{hour:02d}:{minute:02d}")

    print("    PASS")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

ALL_TESTS = [
    test_01_cron_fires_at_next_minute,
    test_02_cron_wrong_minute_no_fire,
]


def main():
    parser = argparse.ArgumentParser(
        description="Integration test for cron-triggered automation rules.",
        epilog=(
            "Example:\n"
            "  python3 test_automation_cron.py 192.168.0.33\n"
            "  python3 test_automation_cron.py 192.168.0.33 --verbose\n"
            "  python3 test_automation_cron.py 192.168.0.33 --test 01"
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

    print("=== Automation Cron Trigger Test ===")
    print(f"Device: {host}\n")

    # Verify connectivity
    try:
        state = get_json(host, "energy_manager/state")
        print(f"EM state: relays={state['relays']}")
    except Exception as e:
        print(f"ERROR: Cannot connect to device: {e}")
        sys.exit(1)

    # Verify NTP is synced (required for cron)
    try:
        hour, minute, epoch_mins = get_device_time(host)
        print(f"Device local time: {hour:02d}:{minute:02d} "
              f"(epoch_mins={epoch_mins})")
    except RuntimeError as e:
        print(f"ERROR: {e}")
        sys.exit(1)

    # Save original automation config
    original_config = get_automation_config(host)
    if verbose:
        print(f"Original config: {json.dumps(original_config, indent=2)}")

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

    # Cleanup: restore config and open relay
    print("\nCleaning up ...")
    try:
        set_automation_config(host, original_config.get("tasks", []))
    except Exception as e:
        print(f"  WARNING: Failed to restore automation config: {e}")
    try:
        set_relay_direct(host, 0, False)
    except Exception as e:
        print(f"  WARNING: Failed to reset relay: {e}")

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
