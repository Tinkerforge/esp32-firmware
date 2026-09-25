#!/usr/bin/env -S uv run --group tests --script

import subprocess
import tempfile
from pathlib import Path
from urllib.error import HTTPError

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import TestContext, run_testsuite


slot = None
key = None
certificate = None


def suite_setup(tc: TestContext):
    global slot, key, certificate
    used = {entry["id"] for entry in tc.api("certs/state")["certs"]}
    slot = next((candidate for candidate in range(7, -1, -1) if candidate not in used), None)
    if slot is None:
        tc.skip("One free certificate slot required")
    with tempfile.TemporaryDirectory(prefix="certs-marker-") as directory:
        key_path = Path(directory) / "key.pem"
        cert_path = Path(directory) / "cert.pem"
        subprocess.run([
            "openssl", "req", "-x509", "-newkey", "ec",
            "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
            "-keyout", str(key_path), "-out", str(cert_path), "-days", "1",
            "-subj", "/CN=Certificate upload diagnostic regression",
        ], check=True, capture_output=True)
        key = key_path.read_text()
        certificate = cert_path.read_text()


def entries(tc):
    return [entry for entry in tc.api("certs/state")["certs"] if entry["id"] == slot]


def expect_error(tc, path, payload, expected, status=400):
    try:
        tc.api(path, payload)
    except HTTPError as error:
        tc.assert_eq(status, error.code)
        tc.assert_(expected in str(error))
    else:
        tc.fail(f"{path} unexpectedly accepted the request")


def assert_no_key_in_diagnostics(tc):
    # Check the encoded private material.
    markers = [line.encode() for line in key.splitlines() if not line.startswith("---")]
    for path in ("/debug_report", "/event_log", "/trace_log"):
        report = tc.http_request("GET", path)
        if any(marker in report for marker in markers):
            tc.fail(f"Private key marker leaked through {path}")


def test_private_key_upload_cleanup(tc: TestContext):
    payload = {"id": slot, "name": "Key upload regression", "cert": key}

    tc.api("certs/add", payload)
    assert_no_key_in_diagnostics(tc)

    # Previously this returned before clearing the retained command value.
    expect_error(tc, "certs/add", payload, "does already exist")
    assert_no_key_in_diagnostics(tc)
    tc.api("certs/remove", {"id": slot})
    expect_error(tc, "certs/add/id", slot, "empty certificate")
    tc.assert_eq([], entries(tc))

    # A missing-ID modify must not leave a key that a later suffix add reuses.
    expect_error(tc, "certs/modify", payload, "No cert with ID")
    assert_no_key_in_diagnostics(tc)
    expect_error(tc, "certs/add/id", slot, "empty certificate")
    tc.assert_eq([], entries(tc))

    # Exercise validation errors and successful key replacement as well.
    expect_error(tc, "certs/add", {**payload, "name": "x" * 128}, "")
    assert_no_key_in_diagnostics(tc)
    expect_error(tc, "certs/add", {**payload, "cert": key + "x" * 20000}, "", status=413)
    assert_no_key_in_diagnostics(tc)
    tc.api("certs/add", {**payload, "cert": certificate}, timeout=10)
    tc.api("certs/modify", payload, timeout=10)
    assert_no_key_in_diagnostics(tc)
    tc.api("certs/remove", {"id": slot})
    expect_error(tc, "certs/add/id", slot, "empty certificate")
    tc.assert_eq([], entries(tc))


def suite_teardown(tc: TestContext):
    if slot is not None and entries(tc):
        tc.api("certs/remove", {"id": slot})


if __name__ == "__main__":
    run_testsuite(locals())

