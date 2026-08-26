#!/usr/bin/env python3
"""TLS 1.3 session resumption via psk_dhe_ke session tickets.

Checks [V2G20-1675 ff] and [V2G20-2677]:
  1. a full TLS 1.3 mutual auth handshake yields a NewSessionTicket with a ticket_lifetime within 20 s and 86400 s [V2G20-2024..2026]
  2. the ticket resumes a TLS 1.3 session (PSK handshake, no certificates)
  3. SAP over the resumed session answers Failed_NoNegotiation, only full-handshake TLS carries V2G communication [V2G20-2677]
  4. a fresh full handshake still negotiates ISO 15118-20 afterwards
  5. TLS 1.2 issues no usable ticket and never resumes, ISO 15118-2 sessions stay at full handshakes

Uses the dev PKI.

Run this in evsim venv:
../../../../../.venv-evsim/bin/python local_test_session_resumption.py --charger <ip>
"""

import argparse
import json
import ssl
import struct
import sys
import time
from pathlib import Path

import common

SCRIPT_DIR = Path(__file__).parent.resolve()
sys.path.insert(0, str(SCRIPT_DIR / ".." / "evsim" / "iso15118"))

CERTS = SCRIPT_DIR / ".." / "certs" / "output"

ISO2 = {"ProtocolNamespace": "urn:iso:15118:2:2013:MsgDef", "VersionNumberMajor": 2,
        "VersionNumberMinor": 0, "SchemaID": 1, "Priority": 2}
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


def make_ctx13():
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    ctx.minimum_version = ssl.TLSVersion.TLSv1_3
    ctx.check_hostname = False
    ctx.verify_mode = ssl.CERT_REQUIRED
    ctx.load_verify_locations(cafile=str(CERTS / "iso20" / "certs" / "v2gRootCACert.pem"))
    ctx.load_cert_chain(certfile=str(CERTS / "iso20" / "certs" / "oemCertChain.pem"),
                        keyfile=str(CERTS / "iso20" / "private_keys" / "oemLeaf.key"),
                        password="12345")
    return ctx


def make_ctx12():
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    ctx.maximum_version = ssl.TLSVersion.TLSv1_2
    ctx.set_ciphers("ECDHE-ECDSA-AES128-SHA256")
    ctx.check_hostname = False
    ctx.verify_mode = ssl.CERT_REQUIRED
    ctx.load_verify_locations(cafile=str(CERTS / "iso2" / "certs" / "v2gRootCACert.pem"))
    return ctx


def connect(ctx, charger, iface, session=None):
    raw = common.connect_secc(charger, iface)
    raw.settimeout(30)
    return ctx.wrap_socket(raw, session=session)


def sap(tls, app_protocols):
    from iso15118.shared.messages.enums import Namespace
    exi = codec().encode(json.dumps({"supportedAppProtocolReq": {"AppProtocol": app_protocols}}), Namespace.SAP)
    tls.sendall(struct.pack("!BBHI", 0x01, 0xFE, 0x8001, len(exi)) + exi)
    resp = tls.recv(4096)
    assert resp[:2] == b"\x01\xfe", resp.hex()
    payload_len = struct.unpack("!I", resp[4:8])[0]
    return json.loads(codec().decode(resp[8:8 + payload_len], Namespace.SAP))["supportedAppProtocolRes"]


def ok(msg, detail=""):
    print(f"ok   {msg}" + (f": {detail}" if detail else ""))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--charger", required=True)
    parser.add_argument("--iface")
    args = parser.parse_args()

    common.enable_debug_mode(args.charger)
    common.require_iso_tls_config(args.charger)
    iface = args.iface or common.default_iface(args.charger)
    time.sleep(2)

    ctx13 = make_ctx13()

    # 1: full handshake, V2G works, ticket arrives
    tls = connect(ctx13, args.charger, iface)
    assert not tls.session_reused
    res = sap(tls, [ISO20_AC])
    assert res["ResponseCode"] == "OK_SuccessfulNegotiation" and res.get("SchemaID") == 2, res
    ok("Full TLS 1.3 handshake negotiates ISO 15118-20", res["ResponseCode"])
    sess = tls.session
    tls.close()
    time.sleep(1)
    assert sess is not None and sess.has_ticket, "no session ticket received"
    assert 20 <= sess.ticket_lifetime_hint <= 86400, sess.ticket_lifetime_hint
    ok("NewSessionTicket received with a valid ticket_lifetime [V2G20-2024..2026]",
       f"{sess.ticket_lifetime_hint}s")

    # 2 + 3: ticket resumes the session, SAP is refused on it
    tls = connect(ctx13, args.charger, iface, session=sess)
    assert tls.session_reused, "session not resumed from the ticket"
    assert tls.version() == "TLSv1.3", tls.version()
    ok("TLS 1.3 session resumed via ticket PSK [V2G20-1675 ff]", tls.cipher()[0])
    res = sap(tls, [ISO20_AC, ISO2])
    assert res["ResponseCode"] == "Failed_NoNegotiation", res
    ok("SAP over the resumed session answers Failed_NoNegotiation [V2G20-2677]", res["ResponseCode"])
    tls.close()
    time.sleep(1)

    # 4: a fresh full handshake still negotiates -20
    tls = connect(ctx13, args.charger, iface)
    assert not tls.session_reused
    res = sap(tls, [ISO20_AC])
    assert res["ResponseCode"] == "OK_SuccessfulNegotiation" and res.get("SchemaID") == 2, res
    ok("Fresh full handshake negotiates ISO 15118-20 again", res["ResponseCode"])
    tls.close()
    time.sleep(1)

    # 5: TLS 1.2 issues no usable ticket and never resumes
    ctx12 = make_ctx12()
    tls = connect(ctx12, args.charger, iface)
    assert tls.version() == "TLSv1.2", tls.version()
    res = sap(tls, [ISO2])
    assert res["ResponseCode"] == "OK_SuccessfulNegotiation" and res.get("SchemaID") == 1, res
    sess12 = tls.session
    tls.close()
    time.sleep(1)
    assert sess12 is None or not sess12.has_ticket, "TLS 1.2 got a session ticket"
    ok("TLS 1.2 session gets no usable ticket, ISO 15118-2 unchanged")
    if sess12 is not None:
        tls = connect(ctx12, args.charger, iface, session=sess12)
        assert not tls.session_reused, "TLS 1.2 session was resumed"
        res = sap(tls, [ISO2])
        assert res["ResponseCode"] == "OK_SuccessfulNegotiation", res
        ok("TLS 1.2 reconnect does a full handshake and still negotiates ISO 15118-2")
        tls.close()

    print("PASS")


if __name__ == "__main__":
    main()
