#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""V2G20-1001/2432, AMD1 B.8: constraints and extension criticality."""

from cryptography import x509
import vehicle_validation as validation
from software.test_runner.test_context import run_testsuite

suite_setup = validation.suite_setup
suite_teardown = validation.suite_teardown
setup = validation.setup
teardown = validation.teardown


def test_constraint_control(tc):
    tc.set_test_timeout(120)
    path, chain = validation.environment.chain("constraints-control")
    validation.environment.positive("Critical BC/KU and exact CA path lengths", path, chain)


def test_private_constraints(tc):
    tc.set_test_timeout(120)
    env = validation.environment
    saved = env.variable("PrivateEnvironmentEnabled")
    try:
        env.variable("PrivateEnvironmentEnabled", "true")
        path, _ = env.chain("constraints-private", leaf_opts={"critical_constraints": False})
        env.negative("Private waiver retains critical constraints", path, "CERTIFICATE_UNKNOWN")
    finally:
        env.variable("PrivateEnvironmentEnabled", saved)


def test_single_ca_unused_path_allowance(tc):
    tc.set_test_timeout(180)
    env = validation.environment
    root, _, root_key = env.pki["vehicle"]
    fixtures = validation.fixtures
    for limit in (0, 1):
        name = fixtures.name("Single vehicle CA", ["EV"])
        ca = fixtures.issue(name, env.sub1_key, root.subject, root_key, ca=True, path_length=limit)
        leaf = fixtures.issue(fixtures.name(fixtures.evccid(), ["EV"]), env.leaf_key, name,
                              env.sub1_key, eku=[fixtures.EKU.CLIENT_AUTH])
        path = env.work / f"single-ca-{limit}.pem"
        path.write_bytes(fixtures.pem(leaf) + fixtures.pem(ca))
        env.positive(f"Single CA with pathLen {limit}", path, [leaf, ca])


def generate_tests():
    tests = {}
    for position in ("leaf", "sub1", "sub2"):
        cases = [
            ("noncritical_bc", {"critical_constraints": False}, "CERTIFICATE_UNKNOWN"),
            ("noncritical_ku", {"critical_usage": False}, "UNSUPPORTED_CERTIFICATE"),
            ("critical_ski", {"critical_ski": True}, "CERTIFICATE_UNKNOWN"),
            ("critical_aki", {"critical_aki": True}, "CERTIFICATE_UNKNOWN"),
            ("missing_bc", {"constraints": False}, "CERTIFICATE_UNKNOWN" if position == "leaf" else "UNKNOWN_CA"),
            ("critical_aia", {"critical_aia": True}, "BAD_CERTIFICATE"),
        ]
        if position != "leaf":
            cases += [("unbounded", {"path_length": None}, "CERTIFICATE_UNKNOWN"),
                      ("excessive", {"path_length": 2}, "CERTIFICATE_UNKNOWN")]
            cases += [("wrong_depth", {"path_length": 0 if position == "sub1" else 1}, "UNKNOWN_CA" if position == "sub1" else "CERTIFICATE_UNKNOWN")]
            length = 1 if position == "sub1" else 0
            cases += [("integer_ca", {"constraints": False, "extra_extensions": [(
                x509.UnrecognizedExtension(x509.ObjectIdentifier("2.5.29.19"), b"\x30\x06\x02\x01\x01\x02\x01" + bytes([length])), True)]}, "CERTIFICATE_UNKNOWN")]
        else:
            # Well-formed ASN.1 that cryptography's BC builder refuses:
            # cA defaults false but pathLenConstraint is present.
            cases += [("leaf_path_length", {"constraints": False, "extra_extensions": [(
                x509.UnrecognizedExtension(x509.ObjectIdentifier("2.5.29.19"), b"\x30\x03\x02\x01\x00"), True)]}, "CERTIFICATE_UNKNOWN")]
        for label, options, alert in cases:
            def test(tc, position=position, label=label, options=options, alert=alert):
                tc.set_test_timeout(90)
                path, _ = validation.environment.chain(f"constraints-{position}-{label}", **{f"{position}_opts": options})
                validation.environment.negative(f"{position} {label}", path, alert)
            tests[f"test_{position}_{label}"] = test
    return tests


if __name__ == "__main__":
    run_testsuite(dict(locals(), **generate_tests()))
