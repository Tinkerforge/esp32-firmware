#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""V2G20-1001/2432, Annex B.8: vehicle certificate profile checks."""

from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec, ed448

import vehicle_validation as validation
from vehicle_validation import TestContext
from software.test_runner.test_context import run_testsuite

suite_setup = validation.suite_setup
suite_teardown = validation.suite_teardown
setup = validation.setup
teardown = validation.teardown


def test_profile_control(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    path, chain = env.chain("profile-p521-sha512")
    env.positive("P521 SHA512 profile control", path, chain)


def test_ed448_intermediates(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    saved = env.sub1_key, env.sub2_key
    try:
        env.sub1_key = ed448.Ed448PrivateKey.generate()
        env.sub2_key = ed448.Ed448PrivateKey.generate()
        path, chain = env.chain("profile-ed448-intermediates")
        env.positive("Ed448 intermediate keys and signatures remain accepted", path, chain)
    finally:
        env.sub1_key, env.sub2_key = saved


def check_hash(tc, position, algorithm):
    tc.set_test_timeout(90)
    env = validation.environment
    label = f"profile-{position}-{algorithm.name}"
    path, _ = env.chain(label, **{f"{position}_opts": {"signing_hash": algorithm}})
    env.negative(label, path, "CERTIFICATE_UNKNOWN")


def check_curve(tc, position, curve):
    tc.set_test_timeout(90)
    env = validation.environment
    field = f"{position}_key"
    saved = getattr(env, field)
    try:
        setattr(env, field, ec.generate_private_key(curve))
        label = f"profile-{position}-{curve.name}"
        path, _ = env.chain(label)
        env.negative(label, path, "CERTIFICATE_UNKNOWN")
    finally:
        setattr(env, field, saved)


def check_extension(tc, position, label, options, alert="CERTIFICATE_UNKNOWN"):
    tc.set_test_timeout(90)
    env = validation.environment
    label = f"profile-{position}-{label}"
    path, _ = env.chain(label, **{f"{position}_opts": options})
    env.negative(label, path, alert)


def test_optional_usage_control(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    optional = dict(content_commitment=True, key_encipherment=True, key_agreement=True)
    path, chain = env.chain("profile-optional-usage", leaf_opts=optional,
                           sub1_opts=optional, sub2_opts=optional)
    env.positive("Annex B.8 permitted optional usage bits", path, chain)


def test_critical_key_identifiers(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    path, chain = env.chain("profile-critical-key-identifiers",
                           **{f"{position}_opts": {"critical_ids": True}
                              for position in ("leaf", "sub1", "sub2")})
    env.negative("AMD1 key identifiers must be noncritical", path, "CERTIFICATE_UNKNOWN")


def test_method1_key_identifiers(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    path, chain = env.chain("profile-method1-key-identifiers",
                           **{f"{position}_opts": {"identifier_method": 1}
                              for position in ("leaf", "sub1", "sub2")})
    env.positive("AMD1 method-1 SKI and AKI are processed", path, chain)


def test_unknown_noncritical_extension(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    extension = x509.UnrecognizedExtension(x509.ObjectIdentifier("1.3.6.1.4.1.55555.15118.1"), b"\x05\x00")
    path, chain = env.chain("profile-unknown-noncritical", leaf_opts={"extra_extensions": [(extension, False)]})
    env.positive("Unknown noncritical extension is ignored", path, chain)


def generate_tests():
    tests = {}
    for position in ("leaf", "sub1", "sub2"):
        for algorithm in (hashes.SHA256(), hashes.SHA384()):
            def test(tc, position=position, algorithm=algorithm):
                check_hash(tc, position, algorithm)
            tests[f"test_{position}_{algorithm.name}"] = test
    for position in ("sub1", "sub2"):
        for curve in (ec.SECP256R1(), ec.SECP384R1()):
            def test(tc, position=position, curve=curve):
                check_curve(tc, position, curve)
            tests[f"test_{position}_{curve.name}"] = test
    for position in ("leaf", "sub1", "sub2"):
        for identifier in ("ski", "aki"):
            for label, value in (("missing", False), ("wrong", b"\x40" + b"\x00" * 7),
                                 ("wrong_method1", b"\x12" * 20)):
                def test(tc, position=position, identifier=identifier, label=label, value=value):
                    check_extension(tc, position, f"{identifier}-{label}", {identifier: value})
                tests[f"test_{position}_{identifier}_{label}"] = test
        for label, options in (("data_encipherment", {"data_encipherment": True}),
                               ("encipher_only", {"key_agreement": True, "encipher_only": True}),
                               ("decipher_only", {"key_agreement": True, "decipher_only": True}),
                               ("crl_sign", {"crl_sign": True})):
            def test(tc, position=position, label=label, options=options):
                # Mbed TLS excludes CAs restricted to encipher/decipher-only
                # during issuer selection, before vehicle-profile verification.
                alert = "UNKNOWN_CA" if position != "leaf" and label in ("encipher_only", "decipher_only") else "UNSUPPORTED_CERTIFICATE"
                check_extension(tc, position, label, options, alert)
            tests[f"test_{position}_{label}"] = test
        def test(tc, position=position):
            extension = x509.UnrecognizedExtension(x509.ObjectIdentifier("1.3.6.1.4.1.55555.15118.1"), b"\x05\x00")
            check_extension(tc, position, "unknown-critical", {"extra_extensions": [(extension, True)]}, "BAD_CERTIFICATE")
        tests[f"test_{position}_unknown_critical"] = test
        def test(tc, position=position):
            # AMD1 Tables B.13/B.14 exclude authorityCertIssuer/serialNumber.
            # Encode the well-formed pair explicitly to isolate that policy.
            key = validation.environment.sub2_key if position == "leaf" else (
                validation.environment.sub1_key if position == "sub2" else validation.environment.pki["vehicle"][2])
            identifier = validation.fixtures.key_identifier(key)
            value = b"\x30\x17\x80\x08" + identifier + b"\xa1\x08\x86\x06http:x\x82\x01\x01"
            extension = x509.UnrecognizedExtension(x509.ObjectIdentifier("2.5.29.35"), value)
            check_extension(tc, position, "aki-issuer-serial", {"aki": False, "extra_extensions": [(extension, False)]})
        tests[f"test_{position}_aki_issuer_serial"] = test
    return tests


def test_eku_client_server_control(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    path, chain = env.chain("eku-client-server", leaf_opts={
        "eku": [validation.fixtures.EKU.SERVER_AUTH, validation.fixtures.EKU.CLIENT_AUTH]})
    env.positive("Vehicle EKU allows clientAuth plus serverAuth in either order", path, chain)


def test_eku_private_waiver(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    saved = env.variable("PrivateEnvironmentEnabled")
    try:
        env.variable("PrivateEnvironmentEnabled", "true")
        path, _ = env.chain("eku-private-missing", leaf_opts={"eku": None, "source": None},
                            sub1_opts={"source": None}, sub2_opts={"source": None})
        env.negative("Private source waiver retains mandatory EKU", path, "UNSUPPORTED_CERTIFICATE")
    finally:
        env.variable("PrivateEnvironmentEnabled", saved)


def generate_eku_tests():
    eku = validation.fixtures.EKU
    tests = {}
    cases = [("missing", {"eku": None}),
             ("noncritical", {"eku": [eku.CLIENT_AUTH], "critical_eku": False}),
             ("server_only", {"eku": [eku.SERVER_AUTH]}),
             ("any_only", {"eku": [eku.ANY_EXTENDED_KEY_USAGE]}),
             ("client_and_any", {"eku": [eku.CLIENT_AUTH, eku.ANY_EXTENDED_KEY_USAGE]}),
             ("client_and_code_signing", {"eku": [eku.CLIENT_AUTH, eku.CODE_SIGNING]})]
    for label, options in cases:
        def test(tc, label=label, options=options):
            check_extension(tc, "leaf", f"eku-{label}", options, "UNSUPPORTED_CERTIFICATE")
        tests[f"test_eku_leaf_{label}"] = test
    for position in ("sub1", "sub2"):
        def test(tc, position=position):
            check_extension(tc, position, "eku-excluded", {"eku": [eku.CLIENT_AUTH]}, "UNSUPPORTED_CERTIFICATE")
        tests[f"test_eku_{position}_excluded"] = test
    return tests


if __name__ == "__main__":
    run_testsuite(dict(locals(), **generate_tests(), **generate_eku_tests()))
