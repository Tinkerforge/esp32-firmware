#!/usr/bin/env -S uv run --script
#
# /// script
# dependencies = [
#   "tinkerforge_util"
# ]
# ///

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ...test_runner.test_context import TestContext, run_testsuite
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext

def test_warp3_testbox(tc: TestContext):
    tc.get_testbox().set_cp('A')
    tc.wait_for(lambda: tc.assert_eq(0, tc.api('evse/state', 'charger_state')))
    tc.get_testbox().set_cp('B')
    tc.wait_for(lambda: tc.assert_eq(1, tc.api('evse/state', 'charger_state')))
    tc.get_testbox().set_cp('A')
    tc.wait_for(lambda: tc.assert_eq(0, tc.api('evse/state', 'charger_state')))

if __name__ == '__main__':
    run_testsuite(locals())
