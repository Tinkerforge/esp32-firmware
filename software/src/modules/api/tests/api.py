#!/usr/bin/env -S uv run --script

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext

def test_api(tc: TestContext):
    tc.assert_in(['brick', 'co2ampel', 'eltako', 'kransteuerung', 'seb', 'warp', 'wem'], tc.api('info/version')['config_type'])

if __name__ == '__main__':
    run_testsuite(locals())
