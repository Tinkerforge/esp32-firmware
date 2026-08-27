#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import subprocess
import os

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

from _common import TEST_DIR


def run(tc: TestContext, script: str, timeout: float, env=None):
    tc.set_test_timeout(timeout)
    result = subprocess.run(
        ["bash", str(TEST_DIR / script)],
        cwd=TEST_DIR,
        capture_output=True,
        text=True,
        timeout=timeout - 5,
        env=env,
    )
    tc.dbg(result.stdout)
    tc.dbg(result.stderr)
    tc.assert_eq(0, result.returncode)


def test_client_hello_parser(tc: TestContext):
    run(tc, "_client_hello_parser.sh", 60)


def test_ocsp_stapling_patch(tc: TestContext):
    env = os.environ.copy()
    env["PORT"] = str(tc.find_free_port(18443))
    run(tc, "_ocsp_stapling_patch.sh", 600, env)


if __name__ == "__main__":
    run_testsuite(locals())
