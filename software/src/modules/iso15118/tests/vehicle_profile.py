#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""V2G20-1001/2432: vehicle certificate cryptographic profile checks."""

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
    return tests


if __name__ == "__main__":
    run_testsuite(dict(locals(), **generate_tests()))
