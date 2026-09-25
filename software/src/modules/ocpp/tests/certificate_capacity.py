#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""OCPP certificate capacity on device, preserving the original inventory."""

from datetime import datetime, timedelta, timezone
import json
from pathlib import Path
import tempfile
import time

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import NameOID

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.src.modules.iso15118.tests._common import CSMSSim, LocalCSMSTls
from software.test_runner.test_context import TestContext, run_testsuite


V2G = "V2GRootCertificate"
OEM = "OEMRootCertificate"
MO = "MORootCertificate"
env = None


class CapacityEnvironment:
    def __init__(self, tc):
        self.tc = tc
        self.work = Path(tempfile.mkdtemp(prefix="device-certificate-capacity-"))
        self.saved = {}
        self.tls = None
        self.csms = None
        self.baseline = None
        self.calls = []
        self.roots = []
        self.keys = []
        print(f"Evidence directory: {self.work}", flush=True)

    def save(self, name, value):
        (self.work / name).write_text(json.dumps(value, indent=2))

    def call(self, action, payload):
        result = self.csms.call(action, payload, timeout=90)
        self.calls.append({"action": action, "request": payload, "response": result})
        self.save("calls.json", self.calls)
        return result

    def inventory(self, types=None):
        result = self.call("GetInstalledCertificateIds", {"certificateType": types} if types else {})
        self.tc.assert_(result["status"] in ("Accepted", "NotFound"))
        return result.get("certificateHashDataChain", [])

    @staticmethod
    def identity(entry):
        data = dict(entry["certificateHashData"])
        for name in ("issuerNameHash", "issuerKeyHash"):
            data[name] = data[name].lower()
        data["serialNumber"] = format(int(data["serialNumber"], 16), "x")
        return json.dumps([entry["certificateType"], data], sort_keys=True)

    def entries(self, attribute="Actual"):
        return self.call("GetVariables", {"getVariableData": [{
            "component": {"name": "SecurityCtrlr"},
            "variable": {"name": "CertificateEntries"}, "attributeType": attribute,
        }]})["getVariableResult"][0]

    def count(self):
        result = self.entries()
        self.tc.assert_eq("Accepted", result["attributeStatus"])
        return int(result["attributeValue"])

    def report(self):
        request_id = 2906
        self.tc.assert_eq("Accepted", self.call("GetBaseReport", {
            "requestId": request_id, "reportBase": "FullInventory",
        })["status"])
        reports = []
        data = []
        while True:
            report, message = self.csms.expect("NotifyReport", timeout=60)
            self.csms.respond(message, {})
            self.tc.assert_eq(request_id, report["requestId"])
            self.tc.assert_eq(len(reports), report["seqNo"])
            reports.append(report)
            data.extend(report.get("reportData", []))
            if not report.get("tbc", False):
                break
        self.save("full-inventory.json", reports)
        matches = [item for item in data if item["component"]["name"] == "SecurityCtrlr"
                   and item["variable"]["name"] == "CertificateEntries"]
        self.tc.assert_eq(1, len(matches))
        return matches[0]

    def start(self):
        for path in ("ocpp/config", "certs/state", "info/version"):
            self.saved[path] = self.tc.api(path)
        self.save("config-backup.json", self.saved)
        print("Device firmware:", self.saved["info/version"], flush=True)
        self.tls = LocalCSMSTls(self.tc._esp_host, self.tc.get_local_ip())
        self.csms = CSMSSim(certfile=str(self.tls.certfile), keyfile=str(self.tls.keyfile),
                            interactive=("NotifyReport",))
        config = dict(self.saved["ocpp/config"])
        config.update(enable=True, protocol=1, url=f"wss://{self.tc.get_local_ip()}:{self.csms.port}",
                      enable_auth=True, **{"pass": "certificate-capacity-test", "cert_id": self.tls.cert_id})
        self.tc.api("ocpp/config_update", config)
        self.tc.assert_(self.csms.connected.wait(60))
        time.sleep(2)
        self.baseline = self.inventory()
        self.save("inventory-before.json", self.baseline)
        self.initial_count = self.count()

    def root(self, index):
        while len(self.roots) <= index:
            key = ec.generate_private_key(ec.SECP521R1())
            name = x509.Name([x509.NameAttribute(NameOID.COMMON_NAME,
                f"Capacity {self.work.name[-8:]} root {len(self.roots)}")])
            now = datetime.now(timezone.utc)
            cert = (x509.CertificateBuilder().subject_name(name).issuer_name(name)
                    .public_key(key.public_key()).serial_number(x509.random_serial_number())
                    .not_valid_before(now - timedelta(days=1)).not_valid_after(now + timedelta(days=365))
                    .add_extension(x509.BasicConstraints(ca=True, path_length=2), critical=True)
                    .add_extension(x509.KeyUsage(False, False, False, False, False, True, True, False, False), True)
                    .sign(key, hashes.SHA512()))
            pem = cert.public_bytes(serialization.Encoding.PEM).decode()
            (self.work / f"root-{len(self.roots)}.pem").write_text(pem)
            self.roots.append(pem)
            self.keys.append(key)
        return self.roots[index]

    def install(self, kind, index):
        return self.call("InstallCertificate", {"certificateType": kind, "certificate": self.root(index)})["status"]

    def delete(self, entry):
        self.tc.assert_eq("Accepted", self.call("DeleteCertificate", {
            "certificateHashData": entry["certificateHashData"],
        })["status"])

    def assert_inventory(self, expected, chains=0):
        actual = self.inventory([V2G, OEM, MO])
        self.tc.assert_eq(len(expected), len(actual))
        expected_hashes = []
        for kind, index in expected:
            cert = x509.load_pem_x509_certificate(self.root(index).encode())
            # RFC 6960 hashes the DER Name and subjectPublicKey BIT STRING.
            # For these EC roots the latter is the uncompressed X9.62 point.
            def digest(data):
                h = hashes.Hash(hashes.SHA256())
                h.update(data)
                return h.finalize().hex()
            expected_hashes.append(self.identity({"certificateType": kind, "certificateHashData": {
                "hashAlgorithm": "SHA256", "issuerNameHash": digest(cert.subject.public_bytes()),
                "issuerKeyHash": digest(cert.public_key().public_bytes(
                    serialization.Encoding.X962, serialization.PublicFormat.UncompressedPoint)),
                "serialNumber": format(cert.serial_number, "X"),
            }}).lower())
        self.tc.assert_eq(sorted(expected_hashes), sorted(self.identity(item).lower() for item in actual))
        self.tc.assert_eq(self.initial_count + len(expected) + chains, self.count())
        return actual

    def clean(self):
        if self.baseline is None:
            return
        original = {self.identity(item) for item in self.baseline}
        deleted = set()
        for item in sorted(self.inventory(), key=lambda entry: "Root" in entry["certificateType"]):
            if self.identity(item) not in original:
                # DeleteCertificate identifies a hash, not a certificate type;
                # one deletion removes matching roots in multiple groups.
                hash_id = json.dumps(item["certificateHashData"], sort_keys=True)
                if hash_id in deleted:
                    continue
                self.delete(item)
                deleted.add(hash_id)
        self.tc.assert_eq(sorted(original), sorted(self.identity(item) for item in self.inventory()))
        self.tc.assert_eq(self.initial_count, self.count())

    def close(self):
        errors = []
        def attempt(action):
            try:
                action()
            except Exception as error:
                errors.append(str(error))
        attempt(self.clean)
        if "ocpp/config" in self.saved:
            attempt(lambda: self.tc.api("ocpp/config_update", self.saved["ocpp/config"]))
            attempt(lambda: self.tc.assert_eq(self.saved["ocpp/config"], self.tc.api("ocpp/config")))
        if self.csms:
            attempt(self.csms.stop)
        if self.tls:
            attempt(self.tls.close)
            attempt(lambda: self.tc.assert_eq(self.saved["certs/state"], self.tc.api("certs/state")))
        self.save("cleanup.json", {"errors": errors})
        self.tc.assert_eq([], errors)
        print("PASS original inventory, OCPP configuration and CSMS trust restored", flush=True)


def suite_setup(tc: TestContext):
    global env
    tc.set_test_timeout(120)
    if not tc.device_type().is_warp(4):
        tc.skip("Certificate capacity test requires a WARP4")
    env = CapacityEnvironment(tc)
    env.start()


def teardown(tc: TestContext):
    tc.set_test_timeout(600)
    if env:
        env.clean()


def suite_teardown(tc: TestContext):
    tc.set_test_timeout(600)
    if env:
        env.close()


def test_certificate_entries_reporting(tc: TestContext):
    """OCPP 2.1 SecurityCtrlr.CertificateEntries; Hubject section 3.3.4."""
    tc.set_test_timeout(120)
    result = env.entries("MaxSet")
    tc.assert_eq("MaxSet", result["attributeType"])
    tc.assert_eq("NotSupportedAttributeType", result["attributeStatus"])
    report = env.report()
    tc.assert_eq("integer", report["variableCharacteristics"]["dataType"])
    tc.assert_(report["variableCharacteristics"]["maxLimit"] >= 130)
    attributes = report["variableAttribute"]
    tc.assert_eq(1, len(attributes))
    tc.assert_eq("Actual", attributes[0].get("type", "Actual"))
    tc.assert_eq("ReadOnly", attributes[0]["mutability"])
    tc.assert_eq(env.count(), int(attributes[0]["value"]))
    print("PASS ReadOnly Actual and maxLimit; catalogue MaxSet expectation conflicts with requirements", flush=True)


def test_alternating_roots_and_cross_type_slot_reuse(tc: TestContext):
    """HUB20-411-005, M05.FR.06/17, M04: advertised capacity and slot reuse."""
    tc.set_test_timeout(1800)
    if env.baseline:
        tc.skip("Exact empty-store capacity test requires an empty OCPP inventory")
    tc.assert_eq([], env.inventory([V2G, OEM]))
    limit = int(env.report()["variableCharacteristics"]["maxLimit"])
    tc.assert_(130 <= limit <= 512)
    connections = env.csms.connection_count
    expected = []
    for index in range(limit):
        kind = V2G if index % 2 == 0 else OEM
        status = env.install(kind, index)
        print(f"Install {index + 1}/{limit} {kind}: {status}", flush=True)
        tc.assert_eq("Accepted", status)
        expected.append((kind, index))
    inventory = env.assert_inventory(expected)
    for kind in (V2G, OEM):
        tc.assert_eq("Rejected", env.install(kind, limit))
    tc.assert_eq("Accepted", env.install(V2G, 0))
    env.assert_inventory(expected)
    # Delete the opposite type and retry the exact previously rejected root.
    for removed_kind, added_kind in ((OEM, V2G), (V2G, OEM)):
        removed = next(item for item in inventory if item["certificateType"] == removed_kind)
        serial = int(removed["certificateHashData"]["serialNumber"], 16)
        expected = [(kind, index) for kind, index in expected
                    if not (kind == removed_kind and x509.load_pem_x509_certificate(
                        env.root(index).encode()).serial_number == serial)]
        env.delete(removed)
        tc.assert_eq(limit - 1, env.count())
        tc.assert_eq("Accepted", env.install(added_kind, limit))
        expected.append((added_kind, limit))
        inventory = env.assert_inventory(expected)
    tc.assert_eq(connections, env.csms.connection_count)
    tc.reboot()
    env.csms.wait_for_connection(after=connections, timeout=90)
    env.assert_inventory(expected)
    print(f"PASS {limit} alternating roots, overflow, replacement, cross-type reuse and reboot inventory", flush=True)

    # Chains consume the same advertised slots. Use an old V2G root beyond
    # the former loader bound after reboot (entries load newest first).
    env.csms.interactive.add("SignCertificate")
    tc.assert_eq("Accepted", env.call("TriggerMessage", {
        "requestedMessage": "SignV2G20Certificate",
    })["status"])
    request, message = env.csms.expect("SignCertificate", timeout=90)
    env.csms.respond(message, {"status": "Accepted"})
    csr = x509.load_pem_x509_csr(request["csr"].encode())
    tc.assert_(csr.is_signature_valid)
    root_index = next(index for kind, index in expected if kind == V2G)
    root = x509.load_pem_x509_certificate(env.root(root_index).encode())
    now = datetime.now(timezone.utc)
    leaf = (x509.CertificateBuilder().subject_name(csr.subject).issuer_name(root.subject)
            .public_key(csr.public_key()).serial_number(x509.random_serial_number())
            .not_valid_before(now - timedelta(minutes=5)).not_valid_after(now + timedelta(days=45))
            .add_extension(x509.BasicConstraints(False, None), True)
            .add_extension(x509.KeyUsage(True, False, False, False, False, False, False, False, False), True)
            .add_extension(x509.AuthorityInformationAccess([x509.AccessDescription(
                x509.AuthorityInformationAccessOID.OCSP,
                x509.UniformResourceIdentifier("http://ocsp.capacity.test/secc"))]), False)
            .sign(env.keys[root_index], hashes.SHA512()))
    signed = {"certificateType": "V2G20Certificate", "requestId": request["requestId"],
              "certificateChain": leaf.public_bytes(serialization.Encoding.PEM).decode()}
    tc.assert_eq("Rejected", env.call("CertificateSigned", signed)["status"])
    env.assert_inventory(expected)
    removed_kind, removed_index = next(item for item in expected if item[0] == OEM)
    serial = x509.load_pem_x509_certificate(env.root(removed_index).encode()).serial_number
    removed = next(item for item in env.inventory([OEM])
                   if int(item["certificateHashData"]["serialNumber"], 16) == serial)
    env.delete(removed)
    expected.remove((removed_kind, removed_index))
    tc.assert_eq("Accepted", env.call("CertificateSigned", signed)["status"])
    env.assert_inventory(expected, chains=1)
    tc.assert_eq("Rejected", env.install(OEM, limit + 1))
    connections = env.csms.connection_count
    tc.reboot()
    env.csms.wait_for_connection(after=connections, timeout=90)
    env.assert_inventory(expected, chains=1)
    tc.assert_eq(1, len(env.inventory(["V2GCertificateChain"])))
    print("PASS shared root/chain capacity and anchor recovery beyond the former root-loader bound", flush=True)


def test_recommended_root_mix(tc: TestContext):
    """HUB20-411-006/007/008: simultaneous 30 V2G, 50 OEM and 40 MO."""
    tc.set_test_timeout(1800)
    if env.baseline:
        tc.skip("Exact root-mix test requires an empty OCPP inventory")
    expected = []
    for kind, count in ((V2G, 30), (OEM, 50), (MO, 40)):
        for _ in range(count):
            index = len(expected)
            tc.assert_eq("Accepted", env.install(kind, index))
            expected.append((kind, index))
        print(f"Installed {count} {kind}", flush=True)
    env.assert_inventory(expected)


if __name__ == "__main__":
    run_testsuite(locals())
