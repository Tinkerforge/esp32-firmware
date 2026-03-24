#!/usr/bin/env -S uv run --group tests --script

from dataclasses import dataclass
import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import run_testsuite, TestContext

@dataclass
class Cnt:
    suite_setup = 0
    suite_teardown = 0
    setup = 0
    teardown = 0
    test = 0

cnt = Cnt()

def suite_setup(tc: TestContext):
    global cnt
    cnt.suite_setup += 1
    print("suite_setup done")

    tc.assert_eq(1, cnt.suite_setup)
    tc.assert_eq(0, cnt.suite_teardown)
    tc.assert_eq(0, cnt.setup)
    tc.assert_eq(0, cnt.teardown)
    tc.assert_eq(0, cnt.test)

def suite_teardown(tc: TestContext):
    global cnt
    cnt.suite_teardown += 1
    print("suite_teardown done")

    tc.assert_eq(1, cnt.suite_setup)
    tc.assert_eq(1, cnt.suite_teardown)
    tc.assert_eq(cnt.test, cnt.setup)
    tc.assert_eq(cnt.test, cnt.teardown)

    # Set everything to -2 to make sure every assertion will fail after this
    cnt.suite_setup = -2
    cnt.suite_teardown = -2
    cnt.setup = -2
    cnt.teardown = -2
    cnt.test = -2

def setup(tc: TestContext):
    global cnt
    cnt.setup += 1
    print("setup done")

    tc.assert_eq(1, cnt.suite_setup)
    tc.assert_eq(0, cnt.suite_teardown)
    tc.assert_eq(cnt.setup - 1, cnt.teardown)
    tc.assert_eq(cnt.setup - 1, cnt.test)

def teardown(tc: TestContext):
    global cnt
    cnt.teardown += 1
    print("teardown done")

    tc.assert_eq(1, cnt.suite_setup)
    tc.assert_eq(0, cnt.suite_teardown)
    tc.assert_eq(cnt.setup, cnt.teardown)
    tc.assert_eq(cnt.setup, cnt.test)


def test_success(tc: TestContext):
    global cnt
    cnt.test += 1
    print("success")
    tc.dbg("logtest")
    tc.assert_(True)

    tc.assert_eq(1, cnt.suite_setup)
    tc.assert_eq(0, cnt.suite_teardown)
    tc.assert_eq(cnt.setup - 1, cnt.teardown)
    tc.assert_eq(cnt.setup, cnt.test)

def test_fail(tc: TestContext):
    global cnt
    cnt.test += 1
    print("fail")
    tc.assert_(False)

def test_error(tc: TestContext):
    global cnt
    cnt.test += 1

    tc.assert_eq(1, cnt.suite_setup)
    tc.assert_eq(0, cnt.suite_teardown)
    tc.assert_eq(cnt.setup - 1, cnt.teardown)
    tc.assert_eq(cnt.setup, cnt.test)

    print("error")
    raise Exception("test_error")

def test_skip(tc: TestContext):
    global cnt
    cnt.test += 1

    tc.assert_eq(1, cnt.suite_setup)
    tc.assert_eq(0, cnt.suite_teardown)
    tc.assert_eq(cnt.setup - 1, cnt.teardown)
    tc.assert_eq(cnt.setup, cnt.test)

    print("skip")
    tc.skip()

if __name__ == '__main__':
    run_testsuite(locals())
