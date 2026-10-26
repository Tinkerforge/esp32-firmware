#!/usr/bin/env -S uv run --group tests --script

# Tests for pull api: Starts a local HTTPS server with a
# self-signed certificate, uploads the CA cert to the device,
# configures it to pull from local server.

import json
import threading
import time
import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import run_testsuite, TestContext, TestHTTPSServer
from software.src.modules.day_ahead_prices.tests._common import make_prices, midnight_today_minutes, SLOTS_TOTAL

CERT_ID = 7


def set_prices(server: TestHTTPSServer, first_date_seconds: int, prices: list[int], next_date_seconds: int):
    payload = {
        "first_date": first_date_seconds,
        "prices": prices,
        "next_date": next_date_seconds,
    }
    server.set_response(json.dumps(payload, separators=(",", ":")))


https_server: TestHTTPSServer | None = None
original_config: dict | None = None

def get_server() -> TestHTTPSServer:
    assert https_server is not None, "suite_setup must run before tests"
    return https_server


def base_config(tc: TestContext, **overrides) -> dict:
    config = tc.api('day_ahead_prices/config')
    config['enable'] = False
    config['source'] = 0 # SpotMarket (pull)
    config['api_url'] = get_server().url
    config['cert_id'] = CERT_ID
    config['region'] = 0 # DE
    config['resolution'] = 0 # Min15
    config['enable_calendar'] = False
    config.update(overrides)
    return config


def suite_setup(tc: TestContext):
    global https_server, original_config

    # The DUT only pulls once its clock is synced
    tc.wait_for_clock_synced()

    original_config = tc.api('day_ahead_prices/config')

    https_server = tc.create_test_https_server(CERT_ID, 'dap-pull-test')

    # Zero out calendar
    tc.api('day_ahead_prices/calendar', {'prices': [0] * SLOTS_TOTAL})


def setup(tc: TestContext):
    tc.api('day_ahead_prices/config_update', base_config(tc))


def trigger_refetch(tc: TestContext):
    # Disable and re-enable the module to force a re-fetch
    # (clears prices, last_sync, last_check).
    tc.api('day_ahead_prices/config_update', base_config(tc, enable=False))
    tc.api('day_ahead_prices/config_update', base_config(tc, enable=True))


def test_pull_15min(tc: TestContext):
    count = 192 # 48 hours x 4 slots/hour
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    prices = make_prices(count, base_price=4000, amplitude=2000)

    next_date_seconds = first_date_seconds + 7 * 86400

    server = get_server()
    set_prices(server, first_date_seconds, prices, next_date_seconds)
    trigger_refetch(tc)

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

    server = get_server()
    set_prices(server, first_date_seconds, prices, next_date_seconds)
    tc.api('day_ahead_prices/config_update', base_config(tc, enable=True, resolution=1)) # Min60

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
    server = get_server()
    set_prices(server, first_date_seconds, prices_v1, next_date_seconds)
    trigger_refetch(tc)

    def check_v1():
        state = tc.api('day_ahead_prices/state')
        tc.assert_eq(next_date_minutes, state.get('next_check'))
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(prices_v1, p.get('prices'))

    tc.wait_for(check_v1)

    # Phase 2: Swap server to new prices and wait for DUT to fetch again
    prices_v2 = make_prices(48, base_price=8000, amplitude=500)
    far_future_seconds = first_date_seconds + 14 * 86400
    set_prices(server, first_date_seconds, prices_v2, far_future_seconds)

    def check_v2():
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(prices_v2, p.get('prices'))

    tc.wait_for(check_v2, timeout=150, poll_delay=5)


def test_pull_server_error(tc: TestContext):
    """Server returns HTTP 500. DUT should update last_check but not last_sync, prices stay empty."""

    server = get_server()
    server.set_response('{"error":"internal server error"}', status=500)
    trigger_refetch(tc)

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

    set_prices(server, first_date_seconds, prices, next_date_seconds)
    trigger_refetch(tc)

    def check_recovery():
        state = tc.api('day_ahead_prices/state')
        tc.assert_gt(0, state.get('last_sync'))
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(48, len(p.get('prices', [])))
        tc.assert_eq(prices, p.get('prices'))

    tc.wait_for(check_recovery)


def test_pull_invalid_json(tc: TestContext):
    """Server returns HTTP 200 with garbage body. DUT should fail to parse, prices stay empty."""

    server = get_server()
    server.set_response('this is not valid json at all', status=200)
    trigger_refetch(tc)

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

    set_prices(server, first_date_seconds, prices, next_date_seconds)
    trigger_refetch(tc)

    def check_recovery():
        state = tc.api('day_ahead_prices/state')
        tc.assert_gt(0, state.get('last_sync'))
        p = tc.api('day_ahead_prices/prices')
        tc.assert_eq(48, len(p.get('prices', [])))
        tc.assert_eq(prices, p.get('prices'))

    tc.wait_for(check_recovery)


def test_config_change_during_download(tc: TestContext):
    """ A config change while a download is running discards the running download and downloads again."""

    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    next_date_seconds = first_date_seconds + 7 * 86400
    stale_prices = make_prices(48, base_price=1000, amplitude=500)
    new_prices = make_prices(48, base_price=9000, amplitude=500)

    def payload(prices: list[int]) -> str:
        return json.dumps({"first_date": first_date_seconds, "prices": prices, "next_date": next_date_seconds}, separators=(",", ":"))

    first_request = threading.Event()

    def respond(method: str, path: str, body: bytes) -> tuple[int, str]:
        if not first_request.is_set():
            first_request.set()
            time.sleep(3)  # Keep the first download running while the config changes
            return 200, payload(stale_prices)
        return 200, payload(new_prices)

    server = get_server()
    server.set_response_fn(respond)
    tc.api('day_ahead_prices/config_update', base_config(tc, enable=True))
    tc.assert_true(first_request.wait(timeout=10))
    trigger_refetch(tc)

    def check():
        tc.assert_eq(new_prices, tc.api('day_ahead_prices/prices').get('prices'))

    tc.wait_for(check, timeout=15, poll_delay=0.5)

    # The response of the first download must not overwrite the new prices
    time.sleep(4)
    check()


def suite_teardown(tc: TestContext):
    global https_server

    # Stop HTTPS server
    if https_server is not None:
        https_server.stop()
        https_server = None

    # Remove test cert from the device
    tc.api('certs/remove', {'id': CERT_ID})

    # Restore original config
    if original_config is not None:
        tc.api('day_ahead_prices/config_update', original_config)

if __name__ == '__main__':
    run_testsuite(locals())
