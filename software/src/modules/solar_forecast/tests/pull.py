#!/usr/bin/env -S uv run --script

# Tests for solar_forecast pull API: Starts a local HTTPS server with a
# self-signed certificate, uploads the CA cert to the device,
# configures it to pull from local server.

import json
import time

from datetime import datetime, timedelta, timezone
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite, TestHTTPSServer
    from ._common import make_forecast, FORECAST_COUNT
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext, TestHTTPSServer
    from software.src.modules.solar_forecast.tests._common import make_forecast, FORECAST_COUNT

_CERT_ID = 5


def _make_solar_api_response(wh_values: list[int], place: str = "Schloß Holte-Stukenbrock, Germany") -> str:
    """Build a forecast.solar-style API response from hourly Wh values"""
    now = datetime.now(timezone.utc)
    day1 = now.replace(hour=0, minute=0, second=0, microsecond=0)
    day2 = day1 + timedelta(days=1)

    wh_period: dict[str, int] = {}
    for i, wh in enumerate(wh_values):
        day = day1 if i < 24 else day2
        hour = i % 24
        dt = day.replace(hour=hour)
        key = dt.strftime("%Y-%m-%d %H:%M:%S")
        wh_period[key] = wh

    response = {
        "result": {
            "watt_hours_period": wh_period,
        },
        "message": {
            "code": 0,
            "text": "Success",
            "info": {"place": place},
            "ratelimit": {
                "limit": 12,
                "remaining": 11,
                "period": 3600,
            },
        },
    }
    return json.dumps(response, separators=(",", ":"))


_server: TestHTTPSServer | None = None
_skip_reason: str | None = None
_original_config: dict | None = None
_PROBE_PLACE = "__pull_test_probe__"

def _get_server() -> TestHTTPSServer:
    assert _server is not None, "suite_setup must run before tests"
    return _server


def _skip_if_needed(tc: TestContext):
    if _skip_reason is not None:
        tc.skip(_skip_reason)


def _reboot_and_wait(tc: TestContext):
    """Reboot the ESP and wait for it to come back online."""
    try:
        tc.api('reboot', None, timeout=2)
    except Exception:
        pass  # Device drops connection during reboot

    time.sleep(3)

    deadline = time.monotonic() + 30
    while time.monotonic() < deadline:
        try:
            tc.api('solar_forecast/state')
            return
        except Exception:
            time.sleep(1)

    raise RuntimeError("Device did not come back after reboot")


def _trigger_refetch(tc: TestContext):
    config = tc.api('solar_forecast/config')
    config['enable'] = False
    tc.api('solar_forecast/config_update', config)

    plane_cfg = tc.api('solar_forecast/planes/0/config')
    plane_cfg['enable'] = False
    tc.api('solar_forecast/planes/0/config_update', plane_cfg)
    plane_cfg['enable'] = True
    tc.api('solar_forecast/planes/0/config_update', plane_cfg)

    config['enable'] = True
    tc.api('solar_forecast/config_update', config)


def _configure_and_probe(tc: TestContext) -> str | None:
    server = _get_server()

    config = tc.api('solar_forecast/config')
    config['enable'] = False
    config['source'] = 0 # ForecastService (pull)
    config['api_url'] = server.url
    config['cert_id'] = _CERT_ID
    tc.api('solar_forecast/config_update', config)

    plane_cfg = tc.api('solar_forecast/planes/0/config')
    plane_cfg['enable'] = True
    plane_cfg['name'] = 'PullTest'
    plane_cfg['lat'] = 519035 # 51.9035
    plane_cfg['long'] = 86720 # 8.6720
    plane_cfg['dec'] = 30
    plane_cfg['az'] = 0
    plane_cfg['wp'] = 5000 # 5 kWp
    tc.api('solar_forecast/planes/0/config_update', plane_cfg)

    server.set_response(_make_solar_api_response([0] * FORECAST_COUNT, place=_PROBE_PLACE))
    _trigger_refetch(tc)

    def check_probe():
        state = tc.api('solar_forecast/planes/0/state')
        place = state.get('place', 'Unknown')
        assert place != 'Unknown', "Still waiting for fetch"
        return place

    try:
        return tc.wait_for(check_probe, timeout=15, poll_delay=1)
    except AssertionError:
        return None


def suite_setup(tc: TestContext):
    global _server, _skip_reason, _original_config

    _original_config = tc.api('solar_forecast/config')
    _server = tc.create_test_https_server(_CERT_ID, 'sf-pull-test')
    place = _configure_and_probe(tc)

    if place is None:
        # Probe timed out, probably blocked by next_sync_forced
        config = tc.api('solar_forecast/config')
        config['enable'] = False
        tc.api('solar_forecast/config_update', config)

        # Reboot to reset next_sync_forced
        _reboot_and_wait(tc)

        place = _configure_and_probe(tc)
        if place is None:
            raise RuntimeError("Probe fetch failed even after reboot")

    # Disable module after successful probe
    config = tc.api('solar_forecast/config')
    config['enable'] = False
    tc.api('solar_forecast/config_update', config)

    if place != _PROBE_PLACE:
        _skip_reason = f"Firmware uses hardcoded test data (unsigned build)"


def test_pull(tc: TestContext):
    _skip_if_needed(tc)

    wh_values = make_forecast(FORECAST_COUNT, base_wh=600, amplitude=300)
    place = "Schloß Holte-Stukenbrock, Germany"

    server = _get_server()
    server.set_response(_make_solar_api_response(wh_values, place=place))
    _trigger_refetch(tc)

    def check():
        forecast = tc.api('solar_forecast/planes/0/forecast')
        tc.assert_gt(0, forecast.get('first_date'))
        tc.assert_eq(1, forecast.get('resolution'))
        tc.assert_eq(FORECAST_COUNT, len(forecast.get('forecast', [])))
        tc.assert_eq(wh_values, forecast.get('forecast'))

        plane_state = tc.api('solar_forecast/planes/0/state')
        tc.assert_gt(0, plane_state.get('last_sync'))
        tc.assert_gt(0, plane_state.get('last_check'))
        tc.assert_eq(place, plane_state.get('place'))

        mod_state = tc.api('solar_forecast/state')
        tc.assert_eq(12, mod_state.get('rate_limit'))
        tc.assert_eq(11, mod_state.get('rate_remaining'))

    tc.wait_for(check, timeout=20, poll_delay=1)


def test_pull_server_error(tc: TestContext):
    _skip_if_needed(tc)

    server = _get_server()
    server.set_response('{"error":"internal server error"}', status=500)
    _trigger_refetch(tc)

    def check_error():
        plane_state = tc.api('solar_forecast/planes/0/state')
        tc.assert_gt(0, plane_state.get('last_check'))
        tc.assert_eq(0, plane_state.get('last_sync'))
        # On HTTP status error, place is set to the status code string
        tc.assert_eq("500", plane_state.get('place'))
        # Forecast should be cleared (plane config was toggled in _trigger_refetch)
        forecast = tc.api('solar_forecast/planes/0/forecast')
        tc.assert_eq(0, len(forecast.get('forecast', [])))

    tc.wait_for(check_error, timeout=20, poll_delay=1)


def suite_teardown(tc: TestContext):
    global _server

    config = tc.api('solar_forecast/config')
    config['enable'] = False
    tc.api('solar_forecast/config_update', config)

    if _server is not None:
        _server.stop()
        _server = None

    tc.api('certs/remove', {'id': _CERT_ID})

    # Restore original config
    if _original_config is not None:
        tc.api('solar_forecast/config_update', _original_config)


if __name__ == '__main__':
    run_testsuite(locals())
