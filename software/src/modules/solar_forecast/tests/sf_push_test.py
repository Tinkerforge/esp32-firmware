#!/usr/bin/env python3
"""Test script for Solar Forecast push mode.

Pushes sample forecast data via the HTTP API to a WARP device
with solar_forecast enabled in Push mode.

Usage:
    python3 sf_push_test.py <device-ip> [--planes 0,1] [--hours N]

Prerequisites on the device:
    - solar_forecast/config: enable=true, source=1 (Push)
"""

import argparse
import json
import math
import sys
import time
import urllib.request

MAX_PLANES = 6
MAX_FORECAST_SLOTS = 49  # 48 hours + 1 for DST switch


def make_forecast(count, base_wh=500, amplitude=400):
    """Generate a bell-curve forecast in Wh (unsigned).

    Simulates a solar day: low in morning, peak at midday, low in evening.
    Default: peaks around 900 Wh, dips to ~100 Wh.
    Values are always >= 0 (clamped).
    """
    forecast = []
    # Generate two days worth of data with a bell curve per day
    hours_per_day = min(count, 24)
    for i in range(count):
        hour_of_day = i % 24
        # Bell curve centered at solar noon (hour 12)
        solar_curve = math.exp(-0.5 * ((hour_of_day - 12) / 3.5) ** 2)
        wh = int(base_wh * solar_curve + amplitude * solar_curve * math.sin(2 * math.pi * i / 12))
        wh = max(0, wh)
        forecast.append(wh)
    return forecast


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
        description="Push test forecast data to a WARP device (solar forecast).",
        epilog=(
            "Examples:\n"
            "  python3 sf_push_test.py 192.168.0.33\n"
            "  python3 sf_push_test.py 192.168.0.33 --planes 0,1 --hours 48\n"
            "  python3 sf_push_test.py 192.168.0.33 --check\n"
            "  python3 sf_push_test.py 192.168.0.33 --planes 0 --base 1000 --amplitude 500"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("host", nargs="?", default=None,
                        help="Device IP or hostname (e.g. 192.168.0.33)")
    parser.add_argument("--planes", type=str, default=None,
                        help="Comma-separated plane indices to push to (default: all enabled)")
    parser.add_argument("--hours", type=int, default=48,
                        help="Number of hours of forecast data to generate (default: 48, max: 49)")
    parser.add_argument("--base", type=int, default=500,
                        help="Base forecast in Wh per hour (default: 500)")
    parser.add_argument("--amplitude", type=int, default=400,
                        help="Forecast amplitude in Wh (default: 400)")
    parser.add_argument("--check", action="store_true",
                        help="Only check current config/state/forecast, don't push")
    args = parser.parse_args()

    if args.host is None:
        parser.print_help()
        sys.exit(1)

    count = min(args.hours, MAX_FORECAST_SLOTS)

    # Show current state
    print(f"=== Device: {args.host} ===\n")

    try:
        config = get_json(args.host, "solar_forecast/config")
        print(f"Config: {json.dumps(config, indent=2)}")
    except Exception as e:
        print(f"ERROR reading config: {e}")
        sys.exit(1)

    try:
        state = get_json(args.host, "solar_forecast/state")
        print(f"\nState: {json.dumps(state, indent=2)}")
    except Exception as e:
        print(f"ERROR reading state: {e}")

    # Read all plane configs and forecasts
    plane_configs = {}
    for i in range(MAX_PLANES):
        try:
            pc = get_json(args.host, f"solar_forecast/planes/{i}/config")
            plane_configs[i] = pc
        except Exception:
            break  # No more planes

    print(f"\n--- Planes ({len(plane_configs)} found) ---")
    for i, pc in plane_configs.items():
        enabled = pc.get("enable", False)
        name = pc.get("name", "")
        print(f"  Plane {i}: enable={enabled}, name=\"{name}\"")
        try:
            ps = get_json(args.host, f"solar_forecast/planes/{i}/state")
            print(f"           state: last_sync={ps.get('last_sync')}, "
                  f"last_check={ps.get('last_check')}, "
                  f"next_check={ps.get('next_check')}, "
                  f"place={ps.get('place')}")
        except Exception:
            pass
        try:
            pf = get_json(args.host, f"solar_forecast/planes/{i}/forecast")
            fc = pf.get("forecast", [])
            print(f"           forecast: first_date={pf.get('first_date')}, "
                  f"resolution={pf.get('resolution')}, "
                  f"count={len(fc)}")
            if fc:
                print(f"           values: [{', '.join(str(v) for v in fc[:6])}{'...' if len(fc) > 6 else ''}]")
        except Exception:
            pass

    if args.check:
        return

    # Validate config
    if not config.get("enable", False):
        print("\nERROR: solar_forecast is not enabled. Set enable=true first.")
        sys.exit(1)
    if config.get("source", 0) != 1:
        print("\nERROR: solar_forecast is not in Push mode (source=1). Set source=1 first.")
        sys.exit(1)

    # Determine which planes to push to
    # Note: In push mode, planes are auto-enabled when data is pushed.
    if args.planes is not None:
        target_planes = [int(p.strip()) for p in args.planes.split(",")]
        for p in target_planes:
            if p not in plane_configs:
                print(f"\nERROR: Plane {p} does not exist (available: {list(plane_configs.keys())})")
                sys.exit(1)
    else:
        # Default: all planes (they will be auto-enabled on push)
        target_planes = list(plane_configs.keys())
        if not target_planes:
            print("\nERROR: No planes found.")
            sys.exit(1)

    # Align first_date to the start of today (midnight local time)
    now = time.time()
    local_midnight = time.mktime(time.localtime(now)[:3] + (0, 0, 0, 0, 0, -1))
    first_date_minutes = int(local_midnight) // 60

    print(f"\n=== Pushing forecast to plane(s) {target_planes} ===")
    print(f"  slots: {count} ({args.hours}h, capped at {MAX_FORECAST_SLOTS})")
    print(f"  first_date: {first_date_minutes} (minutes) = "
          f"{time.strftime('%Y-%m-%d %H:%M %Z', time.localtime(local_midnight))}")
    print(f"  resolution: 1 (60min)")

    success_count = 0
    for plane_idx in target_planes:
        forecast = make_forecast(count, base_wh=args.base, amplitude=args.amplitude)
        total_wh = sum(forecast)

        payload = {
            "first_date": first_date_minutes,
            "resolution": 1,  # RESOLUTION_60MIN
            "forecast": forecast,
        }

        name = plane_configs[plane_idx].get("name", "")
        print(f"\n  Plane {plane_idx} (\"{name}\"):")
        print(f"    total: {total_wh} Wh ({total_wh / 1000:.1f} kWh)")
        print(f"    range: {min(forecast)} .. {max(forecast)} Wh")
        print(f"    values: [{', '.join(str(v) for v in forecast[:6])}...]")

        status, body = put_json(
            args.host,
            f"solar_forecast/planes/{plane_idx}/forecast_update",
            payload,
        )
        if status == 200:
            print(f"    => OK (HTTP {status})")
            success_count += 1
        else:
            print(f"    => FAILED (HTTP {status}): {body}")

    if success_count == 0:
        print("\nFAILED: No planes were updated successfully.")
        sys.exit(1)

    # Verify
    print(f"\n=== Verifying ({success_count}/{len(target_planes)} pushed) ===")
    time.sleep(0.5)

    try:
        state = get_json(args.host, "solar_forecast/state")
        print(f"\nState after push: {json.dumps(state, indent=2)}")
    except Exception as e:
        print(f"ERROR reading state: {e}")

    all_ok = True
    for plane_idx in target_planes:
        try:
            pf = get_json(args.host, f"solar_forecast/planes/{plane_idx}/forecast")
            fc = pf.get("forecast", [])
            n = len(fc)
            print(f"\nPlane {plane_idx} forecast: first_date={pf.get('first_date')}, "
                  f"resolution={pf.get('resolution')}, count={n}")
            if fc:
                print(f"  values: [{', '.join(str(v) for v in fc[:6])}{'...' if len(fc) > 6 else ''}]")
            if n != count:
                print(f"  WARNING: Expected {count} slots but got {n}.")
                all_ok = False
        except Exception as e:
            print(f"ERROR reading plane {plane_idx} forecast: {e}")
            all_ok = False

    if all_ok:
        print(f"\nSUCCESS: {success_count} plane(s) updated and verified.")
    else:
        print(f"\nWARNING: Some verifications failed (see above).")


if __name__ == "__main__":
    main()
