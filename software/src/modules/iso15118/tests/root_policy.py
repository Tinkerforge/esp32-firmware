#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""HUB20-21-007:

Optional ISO15118_PRODUCTIVE_ROOTS points to a directory of public PEM root
certificates downloaded from https://www.hubject.com/download-pki. Their
issuerKeyHash is checked independently before installation. No CA keys needed.
"""
import os
from pathlib import Path
import time

from cryptography import x509
from cryptography.x509.oid import NameOID

import vehicle_validation as validation
from vehicle_validation import TestContext, common, vehicle, fixtures, certificate_hash_data
from software.test_runner.test_context import run_testsuite

PRODUCTIVE_KEYS = {
    "iso2_eu": "f3a72a6da6ce3f524cddb0935c5814266318e68b57afa36a1934fedfdb906402",
    "iso2_us": "b2fd0b3417aa6f596b2b219915e4f6764e14b2502221771177149500a903f7c7",
    "iso20": "0333d0fe04eb2d8f598edd0d0e01da3542980d890fcefeaada8eacbb61ff9e62",
}

suite_setup = validation.suite_setup
suite_teardown = validation.suite_teardown
teardown = validation.teardown


def setup(tc: TestContext):
    validation.setup(tc)
    env = validation.environment
    # The reboot case clears the device's in-memory SECC OCSP cache. Answer
    # queued M06 refreshes before another public-mode handshake.
    while True:
        try:
            req, mid = env.csms.expect("GetCertificateStatus", timeout=2)
        except TimeoutError:
            break
        serial = req["ocspRequestData"]["serialNumber"].lower().lstrip("0")
        der = env.statuses.get(serial)
        env.csms.respond(mid, {"status": "Accepted", "ocspResult": der} if der else {"status": "Failed"})


def test_environment_transition_and_reboot(tc: TestContext):
    validation.test_environment_transition_and_reboot(tc)


def advertised_authorities(tc, env, path):
    """Observe decrypted TLS 1.3 CertificateRequest, not a debug API."""
    authorities = []

    def message(tls, direction, version, content_type, message_type, data):
        if direction != "read" or int(content_type) != 22 or int(message_type) != 13:
            return
        data = bytes(data)
        tc.assert_eq(len(data) - 4, int.from_bytes(data[1:4], "big"))
        pos = 5 + data[4]  # handshake header and certificate_request_context
        size = int.from_bytes(data[pos:pos + 2], "big")
        pos += 2
        tc.assert_eq(len(data), pos + size)
        while pos < len(data):
            kind = int.from_bytes(data[pos:pos + 2], "big")
            length = int.from_bytes(data[pos + 2:pos + 4], "big")
            body = data[pos + 4:pos + 4 + length]
            tc.assert_eq(length, len(body))
            pos += 4 + length
            if kind != 47:
                continue
            tc.assert_eq(len(body) - 2, int.from_bytes(body[:2], "big"))
            cursor = 2
            while cursor < len(body):
                length = int.from_bytes(body[cursor:cursor + 2], "big")
                cursor += 2
                name = body[cursor:cursor + length]
                tc.assert_eq(length, len(name))
                authorities.append(name)
                cursor += length

    with env.connect(path, message_callback=message) as tls:
        vehicle.sap_iso20(tls)
        try:
            req, mid = env.csms.expect("GetCertificateChainStatus", timeout=2)
        except TimeoutError:
            pass  # private non-PnC waiver or cached Good
        else:
            env.csms.respond(mid, vehicle.chain_status_response(req, ["Good"] * 3))
        session = vehicle.session_setup(tls)
        vehicle.authorization_setup(tls, session)
        tc.assert_eq("OK", vehicle.final_authorization(tls, session)["ResponseCode"])
    time.sleep(2)
    tc.assert_(len(authorities) > 0)
    return authorities


def test_private_root_remains_usable_in_both_modes(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    saved = env.variable("PrivateEnvironmentEnabled")
    before = sorted(env.inventory(), key=env.identity)
    path, _ = env.chain("private-root-policy")
    cn = env.pki["vehicle"][0].subject.get_attributes_for_oid(NameOID.COMMON_NAME)[0].value.encode()
    try:
        for mode in ("false", "true", "false"):
            env.variable("PrivateEnvironmentEnabled", mode)
            tc.assert_(any(cn in dn for dn in advertised_authorities(tc, env, path)))
            tc.assert_eq(before, sorted(env.inventory(), key=env.identity))
        env.record("private root advertised and ISO-20 authorized in both environments")
    finally:
        env.variable("PrivateEnvironmentEnabled", saved)


def productive_root(tc, expected):
    directory = os.environ.get("ISO15118_PRODUCTIVE_ROOTS")
    if not directory:
        tc.skip("Set ISO15118_PRODUCTIVE_ROOTS to public Hubject root PEM fixtures")
    for path in sorted(Path(directory).glob("*.pem")):
        cert = x509.load_pem_x509_certificate(path.read_bytes())
        if certificate_hash_data(cert, cert)["issuerKeyHash"].lower() == expected:
            tc.assert_eq(cert.subject, cert.issuer)
            return cert
    tc.skip("Required HUB20-21-007 public root certificate is absent")


def check_productive_root(tc, key, kind):
    root = productive_root(tc, key)
    tc.set_test_timeout(180)
    env = validation.environment
    saved = env.variable("PrivateEnvironmentEnabled")
    before = sorted(env.inventory(), key=env.identity)
    root_hash = certificate_hash_data(root, root)
    # Avoid replacement/deletion of any pre-existing root with this key.
    if any(e["certificateHashData"]["issuerKeyHash"].lower() == key for e in before):
        tc.skip("Productive root already installed; preserve existing inventory")
    cn = root.subject.get_attributes_for_oid(NameOID.COMMON_NAME)[0].value.encode()
    path, _ = env.chain("productive-root-policy")
    installed = False
    try:
        result = env.csms.call("InstallCertificate", {"certificateType": kind, "certificate": fixtures.pem(root).decode()})
        tc.assert_eq("Accepted", result["status"])
        installed = True
        installed_inventory = sorted(env.inventory(), key=env.identity)
        tc.assert_(any(e["certificateHashData"] == root_hash for e in installed_inventory))
        for mode, expected in (("false", True), ("true", False), ("false", True)):
            env.variable("PrivateEnvironmentEnabled", mode)
            names = advertised_authorities(tc, env, path)
            tc.assert_eq(expected, any(cn in dn for dn in names))
            tc.assert_eq(installed_inventory, sorted(env.inventory(), key=env.identity))
        env.record("productive root excluded only in private mode", {"issuerKeyHash": key, "type": kind})
    finally:
        if installed:
            tc.assert_eq("Accepted", env.csms.call("DeleteCertificate", {"certificateHashData": root_hash})["status"])
        env.variable("PrivateEnvironmentEnabled", saved)
        tc.assert_eq(before, sorted(env.inventory(), key=env.identity))


def generate_tests():
    tests = {}
    for label, key in PRODUCTIVE_KEYS.items():
        for kind in ("V2GRootCertificate", "OEMRootCertificate"):
            def test(tc, key=key, kind=kind):
                check_productive_root(tc, key, kind)
            tests[f"test_productive_{label}_{kind}"] = test
    return tests


if __name__ == "__main__":
    run_testsuite(dict(locals(), **generate_tests()))
