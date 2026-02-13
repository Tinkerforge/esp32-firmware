#!/usr/bin/env python3
"""Test script for Day Ahead Prices push mode.

Pushes sample price data via the HTTP API to a WARP device
with day_ahead_prices enabled in Push mode.

Usage:
    python3 dap_push_test.py <device-ip> [--resolution 15|60] [--hours N]

Prerequisites on the device:
    - day_ahead_prices/config: enable=true, source=1 (Push)
"""

import argparse
import json
import math
import sys
import time
import urllib.request

def minutes_since_epoch(ts=None):
    """Convert unix timestamp (seconds) to minutes."""
    if ts is None:
        ts = time.time()
    return int(ts) // 60

def make_prices(count, base_price=5000, amplitude=3000):
    """Generate a sine-wave price curve in ct/1000 per kWh.

    Default: oscillates between 2000 (2 ct/kWh) and 8000 (8 ct/kWh)
    around a base of 5000 (5 ct/kWh).
    """
    prices = []
    for i in range(count):
        price = int(base_price + amplitude * math.sin(2 * math.pi * i / count))
        prices.append(price)
    return prices

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

def main():
    parser = argparse.ArgumentParser(
        description="Push test price data to a WARP device.",
        epilog="Example:\n  python3 dap_push_test.py 192.168.0.33 --resolution 15 --hours 48 --base 10000 --amplitude 5000",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("host", nargs="?", default=None, help="Device IP or hostname")
    parser.add_argument("--resolution", type=int, choices=[15, 60], default=60,
                        help="Price resolution in minutes (default: 60)")
    parser.add_argument("--hours", type=int, default=48,
                        help="Number of hours of price data to generate (default: 48)")
    parser.add_argument("--base", type=int, default=5000,
                        help="Base price in ct/1000 per kWh (default: 5000 = 5 ct/kWh)")
    parser.add_argument("--amplitude", type=int, default=3000,
                        help="Price amplitude in ct/1000 per kWh (default: 3000)")
    parser.add_argument("--check", action="store_true",
                        help="Only check current config/state/prices, don't push")
    args = parser.parse_args()

    if args.host is None:
        parser.print_help()
        sys.exit(1)

    # Show current state
    print(f"=== Device: {args.host} ===\n")

    try:
        config = get_json(args.host, "day_ahead_prices/config")
        print(f"Config: {json.dumps(config, indent=2)}")
    except Exception as e:
        print(f"ERROR reading config: {e}")
        sys.exit(1)

    try:
        state = get_json(args.host, "day_ahead_prices/state")
        print(f"\nState: {json.dumps(state, indent=2)}")
    except Exception as e:
        print(f"ERROR reading state: {e}")

    try:
        prices_state = get_json(args.host, "day_ahead_prices/prices")
        n = len(prices_state.get("prices", []))
        print(f"\nPrices: first_date={prices_state.get('first_date')}, "
              f"resolution={prices_state.get('resolution')}, "
              f"count={n}")
    except Exception as e:
        print(f"ERROR reading prices: {e}")

    if args.check:
        return

    # Validate config
    if not config.get("enable", False):
        print("\nERROR: day_ahead_prices is not enabled. Set enable=true first.")
        sys.exit(1)
    if config.get("source", 0) != 1:
        print("\nERROR: day_ahead_prices is not in Push mode (source=1). Set source=1 first.")
        sys.exit(1)

    # Build payload
    resolution_minutes = args.resolution
    resolution_enum = 0 if resolution_minutes == 15 else 1  # Min15=0, Min60=1
    slots_per_hour = 60 // resolution_minutes
    count = args.hours * slots_per_hour

    # Align first_date to the start of today (midnight UTC)
    now = time.time()
    midnight_today = int(now) - int(now) % 86400
    first_date_minutes = midnight_today // 60

    prices = make_prices(count, base_price=args.base, amplitude=args.amplitude)

    payload = {
        "first_date": first_date_minutes,
        "resolution": resolution_enum,
        "prices": prices,
    }

    print(f"\n=== Pushing {count} price slots ({args.hours}h @ {resolution_minutes}min) ===")
    print(f"first_date: {first_date_minutes} (minutes) = {time.strftime('%Y-%m-%d %H:%M UTC', time.gmtime(midnight_today))}")
    print(f"resolution: {resolution_enum} ({'15min' if resolution_enum == 0 else '60min'})")
    print(f"price range: {min(prices)} .. {max(prices)} ct/1000 per kWh")
    print(f"             ({min(prices)/1000:.1f} .. {max(prices)/1000:.1f} ct/kWh)\n")

    status, body = put_json(args.host, "day_ahead_prices/prices_update", payload)
    if status == 200:
        print(f"OK (HTTP {status})")
    else:
        print(f"FAILED (HTTP {status}): {body}")
        sys.exit(1)

    # Verify
    print("\n=== Verifying ===")
    time.sleep(0.5)

    state = get_json(args.host, "day_ahead_prices/state")
    print(f"State after push: {json.dumps(state, indent=2)}")

    prices_state = get_json(args.host, "day_ahead_prices/prices")
    n = len(prices_state.get("prices", []))
    print(f"Prices after push: first_date={prices_state.get('first_date')}, "
          f"resolution={prices_state.get('resolution')}, "
          f"count={n}")

    if n == count:
        print(f"\nSUCCESS: {n} price slots pushed and verified.")
    else:
        print(f"\nWARNING: Expected {count} slots but got {n}.")

if __name__ == "__main__":
    main()
