#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""V2G20-1001/2432: RFC 5280 certificate time encodings on the device."""

import base64
from datetime import datetime, timezone

from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec

import vehicle_validation as validation
from vehicle_validation import TestContext
from software.test_runner.test_context import run_testsuite

suite_setup = validation.suite_setup
suite_teardown = validation.suite_teardown
setup = validation.setup
teardown = validation.teardown


def tlv(tag, value):
    size = len(value)
    length = bytes([size]) if size < 128 else bytes([0x80 | ((size.bit_length() + 7) // 8)]) + size.to_bytes((size.bit_length() + 7) // 8, "big")
    return bytes([tag]) + length + value


def parts(data):
    """Split trusted generated DER, preserving tags and payloads for mutation."""
    result = []
    offset = 0
    while offset < len(data):
        tag, size = data[offset:offset + 2]
        offset += 2
        if size & 0x80:
            count = size & 0x7f
            assert count != 0 and offset + count <= len(data)
            size = int.from_bytes(data[offset:offset + count], "big")
            offset += count
        assert offset + size <= len(data)
        result.append((tag, data[offset:offset + size]))
        offset += size
    return result


def change_validity(cert, issuer_key, field, transform):
    outer, = parts(cert.public_bytes(serialization.Encoding.DER))
    assert outer[0] == 0x30
    certificate = parts(outer[1])
    tbs = parts(certificate[0][1])
    # Generated v3 TBSCertificate: version, serial, signature, issuer, validity.
    assert tbs[0][0] == 0xa0 and tbs[4][0] == 0x30
    validity = parts(tbs[4][1])
    assert len(validity) == 2
    validity[field] = transform(*validity[field])
    tbs[4] = 0x30, b"".join(tlv(*item) for item in validity)
    encoded = tlv(0x30, b"".join(tlv(*item) for item in tbs))
    signature = issuer_key.sign(encoded, ec.ECDSA(hashes.SHA512()))
    # Verify independently before sending, so invalid signature is not the
    # rejection cause. Only the selected time field and signature are changed.
    issuer_key.public_key().verify(signature, encoded, ec.ECDSA(hashes.SHA512()))
    return tlv(0x30, encoded + tlv(*certificate[1]) + tlv(3, b"\x00" + signature))


def mutated_chain(label, position, field, transform):
    env = validation.environment
    path, chain = env.chain(label)
    index = {"leaf": 0, "sub2": 1, "sub1": 2}[position]
    key = [env.sub2_key, env.sub1_key, env.pki["vehicle"][2]][index]
    der = change_validity(chain[index], key, field, transform)
    encoded = base64.b64encode(der)
    pem = b"-----BEGIN CERTIFICATE-----\n" + b"\n".join(encoded[i:i + 64] for i in range(0, len(encoded), 64)) + b"\n-----END CERTIFICATE-----\n"
    path.write_bytes(b"".join(pem if i == index else validation.fixtures.pem(cert) for i, cert in enumerate(chain)))
    return path, chain, index, der


def test_utc_generalized_boundary(tc: TestContext):
    tc.set_test_timeout(180)
    env = validation.environment
    for year, tag in ((2049, 0x17), (2050, 0x18)):
        options = {"not_before": datetime(1950, 1, 1, tzinfo=timezone.utc),
                   "not_after": datetime(year, 12, 31, 23, 59, 59, tzinfo=timezone.utc)}
        path, chain = env.chain(f"time-boundary-{year}", leaf_opts=options)
        outer, = parts(chain[0].public_bytes(serialization.Encoding.DER))
        times = parts(parts(parts(outer[1])[0][1])[4][1])
        tc.assert_eq([(0x17, b"500101000000Z"), (tag, (f"{year % 100:02}" if tag == 0x17 else str(year)).encode() + b"1231235959Z")], times)
        env.positive(f"UTCTime 1950 start and {year} expiry encoding", path, chain)


def test_generalized_no_expiry(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    path, chain = env.chain("time-no-expiry", leaf_opts={"not_after": datetime(9999, 12, 31, 23, 59, 59, tzinfo=timezone.utc)})
    env.positive("RFC 5280 GeneralizedTime no-expiry sentinel", path, chain)


def test_leap_centuries(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    path, chain = env.chain("time-leap-2000", leaf_opts={"not_before": datetime(2000, 2, 29, tzinfo=timezone.utc)})
    env.positive("Gregorian leap-century 2000 accepted", path, chain)
    path, _, _, _ = mutated_chain("time-leap-2100", "leaf", 1, lambda tag, value: (0x18, b"21000229000000Z"))
    env.negative("Gregorian non-leap-century 2100 rejected", path, "BAD_CERTIFICATE")


def test_generalized_missing_z(tc: TestContext):
    tc.set_test_timeout(90)
    path, _, _, _ = mutated_chain("time-generalized-no-z", "leaf", 1, lambda tag, value: (0x18, b"20501231235959"))
    validation.environment.negative("GeneralizedTime requires Z", path, "CERTIFICATE_UNKNOWN")


def test_utc_50_means_1950(tc: TestContext):
    tc.set_test_timeout(90)
    path, _, _, _ = mutated_chain("time-utc-50-expired", "leaf", 1, lambda tag, value: (0x17, b"501231235959Z"))
    validation.environment.negative("UTCTime 50 is expired 1950, not future 2050", path, "CERTIFICATE_EXPIRED")


def test_private_waiver_retains_encoding(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    saved = env.variable("PrivateEnvironmentEnabled")
    try:
        env.variable("PrivateEnvironmentEnabled", "true")
        path, _, _, _ = mutated_chain("time-private-no-z", "leaf", 0, lambda tag, value: (tag, value[:-1]))
        env.negative("Private waiver retains time encoding checks", path, "CERTIFICATE_UNKNOWN")
    finally:
        env.variable("PrivateEnvironmentEnabled", saved)


def generate_tests():
    tests = {}
    cases = {
        "missing_z": (lambda tag, value: (tag, value[:-1]), "CERTIFICATE_UNKNOWN"),
        "generalized_before_2050": (lambda tag, value: (0x18, b"20" + value), "CERTIFICATE_UNKNOWN"),
        "missing_seconds": (lambda tag, value: (tag, value[:-3] + b"Z"), "BAD_CERTIFICATE"),
        "offset": (lambda tag, value: (tag, value[:-1] + b"+0000"), "BAD_CERTIFICATE"),
        "fraction": (lambda tag, value: (tag, value[:-1] + b".0Z"), "BAD_CERTIFICATE"),
        "invalid_leap_day": (lambda tag, value: (tag, b"260229000000Z"), "BAD_CERTIFICATE"),
    }
    for position in ("leaf", "sub1", "sub2"):
        for field in (0, 1):
            for label, (transform, alert) in cases.items():
                def test(tc, position=position, field=field, label=label, transform=transform, alert=alert):
                    tc.set_test_timeout(90)
                    name = f"time-{position}-{field}-{label}"
                    path, _, _, _ = mutated_chain(name, position, field, transform)
                    validation.environment.negative(name, path, alert)
                tests[f"test_{position}_{field}_{label}"] = test
    return tests


if __name__ == "__main__":
    run_testsuite(dict(locals(), **generate_tests()))
