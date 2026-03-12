#!/usr/bin/env -S uv run --script
#
# /// script
# dependencies = [
#   "tinkerforge_util"
# ]
# ///

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext


def suite_setup(tc: TestContext):
    print("suite_setup done")

def suite_teardown(tc: TestContext):
    print("suite_teardown done")

def setup(tc: TestContext):
    print("setup done")

def teardown(tc: TestContext):
    print("teardown done")


def test_success(tc: TestContext):
    print("success")
    tc.log("logtest")
    tc.assert_(True)

def test_fail(tc: TestContext):
    print("fail")
    tc.assert_(False)

def test_error(tc: TestContext):
    print("error")
    raise Exception("test_error")

def test_skip(tc: TestContext):
    print("skip")
    tc.skip()

def test_api(tc: TestContext):
    t = tc.api_get('info/version')['config_type']
    tc.assert_(t == 'warp')


if __name__ == '__main__':
    run_testsuite(locals())
