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
                       .add_extension(x509.KeyUsage(True, False, False, False, label == "iso2", False, False, False, False), True))
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

    def connect(self, cert, *, tls12=False, message_callback=None):
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        if message_callback is not None:
            context._msg_callback = message_callback
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

    def chain(self, label, leaf_opts=None, sub1_opts=None, sub2_opts=None, leaf_role="EV", sub1_role="EV", forged=False,
              leaf_name=None, sub1_name=None, sub2_name=None):
        root, _, root_key = self.pki["vehicle"]
        n1 = sub1_name if sub1_name is not None else fixtures.name("Sub1", [sub1_role])
        n2 = sub2_name if sub2_name is not None else fixtures.name("Sub2", ["EV"])
        sub1 = fixtures.issue(n1, self.sub1_key, root.subject,
            ec.generate_private_key(ec.SECP521R1()) if forged else root_key,
            **(dict(ca=True, path_length=1) | (sub1_opts or {})))
        sub2 = fixtures.issue(n2, self.sub2_key, n1, self.sub1_key, **(dict(ca=True, path_length=0) | (sub2_opts or {})))
        leaf = fixtures.issue(leaf_name if leaf_name is not None else fixtures.name(fixtures.evccid(), [leaf_role]), self.leaf_key, n2, self.sub2_key,
                              **(dict(eku=[fixtures.EKU.CLIENT_AUTH]) | (leaf_opts or {})))
        path = self.work / f"vehicle-{label}.pem"
        path.write_bytes(b"".join(fixtures.pem(c) for c in [leaf, sub2, sub1]))
        return path, [leaf, sub2, sub1]

    def negative(self, label, path, expected_alert):
        error = None
        start = time.monotonic()
        try:
            with self.connect(path) as tls:
                # TLS 1.3 clients can return before the server has validated
                # their certificate flight. Read its rejection directly:
                # writing SAP can race close and mask the alert with EOF.
                self.tc.assert_eq(b"", tls.recv(1))
        except ssl.SSLError as exc:
            error = str(exc)
            print(f"TLS rejection for {label}: {error}")
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

    def positive(self, label, path, certificates, statuses=None, *, allow_cached=False, expected_server=None):
        start = time.monotonic()
        with self.connect(path) as tls:
            if expected_server is not None:
                self.tc.assert_eq(expected_server.public_bytes(serialization.Encoding.DER), tls.getpeercert(binary_form=True))
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
                self.tc.assert_eq(len(certificates), len(actual))
                for entry, expected_hash in zip(actual, expected_hashes):
                    self.tc.assert_eq(expected_hash, entry["certificateHashData"])
                    self.tc.assert_eq("OCSP", entry["source"])
                    self.tc.assert_eq([fixtures.URL], entry["urls"])
            session = vehicle.session_setup(tls)
            vehicle.authorization_setup(tls, session)
            status = statuses or ["Good"] * len(certificates)
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


def test_automatic_renewal_after_five_minute_expiry(tc: TestContext):
    # TC_HU_SECC_ISO20_Install_SECC_Cert_Chain_Expiry_5_Minutes_001;
    # OCPP A03.FR.02/A03.FR.23: automatic enrollment survives old-chain expiry.
    tc.set_test_timeout(480)
    env = environment
    csms = env.csms
    root, _, root_key = env.pki["iso20"]
    old_leaf = x509.load_pem_x509_certificate((env.work / "iso20-secc.pem").read_bytes())
    old_hash = certificate_hash_data(old_leaf, root)
    before = {env.identity(entry) for entry in env.inventory()}
    old_id = env.identity({"certificateType": "V2GCertificateChain", "certificateHashData": old_hash})
    wait_variable = {"component": {"name": "SecurityCtrlr"}, "variable": {"name": "CertSigningWaitMinimum"}}
    saved_wait = csms.call("GetVariables", {"getVariableData": [wait_variable]})["getVariableResult"][0]
    tc.assert_eq("Accepted", saved_wait["attributeStatus"])

    def set_wait(value):
        result = csms.call("SetVariables", {"setVariableData": [dict(wait_variable, attributeValue=value)]})
        tc.assert_eq("Accepted", result["setVariableResult"][0]["attributeStatus"])

    def issue(request, start, end):
        csr = x509.load_pem_x509_csr(request["csr"].encode())
        tc.assert_(csr.is_signature_valid)
        tc.assert_eq(old_leaf.subject, csr.subject)
        tc.assert_eq("secp521r1", csr.public_key().curve.name)
        return (x509.CertificateBuilder().subject_name(csr.subject).issuer_name(root.subject)
                .public_key(csr.public_key()).serial_number(x509.random_serial_number())
                .not_valid_before(start).not_valid_after(end)
                .add_extension(x509.BasicConstraints(False, None), True)
                .add_extension(x509.KeyUsage(True, False, False, False, False, False, False, False, False), True)
                .add_extension(x509.AuthorityInformationAccess([x509.AccessDescription(
                    fixtures.AIA.OCSP, x509.UniformResourceIdentifier("http://ocsp.vehicle.test/secc"))]), False)
                .sign(root_key, hashes.SHA512()))

    def install(request, leaf):
        result = csms.call("CertificateSigned", {"certificateType": "V2G20Certificate",
                          "requestId": request["requestId"], "certificateChain": fixtures.pem(leaf).decode()})
        tc.assert_eq("Accepted", result["status"])
        now = datetime.now(timezone.utc)
        response = (ocsp.OCSPResponseBuilder().add_response(leaf, root, hashes.SHA256(),
            ocsp.OCSPCertStatus.GOOD, now - timedelta(minutes=1), now + timedelta(days=1), None, None)
            .responder_id(ocsp.OCSPResponderEncoding.HASH, root).certificates([root]).sign(root_key, hashes.SHA512()))
        status, mid = csms.expect("GetCertificateStatus", timeout=30)
        tc.assert_eq(certificate_hash_data(leaf, root), {k: status["ocspRequestData"][k] for k in certificate_hash_data(leaf, root)})
        csms.respond(mid, {"status": "Accepted", "ocspResult": base64.b64encode(response.public_bytes(serialization.Encoding.DER)).decode()})
        (env.work / "iso20-secc.pem").write_bytes(fixtures.pem(leaf))
        leaf_id = env.identity({"certificateType": "V2GCertificateChain", "certificateHashData": certificate_hash_data(leaf, root)})
        tc.assert_eq(before - {old_id} | {leaf_id}, {env.identity(entry) for entry in env.inventory()})

    pending = False
    try:
        # Isolate expiry from the independent CSR retry/backoff mechanism.
        set_wait("600")
        tc.assert_eq("Accepted", csms.call("TriggerMessage", {"requestedMessage": "SignV2G20Certificate"})["status"])
        pending = True
        request, mid = csms.expect("SignCertificate", timeout=60)
        csms.respond(mid, {"status": "Accepted"})
        now = datetime.now(timezone.utc).replace(microsecond=0)
        short_leaf = issue(request, now - timedelta(seconds=5), now + timedelta(seconds=295))
        installed_at = time.monotonic()
        install(request, short_leaf)
        pending = False
        renewal, mid = csms.expect("SignCertificate", timeout=30)
        pending = True
        elapsed = time.monotonic() - installed_at
        tc.assert_eq("V2G20Certificate", renewal["certificateType"])
        tc.assert_(renewal["requestId"] != request["requestId"])
        tc.assert_eq(certificate_hash_data(root, root), renewal["hashRootCertificate"])
        csms.respond(mid, {"status": "Accepted"})
        env.record("automatic five-minute renewal CSR", {"seconds_after_install": round(elapsed, 3),
                   "not_after": short_leaf.not_valid_after_utc.isoformat(), "request_id": renewal["requestId"]})

        # Real wall-clock expiry; no device clock manipulation or reconnect.
        while datetime.now(timezone.utc) <= short_leaf.not_valid_after_utc + timedelta(seconds=2):
            time.sleep(min(10, max(0.1, (short_leaf.not_valid_after_utc + timedelta(seconds=3) - datetime.now(timezone.utc)).total_seconds())))
        tc.assert_(datetime.now(timezone.utc) > short_leaf.not_valid_after_utc)
        replacement = issue(renewal, datetime.now(timezone.utc) - timedelta(seconds=1), datetime.now(timezone.utc) + timedelta(days=45))
        install(renewal, replacement)
        pending = False
        path, certs = env.chain("automatic-renewal")
        time.sleep(2)
        env.positive("ISO-20 authorization after old-chain expiry and automatic renewal", path, certs)
        env.record("five-minute expiry renewal replaced old inventory", {
            "expired_serial": format(short_leaf.serial_number, "x"),
            "replacement_serial": format(replacement.serial_number, "x")})
    finally:
        if pending:
            tc.assert_eq("Accepted", csms.call("TriggerMessage", {"requestedMessage": "SignV2G20Certificate"})["status"])
            _, mid = csms.expect("SignCertificate", timeout=60)
            csms.respond(mid, {"status": "Rejected"})
        set_wait(saved_wait["attributeValue"])
        tc.assert_eq(saved_wait, csms.call("GetVariables", {"getVariableData": [wait_variable]})["getVariableResult"][0])


def test_seven_day_renewal_with_sub_ca_inventory(tc: TestContext):
    # TC_HU_SECC_ISO20_Install_SECC_Cert_Chain_Expiry_7_Days_001;
    # OCPP A03.FR.02/A03.FR.23 and M03: replace one complete SECC chain.
    import _certificate_profiles as profiles

    tc.set_test_timeout(180)
    env = environment
    csms = env.csms
    root, _, root_key = env.pki["iso20"]
    old_leaf = x509.load_pem_x509_certificate((env.work / "iso20-secc.pem").read_bytes())
    old_id = env.identity({"certificateType": "V2GCertificateChain",
                           "certificateHashData": certificate_hash_data(old_leaf, root)})
    before = env.inventory()
    independent = [entry for entry in before if env.identity(entry) != old_id]
    connection_count = csms.connection_count
    security_count = len(csms.security_events)
    now = datetime.now(timezone.utc).replace(microsecond=0)
    sub1_key, sub2_key = [ec.generate_private_key(ec.SECP521R1()) for _ in range(2)]
    sub1 = profiles.certificate(profiles.name("Renewal Sub CA 1"), sub1_key, root.subject, root_key,
                                x509.random_serial_number(), now - timedelta(days=1), now + timedelta(days=90),
                                hashes.SHA512(), path_length=1)
    sub2 = profiles.certificate(profiles.name("Renewal Sub CA 2"), sub2_key, sub1.subject, sub1_key,
                                x509.random_serial_number(), now - timedelta(days=1), now + timedelta(days=60),
                                hashes.SHA512(), path_length=0)

    def issue(request, start, end):
        tc.assert_eq("V2G20Certificate", request["certificateType"])
        csr = x509.load_pem_x509_csr(request["csr"].encode())
        tc.assert_(csr.is_signature_valid)
        tc.assert_eq(old_leaf.subject, csr.subject)
        tc.assert_eq("secp521r1", csr.public_key().curve.name)
        return (x509.CertificateBuilder().subject_name(csr.subject).issuer_name(sub2.subject)
                .public_key(csr.public_key()).serial_number(x509.random_serial_number())
                .not_valid_before(start).not_valid_after(end)
                .add_extension(x509.BasicConstraints(False, None), True)
                .add_extension(profiles.key_usage(ca=False), True)
                .add_extension(x509.AuthorityKeyIdentifier.from_issuer_public_key(sub2_key.public_key()), False)
                .add_extension(x509.SubjectKeyIdentifier.from_public_key(csr.public_key()), False)
                .add_extension(x509.ExtendedKeyUsage([fixtures.EKU.SERVER_AUTH]), True)
                .add_extension(x509.AuthorityInformationAccess([x509.AccessDescription(
                    fixtures.AIA.OCSP, x509.UniformResourceIdentifier(profiles.OCSP_URL))]), False)
                .sign(sub2_key, hashes.SHA512()))

    def install(request, leaf):
        chain = [leaf, sub2, sub1]
        hashes_expected = [certificate_hash_data(cert, issuer) for cert, issuer in
                           zip(chain, [sub2, sub1, root])]
        result = csms.call("CertificateSigned", {"certificateType": "V2G20Certificate",
            "requestId": request["requestId"], "certificateChain": b"".join(fixtures.pem(c) for c in chain).decode()})
        tc.assert_eq("Accepted", result["status"])
        expected_entry = {"certificateType": "V2GCertificateChain", "certificateHashData": hashes_expected[0],
                          "childCertificateHashData": hashes_expected[1:]}
        actual = env.inventory()
        tc.assert_eq(sorted(independent + [expected_entry], key=env.identity), sorted(actual, key=env.identity))
        filtered = csms.call("GetInstalledCertificateIds", {"certificateType": ["V2GCertificateChain"]})
        tc.assert_eq("Accepted", filtered["status"])
        tc.assert_eq(sorted([e for e in actual if e["certificateType"] == "V2GCertificateChain"], key=env.identity),
                     sorted(filtered["certificateHashDataChain"], key=env.identity))

        # M06 requests and Good responses cover all three non-root certificates.
        responses = {}
        stamp = datetime.now(timezone.utc)
        for cert, issuer, key, hash_data in zip(chain, [sub2, sub1, root], [sub2_key, sub1_key, root_key], hashes_expected):
            response = (ocsp.OCSPResponseBuilder().add_response(cert, issuer, hashes.SHA256(),
                ocsp.OCSPCertStatus.GOOD, stamp - timedelta(minutes=1), stamp + timedelta(days=1), None, None)
                .responder_id(ocsp.OCSPResponderEncoding.HASH, issuer).certificates([issuer]).sign(key, hashes.SHA512()))
            responses[json.dumps(hash_data, sort_keys=True)] = base64.b64encode(response.public_bytes(serialization.Encoding.DER)).decode()
        while responses:
            request_status, mid = csms.expect("GetCertificateStatus", timeout=30)
            data = request_status["ocspRequestData"]
            key = json.dumps({k: data[k] for k in hashes_expected[0]}, sort_keys=True)
            tc.assert_(key in responses)
            tc.assert_eq(profiles.OCSP_URL, data["responderURL"])
            csms.respond(mid, {"status": "Accepted", "ocspResult": responses.pop(key)})
        return expected_entry

    pending = False
    try:
        tc.assert_eq("Accepted", csms.call("TriggerMessage", {"requestedMessage": "SignV2G20Certificate"})["status"])
        pending = True
        request, mid = csms.expect("SignCertificate", timeout=60)
        csms.respond(mid, {"status": "Accepted"})
        first = issue(request, now - timedelta(seconds=5), now + timedelta(days=7, minutes=5))
        start = time.monotonic()
        first_entry = install(request, first)
        pending = False
        renewal, mid = csms.expect("SignCertificate", timeout=30)
        pending = True
        elapsed = time.monotonic() - start
        tc.assert_(renewal["requestId"] != request["requestId"])
        tc.assert_eq(certificate_hash_data(root, root), renewal["hashRootCertificate"])
        csms.respond(mid, {"status": "Accepted"})
        replacement = issue(renewal, datetime.now(timezone.utc) - timedelta(seconds=1), now + timedelta(days=45))
        replaced_entry = install(renewal, replacement)
        pending = False
        tc.assert_(first_entry["certificateHashData"] != replaced_entry["certificateHashData"])
        path, certs = env.chain("sub-ca-renewal")
        time.sleep(2)
        # Root-only EV trust validates the served intermediate path.
        env.positive("renewed two-Sub-CA SECC chain completes ISO-20 authorization", path, certs,
                     expected_server=replacement)
        tc.assert_eq(connection_count, csms.connection_count)
        tc.assert_eq(security_count, len(csms.security_events))
        env.record("seven-day automatic renewal with exact Sub-CA inventory", {
            "seconds_after_install": round(elapsed, 3), "initial": first_entry, "replacement": replaced_entry})
    finally:
        if pending:
            tc.assert_eq("Accepted", csms.call("TriggerMessage", {"requestedMessage": "SignV2G20Certificate"})["status"])
            _, mid = csms.expect("SignCertificate", timeout=60)
            csms.respond(mid, {"status": "Rejected"})


def test_environment_transition_and_reboot(tc: TestContext):
    """HUB20-21-007: ISO15118Ctrlr.PrivateEnvironmentEnabled changes reset TLS policy."""
    tc.set_test_timeout(360)
    env = environment
    saved = env.variable("PrivateEnvironmentEnabled")
    before = sorted(env.inventory(), key=env.identity)
    path, certs = env.chain("environment-transition")
    try:
        for initial, changed in [("false", "true"), ("true", "false")]:
            env.variable("PrivateEnvironmentEnabled", initial)
            with env.connect(path) as tls:
                vehicle.sap_iso20(tls)
                # Public mode emits M07; private mode may do so in PnC builds.
                try:
                    request, mid = env.csms.expect("GetCertificateChainStatus", timeout=2)
                except TimeoutError:
                    pass
                else:
                    env.csms.respond(mid, vehicle.chain_status_response(request, ["Good"] * 3))
                session = vehicle.session_setup(tls)
                vehicle.authorization_setup(tls, session)
                tc.assert_eq("OK", vehicle.final_authorization(tls, session)["ResponseCode"])
                # An unchanged value must not tear down the connection.
                env.variable("PrivateEnvironmentEnabled", initial)
                tc.assert_eq("OK", vehicle.final_authorization(tls, session)["ResponseCode"])
                env.variable("PrivateEnvironmentEnabled", changed)
                tc.assert_eq(changed, env.variable("PrivateEnvironmentEnabled"))
                tc.assert_(vehicle.tls_closed(tls))
                env.record("live environment transition closes TLS", {"from": initial, "to": changed})
            time.sleep(2)

        # A new leaf forces a fresh public M07 request after the transition.
        path, certs = env.chain("environment-public-recovery")
        env.positive("public-mode recovery enforces fresh M07", path, certs)
        env.variable("PrivateEnvironmentEnabled", "true")
        connection_count = env.csms.connection_count
        tc.reboot()
        env.csms.wait_for_connection(after=connection_count, timeout=60)
        tc.assert_eq("true", env.variable("PrivateEnvironmentEnabled"))
        tc.assert_eq(before, sorted(env.inventory(), key=env.identity))
        common.enable_debug_mode(env.host)
        tc.wait_for(lambda: tc.assert_(common.sdp_request(env.iface, expected_from=env.target_ll) is not None), timeout=30)
        # OCSP is memory-only. Private non-PnC operation must recover using
        # the persisted private PKI without inheriting a previous TLS context.
        with env.connect(path) as tls:
            vehicle.sap_iso20(tls)
            session = vehicle.session_setup(tls)
            vehicle.authorization_setup(tls, session)
            tc.assert_eq("OK", vehicle.final_authorization(tls, session)["ResponseCode"])
        env.record("private environment and certificate inventory survive reboot")
    finally:
        env.variable("PrivateEnvironmentEnabled", saved)
        tc.assert_eq(saved, env.variable("PrivateEnvironmentEnabled"))
        tc.assert_eq(before, sorted(env.inventory(), key=env.identity))


def test_private_source_waiver_preserves_role_validation(tc: TestContext):
    """HUB20-21-007 / V2G20-2443: a source waiver never waives vehicle role."""
    tc.set_test_timeout(120)
    env = environment
    saved = env.variable("PrivateEnvironmentEnabled")
    try:
        env.variable("PrivateEnvironmentEnabled", "true")
        path, _ = env.chain("private-no-sources", leaf_opts={"source": None},
                            sub1_opts={"source": None}, sub2_opts={"source": None})
        with env.connect(path) as tls:
            vehicle.sap_iso20(tls)
            session = vehicle.session_setup(tls)
            vehicle.authorization_setup(tls, session)
            tc.assert_eq("OK", vehicle.final_authorization(tls, session)["ResponseCode"])
        time.sleep(2)
        path, _ = env.chain("private-contract-no-sources", leaf_role="MSP",
                            leaf_opts={"source": None}, sub1_opts={"source": None},
                            sub2_opts={"source": None})
        env.negative("private waiver retains role rejection", path, "CERTIFICATE_UNKNOWN")
    finally:
        env.variable("PrivateEnvironmentEnabled", saved)


def generate_tests():
    cases = [
        ("contract_leaf", {"leaf_role": "MSP"}, "CERTIFICATE_UNKNOWN"),
        ("provisioning_leaf", {"leaf_role": "OEM"}, "CERTIFICATE_UNKNOWN"),
        ("contract_intermediate", {"sub1_role": "MSP"}, "CERTIFICATE_UNKNOWN"),
        ("missing_leaf_source", {"leaf_opts": {"source": None}}, "CERTIFICATE_UNKNOWN"),
        ("missing_intermediate_source", {"sub2_opts": {"source": None}}, "CERTIFICATE_UNKNOWN"),
        ("missing_all_sources", {"leaf_opts": {"source": None}, "sub1_opts": {"source": None},
                                 "sub2_opts": {"source": None}}, "CERTIFICATE_UNKNOWN"),
        ("crl_only_source", {"leaf_opts": {"source": "crl"}}, "CERTIFICATE_UNKNOWN"),
        ("expired_contract_leaf", {"leaf_role": "MSP", "leaf_opts": {"expired": True}}, "CERTIFICATE_EXPIRED"),
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
