#!/usr/bin/env -S uv run --group tests --script

# Tests for calendar prices

import time

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite
    from ._common import make_prices, midnight_today_minutes, SLOTS_PER_DAY, SLOTS_TOTAL
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext
    from software.src.modules.day_ahead_prices.tests._common import make_prices, midnight_today_minutes, SLOTS_PER_DAY, SLOTS_TOTAL


def suite_setup(tc: TestContext):
    config = tc.api('day_ahead_prices/config')
    config['enable'] = True
    config['source'] = 1  # Push
    config['enable_calendar'] = True
    tc.api('day_ahead_prices/config_update', config)

    # Clear any leftover spot prices
    tc.api('day_ahead_prices/prices_update', {
        'first_date': midnight_today_minutes(),
        'resolution': 0,
        'prices': [0] * 192,
    })

    # Start with zeroed calendar
    tc.api('day_ahead_prices/calendar', {'prices': [0] * SLOTS_TOTAL})


def test_write_and_read_back(tc: TestContext):
    prices = [i % 32767 for i in range(SLOTS_TOTAL)]
    tc.api('day_ahead_prices/calendar', {'prices': prices})

    calendar = tc.api('day_ahead_prices/calendar')
    tc.assert_eq(prices, calendar['prices'])


def test_zero_calendar(tc: TestContext):
    tc.api('day_ahead_prices/calendar', {'prices': [0] * SLOTS_TOTAL})

    calendar = tc.api('day_ahead_prices/calendar')
    tc.assert_eq([0] * SLOTS_TOTAL, calendar['prices'])

    prices_state = tc.api('day_ahead_prices/prices')
    for p in prices_state.get('prices', []):
        tc.assert_eq(0, p)


def test_negative_prices(tc: TestContext):
    cal_value = -100  # -1.00 ct/kWh in ct/100
    expected_price = cal_value * 10  # -1000 ct/1000

    tc.api('day_ahead_prices/calendar', {'prices': [cal_value] * SLOTS_TOTAL})

    prices_state = tc.api('day_ahead_prices/prices')
    actual_prices = prices_state.get('prices', [])
    tc.assert_gt(0, len(actual_prices))
    for p in actual_prices:
        tc.assert_eq(expected_price, p)


def test_per_day_pattern(tc: TestContext):
    # Each day gets a distinct value: Monday=100, Tuesday=200, ..., Sunday=700
    cal_prices = [0] * SLOTS_TOTAL
    for day in range(7):
        value = (day + 1) * 100
        for slot in range(SLOTS_PER_DAY):
            cal_prices[day * SLOTS_PER_DAY + slot] = value

    tc.api('day_ahead_prices/calendar', {'prices': cal_prices})

    # Verify calendar
    calendar = tc.api('day_ahead_prices/calendar')
    tc.assert_eq(cal_prices, calendar['prices'])

    # The prices endpoint should contain today's and tomorrow's values.
    # Determine today's weekday and verify the first 96 slots match.
    now = time.time()
    wday = time.gmtime(now).tm_wday
    expected_today = (wday + 1) * 100 * 10  # ct/100 -> ct/1000

    prices_state = tc.api('day_ahead_prices/prices')
    actual_prices = prices_state.get('prices', [])
    tc.assert_ge(SLOTS_PER_DAY, len(actual_prices))

    # First slots should be today's price
    for p in actual_prices[:SLOTS_PER_DAY]:
        tc.assert_eq(expected_today, p)


def test_push_plus_calendar(tc: TestContext):
    count = 192
    spot_prices = make_prices(count, base_price=5000, amplitude=3000)

    first_date = midnight_today_minutes()
    payload = {
        'first_date': first_date,
        'resolution': 0,  # Min15
        'prices': spot_prices,
    }

    # Zero calendar
    tc.api('day_ahead_prices/calendar', {'prices': [0] * SLOTS_TOTAL})
    tc.api('day_ahead_prices/prices_update', payload)

    # Verify spot-only prices
    prices_state = tc.api('day_ahead_prices/prices')
    tc.assert_eq(spot_prices, prices_state['prices'])

    # Now set a calendar
    cal_value = 200
    tc.api('day_ahead_prices/calendar', {'prices': [cal_value] * SLOTS_TOTAL})

    # Prices endpoint should now return spot + calendar
    prices_state = tc.api('day_ahead_prices/prices')
    actual_prices = prices_state['prices']
    tc.assert_eq(count, len(actual_prices))

    expected_combined = [s + cal_value * 10 for s in spot_prices]
    tc.assert_eq(expected_combined, actual_prices)


def suite_teardown(tc: TestContext):
    tc.api('day_ahead_prices/calendar', {'prices': [0] * SLOTS_TOTAL})

    config = tc.api('day_ahead_prices/config')
    config['enable_calendar'] = False
    tc.api('day_ahead_prices/config_update', config)


if __name__ == '__main__':
    run_testsuite(locals())
