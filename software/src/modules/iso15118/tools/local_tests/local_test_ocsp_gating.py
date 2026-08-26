#!/usr/bin/env python3
"""HUB20-532-002 gating and OCSP staple plumbing on a real charger.

Provisions the certificate store step by step and checks the TLS server behavior after every step:
  1. store live but empty, no TLS handshake possible
  2. -2 and -20 chains provisioned, OCSP unknown: TLS 1.2 works and is
     verified against the dev V2G root, TLS 1.3 refused
  3. PrivateEnviromentEnabled true waives the OCSP obligation, TLS 1.3
     works with mutual auth (OEM client chain), false refuses again
  4. a Good OCSP response for the -20 leaf (signed by the dev CPO sub
     CA 2, delivered via GetCertificateStatus) enables TLS 1.3 and the
     staple DER shows up in the trace log

Run this in evsim venv:
../../../../../.venv-evsim/bin/python local_test_ocsp_gating.py --charger <ip>
"""

import argparse
import base64
import ssl
import subprocess
import sys
import tempfile
import time
import urllib.request
from pathlib import Path

import common
from local_test_ocpp_ctrlr import Csms

SCRIPT_DIR = Path(__file__).parent.resolve()
CERTS = SCRIPT_DIR / ".." / "certs" / "output"
OCSP_URL = "http://ocsp.test.example/"


def run(cmd, **kwargs):
    return subprocess.run(cmd, check=True, capture_output=True, text=True, **kwargs)


def sign_csr(workdir, csr_pem, iso20, with_aia):
    """Signs a device CSR with the dev CPO sub CA 2, returns the chain PEM."""
    pki = CERTS / ("iso20" if iso20 else "iso2")
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
         "-CA", str(pki / "certs" / "cpoSubCA2Cert.pem"),
         "-CAkey", str(pki / "private_keys" / "cpoSubCA2.key"), "-passin", "pass:12345",
         "-CAcreateserial", "-days", "60", "-sha512" if iso20 else "-sha256",
         "-extfile", str(ext), "-extensions", "ext", "-out", str(leaf)])
    chain = leaf.read_text()
    chain += (pki / "certs" / "cpoSubCA2Cert.pem").read_text()
    chain += (pki / "certs" / "cpoSubCA1Cert.pem").read_text()
    return chain


def ocsp_response_b64(workdir, leaf_pem_path):
    """Good OCSP response for the leaf, signed by the dev CPO sub CA 2 (iso20)."""
    pki = CERTS / "iso20"
    serial = run(["openssl", "x509", "-in", str(leaf_pem_path), "-noout", "-serial"]).stdout.strip().split("=")[1]
    enddate = run(["openssl", "x509", "-in", str(leaf_pem_path), "-noout", "-enddate"]).stdout.strip().split("=", 1)[1]
    stamp = run(["date", "-d", enddate, "-u", "+%y%m%d%H%M%SZ"]).stdout.strip()
    index = workdir / "index.txt"
    index.write_text(f"V\t{stamp}\t\t{serial}\tunknown\t/CN=x\n")
    resp = workdir / "resp.der"
    run(["openssl", "ocsp", "-index", str(index),
         "-CA", str(pki / "certs" / "cpoSubCA2Cert.pem"),
         "-rsigner", str(pki / "certs" / "cpoSubCA2Cert.pem"),
         "-rkey", str(pki / "private_keys" / "cpoSubCA2.key"), "-passin", "pass:12345",
         "-issuer", str(pki / "certs" / "cpoSubCA2Cert.pem"),
         "-cert", str(leaf_pem_path),
         "-reqout", str(workdir / "req.der"), "-respout", str(resp), "-ndays", "7"])
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


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--charger", required=True)
    p.add_argument("--iface")
    p.add_argument("--port", type=int, default=9500)
    args = p.parse_args()

    common.require_iso_tls_config(args.charger)
    iface = args.iface or common.default_iface(args.charger)
    local_ip = common.local_ip_towards(args.charger)
    workdir = Path(tempfile.mkdtemp(prefix="ocsp_gating_"))

    failures = 0

    def check(name, ok, detail=""):
        nonlocal failures
        print(f'{"ok  " if ok else "FAIL"} {name}{": " + str(detail) if detail else ""}')
        failures += 0 if ok else 1

    saved_config = common.api_get(args.charger, "ocpp/config")
    csms = Csms(args.port, interactive=("SignCertificate", "GetCertificateStatus"))
    try:
        common.api_put(args.charger, "ocpp/reset", None)
        test_config = dict(saved_config)
        test_config.update({"enable": True, "protocol": 1, "url": f"ws://{local_ip}:{args.port}",
                            "enable_auth": False, "pass": ""})
        common.api_put(args.charger, "ocpp/config_update", test_config)
        if not csms.connected.wait(timeout=60):
            raise SystemExit("charger did not connect to the embedded CSMS")
        time.sleep(2)

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
        chain20 = provision_chain(csms, workdir, iso20=True, with_aia=True)
        (workdir / "leaf20.pem").write_text(chain20.split("-----END CERTIFICATE-----")[0] + "-----END CERTIFICATE-----\n")
        time.sleep(3)

        check("-2 chain live, TLS 1.2 works",
              try_tls(args.charger, iface, tls13=False) == "TLSv1.2")
        check("OCSP unknown, TLS 1.3 refused [HUB20-532-002]",
              try_tls(args.charger, iface, tls13=True, mutual=True) is None)

        assert csms.call("SetVariables", {"setVariableData": [{
            "component": {"name": "ISO15118Ctrlr"}, "variable": {"name": "PrivateEnviromentEnabled"},
            "attributeValue": "true"}]})["setVariableResult"][0]["attributeStatus"] == "Accepted"
        time.sleep(3)
        check("private environment waives OCSP, TLS 1.3 works",
              try_tls(args.charger, iface, tls13=True, mutual=True) == "TLSv1.3")

        assert csms.call("SetVariables", {"setVariableData": [{
            "component": {"name": "ISO15118Ctrlr"}, "variable": {"name": "PrivateEnviromentEnabled"},
            "attributeValue": "false"}]})["setVariableResult"][0]["attributeStatus"] == "Accepted"
        time.sleep(3)
        check("back to public environment, TLS 1.3 refused again",
              try_tls(args.charger, iface, tls13=True, mutual=True) is None)

        # The -20 leaf carries an AIA URL, the device asks for its status.
        status_req, msg_id = csms.expect("GetCertificateStatus", timeout=120)
        check("GetCertificateStatus for the -20 leaf",
              status_req["ocspRequestData"]["responderURL"] == OCSP_URL)
        b64, der = ocsp_response_b64(workdir, workdir / "leaf20.pem")
        csms.respond(msg_id, {"status": "Accepted", "ocspResult": b64})
        time.sleep(5)

        check("OCSP good, TLS 1.3 works",
              try_tls(args.charger, iface, tls13=True, mutual=True) == "TLSv1.3")
        log = trace_log(args.charger)
        check("staple DER loaded into the TLS server",
              f"OCSP staple for -20 chain certificate 0: {len(der)} bytes" in log)
    finally:
        try:
            disabled = dict(saved_config)
            disabled["enable"] = False
            common.api_put(args.charger, "ocpp/config_update", disabled)
            time.sleep(2)
            common.api_put(args.charger, "ocpp/reset", None)
            common.api_put(args.charger, "ocpp/config_update", saved_config)
        except Exception as e:
            print(f"cleanup failed, restore the ocpp config manually: {e}")
        csms.stop()

    print("PASS" if failures == 0 else f"FAIL ({failures} failures)")
    sys.exit(0 if failures == 0 else 1)


if __name__ == "__main__":
    main()
