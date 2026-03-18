#!/usr/bin/env -S uv run --script

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..test_context import TestContext, run_testsuite
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext

def generate_tests():
    return {f"test_{k}": lambda tc: tc.assert_eq(i, k) for i, k in enumerate(range(3))}

if __name__ == '__main__':
    run_testsuite(dict(locals(), **generate_tests()))
