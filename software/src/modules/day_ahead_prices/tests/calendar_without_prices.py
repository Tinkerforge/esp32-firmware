#!/usr/bin/env -S uv run --group tests --script

# Tests for calendar prices

import time
import requests

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite
    from ._common import make_prices, midnight_today_minutes, SLOTS_PER_DAY, SLOTS_TOTAL
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext
    from software.src.modules.day_ahead_prices.tests._common import make_prices, midnight_today_minutes, SLOTS_PER_DAY, SLOTS_TOTAL

old_calendar = None

def suite_setup(tc: TestContext):
    global old_calendar
    old_calendar = tc.api('day_ahead_prices/calendar')

# Firmwares pre c13bf1a3 did crash when enabling calendar in config
# but not writing to dap/calendar api.
def test_calendar_enabled_without_prices(tc: TestContext):
    tc.api('day_ahead_prices/calendar_reset', None)

    config = tc.api('day_ahead_prices/config')
    config['enable'] = True
    config['source'] = 1  # Push
    config['enable_calendar'] = True
    tc.api('day_ahead_prices/config_update', config)

    # Clear any leftover spot prices
    tc.api('day_ahead_prices/prices_update', {
        'first_date': midnight_today_minutes(),
        'resolution': 0,
        'prices': [123] * 192,
    })

    # Read back to be sure old calendar was not applied (and ESP did not crash)
    tc.assert_eq([123] * 192, tc.api('day_ahead_prices/prices/prices'))

    tc.reboot()

    tc.assert_eq([], tc.api('day_ahead_prices/prices/prices'))

    tc.api('day_ahead_prices/prices_update', {
        'first_date': midnight_today_minutes(),
        'resolution': 0,
        'prices': [123] * 192,
    })

    tc.assert_eq([123] * 192, tc.api('day_ahead_prices/prices/prices'))


def suite_teardown(tc: TestContext):
    if old_calendar and len(old_calendar['prices']) > 0:
        tc.api('day_ahead_prices/calendar', old_calendar)

if __name__ == '__main__':
    run_testsuite(locals())
