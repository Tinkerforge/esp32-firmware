#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""A02.FR.06/A03.FR.06: reject non-profile SECC deliveries without losing keys."""

import base64
from datetime import datetime, timedelta, timezone
import time

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec, ed448
from cryptography.x509 import ocsp

import vehicle_validation as validation
from vehicle_validation import TestContext, fixtures, certificate_hash_data
from software.test_runner.test_context import run_testsuite

suite_setup = validation.suite_setup
suite_teardown = validation.suite_teardown
setup = validation.setup
teardown = validation.teardown


def issue(request, label, mutation=None, variant=None):
    env = validation.environment
    root, _, root_key = env.pki[label]
    curve = ec.SECP521R1() if label == "iso20" else ec.SECP256R1()
    algorithm = None if isinstance(root_key, ed448.Ed448PrivateKey) else hashes.SHA512() if label == "iso20" else hashes.SHA256()
    keys = [ed448.Ed448PrivateKey.generate() if variant == "ed448" else ec.generate_private_key(curve) for _ in range(2)]
    names = [fixtures.name("SECC installation Sub1", ["CSO"]), fixtures.name("SECC installation Sub2", ["CSO"])]
    if mutation == "weak_sub2":
        keys[1] = ec.generate_private_key(ec.SECP256R1())
    sub1 = fixtures.issue(names[0], keys[0], root.subject, root_key, ca=True, path_length=1,
                         digital=False, signing_hash=algorithm)
    opts = {}
    if mutation == "sub2_pathlen": opts["path_length"] = 1
    if mutation == "sub2_no_ocsp": opts["source"] = None
    if mutation == "sub2_eku": opts["eku"] = [fixtures.EKU.SERVER_AUTH]
    if mutation == "sub2_crl_sign": opts["crl_sign"] = True
    sub2 = fixtures.issue(names[1], keys[1], sub1.subject, keys[0],
        **(dict(ca=True, path_length=0, digital=False, signing_hash=algorithm) | opts))
    if variant in ("single_ca_zero", "single_ca_one"):
        sub2 = fixtures.issue(names[1], keys[1], root.subject, root_key,
            ca=True, path_length=0 if variant == "single_ca_zero" else 1, digital=False, signing_hash=algorithm)
    parsed = x509.load_pem_x509_csr(request["csr"].encode())
    subject = parsed.subject
    if variant == "role_suffix":
        subject = x509.Name([x509.NameAttribute(a.oid, "OperatorCSO" if a.oid == fixtures.NameOID.DOMAIN_COMPONENT else a.value) for a in subject])
    if mutation in ("wrong_role", "wrong_cn", "no_country", "no_organization"):
        fields = []
        for attr in subject:
            if mutation == "no_country" and attr.oid == fixtures.NameOID.COUNTRY_NAME: continue
            if mutation == "no_organization" and attr.oid == fixtures.NameOID.ORGANIZATION_NAME: continue
            value = "MSP" if mutation == "wrong_role" and attr.oid == fixtures.NameOID.DOMAIN_COMPONENT else \
                    "OTHER-STATION" if mutation == "wrong_cn" and attr.oid == fixtures.NameOID.COMMON_NAME else attr.value
            fields.append(x509.NameAttribute(attr.oid, value))
        subject = x509.Name(fields)
    now = datetime.now(timezone.utc).replace(microsecond=0)
    builder = (x509.CertificateBuilder().subject_name(subject).issuer_name(sub2.subject)
        .public_key(parsed.public_key()).serial_number(x509.random_serial_number())
        .not_valid_before(now - timedelta(seconds=5)).not_valid_after(now + timedelta(days=45)))
    extensions = [
        ("bc", x509.BasicConstraints(mutation == "ca_leaf", None), mutation != "noncritical_bc"),
        ("ku", x509.KeyUsage(True, False, False, False, mutation != "no_agreement" and variant != "ed448", False,
                             mutation == "leaf_crl_sign", False, False), mutation != "noncritical_ku"),
        ("ski", x509.SubjectKeyIdentifier((lambda digest: bytes([(digest[-8] & 15) | 0x40]) + digest[-7:])(
            x509.SubjectKeyIdentifier.from_public_key(parsed.public_key()).digest)) if variant == "short_ski" else
            x509.SubjectKeyIdentifier.from_public_key(parsed.public_key()), mutation == "critical_ski"),
        ("aki", x509.AuthorityKeyIdentifier(b"\x42" * 8 if mutation == "wrong_aki" else
            sub2.extensions.get_extension_for_class(x509.SubjectKeyIdentifier).value.digest, None, None), False),
        ("eku", x509.ExtendedKeyUsage([fixtures.EKU.CLIENT_AUTH if mutation == "wrong_eku" else fixtures.EKU.SERVER_AUTH]),
         mutation != "noncritical_eku"),
        ("ocsp", x509.AuthorityInformationAccess([x509.AccessDescription(fixtures.AIA.OCSP,
            x509.UniformResourceIdentifier(fixtures.URL))]), False),
    ]
    omitted = {"no_bc": "bc", "no_ku": "ku", "no_ski": "ski", "no_aki": "aki",
               "no_eku": "eku", "no_ocsp": "ocsp"}.get(mutation)
    for name, extension, critical in extensions:
        if name != omitted and not (variant == "iso2_optional_absent" and name in ("ski", "aki", "eku", "ocsp")):
            builder = builder.add_extension(extension, critical)
    leaf = builder.sign(keys[1], hashes.SHA256() if mutation == "wrong_signature" else algorithm)
    if variant in ("single_ca_zero", "single_ca_one"):
        return [leaf, sub2], [keys[1], root_key]
    return [leaf, sub2, sub1], [keys[1], keys[0], root_key]


def delivery(env, request, chain):
    return env.csms.call("CertificateSigned", {
        "certificateType": request["certificateType"], "requestId": request["requestId"],
        "certificateChain": b"".join(fixtures.pem(c) for c in chain).decode()}, timeout=120)


def exercise(tc, mutation=None, label="iso20", variant=None):
    tc.set_test_timeout(240)
    env = validation.environment
    kind = "V2G20Certificate" if label == "iso20" else "V2GCertificate"
    saved_pki = env.pki[label]
    saved_suite = env.variable("V2G20SECCLeafCryptoSuite")
    if variant == "ed448":
        key = ed448.Ed448PrivateKey.generate()
        name = fixtures.name("SECC Ed448 root", None)
        root = fixtures.issue(name, key, name, key, ca=True, path_length=2, source=None)
        tc.assert_eq("Accepted", env.csms.call("InstallCertificate", {
            "certificateType": "V2GRootCertificate", "certificate": fixtures.pem(root).decode()})["status"])
        env.pki[label] = (root, fixtures.pem(root), key)
        env.variable("V2G20SECCLeafCryptoSuite", "ed448")
    baseline = sorted(env.inventory(), key=env.identity)
    events = len(env.csms.security_events)
    tc.assert_eq("Accepted", env.csms.call("TriggerMessage", {"requestedMessage": "Sign" + kind})["status"])
    request, message = env.csms.expect("SignCertificate", timeout=60)
    env.csms.respond(message, {"status": "Accepted"})
    try:
        if mutation:
            bad, _ = issue(request, label, mutation)
            response = delivery(env, request, bad)
            env.write_json(label + "-" + mutation + ".json", response)
            tc.assert_eq("Rejected", response["status"])
            tc.assert_eq(baseline, sorted(env.inventory(), key=env.identity))
        # Corrected delivery must still match the original pending key/requestId.
        chain, keys = issue(request, label, variant=variant)
        now = datetime.now(timezone.utc)
        responses = {}
        issuers = chain[1:] + [env.pki[label][0]]
        for cert, issuer, key in zip(chain, issuers, keys):
            response = (ocsp.OCSPResponseBuilder().add_response(cert, issuer, hashes.SHA256(),
                ocsp.OCSPCertStatus.GOOD, now - timedelta(minutes=1), now + timedelta(days=1), None, None)
                .responder_id(ocsp.OCSPResponderEncoding.HASH, issuer).certificates([issuer])
                .sign(key, None if isinstance(key, ed448.Ed448PrivateKey) else hashes.SHA512() if label == "iso20" else hashes.SHA256()))
            if not (variant == "iso2_optional_absent" and cert == chain[0]):
                responses[format(cert.serial_number, "x")] = base64.b64encode(response.public_bytes(serialization.Encoding.DER)).decode()
        tc.assert_eq("Accepted", delivery(env, request, chain)["status"])
        expected = {"certificateType": "V2GCertificateChain", "certificateHashData": certificate_hash_data(chain[0], chain[1]),
                    "childCertificateHashData": [certificate_hash_data(c, i) for c, i in zip(chain[1:], issuers[1:])]}
        tc.assert_(expected in env.inventory())
        remaining = set(responses)
        while remaining:
            req, msg = env.csms.expect("GetCertificateStatus", timeout=60)
            serial = req["ocspRequestData"]["serialNumber"].lower().lstrip("0")
            value = responses.get(serial)
            env.csms.respond(msg, {"status": "Accepted", "ocspResult": value} if value else {"status": "Failed"})
            remaining.discard(serial)
        time.sleep(2)
        if label == "iso20":
            path, certificates = env.chain("secc-installation-" + (mutation or variant or "valid"))
            env.positive("corrected SECC delivery: " + (mutation or variant or "valid"), path, certificates,
                         expected_server=chain[0], group="X448" if variant == "ed448" else "secp521r1")
        else:
            with env.connect(None, tls12=True) as tls:
                tc.assert_eq(chain[0].public_bytes(serialization.Encoding.DER), tls.getpeercert(binary_form=True))
                from iso15118.shared.messages.enums import Namespace
                result = validation.vehicle.exchange(tls, "supportedAppProtocolReq",
                    {"AppProtocol": [validation.common.ISO2]}, Namespace.SAP, 0x8001)
                tc.assert_eq("OK_SuccessfulNegotiation", result["supportedAppProtocolRes"]["ResponseCode"])
            env.record("corrected ISO2 SECC delivery: " + (mutation or variant or "valid"))
        tc.assert_eq(events, len(env.csms.security_events))
    finally:
        tc.assert_eq("Accepted", env.csms.call("TriggerMessage", {"requestedMessage": "Sign" + kind})["status"])
        _, msg = env.csms.expect("SignCertificate", timeout=60)
        env.csms.respond(msg, {"status": "Rejected"})
        if variant == "ed448":
            env.pki[label] = saved_pki
            env.variable("V2G20SECCLeafCryptoSuite", saved_suite)


def test_iso20_valid_chain(tc: TestContext): exercise(tc)
def test_iso20_wrong_role(tc: TestContext): exercise(tc, "wrong_role")
def test_iso20_wrong_cn(tc: TestContext): exercise(tc, "wrong_cn")
def test_iso20_ca_leaf(tc: TestContext): exercise(tc, "ca_leaf")
def test_iso20_no_bc(tc: TestContext): exercise(tc, "no_bc")
def test_iso20_noncritical_bc(tc: TestContext): exercise(tc, "noncritical_bc")
def test_iso20_no_ku(tc: TestContext): exercise(tc, "no_ku")
def test_iso20_noncritical_ku(tc: TestContext): exercise(tc, "noncritical_ku")
def test_iso20_no_agreement(tc: TestContext): exercise(tc, "no_agreement")
def test_iso20_leaf_crl_sign(tc: TestContext): exercise(tc, "leaf_crl_sign")
def test_iso20_no_ski(tc: TestContext): exercise(tc, "no_ski")
def test_iso20_critical_ski(tc: TestContext): exercise(tc, "critical_ski")
def test_iso20_no_aki(tc: TestContext): exercise(tc, "no_aki")
def test_iso20_wrong_aki(tc: TestContext): exercise(tc, "wrong_aki")
def test_iso20_no_eku(tc: TestContext): exercise(tc, "no_eku")
def test_iso20_wrong_eku(tc: TestContext): exercise(tc, "wrong_eku")
def test_iso20_noncritical_eku(tc: TestContext): exercise(tc, "noncritical_eku")
def test_iso20_no_ocsp(tc: TestContext): exercise(tc, "no_ocsp")
def test_iso20_no_country(tc: TestContext): exercise(tc, "no_country")
def test_iso20_no_organization(tc: TestContext): exercise(tc, "no_organization")
def test_iso20_wrong_signature(tc: TestContext): exercise(tc, "wrong_signature")
def test_iso20_weak_sub2(tc: TestContext): exercise(tc, "weak_sub2")
def test_iso20_sub2_pathlen(tc: TestContext): exercise(tc, "sub2_pathlen")
def test_iso20_sub2_no_ocsp(tc: TestContext): exercise(tc, "sub2_no_ocsp")
def test_iso20_sub2_eku(tc: TestContext): exercise(tc, "sub2_eku")
def test_iso20_sub2_crl_sign(tc: TestContext): exercise(tc, "sub2_crl_sign")
def test_iso2_valid_chain(tc: TestContext): exercise(tc, label="iso2")
def test_iso2_wrong_role(tc: TestContext): exercise(tc, "wrong_role", "iso2")
def test_iso2_ca_leaf(tc: TestContext): exercise(tc, "ca_leaf", "iso2")
def test_iso20_short_ski(tc: TestContext): exercise(tc, variant="short_ski")
def test_iso20_role_suffix(tc: TestContext): exercise(tc, variant="role_suffix")
def test_iso20_single_ca_zero(tc: TestContext): exercise(tc, variant="single_ca_zero")
def test_iso20_single_ca_one(tc: TestContext): exercise(tc, variant="single_ca_one")
def test_iso2_optional_extensions_absent(tc: TestContext): exercise(tc, label="iso2", variant="iso2_optional_absent")
def test_iso20_ed448_chain(tc: TestContext): exercise(tc, variant="ed448")


if __name__ == "__main__":
    run_testsuite(locals())
