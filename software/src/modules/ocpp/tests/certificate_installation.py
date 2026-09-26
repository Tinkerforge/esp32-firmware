#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""A02/A03 delivery checks on an isolated device-side namespace."""

from datetime import datetime, timedelta, timezone
import json
from pathlib import Path
import tempfile
import time
import ssl

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec, rsa
from cryptography.x509.oid import NameOID, ExtendedKeyUsageOID

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.src.modules.iso15118.tests._common import CSMSSim, LocalCSMSTls
from software.test_runner.test_context import TestContext, run_testsuite


env = None


def pem(cert):
    return cert.public_bytes(serialization.Encoding.PEM).decode()


class InstallationEnvironment:
    def __init__(self, tc):
        self.tc = tc
        self.work = Path(tempfile.mkdtemp(prefix="device-certificate-installation-"))
        # Leave room for the store suffix and multi-digit object IDs in the
        # embedded filesystem's path limit.
        self.identity = "install-" + self.work.name[-8:]
        (self.work / "namespace.txt").write_text(self.identity)
        self.saved = {}
        self.calls = []
        self.csms = None
        self.tls = None
        self.root_key = ec.generate_private_key(ec.SECP256R1())
        self.root = None
        self.namespace_ready = False
        print(f"Evidence directory: {self.work}", flush=True)

    def save(self, name, data):
        (self.work / name).write_text(json.dumps(data, indent=2))

    def call(self, action, payload):
        result = self.csms.call(action, payload, timeout=90)
        self.calls.append({"action": action, "request": payload, "response": result})
        self.save("calls.json", self.calls)
        return result

    def count(self):
        result = self.call("GetVariables", {"getVariableData": [{
            "component": {"name": "SecurityCtrlr"}, "variable": {"name": "CertificateEntries"},
        }]})["getVariableResult"][0]
        self.tc.assert_eq("Accepted", result["attributeStatus"])
        return int(result["attributeValue"])

    def inventory(self):
        result = self.call("GetInstalledCertificateIds", {})
        self.tc.assert_(result["status"] in ("Accepted", "NotFound"))
        return result.get("certificateHashDataChain", [])

    def start(self):
        for path in ("ocpp/config", "certs/state", "info/version"):
            self.saved[path] = self.tc.api(path)
        self.save("config-backup.json", self.saved)
        print("Firmware:", self.saved["info/version"], flush=True)
        self.tls = LocalCSMSTls(self.tc._esp_host, self.tc.get_local_ip())
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.load_cert_chain(self.tls.certfile, self.tls.keyfile)
        self.csms = CSMSSim(ssl_context=context,
                            interactive=("BootNotification", "SignCertificate", "SecurityEventNotification"))
        config = dict(self.saved["ocpp/config"])
        config.update(enable=True, protocol=1, identity=self.identity,
                      url=f"wss://{self.tc.get_local_ip()}:{self.csms.port}",
                      enable_auth=True, **{"pass": "certificate-installation-test", "cert_id": self.tls.cert_id})
        self.tc.api("ocpp/config_update", config)
        self.tc.assert_(self.csms.connected.wait(60))
        boot, message = self.csms.expect("BootNotification", timeout=60)
        self.serial = boot["chargingStation"]["serialNumber"]
        self.csms.respond(message, {"status": "Accepted", "interval": 300,
            "currentTime": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")})
        time.sleep(2)
        self.tc.assert_eq([], self.inventory())
        self.namespace_ready = True
        name = x509.Name([x509.NameAttribute(NameOID.ORGANIZATION_NAME, "Installation tests"),
                          x509.NameAttribute(NameOID.COMMON_NAME, self.work.name)])
        now = datetime.now(timezone.utc)
        self.root = (x509.CertificateBuilder().subject_name(name).issuer_name(name)
                     .public_key(self.root_key.public_key()).serial_number(x509.random_serial_number())
                     .not_valid_before(now - timedelta(days=1)).not_valid_after(now + timedelta(days=365))
                     .add_extension(x509.BasicConstraints(True, 2), True)
                     .add_extension(x509.KeyUsage(False, False, False, False, False, True, True, False, False), True)
                     .sign(self.root_key, hashes.SHA256()))
        context.load_verify_locations(cadata=pem(self.root))
        context.verify_mode = ssl.CERT_OPTIONAL
        self.tc.assert_eq("Accepted", self.call("InstallCertificate", {
            "certificateType": "CSMSRootCertificate", "certificate": pem(self.root)})["status"])

    def request(self):
        self.tc.assert_eq("Accepted", self.call("TriggerMessage", {
            "requestedMessage": "SignChargingStationCertificate"})["status"])
        req, message = self.csms.expect("SignCertificate", timeout=60)
        self.csms.respond(message, {"status": "Accepted"})
        csr = x509.load_pem_x509_csr(req["csr"].encode())
        self.tc.assert_(csr.is_signature_valid)
        self.tc.assert_eq("ChargingStationCertificate", req["certificateType"])
        return req, csr

    def certificate(self, csr, *, cn=None, organization="Installation tests", ca=False,
                    future=0, expired=False, issuer_key=None, issuer=None, public_key=None,
                    usage=None, eku=None, lifetime=45 * 86400, extra_subject=()):
        attributes = [x509.NameAttribute(NameOID.COMMON_NAME, cn if cn is not None else self.serial)]
        if organization is not None:
            attributes.append(x509.NameAttribute(NameOID.ORGANIZATION_NAME, organization))
        attributes.extend(extra_subject)
        now = datetime.now(timezone.utc)
        builder = (x509.CertificateBuilder().subject_name(x509.Name(attributes))
                .issuer_name((issuer or self.root).subject)
                .public_key(public_key or csr.public_key()).serial_number(x509.random_serial_number())
                .not_valid_before(now + timedelta(seconds=future) if future else now - timedelta(minutes=5))
                .not_valid_after(now - timedelta(minutes=1) if expired else now + timedelta(seconds=lifetime))
                .add_extension(x509.BasicConstraints(ca, 0 if ca else None), True)
                .add_extension(usage or x509.KeyUsage(True, False, False, False, False, ca, False, False, False), True))
        if eku is not None:
            builder = builder.add_extension(x509.ExtendedKeyUsage(eku), False)
        return builder.sign(issuer_key or self.root_key, hashes.SHA256())

    def deliver(self, req, chain):
        connections = self.csms.connection_count
        result = self.call("CertificateSigned", {"certificateType": "ChargingStationCertificate",
            "requestId": req["requestId"], "certificateChain": chain})
        if result["status"] == "Accepted":
            self.csms.wait_for_connection(after=connections, timeout=60)
            self.tc.assert_(self.csms.connected.wait(5))
            observation = self.csms.wait_for_tls_connection(after=connections, timeout=10)
            expected = x509.load_pem_x509_certificate(chain.encode()).public_bytes(serialization.Encoding.DER)
            self.tc.assert_eq(expected, observation["peer_certificate_der"])
            time.sleep(1)
            # Public inventory omits CSMS client chains; the live entry count
            # catches retained obsolete identities after fast reconnects.
            self.tc.assert_eq(2, self.count())
        return result

    def negative(self, **options):
        req, csr = self.request()
        self.reject_and_recover(req, csr, pem(self.certificate(csr, **options)))

    def reject_and_recover(self, req, csr, chain):
        before = self.inventory()
        result = self.deliver(req, chain)
        self.tc.assert_eq("Rejected", result["status"])
        self.tc.assert_eq(before, self.inventory())
        event, message = self.csms.expect("SecurityEventNotification", timeout=30)
        self.csms.respond(message, {})
        self.tc.assert_eq("InvalidChargingStationCertificate", event["type"])
        datetime.fromisoformat(event["timestamp"].replace("Z", "+00:00"))
        # A rejection must not consume the pending key or prevent corrected delivery.
        self.tc.assert_eq("Accepted", self.deliver(req, pem(self.certificate(csr)))["status"])
        time.sleep(4)

    def close(self):
        errors = []
        def attempt(action):
            try:
                action()
            except Exception as error:
                errors.append(str(error))
        attempt(lambda: (self.work / "trace.log").write_bytes(self.tc.http_request("GET", "/trace_log", timeout=15)))
        if self.csms and self.namespace_ready:
            def clean():
                self.tc.assert_(self.csms.connected.wait(60))
                deleted = set()
                for item in sorted(self.inventory(), key=lambda entry: "Root" in entry["certificateType"]):
                    identity = json.dumps(item["certificateHashData"], sort_keys=True)
                    if identity in deleted:
                        continue
                    self.tc.assert_eq("Accepted", self.call("DeleteCertificate", {
                        "certificateHashData": item["certificateHashData"]})["status"])
                    deleted.add(identity)
                self.tc.assert_eq([], self.inventory())
            attempt(clean)
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
        print("PASS public test inventory removed; original configuration and CSMS trust restored", flush=True)
        if self.namespace_ready:
            print(f"Local cleanup required for inactive client credentials: /ocpp/{self.identity}.certs", flush=True)


def suite_setup(tc: TestContext):
    global env
    tc.set_test_timeout(180)
    env = InstallationEnvironment(tc)
    env.start()


def suite_teardown(tc: TestContext):
    tc.set_test_timeout(180)
    if env:
        env.close()


def test_wrong_station_identity(tc: TestContext):
    env.negative(cn="another-station")


def test_missing_organization(tc: TestContext):
    env.negative(organization=None)


def test_ca_as_station_leaf(tc: TestContext):
    env.negative(ca=True)


def test_future_station_certificate(tc: TestContext):
    env.negative(future=240)


def test_expired_station_certificate(tc: TestContext):
    env.negative(expired=True)


def test_wrong_pending_key(tc: TestContext):
    env.negative(public_key=ec.generate_private_key(ec.SECP256R1()).public_key())


def test_forged_station_signature(tc: TestContext):
    env.negative(issuer_key=ec.generate_private_key(ec.SECP256R1()))


def test_csr_uses_device_serial(tc: TestContext):
    req, csr = env.request()
    tc.assert_eq(env.serial, csr.subject.get_attributes_for_oid(NameOID.COMMON_NAME)[0].value)
    tc.assert_eq("Accepted", env.deliver(req, pem(env.certificate(csr)))["status"])
    time.sleep(4)


def test_duplicate_station_cn(tc: TestContext):
    env.negative(extra_subject=[x509.NameAttribute(NameOID.COMMON_NAME, env.serial)])


def test_duplicate_organization(tc: TestContext):
    env.negative(extra_subject=[x509.NameAttribute(NameOID.ORGANIZATION_NAME, "Other")])


def test_wrong_key_usage(tc: TestContext):
    env.negative(usage=x509.KeyUsage(False, False, True, False, False, False, False, False, False))


def test_wrong_extended_usage(tc: TestContext):
    env.negative(eku=[ExtendedKeyUsageOID.SERVER_AUTH])


def test_client_extended_usage(tc: TestContext):
    req, csr = env.request()
    tc.assert_eq("Accepted", env.deliver(req, pem(env.certificate(csr, eku=[ExtendedKeyUsageOID.CLIENT_AUTH])))["status"])
    time.sleep(4)


def test_intermediate_missing_organization(tc: TestContext):
    req, csr = env.request()
    key = ec.generate_private_key(ec.SECP256R1())
    sub = env.certificate(csr, cn="Intermediate", organization=None, ca=True, public_key=key.public_key())
    leaf = env.certificate(csr, issuer=sub, issuer_key=key)
    env.reject_and_recover(req, csr, pem(leaf) + pem(sub))


def test_valid_intermediate_chain(tc: TestContext):
    req, csr = env.request()
    key = ec.generate_private_key(ec.SECP256R1())
    sub = env.certificate(csr, cn="Intermediate", ca=True, public_key=key.public_key())
    leaf = env.certificate(csr, issuer=sub, issuer_key=key)
    tc.assert_eq("Accepted", env.deliver(req, pem(leaf) + pem(sub))["status"])
    time.sleep(4)


def test_weak_intermediate_key(tc: TestContext):
    req, csr = env.request()
    key = rsa.generate_private_key(public_exponent=65537, key_size=1024)
    sub = env.certificate(csr, cn="Weak intermediate", ca=True, public_key=key.public_key())
    leaf = env.certificate(csr, issuer=sub, issuer_key=key)
    env.reject_and_recover(req, csr, pem(leaf) + pem(sub))


def test_a03_renewal_validation(tc: TestContext):
    tc.set_test_timeout(180)
    req, csr = env.request()
    # A real short-lived installation triggers the autonomous expiry scheduler.
    tc.assert_eq("Accepted", env.deliver(req, pem(env.certificate(csr, lifetime=300)))["status"])
    renewal, message = env.csms.expect("SignCertificate", timeout=60)
    env.csms.respond(message, {"status": "Accepted"})
    tc.assert_("hashRootCertificate" in renewal)
    tc.assert_(renewal["requestId"] != req["requestId"])
    new_csr = x509.load_pem_x509_csr(renewal["csr"].encode())
    tc.assert_(new_csr.is_signature_valid)
    env.reject_and_recover(renewal, new_csr, pem(env.certificate(new_csr, cn="wrong-renewal-station")))


def test_secc_clock_allowance_retained(tc: TestContext):
    tc.set_test_timeout(180)
    tc.assert_eq("Accepted", env.call("InstallCertificate", {
        "certificateType": "V2GRootCertificate", "certificate": pem(env.root)})["status"])
    tc.assert_eq("Accepted", env.call("TriggerMessage", {"requestedMessage": "SignV2GCertificate"})["status"])
    deadline = time.monotonic() + 60
    while True:
        req, message = env.csms.expect("SignCertificate", timeout=max(1, deadline - time.monotonic()))
        env.csms.respond(message, {"status": "Accepted"})
        if req["certificateType"] == "V2GCertificate" or time.monotonic() >= deadline:
            break
    tc.assert_eq("V2GCertificate", req["certificateType"])
    csr = x509.load_pem_x509_csr(req["csr"].encode())
    before = env.inventory()
    for future, status in ((600, "Rejected"), (240, "Accepted")):
        cert = env.certificate(csr, future=future)
        tc.assert_eq(status, env.call("CertificateSigned", {
            "certificateType": "V2GCertificate", "requestId": req["requestId"],
            "certificateChain": pem(cert)})["status"])
        if status == "Rejected":
            tc.assert_eq(before, env.inventory())


if __name__ == "__main__":
    run_testsuite(locals())
