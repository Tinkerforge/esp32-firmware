#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import subprocess
import sys

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

from _common import IsoTestEnvironment, TEST_DIR


environment = None
saved_ocpp = None


def suite_setup(tc: TestContext):
    global environment, saved_ocpp
    environment = IsoTestEnvironment(tc)
    saved_ocpp = tc.api("ocpp/config")


def setup(tc: TestContext):
    assert environment is not None
    environment.start()


def teardown(tc: TestContext):
    errors = []
    if saved_ocpp is not None:
        try:
            disabled = dict(saved_ocpp)
            disabled["enable"] = False
            tc.api("ocpp/config_update", disabled, timeout=5)
            tc.api("ocpp/reset", None, timeout=5)
            tc.api("ocpp/config_update", saved_ocpp, timeout=5)
        except Exception as e:
            errors.append(e)
    if environment is not None:
        try:
            environment.stop()
        except Exception as e:
            errors.append(e)
    if errors:
        raise errors[0]


def run(tc: TestContext, script: str, timeout: float):
    tc.set_test_timeout(timeout)
    port = tc.find_free_port(9500)
    result = subprocess.run(
        [
            sys.executable,
            str(TEST_DIR / script),
            "--charger", environment.host,
            "--iface", environment.iface,
            "--port", str(port),
        ],
        cwd=TEST_DIR,
        capture_output=True,
        text=True,
        timeout=timeout - 10,
    )
    tc.dbg(result.stdout)
    tc.dbg(result.stderr)
    if result.returncode != 0:
        print(result.stdout)
        print(result.stderr, file=sys.stderr)
    tc.assert_eq(0, result.returncode)


def test_ocsp_gating_and_stapling(tc: TestContext):
    run(tc, "_ocsp_gating.py", 600)


def test_ed448_tls(tc: TestContext):
    run(tc, "_ed448_tls.py", 900)


def test_vehicle_chain_status(tc: TestContext):
    run(tc, "_vehicle_chain.py", 900)


def test_v2g2_871_iso2_chain_selection(tc: TestContext):
    run(tc, "_iso2_chain_selection.py", 900)


def test_plug_and_charge(tc: TestContext):
    global environment
    assert environment is not None
    environment.stop()
    tc.reboot()
    environment = IsoTestEnvironment(tc)
    environment.start()
    run(tc, "_pnc.py", 1200)


if __name__ == "__main__":
    run_testsuite(locals())
