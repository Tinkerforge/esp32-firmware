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

def test_asserts(tc: TestContext):
    tc.assert_(True)
    tc.assert_false(False)
    tc.assert_eq(1, 1)
    tc.assert_ne(1, 2)
    tc.assert_le(3, 1)
    tc.assert_lt(3, 1)
    tc.assert_gt(1, 3)
    tc.assert_ge(1, 3)
    tc.assert_search("[0-9]123[0-9]", "-1012345")
    tc.assert_in([1, 3, 27], 3)
    tc.assert_in(range(5), 3)
    tc.assert_epsilon(1, 0.2, 0.9)
    import time
    start = time.monotonic()
    tc.wait_for(lambda: tc.assert_gt(start + 2.5, time.monotonic()))
    tc.assert_fail(lambda: tc.fail("fail"))
    tc.assert_xor(lambda: tc.fail("fail"), lambda: tc.assert_(True))

    tc.assert_fail(lambda: tc.assert_(False))
    tc.assert_fail(lambda: tc.assert_false(True))
    tc.assert_fail(lambda: tc.assert_eq(1, 2))
    tc.assert_fail(lambda: tc.assert_ne(1, 1))
    tc.assert_fail(lambda: tc.assert_le(1, 3))
    tc.assert_fail(lambda: tc.assert_lt(1, 3))
    tc.assert_fail(lambda: tc.assert_gt(3, 1))
    tc.assert_fail(lambda: tc.assert_ge(3, 1))
    tc.assert_fail(lambda: tc.assert_search("[0-9]123[0-9]", "abcd"))
    tc.assert_fail(lambda: tc.assert_in([1,2,3], 4))
    tc.assert_fail(lambda: tc.assert_in(range(5), 6))
    tc.assert_fail(lambda: tc.assert_epsilon(1, 0.2, 1.9))

    start = time.monotonic()
    tc.assert_fail(lambda: tc.wait_for(lambda: tc.assert_gt(start + 2.5, time.monotonic()), timeout=1))
    tc.assert_fail(lambda: tc.assert_fail(lambda: tc.assert_(True)))
    tc.assert_fail(lambda: tc.assert_xor(lambda: tc.fail("fail"), lambda: tc.fail("fail2")))
    tc.assert_fail(lambda: tc.assert_xor(lambda: tc.assert_(True), lambda: tc.assert_(True)))

if __name__ == '__main__':
    run_testsuite(locals())
