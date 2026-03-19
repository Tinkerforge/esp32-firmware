#!/usr/bin/env -S uv run --script

# Tests for temperatures push API

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite
    from ._common import make_temperatures, midnight_today_minutes, TEMP_COUNT
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext
    from software.src.modules.temperatures.tests._common import make_temperatures, midnight_today_minutes, TEMP_COUNT


def suite_setup(tc: TestContext):
    config = tc.api('temperatures/config')
    config['enable'] = True
    config['source'] = 1  # Push
    tc.api('temperatures/config_update', config)


def test_push_std(tc: TestContext):
    count = TEMP_COUNT
    first_date = midnight_today_minutes()
    temps = make_temperatures(count)

    payload = {
        'first_date': first_date,
        'temperatures': temps,
    }

    tc.api('temperatures/temperatures_update', payload)

    state = tc.api('temperatures/temperatures')
    tc.assert_eq(first_date, state.get('first_date'))
    tc.assert_eq(count, len(state.get('temperatures', [])))
    tc.assert_eq(temps, state.get('temperatures'))


def test_push_daylight_saving_time_minus1(tc: TestContext):
    count = TEMP_COUNT - 1
    first_date = midnight_today_minutes()
    temps = make_temperatures(count, base_temp=50, amplitude=80)

    payload = {
        'first_date': first_date,
        'temperatures': temps,
    }

    tc.api('temperatures/temperatures_update', payload)

    state = tc.api('temperatures/temperatures')
    tc.assert_eq(first_date, state.get('first_date'))
    tc.assert_eq(count, len(state.get('temperatures', [])))
    tc.assert_eq(temps, state.get('temperatures'))


def test_push_daylight_saving_time_plus1(tc: TestContext):
    count = TEMP_COUNT + 1
    first_date = midnight_today_minutes()
    temps = make_temperatures(count, base_temp=200, amplitude=100)

    payload = {
        'first_date': first_date,
        'temperatures': temps,
    }

    tc.api('temperatures/temperatures_update', payload)

    state = tc.api('temperatures/temperatures')
    tc.assert_eq(first_date, state.get('first_date'))
    tc.assert_eq(count, len(state.get('temperatures', [])))
    tc.assert_eq(temps, state.get('temperatures'))


def test_push_updates_state(tc: TestContext):
    """Push sets last_sync, last_check, and clears next_check."""

    first_date = midnight_today_minutes()
    temps = make_temperatures(TEMP_COUNT)

    payload = {
        'first_date': first_date,
        'temperatures': temps,
    }

    tc.api('temperatures/temperatures_update', payload)

    state = tc.api('temperatures/state')
    tc.assert_gt(0, state.get('last_sync'))
    tc.assert_gt(0, state.get('last_check'))
    tc.assert_eq(0, state.get('next_check'))


if __name__ == '__main__':
    run_testsuite(locals())
