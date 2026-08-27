#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

from _common import EVSim, IsoTestEnvironment


environment = None


def suite_setup(tc: TestContext):
    global environment
    environment = IsoTestEnvironment(tc)
    environment.start()


def setup(tc: TestContext):
    assert environment is not None
    environment.reset_session()


def suite_teardown(tc: TestContext):
    if environment is not None:
        environment.stop()


def test_iso20_ac_session(tc: TestContext):
    tc.set_test_timeout(180)
    assert environment is not None
    EVSim(environment.iface, charge_loop_cycles=1).run(timeout=150)
    state = tc.api("iso15118/state_common")
    tc.assert_eq("urn:iso:std:iso:15118:-20:AC", state["protocol"])
    tc.assert_eq(["urn:iso:std:iso:15118:-20:AC"], state["supported_protocols"])


if __name__ == "__main__":
    run_testsuite(locals())
