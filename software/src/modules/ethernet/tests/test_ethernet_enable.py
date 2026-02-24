#!/usr/bin/env python3
"""Integration test for ethernet enable/disable and state transitions.

Tests the save-without-restart behavior for enabling/disabling ethernet and
verifies state transitions. Since the test communicates over ethernet, we
cannot fully disconnect, but we can verify:
  - Disable saves config but interface stays active (state remains Connected)
  - Re-enable after disable works without reboot
  - State transitions on IP config changes (DHCP <-> static)

Usage:
    python3 test_ethernet_enable.py <device-ip>
    python3 test_ethernet_enable.py <device-ip> --verbose

This script needs some very specific requirements to run:
    - Device connected via ethernet
    - Device gets IP <device-ip> via DHCP
    - Network where device is connected to allows to set static IPs in
      the range 192.168.0.200-192.168.0.255
    - Recommended: Have a backup connection via WIFI in a different subnet.
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
# Ethernet helpers
# ---------------------------------------------------------------------------

# EthernetState enum values
NOT_CONFIGURED = 0
NOT_CONNECTED = 1
CONNECTING = 2
CONNECTED = 3

STATE_NAMES = {
    NOT_CONFIGURED: "NotConfigured",
    NOT_CONNECTED: "NotConnected",
    CONNECTING: "Connecting",
    CONNECTED: "Connected",
}


def get_ethernet_config(host):
    return get_json(host, "ethernet/config")


def get_ethernet_state(host):
    return get_json(host, "ethernet/state")


def set_ethernet_config(host, config):
    status, body = put_json(host, "ethernet/config_update", config)
    return status, body


def make_config(enable=True, ip="0.0.0.0", gateway="0.0.0.0",
                subnet="0.0.0.0", dns="0.0.0.0", dns2="0.0.0.0"):
    return {
        "enable_ethernet": enable,
        "ip": ip,
        "gateway": gateway,
        "subnet": subnet,
        "dns": dns,
        "dns2": dns2,
    }


def wait_for_state(host, target_state, timeout=15, poll_interval=0.5):
    """Poll until ethernet reaches target connection_state or timeout."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            state = get_ethernet_state(host)
            actual = state["connection_state"]
            if actual == target_state:
                return True
        except Exception:
            pass
        time.sleep(poll_interval)
    return False


def wait_for_state_ip(host, target_state, expected_ip, timeout=15, poll_interval=0.5):
    """Poll until ethernet reaches target state AND reports expected IP."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            state = get_ethernet_state(host)
        except Exception:
            time.sleep(poll_interval)
            continue
        if (state["connection_state"] == target_state
                and state["ip"] == expected_ip):
            return True
        time.sleep(poll_interval)
    return False


def wait_for_host(host, timeout=15):
    """Poll until the device is reachable at *host*."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            get_json(host, "ethernet/state")
            return True
        except Exception:
            time.sleep(0.5)
    return False


def set_ethernet_config_ip_change(src_host, config, dst_host=None):
    """set_ethernet_config wrapper for calls that change the device's IP.

    If the HTTP response is lost due to a connection reset (the device
    accepted the config and started changing IP), we verify success by
    checking that the device appears at *dst_host*.  If *dst_host* is
    None, we only catch the error and return (200, "").
    """
    try:
        status, body = set_ethernet_config(src_host, config)
        return status, body
    except Exception:
        # Connection error: config may have been accepted but the IP
        # change killed the response.
        if dst_host and wait_for_host(dst_host, timeout=10):
            return 200, ""
        elif dst_host is None:
            # No specific host to check; assume success since the
            # connection error is the expected symptom of an IP change.
            return 200, ""
        raise


# ---------------------------------------------------------------------------
# Test helpers
# ---------------------------------------------------------------------------

class TestFailure(Exception):
    pass


# ---------------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------------

def test_01_initial_state_connected(host, verbose, network_info):
    """Device should be connected via ethernet at test start."""
    print("  [01] Initial state is Connected ...")

    state = get_ethernet_state(host)
    conn_state = state["connection_state"]

    if conn_state != CONNECTED:
        raise TestFailure(
            f"Expected state Connected ({CONNECTED}), "
            f"got {STATE_NAMES.get(conn_state, conn_state)} ({conn_state})")

    config = get_ethernet_config(host)
    if not config["enable_ethernet"]:
        raise TestFailure("enable_ethernet is false at test start")

    if verbose:
        print(f"    State: {STATE_NAMES[conn_state]}, IP: {state['ip']}")

    print("    PASS")


def test_02_disable_stays_active(host, verbose, network_info):
    """Disabling ethernet saves config but interface stays active."""
    print("  [02] Disable saves config, interface stays active ...")

    # Disable ethernet
    config = get_ethernet_config(host)
    config["enable_ethernet"] = False
    status, body = set_ethernet_config(host, config)
    if status != 200:
        raise TestFailure(f"Config update failed: HTTP {status}: {body}")

    time.sleep(2)

    # Config should reflect disabled
    readback = get_ethernet_config(host)
    if readback["enable_ethernet"]:
        raise TestFailure("enable_ethernet should be false after disable")

    # But state should still show connected (interface is still active)
    state = get_ethernet_state(host)
    conn_state = state["connection_state"]
    if conn_state == NOT_CONFIGURED:
        raise TestFailure(
            "State went to NotConfigured -- interface was shut down! "
            "It should remain active until reboot.")

    # State should be Connected or at least Connecting (not NotConfigured)
    if conn_state not in (CONNECTED, CONNECTING):
        raise TestFailure(
            f"Expected Connected or Connecting after disable, "
            f"got {STATE_NAMES.get(conn_state, conn_state)}")

    if verbose:
        print(f"    Config: enable_ethernet={readback['enable_ethernet']}")
        print(f"    State: {STATE_NAMES[conn_state]} (interface still active)")

    print("    PASS")


def test_03_re_enable_after_disable(host, verbose, network_info):
    """Re-enabling ethernet after disable works without reboot."""
    print("  [03] Re-enable after disable ...")

    # Re-enable (device may already be running, this is the enable-while-active path)
    config = get_ethernet_config(host)
    config["enable_ethernet"] = True
    status, body = set_ethernet_config(host, config)
    if status != 200:
        raise TestFailure(f"Config update failed: HTTP {status}: {body}")

    time.sleep(2)

    # Verify config
    readback = get_ethernet_config(host)
    if not readback["enable_ethernet"]:
        raise TestFailure("enable_ethernet should be true after re-enable")

    # Verify still connected
    state = get_ethernet_state(host)
    conn_state = state["connection_state"]
    if conn_state not in (CONNECTED, CONNECTING):
        raise TestFailure(
            f"Expected Connected/Connecting after re-enable, "
            f"got {STATE_NAMES.get(conn_state, conn_state)}")

    if verbose:
        print(f"    State: {STATE_NAMES[conn_state]}, IP: {state['ip']}")

    print("    PASS")


def test_04_disable_enable_cycle(host, verbose, network_info):
    """Rapid disable/enable cycle should not crash."""
    print("  [04] Disable/enable cycle ...")

    config = get_ethernet_config(host)

    # Disable
    config["enable_ethernet"] = False
    status, body = set_ethernet_config(host, config)
    if status != 200:
        raise TestFailure(f"Disable failed: HTTP {status}: {body}")
    time.sleep(0.5)

    # Enable
    config["enable_ethernet"] = True
    status, body = set_ethernet_config(host, config)
    if status != 200:
        raise TestFailure(f"Enable failed: HTTP {status}: {body}")
    time.sleep(2)

    # Should still be reachable and connected
    state = get_ethernet_state(host)
    conn_state = state["connection_state"]
    if conn_state not in (CONNECTED, CONNECTING):
        raise TestFailure(
            f"Expected Connected/Connecting after cycle, "
            f"got {STATE_NAMES.get(conn_state, conn_state)}")

    # Wait for state to fully settle before the next test.
    if conn_state != CONNECTED:
        wait_for_state(host, CONNECTED)

    if verbose:
        print(f"    State: {STATE_NAMES[conn_state]}")

    print("    PASS")


def test_05_switch_to_static_ip(host, verbose, network_info):
    """Switch from DHCP to static IP while connected."""
    print("  [05] Switch to static IP ...")

    static_ip = "192.168.0.200"
    config = make_config(
        enable=True,
        ip=static_ip,
        gateway=network_info["gateway"],
        subnet=network_info["subnet"],
        dns="8.8.8.8",
    )
    status, body = set_ethernet_config_ip_change(host, config, static_ip)
    if status != 200:
        raise TestFailure(f"Config update failed: HTTP {status}: {body}")

    # Wait for the device to acquire the new static IP
    if not wait_for_state_ip(static_ip, CONNECTED, static_ip, timeout=15):
        # Try the original host as fallback to report state
        try:
            state = get_ethernet_state(host)
            raise TestFailure(
                f"Device did not become Connected with IP {static_ip}. "
                f"State on {host}: {STATE_NAMES.get(state['connection_state'], '?')}, "
                f"IP: {state['ip']}")
        except Exception:
            raise TestFailure(
                f"Device unreachable on both {static_ip} and {host} after "
                f"static IP config")

    state = get_ethernet_state(static_ip)
    if verbose:
        print(f"    State: {STATE_NAMES[state['connection_state']]}, IP: {state['ip']}")

    print("    PASS")


def test_06_switch_back_to_dhcp(host, verbose, network_info):
    """Switch from static IP back to DHCP."""
    print("  [06] Switch back to DHCP ...")

    # We might be on static IP from test_05, try both addresses
    static_ip = "192.168.0.200"
    current_host = static_ip

    try:
        get_ethernet_state(static_ip)
    except Exception:
        current_host = host

    dhcp_config = make_config(enable=True)
    status, body = set_ethernet_config_ip_change(current_host, dhcp_config, host)
    if status != 200:
        raise TestFailure(f"Config update failed: HTTP {status}: {body}")

    # Wait for DHCP to assign an IP (device should come back on original host)
    if not wait_for_state(host, CONNECTED, timeout=20):
        raise TestFailure(
            f"Device did not become Connected via DHCP at {host}")

    # Allow web server to stabilize after IP change.
    time.sleep(1)

    state = get_ethernet_state(host)
    if verbose:
        print(f"    State: {STATE_NAMES[state['connection_state']]}, "
              f"IP: {state['ip']}")

    print("    PASS")


def test_07_static_to_different_static(host, verbose, network_info):
    """Switch from one static IP to another while connected."""
    print("  [07] Static to different static IP ...")

    first_ip = "192.168.0.201"
    second_ip = "192.168.0.202"

    # Set first static IP
    config = make_config(
        enable=True,
        ip=first_ip,
        gateway=network_info["gateway"],
        subnet=network_info["subnet"],
    )
    status, body = set_ethernet_config_ip_change(host, config, first_ip)
    if status != 200:
        raise TestFailure(f"First static config failed: HTTP {status}: {body}")

    if not wait_for_state_ip(first_ip, CONNECTED, first_ip, timeout=15):
        raise TestFailure(f"Device did not acquire first IP {first_ip}")

    # Allow web server to stabilize before the next PUT.
    time.sleep(1)

    # Now switch to second static IP
    config["ip"] = second_ip
    status, body = set_ethernet_config_ip_change(first_ip, config, second_ip)
    if status != 200:
        raise TestFailure(f"Second static config failed: HTTP {status}: {body}")

    if not wait_for_state_ip(second_ip, CONNECTED, second_ip, timeout=15):
        raise TestFailure(f"Device did not acquire second IP {second_ip}")

    state = get_ethernet_state(second_ip)
    if verbose:
        print(f"    State: {STATE_NAMES[state['connection_state']]}, "
              f"IP: {state['ip']}")

    # Restore to DHCP
    dhcp_config = make_config(enable=True)
    set_ethernet_config_ip_change(second_ip, dhcp_config, host)

    if not wait_for_state(host, CONNECTED, timeout=20):
        raise TestFailure(f"Device did not return to DHCP at {host}")

    # Allow web server to stabilize after IP change.
    time.sleep(1)

    print("    PASS")


def test_08_dns_change_while_connected(host, verbose, network_info):
    """Change DNS settings while connected (should not disrupt connection)."""
    print("  [08] DNS change while connected ...")

    state_before = get_ethernet_state(host)
    if state_before["connection_state"] != CONNECTED:
        raise TestFailure("Device not connected at test start")

    # Set DNS to specific values
    config = get_ethernet_config(host)
    config["dns"] = "8.8.8.8"
    config["dns2"] = "8.8.4.4"
    status, body = set_ethernet_config(host, config)
    if status != 200:
        raise TestFailure(f"DNS config update failed: HTTP {status}: {body}")

    time.sleep(2)

    # Should still be connected
    state_after = get_ethernet_state(host)
    if state_after["connection_state"] != CONNECTED:
        raise TestFailure(
            f"Connection dropped after DNS change: "
            f"{STATE_NAMES.get(state_after['connection_state'], '?')}")

    # Verify config persisted
    readback = get_ethernet_config(host)
    if readback["dns"] != "8.8.8.8":
        raise TestFailure(f"DNS not persisted: {readback['dns']}")
    if readback["dns2"] != "8.8.4.4":
        raise TestFailure(f"DNS2 not persisted: {readback['dns2']}")

    if verbose:
        print(f"    State: {STATE_NAMES[state_after['connection_state']]}")
        print(f"    DNS: {readback['dns']}, DNS2: {readback['dns2']}")

    print("    PASS")


def test_09_rapid_ip_changes(host, verbose, network_info):
    """Rapidly change IP addresses without crash."""
    print("  [09] Rapid IP changes ...")

    accepted = 0
    for i in range(5):
        ip_last = 210 + i
        config = make_config(
            enable=True,
            ip=f"192.168.0.{ip_last}",
            gateway=network_info["gateway"],
            subnet=network_info["subnet"],
        )
        try:
            status, body = set_ethernet_config(host, config)
            if status == 200:
                accepted += 1
            elif verbose:
                print(f"    Config {i} rejected: HTTP {status}")
        except Exception as e:
            # Connection error expected once device has moved IP.
            # A reset on the very first PUT typically means the device
            # accepted the config and the IP change killed the response.
            if verbose:
                print(f"    Config {i} connection error (expected): {e}")
            if i == 0:
                # Treat first connection reset as accepted -- the device
                # likely started changing IP.
                accepted += 1
            break
        time.sleep(0.3)

    if accepted == 0:
        raise TestFailure("No configs were accepted at all")

    if verbose:
        print(f"    {accepted} of 5 configs accepted before device moved")

    # The device should settle on one of the IPs we sent.
    # Try all of them in reverse order, then the original host.
    settled_ip = None
    state = None
    for i in reversed(range(5)):
        try_ip = f"192.168.0.{210 + i}"
        try:
            state = get_ethernet_state(try_ip)
            settled_ip = try_ip
            break
        except Exception:
            continue

    if settled_ip is None:
        try:
            state = get_ethernet_state(host)
            settled_ip = host
        except Exception:
            pass

    if settled_ip is None:
        raise TestFailure("Device unreachable on any expected IP after rapid changes")

    assert state is not None  # settled_ip is set only when state was fetched
    if verbose:
        print(f"    Device settled at {settled_ip}")
        print(f"    State: {STATE_NAMES.get(state['connection_state'], '?')}, "
              f"IP: {state['ip']}")

    # Restore to DHCP
    try:
        set_ethernet_config(settled_ip, make_config(enable=True))
    except Exception:
        try:
            set_ethernet_config(host, make_config(enable=True))
        except Exception:
            pass

    time.sleep(3)
    wait_for_state(host, CONNECTED, timeout=20)

    print("    PASS")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

ALL_TESTS = [
    test_01_initial_state_connected,
    test_02_disable_stays_active,
    test_03_re_enable_after_disable,
    test_04_disable_enable_cycle,
    test_05_switch_to_static_ip,
    test_06_switch_back_to_dhcp,
    test_07_static_to_different_static,
    test_08_dns_change_while_connected,
    test_09_rapid_ip_changes,
]


def main():
    parser = argparse.ArgumentParser(
        description="Integration test for ethernet enable/disable and state transitions.",
        epilog=(
            "Example:\n"
            "  python3 test_ethernet_enable.py 192.168.0.47\n"
            "  python3 test_ethernet_enable.py 192.168.0.47 --verbose\n"
            "  python3 test_ethernet_enable.py 192.168.0.47 --test 05"
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
    print("=== Ethernet Enable/State Test ===")
    print(f"Device: {host}\n")

    try:
        state = get_ethernet_state(host)
        print(f"Ethernet state: connection_state={state['connection_state']}, "
              f"ip={state['ip']}, mac={state['mac']}")
    except Exception as e:
        print(f"ERROR: Cannot connect to device: {e}")
        sys.exit(1)

    # Save original config to restore later
    original_config = get_ethernet_config(host)
    if verbose:
        print(f"Original config: {json.dumps(original_config, indent=2)}")

    # Discover network parameters from DHCP state so that static IP tests
    # use the correct subnet mask and gateway for the test network.
    network_info = {
        "subnet": state.get("subnet", "255.255.255.0"),
        "gateway": original_config.get("gateway", "192.168.0.1"),
    }
    # If config gateway is 0.0.0.0 (DHCP), use a sensible default.
    if network_info["gateway"] == "0.0.0.0":
        network_info["gateway"] = "192.168.0.1"
    print(f"Network: subnet={network_info['subnet']}, "
          f"gateway={network_info['gateway']}")

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
            test_fn(host, verbose, network_info)
            passed += 1
        except TestFailure as e:
            failed += 1
            errors.append((test_fn.__name__, str(e)))
            print(f"    FAIL: {e}")
        except Exception as e:
            failed += 1
            errors.append((test_fn.__name__, f"Unexpected error: {e}"))
            print(f"    ERROR: {e}")

    # Cleanup: restore original config
    print("\nCleaning up ...")

    # Try to reach device on any known IP
    cleanup_hosts = ([host, "192.168.0.200", "192.168.0.201", "192.168.0.202"]
                     + [f"192.168.0.{210+i}" for i in range(5)])
    cleaned = False
    for cleanup_host in cleanup_hosts:
        try:
            status, body = set_ethernet_config(cleanup_host, original_config)
            if status == 200:
                cleaned = True
                if cleanup_host != host:
                    print(f"  Restored config via {cleanup_host}")
                break
        except Exception:
            continue

    if not cleaned:
        print("  WARNING: Failed to restore original config on any known IP")

    # Wait for device to come back on original host and fully settle.
    wait_for_state(host, CONNECTED, timeout=15)
    try:
        get_ethernet_state(host)
    except Exception:
        print(f"  WARNING: Device not reachable on {host} after cleanup")

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
