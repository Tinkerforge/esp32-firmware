#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""A02.FR.17/18/19: SECC renewal retries preserve active credentials.

TC_HU_SECC_ISO20_Reject_Leaf_Certificate_Installation_001.
Uses the device's actual retry settings and restores the original inventory.
"""

import base64
from datetime import datetime, timedelta, timezone
import json
import time

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509 import ocsp

import _certificate_profiles as profiles
import vehicle_validation as validation
from vehicle_validation import TestContext, certificate_hash_data, fixtures
from software.test_runner.test_context import run_testsuite

suite_teardown = validation.suite_teardown
setup = validation.setup
teardown = validation.teardown
chains = {}
ocsp_responses = {}


def csr(tc, kind):
    env = validation.environment
    tc.assert_eq("Accepted", env.csms.call("TriggerMessage", {"requestedMessage": "Sign" + kind})["status"])
    request, message = env.csms.expect("SignCertificate", timeout=60)
    tc.assert_eq(kind, request["certificateType"])
    env.csms.respond(message, {"status": "Accepted"})
    return request


def install_chain(tc, label):
    env = validation.environment
    kind = "V2G20Certificate" if label == "iso20" else "V2GCertificate"
    root, _, root_key = env.pki[label]
    algorithm = hashes.SHA512() if label == "iso20" else hashes.SHA256()
    curve = ec.SECP521R1() if label == "iso20" else ec.SECP256R1()
    now = datetime.now(timezone.utc).replace(microsecond=0)
    keys = [ec.generate_private_key(curve) for _ in range(2)]
    # The environment root uses RFC 5280 identifier method 2. Its child's AKI
    # must copy that SKI, even though these SECC intermediates use method 1.
    sub1 = fixtures.issue(profiles.name(label + " Retry Sub1"), keys[0], root.subject, root_key,
        ca=True, path_length=1, digital=False, signing_hash=algorithm, identifier_method=1,
        aki=root.extensions.get_extension_for_class(x509.SubjectKeyIdentifier).value.digest,
        source=profiles.OCSP_URL, not_before=now - timedelta(days=1), not_after=now + timedelta(days=90))
    sub2 = profiles.certificate(profiles.name(label + " Retry Sub2"), keys[1], sub1.subject, keys[0],
        x509.random_serial_number(), now - timedelta(days=1), now + timedelta(days=75), algorithm, path_length=0)
    request = csr(tc, kind)
    parsed = x509.load_pem_x509_csr(request["csr"].encode())
    tc.assert_(parsed.is_signature_valid)
    leaf = (x509.CertificateBuilder().subject_name(parsed.subject).issuer_name(sub2.subject)
        .public_key(parsed.public_key()).serial_number(x509.random_serial_number())
        .not_valid_before(now - timedelta(seconds=5)).not_valid_after(now + timedelta(days=45))
        .add_extension(x509.BasicConstraints(False, None), True)
        .add_extension(profiles.key_usage(ca=False, key_agreement=True), True)
        .add_extension(x509.SubjectKeyIdentifier.from_public_key(parsed.public_key()), False)
        .add_extension(x509.AuthorityKeyIdentifier.from_issuer_public_key(keys[1].public_key()), False)
        .add_extension(x509.ExtendedKeyUsage([fixtures.EKU.SERVER_AUTH]), True)
        .add_extension(x509.AuthorityInformationAccess([x509.AccessDescription(fixtures.AIA.OCSP,
            x509.UniformResourceIdentifier(profiles.OCSP_URL))]), False).sign(keys[1], algorithm))
    chain = [leaf, sub2, sub1]
    chain_hashes = [certificate_hash_data(cert, issuer) for cert, issuer in zip(chain, [sub2, sub1, root])]
    tc.assert_eq("Accepted", env.csms.call("CertificateSigned", {
        "certificateType": kind, "requestId": request["requestId"],
        "certificateChain": b"".join(fixtures.pem(cert) for cert in chain).decode(),
    }, timeout=120)["status"])
    responses = {}
    for cert, issuer, key, hash_data in zip(chain, [sub2, sub1, root], [keys[1], keys[0], root_key], chain_hashes):
        response = (ocsp.OCSPResponseBuilder().add_response(cert, issuer, hashes.SHA256(),
            ocsp.OCSPCertStatus.GOOD, now - timedelta(minutes=1), now + timedelta(days=1), None, None)
            .responder_id(ocsp.OCSPResponderEncoding.HASH, issuer).certificates([issuer]).sign(key, algorithm))
        responses[json.dumps(hash_data, sort_keys=True)] = base64.b64encode(response.public_bytes(serialization.Encoding.DER)).decode()
    ocsp_responses.update(responses)
    if label == "iso20":
        remaining = set(responses)
        while remaining:
            status, message = env.csms.expect("GetCertificateStatus", timeout=60)
            data = status["ocspRequestData"]
            identity = json.dumps({key: data[key] for key in chain_hashes[0]}, sort_keys=True)
            tc.assert_eq(profiles.OCSP_URL, data["responderURL"])
            env.csms.respond(message, {"status": "Accepted", "ocspResult": ocsp_responses[identity]})
            remaining.discard(identity)
    entry = {"certificateType": "V2GCertificateChain", "certificateHashData": chain_hashes[0],
             "childCertificateHashData": chain_hashes[1:]}
    tc.assert_(entry in env.inventory())
    chains[label] = leaf
    (env.work / (label + "-retry-chain.pem")).write_bytes(b"".join(fixtures.pem(cert) for cert in chain))
    env.write_json(label + "-retry-chain-inventory.json", entry)


def suite_setup(tc: TestContext):
    validation.suite_setup(tc)
    for label in ("iso2", "iso20"):
        install_chain(tc, label)
    time.sleep(2)
    # Establish credential usability before spending several minutes on retries.
    def capture(connection, direction, version, content_type, message_type, data):
        if direction == "read" and content_type == 22 and message_type == 11:
            (validation.environment.work / "iso2-server-certificate-message.bin").write_bytes(data)
    with validation.environment.connect(None, tls12=True, message_callback=capture) as tls:
        tc.assert_eq(chains["iso2"].public_bytes(serialization.Encoding.DER), tls.getpeercert(binary_form=True))
    time.sleep(2)


def test_accepted_csr_without_delivery_retries_and_preserves_chains(tc: TestContext):
    tc.set_test_timeout(600)
    env = validation.environment
    csms = env.csms
    results = csms.call("GetVariables", {"getVariableData": [
        {"component": {"name": "SecurityCtrlr"}, "variable": {"name": name}}
        for name in ("CertSigningWaitMinimum", "CertSigningRepeatTimes")
    ]})["getVariableResult"]
    tc.assert_eq(["Accepted", "Accepted"], [item["attributeStatus"] for item in results])
    minimum, doublings = [int(item["attributeValue"]) for item in results]
    # Bound test duration before starting a pending CSR; don't silently alter
    # the campaign settings to make the timing assertions easier.
    tc.assert_(minimum > 0 and 0 <= doublings <= 3)
    tc.assert_(minimum * (2 ** (doublings + 1) - 1 + 2 ** doublings) < 480)
    baseline = sorted(env.inventory(), key=env.identity)
    connections = csms.connection_count
    events = len(csms.security_events)
    log = []
    request = csr(tc, "V2G20Certificate")
    accepted = time.monotonic()
    parsed = x509.load_pem_x509_csr(request["csr"].encode())
    tc.assert_(parsed.is_signature_valid)
    tc.assert_eq(chains["iso20"].subject, parsed.subject)
    tc.assert_eq("secp521r1", parsed.public_key().curve.name)
    tc.assert_("hashRootCertificate" not in request)
    try:
        for index in range(doublings + 1):
            interval = minimum * 2 ** index
            retry, message = csms.expect("SignCertificate", timeout=interval + 10)
            elapsed = time.monotonic() - accepted
            log.append({"retry": index + 1, "expected_seconds": interval, "seconds": elapsed,
                        "request": retry})
            env.write_json("retry-timing.json", log)
            tc.assert_eq(request, retry)
            tc.assert_(interval - .5 <= elapsed <= interval + 5)
            accepted = time.monotonic()
            csms.respond(message, {"status": "Accepted"})
            tc.assert_eq(baseline, sorted(env.inventory(), key=env.identity))
        # Exhaustion is reached after the final response's backoff; observe an
        # additional minimum interval without a further request.
        try:
            extra = csms.expect("SignCertificate", timeout=minimum * 2 ** doublings + minimum + 2)
        except TimeoutError:
            extra = None
        tc.assert_eq(None, extra)
        tc.assert_eq(baseline, sorted(env.inventory(), key=env.identity))
        with env.connect(None, tls12=True) as tls:
            tc.assert_eq(chains["iso2"].public_bytes(serialization.Encoding.DER), tls.getpeercert(binary_form=True))
            from iso15118.shared.messages.enums import Namespace
            response = validation.vehicle.exchange(tls, "supportedAppProtocolReq",
                {"AppProtocol": [validation.common.ISO2]}, Namespace.SAP, 0x8001)
            tc.assert_eq("OK_SuccessfulNegotiation", response["supportedAppProtocolRes"]["ResponseCode"])
        time.sleep(2)
        path, certs = env.chain("retry-original-credential")
        env.positive("original full ISO20 chain remains usable after CSR exhaustion", path, certs,
                     expected_server=chains["iso20"])
        tc.assert_eq(connections, csms.connection_count)
        tc.assert_eq(events, len(csms.security_events))
        tc.assert_eq("Accepted", csms.call("TriggerMessage", {"requestedMessage": "SignV2G20Certificate"})["status"])
        fresh, message = csms.expect("SignCertificate", timeout=60)
        csms.respond(message, {"status": "Rejected"})
        tc.assert_(fresh["requestId"] != request["requestId"])
        tc.assert_(fresh["csr"] != request["csr"])
        tc.assert_(x509.load_pem_x509_csr(fresh["csr"].encode()).is_signature_valid)
        tc.assert_eq(baseline, sorted(env.inventory(), key=env.identity))
        env.record("SECC retry timing/exhaustion, original chains and retrigger",
                   [{key: value for key, value in item.items() if key != "request"} for item in log])
    finally:
        # Abort any pending key after assertion failures as well as success.
        tc.assert_eq("Accepted", csms.call("TriggerMessage", {"requestedMessage": "SignV2G20Certificate"})["status"])
        _, message = csms.expect("SignCertificate", timeout=60)
        csms.respond(message, {"status": "Rejected"})


if __name__ == "__main__":
    run_testsuite(locals())
