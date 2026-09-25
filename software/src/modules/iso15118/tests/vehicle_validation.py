#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""Temporary-PKI device validation over Ethernet debug mode, with restoration."""

import base64
from datetime import datetime, timedelta, timezone
import json
from pathlib import Path
import socket
import ssl
import tempfile
import threading
import time

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509 import ocsp

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

import _common as common
import _vehicle_certificate as fixtures
import _vehicle_chain as vehicle
from _ocsp_gating import certificate_hash_data


class VehicleValidationEnvironment:
    def __init__(self, tc: TestContext):
        self.tc = tc
        self.host = tc._esp_host
        # Configuration backups and generated private keys must stay private.
        self.work = Path(tempfile.mkdtemp(prefix="device-vehicle-"))
        self.saved = {}
        self.csms_tls = None
        self.csms = None
        self.baseline = None
        self.variables = {}
        self.stop = threading.Event()
        self.responder = None
        self.results = []
        self.statuses = {}
        self.errors = []
        self.pki = {}
        self.debug_changed = False
        print(f"Evidence directory: {self.work}", flush=True)

    def record(self, label, detail=None):
        self.results.append({"test": label, "detail": detail})
        self.write_json("results.json", self.results)
        print("PASS", label, detail if detail is not None else "", flush=True)

    def write_json(self, name, data):
        (self.work / name).write_text(json.dumps(data, indent=2))

    def inventory(self):
        response = self.csms.call("GetInstalledCertificateIds", {})
        self.tc.assert_(response["status"] in ["Accepted", "NotFound"])
        return response.get("certificateHashDataChain", [])

    @staticmethod
    def identity(entry):
        return json.dumps([entry["certificateType"], entry["certificateHashData"]], sort_keys=True)

    def variable(self, name, value=None):
        item = {"component": {"name": "ISO15118Ctrlr"}, "variable": {"name": name}}
        if value is None:
            result = self.csms.call("GetVariables", {"getVariableData": [item]})["getVariableResult"][0]
            self.tc.assert_eq("Accepted", result["attributeStatus"])
            return result["attributeValue"]
        item["attributeValue"] = value
        result = self.csms.call("SetVariables", {"setVariableData": [item]})["setVariableResult"][0]
        self.tc.assert_eq("Accepted", result["attributeStatus"])

    def respond_status(self):
        while not self.stop.is_set():
            try:
                req, message = self.csms.expect("GetCertificateStatus", timeout=.5)
            except TimeoutError:
                continue
            except Exception as exc:
                self.errors.append(str(exc))
                return
            serial = req["ocspRequestData"]["serialNumber"].lower().lstrip("0")
            der = self.statuses.get(serial)
            self.csms.respond(message, {"status": "Accepted", "ocspResult": der} if der else {"status": "Failed"})

    def start(self):
        tc = self.tc
        self.iface = common.route_interface(self.host)
        for path in ["ocpp/config", "iso15118/config", "iso15118/debug", "certs/state", "info/version"]:
            self.saved[path] = tc.api(path, timeout=5)
        self.write_json("config-backup.json", self.saved)
        print("Device firmware:", self.saved["info/version"], flush=True)

        self.csms_tls = common.LocalCSMSTls(self.host, tc.get_local_ip())
        self.csms = common.CSMSSim(
            port=tc.find_free_port(9500),
            interactive=("SignCertificate", "GetCertificateStatus", "GetCertificateChainStatus", "NotifyEvent"),
            certfile=str(self.csms_tls.certfile), keyfile=str(self.csms_tls.keyfile))
        config = dict(self.saved["ocpp/config"])
        config.update(enable=True, protocol=1, url=f"wss://{tc.get_local_ip()}:{self.csms.port}",
                      enable_auth=True, **{"pass": "vehicle-validation-test-password", "cert_id": self.csms_tls.cert_id})
        tc.api("ocpp/config_update", config, timeout=5)
        tc.assert_(self.csms.connected.wait(60))
        time.sleep(2)
        self.baseline = self.inventory()
        self.write_json("inventory-before.json", self.baseline)
        for name in ["Enabled", "PrivateEnvironmentEnabled", "V2G20SECCLeafCryptoSuite", "V2GCertificateInstallationEnabled"]:
            self.variables[name] = self.variable(name)
        self.write_json("variables-backup.json", self.variables)
        self.variable("Enabled", "true")
        self.variable("PrivateEnvironmentEnabled", "false")
        self.variable("V2GCertificateInstallationEnabled", "true")
        self.variable("V2G20SECCLeafCryptoSuite", "ecdsa_secp521r1_sha512")

        for label, curve, kind in [("iso2", ec.SECP256R1(), "V2GRootCertificate"),
                                   ("iso20", ec.SECP521R1(), "V2GRootCertificate"),
                                   ("vehicle", ec.SECP521R1(), "OEMRootCertificate")]:
            key = ec.generate_private_key(curve)
            name = fixtures.name(f"Device test {label} root {self.work.name}", None)
            root = fixtures.issue(name, key, name, key, ca=True, path_length=2, source=None)
            self.pki[label] = (root, fixtures.pem(root), key)
            (self.work / f"{label}-root.pem").write_bytes(fixtures.pem(root))
            result = self.csms.call("InstallCertificate", {"certificateType": kind, "certificate": fixtures.pem(root).decode()}, timeout=90)
            tc.assert_eq("Accepted", result["status"])

        for label, kind in [("iso2", "V2GCertificate"), ("iso20", "V2G20Certificate")]:
            result = self.csms.call("TriggerMessage", {"requestedMessage": f"Sign{kind}"})
            tc.assert_eq("Accepted", result["status"])
            req, message = self.csms.expect("SignCertificate", timeout=90)
            self.csms.respond(message, {"status": "Accepted"})
            tc.assert_eq(kind, req["certificateType"])
            csr = x509.load_pem_x509_csr(req["csr"].encode())
            tc.assert_(csr.is_signature_valid)
            root, _, root_key = self.pki[label]
            now = datetime.now(timezone.utc)
            builder = (x509.CertificateBuilder().subject_name(csr.subject).issuer_name(root.subject)
                       .public_key(csr.public_key()).serial_number(x509.random_serial_number())
                       .not_valid_before(now - timedelta(minutes=5)).not_valid_after(now + timedelta(days=45))
                       .add_extension(x509.BasicConstraints(False, None), True)
                       .add_extension(x509.KeyUsage(True, False, False, False, False, False, False, False, False), True))
            if label == "iso20":
                builder = builder.add_extension(x509.AuthorityInformationAccess([x509.AccessDescription(
                    fixtures.AIA.OCSP, x509.UniformResourceIdentifier("http://ocsp.vehicle.test/secc"))]), False)
            leaf = builder.sign(root_key, hashes.SHA512() if label == "iso20" else hashes.SHA256())
            (self.work / f"{label}-secc.pem").write_bytes(fixtures.pem(leaf))
            if label == "iso20":
                response = (ocsp.OCSPResponseBuilder().add_response(leaf, root, hashes.SHA256(),
                    ocsp.OCSPCertStatus.GOOD, now - timedelta(minutes=1), now + timedelta(days=1), None, None)
                    .responder_id(ocsp.OCSPResponderEncoding.HASH, root).certificates([root]).sign(root_key, hashes.SHA512()))
                self.statuses[format(leaf.serial_number, "x")] = base64.b64encode(response.public_bytes(serialization.Encoding.DER)).decode()
            result = self.csms.call("CertificateSigned", {"certificateType": kind, "requestId": req["requestId"],
                                                        "certificateChain": fixtures.pem(leaf).decode()}, timeout=120)
            tc.assert_eq("Accepted", result["status"])
        self.responder = threading.Thread(target=self.respond_status, daemon=True)
        self.responder.start()
        tc.api("iso15118/config", dict(self.saved["iso15118/config"], charge_via_iso15118=True), timeout=5)
        self.debug_changed = True
        common.enable_debug_mode(self.host)
        time.sleep(8)
        # CSMSSim's expect/pending queue is single-consumer. Fresh SECC OCSP is
        # valid for this run; stop its consumer before awaiting M07.
        self.stop.set()
        self.responder.join(timeout=5)
        tc.assert_not(self.responder.is_alive())
        tc.assert_eq([], self.errors)
        mac = tc.api("ethernet/state", timeout=5)["mac"]
        octets = bytes.fromhex(mac.replace(":", ""))
        iid = bytes([octets[0] ^ 2]) + octets[1:3] + b"\xff\xfe" + octets[3:]
        self.target_ll = socket.inet_ntop(socket.AF_INET6, bytes.fromhex("fe80000000000000") + iid)
        print("Target IPv6:", self.target_ll, flush=True)
        self.record("Profile 2 root/CSR/SECC provisioning", self.csms.tls_connections[-1]["version"])
        self.sub1_key, self.sub2_key, self.leaf_key = [ec.generate_private_key(ec.SECP521R1()) for _ in range(3)]
        fixtures.write_key(self.work / "vehicle.key", self.leaf_key)

    def connect(self, cert, *, tls12=False):
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        context.minimum_version = context.maximum_version = ssl.TLSVersion.TLSv1_2 if tls12 else ssl.TLSVersion.TLSv1_3
        context.check_hostname = False
        context.load_verify_locations(cadata=self.pki["iso2" if tls12 else "iso20"][1].decode())
        if tls12:
            context.set_ciphers("ECDHE-ECDSA-AES128-SHA256")
        else:
            context.set_ecdh_curve("secp521r1")
            if cert is not None:
                context.load_cert_chain(str(cert), str(self.work / "vehicle.key"))
        response = common.sdp_request(self.iface, expected_from=self.target_ll)
        self.tc.assert_(response is not None and response["security"] == common.SDP_SECURITY_TLS)
        raw = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
        raw.settimeout(45)
        try:
            raw.connect((response["secc_ll"], response["port"], 0, socket.if_nametoindex(self.iface)))
            return context.wrap_socket(raw)
        except Exception:
            raw.close()
            raise

    def chain(self, label, leaf_opts=None, sub1_opts=None, sub2_opts=None, leaf_role="EV", sub1_role="EV", forged=False):
        root, _, root_key = self.pki["vehicle"]
        n1, n2 = fixtures.name("Sub1", [sub1_role]), fixtures.name("Sub2", ["EV"])
        sub1 = fixtures.issue(n1, self.sub1_key, root.subject,
            ec.generate_private_key(ec.SECP521R1()) if forged else root_key,
            **(dict(ca=True, path_length=1) | (sub1_opts or {})))
        sub2 = fixtures.issue(n2, self.sub2_key, n1, self.sub1_key, **(dict(ca=True, path_length=0) | (sub2_opts or {})))
        leaf = fixtures.issue(fixtures.name("EVCCID", [leaf_role]), self.leaf_key, n2, self.sub2_key,
                              **(dict(eku=[fixtures.EKU.CLIENT_AUTH]) | (leaf_opts or {})))
        path = self.work / f"vehicle-{label}.pem"
        path.write_bytes(b"".join(fixtures.pem(c) for c in [leaf, sub2, sub1]))
        return path, [leaf, sub2, sub1]

    def negative(self, label, path, expected_alert):
        error = None
        start = time.monotonic()
        try:
            with self.connect(path) as tls:
                vehicle.sap_iso20(tls)
        except ssl.SSLError as exc:
            error = str(exc)
        # EOF, timeouts, and malformed SAP responses must fail the runner case,
        # rather than count as successful certificate rejection.
        try:
            unexpected = self.csms.expect("GetCertificateChainStatus", timeout=1)
        except TimeoutError:
            unexpected = None
        if unexpected is not None:
            req, message = unexpected
            self.csms.respond(message, vehicle.chain_status_response(req, ["Unknown"] * 3))
        self.tc.assert_eq(None, unexpected)
        self.tc.assert_(error is not None)
        self.tc.assert_("ALERT" in error.upper())
        self.tc.assert_(expected_alert in error.upper())
        self.record(label, {"error": error, "seconds": round(time.monotonic() - start, 3)})
        time.sleep(1)

    def positive(self, label, path, certificates, statuses=None, *, allow_cached=False):
        start = time.monotonic()
        with self.connect(path) as tls:
            vehicle.sap_iso20(tls)
            elapsed = time.monotonic() - start
            try:
                request = self.csms.expect("GetCertificateChainStatus", timeout=2 if allow_cached else 30)
            except TimeoutError:
                if not allow_cached:
                    raise
                request = None
            if request is not None:
                req, message = request
                root = self.pki["vehicle"][0]
                expected_hashes = [certificate_hash_data(c, issuer) for c, issuer in zip(certificates, certificates[1:] + [root])]
                actual = req["certificateStatusRequests"]
                self.tc.assert_eq(3, len(actual))
                for entry, expected_hash in zip(actual, expected_hashes):
                    self.tc.assert_eq(expected_hash, entry["certificateHashData"])
                    self.tc.assert_eq("OCSP", entry["source"])
                    self.tc.assert_eq([fixtures.URL], entry["urls"])
            session = vehicle.session_setup(tls)
            vehicle.authorization_setup(tls, session)
            status = statuses or ["Good"] * 3
            if request is not None:
                pending = vehicle.authorization(tls, session)
                self.tc.assert_eq("Ongoing", pending["EVSEProcessing"])
                self.csms.respond(message, vehicle.chain_status_response(req, status))
            final = vehicle.final_authorization(tls, session)
            if all(s == "Good" for s in status):
                self.tc.assert_eq("OK", final["ResponseCode"])
                self.tc.assert_eq("Finished", final["EVSEProcessing"])
            else:
                self.tc.assert_(final["ResponseCode"].startswith("FAILED"))
                self.tc.assert_(vehicle.tls_closed(tls))
            self.record(label, {"tls_and_sap_seconds": round(elapsed, 3), "authorization": final["ResponseCode"]})
        time.sleep(2)

    def close(self):
        cleanup_errors = []

        def attempt(label, fn):
            try:
                fn()
            except Exception as exc:
                cleanup_errors.append(f"{label}: {exc}")

        attempt("trace", lambda: (self.work / "trace.log").write_bytes(
            self.tc.http_request("GET", "/trace_log", timeout=15)))
        if self.responder:
            self.stop.set()
            self.responder.join(timeout=5)
            if self.responder.is_alive():
                cleanup_errors.append("OCSP responder did not stop")

        def restore_inventory():
            old = {self.identity(e) for e in self.baseline}
            added = [e for e in self.inventory() if self.identity(e) not in old]
            added.sort(key=lambda e: 0 if e["certificateType"] == "V2GCertificateChain" else 1)
            for entry in added:
                def delete(entry=entry):
                    result = self.csms.call("DeleteCertificate", {"certificateHashData": entry["certificateHashData"]}, timeout=60)
                    self.tc.assert_eq("Accepted", result["status"])
                attempt("delete certificate", delete)
            current = self.inventory()
            self.write_json("inventory-after.json", current)
            self.tc.assert_eq(old, {self.identity(e) for e in current})

        if self.debug_changed:
            attempt("disable debug", lambda: common.disable_debug_mode(self.host))
        if self.csms and self.baseline is not None:
            attempt("OCPP inventory", restore_inventory)
            for name, value in self.variables.items():
                def restore_variable(name=name, value=value):
                    self.variable(name, value)
                    self.tc.assert_eq(value, self.variable(name))
                attempt(name, restore_variable)
        for path, target in [("iso15118/debug", "iso15118/debug_update"), ("iso15118/config", "iso15118/config"),
                             ("ocpp/config", "ocpp/config_update")]:
            if path in self.saved:
                def restore_config(path=path, target=target):
                    self.tc.api(target, self.saved[path], timeout=5)
                    self.tc.assert_eq(self.saved[path], self.tc.api(path, timeout=5))
                attempt(path, restore_config)
        if self.csms:
            attempt("CSMS stop", self.csms.stop)
        if self.csms_tls:
            attempt("CSMS trust cleanup", self.csms_tls.close)
        if "certs/state" in self.saved:
            attempt("CSMS trust restored", lambda: self.tc.assert_eq(self.saved["certs/state"], self.tc.api("certs/state", timeout=5)))
        self.write_json("cleanup.json", {"errors": cleanup_errors})
        self.tc.assert_eq([], cleanup_errors)
        self.record("original inventory, variables, API configuration and CSMS trust restored")


environment = None


def suite_setup(tc: TestContext):
    global environment
    if tc._esp_host is None:
        tc.skip("ESP Host not passed")
    # Check the running firmware, not config_type: development bricks can run
    # WARP4 firmware while retaining their original brick configuration.
    if not tc.device_type().is_warp(4):
        tc.skip("Vehicle validation requires WARP4 firmware")
    tc.set_test_timeout(900)
    environment = VehicleValidationEnvironment(tc)
    environment.start()


def suite_teardown(tc: TestContext):
    if environment is not None:
        tc.set_test_timeout(600)
        environment.close()


def setup(tc: TestContext):
    tc.assert_(environment is not None)
    tc.assert_eq([], environment.errors)


def teardown(tc: TestContext):
    # Recover the debug session even after a failed assertion, so the next
    # filtered case does not inherit an unfinished authorization exchange.
    tc.set_test_timeout(60)
    common.disable_debug_mode(environment.host)
    time.sleep(.2)
    common.enable_debug_mode(environment.host)
    tc.wait_for(lambda: tc.assert_(common.sdp_request(environment.iface, expected_from=environment.target_ll) is not None), timeout=30)


def test_valid_chain_and_authorization(tc: TestContext):
    tc.set_test_timeout(120)
    path, certs = environment.chain("good")
    environment.positive("valid vehicle + full-chain M07 + pending/Good authorization", path, certs)


def test_same_leaf_changed_intermediate_then_recovery(tc: TestContext):
    tc.set_test_timeout(180)
    good, certs = environment.chain("cache-good")
    environment.positive("valid chain before changed intermediate", good, certs)
    forged, _ = environment.chain("cache-forged", forged=True)
    parts = good.read_bytes().split(b"-----BEGIN CERTIFICATE-----")
    forged_parts = forged.read_bytes().split(b"-----BEGIN CERTIFICATE-----")
    forged.write_bytes(b"-----BEGIN CERTIFICATE-----".join(parts[:3] + forged_parts[3:]))
    environment.negative("same leaf changed intermediate signature rejected", forged, "UNKNOWN_CA")
    # Reuse the exact original chain. Its Good revocation result may be cached,
    # but every connection above still performs a fresh full TLS handshake.
    environment.positive("same valid chain recovers after forged intermediate", good, certs, allow_cached=True)


def test_missing_client_identity(tc: TestContext):
    tc.set_test_timeout(90)
    environment.negative("missing client identity rejected", None, "CERTIFICATE_REQUIRED")


def test_revoked_intermediate(tc: TestContext):
    tc.set_test_timeout(120)
    path, certs = environment.chain("revoked")
    environment.positive("revoked intermediate blocks authorization", path, certs, ["Good", "Revoked", "Good"])


def test_missing_status(tc: TestContext):
    tc.set_test_timeout(120)
    path, certs = environment.chain("missing-status")
    environment.positive("missing status blocks authorization", path, certs, [None, "Good", "Good"])


def test_iso2_tls12_and_sap(tc: TestContext):
    tc.set_test_timeout(90)
    from iso15118.shared.messages.enums import Namespace
    with environment.connect(None, tls12=True) as tls:
        result = vehicle.exchange(tls, "supportedAppProtocolReq", {"AppProtocol": [common.ISO2]}, Namespace.SAP, 0x8001)
        tc.assert_eq("OK_SuccessfulNegotiation", result["supportedAppProtocolRes"]["ResponseCode"])
        environment.record("ISO-2 TLS 1.2 + SAP", tls.cipher())


def missing_root_lifecycle(tc: TestContext, bundled_root: bool):
    # TC_HU_SECC_ISO20_Install_Leaf_Certificate_Without_Trusted_Root_001;
    # OCPP A02.FR.06, A02.FR.07, M03, M04 and N07.
    tc.set_test_timeout(240)
    env = environment
    csms = env.csms
    root, root_pem, root_key = env.pki["iso20"]
    root_hash = certificate_hash_data(root, root)
    before = env.inventory()
    before_ids = {env.identity(entry) for entry in before}
    root_id = env.identity({"certificateType": "V2GRootCertificate", "certificateHashData": root_hash})
    tc.assert_(root_id in before_ids)
    old_leaf = x509.load_pem_x509_certificate((env.work / "iso20-secc.pem").read_bytes())
    iso2_leaf = x509.load_pem_x509_certificate((env.work / "iso2-secc.pem").read_bytes())
    old_chain_id = env.identity({"certificateType": "V2GCertificateChain",
                                 "certificateHashData": certificate_hash_data(old_leaf, root)})
    for cert, issuer in ((old_leaf, root), (iso2_leaf, env.pki["iso2"][0])):
        tc.assert_(env.identity({"certificateType": "V2GCertificateChain",
                                "certificateHashData": certificate_hash_data(cert, issuer)}) in before_ids)

    path, certs = env.chain(f"root-deletion-{bundled_root}")
    security_before = len(csms.security_events)
    event_ids = []
    pending = False
    deleted = False
    payload = None
    try:
        result = csms.call("DeleteCertificate", {"certificateHashData": root_hash})
        tc.assert_eq("Accepted", result["status"])
        deleted = True
        after = env.inventory()
        # The dependent ISO-20 chain is deleted, preserving the ISO-2 identity.
        tc.assert_eq(before_ids - {root_id, old_chain_id}, {env.identity(entry) for entry in after})
        count_request = {"getVariableData": [{"component": {"name": "SecurityCtrlr"},
                                             "variable": {"name": "CertificateEntries"}}]}
        count = csms.call("GetVariables", count_request)["getVariableResult"][0]
        tc.assert_eq("Accepted", count["attributeStatus"])
        tc.assert_eq("Accepted", csms.call("TriggerMessage", {"requestedMessage": "SignV2G20Certificate"})["status"])
        pending = True
        request, mid = csms.expect("SignCertificate", timeout=60)
        csms.respond(mid, {"status": "Accepted"})
        tc.assert_eq("V2G20Certificate", request["certificateType"])
        csr = x509.load_pem_x509_csr(request["csr"].encode())
        tc.assert_(csr.is_signature_valid)
        tc.assert_(isinstance(csr.public_key(), ec.EllipticCurvePublicKey))
        tc.assert_eq("secp521r1", csr.public_key().curve.name)
        tc.assert_eq(old_leaf.subject, csr.subject)
        now = datetime.now(timezone.utc)
        replacement = (x509.CertificateBuilder().subject_name(csr.subject).issuer_name(root.subject)
                       .public_key(csr.public_key()).serial_number(x509.random_serial_number())
                       .not_valid_before(now - timedelta(minutes=5)).not_valid_after(now + timedelta(days=45))
                       .add_extension(x509.BasicConstraints(False, None), True)
                       .add_extension(x509.KeyUsage(True, False, False, False, False, False, False, False, False), True)
                       .add_extension(x509.AuthorityInformationAccess([x509.AccessDescription(
                           fixtures.AIA.OCSP, x509.UniformResourceIdentifier("http://ocsp.vehicle.test/secc"))]), False)
                       .sign(root_key, hashes.SHA512()))
        chain = fixtures.pem(replacement) + (root_pem if bundled_root else b"")
        payload = {"certificateType": "V2G20Certificate", "requestId": request["requestId"],
                   "certificateChain": chain.decode()}
        for _ in range(2):
            rejected = csms.call("CertificateSigned", payload)
            tc.assert_eq("Rejected", rejected["status"])
            tc.assert_(rejected["statusInfo"]["reasonCode"] in ("NoTrustedRoot", "UntrustedChain"))
            event, mid = csms.expect("NotifyEvent", timeout=10)
            csms.respond(mid, {})
            tc.assert_eq(0, event["seqNo"])
            tc.assert_eq(False, event.get("tbc", False))
            tc.assert_eq(1, len(event["eventData"]))
            data = event["eventData"][0]
            tc.assert_eq({"name": "SecurityCtrlr"}, data["component"])
            tc.assert_eq({"name": "CertificateEntries"}, data["variable"])
            tc.assert_eq(count["attributeValue"], data["actualValue"])
            tc.assert_eq("Alerting", data["trigger"])
            tc.assert_eq("HardWiredNotification", data["eventNotificationType"])
            tc.assert_eq(3, data["severity"])
            tc.assert_eq(rejected["statusInfo"]["reasonCode"], data["techCode"])
            tc.assert_eq("V2GCertificateChain installation failed because the corresponding V2G root was not found.", data["techInfo"])
            tc.assert_("variableMonitoringId" not in data and "cleared" not in data)
            tc.assert_(isinstance(data["eventId"], int) and data["eventId"] >= 0)
            event_ids.append(data["eventId"])
            for stamp in (event["generatedAt"], data["timestamp"]):
                tc.assert_(abs((datetime.now(timezone.utc) - datetime.fromisoformat(stamp.replace("Z", "+00:00"))).total_seconds()) < 120)
            tc.assert_eq(after, env.inventory())
        tc.assert_(event_ids[0] != event_ids[1])
        tc.assert_eq(security_before, len(csms.security_events))

        # With the ISO-20 identity removed, complete TLS 1.2 and ISO-2 SAP
        # using the exact unaffected ISO-2 identity.
        from iso15118.shared.messages.enums import Namespace
        with env.connect(None, tls12=True) as tls:
            tc.assert_eq(iso2_leaf.public_bytes(serialization.Encoding.DER), tls.getpeercert(binary_form=True))
            result = vehicle.exchange(tls, "supportedAppProtocolReq", {"AppProtocol": [common.ISO2]}, Namespace.SAP, 0x8001)
            tc.assert_eq("OK_SuccessfulNegotiation", result["supportedAppProtocolRes"]["ResponseCode"])
        tc.assert_eq(security_before, len(csms.security_events))
        env.record("missing-root lifecycle", {"bundled_root": bundled_root, "event_ids": event_ids,
                   "deleted_leaf_serial": format(old_leaf.serial_number, "x"),
                   "rejected_leaf_serial": format(replacement.serial_number, "x")})
    finally:
        # Independent root provisioning makes the rejected enrollment usable.
        if deleted:
            tc.assert_eq("Accepted", csms.call("InstallCertificate", {
                "certificateType": "V2GRootCertificate", "certificate": root_pem.decode()})["status"])
        if pending and payload is not None:
            tc.assert_eq("Accepted", csms.call("CertificateSigned", payload)["status"])
            now = datetime.now(timezone.utc)
            response = (ocsp.OCSPResponseBuilder().add_response(replacement, root, hashes.SHA256(),
                ocsp.OCSPCertStatus.GOOD, now - timedelta(minutes=1), now + timedelta(days=1), None, None)
                .responder_id(ocsp.OCSPResponderEncoding.HASH, root).certificates([root]).sign(root_key, hashes.SHA512()))
            env.statuses[format(replacement.serial_number, "x")] = base64.b64encode(response.public_bytes(serialization.Encoding.DER)).decode()
            (env.work / "iso20-secc.pem").write_bytes(fixtures.pem(replacement))
            req, mid = csms.expect("GetCertificateStatus", timeout=30)
            tc.assert_eq(format(replacement.serial_number, "x"), req["ocspRequestData"]["serialNumber"].lower().lstrip("0"))
            csms.respond(mid, {"status": "Accepted", "ocspResult": env.statuses[format(replacement.serial_number, "x")]})
            replacement_id = env.identity({"certificateType": "V2GCertificateChain",
                                           "certificateHashData": certificate_hash_data(replacement, root)})
            tc.assert_eq(before_ids - {old_chain_id} | {replacement_id}, {env.identity(entry) for entry in env.inventory()})
        elif pending:
            tc.assert_eq("Accepted", csms.call("TriggerMessage", {"requestedMessage": "SignV2G20Certificate"})["status"])
            _, mid = csms.expect("SignCertificate", timeout=60)
            csms.respond(mid, {"status": "Rejected"})

    time.sleep(2)
    env.positive("ISO-20 recovery after independent root provisioning", path, certs)
    tc.assert_eq(security_before, len(csms.security_events))


def test_missing_root_lifecycle_root_free(tc: TestContext):
    missing_root_lifecycle(tc, False)


def test_missing_root_lifecycle_bundled_root(tc: TestContext):
    missing_root_lifecycle(tc, True)


def generate_tests():
    cases = [
        ("contract_leaf", {"leaf_role": "MSP"}, "ACCESS_DENIED"),
        ("provisioning_leaf", {"leaf_role": "OEM"}, "ACCESS_DENIED"),
        ("contract_intermediate", {"sub1_role": "MSP"}, "ACCESS_DENIED"),
        ("missing_leaf_source", {"leaf_opts": {"source": None}}, "ACCESS_DENIED"),
        ("missing_intermediate_source", {"sub2_opts": {"source": None}}, "ACCESS_DENIED"),
        ("crl_only_source", {"leaf_opts": {"source": "crl"}}, "ACCESS_DENIED"),
        ("expired_leaf", {"leaf_opts": {"expired": True}}, "CERTIFICATE_EXPIRED"),
        ("future_leaf", {"leaf_opts": {"future": True}}, "CERTIFICATE_UNKNOWN"),
        ("expired_intermediate", {"sub1_opts": {"expired": True}}, "CERTIFICATE_EXPIRED"),
        ("future_intermediate", {"sub2_opts": {"future": True}}, "CERTIFICATE_UNKNOWN"),
        ("server_only_usage", {"leaf_opts": {"eku": [fixtures.EKU.SERVER_AUTH]}}, "UNSUPPORTED_CERTIFICATE"),
    ]
    tests = {}
    for label, options, alert in cases:
        def reject(tc: TestContext, label=label, options=options, alert=alert):
            tc.set_test_timeout(90)
            path, _ = environment.chain(label, **options)
            environment.negative(label + " rejected", path, alert)
        tests["test_" + label] = reject
    return tests


if __name__ == "__main__":
    run_testsuite(dict(locals(), **generate_tests()))
