#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import contextlib
from pathlib import Path
import ssl
import subprocess
import tempfile
import time

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.src.modules.iso15118.tests._common import CSMSSim
from software.test_runner.test_context import TestContext, run_testsuite


IDENTITY = "warp4-certificate-capacity-test"
PASSWORD = "certificate-capacity-password"
LIMITS = {
    "V2GRootCertificate": 30,
    "OEMRootCertificate": 50,
    "MORootCertificate": 40,
}


def run(command):
    return subprocess.run(command, check=True, capture_output=True, text=True)


def make_server_pki(directory: Path, server_ip: str):
    ca_key = directory / "server-ca-key.pem"
    ca_cert = directory / "server-ca.pem"
    server_key = directory / "server-key.pem"
    server_csr = directory / "server.csr"
    server_cert = directory / "server.pem"
    extensions = directory / "server-ext.cnf"
    run([
        "openssl", "req", "-x509", "-newkey", "ec",
        "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
        "-keyout", ca_key, "-out", ca_cert, "-days", "365",
        "-subj", "/CN=WARP4 certificate capacity CSMS CA",
        "-addext", "basicConstraints=critical,CA:TRUE",
    ])
    run([
        "openssl", "req", "-newkey", "ec",
        "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
        "-keyout", server_key, "-out", server_csr,
        "-subj", "/CN=warp4-certificate-capacity-csms",
    ])
    extensions.write_text(f"subjectAltName=IP:{server_ip}\n")
    run([
        "openssl", "x509", "-req", "-in", server_csr,
        "-CA", ca_cert, "-CAkey", ca_key, "-CAcreateserial",
        "-out", server_cert, "-days", "365", "-sha256",
        "-extfile", extensions,
    ])
    return ca_cert, server_cert, server_key


def make_root(directory: Path, index: int):
    key = directory / f"root-{index}.key"
    cert = directory / f"root-{index}.pem"
    run([
        "openssl", "req", "-x509", "-newkey", "ec",
        "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
        "-keyout", key, "-out", cert, "-days", "365",
        "-subj", f"/CN=WARP4 capacity root {index}",
        "-addext", "basicConstraints=critical,CA:TRUE",
    ])
    return cert.read_text()


def certificate_entries(csms: CSMSSim):
    result = csms.call("GetVariables", {"getVariableData": [{
        "component": {"name": "SecurityCtrlr"},
        "variable": {"name": "CertificateEntries"},
    }]})["getVariableResult"][0]
    assert result["attributeStatus"] == "Accepted"
    return int(result["attributeValue"])


def test_root_certificate_capacity_boundaries(tc: TestContext):
    tc.set_test_timeout(1200)
    if not tc.device_type().is_warp(4):
        tc.skip("Certificate capacity test requires a WARP4")

    saved_ocpp = tc.api("ocpp/config")
    temporary_cert_id = None
    csms = None
    cleanup_errors = []
    with tempfile.TemporaryDirectory(prefix="warp4-certificate-capacity-") as tmp:
        directory = Path(tmp)
        local_ip = tc.get_local_ip()
        ca_cert, server_cert, server_key = make_server_pki(directory, local_ip)
        try:
            disabled = dict(saved_ocpp)
            disabled["enable"] = False
            tc.api("ocpp/config_update", disabled, timeout=15)
            time.sleep(1)
            tc.api("ocpp/reset", None, timeout=15)

            used = {certificate["id"] for certificate in tc.api("certs/state")["certs"]}
            temporary_cert_id = next(
                (candidate for candidate in range(7, -1, -1) if candidate not in used), None)
            if temporary_cert_id is None:
                tc.skip("Certificate capacity test needs one free TLS certificate slot")
            tc.api("certs/add", {
                "id": temporary_cert_id,
                "name": "OCPP capacity test CA",
                "cert": ca_cert.read_text(),
            })

            context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
            context.load_cert_chain(server_cert, server_key)
            csms = CSMSSim(
                ssl_context=context,
                expected_basic_auth=(IDENTITY, PASSWORD),
            )
            config = dict(saved_ocpp)
            config.update({
                "enable": True,
                "protocol": 1,
                "url": f"wss://{local_ip}:{csms.port}",
                "identity": IDENTITY,
                "enable_auth": True,
                "pass": PASSWORD,
                "cert_id": temporary_cert_id,
            })
            tc.api("ocpp/config_update", config, timeout=15)
            if not csms.connected.wait(timeout=60):
                raise TimeoutError("WARP4 did not connect to the capacity-test CSMS")
            tc.wait_for(lambda: tc.assert_true(tc.api("ocpp/state/connected")), timeout=15)

            roots = [make_root(directory, index) for index in range(sum(LIMITS.values()) + len(LIMITS))]
            root_index = 0
            expected_count = 0
            for certificate_type, limit in LIMITS.items():
                first = roots[root_index]
                for _ in range(limit):
                    response = csms.call("InstallCertificate", {
                        "certificateType": certificate_type,
                        "certificate": roots[root_index],
                    })
                    tc.assert_eq("Accepted", response["status"])
                    root_index += 1
                    expected_count += 1
                tc.assert_eq(expected_count, certificate_entries(csms))
                tc.assert_eq("Rejected", csms.call("InstallCertificate", {
                    "certificateType": certificate_type,
                    "certificate": roots[root_index],
                })["status"])
                root_index += 1
                tc.assert_eq(expected_count, certificate_entries(csms))
                tc.assert_eq("Accepted", csms.call("InstallCertificate", {
                    "certificateType": certificate_type,
                    "certificate": first,
                })["status"])
                tc.assert_eq(expected_count, certificate_entries(csms))

            tc.assert_eq(sum(LIMITS.values()), certificate_entries(csms))
            csms.connected.clear()
            tc.reboot()
            if not csms.connected.wait(timeout=60):
                raise TimeoutError("WARP4 did not reconnect after the capacity-test reboot")
            tc.assert_eq(sum(LIMITS.values()), certificate_entries(csms))
        finally:
            with contextlib.suppress(Exception):
                current = tc.api("ocpp/config")
                current["enable"] = False
                tc.api("ocpp/config_update", current, timeout=15)
                time.sleep(1)
            if csms is not None:
                with contextlib.suppress(Exception):
                    csms.stop()
            try:
                tc.api("ocpp/reset", None, timeout=15)
            except Exception as error:  # noqa: BLE001
                cleanup_errors.append(error)
            try:
                tc.api("ocpp/config_update", saved_ocpp, timeout=15)
            except Exception as error:  # noqa: BLE001
                cleanup_errors.append(error)
            if temporary_cert_id is not None:
                try:
                    tc.api("certs/remove", {"id": temporary_cert_id}, timeout=15)
                except Exception as error:  # noqa: BLE001
                    cleanup_errors.append(error)
    if cleanup_errors:
        raise cleanup_errors[0]


if __name__ == "__main__":
    run_testsuite(locals())
