#!/usr/bin/env python3
"""Stress test for ESP32 web server.

Rapidly enables/disables solar_forecast and day_ahead_prices modules
and sends random solare forecast / day ahead price data via push api
while concurrently fetching web pages and holding WebSocket connections.
Designed to trigger heap corruption, use-after-free, and cross-thread
race conditions in the HTTP/WS path.

Usage:
    python3 stress_test.py <device-ip> [--duration 300] [--fetchers 10] [--ws-clients 10] [--seed 42]
"""

import argparse
import json
import math
import random
import socket
import sys
import threading
import time
import urllib.error
import urllib.request

import websocket


# ---------------------------------------------------------------------------
# Stats (thread-safe via simple atomics / lock)
# ---------------------------------------------------------------------------

class Stats:
    def __init__(self):
        self.lock = threading.Lock()
        self.toggle_cycles = 0
        self.http_ok = 0
        self.http_err = 0
        self.http_timeout = 0
        self.ws_sessions = 0
        self.ws_messages = 0
        self.ws_errors = 0
        self.health_ok = 0
        self.health_fail = 0
        self.device_died_at = None

    def inc(self, field, n=1):
        with self.lock:
            setattr(self, field, getattr(self, field) + n)

    def snapshot(self):
        with self.lock:
            return {k: v for k, v in self.__dict__.items() if k != "lock"}


# ---------------------------------------------------------------------------
# HTTP helpers
# ---------------------------------------------------------------------------

def get_json(host, path, timeout=5):
    url = f"http://{host}/{path}"
    req = urllib.request.Request(url)
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        return json.loads(resp.read())


def put_json(host, path, payload, timeout=10):
    url = f"http://{host}/{path}"
    data = json.dumps(payload).encode("utf-8")
    req = urllib.request.Request(url, data=data, method="PUT")
    req.add_header("Content-Type", "application/json")
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            body = resp.read().decode("utf-8", errors="replace")
            return resp.status, body
    except urllib.error.HTTPError as e:
        body = e.read().decode("utf-8", errors="replace")
        return e.code, body


def get_raw(host, path, timeout=5):
    """GET a URL and return (status, length). Discards body."""
    url = f"http://{host}/{path}"
    req = urllib.request.Request(url)
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        body = resp.read()
        return resp.status, len(body)


# ---------------------------------------------------------------------------
# Data generators (from existing test scripts)
# ---------------------------------------------------------------------------

MAX_FORECAST_SLOTS = 49
DAP_MAX_SLOTS_60 = 48
DAP_MAX_SLOTS_15 = 192


def make_forecast(count, rng):
    """Generate a bell-curve solar forecast in Wh."""
    forecast = []
    base = rng.randint(200, 800)
    amplitude = rng.randint(100, 500)
    for i in range(count):
        hour_of_day = i % 24
        solar_curve = math.exp(-0.5 * ((hour_of_day - 12) / 3.5) ** 2)
        wh = int(base * solar_curve + amplitude * solar_curve * math.sin(2 * math.pi * i / 12))
        forecast.append(max(0, wh))
    return forecast


def make_prices(count, rng):
    """Generate a sine-wave price curve in ct/1000 per kWh."""
    base = rng.randint(2000, 10000)
    amplitude = rng.randint(1000, 5000)
    return [int(base + amplitude * math.sin(2 * math.pi * i / count)) for i in range(count)]


# ---------------------------------------------------------------------------
# Config payloads
# ---------------------------------------------------------------------------

SF_CONFIG_ENABLE = {
    "enable": True,
    "source": 1,  # Push
    "api_url": "",
    "cert_id": -1,
}

SF_CONFIG_DISABLE = {
    "enable": False,
    "source": 1,
    "api_url": "",
    "cert_id": -1,
}

DAP_CONFIG_ENABLE = {
    "enable": True,
    "source": 1,  # Push
    "api_url": "",
    "region": 0,
    "resolution": 1,
    "cert_id": -1,
    "vat": 0,
    "grid_costs_and_taxes": 0,
    "supplier_markup": 0,
    "supplier_base_fee": 0,
    "enable_calendar": False,
}

DAP_CONFIG_DISABLE = {
    "enable": False,
    "source": 1,
    "api_url": "",
    "region": 0,
    "resolution": 1,
    "cert_id": -1,
    "vat": 0,
    "grid_costs_and_taxes": 0,
    "supplier_markup": 0,
    "supplier_base_fee": 0,
    "enable_calendar": False,
}


# ---------------------------------------------------------------------------
# Config toggler thread
# ---------------------------------------------------------------------------

def config_toggler(host, stop_event, stats, rng_seed):
    """Randomly enable/disable SF and DAP, pushing data in between."""
    rng = random.Random(rng_seed)

    while not stop_event.is_set():
        try:
            module = rng.choice(["sf", "dap"])

            if module == "sf":
                _cycle_solar_forecast(host, rng, stats)
            else:
                _cycle_day_ahead_prices(host, rng, stats)

            stats.inc("toggle_cycles")

        except (urllib.error.URLError, OSError, TimeoutError) as e:
            # Device might be busy; back off a bit
            time.sleep(0.5)
        except Exception as e:
            sys.stderr.write(f"[toggler] unexpected error: {e}\n")
            time.sleep(1.0)

        # Random delay between cycles (0.2 - 2 seconds)
        stop_event.wait(rng.uniform(0.2, 2.0))


def _cycle_solar_forecast(host, rng, stats):
    """One enable -> push -> read -> disable cycle for solar_forecast."""
    # Enable
    put_json(host, "solar_forecast/config_update", SF_CONFIG_ENABLE)

    # Small delay to let config take effect
    time.sleep(rng.uniform(0.05, 0.3))

    # Push forecast to 1-3 random planes
    now = time.time()
    local_midnight = time.mktime(time.localtime(now)[:3] + (0, 0, 0, 0, 0, -1))
    first_date_minutes = int(local_midnight) // 60

    num_planes = rng.randint(1, 3)
    planes = rng.sample(range(6), num_planes)

    for plane_idx in planes:
        count = rng.randint(12, MAX_FORECAST_SLOTS)
        forecast = make_forecast(count, rng)
        payload = {
            "first_date": first_date_minutes,
            "resolution": 1,  # 60min
            "forecast": forecast,
        }
        put_json(host, f"solar_forecast/planes/{plane_idx}/forecast_update", payload)

    # Optionally read back state
    if rng.random() < 0.7:
        time.sleep(rng.uniform(0.05, 0.2))
        try:
            get_json(host, "solar_forecast/state")
            for p in planes:
                get_json(host, f"solar_forecast/planes/{p}/forecast")
        except Exception:
            pass

    # Disable
    time.sleep(rng.uniform(0.05, 0.5))
    put_json(host, "solar_forecast/config_update", SF_CONFIG_DISABLE)


def _cycle_day_ahead_prices(host, rng, stats):
    """One enable -> push -> read -> disable cycle for day_ahead_prices."""
    # Enable
    put_json(host, "day_ahead_prices/config_update", DAP_CONFIG_ENABLE)

    time.sleep(rng.uniform(0.05, 0.3))

    # Push prices
    now = time.time()
    midnight_today = int(now) - int(now) % 86400
    first_date_minutes = midnight_today // 60
    count = rng.randint(12, DAP_MAX_SLOTS_60)
    prices = make_prices(count, rng)

    payload = {
        "first_date": first_date_minutes,
        "resolution": 1,  # Min60
        "prices": prices,
    }
    put_json(host, "day_ahead_prices/prices_update", payload)

    # Optionally read back
    if rng.random() < 0.7:
        time.sleep(rng.uniform(0.05, 0.2))
        try:
            get_json(host, "day_ahead_prices/state")
            get_json(host, "day_ahead_prices/prices")
        except Exception:
            pass

    # Disable
    time.sleep(rng.uniform(0.05, 0.5))
    put_json(host, "day_ahead_prices/config_update", DAP_CONFIG_DISABLE)


# ---------------------------------------------------------------------------
# HTTP fetcher threads
# ---------------------------------------------------------------------------

# Endpoints to hammer. Mix of large and small responses.
FETCH_URLS = [
    "/",                          # ~360KB SPA (largest response)
    "/debug_report",              # Large JSON state dump
    "/event_log",                 # Event log ring buffer
    "/credential_check",          # Tiny 200 response
    "/login_state",               # Tiny 200 response
    "/info/version",              # Small JSON
    "/info/modules",              # Module list
    "/solar_forecast/state",      # SF state (may 404 if disabled)
    "/day_ahead_prices/state",    # DAP state (may 404 if disabled)
    "/day_ahead_prices/prices",   # DAP prices
]


def http_fetcher(host, stop_event, stats, rng_seed, fetcher_id):
    """Continuously fetch random URLs from the device."""
    rng = random.Random(rng_seed)

    while not stop_event.is_set():
        url = rng.choice(FETCH_URLS)
        try:
            status, length = get_raw(host, url, timeout=10)
            stats.inc("http_ok")
        except (urllib.error.HTTPError) as e:
            # 4xx/5xx are still "the server responded" — count as ok
            stats.inc("http_ok")
        except (socket.timeout, TimeoutError):
            stats.inc("http_timeout")
        except (urllib.error.URLError, OSError):
            stats.inc("http_err")
        except Exception:
            stats.inc("http_err")

        # Short random delay (0 - 0.5s)
        stop_event.wait(rng.uniform(0.0, 0.5))


# ---------------------------------------------------------------------------
# WebSocket client threads (using websocket-client library)
# ---------------------------------------------------------------------------

def ws_client(host, port, stop_event, stats, rng_seed, client_id):
    """Repeatedly connect via WebSocket, read for a while, disconnect."""
    rng = random.Random(rng_seed)
    url = f"ws://{host}:{port}/ws"

    while not stop_event.is_set():
        ws = None
        try:
            ws = websocket.WebSocket()
            ws.settimeout(5.0)
            ws.connect(url)

            stats.inc("ws_sessions")

            # Read messages for a short random duration (0.5 - 4 seconds)
            session_duration = rng.uniform(0.5, 4.0)
            session_start = time.monotonic()
            ws.settimeout(2.0)

            while not stop_event.is_set():
                elapsed = time.monotonic() - session_start
                if elapsed >= session_duration:
                    break

                try:
                    opcode, data = ws.recv_data(control_frame=True)
                    if opcode == websocket.ABNF.OPCODE_CLOSE:
                        break
                    stats.inc("ws_messages")
                except websocket.WebSocketTimeoutException:
                    continue
                except (websocket.WebSocketException, OSError):
                    break

            # Graceful close
            try:
                ws.close()
            except Exception:
                pass

        except (websocket.WebSocketTimeoutException, TimeoutError):
            stats.inc("ws_errors")
        except (websocket.WebSocketException, OSError, ConnectionError):
            stats.inc("ws_errors")
        except Exception as e:
            stats.inc("ws_errors")
            sys.stderr.write(f"[ws-{client_id}] unexpected: {e}\n")
        finally:
            if ws:
                try:
                    ws.close()
                except Exception:
                    pass

        # Brief pause before reconnecting (0 - 0.5 seconds)
        stop_event.wait(rng.uniform(0.0, 0.5))


# ---------------------------------------------------------------------------
# Health check
# ---------------------------------------------------------------------------

def health_checker(host, stop_event, stats, interval=10.0):
    """Periodically check if the device is still responding."""
    consecutive_failures = 0

    while not stop_event.is_set():
        stop_event.wait(interval)
        if stop_event.is_set():
            break

        try:
            get_json(host, "info/version", timeout=5)
            stats.inc("health_ok")
            consecutive_failures = 0
        except Exception:
            stats.inc("health_fail")
            consecutive_failures += 1

            if consecutive_failures >= 3:
                with stats.lock:
                    if stats.device_died_at is None:
                        stats.device_died_at = time.monotonic()
                sys.stderr.write(
                    f"\n*** DEVICE UNRESPONSIVE after {consecutive_failures} consecutive health check failures ***\n"
                )


# ---------------------------------------------------------------------------
# Status printer
# ---------------------------------------------------------------------------

def print_status(stats, start_time):
    """Print a one-line status update."""
    s = stats.snapshot()
    elapsed = time.monotonic() - start_time
    total_http = s["http_ok"] + s["http_err"] + s["http_timeout"]

    parts = [
        f"T+{int(elapsed):>4d}s",
        f"cycles={s['toggle_cycles']}",
        f"http={total_http}({s['http_ok']}ok/{s['http_err']}err/{s['http_timeout']}tout)",
        f"ws={s['ws_sessions']}sess/{s['ws_messages']}msg/{s['ws_errors']}err",
        f"health={s['health_ok']}ok/{s['health_fail']}fail",
    ]

    if s["device_died_at"] is not None:
        died_at = s["device_died_at"] - start_time
        parts.append(f"DEAD@{int(died_at)}s")

    sys.stdout.write("\r" + " | ".join(parts) + "   ")
    sys.stdout.flush()


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Stress test ESP32 web server with config toggling and concurrent HTTP/WS load.",
        epilog=(
            "Examples:\n"
            "  python3 stress_test.py 192.168.0.33\n"
            "  python3 stress_test.py 192.168.0.33 --duration 600 --fetchers 4\n"
            "  python3 stress_test.py 192.168.0.33 --seed 42 --ws-clients 3\n"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("host", nargs="?", default=None, help="Device IP or hostname")
    parser.add_argument("--port", type=int, default=80, help="HTTP port (default: 80)")
    parser.add_argument("--duration", type=int, default=300,
                        help="Test duration in seconds (default: 300)")
    parser.add_argument("--fetchers", type=int, default=10,
                        help="Number of concurrent HTTP fetcher threads (default: 10)")
    parser.add_argument("--ws-clients", type=int, default=10,
                        help="Number of concurrent WebSocket clients (default: 10)")
    parser.add_argument("--seed", type=int, default=None,
                        help="Random seed for reproducibility (default: random)")
    args = parser.parse_args()

    if args.host is None:
        parser.print_help()
        sys.exit(1)

    seed = args.seed if args.seed is not None else random.randint(0, 2**32 - 1)
    print(f"Stress test: host={args.host}:{args.port}, duration={args.duration}s, "
          f"fetchers={args.fetchers}, ws_clients={args.ws_clients}, seed={seed}")

    # Check device is reachable
    print(f"Checking device connectivity... ", end="", flush=True)
    try:
        version = get_json(args.host, "info/version", timeout=5)
        fw = version.get("firmware", "?")
        print(f"OK (firmware: {fw})")
    except Exception as e:
        print(f"FAILED: {e}")
        sys.exit(1)

    # Check if modules are available
    print("Checking module availability... ", end="", flush=True)
    modules_ok = True
    for mod in ["solar_forecast", "day_ahead_prices"]:
        try:
            get_json(args.host, f"{mod}/config", timeout=5)
        except Exception as e:
            print(f"\n  WARNING: {mod} not available ({e})")
            modules_ok = False
    if modules_ok:
        print("OK (solar_forecast + day_ahead_prices)")
    else:
        print("\nSome modules missing. Test may have reduced coverage.")

    # Save original configs to restore later
    print("Saving original configs... ", end="", flush=True)
    original_configs = {}
    for mod in ["solar_forecast", "day_ahead_prices"]:
        try:
            original_configs[mod] = get_json(args.host, f"{mod}/config", timeout=5)
        except Exception:
            pass
    print(f"saved {len(original_configs)} config(s)")

    stop_event = threading.Event()
    stats = Stats()
    threads = []

    # Use seed to derive sub-seeds for each thread
    master_rng = random.Random(seed)

    # Config toggler
    t = threading.Thread(
        target=config_toggler,
        args=(args.host, stop_event, stats, master_rng.randint(0, 2**32)),
        name="toggler",
        daemon=True,
    )
    threads.append(t)

    # HTTP fetchers
    for i in range(args.fetchers):
        t = threading.Thread(
            target=http_fetcher,
            args=(args.host, stop_event, stats, master_rng.randint(0, 2**32), i),
            name=f"fetcher-{i}",
            daemon=True,
        )
        threads.append(t)

    # WebSocket clients
    for i in range(args.ws_clients):
        t = threading.Thread(
            target=ws_client,
            args=(args.host, args.port, stop_event, stats, master_rng.randint(0, 2**32), i),
            name=f"ws-{i}",
            daemon=True,
        )
        threads.append(t)

    # Health checker
    t = threading.Thread(
        target=health_checker,
        args=(args.host, stop_event, stats),
        name="health",
        daemon=True,
    )
    threads.append(t)

    # Start all threads
    print(f"\nStarting {len(threads)} threads (1 toggler, {args.fetchers} fetchers, "
          f"{args.ws_clients} WS clients, 1 health checker)...")
    start_time = time.monotonic()

    for t in threads:
        t.start()

    # Run for the specified duration, printing status every 5 seconds
    try:
        deadline = start_time + args.duration
        while time.monotonic() < deadline:
            time.sleep(5.0)
            print_status(stats, start_time)

            # Early exit if device is dead
            s = stats.snapshot()
            if s["device_died_at"] is not None:
                elapsed = s["device_died_at"] - start_time
                sys.stdout.write(f"\n\nDevice died at T+{int(elapsed)}s. Stopping test.\n")
                break
    except KeyboardInterrupt:
        sys.stdout.write("\n\nInterrupted by user. Stopping...\n")

    stop_event.set()

    # Wait for threads to finish (with timeout)
    for t in threads:
        t.join(timeout=5.0)

    # Final stats
    s = stats.snapshot()
    elapsed = time.monotonic() - start_time
    total_http = s["http_ok"] + s["http_err"] + s["http_timeout"]

    print(f"\n\n{'='*60}")
    print(f"  STRESS TEST RESULTS (seed={seed})")
    print(f"{'='*60}")
    print(f"  Duration:          {int(elapsed)}s")
    print(f"  Config cycles:     {s['toggle_cycles']}")
    print(f"  HTTP requests:     {total_http} ({s['http_ok']} ok, {s['http_err']} errors, {s['http_timeout']} timeouts)")
    print(f"  WebSocket:         {s['ws_sessions']} sessions, {s['ws_messages']} messages, {s['ws_errors']} errors")
    print(f"  Health checks:     {s['health_ok']} ok, {s['health_fail']} failed")

    if s["device_died_at"] is not None:
        died_at = s["device_died_at"] - start_time
        print(f"  Device status:     DEAD (unresponsive at T+{int(died_at)}s)")
    else:
        print(f"  Device status:     ALIVE")

    print(f"{'='*60}")

    # Restore original configs
    print("\nRestoring original configs... ", end="", flush=True)
    restored = 0
    for mod, config in original_configs.items():
        try:
            put_json(args.host, f"{mod}/config_update", config, timeout=10)
            restored += 1
        except Exception as e:
            sys.stderr.write(f"\n  WARNING: Could not restore {mod} config: {e}")
    print(f"restored {restored}/{len(original_configs)}")

    if s["device_died_at"] is not None:
        sys.exit(1)


if __name__ == "__main__":
    main()
