#!/usr/bin/env -S uv run --script

# Tests for push api

import time

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext

from software.src.modules.day_ahead_prices.tests._common import make_prices, midnight_today_minutes, SLOTS_TOTAL


def suite_setup(tc: TestContext):
    config = tc.api('day_ahead_prices/config')
    config['enable'] = True
    config['source'] = 1  # Push
    tc.api('day_ahead_prices/config_update', config)

    # Zero out calendar price
    tc.api('day_ahead_prices/calendar', {'prices': [0] * SLOTS_TOTAL})


def test_push_60min(tc: TestContext):
    tc.set_test_timeout(30)

    resolution_minutes = 60
    resolution_enum = 1  # Min60
    hours = 48
    slots_per_hour = 60 // resolution_minutes
    count = hours * slots_per_hour

    first_date = midnight_today_minutes()
    prices = make_prices(count)

    payload = {
        'first_date': first_date,
        'resolution': resolution_enum,
        'prices': prices,
    }

    tc.api('day_ahead_prices/prices_update', payload, timeout=10)

    prices_state = tc.api('day_ahead_prices/prices')
    tc.assert_eq(first_date, prices_state.get('first_date'))
    tc.assert_eq(resolution_enum, prices_state.get('resolution'))
    tc.assert_eq(count, len(prices_state.get('prices', [])))
    tc.assert_eq(prices, prices_state.get('prices'))


def test_push_15min(tc: TestContext):
    tc.set_test_timeout(30)

    resolution_minutes = 15
    resolution_enum = 0  # Min15
    hours = 48
    slots_per_hour = 60 // resolution_minutes
    count = hours * slots_per_hour

    first_date = midnight_today_minutes()
    prices = make_prices(count)

    payload = {
        'first_date': first_date,
        'resolution': resolution_enum,
        'prices': prices,
    }

    tc.api('day_ahead_prices/prices_update', payload, timeout=10)

    prices_state = tc.api('day_ahead_prices/prices')
    tc.assert_eq(first_date, prices_state.get('first_date'))
    tc.assert_eq(resolution_enum, prices_state.get('resolution'))
    tc.assert_eq(count, len(prices_state.get('prices', [])))
    tc.assert_eq(prices, prices_state.get('prices'))

if __name__ == '__main__':
    run_testsuite(locals())
