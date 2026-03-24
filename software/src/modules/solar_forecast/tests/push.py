#!/usr/bin/env -S uv run --group tests --script

# Tests for solar_forecast push API

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import run_testsuite, TestContext
from software.src.modules.solar_forecast.tests._common import make_forecast, midnight_today_minutes, FORECAST_COUNT, MAX_FORECAST_SLOTS


def suite_setup(tc: TestContext):
    # Configure module for push mode
    config = tc.api('solar_forecast/config')
    config['enable'] = True
    config['source'] = 1  # Push
    tc.api('solar_forecast/config_update', config)

    # Configure plane 0 with valid params
    plane_cfg = tc.api('solar_forecast/planes/0/config')
    plane_cfg['enable'] = True
    plane_cfg['name'] = 'TestPlane0'
    plane_cfg['lat'] = 519035 # 51.9035
    plane_cfg['long'] = 86720 # 8.6720
    plane_cfg['dec'] = 30
    plane_cfg['az'] = 0
    plane_cfg['wp'] = 5000 # 5 kWp
    tc.api('solar_forecast/planes/0/config_update', plane_cfg)


def test_push_single_plane(tc: TestContext):
    first_date = midnight_today_minutes()
    forecast = make_forecast(FORECAST_COUNT, base_wh=600, amplitude=300)

    payload = {
        'first_date': first_date,
        'resolution': 1, # 60min
        'forecast': forecast,
    }

    tc.api('solar_forecast/planes/0/forecast_update', payload, timeout=10)

    result = tc.api('solar_forecast/planes/0/forecast')
    tc.assert_eq(first_date, result.get('first_date'))
    tc.assert_eq(1, result.get('resolution'))
    tc.assert_eq(FORECAST_COUNT, len(result.get('forecast', [])))
    tc.assert_eq(forecast, result.get('forecast'))


def test_push_auto_enables_plane(tc: TestContext):
    # Disable plane 1
    plane_cfg = tc.api('solar_forecast/planes/1/config')
    plane_cfg['enable'] = False
    plane_cfg['name'] = 'TestPlane1'
    plane_cfg['lat'] = 519035
    plane_cfg['long'] = 86720
    plane_cfg['dec'] = 25
    plane_cfg['az'] = -10
    plane_cfg['wp'] = 3000
    tc.api('solar_forecast/planes/1/config_update', plane_cfg)

    # Verify it's disabled
    plane_cfg = tc.api('solar_forecast/planes/1/config')
    tc.assert_eq(False, plane_cfg.get('enable'))

    # Push forecast data
    first_date = midnight_today_minutes()
    forecast = make_forecast(FORECAST_COUNT, base_wh=400, amplitude=200)

    payload = {
        'first_date': first_date,
        'resolution': 1,
        'forecast': forecast,
    }

    tc.api('solar_forecast/planes/1/forecast_update', payload, timeout=10)

    # Verify plane got auto-enabled
    plane_cfg = tc.api('solar_forecast/planes/1/config')
    tc.assert_eq(True, plane_cfg.get('enable'))

    # Verify data was stored
    result = tc.api('solar_forecast/planes/1/forecast')
    tc.assert_eq(first_date, result.get('first_date'))
    tc.assert_eq(forecast, result.get('forecast'))


def test_push_updates_state(tc: TestContext):
    first_date = midnight_today_minutes()
    forecast = make_forecast(FORECAST_COUNT)

    payload = {
        'first_date': first_date,
        'resolution': 1,
        'forecast': forecast,
    }

    tc.api('solar_forecast/planes/0/forecast_update', payload, timeout=10)

    state = tc.api('solar_forecast/planes/0/state')
    tc.assert_gt(0, state.get('last_sync'))
    tc.assert_gt(0, state.get('last_check'))


def test_push_daylight_saving_time(tc: TestContext):
    first_date = midnight_today_minutes()
    forecast = make_forecast(MAX_FORECAST_SLOTS, base_wh=700, amplitude=350)

    payload = {
        'first_date': first_date,
        'resolution': 1,
        'forecast': forecast,
    }

    tc.api('solar_forecast/planes/0/forecast_update', payload, timeout=10)

    result = tc.api('solar_forecast/planes/0/forecast')
    tc.assert_eq(first_date, result.get('first_date'))
    tc.assert_eq(MAX_FORECAST_SLOTS, len(result.get('forecast', [])))
    tc.assert_eq(forecast, result.get('forecast'))


def suite_teardown(tc: TestContext):
    plane_cfg = tc.api('solar_forecast/planes/1/config')
    plane_cfg['enable'] = False
    tc.api('solar_forecast/planes/1/config_update', plane_cfg)


if __name__ == '__main__':
    run_testsuite(locals())
