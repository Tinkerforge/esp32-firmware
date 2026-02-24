#!/usr/bin/env python3
"""Integration test for ethernet config validation.

Tests that the ethernet config validator correctly rejects invalid configs
(bad IP format, invalid subnet masks, gateway not in subnet, localhost
routing) and accepts valid configs.

Usage:
    python3 test_ethernet_config.py <device-ip>
    python3 test_ethernet_config.py <device-ip> --verbose

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

def get_ethernet_config(host):
    return get_json(host, "ethernet/config")


def get_ethernet_state(host):
    return get_json(host, "ethernet/state")


def set_ethernet_config(host, config):
    """PUT /ethernet/config_update with the given config dict."""
    status, body = put_json(host, "ethernet/config_update", config)
    return status, body


def make_config(enable=True, ip="0.0.0.0", gateway="0.0.0.0",
                subnet="0.0.0.0", dns="0.0.0.0", dns2="0.0.0.0"):
    """Build an ethernet config payload."""
    return {
        "enable_ethernet": enable,
        "ip": ip,
        "gateway": gateway,
        "subnet": subnet,
        "dns": dns,
        "dns2": dns2,
    }


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


# Connected state constant (EthernetState::Connected == 3).
CONNECTED = 3


def wait_for_connected(host, timeout=15):
    """Poll until ethernet reaches Connected state."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            state = get_json(host, "ethernet/state")
            if state["connection_state"] == CONNECTED:
                return True
        except Exception:
            pass
        time.sleep(0.5)
    return False


def set_ethernet_config_ip_change(src_host, config, dst_host=None):
    """set_ethernet_config wrapper for calls that change the device's IP.

    If the HTTP response is lost due to a connection reset (the device
    accepted the config and started changing IP), we verify success by
    checking that the device appears at *dst_host*.  If *dst_host* is
    None, we treat the connection error as success.
    """
    try:
        status, body = set_ethernet_config(src_host, config)
        return status, body
    except Exception:
        if dst_host and wait_for_host(dst_host, timeout=10):
            return 200, ""
        elif dst_host is None:
            return 200, ""
        raise


def set_static_and_restore(host, static_ip, original_config, verbose,
                           gateway=None, subnet=None,
                           dns="0.0.0.0", dns2="0.0.0.0",
                           network_info=None):
    """Set a static IP config, verify acceptance, restore original config.

    1. PUT static config -> verify HTTP 200.
    2. Wait for device at *static_ip*.
    3. Restore *original_config* via *static_ip*.
    4. Wait for device at *host* (original DHCP address).

    If *gateway* or *subnet* are None, falls back to values from
    *network_info* (discovered at startup from DHCP state).

    Returns the HTTP status of the initial PUT.
    Raises TestFailure / Exception on unexpected errors.
    """
    if network_info is None:
        network_info = {}
    if gateway is None:
        gateway = network_info.get("gateway", "192.168.0.1")
    if subnet is None:
        subnet = network_info.get("subnet", "255.255.255.0")
    config = make_config(
        enable=True, ip=static_ip, gateway=gateway,
        subnet=subnet, dns=dns, dns2=dns2,
    )
    try:
        status, body = set_ethernet_config(host, config)
    except Exception:
        # Connection error: the config may have been accepted but the
        # response was lost because the IP change reset the connection.
        # Check whether the device appeared at the new static IP.
        if wait_for_host(static_ip, timeout=10):
            status, body = 200, ""
        else:
            raise

    if status != 200:
        return status, body

    # apply_config() runs asynchronously; wait for the device at its new IP.
    if not wait_for_host(static_ip):
        raise Exception(
            f"Device did not appear at {static_ip} after config change")

    if verbose:
        print(f"    Device reachable at {static_ip}")

    # Restore original config via the new IP.
    try:
        rst_status, rst_body = set_ethernet_config(static_ip, original_config)
    except Exception:
        # Connection error during restore: the IP change back to DHCP may
        # have reset the connection.  Check if the device reappears at host.
        if wait_for_host(host, timeout=15):
            rst_status, rst_body = 200, ""
        else:
            raise
    if rst_status != 200:
        raise Exception(
            f"Failed to restore config via {static_ip}: HTTP {rst_status}: {rst_body}")

    # Wait for device back at original host.
    if not wait_for_host(host):
        raise Exception(
            f"Device did not reappear at {host} after config restore")

    # Wait for connection state to fully settle (Connected) before
    # returning, so subsequent tests don't start while still Connecting.
    wait_for_connected(host)

    if verbose:
        print(f"    Restored, device back at {host}")

    return status, body


# ---------------------------------------------------------------------------
# Test helpers
# ---------------------------------------------------------------------------

class TestFailure(Exception):
    pass


# ---------------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------------

def test_01_read_config(host, verbose, original_config, network_info):
    """Verify we can read the ethernet config."""
    print("  [01] Read ethernet config ...")

    config = get_ethernet_config(host)

    required_fields = ["enable_ethernet", "ip", "gateway", "subnet", "dns", "dns2"]
    for field in required_fields:
        if field not in config:
            raise TestFailure(f"Missing field '{field}' in config: {json.dumps(config)}")

    if verbose:
        print(f"    Config: {json.dumps(config, indent=2)}")

    print("    PASS")


def test_02_read_state(host, verbose, original_config, network_info):
    """Verify we can read the ethernet state."""
    print("  [02] Read ethernet state ...")

    state = get_ethernet_state(host)

    required_fields = ["connection_state", "mac", "ip", "subnet",
                       "full_duplex", "link_speed"]
    for field in required_fields:
        if field not in state:
            raise TestFailure(f"Missing field '{field}' in state: {json.dumps(state)}")

    if verbose:
        print(f"    State: {json.dumps(state, indent=2)}")

    print("    PASS")


def test_03_reject_bad_ip_format(host, verbose, original_config, network_info):
    """Config with malformed IP string should be rejected."""
    print("  [03] Reject bad IP format ...")

    config = make_config(ip="not-an-ip", subnet="255.255.255.0")
    status, body = set_ethernet_config(host, config)

    if status == 200:
        raise TestFailure(f"Expected rejection for bad IP, got HTTP 200: {body}")

    if verbose:
        print(f"    Rejected with HTTP {status}: {body}")

    print("    PASS")


def test_04_reject_bad_gateway_format(host, verbose, original_config, network_info):
    """Config with malformed gateway string should be rejected."""
    print("  [04] Reject bad gateway format ...")

    config = make_config(ip="192.168.0.200", gateway="abc.def.ghi.jkl",
                         subnet="255.255.255.0")
    status, body = set_ethernet_config(host, config)

    if status == 200:
        raise TestFailure(f"Expected rejection for bad gateway, got HTTP 200: {body}")

    if verbose:
        print(f"    Rejected with HTTP {status}: {body}")

    print("    PASS")


def test_05_reject_bad_subnet_format(host, verbose, original_config, network_info):
    """Config with malformed subnet string should be rejected."""
    print("  [05] Reject bad subnet format ...")

    config = make_config(ip="192.168.0.200", subnet="bad")
    status, body = set_ethernet_config(host, config)

    if status == 200:
        raise TestFailure(f"Expected rejection for bad subnet, got HTTP 200: {body}")

    if verbose:
        print(f"    Rejected with HTTP {status}: {body}")

    print("    PASS")


def test_06_reject_invalid_subnet_mask(host, verbose, original_config, network_info):
    """Config with non-contiguous subnet mask (e.g. 255.0.255.0) should be rejected."""
    print("  [06] Reject invalid subnet mask ...")

    config = make_config(ip="192.168.0.200", subnet="255.0.255.0")
    status, body = set_ethernet_config(host, config)

    if status == 200:
        raise TestFailure(f"Expected rejection for invalid subnet mask, got HTTP 200: {body}")

    if verbose:
        print(f"    Rejected with HTTP {status}: {body}")

    print("    PASS")


def test_07_reject_gateway_not_in_subnet(host, verbose, original_config, network_info):
    """Config where gateway is outside the subnet should be rejected."""
    print("  [07] Reject gateway not in subnet ...")

    config = make_config(
        ip="192.168.0.200",
        gateway="10.0.0.1",
        subnet="255.255.255.0",
    )
    status, body = set_ethernet_config(host, config)

    if status == 200:
        raise TestFailure(f"Expected rejection for out-of-subnet gateway, got HTTP 200: {body}")

    if verbose:
        print(f"    Rejected with HTTP {status}: {body}")

    print("    PASS")


def test_08_reject_localhost_routing(host, verbose, original_config, network_info):
    """Config that would route 127.0.0.1 through ethernet should be rejected."""
    print("  [08] Reject localhost routing ...")

    # 127.0.0.0/8 subnet mask with an IP in 127.x range
    config = make_config(
        ip="127.0.0.1",
        subnet="255.0.0.0",
    )
    status, body = set_ethernet_config(host, config)

    if status == 200:
        raise TestFailure(f"Expected rejection for localhost routing, got HTTP 200: {body}")

    if verbose:
        print(f"    Rejected with HTTP {status}: {body}")

    print("    PASS")


def test_09_reject_bad_dns_format(host, verbose, original_config, network_info):
    """Config with malformed DNS string should be rejected."""
    print("  [09] Reject bad DNS format ...")

    config = make_config(dns="not-a-dns")
    status, body = set_ethernet_config(host, config)

    if status == 200:
        raise TestFailure(f"Expected rejection for bad DNS, got HTTP 200: {body}")

    if verbose:
        print(f"    Rejected with HTTP {status}: {body}")

    print("    PASS")


def test_10_reject_bad_dns2_format(host, verbose, original_config, network_info):
    """Config with malformed DNS2 string should be rejected."""
    print("  [10] Reject bad DNS2 format ...")

    config = make_config(dns2="also-bad")
    status, body = set_ethernet_config(host, config)

    if status == 200:
        raise TestFailure(f"Expected rejection for bad DNS2, got HTTP 200: {body}")

    if verbose:
        print(f"    Rejected with HTTP {status}: {body}")

    print("    PASS")


def test_11_accept_valid_static_config(host, verbose, original_config, network_info):
    """A well-formed static IP config should be accepted (HTTP 200)."""
    print("  [11] Accept valid static config ...")

    status, body = set_static_and_restore(
        host, "192.168.0.200", original_config, verbose,
        dns="8.8.8.8", dns2="8.8.4.4",
        network_info=network_info,
    )

    if status != 200:
        raise TestFailure(f"Expected HTTP 200 for valid config, got {status}: {body}")

    if verbose:
        print(f"    Accepted with HTTP {status}")

    print("    PASS")


def test_12_accept_dhcp_config(host, verbose, original_config, network_info):
    """DHCP config (all zeros) should be accepted."""
    print("  [12] Accept DHCP config (all zeros) ...")

    config = make_config(
        enable=True,
        ip="0.0.0.0",
        gateway="0.0.0.0",
        subnet="0.0.0.0",
        dns="0.0.0.0",
        dns2="0.0.0.0",
    )
    # apply_config() restarts DHCP which can briefly reset the TCP
    # connection even though the IP doesn't change.
    status, body = set_ethernet_config_ip_change(host, config, host)

    if status != 200:
        raise TestFailure(f"Expected HTTP 200 for DHCP config, got {status}: {body}")

    wait_for_connected(host)

    if verbose:
        print(f"    Accepted with HTTP {status}")

    print("    PASS")


def test_13_accept_gateway_zero_with_static_ip(host, verbose, original_config, network_info):
    """Static IP with 0.0.0.0 gateway should be accepted (no gateway)."""
    print("  [13] Accept static IP with zero gateway ...")

    status, body = set_static_and_restore(
        host, "192.168.0.201", original_config, verbose,
        gateway="0.0.0.0",
        network_info=network_info,
    )

    if status != 200:
        raise TestFailure(f"Expected HTTP 200, got {status}: {body}")

    if verbose:
        print(f"    Accepted with HTTP {status}")

    print("    PASS")


def test_14_config_persisted(host, verbose, original_config, network_info):
    """Config changes should persist (be readable back)."""
    print("  [14] Config changes persist ...")

    static_ip = "192.168.0.202"
    target = make_config(
        enable=True,
        ip=static_ip,
        gateway=network_info["gateway"],
        subnet=network_info["subnet"],
        dns="1.1.1.1",
        dns2="1.0.0.1",
    )
    status, body = set_ethernet_config(host, target)
    if status != 200:
        raise TestFailure(f"Failed to set config: HTTP {status}: {body}")

    # Wait for device at new IP, then read back the config there.
    if not wait_for_host(static_ip):
        raise TestFailure(f"Device did not appear at {static_ip}")

    readback = get_ethernet_config(static_ip)

    for field in ["enable_ethernet", "ip", "gateway", "subnet", "dns", "dns2"]:
        expected = target[field]
        actual = readback.get(field)
        if actual != expected:
            # Restore before raising.
            set_ethernet_config(static_ip, original_config)
            wait_for_host(host)
            raise TestFailure(
                f"Field '{field}' mismatch: expected {expected!r}, got {actual!r}")

    if verbose:
        print(f"    Readback matches: {json.dumps(readback, indent=2)}")

    # Restore original config via new IP.
    try:
        set_ethernet_config(static_ip, original_config)
    except Exception:
        # Connection error during restore is expected if the IP change
        # reset the connection.  Fall through to wait_for_host.
        pass
    if not wait_for_host(host):
        raise Exception(f"Device did not reappear at {host} after restore")
    wait_for_connected(host)

    print("    PASS")


def test_15_rapid_valid_configs(host, verbose, original_config, network_info):
    """Rapidly send multiple valid configs without crash."""
    print("  [15] Rapid valid config changes ...")

    # Send several valid configs rapidly.  Each changes the static IP so
    # apply_config() has real work to do.  We don't wait between them --
    # the point is to stress-test the scheduler queue.
    #
    # Note: after the first apply_config() runs, the device moves to a new
    # IP so subsequent PUTs to `host` may fail with connection errors.
    # That's expected -- we only care that the device survives and settles.
    last_ip = None
    accepted = 0
    for i in range(5):
        last_ip = f"192.168.0.{220 + i}"
        config = make_config(
            enable=True,
            ip=last_ip,
            gateway=network_info["gateway"],
            subnet=network_info["subnet"],
            dns=f"8.8.{i}.{i}",
        )
        try:
            status, body = set_ethernet_config(host, config)
            if status == 200:
                accepted += 1
            elif verbose:
                print(f"    Config {i} rejected: HTTP {status}")
        except Exception as e:
            # Connection error is expected once the device has moved to a
            # different IP from an earlier apply_config().
            if verbose:
                print(f"    Config {i} connection error (expected): {e}")
            # On the very first PUT, retry once after a brief pause since
            # the device may still be stabilizing from the previous test.
            if i == 0:
                time.sleep(2)
                try:
                    status, body = set_ethernet_config(host, config)
                    if status == 200:
                        accepted += 1
                    elif verbose:
                        print(f"    Config {i} retry rejected: HTTP {status}")
                except Exception as e2:
                    if verbose:
                        print(f"    Config {i} retry also failed: {e2}")
                    break
            else:
                break

    if accepted == 0:
        raise TestFailure("No configs were accepted at all")

    if verbose:
        print(f"    {accepted} of 5 configs accepted before device moved")

    # The device should eventually settle on one of the IPs we sent.
    # Try all of them, starting from the last.
    settled_ip = None
    for i in reversed(range(5)):
        try_ip = f"192.168.0.{220 + i}"
        if wait_for_host(try_ip, timeout=5):
            settled_ip = try_ip
            break

    if settled_ip is None:
        # Maybe still on original DHCP IP?
        if wait_for_host(host, timeout=5):
            settled_ip = host
    if settled_ip is None:
        raise TestFailure("Device not reachable on any expected IP after rapid changes")

    state = get_ethernet_state(settled_ip)
    if "connection_state" not in state:
        raise TestFailure("Device not responding correctly after rapid changes")

    if verbose:
        print(f"    Device settled at {settled_ip}")
        print(f"    Final state: {json.dumps(state, indent=2)}")

    # Restore original config.
    set_ethernet_config_ip_change(settled_ip, original_config, host)
    if not wait_for_host(host):
        raise Exception(f"Device did not reappear at {host} after restore")

    # Extra stabilization after rapid changes before the next test.
    wait_for_connected(host)

    print("    PASS")


def test_16_rejected_config_not_saved(host, verbose, original_config, network_info):
    """A rejected config should not change the stored config."""
    print("  [16] Rejected config not saved ...")

    static_ip = "192.168.0.210"

    # Set a known good config first.
    good = make_config(
        enable=True,
        ip=static_ip,
        gateway=network_info["gateway"],
        subnet=network_info["subnet"],
    )
    status, body = set_ethernet_config_ip_change(host, good, static_ip)
    if status != 200:
        raise TestFailure(f"Failed to set baseline config: HTTP {status}: {body}")

    if not wait_for_host(static_ip):
        raise TestFailure(f"Device did not appear at {static_ip}")

    # Try to set a bad config (via the current IP).
    bad = make_config(ip="not-valid")
    status, body = set_ethernet_config(static_ip, bad)
    if status == 200:
        raise TestFailure("Bad config was unexpectedly accepted")

    # Read back and verify the good config is still there.
    readback = get_ethernet_config(static_ip)
    if readback["ip"] != static_ip:
        set_ethernet_config(static_ip, original_config)
        wait_for_host(host)
        raise TestFailure(
            f"Config was corrupted by rejected update: ip={readback['ip']!r}")

    if verbose:
        print(f"    Config unchanged: ip={readback['ip']}")

    # Restore original config.
    try:
        set_ethernet_config(static_ip, original_config)
    except Exception:
        pass
    if not wait_for_host(host):
        raise Exception(f"Device did not reappear at {host} after restore")
    wait_for_connected(host)

    print("    PASS")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

ALL_TESTS = [
    test_01_read_config,
    test_02_read_state,
    test_03_reject_bad_ip_format,
    test_04_reject_bad_gateway_format,
    test_05_reject_bad_subnet_format,
    test_06_reject_invalid_subnet_mask,
    test_07_reject_gateway_not_in_subnet,
    test_08_reject_localhost_routing,
    test_09_reject_bad_dns_format,
    test_10_reject_bad_dns2_format,
    test_11_accept_valid_static_config,
    test_12_accept_dhcp_config,
    test_13_accept_gateway_zero_with_static_ip,
    test_14_config_persisted,
    test_15_rapid_valid_configs,
    test_16_rejected_config_not_saved,
]


def main():
    parser = argparse.ArgumentParser(
        description="Integration test for ethernet config validation.",
        epilog=(
            "Example:\n"
            "  python3 test_ethernet_config.py 192.168.0.47\n"
            "  python3 test_ethernet_config.py 192.168.0.47 --verbose\n"
            "  python3 test_ethernet_config.py 192.168.0.47 --test 07"
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
    print("=== Ethernet Config Validation Test ===")
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
            test_fn(host, verbose, original_config, network_info)
            passed += 1
        except TestFailure as e:
            failed += 1
            errors.append((test_fn.__name__, str(e)))
            print(f"    FAIL: {e}")
        except Exception as e:
            failed += 1
            errors.append((test_fn.__name__, f"Unexpected error: {e}"))
            print(f"    ERROR: {e}")

    # Cleanup: restore original config.  The device might be at a
    # different IP if a test failed mid-way, so try the original host
    # first and fall back to well-known static IPs used by the tests.
    print("\nCleaning up ...")
    restored = False
    for try_host in [host, "192.168.0.200", "192.168.0.201", "192.168.0.202",
                     "192.168.0.210"] + [f"192.168.0.{220+i}" for i in range(5)]:
        try:
            status, body = set_ethernet_config(try_host, original_config)
            if status == 200:
                restored = True
                if try_host != host:
                    print(f"  Restored config via {try_host}, waiting for {host} ...")
                    wait_for_host(host, timeout=15)
                wait_for_connected(host)
                break
        except Exception:
            continue

    if not restored:
        print("  WARNING: Could not restore original config on any known IP!")

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
