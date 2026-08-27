#!/usr/bin/env python3
"""M07 vehicle certificate chain OCSP gating

After a TLS 1.3 mutual auth handshake the charger requests the OCSP
status of the presented vehicle (OEM) certificate chain via OCPP
GetCertificateChainStatus and gates the ISO 15118-20 authorization on
the result:
  1. while the status is pending the AuthorizationRes carries EVSEProcessing Ongoing (HUB20-432-004)
  2. an all Good chain status lets the next AuthorizationRes finish with OK (HUB20-432-001/007)
  3. a Revoked certificate fails the authorization and closes the TLS session (HUB20-432-008)
  4. a missing status entry counts as Unknown, fails the authorization and closes the session (HUB20-432-009/010)
  5. security: a forged OEM chain with matching subject names but different keys is rejected by the handshake (the chain must anchor
     to the trust store by key, not by name)

Invoked by certificates.py through the firmware test runner.
"""

import argparse
import json
import os
import ssl
import struct
import sys
import tempfile
import shutil
import time
from pathlib import Path

import _common as common
from _common import CSMSSim as Csms
from _ocsp_gating import (CERTS, provision_chain, ocsp_response_b64, run)

SCRIPT_DIR = Path(__file__).parent.resolve()

ISO20_AC = {"ProtocolNamespace": "urn:iso:std:iso:15118:-20:AC", "VersionNumberMajor": 1,
            "VersionNumberMinor": 0, "SchemaID": 2, "Priority": 1}

_codec = None


def codec():
    global _codec
    if _codec is None:
        from iso15118.shared.settings import load_shared_settings
        from iso15118.shared.exificient_exi_codec import ExificientEXICodec
        load_shared_settings()
        _codec = ExificientEXICodec()
    return _codec


def connect_tls13(charger, iface, cert=None, key=None):
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    ctx.minimum_version = ssl.TLSVersion.TLSv1_3
    ctx.check_hostname = False
    ctx.verify_mode = ssl.CERT_REQUIRED
    ctx.load_verify_locations(cafile=str(CERTS / "iso20" / "certs" / "v2gRootCACert.pem"))
    ctx.load_cert_chain(certfile=str(cert or CERTS / "iso20" / "certs" / "oemCertChain.pem"),
                        keyfile=str(key or CERTS / "iso20" / "private_keys" / "oemLeaf.key"),
                        password="12345")
    raw = common.connect_secc(charger, iface)
    raw.settimeout(30)
    return ctx.wrap_socket(raw)


def mint_oem_leaf(workdir, tag):
    """A genuine OEM leaf chain with a fresh key and unique serial.

    Signed by the dev OEM sub CA 2, so it anchors to the trusted OEM root
    but its OCSP hash differs from every other leaf, which keeps the
    charger's vehicle chain status cache from short circuiting a scenario.
    """
    pki = CERTS / "iso20"
    key = workdir / f"oem_{tag}.key"
    run(["openssl", "ecparam", "-name", "secp521r1", "-genkey", "-noout", "-out", str(key)])
    csr = workdir / f"oem_{tag}.csr"
    run(["openssl", "req", "-new", "-key", str(key),
         "-subj", "/CN=OEMProvCert/O=WARP/C=DE/DC=OEM", "-out", str(csr)])
    ext = workdir / f"oem_{tag}.cnf"
    ext.write_text("[e]\nbasicConstraints=critical,CA:false\n"
                   "keyUsage=critical,digitalSignature,keyAgreement\nsubjectKeyIdentifier=hash\n")
    leaf = workdir / f"oem_{tag}.pem"
    run(["openssl", "x509", "-req", "-in", str(csr),
         "-CA", str(pki / "certs" / "oemSubCA2Cert.pem"),
         "-CAkey", str(pki / "private_keys" / "oemSubCA2.key"), "-passin", "pass:12345",
         "-set_serial", str(int.from_bytes(tag.encode() + os.urandom(8), "big")),
         "-days", "60", "-sha512", "-extfile", str(ext), "-extensions", "e", "-out", str(leaf)])
    chain = workdir / f"oem_{tag}_chain.pem"
    chain.write_text(leaf.read_text()
                     + (pki / "certs" / "oemSubCA2Cert.pem").read_text()
                     + (pki / "certs" / "oemSubCA1Cert.pem").read_text())
    return chain, key


def exchange(tls, wrapper, request, namespace, payload_type):
    exi = codec().encode(json.dumps({wrapper: request}), namespace)
    tls.sendall(struct.pack("!BBHI", 0x01, 0xFE, payload_type, len(exi)) + exi)
    resp = tls.recv(4096)
    assert resp[:2] == b"\x01\xfe", resp.hex()
    payload_len = struct.unpack("!I", resp[4:8])[0]
    return json.loads(codec().decode(resp[8:8 + payload_len], namespace))


def header(session_id="0000000000000000"):
    return {"SessionID": session_id, "TimeStamp": int(time.time())}


def sap_iso20(tls):
    from iso15118.shared.messages.enums import Namespace
    res = exchange(tls, "supportedAppProtocolReq", {"AppProtocol": [ISO20_AC]},
                   Namespace.SAP, 0x8001)["supportedAppProtocolRes"]
    assert res["ResponseCode"] == "OK_SuccessfulNegotiation" and res.get("SchemaID") == 2, res


def session_setup(tls):
    from iso15118.shared.messages.enums import Namespace
    res = exchange(tls, "SessionSetupReq", {"Header": header(), "EVCCID": "020000000001"},
                   Namespace.ISO_V20_COMMON_MSG, 0x8002)["SessionSetupRes"]
    return res["Header"]["SessionID"]


def authorization_setup(tls, session_id):
    from iso15118.shared.messages.enums import Namespace
    return exchange(tls, "AuthorizationSetupReq", {"Header": header(session_id)},
                    Namespace.ISO_V20_COMMON_MSG, 0x8002)["AuthorizationSetupRes"]


def authorization(tls, session_id):
    from iso15118.shared.messages.enums import Namespace
    return exchange(tls, "AuthorizationReq",
                    {"Header": header(session_id), "SelectedAuthorizationService": "EIM",
                     "EIM_AReqAuthorizationMode": {}},
                    Namespace.ISO_V20_COMMON_MSG, 0x8002)["AuthorizationRes"]


def open_iso20_session(charger, iface, cert=None, key=None):
    """TLS 1.3 mutual auth, SAP for -20, SessionSetup, AuthorizationSetup.

    Returns (tls socket, session id) positioned right before Authorization.
    """
    tls = connect_tls13(charger, iface, cert, key)
    sap_iso20(tls)
    session_id = session_setup(tls)
    setup = authorization_setup(tls, session_id)
    assert setup["ResponseCode"] == "OK", setup
    return tls, session_id


def chain_status_response(request, statuses):
    """Builds a GetCertificateChainStatusResponse, statuses per request entry.

    A None status omits the entry (HUB20-432-010 missing entry case).
    """
    entries = []
    for req, status in zip(request["certificateStatusRequests"], statuses):
        if status is None:
            continue
        entries.append({"certificateHashData": req["certificateHashData"],
                        "source": "OCSP", "status": status,
                        "nextUpdate": "2027-01-01T00:00:00Z"})
    return {"certificateStatus": entries}


def forge_oem_chain(workdir):
    """OEM leaf chain with the real subject names but fresh keys.

    Anchors by name to the trusted OEM root but not by key, which the
    handshake must reject.
    """
    pki = CERTS / "iso20" / "certs"
    subjects = run(["openssl", "x509", "-in", str(pki / "oemRootCACert.pem"),
                    "-noout", "-subject", "-nameopt", "RFC2253"]).stdout
    # Rebuild root, two sub CAs and a leaf, each with the original subject DN.
    def dn(cert):
        out = run(["openssl", "x509", "-in", str(cert), "-noout", "-subject",
                   "-nameopt", "RFC2253"]).stdout.strip()
        return out.split("subject=", 1)[1].strip()

    names = {
        "root": dn(pki / "oemRootCACert.pem"),
        "sub1": dn(pki / "oemSubCA1Cert.pem"),
        "sub2": dn(pki / "oemSubCA2Cert.pem"),
        "leaf": dn(pki / "oemLeafCert.pem"),
    }
    keys = {}
    for k in names:
        key = workdir / f"forge_{k}.key"
        run(["openssl", "ecparam", "-name", "secp521r1", "-genkey", "-noout", "-out", str(key)])
        keys[k] = key

    root_crt = workdir / "forge_root.pem"
    run(["openssl", "req", "-x509", "-new", "-key", str(keys["root"]), "-sha512",
         "-subj", "/" + names["root"].replace(",", "/"), "-days", "60", "-out", str(root_crt)])

    def sign(name, issuer_key, issuer_crt, ca):
        csr = workdir / f"forge_{name}.csr"
        run(["openssl", "req", "-new", "-key", str(keys[name]),
             "-subj", "/" + names[name].replace(",", "/"), "-out", str(csr)])
        ext = workdir / f"forge_{name}.cnf"
        ext.write_text("[e]\nbasicConstraints=critical,CA:" + ("true" if ca else "false") + "\n"
                       "keyUsage=critical," + ("keyCertSign,cRLSign" if ca else "digitalSignature,keyAgreement") + "\n")
        crt = workdir / f"forge_{name}.pem"
        run(["openssl", "x509", "-req", "-in", str(csr), "-CA", str(issuer_crt),
             "-CAkey", str(issuer_key), "-CAcreateserial", "-days", "60", "-sha512",
             "-extfile", str(ext), "-extensions", "e", "-out", str(crt)])
        return crt

    sub1 = sign("sub1", keys["root"], root_crt, True)
    sub2 = sign("sub2", keys["sub1"], sub1, True)
    leaf = sign("leaf", keys["sub2"], sub2, False)
    chain = workdir / "forge_chain.pem"
    chain.write_text(leaf.read_text() + sub2.read_text() + sub1.read_text())
    return chain, keys["leaf"]


def try_forged_handshake(charger, iface, chain_pem, key_pem):
    """Returns True when the charger accepts the forged client chain.

    In TLS 1.3 the client cert is validated after the client considers the
    handshake done, so wrap_socket succeeds and the reject arrives as a
    later alert. A follow up read surfaces it.
    """
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    ctx.minimum_version = ssl.TLSVersion.TLSv1_3
    ctx.check_hostname = False
    ctx.verify_mode = ssl.CERT_REQUIRED
    ctx.load_verify_locations(cafile=str(CERTS / "iso20" / "certs" / "v2gRootCACert.pem"))
    ctx.load_cert_chain(certfile=str(chain_pem), keyfile=str(key_pem))
    try:
        raw = common.connect_secc(charger, iface)
    except Exception:
        return False
    try:
        raw.settimeout(15)
        tls = ctx.wrap_socket(raw)
        # A SAP request forces the server to act on the connection. On a
        # rejected client cert the read raises or returns empty.
        from iso15118.shared.messages.enums import Namespace
        exi = codec().encode(json.dumps({"supportedAppProtocolReq": {"AppProtocol": [ISO20_AC]}}),
                             Namespace.SAP)
        tls.sendall(struct.pack("!BBHI", 0x01, 0xFE, 0x8001, len(exi)) + exi)
        accepted = len(tls.recv(16)) > 0
        tls.close()
        return accepted
    except (ssl.SSLError, OSError):
        try:
            raw.close()
        except OSError:
            pass
        return False
    finally:
        time.sleep(1)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--charger", required=True)
    p.add_argument("--iface")
    p.add_argument("--port", type=int, default=9500)
    args = p.parse_args()

    common.require_iso_tls_config(args.charger)
    iface = args.iface or common.default_iface(args.charger)
    local_ip = common.local_ip_towards(args.charger)
    workdir = Path(tempfile.mkdtemp(prefix="vehicle_chain_"))

    failures = 0

    def check(name, ok, detail=""):
        nonlocal failures
        print(f'{"ok  " if ok else "FAIL"} {name}{": " + str(detail) if detail else ""}')
        failures += 0 if ok else 1

    saved_config = common.api_get(args.charger, "ocpp/config")
    csms = Csms(args.port, interactive=("SignCertificate", "GetCertificateStatus",
                                        "GetCertificateChainStatus"))
    try:
        common.api_put(args.charger, "ocpp/reset", None)
        test_config = dict(saved_config)
        test_config.update({"enable": True, "protocol": 1, "url": f"ws://{local_ip}:{args.port}",
                            "enable_auth": False})
        common.api_put(args.charger, "ocpp/config_update", test_config)
        if not csms.connected.wait(timeout=60):
            raise SystemExit("charger did not connect to the embedded CSMS")
        time.sleep(2)
        common.enable_debug_mode(args.charger)
        time.sleep(2)

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

        # Make TLS 1.3 ready: answer the SECC leaf OCSP with Good.
        status_req, msg_id = csms.expect("GetCertificateStatus", timeout=120)
        b64, _ = ocsp_response_b64(workdir, workdir / "leaf20.pem")
        csms.respond(msg_id, {"status": "Accepted", "ocspResult": b64})
        time.sleep(5)

        # 1 and 2: pending gives Ongoing, all Good then finishes.
        cert, key = mint_oem_leaf(workdir, "good")
        tls, session_id = open_iso20_session(args.charger, iface, cert, key)
        chain_req, chain_msg = csms.expect("GetCertificateChainStatus", timeout=30)
        serials = [r["certificateHashData"]["serialNumber"] for r in chain_req["certificateStatusRequests"]]
        check("GetCertificateChainStatus for the full vehicle chain leaf first",
              len(serials) == 3, f"{len(serials)} certificates")
        auth = authorization(tls, session_id)
        check("authorization Ongoing while the OCSP status is pending [HUB20-432-004]",
              auth["ResponseCode"] == "OK" and auth["EVSEProcessing"] == "Ongoing", auth)

        csms.respond(chain_msg, chain_status_response(chain_req, ["Good", "Good", "Good"]))
        time.sleep(3)
        auth = authorization(tls, session_id)
        check("authorization Finished OK after all Good [HUB20-432-001/007]",
              auth["ResponseCode"] == "OK" and auth["EVSEProcessing"] == "Finished", auth)
        tls.close()
        time.sleep(2)

        # 3: Revoked fails and closes the session.
        cert, key = mint_oem_leaf(workdir, "revoked")
        tls, session_id = open_iso20_session(args.charger, iface, cert, key)
        chain_req, chain_msg = csms.expect("GetCertificateChainStatus", timeout=30)
        csms.respond(chain_msg, chain_status_response(chain_req, ["Good", "Good", "Revoked"]))
        time.sleep(3)
        auth = authorization(tls, session_id)
        check("authorization failed on a revoked certificate [HUB20-432-008]",
              auth["ResponseCode"].startswith("FAILED"), auth)
        closed = False
        try:
            tls.settimeout(10)
            closed = tls.recv(16) == b""
        except (ssl.SSLError, OSError):
            closed = True
        check("TLS session closed after revocation [HUB20-432-008]", closed)
        try:
            tls.close()
        except OSError:
            pass
        time.sleep(2)

        # 4: a missing status entry counts as Unknown, same fail closed
        # path. The leaf carries a fresh serial, so no cached status can
        # stand in for the omitted response entry (HUB20-432-010).
        cert, key = mint_oem_leaf(workdir, "missing")
        tls, session_id = open_iso20_session(args.charger, iface, cert, key)
        chain_req, chain_msg = csms.expect("GetCertificateChainStatus", timeout=30)
        csms.respond(chain_msg, chain_status_response(chain_req, [None, "Good", "Good"]))
        time.sleep(3)
        auth = authorization(tls, session_id)
        check("authorization failed on a missing status entry [HUB20-432-009/010]",
              auth["ResponseCode"].startswith("FAILED"), auth)
        try:
            tls.close()
        except OSError:
            pass
        time.sleep(2)

        # 5: forged OEM chain, matching names, different keys.
        forged_chain, forged_key = forge_oem_chain(workdir)
        accepted = try_forged_handshake(args.charger, iface, forged_chain, forged_key)
        check("forged vehicle chain rejected by the handshake", not accepted,
              "charger accepted the forged chain" if accepted else "")
        # The genuine chain still works after the rejection.
        cert, key = mint_oem_leaf(workdir, "after")
        tls, session_id = open_iso20_session(args.charger, iface, cert, key)
        chain_req, chain_msg = csms.expect("GetCertificateChainStatus", timeout=30)
        csms.respond(chain_msg, chain_status_response(chain_req, ["Good", "Good", "Good"]))
        time.sleep(3)
        auth = authorization(tls, session_id)
        check("genuine chain still authorizes after the forged rejection",
              auth["ResponseCode"] == "OK" and auth["EVSEProcessing"] == "Finished", auth)
        tls.close()
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
            failures += 1
        csms.stop()
        shutil.rmtree(workdir, ignore_errors=True)

    print("PASS" if failures == 0 else f"FAIL ({failures} failures)")
    sys.exit(0 if failures == 0 else 1)


if __name__ == "__main__":
    main()
