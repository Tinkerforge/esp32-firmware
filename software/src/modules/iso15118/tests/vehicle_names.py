#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""V2G20-2598/3038/3087: vehicle names and EVCCID on the device."""

from cryptography import x509
from cryptography.x509.oid import NameOID
from cryptography.x509.name import _ASN1Type

import vehicle_validation as validation
from vehicle_validation import TestContext
from software.test_runner.test_context import run_testsuite

suite_setup = validation.suite_setup
suite_teardown = validation.suite_teardown
setup = validation.setup
teardown = validation.teardown


def subject(cn=None, organization="Vehicle Test", extra=(), organization_type=None):
    attributes = []
    if organization is not None:
        attributes.append(x509.NameAttribute(NameOID.ORGANIZATION_NAME, organization, _type=organization_type))
    if cn is not None:
        attributes.append(x509.NameAttribute(NameOID.COMMON_NAME, cn, _validate=False))
    return x509.Name(attributes + [x509.NameAttribute(NameOID.DOMAIN_COMPONENT, "EV")] + list(extra))


def positive(tc, label, **options):
    tc.set_test_timeout(120)
    env = validation.environment
    path, chain = env.chain(label, **options)
    env.positive(label, path, chain)


def negative(tc, label, **options):
    tc.set_test_timeout(90)
    env = validation.environment
    path, _ = env.chain(label, **options)
    env.negative(label, path, "CERTIFICATE_UNKNOWN")


def test_evccid_minimum(tc: TestContext):
    identifier = validation.fixtures.evccid()
    tc.assert_eq(20, len(identifier))
    positive(tc, "EVCCID minimum length", leaf_name=subject(identifier))


def test_evccid_maximum(tc: TestContext):
    identifier = validation.fixtures.evccid("123456789ABCDEF" * 3 + "123456789ABCDE")
    tc.assert_eq(64, len(identifier))
    positive(tc, "EVCCID maximum length and weight wrap", leaf_name=subject(identifier))


def test_evccid_case_separators_zeros(tc: TestContext):
    identifier = validation.fixtures.evccid("000000000ABCDEF", separators=True).lower()
    positive(tc, "EVCCID lowercase separators and leading zeros", leaf_name=subject(identifier))


def test_unicode_organization_boundary(tc: TestContext):
    positive(tc, "64 Unicode characters in organization", leaf_name=subject(validation.fixtures.evccid(), "😀" * 64))


def test_intermediate_optional_names(tc: TestContext):
    extras = [x509.NameAttribute(NameOID.COUNTRY_NAME, "DE"),
              x509.NameAttribute(NameOID.ORGANIZATIONAL_UNIT_NAME, "Fahrzeugprüfung")]
    positive(tc, "Intermediate optional country and Unicode OU", sub1_name=subject("Sub1", extra=extras))


def test_evccid_fixed_vector(tc: TestContext):
    # Fixed independently calculated vector: decimal value string
    # 32313231123456789101112131415 gives checksum 508166864, remainder 7.
    positive(tc, "EVCCID fixed check-digit vector", leaf_name=subject("WVWV123456789ABCDEF7"))


def test_issuer_missing_organization(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    saved = env.pki["vehicle"]
    root, _, key = saved
    name = x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, "Root without organization")])
    replacement = validation.fixtures.issue(name, key, name, key, ca=True, path_length=2, source=None)
    pem = validation.fixtures.pem(replacement)
    tc.assert_eq("Accepted", env.csms.call("InstallCertificate", {
        "certificateType": "OEMRootCertificate", "certificate": pem.decode()})["status"])
    try:
        env.pki["vehicle"] = replacement, pem, key
        negative(tc, "Top vehicle CA issuer missing organization")
    finally:
        env.pki["vehicle"] = saved


def test_names_private_waiver(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    saved = env.variable("PrivateEnvironmentEnabled")
    try:
        env.variable("PrivateEnvironmentEnabled", "true")
        negative(tc, "Private waiver retains EVCCID validation", leaf_name=subject("EVCCID"),
                 leaf_opts={"source": None}, sub1_opts={"source": None}, sub2_opts={"source": None})
    finally:
        env.variable("PrivateEnvironmentEnabled", saved)


def generate_tests():
    valid = validation.fixtures.evccid()
    cases = {
        "short": validation.fixtures.evccid("123456789ABCDE"),
        "long": validation.fixtures.evccid("1" * 60),
        "bad_check": valid[:-1] + ("1" if valid[-1] == "0" else "0"),
        "bad_type": valid[:3] + "S" + valid[4:],
        "forbidden_letter": validation.fixtures.evccid("123456789ABCDEI"),
        "punctuation": valid[:6] + "_" + valid[7:],
        "misplaced_separator": valid[:6] + "-" + valid[6:],
        "double_separator": valid[:3] + "--" + valid[3:],
        "trailing_separator": valid + "-",
    }
    tests = {}
    for label, identifier in cases.items():
        def test(tc, label=label, identifier=identifier):
            negative(tc, f"EVCCID {label}", leaf_name=subject(identifier))
        tests[f"test_evccid_{label}"] = test
    for position in ("leaf", "sub1", "sub2"):
        cn = valid if position == "leaf" else "SubCA"
        names = {
            "missing_cn": subject(),
            "missing_o": subject(cn, None),
            "empty_o": subject(cn, ""),
            "long_o": subject(cn, "é" * 65),
            "printable_o": subject(cn, organization_type=_ASN1Type.PrintableString),
            "duplicate_cn": subject(cn, extra=[x509.NameAttribute(NameOID.COMMON_NAME, cn)]),
            "duplicate_o": subject(cn, extra=[x509.NameAttribute(NameOID.ORGANIZATION_NAME, "Second OEM")]),
            "extra_attribute": subject(cn, extra=[x509.NameAttribute(NameOID.LOCALITY_NAME, "Berlin")]),
        }
        for label, name in names.items():
            def test(tc, label=label, name=name, position=position):
                negative(tc, f"{position} {label}", **{f"{position}_name": name})
            tests[f"test_{position}_{label}"] = test
    for label, attribute in (("country", x509.NameAttribute(NameOID.COUNTRY_NAME, "DE")),
                             ("ou", x509.NameAttribute(NameOID.ORGANIZATIONAL_UNIT_NAME, "EV"))):
        def test(tc, label=label, attribute=attribute):
            negative(tc, f"Leaf excluded {label}", leaf_name=subject(valid, extra=[attribute]))
        tests[f"test_leaf_excluded_{label}"] = test
    return tests


if __name__ == "__main__":
    run_testsuite(dict(locals(), **generate_tests()))
