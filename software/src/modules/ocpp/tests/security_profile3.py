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


IDENTITY = "warp4-profile3-cipher-test"
PASSWORD = "profile3-bootstrap-password"
SUITES = (
    "ECDHE-ECDSA-AES128-GCM-SHA256",
    "ECDHE-ECDSA-AES256-GCM-SHA384",
)


def run(command):
    return subprocess.run(command, check=True, capture_output=True, text=True)


def make_pki(directory: Path, server_ip: str):
    ca_key = directory / "ca-key.pem"
    ca_cert = directory / "ca.pem"
    server_key = directory / "server-key.pem"
    server_csr = directory / "server.csr"
    server_cert = directory / "server.pem"
    server_ext = directory / "server-ext.cnf"

    run([
        "openssl", "req", "-x509", "-newkey", "ec",
        "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
        "-keyout", str(ca_key), "-out", str(ca_cert), "-days", "365",
        "-subj", "/CN=WARP4 Profile 3 Test CA",
        "-addext", "basicConstraints=critical,CA:TRUE",
        "-addext", "keyUsage=critical,keyCertSign,cRLSign",
    ])
    run([
        "openssl", "req", "-newkey", "ec",
        "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
        "-keyout", str(server_key), "-out", str(server_csr),
        "-subj", "/CN=warp4-profile3-test-csms",
    ])
    server_ext.write_text(
        "basicConstraints=critical,CA:false\n"
        "keyUsage=critical,digitalSignature\n"
        "extendedKeyUsage=serverAuth\n"
        f"subjectAltName=IP:{server_ip}\n"
    )
    run([
        "openssl", "x509", "-req", "-in", str(server_csr),
        "-CA", str(ca_cert), "-CAkey", str(ca_key), "-CAcreateserial",
        "-out", str(server_cert), "-days", "365", "-sha256",
        "-extfile", str(server_ext),
    ])
    return ca_key, ca_cert, server_key, server_cert


def sign_csr(directory: Path, csr_pem: str, ca_key: Path, ca_cert: Path):
    csr = directory / "charging-station.csr"
    certificate = directory / "charging-station.pem"
    certificate_der = directory / "charging-station.der"
    extensions = directory / "client-ext.cnf"
    csr.write_text(csr_pem)
    extensions.write_text(
        "basicConstraints=critical,CA:false\n"
        "keyUsage=critical,digitalSignature\n"
        "extendedKeyUsage=clientAuth\n"
        "subjectKeyIdentifier=hash\n"
        "authorityKeyIdentifier=keyid,issuer\n"
    )
    run([
        "openssl", "x509", "-req", "-in", str(csr),
        "-CA", str(ca_cert), "-CAkey", str(ca_key), "-CAcreateserial",
        "-out", str(certificate), "-days", "365", "-sha256",
        "-extfile", str(extensions),
    ])
    run([
        "openssl", "x509", "-in", str(certificate),
        "-outform", "DER", "-out", str(certificate_der),
    ])
    return certificate.read_text(), certificate_der.read_bytes()


def tls_context(server_cert: Path, server_key: Path, ca_cert: Path,
                cipher: str | None, require_client: bool):
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.minimum_version = ssl.TLSVersion.TLSv1_2
    context.maximum_version = ssl.TLSVersion.TLSv1_2
    context.load_cert_chain(server_cert, server_key)
    context.load_verify_locations(cafile=str(ca_cert))
    context.verify_mode = ssl.CERT_REQUIRED if require_client else ssl.CERT_OPTIONAL
    context.options |= ssl.OP_NO_COMPRESSION
    if cipher is not None:
        context.set_ciphers(cipher)
    return context


def set_profile(csms: CSMSSim, slot: int, url: str):
    return csms.call("SetNetworkProfile", {
        "configurationSlot": slot,
        "connectionData": {
            "ocppVersion": "OCPP21",
            "ocppTransport": "JSON",
            "ocppInterface": "Wired0",
            "messageTimeout": 30,
            "ocppCsmsUrl": url,
            "securityProfile": 3,
            "identity": IDENTITY,
        },
    })


def set_priority(csms: CSMSSim, slot: int):
    result = csms.call("SetVariables", {"setVariableData": [{
        "component": {"name": "OCPPCommCtrlr"},
        "variable": {"name": "NetworkConfigurationPriority"},
        "attributeValue": str(slot),
    }]})
    return result["setVariableResult"][0]["attributeStatus"]


def assert_profile3_connection(tc: TestContext, csms: CSMSSim, observation,
                               expected_cipher: str, expected_certificate: bytes):
    tc.assert_eq("TLSv1.2", observation["version"])
    tc.assert_eq(expected_cipher, observation["cipher"][0])
    tc.assert_(observation["compression"] is None)
    tc.assert_eq(expected_certificate, observation["peer_certificate_der"])
    tc.assert_eq("ocpp2.1", observation["subprotocol"])

    variables = csms.call("GetVariables", {"getVariableData": [{
        "component": {"name": "SecurityCtrlr"},
        "variable": {"name": "SecurityProfile"},
    }]})["getVariableResult"][0]
    tc.assert_eq("Accepted", variables["attributeStatus"])
    tc.assert_eq("3", variables["attributeValue"])


def reboot_and_assert_boot(tc: TestContext, csms: CSMSSim, expected_cipher: str,
                           expected_certificate: bytes):
    connection_index = len(csms.tls_connections)
    tc.reboot()
    observation = csms.wait_for_tls_connection(after=connection_index, timeout=60)
    assert_profile3_connection(tc, csms, observation, expected_cipher, expected_certificate)
    boot, message_id = csms.expect("BootNotification", timeout=30)
    tc.assert_eq("PowerUp", boot["reason"])
    tc.assert_(isinstance(boot["chargingStation"]["model"], str))
    tc.assert_(isinstance(boot["chargingStation"]["vendorName"], str))
    csms.respond(message_id, {
        "currentTime": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "interval": 300,
        "status": "Accepted",
    })


def test_profile3_required_tls12_suites(tc: TestContext):
    tc.set_test_timeout(600)
    if not tc.device_type().is_warp(4):
        tc.skip("OCPP Profile 3 target test requires a WARP4")

    saved_ocpp = tc.api("ocpp/config")
    temporary_cert_id = None
    servers = []
    cleanup_errors = []
    with tempfile.TemporaryDirectory(prefix="warp4-profile3-") as tmp:
        directory = Path(tmp)
        local_ip = tc.get_local_ip()
        ca_key, ca_cert, server_key, server_cert = make_pki(directory, local_ip)
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
                tc.skip("OCPP Profile 3 target test needs one free certificate slot")
            tc.api("certs/add", {
                "id": temporary_cert_id,
                "name": "OCPP Profile 3 test CA",
                "cert": ca_cert.read_text(),
            })

            bootstrap = CSMSSim(
                ssl_context=tls_context(server_cert, server_key, ca_cert, None, False),
                expected_basic_auth=(IDENTITY, PASSWORD),
                interactive=("BootNotification", "SignCertificate"),
            )
            servers.append(bootstrap)
            config = dict(saved_ocpp)
            config.update({
                "enable": True,
                "protocol": 1,
                "url": f"wss://{local_ip}:{bootstrap.port}",
                "identity": IDENTITY,
                "enable_auth": True,
                "pass": PASSWORD,
                "cert_id": temporary_cert_id,
            })
            tc.api("ocpp/config_update", config, timeout=15)
            first = bootstrap.wait_for_tls_connection(timeout=60)
            tc.assert_(first["peer_certificate_der"] is None)
            boot, boot_message_id = bootstrap.expect("BootNotification", timeout=30)
            tc.assert_(isinstance(boot["chargingStation"]["model"], str))
            tc.assert_(isinstance(boot["chargingStation"]["vendorName"], str))
            bootstrap.respond(boot_message_id, {
                "currentTime": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
                "interval": 300,
                "status": "Accepted",
            })
            time.sleep(1)

            tc.assert_eq("Accepted", bootstrap.call("InstallCertificate", {
                "certificateType": "CSMSRootCertificate",
                "certificate": ca_cert.read_text(),
            })["status"])
            tc.assert_eq("Accepted", bootstrap.call("TriggerMessage", {
                "requestedMessage": "SignChargingStationCertificate",
            })["status"])
            sign_request, sign_message_id = bootstrap.expect("SignCertificate", timeout=60)
            bootstrap.respond(sign_message_id, {"status": "Accepted"})
            tc.assert_eq("ChargingStationCertificate", sign_request["certificateType"])
            certificate_pem, certificate_der = sign_csr(
                directory, sign_request["csr"], ca_key, ca_cert)
            tc.assert_eq("Accepted", bootstrap.call("CertificateSigned", {
                "certificateChain": certificate_pem,
                "certificateType": "ChargingStationCertificate",
                "requestId": sign_request["requestId"],
            }, timeout=90)["status"])
            reconnected = bootstrap.wait_for_tls_connection(after=1, timeout=60)
            tc.assert_eq(certificate_der, reconnected["peer_certificate_der"])

            profile_servers = []
            for cipher in SUITES:
                server = CSMSSim(
                    ssl_context=tls_context(server_cert, server_key, ca_cert, cipher, True),
                    interactive=("BootNotification",),
                )
                servers.append(server)
                profile_servers.append(server)

            current = bootstrap
            for slot, (cipher, server) in enumerate(zip(SUITES, profile_servers), 1):
                tc.assert_eq("Accepted", set_profile(
                    current, slot, f"wss://{local_ip}:{server.port}")["status"])
                tc.assert_eq("Accepted", set_priority(current, slot))
                observation = server.wait_for_tls_connection(timeout=60)
                assert_profile3_connection(tc, server, observation, cipher, certificate_der)
                reboot_and_assert_boot(tc, server, cipher, certificate_der)
                current = server
        finally:
            with contextlib.suppress(Exception):
                current_config = tc.api("ocpp/config")
                current_config["enable"] = False
                tc.api("ocpp/config_update", current_config, timeout=15)
                time.sleep(1)
            for server in reversed(servers):
                with contextlib.suppress(Exception):
                    server.stop()
            try:
                tc.api("ocpp/reset", None, timeout=15)
            except Exception as error:
                cleanup_errors.append(error)
            try:
                tc.api("ocpp/config_update", saved_ocpp, timeout=15)
            except Exception as error:
                cleanup_errors.append(error)
            if temporary_cert_id is not None:
                try:
                    tc.api("certs/remove", {"id": temporary_cert_id}, timeout=15)
                except Exception as error:
                    cleanup_errors.append(error)
    if cleanup_errors:
        raise cleanup_errors[0]


if __name__ == "__main__":
    run_testsuite(locals())
