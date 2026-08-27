#!/usr/bin/env python3
"""Plug and Charge (M01/M02) on a real charger, ISO 15118-20 and -2.

Drives PnC sessions over the EXI codec:

ISO 15118-20 (TLS 1.3 mutual auth):
  1. AuthorizationSetupRes offers PnC and CertificateInstallationService with a GenChallenge
  2. a signed PnC AuthorizationReq with a valid contract chain authorizes (Ongoing then OK)
  3. a wrong GenChallenge answers WARNING_ChallengeInvalid
  4. a forged contract chain (real names, fresh keys) answers WARNING_CertificateValidationError
  5. CertificateInstallationReq is forwarded to the CSMS (Get15118EVCertificate action Install,
     maximumContractCertificateChains set) and stays Ongoing until the CSMS answers

ISO 15118-2 (TLS 1.2):
  6. ServiceDiscoveryRes offers Contract payment and the certificate service
  7. PaymentDetailsReq validates the contract chain and returns a GenChallenge
  8. a signed AuthorizationReq authorizes, a forged chain fails the PaymentDetails
  9. CertificateInstallationReq is forwarded and the CSMS response reaches the EV

Invoked by certificates.py through the firmware test runner.
"""

import argparse
import base64
import hashlib
import json
import struct
import sys
import tempfile
import shutil
import time
from pathlib import Path

import _common as common
from _common import CSMSSim as Csms
from _ocsp_gating import CERTS, provision_chain, ocsp_response_b64, run

SCRIPT_DIR = Path(__file__).parent.resolve()

from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.utils import decode_dss_signature
from cryptography.hazmat.primitives.serialization import load_pem_private_key

ISO20_AC = {"ProtocolNamespace": "urn:iso:std:iso:15118:-20:AC", "VersionNumberMajor": 1,
            "VersionNumberMinor": 0, "SchemaID": 2, "Priority": 1}
ISO2 = {"ProtocolNamespace": "urn:iso:15118:2:2013:MsgDef", "VersionNumberMajor": 2,
        "VersionNumberMinor": 0, "SchemaID": 1, "Priority": 1}

_ready = False


def setup_codec():
    global _ready
    if _ready:
        return
    from iso15118.shared.settings import load_shared_settings
    from iso15118.shared.exificient_exi_codec import ExificientEXICodec
    from iso15118.shared.exi_codec import EXI
    load_shared_settings()
    EXI().set_exi_codec(ExificientEXICodec())
    _ready = True


# ---------------------------------------------------------------------------
# XML signature over an EXI encoded element, for -2 (secp256r1/sha256) and
# -20 (secp521r1/sha512). EcoG's create_signature is secp256r1 only.
# ---------------------------------------------------------------------------

_helper_bin = None


def cbv2g_helper():
    """Compile and cache the libcbv2g SignedInfo encoder helper.

    The firmware reconstructs the SignedInfo with libcbv2g to verify the
    signature. EcoG's exificient codec encodes the same SignedInfo into
    different (also valid) bytes, so a signature over the exificient
    encoding would not verify. We sign over the libcbv2g encoding instead.
    """
    global _helper_bin
    if _helper_bin is not None:
        return _helper_bin
    import glob, subprocess, tempfile
    libs = glob.glob(str(SCRIPT_DIR.parents[3] / ".pio" / "libdeps" / "*" / "libcbv2g"))
    if not libs:
        raise SystemExit("libcbv2g not found under .pio/libdeps, build the firmware first")
    lib = libs[0]
    out = Path(tempfile.gettempdir()) / "pnc_exi_helper"
    srcs = [str(SCRIPT_DIR / "_pnc_exi_helper.c.inc"),
            f"{lib}/lib/cbv2g/common/exi_bitstream.c", f"{lib}/lib/cbv2g/common/exi_basetypes.c",
            f"{lib}/lib/cbv2g/common/exi_basetypes_encoder.c", f"{lib}/lib/cbv2g/common/exi_header.c",
            f"{lib}/lib/cbv2g/iso_20/iso20_CommonMessages_Datatypes.c",
            f"{lib}/lib/cbv2g/iso_20/iso20_CommonMessages_Encoder.c",
            f"{lib}/lib/cbv2g/iso_2/iso2_msgDefDatatypes.c",
            f"{lib}/lib/cbv2g/iso_2/iso2_msgDefEncoder.c"]
    subprocess.run(["gcc", "-O1", f"-I{lib}/include", "-x", "c", *srcs, "-o", str(out)], check=True)
    _helper_bin = str(out)
    return _helper_bin


def canonical_signed_info(mode, digest):
    import subprocess
    out = subprocess.run([cbv2g_helper(), str(mode), digest.hex()],
                         capture_output=True, text=True, check=True)
    return bytes.fromhex(out.stdout.strip())


def make_signature(element, element_id, priv_key, element_ns, sha512):
    from iso15118.shared.exi_codec import EXI
    from iso15118.shared.messages.enums import Namespace
    from iso15118.shared.messages.xmldsig import (
        Transform, Transforms, DigestMethod, SignatureMethod, CanonicalizationMethod,
        Reference, SignedInfo, SignatureValue, Signature)

    element_exi = EXI().to_exi(element, element_ns)
    if sha512:
        digest = hashlib.sha512(element_exi).digest()
        digest_uri = "http://www.w3.org/2001/04/xmlenc#sha512"
        sig_uri = "http://www.w3.org/2001/04/xmldsig-more#ecdsa-sha512"
        hash_alg, coord, mode = ec.ECDSA(__import__("cryptography.hazmat.primitives.hashes",
                                          fromlist=["SHA512"]).SHA512()), 66, 20
    else:
        digest = hashlib.sha256(element_exi).digest()
        digest_uri = "http://www.w3.org/2001/04/xmlenc#sha256"
        sig_uri = "http://www.w3.org/2001/04/xmldsig-more#ecdsa-sha256"
        hash_alg, coord, mode = ec.ECDSA(__import__("cryptography.hazmat.primitives.hashes",
                                          fromlist=["SHA256"]).SHA256()), 32, 2

    ref = Reference(
        transforms=Transforms(transform=[Transform(algorithm="http://www.w3.org/TR/canonical-exi/")]),
        digest_method=DigestMethod(algorithm=digest_uri),
        digest_value=digest, uri="#" + element_id)
    signed_info = SignedInfo(
        canonicalization_method=CanonicalizationMethod(algorithm="http://www.w3.org/TR/canonical-exi/"),
        signature_method=SignatureMethod(algorithm=sig_uri),
        reference=[ref])
    # Sign over the libcbv2g encoding the charger reconstructs, not exificient's
    si_exi = canonical_signed_info(mode, digest)
    der = priv_key.sign(si_exi, hash_alg)
    r, s = decode_dss_signature(der)
    raw = r.to_bytes(coord, "big") + s.to_bytes(coord, "big")
    return Signature(signed_info=signed_info, signature_value=SignatureValue(value=raw))


# ---------------------------------------------------------------------------
# V2GTP framing over the TLS socket
# ---------------------------------------------------------------------------

def send_exi(tls, exi, payload_type):
    tls.sendall(struct.pack("!BBHI", 0x01, 0xFE, payload_type, len(exi)) + exi)


def recv_exi(tls, namespace):
    from iso15118.shared.exi_codec import EXI
    resp = tls.recv(8192)
    assert resp[:2] == b"\x01\xfe", resp.hex()
    payload_len = struct.unpack("!I", resp[4:8])[0]
    return EXI().from_exi(resp[8:8 + payload_len], namespace)


def recv_exi_json(tls, namespace):
    from iso15118.shared.exi_codec import EXI

    resp = tls.recv(8192)
    assert resp[:2] == b"\x01\xfe", resp.hex()
    payload_len = struct.unpack("!I", resp[4:8])[0]
    decoded = EXI().get_exi_codec().decode(resp[8:8 + payload_len], namespace)
    return json.loads(decoded)


def exchange20(tls, message, payload_type=0x8002):
    from iso15118.shared.exi_codec import EXI
    from iso15118.shared.messages.enums import Namespace
    send_exi(tls, EXI().to_exi(message, Namespace.ISO_V20_COMMON_MSG), payload_type)
    return recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)


def exchange2(tls, message, payload_type=0x8001):
    from iso15118.shared.exi_codec import EXI
    from iso15118.shared.messages.enums import Namespace
    send_exi(tls, EXI().to_exi(message, Namespace.ISO_V2_MSG_DEF), payload_type)
    return recv_exi(tls, Namespace.ISO_V2_MSG_DEF)


def sap(tls, protocol):
    import json
    from iso15118.shared.exi_codec import EXI
    from iso15118.shared.messages.enums import Namespace
    exi = EXI().get_exi_codec().encode(
        json.dumps({"supportedAppProtocolReq": {"AppProtocol": [protocol]}}), Namespace.SAP)
    send_exi(tls, exi, 0x8001)
    return recv_exi(tls, Namespace.SAP)


# ---------------------------------------------------------------------------
# TLS connections
# ---------------------------------------------------------------------------

def connect(charger, iface, tls13, client_cert=None, client_key=None):
    import ssl
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    ctx.check_hostname = False
    if tls13:
        ctx.minimum_version = ssl.TLSVersion.TLSv1_3
        ctx.verify_mode = ssl.CERT_REQUIRED
        ctx.load_verify_locations(cafile=str(CERTS / "iso20" / "certs" / "v2gRootCACert.pem"))
        ctx.load_cert_chain(certfile=str(client_cert or CERTS / "iso20" / "certs" / "oemCertChain.pem"),
                            keyfile=str(client_key or CERTS / "iso20" / "private_keys" / "oemLeaf.key"),
                            password="12345")
    else:
        ctx.maximum_version = ssl.TLSVersion.TLSv1_2
        ctx.verify_mode = ssl.CERT_REQUIRED
        ctx.load_verify_locations(cafile=str(CERTS / "iso2" / "certs" / "v2gRootCACert.pem"))
    raw = common.connect_secc(charger, iface)
    raw.settimeout(30)
    return ctx.wrap_socket(raw)


# ---------------------------------------------------------------------------
# Contract chain material
# ---------------------------------------------------------------------------

def contract_chain_objs(iso20):
    """(leaf DER, [subca2 DER, subca1 DER], contract private key)."""
    pki = CERTS / ("iso20" if iso20 else "iso2")
    certs = pki / "certs"
    leaf = (certs / "contractLeafCert.der").read_bytes()
    sub2 = (certs / "moSubCA2Cert.der").read_bytes()
    sub1 = (certs / "moSubCA1Cert.der").read_bytes()
    key = load_pem_private_key((pki / "private_keys" / "contractLeaf.key").read_bytes(), b"12345")
    return leaf, [sub2, sub1], key


def forge_contract_chain(workdir, iso20):
    """Contract chain with the real subject names but fresh keys."""
    pki = CERTS / ("iso20" if iso20 else "iso2") / "certs"
    curve = "secp521r1" if iso20 else "prime256v1"

    def dn(cert):
        out = run(["openssl", "x509", "-in", str(cert), "-noout", "-subject",
                   "-nameopt", "RFC2253"]).stdout.strip()
        return "/" + out.split("subject=", 1)[1].strip().replace(",", "/")

    names = {n: dn(pki / f"{f}.pem") for n, f in
             (("root", "moRootCACert"), ("sub1", "moSubCA1Cert"),
              ("sub2", "moSubCA2Cert"), ("leaf", "contractLeafCert"))}
    keys = {}
    for k in names:
        key = workdir / f"fc_{k}.key"
        run(["openssl", "ecparam", "-name", curve, "-genkey", "-noout", "-out", str(key)])
        keys[k] = key

    root = workdir / "fc_root.pem"
    run(["openssl", "req", "-x509", "-new", "-key", str(keys["root"]), "-sha256",
         "-subj", names["root"], "-days", "60", "-out", str(root)])

    def sign(name, issuer_key, issuer_crt, ca):
        csr = workdir / f"fc_{name}.csr"
        run(["openssl", "req", "-new", "-key", str(keys[name]), "-subj", names[name], "-out", str(csr)])
        ext = workdir / f"fc_{name}.cnf"
        ext.write_text("[e]\nbasicConstraints=critical,CA:" + ("true" if ca else "false") + "\n"
                       "keyUsage=critical," + ("keyCertSign,cRLSign" if ca else "digitalSignature,keyAgreement") + "\n")
        crt = workdir / f"fc_{name}.pem"
        run(["openssl", "x509", "-req", "-in", str(csr), "-CA", str(issuer_crt),
             "-CAkey", str(issuer_key), "-CAcreateserial", "-days", "60", "-sha256",
             "-extfile", str(ext), "-extensions", "e", "-out", str(crt)])
        return crt

    sub1 = sign("sub1", keys["root"], root, True)
    sub2 = sign("sub2", keys["sub1"], sub1, True)
    leaf = sign("leaf", keys["sub2"], sub2, False)

    def der(pem):
        out = workdir / (pem.stem + ".der")
        run(["openssl", "x509", "-in", str(pem), "-outform", "der", "-out", str(out)])
        return out.read_bytes()

    key = load_pem_private_key(keys["leaf"].read_bytes(), None)
    return der(leaf), [der(sub2), der(sub1)], key


# ---------------------------------------------------------------------------
# ISO 15118-20 message builders
# ---------------------------------------------------------------------------

def build_auth_req20(session_id, gen_challenge, leaf, subs, key):
    import time as _t
    from iso15118.shared.messages.enums import Namespace
    from iso15118.shared.messages.iso15118_20.common_messages import (
        AuthorizationReq, PnCAuthReqParams, ContractCertificateChain, AuthEnum)
    from iso15118.shared.messages.iso15118_20.common_types import MessageHeader

    chain = ContractCertificateChain(certificate=leaf, sub_certificates={"Certificate": subs})
    pnc = PnCAuthReqParams(gen_challenge=gen_challenge, contract_cert_chain=chain, id="id1")
    sig = make_signature(pnc, "id1", key, Namespace.ISO_V20_COMMON_MSG, sha512=True)
    return AuthorizationReq(
        header=MessageHeader(session_id=session_id, timestamp=int(_t.time()), signature=sig),
        selected_auth_service=AuthEnum.PNC, pnc_params=pnc)


def open_session20(charger, iface, client_cert=None, client_key=None):
    import time as _t
    from iso15118.shared.messages.iso15118_20.common_messages import (
        SessionSetupReq, AuthorizationSetupReq)
    from iso15118.shared.messages.iso15118_20.common_types import MessageHeader

    tls = connect(charger, iface, tls13=True, client_cert=client_cert, client_key=client_key)
    sap_res = sap(tls, ISO20_AC)
    assert sap_res.response_code == "OK_SuccessfulNegotiation", sap_res

    h = MessageHeader(session_id="00", timestamp=int(_t.time()))
    ss = exchange20(tls, SessionSetupReq(header=h, evcc_id="020000000001"))
    session_id = ss.header.session_id

    h = MessageHeader(session_id=session_id, timestamp=int(_t.time()))
    setup = exchange20(tls, AuthorizationSetupReq(header=h))
    return tls, session_id, setup


# ---------------------------------------------------------------------------
# ISO 15118-2 message builders
# ---------------------------------------------------------------------------

def build_auth_req2(gen_challenge, key):
    from iso15118.shared.messages.enums import Namespace
    from iso15118.shared.messages.iso15118_2.body import AuthorizationReq
    auth = AuthorizationReq(id="id1", gen_challenge=gen_challenge)
    sig = make_signature(auth, "id1", key, Namespace.ISO_V2_MSG_DEF, sha512=False)
    return auth, sig


def wrap_v2g2(body, signature=None):
    import time as _t
    from iso15118.shared.messages.iso15118_2.body import Body
    from iso15118.shared.messages.iso15118_2.msgdef import V2GMessage
    from iso15118.shared.messages.iso15118_2.header import MessageHeader
    header = MessageHeader(session_id=wrap_v2g2.session_id, signature=signature)
    return V2GMessage(header=header, body=body)


def get_variable(csms, name):
    return csms.call("GetVariables", {"getVariableData": [{
        "component": {"name": "ISO15118Ctrlr"},
        "variable": {"name": name},
    }]})["getVariableResult"][0]


def set_variable(csms, name, value):
    return csms.call("SetVariables", {"setVariableData": [{
        "component": {"name": "ISO15118Ctrlr"},
        "variable": {"name": name},
        "attributeValue": value,
    }]})["setVariableResult"][0]["attributeStatus"]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--charger", required=True)
    p.add_argument("--iface")
    p.add_argument("--port", type=int, default=9500)
    args = p.parse_args()

    setup_codec()
    common.require_iso_tls_config(args.charger)
    iface = args.iface or common.route_interface(args.charger)
    local_ip = common.local_ip_towards(args.charger)
    pnc_supported = "iso15118_pnc" in common.api_get(args.charger, "info/features")
    workdir = Path(tempfile.mkdtemp(prefix="pnc_"))

    failures = 0
    saved_pnc_variables = {}
    csms_tls = common.LocalCSMSTls(args.charger, local_ip)

    def check(name, ok, detail=""):
        nonlocal failures
        print(f'{"ok  " if ok else "FAIL"} {name}{": " + str(detail) if detail else ""}')
        failures += 0 if ok else 1

    saved_config = common.api_get(args.charger, "ocpp/config")
    csms = Csms(args.port, interactive=("SignCertificate", "GetCertificateStatus",
                                        "GetCertificateChainStatus", "Get15118EVCertificate"),
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
        pnc_variables = ["Enabled"]
        if pnc_supported:
            pnc_variables.append("ContractCertificateInstallationEnabled")
        for name in pnc_variables:
            result = get_variable(csms, name)
            check(f"ISO15118Ctrlr.{name} is available", result["attributeStatus"] == "Accepted", result)
            if result["attributeStatus"] == "Accepted":
                saved_pnc_variables[name] = result["attributeValue"]
                check(f"ISO15118Ctrlr.{name} can be enabled",
                      set_variable(csms, name, "true") == "Accepted")
        common.enable_debug_mode(args.charger)
        time.sleep(2)

        for kind, pem, name in (
                ("V2GRootCertificate", "iso2", "v2gRootCACert.pem"),
                ("V2GRootCertificate", "iso20", "v2gRootCACert.pem"),
                ("OEMRootCertificate", "iso20", "oemRootCACert.pem"),
                ("MORootCertificate", "iso2", "moRootCACert.pem"),
                ("MORootCertificate", "iso20", "moRootCACert.pem")):
            res = csms.call("InstallCertificate", {
                "certificateType": kind,
                "certificate": (CERTS / pem / "certs" / name).read_text()})
            assert res["status"] == "Accepted", (kind, pem, res)

        provision_chain(csms, workdir, iso20=False, with_aia=False)
        chain20 = provision_chain(csms, workdir, iso20=True, with_aia=True)
        (workdir / "leaf20.pem").write_text(chain20.split("-----END CERTIFICATE-----")[0] + "-----END CERTIFICATE-----\n")
        time.sleep(3)

        status_req, msg_id = csms.expect("GetCertificateStatus", timeout=120)
        b64, _ = ocsp_response_b64(workdir, workdir / "leaf20.pem")
        csms.respond(msg_id, {"status": "Accepted", "ocspResult": b64})
        time.sleep(5)

        if pnc_supported:
            run_iso2(args, iface, csms, workdir, check)
            common.disable_debug_mode(args.charger)
            time.sleep(1)
            common.enable_debug_mode(args.charger)
            time.sleep(2)
            run_iso20(args, iface, csms, workdir, check)
            run_service_gating(args, iface, csms, check)
        else:
            run_pnc_disabled(args, iface, csms, check)

    finally:
        cleanup_errors = []
        for name, value in saved_pnc_variables.items():
            try:
                if set_variable(csms, name, value) != "Accepted":
                    raise RuntimeError(f"Could not restore ISO15118Ctrlr.{name}")
            except Exception as e:
                cleanup_errors.append(e)
        try:
            disabled = dict(saved_config)
            disabled["enable"] = False
            common.api_put(args.charger, "ocpp/config_update", disabled)
            time.sleep(2)
            common.api_put(args.charger, "ocpp/reset", None)
            common.api_put(args.charger, "ocpp/config_update", saved_config)
        except Exception as e:
            cleanup_errors.append(e)
        for e in cleanup_errors:
            print(f"cleanup failed, restore the OCPP state manually: {e}")
            failures += 1
        csms.stop()
        csms_tls.close()
        shutil.rmtree(workdir, ignore_errors=True)

    print("PASS" if failures == 0 else f"FAIL ({failures} failures)")
    sys.exit(0 if failures == 0 else 1)


def poll_auth20(tls, session_id, req, tries=8):
    """Send req, follow EVSEProcessing Ongoing until a final response."""
    res = exchange20(tls, req)
    while res.evse_processing == "Ongoing" and tries > 0:
        time.sleep(1)
        res = exchange20(tls, req)
        tries -= 1
    return res


def answer_vehicle_chain_good(csms, timeout=30):
    """Answer the vehicle chain OCSP request for the OEM client cert.

    A cached Good status may suppress the request.
    """
    try:
        req, msg_id = csms.expect("GetCertificateChainStatus", timeout=timeout)
    except TimeoutError:
        return
    entries = [{"certificateHashData": r["certificateHashData"], "source": "OCSP",
                "status": "Good", "nextUpdate": "2027-01-01T00:00:00Z"}
               for r in req["certificateStatusRequests"]]
    csms.respond(msg_id, {"certificateStatus": entries})


def run_service_gating(args, iface, csms, check):
    from iso15118.shared.messages.iso15118_2.body import (
        Body, ServiceDiscoveryReq, SessionSetupReq as SS2)
    from iso15118.shared.messages.iso15118_2.datatypes import ServiceCategory

    variable = "ContractCertificateInstallationEnabled"
    check(f"ISO15118Ctrlr.{variable} can be disabled",
          set_variable(csms, variable, "false") == "Accepted")
    time.sleep(1)

    tls, session_id, setup = open_session20(args.charger, iface)
    answer_vehicle_chain_good(csms, timeout=2)
    check(f"ISO-20 certificate installation is not offered when {variable} is false",
          not setup.cert_install_service, setup.cert_install_service)
    tls.close()
    time.sleep(1)

    tls = connect(args.charger, iface, tls13=False)
    sap(tls, ISO2)
    wrap_v2g2.session_id = "00"
    res = exchange2_body(tls, Body(SessionSetupReq=SS2(evcc_id="0A1B2C3D4E5F")))
    wrap_v2g2.session_id = res.header.session_id
    res = exchange2_body(tls, Body(ServiceDiscoveryReq=ServiceDiscoveryReq()))
    services = res.body.service_discovery_res.service_list
    has_cert = services is not None and any(
        service.service_category == ServiceCategory.CERTIFICATE for service in services.services)
    check(f"ISO-2 certificate service is not offered when {variable} is false", not has_cert)
    tls.close()
    time.sleep(1)

    check(f"ISO15118Ctrlr.{variable} can be re-enabled",
          set_variable(csms, variable, "true") == "Accepted")
    time.sleep(1)

    check("ISO15118Ctrlr.Enabled can be disabled",
          set_variable(csms, "Enabled", "false") == "Accepted")
    time.sleep(1)
    check("ISO15118Ctrlr.Enabled disables SDP",
          common.sdp_request(iface, timeout=2) is None)
    assert_no_ev_certificate_request(
        csms, check, "no Get15118EVCertificate while ISO15118Ctrlr.Enabled is false")
    check("ISO15118Ctrlr.Enabled can be re-enabled",
          set_variable(csms, "Enabled", "true") == "Accepted")
    deadline = time.monotonic() + 15
    while common.sdp_request(iface, timeout=1) is None:
        if time.monotonic() >= deadline:
            raise TimeoutError("ISO15118Ctrlr.Enabled did not restore SDP")

    tls, session_id, setup = open_session20(args.charger, iface)
    answer_vehicle_chain_good(csms, timeout=2)
    check("ISO-20 certificate installation is offered again after service gating tests",
          setup.cert_install_service)
    tls.close()
    time.sleep(1)


def assert_no_ev_certificate_request(csms, check, description):
    unexpected = None
    try:
        unexpected = csms.expect("Get15118EVCertificate", timeout=2)
    except TimeoutError:
        pass
    check(description, unexpected is None, unexpected)


def run_pnc_disabled(args, iface, csms, check):
    from iso15118.shared.exi_codec import EXI
    from iso15118.shared.messages.enums import Namespace
    from iso15118.shared.messages.iso15118_20.common_messages import AuthEnum

    tls, session_id, setup = open_session20(args.charger, iface)
    check("ISO-20 non-PnC build offers only EIM",
          setup.auth_services == [AuthEnum.EIM], setup.auth_services)
    check("ISO-20 non-PnC build omits CertificateInstallationService",
          not setup.cert_install_service, setup.cert_install_service)
    leaf, subs, key = contract_chain_objs(iso20=True)
    req = build_auth_req20(session_id, bytes(16), leaf, subs, key)
    res = exchange20(tls, req)
    check("ISO-20 non-PnC build rejects PnC authorization selection",
          res.response_code == "WARNING_AuthorizationSelectionInvalid", res.response_code)

    cin = build_cert_install_req20(session_id)
    send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
    res = recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
    check("ISO-20 non-PnC build rejects CertificateInstallationReq",
          res.response_code.startswith(("WARNING", "FAILED")) and res.evse_processing == "Finished",
          res.response_code)
    assert_no_ev_certificate_request(csms, check,
                                     "ISO-20 non-PnC build does not forward certificate installation")
    tls.close()
    time.sleep(1)

    from iso15118.shared.messages.datatypes import SelectedService
    from iso15118.shared.messages.enums import AuthEnum as AuthEnum2
    from iso15118.shared.messages.iso15118_2.body import (
        Body, CertificateInstallationReq, PaymentServiceSelectionReq,
        SelectedServiceList, ServiceDiscoveryReq, SessionSetupReq,
    )
    from iso15118.shared.messages.iso15118_2.datatypes import ServiceCategory

    tls = connect(args.charger, iface, tls13=False)
    sap(tls, ISO2)
    wrap_v2g2.session_id = "00"
    res = exchange2_body(tls, Body(SessionSetupReq=SessionSetupReq(evcc_id="0A1B2C3D4E5F")))
    wrap_v2g2.session_id = res.header.session_id
    res = exchange2_body(tls, Body(ServiceDiscoveryReq=ServiceDiscoveryReq()))
    discovery = res.body.service_discovery_res
    payment_opts = [str(getattr(option, "value", option)) for option in discovery.auth_option_list.auth_options]
    check("ISO-2 non-PnC build offers only ExternalPayment",
          not any("Contract" in option for option in payment_opts), payment_opts)
    services = discovery.service_list
    has_certificate_service = services is not None and any(
        service.service_category == ServiceCategory.CERTIFICATE for service in services.services)
    check("ISO-2 non-PnC build omits the certificate service", not has_certificate_service)

    res = exchange2_body(tls, Body(PaymentServiceSelectionReq=PaymentServiceSelectionReq(
        selected_auth_option=AuthEnum2.PNC_V2,
        selected_service_list=SelectedServiceList(
            selected_service=[SelectedService(service_id=1)]))))
    check("ISO-2 non-PnC build rejects Contract payment selection",
          res.body.payment_service_selection_res.response_code == "FAILED_PaymentSelectionInvalid",
          res.body.payment_service_selection_res.response_code)

    msg = wrap_v2g2(Body(CertificateInstallationReq=build_cert_install_req2()))
    send_exi(tls, EXI().to_exi(msg, Namespace.ISO_V2_MSG_DEF), 0x8001)
    res = recv_exi(tls, Namespace.ISO_V2_MSG_DEF)
    check("ISO-2 non-PnC build rejects CertificateInstallationReq",
          res.body.certificate_installation_res.response_code == "FAILED_NoCertificateAvailable",
          res.body.certificate_installation_res.response_code)
    assert_no_ev_certificate_request(csms, check,
                                     "ISO-2 non-PnC build does not forward certificate installation")
    tls.close()
    time.sleep(1)


def expect_ev_certificate(csms, timeout=60):
    deadline = time.monotonic() + timeout
    while True:
        action, payload, msg_id = csms.expect_any(
            {"Get15118EVCertificate", "GetCertificateChainStatus"},
            timeout=deadline - time.monotonic())
        if action == "Get15118EVCertificate":
            return payload, msg_id
        entries = [{"certificateHashData": r["certificateHashData"], "source": "OCSP",
                    "status": "Good", "nextUpdate": "2027-01-01T00:00:00Z"}
                   for r in payload["certificateStatusRequests"]]
        csms.respond(msg_id, {"certificateStatus": entries})


def run_iso20(args, iface, csms, workdir, check):
    from iso15118.shared.messages.iso15118_20.common_messages import AuthEnum

    # 1. AuthorizationSetup offers PnC and cert installation
    tls, session_id, setup = open_session20(args.charger, iface)
    answer_vehicle_chain_good(csms)  # the OEM client chain, cached for later sessions
    check("AuthorizationSetupRes offers PnC [V2G20-2564]", AuthEnum.PNC in setup.auth_services, setup.auth_services)
    check("AuthorizationSetupRes offers CertificateInstallationService [M01]", setup.cert_install_service)
    challenge = setup.pnc_as_res.gen_challenge if setup.pnc_as_res else None
    check("AuthorizationSetupRes carries a 16 byte GenChallenge [V2G20-2565]",
          challenge is not None and len(challenge) == 16)

    # 2. valid signed PnC AuthorizationReq authorizes
    leaf, subs, key = contract_chain_objs(iso20=True)
    req = build_auth_req20(session_id, challenge, leaf, subs, key)
    res = poll_auth20(tls, session_id, req)
    check("PnC authorization OK with a valid contract chain [V2G20-2564]",
          res.response_code == "OK" and res.evse_processing == "Finished", res.response_code)
    tls.close()
    time.sleep(1)

    # Missing and corrupt signatures must not authorize a valid contract chain.
    for description, corrupt in (("missing", False), ("corrupt", True)):
        tls, session_id, setup = open_session20(args.charger, iface)
        answer_vehicle_chain_good(csms, timeout=2)
        req = build_auth_req20(session_id, setup.pnc_as_res.gen_challenge, leaf, subs, key)
        if corrupt:
            value = req.header.signature.signature_value.value
            req.header.signature.signature_value.value = bytes([value[0] ^ 1]) + value[1:]
        else:
            req.header.signature = None
        res = poll_auth20(tls, session_id, req)
        check(f"PnC authorization rejects a {description} signature [V2G20-2564]",
              res.response_code == "WARNING_CertificateValidationError", res.response_code)
        tls.close()
        time.sleep(1)

    # 3. wrong GenChallenge
    tls, session_id, setup = open_session20(args.charger, iface)
    answer_vehicle_chain_good(csms, timeout=2)
    req = build_auth_req20(session_id, bytes(16), leaf, subs, key)
    res = poll_auth20(tls, session_id, req)
    check("PnC authorization WARNING_ChallengeInvalid on a wrong challenge [V2G20-2216]",
          res.response_code == "WARNING_ChallengeInvalid", res.response_code)
    tls.close()
    time.sleep(1)

    # 4. forged contract chain, real names, fresh keys
    tls, session_id, setup = open_session20(args.charger, iface)
    answer_vehicle_chain_good(csms, timeout=2)
    challenge = setup.pnc_as_res.gen_challenge
    f_leaf, f_subs, f_key = forge_contract_chain(workdir, iso20=True)
    req = build_auth_req20(session_id, challenge, f_leaf, f_subs, f_key)
    res = poll_auth20(tls, session_id, req)
    check("PnC authorization WARNING_CertificateValidationError on a forged chain [V2G20-2215]",
          res.response_code == "WARNING_CertificateValidationError", res.response_code)
    tls.close()
    time.sleep(1)

    # 5. CertificateInstallationReq forwarded to the CSMS
    tls, session_id, setup = open_session20(args.charger, iface)
    answer_vehicle_chain_good(csms, timeout=2)
    cin = build_cert_install_req20(session_id)
    from iso15118.shared.exi_codec import EXI
    from iso15118.shared.messages.enums import Namespace
    send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
    ev_req, ev_msg = expect_ev_certificate(csms)
    check("CertificateInstallationReq forwarded with action Install [M01.FR.01]",
          ev_req.get("action") == "Install", ev_req.get("action"))
    check("Get15118EVCertificate carries maximumContractCertificateChains [M01.FR.03]",
          "maximumContractCertificateChains" in ev_req)
    res = recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
    check("CertificateInstallationRes Ongoing while the CSMS is pending [V2G20-2583]",
          res.evse_processing == "Ongoing", res.evse_processing)
    csms.respond(ev_msg, {"status": "Accepted", "exiResponse": build_cert_install_res20(session_id),
                          "remainingContracts": 7})
    time.sleep(1)
    send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
    res = recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
    check("ISO-20 CertificateInstallationRes from the CSMS reaches the EV [M01]",
          res.response_code == "OK" and res.evse_processing == "Finished", res.response_code)
    check("ISO-20 CertificateInstallationRes carries remainingContracts [M01.FR.05]",
          res.remaining_contract_cert_chains == 7, res.remaining_contract_cert_chains)
    tls.close()
    time.sleep(1)

    for remaining, expected in ((-17, 0), (300, 255)):
        tls, session_id, setup = open_session20(args.charger, iface)
        answer_vehicle_chain_good(csms, timeout=2)
        cin = build_cert_install_req20(session_id)
        send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
        ev_req, ev_msg = expect_ev_certificate(csms)
        recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
        csms.respond(ev_msg, {"status": "Accepted", "exiResponse": build_cert_install_res20(session_id),
                              "remainingContracts": remaining})
        time.sleep(1)
        send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
        res = recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
        check(f"ISO-20 remainingContracts {remaining} clamps to {expected} [M01.FR.05]",
              res.remaining_contract_cert_chains == expected, res.remaining_contract_cert_chains)
        tls.close()
        time.sleep(1)

    run_iso20_malformed_responses(args, iface, csms, check)

    # A failed OCPP result must produce a final ISO response.
    tls, session_id, setup = open_session20(args.charger, iface)
    answer_vehicle_chain_good(csms, timeout=2)
    cin = build_cert_install_req20(session_id)
    send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
    ev_req, ev_msg = expect_ev_certificate(csms)
    recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
    csms.respond(ev_msg, {"status": "Failed", "exiResponse": ""})
    time.sleep(1)
    send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
    res = recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
    check("CertificateInstallationRes fails after a CSMS failure [M01]",
          res.response_code.startswith("WARNING") or res.response_code.startswith("FAILED"), res.response_code)
    tls.close()
    time.sleep(1)


def build_cert_install_req20(session_id):
    import time as _t
    from iso15118.shared.messages.enums import Namespace
    from iso15118.shared.messages.iso15118_20.common_messages import (
        CertificateInstallationReq, SignedCertificateChain)
    from iso15118.shared.messages.iso15118_20.common_types import MessageHeader, RootCertificateIDList
    from iso15118.shared.messages.xmldsig import X509IssuerSerial

    certs = CERTS / "iso20" / "certs"
    oem_leaf = (certs / "oemLeafCert.der").read_bytes()
    oem_sub2 = (certs / "oemSubCA2Cert.der").read_bytes()
    oem_sub1 = (certs / "oemSubCA1Cert.der").read_bytes()
    oem_chain = SignedCertificateChain(
        id="id1", certificate=oem_leaf, sub_certificates={"Certificate": [oem_sub2, oem_sub1]})
    root_id = X509IssuerSerial(x509_issuer_name="CN=V2GRootCA,O=WARP,C=DE,DC=V2G", x509_serial_number=22345)
    key = load_pem_private_key((CERTS / "iso20" / "private_keys" / "oemLeaf.key").read_bytes(), b"12345")
    sig = make_signature(oem_chain, "id1", key, Namespace.ISO_V20_COMMON_MSG, sha512=True)
    return CertificateInstallationReq(
        header=MessageHeader(session_id=session_id, timestamp=int(_t.time()), signature=sig),
        oem_prov_cert_chain=oem_chain,
        root_cert_id_list=RootCertificateIDList(root_cert_ids=[root_id]),
        max_contract_cert_chains=3, prioritized_emaids=None)


def build_cert_install_res20(session_id):
    import time as _t
    from iso15118.shared.exi_codec import EXI
    from iso15118.shared.messages.enums import Namespace
    from iso15118.shared.messages.iso15118_20.common_messages import (
        CertificateChain, CertificateInstallationRes, ContractCertificateChain,
        ECDHCurve, SignedInstallationData)
    from iso15118.shared.messages.iso15118_20.common_types import MessageHeader

    certs = CERTS / "iso20" / "certs"
    cps_chain = CertificateChain(
        certificate=(certs / "cpsLeafCert.der").read_bytes(),
        sub_certificates={"Certificate": [
            (certs / "cpsSubCA2Cert.der").read_bytes(),
            (certs / "cpsSubCA1Cert.der").read_bytes(),
        ]})
    contract_chain = ContractCertificateChain(
        certificate=(certs / "contractLeafCert.der").read_bytes(),
        sub_certificates={"Certificate": [
            (certs / "moSubCA2Cert.der").read_bytes(),
            (certs / "moSubCA1Cert.der").read_bytes(),
        ]})
    data = SignedInstallationData(
        id="id1", contract_cert_chain=contract_chain, ecdh_curve=ECDHCurve.secp_521,
        dh_public_key=b"\x04", secp521_encrypted_private_key=bytes(94))
    res = CertificateInstallationRes(
        header=MessageHeader(session_id=session_id, timestamp=int(_t.time())),
        response_code="OK", evse_processing="Finished", cps_certificate_chain=cps_chain,
        signed_installation_data=data, remaining_contract_cert_chains=0)
    return base64.b64encode(EXI().to_exi(res, Namespace.ISO_V20_COMMON_MSG)).decode()


def run_iso20_malformed_responses(args, iface, csms, check):
    from iso15118.shared.exi_codec import EXI
    from iso15118.shared.messages.enums import Namespace

    def submit(exi_response):
        tls, session_id, setup = open_session20(args.charger, iface)
        answer_vehicle_chain_good(csms, timeout=2)
        cin = build_cert_install_req20(session_id)
        send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
        ev_req, ev_msg = expect_ev_certificate(csms)
        recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
        csms.respond(ev_msg, {"status": "Accepted", "exiResponse": exi_response,
                              "remainingContracts": 1})
        time.sleep(1)
        send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
        return tls, recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)

    tls, res = submit(base64.b64encode(b"\x00").decode())
    check("ISO-20 rejects an accepted OCPP response with malformed EXI",
          res.response_code.startswith(("WARNING", "FAILED")), res.response_code)
    tls.close()
    time.sleep(1)

    # A schema-valid request is valid EXI but the wrong response message.
    tls, session_id, setup = open_session20(args.charger, iface)
    answer_vehicle_chain_good(csms, timeout=2)
    cin = build_cert_install_req20(session_id)
    send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
    ev_req, ev_msg = expect_ev_certificate(csms)
    recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
    csms.respond(ev_msg, {"status": "Accepted", "exiResponse": ev_req["exiRequest"],
                          "remainingContracts": 1})
    time.sleep(1)
    send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
    res = recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
    check("ISO-20 rejects an accepted OCPP response with the wrong EXI message",
          res.response_code.startswith(("WARNING", "FAILED")), res.response_code)
    tls.close()
    time.sleep(1)

    # A decoded malformed response must not poison the next session.
    tls, session_id, setup = open_session20(args.charger, iface)
    answer_vehicle_chain_good(csms, timeout=2)
    cin = build_cert_install_req20(session_id)
    send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
    ev_req, ev_msg = expect_ev_certificate(csms)
    recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
    csms.respond(ev_msg, {"status": "Accepted", "exiResponse": build_cert_install_res20(session_id),
                          "remainingContracts": 1})
    time.sleep(1)
    send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
    res = recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
    check("ISO-20 certificate installation recovers in the next session",
          res.response_code == "OK" and res.remaining_contract_cert_chains == 1, res.response_code)
    tls.close()
    time.sleep(1)

    for description, exi_response in (("invalid base64", "%%%"), ("empty", "")):
        tls, res = submit(exi_response)
        check(f"ISO-20 rejects an accepted OCPP response with {description}",
              res.response_code.startswith(("WARNING", "FAILED")), res.response_code)
        tls.close()
        time.sleep(1)

        # Base64 failures must reset the OCPP operation for a fresh ISO-20 session.
        tls, session_id, setup = open_session20(args.charger, iface)
        answer_vehicle_chain_good(csms, timeout=2)
        cin = build_cert_install_req20(session_id)
        send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
        res = recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
        recovered = res.evse_processing == "Ongoing"
        if recovered:
            ev_req, ev_msg = expect_ev_certificate(csms)
            csms.respond(ev_msg, {"status": "Accepted", "exiResponse": build_cert_install_res20(session_id),
                                  "remainingContracts": 1})
            time.sleep(1)
            send_exi(tls, EXI().to_exi(cin, Namespace.ISO_V20_COMMON_MSG), 0x8002)
            res = recv_exi(tls, Namespace.ISO_V20_COMMON_MSG)
            recovered = res.response_code == "OK"
        check(f"ISO-20 recovers after {description} in the next session", recovered, res.response_code)
        tls.close()
        time.sleep(1)
        if not recovered:
            clear_ev_cert_state_via_iso2(args, iface, csms)


def clear_ev_cert_state_via_iso2(args, iface, csms):
    """Allow later cases to run after an ISO-20 Failed state recovery defect."""
    from iso15118.shared.exi_codec import EXI
    from iso15118.shared.messages.enums import Namespace
    from iso15118.shared.messages.iso15118_2.body import Body

    cin = build_cert_install_req2()
    tls = open_cert_session2(args, iface)
    msg = wrap_v2g2(Body(CertificateInstallationReq=cin))
    send_exi(tls, EXI().to_exi(msg, Namespace.ISO_V2_MSG_DEF), 0x8001)
    ev_req, ev_msg = expect_ev_certificate(csms)
    csms.respond(ev_msg, {"status": "Accepted", "exiResponse": build_cert_res2(ev_req["exiRequest"])})
    recv_exi(tls, Namespace.ISO_V2_MSG_DEF)
    tls.close()
    time.sleep(1)


def run_iso2(args, iface, csms, workdir, check):
    from iso15118.shared.messages.iso15118_2.body import (
        Body, ServiceDiscoveryReq, ServiceDetailReq, PaymentServiceSelectionReq,
        PaymentDetailsReq, AuthorizationReq, SelectedServiceList)
    from iso15118.shared.messages.datatypes import SelectedService
    from iso15118.shared.messages.iso15118_2.datatypes import (
        ServiceCategory, ServiceID as SID)
    from iso15118.shared.messages.enums import AuthEnum as AuthEnum2

    tls = connect(args.charger, iface, tls13=False)
    sap_res = sap(tls, ISO2)
    assert sap_res.response_code == "OK_SuccessfulNegotiation", sap_res

    from iso15118.shared.messages.iso15118_2.body import SessionSetupReq as SS2
    wrap_v2g2.session_id = "00"
    res = exchange2_body(tls, Body(SessionSetupReq=SS2(evcc_id="0A1B2C3D4E5F")))
    wrap_v2g2.session_id = res.header.session_id

    res = exchange2_body(tls, Body(ServiceDiscoveryReq=ServiceDiscoveryReq()))
    sd = res.body.service_discovery_res
    payment_opts = [str(getattr(o, "value", o)) for o in sd.auth_option_list.auth_options]
    check("ServiceDiscoveryRes offers Contract payment [PnC]",
          any("Contract" in o for o in payment_opts), payment_opts)
    has_cert = sd.service_list is not None and any(
        s.service_category == ServiceCategory.CERTIFICATE for s in sd.service_list.services)
    check("ServiceDiscoveryRes offers the certificate service [M01]", has_cert)

    res = exchange2_body(tls, Body(PaymentServiceSelectionReq=PaymentServiceSelectionReq(
        selected_auth_option=AuthEnum2.PNC_V2,
        selected_service_list=SelectedServiceList(selected_service=[SelectedService(service_id=1)]))))
    check("PaymentServiceSelection Contract accepted",
          res.body.payment_service_selection_res.response_code == "OK",
          res.body.payment_service_selection_res.response_code)

    leaf, subs, key = contract_chain_objs(iso20=False)
    from iso15118.shared.messages.iso15118_2.datatypes import (
        CertificateChain as CC2, SubCertificates as SC2)
    chain = CC2(certificate=leaf, sub_certificates=SC2(certificates=subs))
    res = exchange2_body(tls, Body(PaymentDetailsReq=PaymentDetailsReq(
        emaid="DEWRP123456789A", cert_chain=chain)))
    pd = res.body.payment_details_res
    check("PaymentDetailsRes OK with a valid contract chain [V2G2-cert]",
          pd.response_code == "OK", pd.response_code)
    gen_challenge = pd.gen_challenge if pd.response_code == "OK" else bytes(16)

    auth, sig = build_auth_req2(gen_challenge, key)
    res = exchange2_body(tls, Body(AuthorizationReq=auth), signature=sig)
    check("Signed AuthorizationReq authorizes [V2G2-461]",
          res.body.authorization_res.response_code == "OK", res.body.authorization_res.response_code)
    tls.close()
    time.sleep(1)

    for description, corrupt in (("missing", False), ("corrupt", True)):
        tls, gen_challenge = open_pnc_session2(args, iface, leaf, subs)
        auth, sig = build_auth_req2(gen_challenge, key)
        if corrupt:
            value = sig.signature_value.value
            sig.signature_value.value = bytes([value[0] ^ 1]) + value[1:]
        else:
            sig = None
        res = exchange2_body(tls, Body(AuthorizationReq=auth), signature=sig)
        check(f"ISO-2 AuthorizationReq rejects a {description} signature [V2G2-461]",
              res.body.authorization_res.response_code == "FAILED_SignatureError",
              res.body.authorization_res.response_code)
        tls.close()
        time.sleep(1)

    # forged chain fails PaymentDetails
    tls = connect(args.charger, iface, tls13=False)
    sap(tls, ISO2)
    wrap_v2g2.session_id = "00"
    res = exchange2_body(tls, Body(SessionSetupReq=SS2(evcc_id="0A1B2C3D4E5F")))
    wrap_v2g2.session_id = res.header.session_id
    exchange2_body(tls, Body(ServiceDiscoveryReq=ServiceDiscoveryReq()))
    exchange2_body(tls, Body(PaymentServiceSelectionReq=PaymentServiceSelectionReq(
        selected_auth_option=AuthEnum2.PNC_V2,
        selected_service_list=SelectedServiceList(selected_service=[SelectedService(service_id=1)]))))
    f_leaf, f_subs, f_key = forge_contract_chain(workdir, iso20=False)
    fchain = CC2(certificate=f_leaf, sub_certificates=SC2(certificates=f_subs))
    res = exchange2_body(tls, Body(PaymentDetailsReq=PaymentDetailsReq(
        emaid="DEWRP123456789A", cert_chain=fchain)))
    check("PaymentDetailsRes fails on a forged contract chain [V2G2-469]",
          res.body.payment_details_res.response_code.startswith("FAILED"),
          res.body.payment_details_res.response_code)
    tls.close()
    time.sleep(1)

    # CertificateInstallationReq forwarded, CSMS answers with a real -2 response
    run_iso2_cert_install(args, iface, csms, check)


def open_pnc_session2(args, iface, leaf, subs):
    from iso15118.shared.messages.iso15118_2.body import (
        Body, PaymentDetailsReq, PaymentServiceSelectionReq, SelectedServiceList,
        ServiceDiscoveryReq, SessionSetupReq as SS2)
    from iso15118.shared.messages.datatypes import SelectedService
    from iso15118.shared.messages.enums import AuthEnum as AuthEnum2
    from iso15118.shared.messages.iso15118_2.datatypes import (
        CertificateChain as CC2, SubCertificates as SC2)

    tls = connect(args.charger, iface, tls13=False)
    sap(tls, ISO2)
    wrap_v2g2.session_id = "00"
    res = exchange2_body(tls, Body(SessionSetupReq=SS2(evcc_id="0A1B2C3D4E5F")))
    wrap_v2g2.session_id = res.header.session_id
    exchange2_body(tls, Body(ServiceDiscoveryReq=ServiceDiscoveryReq()))
    exchange2_body(tls, Body(PaymentServiceSelectionReq=PaymentServiceSelectionReq(
        selected_auth_option=AuthEnum2.PNC_V2,
        selected_service_list=SelectedServiceList(selected_service=[SelectedService(service_id=1)]))))
    chain = CC2(certificate=leaf, sub_certificates=SC2(certificates=subs))
    res = exchange2_body(tls, Body(PaymentDetailsReq=PaymentDetailsReq(
        emaid="DEWRP123456789A", cert_chain=chain)))
    assert res.body.payment_details_res.response_code == "OK", res.body.payment_details_res
    return tls, res.body.payment_details_res.gen_challenge


def exchange2_body(tls, body, signature=None):
    from iso15118.shared.exi_codec import EXI
    from iso15118.shared.messages.enums import Namespace
    msg = wrap_v2g2(body, signature)
    send_exi(tls, EXI().to_exi(msg, Namespace.ISO_V2_MSG_DEF), 0x8001)
    return recv_exi(tls, Namespace.ISO_V2_MSG_DEF)


def build_cert_install_req2():
    from iso15118.shared.messages.iso15118_2.body import CertificateInstallationReq
    from iso15118.shared.messages.iso15118_2.datatypes import RootCertificateIDList
    from iso15118.shared.messages.xmldsig import X509IssuerSerial

    root_id = X509IssuerSerial(
        x509_issuer_name="CN=V2GRootCA,O=WARP,C=DE,DC=V2G", x509_serial_number=12345)
    return CertificateInstallationReq(
        id="id1", oem_provisioning_cert=(CERTS / "iso2" / "certs" / "oemLeafCert.der").read_bytes(),
        list_of_root_cert_ids=RootCertificateIDList(x509_issuer_serials=[root_id]))


def run_iso2_cert_install(args, iface, csms, check):
    from iso15118.shared.messages.iso15118_2.body import Body, CertificateUpdateReq
    from iso15118.shared.messages.enums import Namespace
    from iso15118.shared.messages.iso15118_2.datatypes import RootCertificateIDList
    from iso15118.shared.messages.xmldsig import X509IssuerSerial
    from iso15118.shared.exi_codec import EXI

    certs = CERTS / "iso2" / "certs"
    from iso15118.shared.messages.iso15118_2.datatypes import SubCertificates as SC2
    root_id = X509IssuerSerial(x509_issuer_name="CN=V2GRootCA,O=WARP,C=DE,DC=V2G", x509_serial_number=12345)
    cin = build_cert_install_req2()
    tls = open_cert_session2(args, iface)
    msg = wrap_v2g2(Body(CertificateInstallationReq=cin))
    send_exi(tls, EXI().to_exi(msg, Namespace.ISO_V2_MSG_DEF), 0x8001)
    ev_req, ev_msg = expect_ev_certificate(csms)
    check("ISO-2 CertificateInstallationReq forwarded, no maximumContractCertificateChains [M01.FR.02]",
          ev_req.get("action") == "Install" and "maximumContractCertificateChains" not in ev_req, ev_req.get("action"))
    csms.respond(ev_msg, {"status": "Accepted", "exiResponse": build_cert_res2(ev_req["exiRequest"])})
    res = recv_exi(tls, Namespace.ISO_V2_MSG_DEF)
    check("ISO-2 CertificateInstallationRes forwarded to the EV [M01]",
          res.body.certificate_installation_res is not None
          and res.body.certificate_installation_res.response_code == "OK",
          res.body.certificate_installation_res.response_code if res.body.certificate_installation_res else "none")
    tls.close()
    time.sleep(1)

    leaf, subs, key = contract_chain_objs(iso20=False)
    contract_chain = SC2(certificates=subs)
    from iso15118.shared.messages.iso15118_2.datatypes import CertificateChain as CC2, EMAID
    update = CertificateUpdateReq(
        id="id1", contract_cert_chain=CC2(certificate=leaf, sub_certificates=contract_chain),
        emaid=EMAID(id="id2", value="DEWRP123456789A"),
        list_of_root_cert_ids=RootCertificateIDList(x509_issuer_serials=[root_id]))
    tls = open_cert_session2(args, iface)
    msg = wrap_v2g2(Body(CertificateUpdateReq=update))
    send_exi(tls, EXI().to_exi(msg, Namespace.ISO_V2_MSG_DEF), 0x8001)
    ev_req, ev_msg = expect_ev_certificate(csms)
    check("ISO-2 CertificateUpdateReq forwarded with action Update [M02]",
          ev_req.get("action") == "Update" and "maximumContractCertificateChains" not in ev_req,
          ev_req.get("action"))
    csms.respond(ev_msg, {"status": "Accepted", "exiResponse": build_cert_res2(
        ev_req["exiRequest"], update=True, session_id=wrap_v2g2.session_id)})
    res = recv_exi(tls, Namespace.ISO_V2_MSG_DEF)
    check("ISO-2 CertificateUpdateRes forwarded to the EV [M02]",
          res.body.certificate_update_res is not None
          and res.body.certificate_update_res.response_code == "OK",
          res.body.certificate_update_res.response_code if res.body.certificate_update_res else "none")
    tls.close()
    time.sleep(1)

    run_iso2_malformed_responses(args, iface, csms, cin, check)


def open_cert_session2(args, iface):
    from iso15118.shared.messages.iso15118_2.body import (
        Body, PaymentServiceSelectionReq, SelectedServiceList, ServiceDiscoveryReq,
        SessionSetupReq as SS2)
    from iso15118.shared.messages.datatypes import SelectedService
    from iso15118.shared.messages.enums import AuthEnum as AuthEnum2

    tls = connect(args.charger, iface, tls13=False)
    sap(tls, ISO2)
    wrap_v2g2.session_id = "00"
    res = exchange2_body(tls, Body(SessionSetupReq=SS2(evcc_id="0A1B2C3D4E5F")))
    wrap_v2g2.session_id = res.header.session_id
    exchange2_body(tls, Body(ServiceDiscoveryReq=ServiceDiscoveryReq()))
    exchange2_body(tls, Body(PaymentServiceSelectionReq=PaymentServiceSelectionReq(
        selected_auth_option=AuthEnum2.PNC_V2,
        selected_service_list=SelectedServiceList(selected_service=[
            SelectedService(service_id=1), SelectedService(service_id=2)]))))
    return tls


def build_cert_res2(exi_request_b64, update=False, session_id=None):
    """Build a real -2 certificate response signed by the dev CPS chain.

    Replicates the EcoG SECC backend mock, whose own isinstance guard never
    matches a full V2G message. The response is a genuine, signed
    CertificateInstallationRes or CertificateUpdateRes the charger relays to the EV verbatim.
    """
    from iso15118.shared.exi_codec import EXI
    from iso15118.shared.messages.enums import Namespace
    from iso15118.shared.messages.iso15118_2.body import (
        Body, CertificateInstallationRes, CertificateUpdateRes)
    from iso15118.shared.messages.iso15118_2.msgdef import V2GMessage as V2GMessageV2
    from iso15118.shared.messages.iso15118_2.header import MessageHeader as MessageHeaderV2
    from iso15118.shared.messages.iso15118_2.datatypes import (
        CertificateChain as CC2, SubCertificates as SC2, EncryptedPrivateKey, DHPublicKey, EMAID,
        ResponseCode as RC2)
    from iso15118.shared.security import (
        load_cert, load_priv_key, encrypt_priv_key, get_cert_cn, create_signature,
        CertPath, KeyPath, KeyPasswordPath, KeyEncoding)

    setup_pki_symlinks()
    if session_id is None:
        req_msg = EXI().from_exi(base64.b64decode(exi_request_b64), Namespace.ISO_V2_MSG_DEF)
        session_id = req_msg.header.session_id

    dh_pub_key, enc_priv = encrypt_priv_key(
        oem_prov_cert=load_cert(CertPath.OEM_LEAF_DER),
        priv_key_to_encrypt=load_priv_key(
            KeyPath.CONTRACT_LEAF_PEM, KeyEncoding.PEM, KeyPasswordPath.CONTRACT_LEAF_KEY_PASSWORD))

    contract_chain = CC2(id="id1", certificate=load_cert(CertPath.CONTRACT_LEAF_DER),
                         sub_certificates=SC2(certificates=[load_cert(CertPath.MO_SUB_CA2_DER),
                                                            load_cert(CertPath.MO_SUB_CA1_DER)]))
    enc_key = EncryptedPrivateKey(id="id2", value=enc_priv)
    dh = DHPublicKey(id="id3", value=dh_pub_key)
    emaid = EMAID(id="id4", value=get_cert_cn(load_cert(CertPath.CONTRACT_LEAF_DER)))
    cps_chain = CC2(certificate=load_cert(CertPath.CPS_LEAF_DER),
                    sub_certificates=SC2(certificates=[load_cert(CertPath.CPS_SUB_CA2_DER),
                                                       load_cert(CertPath.CPS_SUB_CA1_DER)]))

    response_type = CertificateUpdateRes if update else CertificateInstallationRes
    res = response_type(
        response_code=RC2.OK, cps_cert_chain=cps_chain, contract_cert_chain=contract_chain,
        encrypted_private_key=enc_key, dh_public_key=dh, emaid=emaid,
        **({"retry_counter": 0} if update else {}))

    elements = [
        (contract_chain.id, EXI().to_exi(contract_chain, Namespace.ISO_V2_MSG_DEF)),
        (enc_key.id, EXI().to_exi(enc_key, Namespace.ISO_V2_MSG_DEF)),
        (dh.id, EXI().to_exi(dh, Namespace.ISO_V2_MSG_DEF)),
        (emaid.id, EXI().to_exi(emaid, Namespace.ISO_V2_MSG_DEF)),
    ]
    signature = create_signature(elements, load_priv_key(
        KeyPath.CPS_LEAF_PEM, KeyEncoding.PEM, KeyPasswordPath.CPS_LEAF_KEY_PASSWORD))

    header = MessageHeaderV2(session_id=session_id, signature=signature)
    response_name = "CertificateUpdateRes" if update else "CertificateInstallationRes"
    body = Body.parse_obj({response_name: res.dict()})
    exi = EXI().to_exi(V2GMessageV2(header=header, body=body), Namespace.ISO_V2_MSG_DEF)
    return base64.b64encode(exi).decode()


def run_iso2_malformed_responses(args, iface, csms, cin, check):
    from iso15118.shared.exi_codec import EXI
    from iso15118.shared.messages.enums import Namespace
    from iso15118.shared.messages.iso15118_2.body import Body

    def check_recovery(description):
        tls = open_cert_session2(args, iface)
        msg = wrap_v2g2(Body(CertificateInstallationReq=cin))
        send_exi(tls, EXI().to_exi(msg, Namespace.ISO_V2_MSG_DEF), 0x8001)
        ev_req, ev_msg = expect_ev_certificate(csms)
        csms.respond(ev_msg, {"status": "Accepted", "exiResponse": build_cert_res2(ev_req["exiRequest"])})
        res = recv_exi(tls, Namespace.ISO_V2_MSG_DEF)
        cert_res = res.body.certificate_installation_res
        check(f"ISO-2 recovers after {description} in the next session",
              cert_res is not None and cert_res.response_code == "OK",
              cert_res.response_code if cert_res else "none")
        tls.close()
        time.sleep(1)

    cases = (("invalid base64", "%%%"), ("empty", ""), ("malformed EXI", base64.b64encode(b"\x00").decode()))
    for description, exi_response in cases:
        tls = open_cert_session2(args, iface)
        msg = wrap_v2g2(Body(CertificateInstallationReq=cin))
        send_exi(tls, EXI().to_exi(msg, Namespace.ISO_V2_MSG_DEF), 0x8001)
        ev_req, ev_msg = expect_ev_certificate(csms)
        csms.respond(ev_msg, {"status": "Accepted", "exiResponse": exi_response})
        try:
            res = recv_exi_json(tls, Namespace.ISO_V2_MSG_DEF)
            cert_res = res["V2G_Message"]["Body"].get("CertificateInstallationRes")
            response_code = None if cert_res is None else cert_res.get("ResponseCode")
            rejected = response_code == "FAILED_NoCertificateAvailable"
            detail = response_code if cert_res is not None else "wrong response message"
        except Exception as e:
            rejected, detail = False, e
        check(f"ISO-2 rejects an accepted OCPP response with {description}", rejected, detail)
        tls.close()
        time.sleep(1)
        check_recovery(description)

    tls = open_cert_session2(args, iface)
    msg = wrap_v2g2(Body(CertificateInstallationReq=cin))
    send_exi(tls, EXI().to_exi(msg, Namespace.ISO_V2_MSG_DEF), 0x8001)
    ev_req, ev_msg = expect_ev_certificate(csms)
    csms.respond(ev_msg, {"status": "Accepted", "exiResponse": ev_req["exiRequest"]})
    try:
        res = recv_exi_json(tls, Namespace.ISO_V2_MSG_DEF)
        cert_res = res["V2G_Message"]["Body"].get("CertificateInstallationRes")
        response_code = None if cert_res is None else cert_res.get("ResponseCode")
        rejected = response_code == "FAILED_NoCertificateAvailable"
        detail = response_code if cert_res is not None else "wrong response message"
    except Exception as e:
        rejected, detail = False, e
    check("ISO-2 rejects an accepted OCPP response with the wrong EXI message", rejected, detail)
    tls.close()
    time.sleep(1)
    check_recovery("the wrong EXI message")


def setup_pki_symlinks():
    import os
    from iso15118.shared.settings import shared_settings, SettingKey
    pki = Path(tempfile.gettempdir()) / "pnc_ecog_pki" / "iso15118_2"
    (pki / "certs").mkdir(parents=True, exist_ok=True)
    (pki / "private_keys").mkdir(parents=True, exist_ok=True)
    src = CERTS / "iso2"
    for f in (src / "certs").iterdir():
        dst = pki / "certs" / f.name
        if dst.is_symlink() or dst.exists():
            dst.unlink()
        os.symlink(f.resolve(), dst)
    for f in (src / "private_keys").iterdir():
        dst = pki / "private_keys" / f.name
        if dst.is_symlink() or dst.exists():
            dst.unlink()
        os.symlink(f.resolve(), dst)
    shared_settings[SettingKey.PKI_PATH] = str(pki.parent)


if __name__ == "__main__":
    main()
