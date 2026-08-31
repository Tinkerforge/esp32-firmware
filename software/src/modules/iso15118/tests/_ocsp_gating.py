#!/usr/bin/env python3
"""HUB20-532-002 gating and OCSP staple plumbing

Provisions the certificate store step by step and checks the TLS server behavior after every step:
  1. store live but empty, no TLS handshake possible
  2. -2 and -20 chains provisioned, OCSP unknown: TLS 1.2 works and is
     verified against the dev V2G root, TLS 1.3 refused
  3. PrivateEnviromentEnabled does not waive OCSP because this station
     supports PnC; TLS 1.3 remains unavailable while OCSP is unknown
  4. a Good OCSP response for the directly root-signed -20 leaf,
     delivered via GetCertificateStatus, enables TLS 1.3 in both private
     and public mode
  5. openssl s_client -status sees the response stapled to the leaf
     CertificateEntry in the TLS 1.3 handshake (needs firmware built
     against libs with the stapling patch)

Invoked by certificates.py through the firmware test runner.
"""

import argparse
import base64
import ssl
import subprocess
import sys
import tempfile
import shutil
import time
import urllib.request
from pathlib import Path

import _common as common
from _common import CSMSSim as Csms, EVTestClient, ISO2, managed_socket

SCRIPT_DIR = Path(__file__).parent.resolve()
CERTS = SCRIPT_DIR / ".." / "tools" / "certs" / "output"
OCSP_URL = "http://ocsp.test.example/"


def run(cmd, **kwargs):
    return subprocess.run(cmd, check=True, capture_output=True, text=True, **kwargs)


def sign_csr(workdir, csr_pem, iso20, with_aia):
    """Signs a device CSR and returns the transmitted chain PEM."""
    pki = CERTS / ("iso20" if iso20 else "iso2")
    issuer = "v2gRootCACert.pem" if iso20 else "cpoSubCA2Cert.pem"
    issuer_key = "v2gRootCA.key" if iso20 else "cpoSubCA2.key"
    csr = workdir / "device.csr"
    csr.write_text(csr_pem)
    ext = workdir / "ext.cnf"
    ext.write_text("[ext]\n"
                   "basicConstraints = critical,CA:false\n"
                   "keyUsage = critical,digitalSignature,keyAgreement\n"
                   "subjectKeyIdentifier = hash\n"
                   + (f"authorityInfoAccess = OCSP;URI:{OCSP_URL}\n" if with_aia else ""))
    leaf = workdir / "leaf.pem"
    run(["openssl", "x509", "-req", "-in", str(csr),
         "-CA", str(pki / "certs" / issuer),
         "-CAkey", str(pki / "private_keys" / issuer_key), "-passin", "pass:12345",
         "-CAcreateserial", "-days", "60", "-sha512" if iso20 else "-sha256",
         "-extfile", str(ext), "-extensions", "ext", "-out", str(leaf)])
    chain = leaf.read_text()
    if not iso20:
        chain += (pki / "certs" / "cpoSubCA2Cert.pem").read_text()
        chain += (pki / "certs" / "cpoSubCA1Cert.pem").read_text()
    return chain


def ocsp_response_b64(workdir, leaf_pem_path, next_update_minutes=None):
    """Good OCSP response for the directly root-signed ISO 15118-20 leaf."""
    pki = CERTS / "iso20"
    serial = run(["openssl", "x509", "-in", str(leaf_pem_path), "-noout", "-serial"]).stdout.strip().split("=")[1]
    enddate = run(["openssl", "x509", "-in", str(leaf_pem_path), "-noout", "-enddate"]).stdout.strip().split("=", 1)[1]
    stamp = run(["date", "-d", enddate, "-u", "+%y%m%d%H%M%SZ"]).stdout.strip()
    index = workdir / "index.txt"
    index.write_text(f"V\t{stamp}\t\t{serial}\tunknown\t/CN=x\n")
    resp = workdir / "resp.der"
    validity = ["-ndays", "7"] if next_update_minutes is None else ["-nmin", str(next_update_minutes)]
    run(["openssl", "ocsp", "-index", str(index),
          "-CA", str(pki / "certs" / "v2gRootCACert.pem"),
         "-rsigner", str(pki / "certs" / "v2gRootCACert.pem"),
         "-rkey", str(pki / "private_keys" / "v2gRootCA.key"), "-passin", "pass:12345",
          "-issuer", str(pki / "certs" / "v2gRootCACert.pem"),
          "-cert", str(leaf_pem_path),
          "-reqout", str(workdir / "req.der"), "-respout", str(resp)] + validity)
    return base64.b64encode(resp.read_bytes()).decode(), resp.read_bytes()


def provision_chain(csms, workdir, iso20, with_aia):
    kind = "V2G20Certificate" if iso20 else "V2GCertificate"
    assert csms.call("TriggerMessage", {"requestedMessage": f"Sign{kind}"})["status"] == "Accepted"
    sign_req, msg_id = csms.expect("SignCertificate", timeout=60)
    csms.respond(msg_id, {"status": "Accepted"})
    assert sign_req["certificateType"] == kind
    chain = sign_csr(workdir, sign_req["csr"], iso20, with_aia)
    # secp521r1 chain validation takes over 10 s on the device.
    res = csms.call("CertificateSigned", {
        "certificateChain": chain,
        "certificateType": kind,
        "requestId": sign_req["requestId"],
    }, timeout=90)
    assert res["status"] == "Accepted", res
    return chain


def try_tls(charger, iface, tls13, mutual=False):
    """Returns the negotiated version string or None when refused."""
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    ctx.check_hostname = False
    if tls13:
        ctx.minimum_version = ssl.TLSVersion.TLSv1_3
        ctx.load_verify_locations(cafile=str(CERTS / "iso20" / "certs" / "v2gRootCACert.pem"))
        if mutual:
            ctx.load_cert_chain(certfile=str(CERTS / "iso20" / "certs" / "oemCertChain.pem"),
                                keyfile=str(CERTS / "iso20" / "private_keys" / "oemLeaf.key"),
                                password="12345")
    else:
        ctx.maximum_version = ssl.TLSVersion.TLSv1_2
        ctx.set_ciphers("ECDHE-ECDSA-AES128-SHA256")
        ctx.load_verify_locations(cafile=str(CERTS / "iso2" / "certs" / "v2gRootCACert.pem"))
    ctx.verify_mode = ssl.CERT_REQUIRED
    try:
        raw = common.connect_secc(charger, iface)
    except Exception:
        return None
    try:
        raw.settimeout(30)
        tls = ctx.wrap_socket(raw)
        ver = tls.version()
        tls.close()
        time.sleep(1)
        return ver
    except (ssl.SSLError, OSError):
        raw.close()
        time.sleep(1)
        return None


def trace_log(charger):
    with urllib.request.urlopen(f"http://{charger}/trace_log", timeout=20) as f:
        return f.read().decode(errors="replace")


def s_client_status(charger, iface):
    """TLS 1.3 handshake with status_request, returns the s_client output."""
    res = common.sdp_request(iface)
    if res is None:
        raise SystemExit("SDP request timed out, is ISO debug mode enabled?")
    return subprocess.run(
        ["openssl", "s_client", "-connect", f'[{res["secc_ll"]}%{iface}]:{res["port"]}',
         "-tls1_3", "-status",
         "-CAfile", str(CERTS / "iso20" / "certs" / "v2gRootCACert.pem"),
         "-cert", str(CERTS / "iso20" / "certs" / "oemLeafCert.pem"),
         "-cert_chain", str(CERTS / "iso20" / "certs" / "oemCertChain.pem"),
         "-key", str(CERTS / "iso20" / "private_keys" / "oemLeaf.key"), "-pass", "pass:12345"],
        input="Q", capture_output=True, text=True, timeout=60).stdout


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--charger", required=True)
    p.add_argument("--iface")
    p.add_argument("--port", type=int, default=9500)
    args = p.parse_args()

    common.require_iso_tls_config(args.charger)
    iface = args.iface or common.route_interface(args.charger)
    local_ip = common.local_ip_towards(args.charger)
    pnc_supported = "iso15118_pnc" in common.api_get(args.charger, "info/features")
    workdir = Path(tempfile.mkdtemp(prefix="ocsp_gating_"))

    failures = 0
    csms_tls = common.LocalCSMSTls(args.charger, local_ip)

    def check(name, ok, detail=""):
        nonlocal failures
        print(f'{"ok  " if ok else "FAIL"} {name}{": " + str(detail) if detail else ""}')
        failures += 0 if ok else 1

    saved_config = common.api_get(args.charger, "ocpp/config")
    csms = Csms(args.port, interactive=("SignCertificate", "GetCertificateStatus"),
                certfile=str(csms_tls.certfile), keyfile=str(csms_tls.keyfile))
    try:
        common.api_put(args.charger, "ocpp/reset", None)
        test_config = dict(saved_config)
        test_config.update({"enable": True, "protocol": 1, "url": f"wss://{local_ip}:{args.port}",
                            "enable_auth": True, "pass": "iso15118-test-password",
                            "cert_id": csms_tls.cert_id})
        common.api_put(args.charger, "ocpp/config_update", test_config)
        if not csms.connected.wait(timeout=60):
            raise SystemExit("charger did not connect to the embedded CSMS")
        time.sleep(2)

        defaults = csms.call("GetVariables", {"getVariableData": [
            {"component": {"name": "ISO15118Ctrlr"}, "variable": {"name": "ISO15118EvseId"}},
            {"component": {"name": "ISO15118Ctrlr"}, "variable": {"name": "EnforceTlsEnabled"}},
        ]})["getVariableResult"]
        check("ISO15118EvseId default after reset", defaults[0].get("attributeValue") == "ZZ00000",
              defaults[0].get("attributeValue"))
        check("EnforceTlsEnabled default after reset", defaults[1].get("attributeValue") == "false",
              defaults[1].get("attributeValue"))

        common.enable_debug_mode(args.charger)
        time.sleep(2)

        check("store live and empty, no TLS 1.2 handshake",
              try_tls(args.charger, iface, tls13=False) is None)

        for kind, pem in (("V2GRootCertificate", "iso2"), ("V2GRootCertificate", "iso20"),
                          ("OEMRootCertificate", "iso20")):
            name = "v2gRootCACert.pem" if kind.startswith("V2G") else "oemRootCACert.pem"
            res = csms.call("InstallCertificate", {
                "certificateType": kind,
                "certificate": (CERTS / pem / "certs" / name).read_text()})
            assert res["status"] == "Accepted", (kind, pem, res)

        provision_chain(csms, workdir, iso20=False, with_aia=False)

        expected_evseid = "DE*TNK*E123456"
        set_result = csms.call("SetVariables", {"setVariableData": [{
            "component": {"name": "ISO15118Ctrlr"},
            "variable": {"name": "ISO15118EvseId"},
            "attributeValue": expected_evseid,
        }]})["setVariableResult"][0]
        check("ISO15118EvseId accepted", set_result["attributeStatus"] == "Accepted", set_result)
        common.disable_debug_mode(args.charger)
        time.sleep(1)
        common.enable_debug_mode(args.charger)
        time.sleep(2)

        from iso15118.shared.messages.enums import Namespace
        client = EVTestClient(args.charger, iface)
        with managed_socket(client.connect_tls(client.tls12_context())) as tls:
            sap_res = client.sap(tls, [ISO2])
            assert sap_res["ResponseCode"] == "OK_SuccessfulNegotiation", sap_res
            response = client.exchange(tls, {
                "V2G_Message": {
                    "Header": {"SessionID": "0000000000000000"},
                    "Body": {"SessionSetupReq": {"EVCCID": "020000000001"}},
                },
            }, Namespace.ISO_V2_MSG_DEF)
        actual_evseid = response["V2G_Message"]["Body"]["SessionSetupRes"]["EVSEID"]
        check("SessionSetupRes carries ISO15118EvseId", actual_evseid == expected_evseid, actual_evseid)
        time.sleep(1)

        check("-2 chain live, TLS 1.2 works",
              try_tls(args.charger, iface, tls13=False) == "TLSv1.2")

        chain20 = provision_chain(csms, workdir, iso20=True, with_aia=True)
        (workdir / "leaf20.pem").write_text(chain20.split("-----END CERTIFICATE-----")[0] + "-----END CERTIFICATE-----\n")
        time.sleep(3)
        check("OCSP unknown, TLS 1.3 refused [HUB20-532-002]",
              try_tls(args.charger, iface, tls13=True, mutual=True) is None)

        assert csms.call("SetVariables", {"setVariableData": [{
            "component": {"name": "ISO15118Ctrlr"}, "variable": {"name": "PrivateEnviromentEnabled"},
            "attributeValue": "true"}]})["setVariableResult"][0]["attributeStatus"] == "Accepted"
        time.sleep(3)
        private_version = try_tls(args.charger, iface, tls13=True, mutual=True)
        if pnc_supported:
            check("private PnC environment still requires SECC OCSP [3.2.9/HUB20-532-002]",
                  private_version is None, private_version)
        else:
            check("private non-PnC environment may waive SECC OCSP [3.2.9]",
                  private_version == "TLSv1.3", private_version)

        # The -20 leaf carries an AIA URL, the device asks for its status.
        status_req, msg_id = csms.expect("GetCertificateStatus", timeout=120)
        check("GetCertificateStatus for the -20 leaf",
              status_req["ocspRequestData"]["responderURL"] == OCSP_URL)
        b64, _ = ocsp_response_b64(workdir, workdir / "leaf20.pem", next_update_minutes=1)
        csms.respond(msg_id, {"status": "Accepted", "ocspResult": b64})
        time.sleep(5)

        check("OCSP good, TLS 1.3 works in a private PnC environment",
              try_tls(args.charger, iface, tls13=True, mutual=True) == "TLSv1.3")
        serial = run(["openssl", "x509", "-in", str(workdir / "leaf20.pem"),
                      "-noout", "-serial"]).stdout.strip().split("=")[1]
        time.sleep(1)
        out = s_client_status(args.charger, iface)
        check("stapled OCSP response on the wire",
              "OCSP Response Status: successful" in out and "Cert Status: good" in out
              and serial in out,
              "" if "OCSP Response Status" in out else "no OCSP response in the handshake")

        assert csms.call("SetVariables", {"setVariableData": [{
            "component": {"name": "ISO15118Ctrlr"}, "variable": {"name": "PrivateEnviromentEnabled"},
            "attributeValue": "false"}]})["setVariableResult"][0]["attributeStatus"] == "Accepted"
        time.sleep(3)
        check("OCSP-good chain remains available after returning to public mode",
              try_tls(args.charger, iface, tls13=True, mutual=True) == "TLSv1.3")

        # Advance the station clock through nextUpdate. The stale Good
        # result and staple must be dropped while replacement is pending.
        csms.current_time_offset_s = 120
        assert csms.call("TriggerMessage", {"requestedMessage": "Heartbeat"})["status"] == "Accepted"
        time.sleep(5)
        expired_log = trace_log(args.charger)
        check("SECC OCSP cache expires at nextUpdate [HUB20-431-001/V2G20-1021]",
              "OCSP cache expired for chain certificate" in expired_log)
        check("expired SECC OCSP forces TLS 1.3 fallback while refresh is pending [HUB20-532-002]",
              try_tls(args.charger, iface, tls13=True, mutual=True) is None)
        check("TLS 1.2 remains available after SECC OCSP expiry",
              try_tls(args.charger, iface, tls13=False) == "TLSv1.2")

        refresh_req, refresh_msg = csms.expect("GetCertificateStatus", timeout=30)
        check("SECC OCSP expiry triggers an immediate refresh",
              refresh_req["ocspRequestData"]["responderURL"] == OCSP_URL)
        fresh_b64, _ = ocsp_response_b64(workdir, workdir / "leaf20.pem")
        csms.respond(refresh_msg, {"status": "Accepted", "ocspResult": fresh_b64})
        time.sleep(5)
        check("fresh SECC OCSP restores TLS 1.3 without reboot",
              try_tls(args.charger, iface, tls13=True, mutual=True) == "TLSv1.3")

        csms.current_time_offset_s = 0
        assert csms.call("TriggerMessage", {"requestedMessage": "Heartbeat"})["status"] == "Accepted"
        time.sleep(2)
    finally:
        try:
            csms.current_time_offset_s = 0
            if csms.connected.is_set():
                csms.call("TriggerMessage", {"requestedMessage": "Heartbeat"})
            disabled = dict(saved_config)
            disabled["enable"] = False
            common.api_put(args.charger, "ocpp/config_update", disabled)
            time.sleep(2)
            common.api_put(args.charger, "ocpp/reset", None)
            common.api_put(args.charger, "ocpp/config_update", saved_config)
        except Exception as e:
            print(f"cleanup failed, restore the ocpp config manually: {e}")
            failures += 1
        csms.stop()
        csms_tls.close()
        shutil.rmtree(workdir, ignore_errors=True)

    print("PASS" if failures == 0 else f"FAIL ({failures} failures)")
    sys.exit(0 if failures == 0 else 1)


if __name__ == "__main__":
    main()
