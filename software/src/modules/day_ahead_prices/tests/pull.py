#!/usr/bin/env -S uv run --script

# Tests for pull api: Starts a local HTTPS server with a
# self-signed certificate, uploads the CA cert to the device,
# configures it to pull from local server.

import json
import ssl
import time
import threading

from http.server import HTTPServer, BaseHTTPRequestHandler
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext

from software.src.modules.day_ahead_prices.tests._common import make_prices, midnight_today_minutes, SLOTS_TOTAL

_CERT_ID = 7


# Tiny HTTPS server that serves a configurable JSON response on any path.
class _Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        body = self.server.response_body
        status = self.server.response_status
        encoded = body.encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(encoded)))
        self.end_headers()
        self.wfile.write(encoded)

    # Suppress per-request log lines.
    def log_message(self, format, *args):
        pass


# Wrapper around HTTPS server running in a thread
class _PriceServer:
    def __init__(self, cert_pem: str, key_pem: str, bind_ip: str = "0.0.0.0"):
        self._cert_pem = cert_pem
        self._key_pem = key_pem
        self._bind_ip = bind_ip
        self._httpd: HTTPServer | None = None
        self._thread: threading.Thread | None = None

    def __enter__(self):
        import tempfile, os

        # Write cert/key to temp files for ssl.SSLContext.load_cert_chain
        self._tmpdir = tempfile.mkdtemp()
        cert_path = os.path.join(self._tmpdir, "cert.pem")
        key_path = os.path.join(self._tmpdir, "key.pem")
        with open(cert_path, "w") as f:
            f.write(self._cert_pem)
        with open(key_path, "w") as f:
            f.write(self._key_pem)

        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        ctx.load_cert_chain(cert_path, key_path)

        self._httpd = HTTPServer((self._bind_ip, 0), _Handler)
        self._httpd.socket = ctx.wrap_socket(self._httpd.socket, server_side=True)
        self._httpd.response_body = '{"error":"no data configured"}'
        self._httpd.response_status = 404

        self._thread = threading.Thread(target=self._httpd.serve_forever, daemon=True)
        self._thread.start()
        return self

    def __exit__(self, *exc):
        if self._httpd:
            self._httpd.shutdown()
        if self._thread:
            self._thread.join(timeout=5)
        import shutil
        shutil.rmtree(self._tmpdir, ignore_errors=True)

    @property
    def port(self) -> int:
        assert self._httpd is not None
        return self._httpd.server_address[1]

    def set_response(self, body: str, status: int = 200):
        assert self._httpd is not None
        self._httpd.response_body = body
        self._httpd.response_status = status

    def set_prices(self, first_date_seconds: int, prices: list[int], next_date_seconds: int):
        payload = {
            "first_date": first_date_seconds,
            "prices": prices,
            "next_date": next_date_seconds,
        }
        self.set_response(json.dumps(payload, separators=(",", ":")))


_server: _PriceServer | None = None


def suite_setup(tc: TestContext):
    global _server

    tc.set_test_timeout(30)

    local_ip = tc.get_local_ip()
    ca_pem, server_key_pem = tc.generate_self_signed_cert(local_ip)

    # Start HTTPS server
    _server = _PriceServer(ca_pem, server_key_pem)
    _server.__enter__()

    # Upload CA cert to device
    tc.api('certs/add', {
        'id': _CERT_ID,
        'name': 'dap-pull-test',
        'cert': ca_pem,
    })

    # Zero out calendar
    tc.api('day_ahead_prices/calendar', {'prices': [0] * SLOTS_TOTAL})

    api_url = f"https://{local_ip}:{_server.port}/"

    # Configure device
    config = tc.api('day_ahead_prices/config')
    config['enable'] = True
    config['source'] = 0  # SpotMarket (pull)
    config['api_url'] = api_url
    config['cert_id'] = _CERT_ID
    config['region'] = 0  # DE
    config['resolution'] = 0  # Min15
    config['enable_calendar'] = False
    tc.api('day_ahead_prices/config_update', config)


def _trigger_refetch(tc: TestContext):
    # Disable and re-enable the module to force a re-fetch
    # (clears prices, last_sync, last_check).
    config = tc.api('day_ahead_prices/config')
    config['enable'] = False
    tc.api('day_ahead_prices/config_update', config)
    config['enable'] = True
    tc.api('day_ahead_prices/config_update', config)


def test_pull_15min(tc: TestContext):
    tc.set_test_timeout(30)

    count = 192  # 48 hours x 4 slots/hour
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    prices = make_prices(count, base_price=4000, amplitude=2000)

    next_date_seconds = first_date_seconds + 7 * 86400

    _server.set_prices(first_date_seconds, prices, next_date_seconds)
    _trigger_refetch(tc)

    # Wait for the device to pull and apply the prices
    def check():
        state = tc.api('day_ahead_prices/prices')
        tc.assert_eq(first_date_minutes, state.get('first_date'))
        tc.assert_eq(0, state.get('resolution'))  # Min15
        tc.assert_eq(count, len(state.get('prices', [])))
        tc.assert_eq(prices, state.get('prices'))

    tc.wait_for(check, timeout=15, poll_delay=1)


def test_pull_60min(tc: TestContext):
    tc.set_test_timeout(30)

    count = 48
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    prices = make_prices(count, base_price=6000, amplitude=4000)
    next_date_seconds = first_date_seconds + 7 * 86400

    _server.set_prices(first_date_seconds, prices, next_date_seconds)

    # Switch to 60-min resolution (also triggers re-fetch)
    config = tc.api('day_ahead_prices/config')
    config['resolution'] = 1  # Min60
    tc.api('day_ahead_prices/config_update', config)

    def check():
        state = tc.api('day_ahead_prices/prices')
        tc.assert_eq(first_date_minutes, state.get('first_date'))
        tc.assert_eq(1, state.get('resolution'))  # Min60
        tc.assert_eq(count, len(state.get('prices', [])))
        tc.assert_eq(prices, state.get('prices'))

    tc.wait_for(check, timeout=15, poll_delay=1)


def test_pull_updates_next_check(tc: TestContext):
    tc.set_test_timeout(180)

    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60

    # Phase 1: Serve initial prices with next_date ~90s from now
    now_seconds = int(time.time())
    next_date_seconds = now_seconds + 90
    next_date_minutes = next_date_seconds // 60

    prices_v1 = make_prices(48, base_price=5000, amplitude=1000)
    _server.set_prices(first_date_seconds, prices_v1, next_date_seconds)
    _trigger_refetch(tc)

    def check_v1():
        state = tc.api('day_ahead_prices/state')
        tc.assert_eq(next_date_minutes, state.get('next_check'))
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(prices_v1, p.get('prices'))

    tc.wait_for(check_v1, timeout=15, poll_delay=1)

    # Phase 2: Swap server to new prices and wait for DUT to fetch again
    prices_v2 = make_prices(48, base_price=8000, amplitude=500)
    far_future_seconds = first_date_seconds + 14 * 86400
    _server.set_prices(first_date_seconds, prices_v2, far_future_seconds)

    def check_v2():
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(prices_v2, p.get('prices'))

    tc.wait_for(check_v2, timeout=150, poll_delay=5)


def test_pull_server_error(tc: TestContext):
    """Server returns HTTP 500. DUT should update last_check but not last_sync, prices stay empty."""
    tc.set_test_timeout(60)

    _server.set_response('{"error":"internal server error"}', status=500)
    _trigger_refetch(tc)

    # After the failed fetch: last_check > 0 but last_sync == 0, no prices
    def check_error():
        state = tc.api('day_ahead_prices/state')
        tc.assert_gt(0, state.get('last_check'))
        tc.assert_eq(0, state.get('last_sync'))
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(0, len(p.get('prices', [])))

    tc.wait_for(check_error, timeout=15, poll_delay=1)

    # Serve valid data and trigger another fetch
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    prices = make_prices(48, base_price=3000, amplitude=1000)
    next_date_seconds = first_date_seconds + 7 * 86400

    _server.set_prices(first_date_seconds, prices, next_date_seconds)
    _trigger_refetch(tc)

    def check_recovery():
        state = tc.api('day_ahead_prices/state')
        tc.assert_gt(0, state.get('last_sync'))
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(48, len(p.get('prices', [])))
        tc.assert_eq(prices, p.get('prices'))

    tc.wait_for(check_recovery, timeout=15, poll_delay=1)


def test_pull_invalid_json(tc: TestContext):
    """Server returns HTTP 200 with garbage body. DUT should fail to parse, prices stay empty."""
    tc.set_test_timeout(60)

    _server.set_response('this is not valid json at all', status=200)
    _trigger_refetch(tc)

    # After the failed parse: last_check > 0 but last_sync == 0, no prices
    def check_error():
        state = tc.api('day_ahead_prices/state')
        tc.assert_gt(0, state.get('last_check'))
        tc.assert_eq(0, state.get('last_sync'))
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(0, len(p.get('prices', [])))

    tc.wait_for(check_error, timeout=15, poll_delay=1)

    # Serve valid data and trigger another fetch
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    prices = make_prices(48, base_price=7000, amplitude=2000)
    next_date_seconds = first_date_seconds + 7 * 86400

    _server.set_prices(first_date_seconds, prices, next_date_seconds)
    _trigger_refetch(tc)

    def check_recovery():
        state = tc.api('day_ahead_prices/state')
        tc.assert_gt(0, state.get('last_sync'))
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(48, len(p.get('prices', [])))
        tc.assert_eq(prices, p.get('prices'))

    tc.wait_for(check_recovery, timeout=15, poll_delay=1)


def suite_teardown(tc: TestContext):
    global _server

    # Stop HTTPS server
    if _server is not None:
        _server.__exit__(None, None, None)
        _server = None

    # Remove test cert from the device
    tc.api('certs/remove', {'id': _CERT_ID})

if __name__ == '__main__':
    run_testsuite(locals())
