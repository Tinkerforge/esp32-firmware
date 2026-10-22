#!/usr/bin/env -S uv run --group tests --script

# Tests for temperatures pull API: Starts a local HTTPS server with a
# self-signed certificate, uploads the CA cert to the device,
# configures it to pull from local server.

import json
import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import run_testsuite, TestContext, TestHTTPSServer
from software.src.modules.temperatures.tests._common import make_temperatures, midnight_today_minutes, TEMP_COUNT

CERT_ID = 6


def set_temperatures(server: TestHTTPSServer, first_date_seconds: int, temperatures: list[int]):
    payload = {
        "first_date": first_date_seconds,
        "temperatures": temperatures,
    }
    server.set_response(json.dumps(payload, separators=(",", ":")))

https_server: TestHTTPSServer | None = None
original_config: dict | None = None

def get_server() -> TestHTTPSServer:
    assert https_server is not None, "suite_setup must run before tests"
    return https_server


def suite_setup(tc: TestContext):
    global https_server, original_config

    original_config = tc.api('temperatures/config')

    https_server = tc.create_test_https_server(CERT_ID, 'temp-pull-test')

    # Configure device for pull mode
    config = tc.api('temperatures/config')
    config['enable'] = True
    config['source'] = 0 # WeatherService (pull)
    config['api_url'] = https_server.url
    config['cert_id'] = CERT_ID
    config['lat'] = 518846 # 51.8846
    config['long'] = 86251 # 8.6251
    tc.api('temperatures/config_update', config)


def trigger_refetch(tc: TestContext):
    config = tc.api('temperatures/config')
    config['enable'] = False
    tc.api('temperatures/config_update', config)
    config['enable'] = True
    tc.api('temperatures/config_update', config)


def test_pull(tc: TestContext):
    count = TEMP_COUNT
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    temps = make_temperatures(count, base_temp=120, amplitude=80)

    server = get_server()
    set_temperatures(server, first_date_seconds, temps)
    trigger_refetch(tc)

    def check():
        state = tc.api('temperatures/temperatures')
        tc.assert_eq(first_date_minutes, state.get('first_date'))
        tc.assert_eq(count, len(state.get('temperatures', [])))
        tc.assert_eq(temps, state.get('temperatures'))

    tc.wait_for(check)


def test_pull_server_error(tc: TestContext):
    """Server returns HTTP 500. DUT should set last_check but not last_sync, temperatures stay empty."""

    server = get_server()
    server.set_response('{"error":"internal server error"}', status=500)
    trigger_refetch(tc)

    # After the failed fetch: last_check > 0 but last_sync == 0, no temperatures
    def check_error():
        state = tc.api('temperatures/state')
        tc.assert_gt(0, state.get('last_check'))
        tc.assert_eq(0, state.get('last_sync'))
        t = tc.api('temperatures/temperatures')
        tc.assert_eq(0, len(t.get('temperatures', [])))

    tc.wait_for(check_error)

    # Serve valid data and trigger another fetch for recovery
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    temps = make_temperatures(TEMP_COUNT, base_temp=80, amplitude=60)

    set_temperatures(server, first_date_seconds, temps)
    trigger_refetch(tc)

    def check_recovery():
        state = tc.api('temperatures/state')
        tc.assert_gt(0, state.get('last_sync'))
        t = tc.api('temperatures/temperatures')
        tc.assert_eq(TEMP_COUNT, len(t.get('temperatures', [])))
        tc.assert_eq(temps, t.get('temperatures'))

    tc.wait_for(check_recovery)


def test_pull_invalid_json(tc: TestContext):
    """Server returns HTTP 200 with garbage body. DUT should fail to parse, temperatures stay empty."""

    server = get_server()
    server.set_response('this is not valid json at all', status=200)
    trigger_refetch(tc)

    # After the failed parse: last_check > 0 but last_sync == 0, no temperatures
    def check_error():
        state = tc.api('temperatures/state')
        tc.assert_gt(0, state.get('last_check'))
        tc.assert_eq(0, state.get('last_sync'))
        t = tc.api('temperatures/temperatures')
        tc.assert_eq(0, len(t.get('temperatures', [])))

    tc.wait_for(check_error)

    # Serve valid data and trigger another fetch for recovery
    first_date_minutes = midnight_today_minutes()
    first_date_seconds = first_date_minutes * 60
    temps = make_temperatures(TEMP_COUNT, base_temp=200, amplitude=100)

    set_temperatures(server, first_date_seconds, temps)
    trigger_refetch(tc)

    def check_recovery():
        state = tc.api('temperatures/state')
        tc.assert_gt(0, state.get('last_sync'))
        t = tc.api('temperatures/temperatures')
        tc.assert_eq(TEMP_COUNT, len(t.get('temperatures', [])))
        tc.assert_eq(temps, t.get('temperatures'))

    tc.wait_for(check_recovery)


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
        tc.api('temperatures/config_update', original_config)

if __name__ == '__main__':
    run_testsuite(locals())
