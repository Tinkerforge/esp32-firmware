#!/usr/bin/env python3
"""supportedAppProtocol negotiation over a real TLS 1.2 handshake.

Checks [V2G20-2356] and the SAP handling:
  1. only -20 offered after TLS 1.2 answers Failed_NoNegotiation
  2. -2 plus -20 offered after TLS 1.2 selects -2
  3. -20 with VersionNumberMajor 2 is ignored, -2 wins
  4. -2 SessionSetup returns the EVSEID (printed, checked with --expect-evseid)

Run this in evsim venv:
  ../../../../../.venv-evsim/bin/python local_test_sap_tls12.py --charger <ip>
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

V2G_ROOT_ISO2 = str(SCRIPT_DIR / ".." / "certs" / "output" / "iso2" / "certs" / "v2gRootCACert.pem")

ISO2 = {"ProtocolNamespace": "urn:iso:15118:2:2013:MsgDef", "VersionNumberMajor": 2,
        "VersionNumberMinor": 0, "SchemaID": 1, "Priority": 1}
ISO20 = {"ProtocolNamespace": "urn:iso:std:iso:15118:-20:AC", "VersionNumberMajor": 1,
         "VersionNumberMinor": 0, "SchemaID": 2, "Priority": 2}
ISO20_MAJOR2 = {"ProtocolNamespace": "urn:iso:std:iso:15118:-20:AC", "VersionNumberMajor": 2,
                "VersionNumberMinor": 0, "SchemaID": 3, "Priority": 1}

_codec = None


def codec():
    global _codec
    if _codec is None:
        from iso15118.shared.settings import load_shared_settings
        from iso15118.shared.exificient_exi_codec import ExificientEXICodec
        load_shared_settings()
        _codec = ExificientEXICodec()
    return _codec


def connect_tls12(charger, iface):
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    ctx.maximum_version = ssl.TLSVersion.TLSv1_2
    ctx.set_ciphers("ECDHE-ECDSA-AES128-SHA256")
    ctx.load_verify_locations(cafile=V2G_ROOT_ISO2)
    ctx.check_hostname = False
    ctx.verify_mode = ssl.CERT_REQUIRED
    raw = common.connect_secc(charger, iface)
    return ctx.wrap_socket(raw)


def exchange(tls, request, namespace):
    from iso15118.shared.messages.enums import Namespace
    exi = codec().encode(json.dumps(request), namespace)
    tls.sendall(struct.pack("!BBHI", 0x01, 0xFE, 0x8001, len(exi)) + exi)
    resp = tls.recv(4096)
    assert resp[:2] == b"\x01\xfe", resp.hex()
    payload_len = struct.unpack("!I", resp[4:8])[0]
    return json.loads(codec().decode(resp[8:8 + payload_len], namespace))


def sap(charger, iface, app_protocols):
    from iso15118.shared.messages.enums import Namespace
    tls = connect_tls12(charger, iface)
    try:
        res = exchange(tls, {"supportedAppProtocolReq": {"AppProtocol": app_protocols}}, Namespace.SAP)
    finally:
        tls.close()
    time.sleep(1)
    return res["supportedAppProtocolRes"]


def session_setup_evseid(charger, iface=None):
    """SAP negotiation of -2 plus SessionSetup, returns the announced EVSEID."""
    from iso15118.shared.messages.enums import Namespace
    iface = iface or common.default_iface(charger)
    tls = connect_tls12(charger, iface)
    try:
        res = exchange(tls, {"supportedAppProtocolReq": {"AppProtocol": [ISO2]}}, Namespace.SAP)
        assert res["supportedAppProtocolRes"]["ResponseCode"] == "OK_SuccessfulNegotiation"
        res = exchange(tls, {"V2G_Message": {
            "Header": {"SessionID": "0000000000000000"},
            "Body": {"SessionSetupReq": {"EVCCID": "020000000001"}},
        }}, Namespace.ISO_V2_MSG_DEF)
    finally:
        tls.close()
    time.sleep(1)
    return res["V2G_Message"]["Body"]["SessionSetupRes"]["EVSEID"]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--charger", required=True)
    p.add_argument("--iface")
    p.add_argument("--expect-evseid", help="fail unless SessionSetupRes carries this EVSEID")
    args = p.parse_args()

    common.require_iso_tls_config(args.charger)
    common.enable_debug_mode(args.charger)
    time.sleep(2)
    iface = args.iface or common.default_iface(args.charger)

    failures = 0

    res = sap(args.charger, iface, [ISO20])
    ok = res["ResponseCode"] == "Failed_NoNegotiation"
    print(f'{"ok  " if ok else "FAIL"} only -20 after TLS 1.2: {res}')
    failures += 0 if ok else 1

    res = sap(args.charger, iface, [ISO2, ISO20])
    ok = res["ResponseCode"] == "OK_SuccessfulNegotiation" and res.get("SchemaID") == 1
    print(f'{"ok  " if ok else "FAIL"} -2 plus -20 after TLS 1.2 selects -2: {res}')
    failures += 0 if ok else 1

    res = sap(args.charger, iface, [ISO20_MAJOR2, ISO2])
    ok = res["ResponseCode"] == "OK_SuccessfulNegotiation" and res.get("SchemaID") == 1
    print(f'{"ok  " if ok else "FAIL"} -20 major 2 ignored, -2 selected: {res}')
    failures += 0 if ok else 1

    evseid = session_setup_evseid(args.charger, iface)
    if args.expect_evseid is not None:
        ok = evseid == args.expect_evseid
        print(f'{"ok  " if ok else "FAIL"} SessionSetupRes EVSEID: {evseid}')
        failures += 0 if ok else 1
    else:
        print(f"info SessionSetupRes EVSEID: {evseid}")

    print("PASS" if failures == 0 else f"FAIL ({failures} failures)")
    sys.exit(0 if failures == 0 else 1)


if __name__ == "__main__":
    main()
