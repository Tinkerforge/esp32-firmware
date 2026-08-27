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


IDENTITY = "warp4-tls-security-events-test"
PASSWORD = "tls-security-events-password"

saved_ocpp = None
tmpdir = None
cert_id = None
cert_added = False
local_ip = None
server_cert = None
server_key = None
bogus_server_cert = None
bogus_server_key = None


def generate_certificates(ip: str):
    global tmpdir, server_cert, server_key

    tmpdir = tempfile.TemporaryDirectory(prefix="ocpp-tls-security-events-")
    directory = Path(tmpdir.name)

    def make_ca(name: str):
        key = directory / f"{name}-key.pem"
        cert = directory / f"{name}.pem"
        subprocess.run([
            "openssl", "req", "-x509", "-newkey", "ec",
            "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
            "-keyout", key, "-out", cert, "-days", "365",
            "-subj", f"/CN={name}",
            "-addext", "basicConstraints=critical,CA:TRUE",
        ], check=True, capture_output=True)
        return key, cert

    def make_server(name: str, ca_key: Path, ca_cert: Path):
        key = directory / f"{name}-key.pem"
        csr = directory / f"{name}.csr"
        cert = directory / f"{name}.pem"
        ext = directory / f"{name}-ext.cnf"
        ext.write_text(f"subjectAltName=IP:{ip}\n")
        subprocess.run([
            "openssl", "req", "-newkey", "ec",
            "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
            "-keyout", key, "-out", csr, "-subj", f"/CN={name}",
        ], check=True, capture_output=True)
        subprocess.run([
            "openssl", "x509", "-req", "-in", csr,
            "-CA", ca_cert, "-CAkey", ca_key, "-CAcreateserial",
            "-out", cert, "-days", "365", "-sha256", "-extfile", ext,
        ], check=True, capture_output=True)
        return cert, key

    ca_key, ca_cert = make_ca("WARP4 TLS security event test CA")
    bogus_ca_key, bogus_ca_cert = make_ca("WARP4 untrusted test CA")
    server_cert, server_key = make_server("ocpp-tls-security-events", ca_key, ca_cert)
    bogus_cert, bogus_key = make_server("ocpp-untrusted", bogus_ca_key, bogus_ca_cert)
    return ca_cert, bogus_cert, bogus_key


def configure(tc: TestContext, url: str):
    assert saved_ocpp is not None
    config = dict(saved_ocpp)
    config.update({
        "enable": True,
        "protocol": 1,
        "url": url,
        "identity": IDENTITY,
        "enable_auth": True,
        "pass": PASSWORD,
        "cert_id": cert_id,
    })
    tc.api("ocpp/config_update", config, timeout=5)


def reconnect_and_assert(tc: TestContext, event_type: str, port: int):
    assert server_cert is not None
    assert server_key is not None
    csms = CSMSSim(port=port, certfile=str(server_cert), keyfile=str(server_key))
    if not csms.connected.wait(timeout=60):
        csms.stop()
        raise TimeoutError("WARP4 did not reconnect to the valid CSMS")
    try:
        tc.wait_for(
            lambda: tc.assert_eq(
                1,
                sum(event.get("type") == event_type for event in csms.security_events),
            ),
            timeout=30,
        )
    finally:
        csms.stop()


def run_openssl_failure(tc: TestContext, event_type: str, options: list[str]):
    assert local_ip is not None
    assert server_cert is not None
    assert server_key is not None

    port = tc.find_free_port(19443)
    process = subprocess.Popen(
        ["openssl", "s_server", "-accept", str(port), "-cert", server_cert,
         "-key", server_key, "-quiet", *options],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
        text=True,
    )
    try:
        time.sleep(1)
        if process.poll() is not None:
            raise RuntimeError(f"openssl s_server failed: {process.stderr.read()}")
        configure(tc, f"wss://{local_ip}:{port}")
        # Cover the automatic 10 second reconnect and prove failure-streak
        # deduplication before the queued event is delivered.
        time.sleep(12)
    finally:
        process.terminate()
        with contextlib.suppress(subprocess.TimeoutExpired):
            process.wait(timeout=5)
        if process.poll() is None:
            process.kill()
            process.wait(timeout=5)
    reconnect_and_assert(tc, event_type, port)


def suite_setup(tc: TestContext):
    global saved_ocpp, cert_id, cert_added, local_ip, bogus_server_cert, bogus_server_key

    if not tc.device_type().is_warp(4):
        tc.skip("TLS security event test requires a WARP4")

    saved_ocpp = tc.api("ocpp/config")
    disabled = dict(saved_ocpp)
    disabled["enable"] = False
    tc.api("ocpp/config_update", disabled, timeout=5)
    time.sleep(1)

    local_ip = tc.get_local_ip()
    ca_cert, bogus_cert, bogus_key = generate_certificates(local_ip)
    used_cert_ids = {cert["id"] for cert in tc.api("certs/state")["certs"]}
    cert_id = next((candidate for candidate in range(7, -1, -1) if candidate not in used_cert_ids), None)
    if cert_id is None:
        tc.skip("TLS security event test needs one free certificate slot")
    tc.api("certs/add", {
        "id": cert_id,
        "name": "OCPP TLS security event test",
        "cert": ca_cert.read_text(),
    })
    cert_added = True

    bogus_server_cert = bogus_cert
    bogus_server_key = bogus_key


def suite_teardown(tc: TestContext):
    errors = []

    def api_with_retry(path, payload):
        last_error = None
        for _ in range(3):
            try:
                return tc.api(path, payload, timeout=15)
            except Exception as e:  # noqa: BLE001
                last_error = e
                time.sleep(1)
        raise last_error

    if saved_ocpp is not None:
        with contextlib.suppress(Exception):
            current = tc.api("ocpp/config")
            current["enable"] = False
            api_with_retry("ocpp/config_update", current)
            time.sleep(1)
        try:
            api_with_retry("ocpp/config_update", saved_ocpp)
        except Exception as e:  # noqa: BLE001
            errors.append(e)
    if cert_added:
        try:
            api_with_retry("certs/remove", {"id": cert_id})
        except Exception as e:  # noqa: BLE001
            errors.append(e)
    if tmpdir is not None:
        tmpdir.cleanup()
    if errors:
        raise errors[0]


def test_invalid_csms_certificate(tc: TestContext):
    assert local_ip is not None
    assert bogus_server_cert is not None
    assert bogus_server_key is not None
    port = tc.find_free_port(19443)
    invalid_csms = CSMSSim(port=port, certfile=str(bogus_server_cert), keyfile=str(bogus_server_key))
    try:
        configure(tc, f"wss://{local_ip}:{port}")
        time.sleep(12)
    finally:
        invalid_csms.stop()
    reconnect_and_assert(tc, "InvalidCsmsCertificate", port)


def test_invalid_tls_version(tc: TestContext):
    run_openssl_failure(tc, "InvalidTLSVersion", ["-tls1_1", "-cipher", "ALL:@SECLEVEL=0"])


def test_invalid_tls_cipher_suite(tc: TestContext):
    run_openssl_failure(tc, "InvalidTLSCipherSuite", ["-tls1_2", "-cipher", "AES128-SHA256:@SECLEVEL=0"])


if __name__ == "__main__":
    run_testsuite(locals())
