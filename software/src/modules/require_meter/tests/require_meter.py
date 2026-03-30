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

def prepare(tc: TestContext, m: Meter, c: int):
    if tc.get_testbox().get_meter_connected() == m and cfg(tc) == c:
        return

    tc.get_testbox().connect_meter(m)
    cfg(tc, c)

    tc.api('evse/reset', None)
    tc.reboot()

_old_config = None
def suite_setup(tc: TestContext):
    global _old_config
    _old_config = cfg(tc)

def suite_teardown(tc: TestContext):
    global _old_config
    cfg(tc, _old_config)

def test_default(tc: TestContext):
    tc.api('require_meter/config_reset', None)
    tc.get_testbox().connect_meter('none')
    tc.api('evse/reset', None)
    tc.reboot()

    if tc.device_type().is_eltako():
        assert_blocked(tc)
    else:
        assert_disabled(tc)

def test_meter_detection(tc: TestContext):
    prepare(tc, 'none', SMART)

    assert_disabled(tc)
    time.sleep(3)
    assert_disabled(tc)

    tc.get_testbox().connect_meter('real')
    tc.wait_for(lambda: tc.assert_eq(PRO_ENABLED, cfg(tc)))
    tc.wait_for(lambda: assert_unblocked(tc), timeout=10)

def test_block_persistent(tc: TestContext):
    tc.skip("not implemented yet")

def test_immediate_block_if_energy_unavailable(tc: TestContext):
    tc.skip("not implemented yet")

def test_boot_grace_period(tc: TestContext):
    tc.skip("not implemented yet")

def test_meter_redetect(tc: TestContext):
    prepare(tc, 'real', PRO_ENABLED)

    assert_unblocked(tc)
    cfg(tc, SMART)
    tc.wait_for(lambda: assert_unblocked(tc))

def test_unblock(tc: TestContext):
    prepare(tc, 'none', PRO_ENABLED)

    assert_blocked(tc)
    cfg(tc, PRO_DISABLED)
    tc.wait_for(lambda: assert_disabled(tc))

if __name__ == "__main__":
    run_testsuite(locals())
