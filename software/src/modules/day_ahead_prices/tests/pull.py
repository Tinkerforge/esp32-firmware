#!/usr/bin/env -S uv run --group tests --script

# Tests for pull api: Starts a local HTTPS server with a
# self-signed certificate, uploads the CA cert to the device,
# configures it to pull from local server.

import json
import time

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite, TestHTTPSServer
    from ._common import make_prices, midnight_today_minutes, SLOTS_TOTAL
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext, TestHTTPSServer
    from software.src.modules.day_ahead_prices.tests._common import make_prices, midnight_today_minutes, SLOTS_TOTAL

_CERT_ID = 7


def _set_prices(server: TestHTTPSServer, first_date_seconds: int, prices: list[int], next_date_seconds: int):
    payload = {
        "first_date": first_date_seconds,
        "prices": prices,
        "next_date": next_date_seconds,
    }
    server.set_response(json.dumps(payload, separators=(",", ":")))


_server: TestHTTPSServer | None = None
_original_config: dict | None = None

def _get_server() -> TestHTTPSServer:
    assert _server is not None, "suite_setup must run before tests"
    return _server


def suite_setup(tc: TestContext):
    global _server, _original_config

    _original_config = tc.api('day_ahead_prices/config')

    _server = tc.create_test_https_server(_CERT_ID, 'dap-pull-test')

    # Zero out calendar
    tc.api('day_ahead_prices/calendar', {'prices': [0] * SLOTS_TOTAL})

    # Configure device
    config = tc.api('day_ahead_prices/config')
    config['enable'] = True
    config['source'] = 0 # SpotMarket (pull)
    config['api_url'] = _server.url
    config['cert_id'] = _CERT_ID
    config['region'] = 0 # DE
    config['resolution'] = 0 # Min15
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
    count = 192 # 48 hours x 4 slots/hour
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    prices = make_prices(count, base_price=4000, amplitude=2000)

    next_date_seconds = first_date_seconds + 7 * 86400

    server = _get_server()
    _set_prices(server, first_date_seconds, prices, next_date_seconds)
    _trigger_refetch(tc)

    # Wait for the device to pull and apply the prices
    def check():
        state = tc.api('day_ahead_prices/prices')
        tc.assert_eq(first_date_minutes, state.get('first_date'))
        tc.assert_eq(0, state.get('resolution')) # Min15
        tc.assert_eq(count, len(state.get('prices', [])))
        tc.assert_eq(prices, state.get('prices'))

    tc.wait_for(check)


def test_pull_60min(tc: TestContext):
    count = 48
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    prices = make_prices(count, base_price=6000, amplitude=4000)
    next_date_seconds = first_date_seconds + 7 * 86400

    server = _get_server()
    _set_prices(server, first_date_seconds, prices, next_date_seconds)

    # Switch to 60-min resolution (also triggers re-fetch)
    config = tc.api('day_ahead_prices/config')
    config['resolution'] = 1 # Min60
    tc.api('day_ahead_prices/config_update', config)

    def check():
        state = tc.api('day_ahead_prices/prices')
        tc.assert_eq(first_date_minutes, state.get('first_date'))
        tc.assert_eq(1, state.get('resolution'))  # Min60
        tc.assert_eq(count, len(state.get('prices', [])))
        tc.assert_eq(prices, state.get('prices'))

    tc.wait_for(check)


def test_pull_updates_next_check(tc: TestContext):
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60

    # Phase 1: Serve initial prices with next_date ~90s from now
    now_seconds = int(time.time())
    next_date_seconds = now_seconds + 90
    next_date_minutes = next_date_seconds // 60

    prices_v1 = make_prices(48, base_price=5000, amplitude=1000)
    server = _get_server()
    _set_prices(server, first_date_seconds, prices_v1, next_date_seconds)
    _trigger_refetch(tc)

    def check_v1():
        state = tc.api('day_ahead_prices/state')
        tc.assert_eq(next_date_minutes, state.get('next_check'))
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(prices_v1, p.get('prices'))

    tc.wait_for(check_v1)

    # Phase 2: Swap server to new prices and wait for DUT to fetch again
    prices_v2 = make_prices(48, base_price=8000, amplitude=500)
    far_future_seconds = first_date_seconds + 14 * 86400
    _set_prices(server, first_date_seconds, prices_v2, far_future_seconds)

    def check_v2():
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(prices_v2, p.get('prices'))

    tc.wait_for(check_v2, timeout=150, poll_delay=5)


def test_pull_server_error(tc: TestContext):
    """Server returns HTTP 500. DUT should update last_check but not last_sync, prices stay empty."""

    server = _get_server()
    server.set_response('{"error":"internal server error"}', status=500)
    _trigger_refetch(tc)

    # After the failed fetch: last_check > 0 but last_sync == 0, no prices
    def check_error():
        state = tc.api('day_ahead_prices/state')
        tc.assert_gt(0, state.get('last_check'))
        tc.assert_eq(0, state.get('last_sync'))
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(0, len(p.get('prices', [])))

    tc.wait_for(check_error)

    # Serve valid data and trigger another fetch
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    prices = make_prices(48, base_price=3000, amplitude=1000)
    next_date_seconds = first_date_seconds + 7 * 86400

    _set_prices(server, first_date_seconds, prices, next_date_seconds)
    _trigger_refetch(tc)

    def check_recovery():
        state = tc.api('day_ahead_prices/state')
        tc.assert_gt(0, state.get('last_sync'))
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(48, len(p.get('prices', [])))
        tc.assert_eq(prices, p.get('prices'))

    tc.wait_for(check_recovery)


def test_pull_invalid_json(tc: TestContext):
    """Server returns HTTP 200 with garbage body. DUT should fail to parse, prices stay empty."""

    server = _get_server()
    server.set_response('this is not valid json at all', status=200)
    _trigger_refetch(tc)

    # After the failed parse: last_check > 0 but last_sync == 0, no prices
    def check_error():
        state = tc.api('day_ahead_prices/state')
        tc.assert_gt(0, state.get('last_check'))
        tc.assert_eq(0, state.get('last_sync'))
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(0, len(p.get('prices', [])))

    tc.wait_for(check_error)

    # Serve valid data and trigger another fetch
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    prices = make_prices(48, base_price=7000, amplitude=2000)
    next_date_seconds = first_date_seconds + 7 * 86400

    _set_prices(server, first_date_seconds, prices, next_date_seconds)
    _trigger_refetch(tc)

    def check_recovery():
        state = tc.api('day_ahead_prices/state')
        tc.assert_gt(0, state.get('last_sync'))
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(48, len(p.get('prices', [])))
        tc.assert_eq(prices, p.get('prices'))

    tc.wait_for(check_recovery)


def suite_teardown(tc: TestContext):
    global _server

    # Stop HTTPS server
    if _server is not None:
        _server.stop()
        _server = None

    # Remove test cert from the device
    tc.api('certs/remove', {'id': _CERT_ID})

    # Restore original config
    if _original_config is not None:
        tc.api('day_ahead_prices/config_update', _original_config)

if __name__ == '__main__':
    run_testsuite(locals())
