#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
# ruff: noqa: I001

import contextlib
from pathlib import Path
import subprocess
import tempfile
import time

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.src.modules.iso15118.tests._common import CSMSSim
from software.test_runner.test_context import TestContext, run_testsuite


IDENTITY = "warp4-trust-store-security-test"
PASSWORD = "trust-store-test-password"

saved_ocpp = None
plain = None
secure = None
secure_no_auth = None
tmpdir = None
ca_pem = None
cert_id = None
cert_added = False


def generate_ca_and_server_cert(local_ip: str):
    global tmpdir, ca_pem

    tmpdir = tempfile.TemporaryDirectory(prefix="ocpp-trust-store-")
    d = Path(tmpdir.name)
    ca_key = d / "ca-key.pem"
    ca_cert = d / "ca.pem"
    server_key = d / "server-key.pem"
    server_csr = d / "server.csr"
    server_cert = d / "server.pem"
    ext = d / "server-ext.cnf"

    subprocess.run([
        "openssl", "req", "-x509", "-newkey", "ec",
        "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
        "-keyout", ca_key, "-out", ca_cert, "-days", "365",
        "-subj", "/CN=WARP4 trust store test CA",
        "-addext", "basicConstraints=critical,CA:TRUE",
    ], check=True, capture_output=True)
    subprocess.run([
        "openssl", "req", "-newkey", "ec",
        "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
        "-keyout", server_key, "-out", server_csr,
        "-subj", "/CN=ocpp-trust-store-test",
    ], check=True, capture_output=True)
    ext.write_text(f"subjectAltName=IP:{local_ip}\n")
    subprocess.run([
        "openssl", "x509", "-req", "-in", server_csr,
        "-CA", ca_cert, "-CAkey", ca_key, "-CAcreateserial",
        "-out", server_cert, "-days", "365", "-sha256", "-extfile", ext,
    ], check=True, capture_output=True)
    ca_pem = ca_cert.read_text()
    return server_cert, server_key


def wait_for_connection(tc: TestContext, csms: CSMSSim):
    if not csms.connected.wait(timeout=60):
        raise TimeoutError("WARP4 did not connect to the OCPP 2.1 CSMS simulator")
    tc.wait_for(lambda: tc.assert_true(tc.api("ocpp/state/connected")), timeout=15)


def configure(tc: TestContext, csms: CSMSSim, url: str, *, cert_id: int, enable_auth: bool):
    assert saved_ocpp is not None
    csms.connected.clear()
    config = dict(saved_ocpp)
    config.update({
        "enable": True,
        "protocol": 1,
        "url": url,
        "identity": IDENTITY,
        "enable_auth": enable_auth,
        "pass": PASSWORD if enable_auth else None,
        "cert_id": cert_id,
    })
    tc.api("ocpp/config_update", config, timeout=5)


def certificate_entries(csms: CSMSSim):
    result = csms.call("GetVariables", {"getVariableData": [{
        "component": {"name": "SecurityCtrlr"},
        "variable": {"name": "CertificateEntries"},
    }]})["getVariableResult"][0]
    assert result["attributeStatus"] == "Accepted"
    return int(result["attributeValue"])


def install(csms: CSMSSim):
    return csms.call("InstallCertificate", {
        "certificateType": "V2GRootCertificate",
        "certificate": ca_pem,
    })


def listed(csms: CSMSSim):
    return csms.call("GetInstalledCertificateIds", {
        "certificateType": ["V2GRootCertificate"],
    })


def suite_setup(tc: TestContext):
    global saved_ocpp, plain, secure, secure_no_auth, cert_id, cert_added

    if not tc.device_type().is_warp(4):
        tc.skip("Trust-store security test requires a WARP4")

    saved_ocpp = tc.api("ocpp/config")
    disabled = dict(saved_ocpp)
    disabled["enable"] = False
    tc.api("ocpp/config_update", disabled, timeout=5)
    time.sleep(1)

    local_ip = tc.get_local_ip()
    server_cert, server_key = generate_ca_and_server_cert(local_ip)
    used_cert_ids = {cert["id"] for cert in tc.api("certs/state")["certs"]}
    cert_id = next((candidate for candidate in range(7, -1, -1) if candidate not in used_cert_ids), None)
    if cert_id is None:
        tc.skip("Trust-store security test needs one free TLS certificate slot")
    tc.api("certs/add", {
        "id": cert_id,
        "name": "OCPP trust-store security test",
        "cert": ca_pem,
    })
    cert_added = True

    plain = CSMSSim()
    secure = CSMSSim(
        certfile=str(server_cert),
        keyfile=str(server_key),
        expected_basic_auth=(IDENTITY, PASSWORD),
    )
    secure_no_auth = CSMSSim(
        certfile=str(server_cert),
        keyfile=str(server_key),
    )


def suite_teardown(tc: TestContext):
    errors = []

    if saved_ocpp is not None:
        with contextlib.suppress(Exception):
            current = tc.api("ocpp/config")
            current["enable"] = False
            tc.api("ocpp/config_update", current, timeout=5)
            time.sleep(1)
        try:
            tc.api("ocpp/config_update", saved_ocpp, timeout=5)
        except Exception as e:  # noqa: BLE001
            errors.append(e)

    for csms in (plain, secure, secure_no_auth):
        if csms is not None:
            try:
                csms.stop()
            except Exception as e:  # noqa: BLE001
                errors.append(e)

    if cert_added:
        try:
            tc.api("certs/remove", {"id": cert_id})
        except Exception as e:  # noqa: BLE001
            errors.append(e)

    if tmpdir is not None:
        tmpdir.cleanup()

    if errors:
        raise errors[0]


def test_trust_store_changes_require_security_profile_2(tc: TestContext):
    tc.set_test_timeout(180)
    assert plain is not None and secure is not None and secure_no_auth is not None
    assert cert_id is not None

    configure(tc, plain, f"ws://{tc.get_local_ip()}:{plain.port}", cert_id=-1, enable_auth=False)
    wait_for_connection(tc, plain)
    baseline = certificate_entries(plain)
    tc.assert_eq("Rejected", install(plain)["status"])
    tc.assert_eq(baseline, certificate_entries(plain))
    tc.assert_eq("NotFound", listed(plain)["status"])

    configure(tc, secure_no_auth, f"wss://{tc.get_local_ip()}:{secure_no_auth.port}", cert_id=cert_id, enable_auth=False)
    wait_for_connection(tc, secure_no_auth)
    tc.assert_eq("Rejected", install(secure_no_auth)["status"])
    tc.assert_eq(baseline, certificate_entries(secure_no_auth))
    tc.assert_eq("NotFound", listed(secure_no_auth)["status"])

    configure(tc, secure, f"wss://{tc.get_local_ip()}:{secure.port}", cert_id=cert_id, enable_auth=True)
    wait_for_connection(tc, secure)
    tc.assert_(secure.authorization is not None)
    tc.assert_eq("Accepted", install(secure)["status"])
    tc.assert_eq(baseline + 1, certificate_entries(secure))
    installed = listed(secure)
    tc.assert_eq("Accepted", installed["status"])
    hash_data = installed["certificateHashDataChain"][0]["certificateHashData"]

    configure(tc, plain, f"ws://{tc.get_local_ip()}:{plain.port}", cert_id=-1, enable_auth=False)
    wait_for_connection(tc, plain)
    tc.assert_eq("Failed", plain.call("DeleteCertificate", {
        "certificateHashData": hash_data,
    })["status"])
    tc.assert_eq(baseline + 1, certificate_entries(plain))
    tc.assert_eq("Accepted", listed(plain)["status"])

    configure(tc, secure, f"wss://{tc.get_local_ip()}:{secure.port}", cert_id=cert_id, enable_auth=True)
    wait_for_connection(tc, secure)
    tc.assert_eq("Accepted", secure.call("DeleteCertificate", {
        "certificateHashData": hash_data,
    })["status"])
    tc.assert_eq(baseline, certificate_entries(secure))
    tc.assert_eq("NotFound", listed(secure)["status"])


if __name__ == "__main__":
    run_testsuite(locals())
