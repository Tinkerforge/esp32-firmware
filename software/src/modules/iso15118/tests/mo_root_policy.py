#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""HUB20-411-003/004/008: MO storage and vehicle-trust separation."""

from cryptography.hazmat.primitives.asymmetric import ec

import vehicle_validation as validation
from vehicle_validation import TestContext, fixtures, certificate_hash_data, common
from root_policy import advertised_authorities, setup
from software.test_runner.test_context import run_testsuite

suite_setup = validation.suite_setup
suite_teardown = validation.suite_teardown
teardown = validation.teardown


def test_non_pnc_mo_storage_and_vehicle_trust(tc: TestContext):
    tc.set_test_timeout(600)
    env = validation.environment
    capability = env.csms.call("GetVariables", {"getVariableData": [{
        "component": {"name": "ISO15118Ctrlr"},
        "variable": {"name": "ContractCertificateInstallationEnabled"},
    }]})["getVariableResult"][0]
    env.write_json("mo-capability.json", capability)
    if capability["attributeStatus"] == "Accepted":
        tc.skip("Requires the non-PnC firmware variant")
    tc.assert_eq("UnknownVariable", capability["attributeStatus"])
    original = sorted(env.inventory(), key=env.identity)
    saved_vehicle = env.pki["vehicle"]
    saved_mode = env.variable("PrivateEnvironmentEnabled")
    roots = []
    operations = []

    def call(action, payload):
        result = env.csms.call(action, payload, timeout=90)
        operations.append({"action": action, "request": payload, "response": result})
        env.write_json("mo-operations.json", operations)
        return result

    def count():
        result = call("GetVariables", {"getVariableData": [{
            "component": {"name": "SecurityCtrlr"}, "variable": {"name": "CertificateEntries"},
        }]})["getVariableResult"][0]
        tc.assert_eq("Accepted", result["attributeStatus"])
        return int(result["attributeValue"])

    initial_count = count()
    initial_mo = call("GetInstalledCertificateIds", {"certificateType": ["MORootCertificate"]})
    try:
        for label, curve in (("iso2", ec.SECP256R1()), ("iso20", ec.SECP521R1())):
            key = ec.generate_private_key(curve)
            name = fixtures.name(f"MO isolation {label} {env.work.name}", None)
            root = fixtures.issue(name, key, name, key, ca=True, path_length=2, source=None)
            pem = fixtures.pem(root)
            roots.append((root, pem, key))
            tc.assert_eq("Accepted", call("InstallCertificate", {
                "certificateType": "MORootCertificate", "certificate": pem.decode(),
            })["status"])
        mo = call("GetInstalledCertificateIds", {"certificateType": ["MORootCertificate"]})
        tc.assert_eq("Accepted", mo["status"])
        expected = initial_mo.get("certificateHashDataChain", []) + [{
            "certificateType": "MORootCertificate", "certificateHashData": certificate_hash_data(root, root),
        } for root, _, _ in roots]
        tc.assert_eq(sorted(expected, key=env.identity), sorted(mo["certificateHashDataChain"], key=env.identity))
        tc.assert_eq(initial_count + 2, count())
        env.record("non-PnC accepts and reports P-256 and P-521 MO roots", mo)

        # Persisted storage must not turn an MO root into vehicle trust.
        connections = env.csms.connection_count
        tc.reboot()
        env.csms.wait_for_connection(after=connections, timeout=90)
        setup(tc)  # refresh SECC OCSP after reboot
        common.enable_debug_mode(env.host)
        tc.wait_for(lambda: tc.assert_(common.sdp_request(
            env.iface, expected_from=env.target_ll) is not None), timeout=30)
        reboot_mo = call("GetInstalledCertificateIds", {"certificateType": ["MORootCertificate"]})
        tc.assert_eq("Accepted", reboot_mo["status"])
        tc.assert_eq(sorted(expected, key=env.identity),
                     sorted(reboot_mo["certificateHashDataChain"], key=env.identity))
        tc.assert_eq(initial_count + 2, count())
        good_path, _ = env.chain("mo-isolation-control")
        env.pki["vehicle"] = roots[1]
        bad_path, bad_chain = env.chain("mo-only-vehicle")
        for mode in ("false", "true"):
            env.variable("PrivateEnvironmentEnabled", mode)
            names = advertised_authorities(tc, env, good_path)
            for root, _, _ in roots:
                tc.assert_not(root.subject.public_bytes() in names)
            env.negative(f"MO-only vehicle rejected in private={mode}", bad_path, "UNKNOWN_CA")
        env.variable("PrivateEnvironmentEnabled", "false")
        # Positive control: the same chain becomes trusted only when its root
        # is explicitly installed for the OEM role (HUB20-411-003).
        tc.assert_eq("Accepted", call("DeleteCertificate", {
            "certificateHashData": certificate_hash_data(roots[1][0], roots[1][0]),
        })["status"])
        tc.assert_eq("Accepted", call("InstallCertificate", {
            "certificateType": "OEMRootCertificate", "certificate": roots[1][1].decode(),
        })["status"])
        env.positive("same vehicle chain authorizes with explicit OEM trust", bad_path, bad_chain)
        env.record("MO roots survive reboot but are excluded from vehicle trust in both environments")
    finally:
        env.pki["vehicle"] = saved_vehicle
        env.variable("PrivateEnvironmentEnabled", saved_mode)
        # A hash-based deletion removes every role assigned to our fresh roots.
        installed = {entry["certificateHashData"]["serialNumber"] for entry in env.inventory()}
        for root, _, _ in roots:
            root_hash = certificate_hash_data(root, root)
            if root_hash["serialNumber"] in installed:
                tc.assert_eq("Accepted", call("DeleteCertificate", {"certificateHashData": root_hash})["status"])
        tc.assert_eq(original, sorted(env.inventory(), key=env.identity))
        tc.assert_eq(initial_count, count())


if __name__ == "__main__":
    run_testsuite(locals())
