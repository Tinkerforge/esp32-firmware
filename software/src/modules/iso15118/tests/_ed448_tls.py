#!/usr/bin/env python3
"""Verify V2G20-2379/3376 root selection, V2G20-2399 fallback and TLS suites."""

import argparse
import base64
import shutil
import subprocess
import sys
import tempfile
import time
import urllib.request
from pathlib import Path

import _common as common
from _common import CSMSSim as Csms

SCRIPT_DIR = Path(__file__).parent.resolve()
CERTS = SCRIPT_DIR / ".." / "tools" / "certs" / "output"
ED448_OCSP_URL = "http://ocsp.ed448.test.example/"
ECDSA_OCSP_URL = "http://ocsp.p521.test.example/"


def run(command, **kwargs):
    return subprocess.run(command, check=True, capture_output=True, text=True, **kwargs)


def expect_sign_request(csms, certificate_type, timeout=90):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        request, message_id = csms.expect(
            "SignCertificate", timeout=deadline - time.monotonic())
        csms.respond(message_id, {"status": "Accepted"})
        if request.get("certificateType") == certificate_type:
            return request
    raise TimeoutError(f"No SignCertificate request for {certificate_type}")


def make_ed448_pki(workdir):
    v2g_key = workdir / "v2g-root.key"
    v2g_root = workdir / "v2g-root.pem"
    oem_key = workdir / "oem-root.key"
    oem_root = workdir / "oem-root.pem"
    client_key = workdir / "client.key"
    client_csr = workdir / "client.csr"
    client_cert = workdir / "client.pem"
    client_ext = workdir / "client-ext.cnf"
    equivalent_authority_key = workdir / "equivalent-authority.key"
    equivalent_authority = workdir / "equivalent-authority.pem"

    for key, cert, subject in (
        (v2g_key, v2g_root, "/C=DE/O=Ed448 Test/CN=Ed448 V2G Root"),
        (oem_key, oem_root, "/C=DE/O=Ed448 Test/CN=Ed448 OEM Root"),
    ):
        run(["openssl", "genpkey", "-algorithm", "ED448", "-out", str(key)])
        run([
            "openssl", "req", "-new", "-x509", "-key", str(key),
            "-out", str(cert), "-days", "30", "-subj", subject,
            "-addext", "basicConstraints=critical,CA:TRUE",
            "-addext", "keyUsage=critical,keyCertSign,cRLSign",
        ])

    run(["openssl", "genpkey", "-algorithm", "ED448", "-out", str(client_key)])
    run([
        "openssl", "req", "-new", "-key", str(client_key),
        "-out", str(client_csr), "-subj", "/C=DE/O=Ed448 Test/CN=Ed448 EVCC",
    ])
    client_ext.write_text(
        "[ext]\n"
        "basicConstraints=critical,CA:false\n"
        "keyUsage=critical,digitalSignature\n"
        "subjectKeyIdentifier=hash\n"
        "authorityKeyIdentifier=keyid,issuer\n"
    )
    run([
        "openssl", "x509", "-req", "-in", str(client_csr),
        "-CA", str(oem_root), "-CAkey", str(oem_key), "-CAcreateserial",
        "-out", str(client_cert), "-days", "30",
        "-extfile", str(client_ext), "-extensions", "ext",
    ])
    run(["openssl", "genpkey", "-algorithm", "ED448", "-out", str(equivalent_authority_key)])
    run([
        "openssl", "req", "-new", "-x509", "-key", str(equivalent_authority_key),
        "-out", str(equivalent_authority), "-days", "30",
        "-subj", "/C=DE/O=ed448 test/CN=ed448 v2g root",
        "-addext", "basicConstraints=critical,CA:TRUE",
        "-addext", "keyUsage=critical,keyCertSign,cRLSign",
    ])
    return (v2g_key, v2g_root, oem_root, client_key, client_cert,
            equivalent_authority)


def sign_csr(workdir, csr_pem, root_key, root_cert, ocsp_url, tag,
             digest=None, root_password=None):
    csr = workdir / f"secc-{tag}.csr"
    leaf = workdir / f"secc-{tag}.pem"
    ext = workdir / f"secc-{tag}-ext.cnf"
    csr.write_text(csr_pem)
    ext.write_text(
        "[ext]\n"
        "basicConstraints=critical,CA:false\n"
        "keyUsage=critical,digitalSignature\n"
        "subjectKeyIdentifier=hash\n"
        "authorityKeyIdentifier=keyid,issuer\n"
        f"authorityInfoAccess=OCSP;URI:{ocsp_url}\n"
    )
    command = [
        "openssl", "x509", "-req", "-in", str(csr),
        "-CA", str(root_cert), "-CAkey", str(root_key), "-CAcreateserial",
        "-out", str(leaf), "-days", "30",
        "-extfile", str(ext), "-extensions", "ext",
    ]
    if digest is not None:
        command.append(digest)
    if root_password is not None:
        command += ["-passin", f"pass:{root_password}"]
    run(command)
    return leaf


def make_ocsp_response(workdir, leaf, root_key, root_cert, tag,
                       root_password=None):
    serial = run([
        "openssl", "x509", "-in", str(leaf), "-noout", "-serial",
    ]).stdout.strip().split("=", 1)[1]
    enddate = run([
        "openssl", "x509", "-in", str(leaf), "-noout", "-enddate",
    ]).stdout.strip().split("=", 1)[1]
    stamp = run(["date", "-d", enddate, "-u", "+%y%m%d%H%M%SZ"]).stdout.strip()
    index = workdir / f"index-{tag}.txt"
    response = workdir / f"ocsp-{tag}.der"
    index.write_text(f"V\t{stamp}\t\t{serial}\tunknown\t/CN=Ed448 SECC\n")
    command = [
        "openssl", "ocsp", "-index", str(index), "-CA", str(root_cert),
        "-rsigner", str(root_cert), "-rkey", str(root_key),
        "-issuer", str(root_cert), "-cert", str(leaf),
        "-reqout", str(workdir / f"ocsp-request-{tag}.der"),
        "-respout", str(response), "-ndays", "7",
    ]
    if root_password is not None:
        command += ["-passin", f"pass:{root_password}"]
    run(command)
    return base64.b64encode(response.read_bytes()).decode(), response.read_bytes(), serial


def provision_iso2_chain(csms, workdir):
    assert csms.call("TriggerMessage", {"requestedMessage": "SignV2GCertificate"})["status"] == "Accepted"
    request = expect_sign_request(csms, "V2GCertificate", timeout=60)
    csr = workdir / "iso2.csr"
    leaf = workdir / "iso2.pem"
    csr.write_text(request["csr"])
    pki = CERTS / "iso2"
    run([
        "openssl", "x509", "-req", "-in", str(csr),
        "-CA", str(pki / "certs" / "cpoSubCA2Cert.pem"),
        "-CAkey", str(pki / "private_keys" / "cpoSubCA2.key"),
        "-passin", "pass:12345", "-CAcreateserial", "-days", "30", "-sha256",
        "-extfile", str(SCRIPT_DIR / ".." / "tools" / "certs" / "configs" / "seccLeafCert.cnf"),
        "-extensions", "ext",
        "-out", str(leaf),
    ])
    chain = leaf.read_text()
    chain += (pki / "certs" / "cpoSubCA2Cert.pem").read_text()
    chain += (pki / "certs" / "cpoSubCA1Cert.pem").read_text()
    response = csms.call("CertificateSigned", {
        "certificateChain": chain,
        "certificateType": "V2GCertificate",
        "requestId": request["requestId"],
    }, timeout=90)
    assert response["status"] == "Accepted", response


def set_suite(csms, value):
    result = csms.call("SetVariables", {"setVariableData": [{
        "component": {"name": "ISO15118Ctrlr"},
        "variable": {"name": "V2G20SECCLeafCryptoSuite"},
        "attributeValue": value,
    }]})["setVariableResult"][0]
    assert result["attributeStatus"] == "Accepted", result


def restart_debug(charger):
    common.disable_debug_mode(charger)
    time.sleep(1)
    common.enable_debug_mode(charger)
    time.sleep(2)


def provision_iso20_chain(csms, workdir, suite, root_key, root_cert,
                          ocsp_url, tag, digest=None, root_password=None,
                          pad_to=None):
    set_suite(csms, suite)
    assert csms.call("TriggerMessage", {
        "requestedMessage": "SignV2G20Certificate",
    })["status"] == "Accepted"
    sign_request = expect_sign_request(csms, "V2G20Certificate")
    leaf = sign_csr(workdir, sign_request["csr"], root_key, root_cert,
                    ocsp_url, tag, digest, root_password)
    chain = leaf.read_text()
    if pad_to is not None:
        assert len(chain.encode("ascii")) < pad_to
        chain += " " * (pad_to - len(chain.encode("ascii")))
        assert len(chain.encode("ascii")) == pad_to
    response = csms.call("CertificateSigned", {
        "certificateChain": chain,
        "certificateType": "V2G20Certificate",
        "requestId": sign_request["requestId"],
    }, timeout=90)
    assert response["status"] == "Accepted", response

    status_request, status_message_id = csms.expect(
        "GetCertificateStatus", timeout=120)
    assert status_request["ocspRequestData"]["responderURL"] == ocsp_url
    ocsp_b64, ocsp_der, serial = make_ocsp_response(
        workdir, leaf, root_key, root_cert, tag, root_password)
    csms.respond(status_message_id, {
        "status": "Accepted", "ocspResult": ocsp_b64,
    })
    time.sleep(5)
    return leaf, ocsp_der, serial


def tls_probe(charger, iface, sigalgs, v2g_root, client_cert, client_key,
               expected_signature=None, expected_ocsp=None,
               unexpected_ocsp=None, client_chain=None, key_password=None,
               expect_success=True, request_ca=None):
    restart_debug(charger)
    response = common.sdp_request(iface)
    if response is None:
        raise RuntimeError("ISO 15118 SDP request timed out")
    endpoint = f"[{response['secc_ll']}%{iface}]:{response['port']}"
    command = [
        "openssl", "s_client", "-connect", endpoint, "-tls1_3",
        "-groups", "X448", "-sigalgs", sigalgs,
        "-client_sigalgs", sigalgs,
        "-CAfile", str(v2g_root), "-cert", str(client_cert),
        "-key", str(client_key), "-status", "-showcerts", "-verify_return_error",
    ]
    if client_chain is not None:
        command += ["-cert_chain", str(client_chain)]
    if key_password is not None:
        command += ["-pass", f"pass:{key_password}"]
    if request_ca is not None:
        command += ["-requestCAfile", str(request_ca)]
    result = subprocess.run(
        command, input="Q\n", capture_output=True, text=True, timeout=90,
        check=False)
    output = result.stdout + result.stderr
    print(output)
    succeeded = result.returncode == 0 and "TLSv1.3" in output
    if not expect_success:
        assert not succeeded, output
        return
    assert succeeded, output
    assert f"Peer signature type: {expected_signature}" in output, output
    assert "Verify return code: 0 (ok)" in output, output
    assert "OCSP Response Status: successful" in output, output
    assert "Cert Status: good" in output, output
    # V2G20-2379/2399: the selected chain must exclude its anchoring root.
    transmitted_chain = output.split("---\nCertificate chain", 1)[1].split(
        "---\nServer certificate", 1)[0]
    assert transmitted_chain.count("-----BEGIN CERTIFICATE-----") == 1, output
    if expected_ocsp is not None:
        assert ocsp_serial(expected_ocsp).lower() in output.lower(), output
    if unexpected_ocsp is not None:
        assert ocsp_serial(unexpected_ocsp).lower() not in output.lower(), output


def ocsp_serial(der):
    with tempfile.NamedTemporaryFile(suffix=".der") as response:
        response.write(der)
        response.flush()
        text = run([
            "openssl", "ocsp", "-respin", response.name, "-text", "-noverify",
        ]).stdout
    return text.split("Serial Number:", 1)[1].splitlines()[0].strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--charger", required=True)
    parser.add_argument("--iface")
    parser.add_argument("--port", type=int, default=9500)
    args = parser.parse_args()

    common.require_iso_tls_config(args.charger)
    iface = args.iface or common.route_interface(args.charger)
    local_ip = common.local_ip_towards(args.charger)
    workdir = Path(tempfile.mkdtemp(prefix="ed448_tls_"))
    saved_config = common.api_get(args.charger, "ocpp/config")
    csms_tls = common.LocalCSMSTls(args.charger, local_ip)
    csms = Csms(args.port, interactive=("SignCertificate", "GetCertificateStatus"),
                certfile=str(csms_tls.certfile), keyfile=str(csms_tls.keyfile),
                expected_basic_auth=(saved_config["identity"], "iso15118-test-password"))
    try:
        common.api_put(args.charger, "ocpp/reset", None)
        config = dict(saved_config)
        config.update({
            "enable": True,
            "protocol": 1,
            "url": f"wss://{local_ip}:{args.port}",
            "enable_auth": True,
            "pass": "iso15118-test-password",
            "cert_id": csms_tls.cert_id,
        })
        common.api_put(args.charger, "ocpp/config_update", config)
        if not csms.connected.wait(timeout=60):
            raise RuntimeError("charger did not connect to the local CSMS")
        time.sleep(2)

        (v2g_key, v2g_root, oem_root, client_key, client_cert,
         equivalent_authority) = make_ed448_pki(workdir)
        for kind, cert in (
            ("V2GRootCertificate", CERTS / "iso2" / "certs" / "v2gRootCACert.pem"),
            ("V2GRootCertificate", CERTS / "iso20" / "certs" / "v2gRootCACert.pem"),
            ("V2GRootCertificate", v2g_root),
            ("OEMRootCertificate", CERTS / "iso20" / "certs" / "oemRootCACert.pem"),
            ("OEMRootCertificate", oem_root),
        ):
            response = csms.call("InstallCertificate", {
                "certificateType": kind,
                "certificate": cert.read_text(),
            })
            assert response["status"] == "Accepted", (kind, response)

        provision_iso2_chain(csms, workdir)

        p521 = CERTS / "iso20"
        p521_leaf, p521_ocsp, _ = provision_iso20_chain(
            csms, workdir, "ecdsa_secp521r1_sha512",
            p521 / "private_keys" / "v2gRootCA.key",
            p521 / "certs" / "v2gRootCACert.pem", ECDSA_OCSP_URL,
            "p521", "-sha512", "12345", pad_to=8192)
        ed448_leaf, ed448_ocsp, _ = provision_iso20_chain(
            csms, workdir, "ed448", v2g_key, v2g_root,
            ED448_OCSP_URL, "ed448")
        assert "ED448" in run([
            "openssl", "req", "-in", str(workdir / "secc-ed448.csr"),
            "-noout", "-text",
        ]).stdout.upper()

        set_suite(csms, "ecdsa_secp521r1_sha512,ed448")

        tls_probe(args.charger, iface, "ecdsa_secp521r1_sha512",
                  p521 / "certs" / "v2gRootCACert.pem",
                  p521 / "certs" / "oemLeafCert.pem",
                  p521 / "private_keys" / "oemLeaf.key",
                  "ECDSA", p521_ocsp, ed448_ocsp,
                  p521 / "certs" / "oemCertChain.pem", "12345")
        tls_probe(args.charger, iface, "ed448", v2g_root, client_cert,
                  client_key, "Ed448", ed448_ocsp, p521_ocsp,
                  request_ca=v2g_root)
        # V2G20-2379/3376: the indicated Ed448 root overrides the normal
        # P-521 signature preference and retains its V2G20-2388 OCSP staple.
        tls_probe(args.charger, iface,
                  "ecdsa_secp521r1_sha512:ed448", v2g_root, client_cert,
                  client_key, "Ed448", ed448_ocsp, p521_ocsp,
                  request_ca=v2g_root)
        # RFC 5280 7.1 matching includes case and space normalization.
        tls_probe(args.charger, iface,
                  "ecdsa_secp521r1_sha512:ed448", v2g_root, client_cert,
                  client_key, "Ed448", ed448_ocsp, p521_ocsp,
                  request_ca=equivalent_authority)
        # V2G20-2399: an unmatched indication uses another valid chain.
        tls_probe(args.charger, iface,
                  "ecdsa_secp521r1_sha512:ed448",
                  p521 / "certs" / "v2gRootCACert.pem",
                  p521 / "certs" / "oemLeafCert.pem",
                  p521 / "private_keys" / "oemLeaf.key",
                  "ECDSA", p521_ocsp, ed448_ocsp,
                  p521 / "certs" / "oemCertChain.pem", "12345",
                  request_ca=oem_root)
        tls_probe(args.charger, iface,
                  "ecdsa_secp521r1_sha512:ed448",
                  p521 / "certs" / "v2gRootCACert.pem",
                  p521 / "certs" / "oemLeafCert.pem",
                  p521 / "private_keys" / "oemLeaf.key",
                  "ECDSA", p521_ocsp, ed448_ocsp,
                  p521 / "certs" / "oemCertChain.pem", "12345")
        tls_probe(args.charger, iface, "ed448:ecdsa_secp521r1_sha512",
                  p521 / "certs" / "v2gRootCACert.pem",
                  p521 / "certs" / "oemLeafCert.pem",
                  p521 / "private_keys" / "oemLeaf.key",
                  "ECDSA", p521_ocsp, ed448_ocsp,
                  p521 / "certs" / "oemCertChain.pem", "12345")
        trace = urllib.request.urlopen(
            f"http://{args.charger}/trace_log", timeout=20).read().decode(errors="replace")
        assert "ISO20 candidate" in trace and "ecdsa" in trace and "ed448" in trace, trace
        assert "Mutual TLS: EVCC certificate verified successfully" in trace, trace
        assert len(p521_ocsp) > 0 and len(ed448_ocsp) > 0

        set_suite(csms, "ecdsa_secp521r1_sha512")
        time.sleep(2)
        tls_probe(args.charger, iface, "ed448", v2g_root, client_cert,
                  client_key, expect_success=False)

        set_suite(csms, "ed448")
        time.sleep(2)
        tls_probe(args.charger, iface, "ecdsa_secp521r1_sha512",
                  p521 / "certs" / "v2gRootCACert.pem",
                  p521 / "certs" / "oemLeafCert.pem",
                  p521 / "private_keys" / "oemLeaf.key",
                  client_chain=p521 / "certs" / "oemCertChain.pem",
                  key_password="12345", expect_success=False)
        assert "Ed448" in run(["openssl", "x509", "-in", str(ed448_leaf), "-noout", "-text"]).stdout
        assert "id-ecPublicKey" in run(["openssl", "x509", "-in", str(p521_leaf), "-noout", "-text"]).stdout
        print("PASS dual P-521/Ed448 TLS selection, per-chain OCSP, 8 KiB installation and suite disablement")
    finally:
        try:
            disabled = dict(saved_config)
            disabled["enable"] = False
            common.api_put(args.charger, "ocpp/config_update", disabled)
            time.sleep(2)
            common.api_put(args.charger, "ocpp/reset", None)
            common.api_put(args.charger, "ocpp/config_update", saved_config)
        finally:
            csms.stop()
            csms_tls.close()
            shutil.rmtree(workdir, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
