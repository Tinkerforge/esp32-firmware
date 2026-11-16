#!/usr/bin/env -S uv run --group tests --script

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import run_testsuite, TestContext
from software.test_runner.testbox.testbox import Meter

import time

SMART = 0
PRO_DISABLED = 1
PRO_ENABLED = 2

def cfg(tc: TestContext, x: int | None = None):
    if x is not None:
        return tc.api('require_meter/config/config', x)
    return tc.api('require_meter/config/config')

def assert_disabled(tc: TestContext):
    tc.assert_ne(PRO_ENABLED, cfg(tc))

    s = tc.api('evse/slots/13')

    # Should always be false
    tc.assert_false(s['clear_on_disconnect'])

    tc.assert_false(s['active'])
    # Not checking max_current: If active is false, the max_current has no effect.

def assert_unblocked(tc: TestContext):
    tc.assert_eq(PRO_ENABLED, cfg(tc))

    s = tc.api('evse/slots/13')

    # Should always be false
    tc.assert_false(s['clear_on_disconnect'])

    tc.assert_true(s['active'])
    tc.assert_eq(32000, s['max_current'])

def assert_blocked(tc: TestContext):
    tc.assert_eq(PRO_ENABLED, cfg(tc))

    s = tc.api('evse/slots/13')

    # Should always be false
    tc.assert_false(s['clear_on_disconnect'])

    tc.assert_true(s['active'])
    tc.assert_eq(0, s['max_current'])

def connect_meter_and_reboot(tc: TestContext, m: Meter):
    tc.get_testbox().connect_meter(m)
    tc.api('evse/reset', None)
    tc.reboot()

# The firmware applies config changes asynchronously.
# When enabled, charging is blocked until the first meter check (6 s after enabling) sees up-to-date meter values.
# A meter is detected only a few seconds after it is connected or after a reboot.
SETTLE_TIMEOUT = 30

def prepare(tc: TestContext, m: Meter, c: int):
    if tc.get_testbox().get_meter_connected() == m and cfg(tc) == c:
        return

    connect_meter_and_reboot(tc, m)

    # Set the config only after the reboot: While a meter is still known,
    # setting SMART switches the config to PRO_ENABLED immediately.
    cfg(tc, c)

_old_config = None
_old_meter = None
def suite_setup(tc: TestContext):
    global _old_config, _old_meter
    _old_config = cfg(tc)
    _old_meter = tc.get_testbox().get_meter_connected()

def suite_teardown(tc: TestContext):
    # Other suites expect the meter that was connected before, e.g. for the phase detection.
    if tc.get_testbox().get_meter_connected() != _old_meter:
        connect_meter_and_reboot(tc, _old_meter)

    cfg(tc, _old_config)

def test_default(tc: TestContext):
    tc.api('config/reset', 'require_meter/config')
    tc.get_testbox().connect_meter('none')
    tc.api('evse/reset', None)
    tc.reboot()

    if tc.device_type().is_eltako():
        # Without a meter, charging is only blocked after the boot grace period of 90 s, so only check that it is enabled.
        tc.wait_for(lambda: tc.assert_eq(PRO_ENABLED, cfg(tc)), timeout=SETTLE_TIMEOUT)
        tc.wait_for(lambda: tc.assert_true(tc.api('evse/slots/13')['active']), timeout=SETTLE_TIMEOUT)
    else:
        tc.wait_for(lambda: assert_disabled(tc), timeout=SETTLE_TIMEOUT)

def test_meter_detection(tc: TestContext):
    prepare(tc, 'none', SMART)

    tc.wait_for(lambda: assert_disabled(tc), timeout=SETTLE_TIMEOUT)
    time.sleep(3)
    assert_disabled(tc)

    tc.get_testbox().connect_meter('real')
    tc.wait_for(lambda: tc.assert_eq(PRO_ENABLED, cfg(tc)), timeout=SETTLE_TIMEOUT)
    tc.wait_for(lambda: assert_unblocked(tc), timeout=SETTLE_TIMEOUT)

def test_block_persistent(tc: TestContext):
    tc.skip("not implemented yet")

def test_immediate_block_if_energy_unavailable(tc: TestContext):
    tc.skip("not implemented yet")

def test_boot_grace_period(tc: TestContext):
    tc.skip("not implemented yet")

def test_meter_redetect(tc: TestContext):
    prepare(tc, 'real', PRO_ENABLED)

    tc.wait_for(lambda: assert_unblocked(tc), timeout=SETTLE_TIMEOUT)
    cfg(tc, SMART)
    # The meter is known, so SMART switches back to PRO_ENABLED.
    tc.wait_for(lambda: assert_unblocked(tc), timeout=SETTLE_TIMEOUT)

def test_unblock(tc: TestContext):
    prepare(tc, 'none', PRO_ENABLED)

    # After a reboot without meter, charging is only blocked after the boot grace period of 90 s.
    # Enabling the config blocks immediately.
    cfg(tc, PRO_DISABLED)
    tc.wait_for(lambda: assert_disabled(tc), timeout=SETTLE_TIMEOUT)
    cfg(tc, PRO_ENABLED)

    tc.wait_for(lambda: assert_blocked(tc), timeout=SETTLE_TIMEOUT)
    cfg(tc, PRO_DISABLED)
    tc.wait_for(lambda: assert_disabled(tc), timeout=SETTLE_TIMEOUT)

if __name__ == "__main__":
    run_testsuite(locals())
