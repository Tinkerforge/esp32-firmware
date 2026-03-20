#!/usr/bin/env -S uv run --group tests --script

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..test_context import TestContext, run_testsuite
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext

def test_warpx_testbox(tc: TestContext):
    b = tc.get_testbox()

    b.set_cp('A')
    tc.wait_for(lambda: tc.assert_eq(0, tc.api('evse/state')['charger_state']))

    b.set_cp('B')
    tc.wait_for(lambda: tc.assert_eq(2, tc.api('evse/state')['charger_state']))

    b.set_cp('A')
    tc.wait_for(lambda: tc.assert_eq(0, tc.api('evse/state')['charger_state']))


if __name__ == '__main__':
    run_testsuite(locals())
