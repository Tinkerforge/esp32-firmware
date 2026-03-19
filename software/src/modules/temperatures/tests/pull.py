#!/usr/bin/env -S uv run --script

# Tests for temperatures pull API: Starts a local HTTPS server with a
# self-signed certificate, uploads the CA cert to the device,
# configures it to pull from local server.

import json

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite, TestHTTPSServer
    from ._common import make_temperatures, midnight_today_minutes, TEMP_COUNT
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext, TestHTTPSServer
    from software.src.modules.temperatures.tests._common import make_temperatures, midnight_today_minutes, TEMP_COUNT

_CERT_ID = 6


def _set_temperatures(server: TestHTTPSServer, first_date_seconds: int, temperatures: list[int]):
    """Set server response. Note: API sends first_date in seconds (device divides by 60)."""
    payload = {
        "first_date": first_date_seconds,
        "temperatures": temperatures,
    }
    server.set_response(json.dumps(payload, separators=(",", ":")))


_server: TestHTTPSServer | None = None
def _get_server() -> TestHTTPSServer:
    assert _server is not None, "suite_setup must run before tests"
    return _server


def suite_setup(tc: TestContext):
    global _server

    tc.set_test_timeout(30)

    _server = tc.create_test_https_server(_CERT_ID, 'temp-pull-test')

    # Configure device for pull mode
    config = tc.api('temperatures/config')
    config['enable'] = True
    config['source'] = 0  # WeatherService (pull)
    config['api_url'] = _server.url
    config['cert_id'] = _CERT_ID
    config['lat'] = 519035   # 51.9035 (German location)
    config['long'] = 86720   # 8.6720
    tc.api('temperatures/config_update', config)


def _trigger_refetch(tc: TestContext):
    """Disable and re-enable to force a re-fetch.

    Uses enable toggle (not other config fields) to avoid race conditions
    with in-flight downloads. When enable=false, update() returns immediately.
    """
    config = tc.api('temperatures/config')
    config['enable'] = False
    tc.api('temperatures/config_update', config)
    config['enable'] = True
    tc.api('temperatures/config_update', config)


def test_pull(tc: TestContext):
    """Serve 48 temperatures, trigger fetch, verify device got them."""
    tc.set_test_timeout(30)

    count = TEMP_COUNT
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    temps = make_temperatures(count, base_temp=120, amplitude=80)

    server = _get_server()
    _set_temperatures(server, first_date_seconds, temps)
    _trigger_refetch(tc)

    def check():
        state = tc.api('temperatures/temperatures')
        tc.assert_eq(first_date_minutes, state.get('first_date'))
        tc.assert_eq(count, len(state.get('temperatures', [])))
        tc.assert_eq(temps, state.get('temperatures'))

    tc.wait_for(check, timeout=15, poll_delay=1)


def test_pull_server_error(tc: TestContext):
    """Server returns HTTP 500. DUT should set last_check but not last_sync, temperatures stay empty."""
    tc.set_test_timeout(60)

    server = _get_server()
    server.set_response('{"error":"internal server error"}', status=500)
    _trigger_refetch(tc)

    # After the failed fetch: last_check > 0 but last_sync == 0, no temperatures
    def check_error():
        state = tc.api('temperatures/state')
        tc.assert_gt(0, state.get('last_check'))
        tc.assert_eq(0, state.get('last_sync'))
        t = tc.api('temperatures/temperatures')
        tc.assert_eq(0, len(t.get('temperatures', [])))

    tc.wait_for(check_error, timeout=15, poll_delay=1)

    # Serve valid data and trigger another fetch for recovery
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    temps = make_temperatures(TEMP_COUNT, base_temp=80, amplitude=60)

    _set_temperatures(server, first_date_seconds, temps)
    _trigger_refetch(tc)

    def check_recovery():
        state = tc.api('temperatures/state')
        tc.assert_gt(0, state.get('last_sync'))
        t = tc.api('temperatures/temperatures')
        tc.assert_eq(TEMP_COUNT, len(t.get('temperatures', [])))
        tc.assert_eq(temps, t.get('temperatures'))

    tc.wait_for(check_recovery, timeout=15, poll_delay=1)


def test_pull_invalid_json(tc: TestContext):
    """Server returns HTTP 200 with garbage body. DUT should fail to parse, temperatures stay empty."""
    tc.set_test_timeout(60)

    server = _get_server()
    server.set_response('this is not valid json at all', status=200)
    _trigger_refetch(tc)

    # After the failed parse: last_check > 0 but last_sync == 0, no temperatures
    def check_error():
        state = tc.api('temperatures/state')
        tc.assert_gt(0, state.get('last_check'))
        tc.assert_eq(0, state.get('last_sync'))
        t = tc.api('temperatures/temperatures')
        tc.assert_eq(0, len(t.get('temperatures', [])))

    tc.wait_for(check_error, timeout=15, poll_delay=1)

    # Serve valid data and trigger another fetch for recovery
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    temps = make_temperatures(TEMP_COUNT, base_temp=200, amplitude=100)

    _set_temperatures(server, first_date_seconds, temps)
    _trigger_refetch(tc)

    def check_recovery():
        state = tc.api('temperatures/state')
        tc.assert_gt(0, state.get('last_sync'))
        t = tc.api('temperatures/temperatures')
        tc.assert_eq(TEMP_COUNT, len(t.get('temperatures', [])))
        tc.assert_eq(temps, t.get('temperatures'))

    tc.wait_for(check_recovery, timeout=15, poll_delay=1)


def suite_teardown(tc: TestContext):
    global _server

    # Stop HTTPS server
    if _server is not None:
        _server.stop()
        _server = None

    # Remove test cert from the device
    tc.api('certs/remove', {'id': _CERT_ID})

if __name__ == '__main__':
    run_testsuite(locals())
