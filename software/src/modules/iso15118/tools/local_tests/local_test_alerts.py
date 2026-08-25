#!/usr/bin/env python3
"""Alert byte checks for the TLS group and sig alg refusals.

Captures ClientHellos from the local openssl s_client via a loopback listener,
replays them raw to the SECC and checks that the refusal is a fatal
handshake_failure alert on the wire (HUB20-533-002/004).

Usage: ./local_test_alerts.py --charger <ip>
"""

import argparse
import socket
import subprocess
import sys
import time

import common

ALERT_NAMES = {
    40: "handshake_failure",
    47: "illegal_parameter",
    70: "protocol_version",
    71: "insufficient_security",
    80: "internal_error",
    109: "missing_extension",
}


def capture_hello(args):
    """Records the ClientHello that openssl s_client with the given arguments sends."""
    server = socket.socket()
    server.bind(("127.0.0.1", 0))
    server.listen(1)
    port = server.getsockname()[1]
    proc = subprocess.Popen(
        ["openssl", "s_client", "-connect", f"127.0.0.1:{port}"] + args,
        stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    conn, _ = server.accept()
    conn.settimeout(2)
    data = bytearray()
    try:
        while len(data) < 5 or len(data) < 5 + int.from_bytes(data[3:5], "big"):
            chunk = conn.recv(4096)
            if not chunk:
                break
            data += chunk
    except socket.timeout:
        pass
    conn.close()
    server.close()
    proc.kill()
    proc.wait()
    return bytes(data)


def replay(charger, iface, name, hello):
    sock = common.connect_secc(charger, iface)
    sock.sendall(hello)
    try:
        resp = sock.recv(4096)
    except socket.timeout:
        resp = b""
    sock.close()
    time.sleep(0.5)

    if len(resp) >= 7 and resp[0] == 0x15:
        level, desc = resp[5], resp[6]
        ok = level == 2 and desc == 40
        print(f'{"ok  " if ok else "FAIL"} {name}: alert level={level} desc={desc} ({ALERT_NAMES.get(desc, "?")})')
        return ok
    if len(resp) == 0:
        print(f"FAIL {name}: connection closed without alert")
    else:
        print(f"FAIL {name}: response record type {resp[0]:#04x} len {len(resp)}")
    return False


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--charger", required=True)
    p.add_argument("--iface")
    args = p.parse_args()

    common.require_iso_tls_config(args.charger)
    common.enable_debug_mode(args.charger)
    time.sleep(2)
    iface = args.iface or common.default_iface(args.charger)

    cases = [
        ("tls13 P-256 only key_share and groups", ["-tls1_3", "-groups", "P-256"]),
        ("tls13 X25519 only", ["-tls1_3", "-groups", "X25519"]),
        ("tls12 ECDSA+SHA1 only sig alg", ["-tls1_2", "-cipher", "ECDHE-ECDSA-AES128-SHA256:@SECLEVEL=0",
                                           "-curves", "P-256", "-sigalgs", "ECDSA+SHA1"]),
        ("tls12 X25519:X448 only", ["-tls1_2", "-cipher", "ECDHE-ECDSA-AES128-SHA256", "-curves", "X25519:X448"]),
        ("tls12 disjoint cipher", ["-tls1_2", "-cipher", "ECDHE-RSA-AES256-GCM-SHA384"]),
    ]

    failures = 0
    for name, ossl_args in cases:
        hello = capture_hello(ossl_args)
        if len(hello) < 5:
            print(f"FAIL {name}: could not capture a ClientHello")
            failures += 1
            continue
        if not replay(args.charger, iface, name, hello):
            failures += 1

    print("PASS" if failures == 0 else f"FAIL ({failures} failures)")
    sys.exit(0 if failures == 0 else 1)


if __name__ == "__main__":
    main()
