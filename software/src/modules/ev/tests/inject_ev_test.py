#!/usr/bin/env -S uv run --script
"""Inject a random EV via the ev/inject_ev HTTP API.

Generates a random MAC address and calls ev/inject_ev on the device,
then reads back ev/state and ev/seen_macs to verify.

Usage:
    uv run inject_ev_test.py <device-ip> [--mac AA:BB:CC:DD:EE:FF]
"""

import argparse
import json
import random
import sys
import time
import urllib.request


def get_json(host, path):
    """GET a JSON endpoint from the device."""
    url = f"http://{host}/{path}"
    req = urllib.request.Request(url)
    with urllib.request.urlopen(req, timeout=5) as resp:
        return json.loads(resp.read())


def put_json(host, path, payload):
    """PUT a JSON payload to the device. Returns (status_code, response_body)."""
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


def random_mac():
    """Generate a random unicast MAC address."""
    octets = [random.randint(0x00, 0xFF) for _ in range(6)]
    # Clear multicast bit (bit 0 of first octet)
    octets[0] &= 0xFE
    return ":".join(f"{b:02X}" for b in octets)


def main():
    parser = argparse.ArgumentParser(
        description="Inject a random EV into a WARP device via ev/inject_ev.",
    )
    parser.add_argument("host", nargs="?", default=None, help="Device IP or hostname")
    parser.add_argument("--mac", default=None,
                        help="MAC address to inject (default: random)")
    parser.add_argument("--check", action="store_true",
                        help="Only show current ev/state and ev/seen_macs, don't inject")
    args = parser.parse_args()

    if args.host is None:
        parser.print_help()
        sys.exit(1)

    print(f"=== Device: {args.host} ===\n")

    # Show current state
    try:
        state = get_json(args.host, "ev/state")
        print(f"State: {json.dumps(state, indent=2)}")
    except Exception as e:
        print(f"ERROR reading ev/state: {e}")
        sys.exit(1)

    try:
        seen = get_json(args.host, "ev/seen_macs")
        print(f"\nSeen MACs ({len(seen)} entries):")
        for entry in seen:
            ts = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(entry["last_seen"])) if entry["last_seen"] else "never"
            print(f"  {entry['mac']}  last_seen={ts}")
    except Exception as e:
        print(f"ERROR reading ev/seen_macs: {e}")

    if args.check:
        return

    # Inject
    mac = args.mac if args.mac else random_mac()
    print(f"\n=== Injecting EV with MAC {mac} ===")

    status, body = put_json(args.host, "ev/inject_ev", {"mac": mac})
    if status == 200:
        print(f"OK (HTTP {status})")
    else:
        print(f"FAILED (HTTP {status}): {body}")
        sys.exit(1)

    # Verify
    print("\n=== Verifying ===")
    time.sleep(0.5)

    state = get_json(args.host, "ev/state")
    print(f"State after inject: {json.dumps(state, indent=2)}")

    seen = get_json(args.host, "ev/seen_macs")
    print(f"\nSeen MACs ({len(seen)} entries):")
    for entry in seen:
        ts = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(entry["last_seen"])) if entry["last_seen"] else "never"
        print(f"  {entry['mac']}  last_seen={ts}")

    # Check that our MAC appeared
    found = any(entry["mac"] == mac.upper() for entry in seen)
    if found and state.get("mac") == mac.upper():
        print(f"\nSUCCESS: MAC {mac} injected and visible in state + seen_macs.")
    elif found:
        print(f"\nPARTIAL: MAC {mac} in seen_macs but state.mac is '{state.get('mac')}'.")
    else:
        print(f"\nFAILED: MAC {mac} not found in seen_macs.")
        sys.exit(1)


if __name__ == "__main__":
    main()
