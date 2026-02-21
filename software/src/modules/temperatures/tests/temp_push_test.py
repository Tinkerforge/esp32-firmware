#!/usr/bin/env python3
"""Test script for Temperatures push mode.

Pushes sample temperature data via the HTTP API to a WARP device
with temperatures enabled in Push mode.

Usage:
    python3 temp_push_test.py <device-ip> [--today-min N] [--today-max N] ...

Prerequisites on the device:
    - temperatures/config: enable=true, source=1 (Push)
"""

import argparse
import json
import sys
import time
import urllib.request


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


def midnight_today_utc():
    now = int(time.time())
    return now - now % 86400


def make_sample_temperatures(today_min=-500, today_max=1200, today_avg=350,
                             tomorrow_min=-200, tomorrow_max=1500, tomorrow_avg=650):
    today_date = midnight_today_utc()
    tomorrow_date = today_date + 86400

    return {
        "today_date": today_date,
        "today_min": today_min,
        "today_max": today_max,
        "today_avg": today_avg,
        "tomorrow_date": tomorrow_date,
        "tomorrow_min": tomorrow_min,
        "tomorrow_max": tomorrow_max,
        "tomorrow_avg": tomorrow_avg,
    }


def format_temp(raw):
    if raw == 32767 or raw == -32768:
        return "no data"
    return f"{raw / 100:.2f} °C"


def print_temperatures(label, data):
    print(f"\n{label}:")
    today_ts = data.get("today_date", 0)
    tomorrow_ts = data.get("tomorrow_date", 0)
    today_str = time.strftime("%Y-%m-%d", time.gmtime(today_ts)) if today_ts else "n/a"
    tomorrow_str = time.strftime("%Y-%m-%d", time.gmtime(tomorrow_ts)) if tomorrow_ts else "n/a"

    print(f"  Today ({today_str}):")
    print(f"    Min: {format_temp(data.get('today_min', 32767))}")
    print(f"    Max: {format_temp(data.get('today_max', -32768))}")
    print(f"    Avg: {format_temp(data.get('today_avg', 32767))}")
    print(f"  Tomorrow ({tomorrow_str}):")
    print(f"    Min: {format_temp(data.get('tomorrow_min', 32767))}")
    print(f"    Max: {format_temp(data.get('tomorrow_max', -32768))}")
    print(f"    Avg: {format_temp(data.get('tomorrow_avg', 32767))}")


def main():
    parser = argparse.ArgumentParser(
        description="Push test temperature data to a WARP device.",
        epilog=(
            "Example:\n"
            "  python3 temp_push_test.py 192.168.0.33 --today-min -500 --today-max 1200\n"
            "  python3 temp_push_test.py 192.168.0.33 --check"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("host", nargs="?", default=None, help="Device IP or hostname")
    parser.add_argument("--today-min", type=int, default=-500,
                        help="Today min temp in °C*100 (default: -500 = -5.00°C)")
    parser.add_argument("--today-max", type=int, default=1200,
                        help="Today max temp in °C*100 (default: 1200 = 12.00°C)")
    parser.add_argument("--today-avg", type=int, default=350,
                        help="Today avg temp in °C*100 (default: 350 = 3.50°C)")
    parser.add_argument("--tomorrow-min", type=int, default=-200,
                        help="Tomorrow min temp in °C*100 (default: -200 = -2.00°C)")
    parser.add_argument("--tomorrow-max", type=int, default=1500,
                        help="Tomorrow max temp in °C*100 (default: 1500 = 15.00°C)")
    parser.add_argument("--tomorrow-avg", type=int, default=650,
                        help="Tomorrow avg temp in °C*100 (default: 650 = 6.50°C)")
    parser.add_argument("--check", action="store_true",
                        help="Only check current config/state/temperatures, don't push")
    args = parser.parse_args()

    if args.host is None:
        parser.print_help()
        sys.exit(1)

    # Show current state
    print(f"=== Device: {args.host} ===")

    try:
        config = get_json(args.host, "temperatures/config")
        print(f"\nConfig: {json.dumps(config, indent=2)}")
    except Exception as e:
        print(f"ERROR reading config: {e}")
        sys.exit(1)

    try:
        state = get_json(args.host, "temperatures/state")
        print(f"\nState: {json.dumps(state, indent=2)}")
    except Exception as e:
        print(f"ERROR reading state: {e}")

    try:
        temps = get_json(args.host, "temperatures/temperatures")
        print_temperatures("Current temperatures", temps)
    except Exception as e:
        print(f"ERROR reading temperatures: {e}")

    if args.check:
        return

    # Validate config
    if not config.get("enable", False):
        print("\nERROR: temperatures is not enabled. Set enable=true first.")
        sys.exit(1)
    if config.get("source", 0) != 1:
        print("\nERROR: temperatures is not in Push mode (source=1). Set source=1 first.")
        sys.exit(1)

    # Build payload
    payload = make_sample_temperatures(
        today_min=args.today_min,
        today_max=args.today_max,
        today_avg=args.today_avg,
        tomorrow_min=args.tomorrow_min,
        tomorrow_max=args.tomorrow_max,
        tomorrow_avg=args.tomorrow_avg,
    )

    print(f"\n=== Pushing temperature data ===")
    print(f"Today:    min={format_temp(payload['today_min'])}, "
          f"max={format_temp(payload['today_max'])}, "
          f"avg={format_temp(payload['today_avg'])}")
    print(f"Tomorrow: min={format_temp(payload['tomorrow_min'])}, "
          f"max={format_temp(payload['tomorrow_max'])}, "
          f"avg={format_temp(payload['tomorrow_avg'])}")

    status, body = put_json(args.host, "temperatures/temperatures_update", payload)
    if status == 200:
        print(f"\nOK (HTTP {status})")
    else:
        print(f"\nFAILED (HTTP {status}): {body}")
        sys.exit(1)

    # Verify
    print("\n=== Verifying ===")
    time.sleep(0.5)

    state = get_json(args.host, "temperatures/state")
    print(f"State after push: {json.dumps(state, indent=2)}")

    temps = get_json(args.host, "temperatures/temperatures")
    print_temperatures("Temperatures after push", temps)

    # Check that values match
    ok = True
    for key in ["today_date", "today_min", "today_max", "today_avg",
                "tomorrow_date", "tomorrow_min", "tomorrow_max", "tomorrow_avg"]:
        if temps.get(key) != payload[key]:
            print(f"\nMISMATCH: {key}: expected {payload[key]}, got {temps.get(key)}")
            ok = False

    if ok:
        print("\nSUCCESS: All temperature values pushed and verified.")
    else:
        print("\nWARNING: Some values did not match.")


if __name__ == "__main__":
    main()
