#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""HUB20-532-002 / V2G20-2399: mixed-version device observations."""

import os
import socket
import ssl
import subprocess
import time

from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec

import vehicle_validation as validation
from vehicle_validation import TestContext, common, vehicle
from software.test_runner.test_context import run_testsuite

suite_setup = validation.suite_setup
suite_teardown = validation.suite_teardown
setup = validation.setup
teardown = validation.teardown


def vector(data):
    return len(data).to_bytes(2, "big") + data


def extension(kind, data):
    return kind.to_bytes(2, "big") + vector(data)


def endpoint(tc, env):
    response = common.sdp_request(env.iface, expected_from=env.target_ll)
    tc.assert_(response is not None and response["security"] == common.SDP_SECURITY_TLS)
    return response["secc_ll"], response["port"], 0, socket.if_nametoindex(env.iface)


def hello(env, ciphers):
    share = ec.generate_private_key(ec.SECP521R1()).public_key().public_bytes(
        serialization.Encoding.X962, serialization.PublicFormat.UncompressedPoint)
    # Only the ISO-2 root is advertised, using DN identifiers in both formats.
    dn = env.pki["iso2"][0].subject.public_bytes()
    extensions = b"".join([
        extension(43, b"\x04\x03\x04\x03\x03"),
        extension(10, vector(b"\x00\x19\x00\x17")),
        extension(11, b"\x01\x00"),
        extension(13, vector(b"\x06\x03\x08\x08\x04\x03")),
        extension(51, vector(b"\x00\x19" + vector(share))),
        extension(45, b"\x01\x01"),
        extension(5, b"\x01\x00\x00\x00\x00"),
        extension(17, vector(b"\x02" + vector(b"\x00\x00\x00\x00"))),
        extension(3, vector(b"\x02" + vector(dn))),
        extension(47, vector(vector(dn))),
    ])
    body = b"\x03\x03" + os.urandom(32) + b"\x00" + vector(ciphers) + b"\x01\x00" + vector(extensions)
    message = b"\x01" + len(body).to_bytes(3, "big") + body
    return b"\x16\x03\x01" + vector(message)


def wire(tc, env, label, ciphers):
    def exact(sock, size):
        data = b""
        while len(data) < size:
            chunk = sock.recv(size - len(data))
            tc.assert_(bool(chunk))
            data += chunk
        return data

    request = hello(env, ciphers)
    with socket.socket(socket.AF_INET6, socket.SOCK_STREAM) as sock:
        sock.settimeout(30)
        sock.connect(endpoint(tc, env))
        sock.sendall(request)
        header = exact(sock, 5)
        body = exact(sock, int.from_bytes(header[3:], "big"))
    env.write_json(f"mixed-{label}.json", {"client_record": request.hex(), "server_record": (header + body).hex()})
    time.sleep(2)
    if header[0] == 21:
        tc.assert_eq(2, len(body))
        return {"alert_level": body[0], "alert": body[1]}
    tc.assert_eq(22, header[0])
    tc.assert_eq(2, body[0])
    tc.assert_(int.from_bytes(body[1:4], "big") + 4 <= len(body))
    data = body[4:4 + int.from_bytes(body[1:4], "big")]
    pos = 35 + data[34]
    cipher = int.from_bytes(data[pos:pos + 2], "big")
    tc.assert_eq(0, data[pos + 2])
    pos += 3
    version = "TLSv1.2"
    if pos < len(data):
        length = int.from_bytes(data[pos:pos + 2], "big")
        pos += 2
        tc.assert_eq(len(data), pos + length)
        while pos < len(data):
            kind = int.from_bytes(data[pos:pos + 2], "big")
            length = int.from_bytes(data[pos + 2:pos + 4], "big")
            if kind == 43:
                tc.assert_eq(b"\x03\x04", data[pos + 4:pos + 4 + length])
                version = "TLSv1.3"
            pos += 4 + length
    return {"version": version, "cipher": cipher, "random_suffix": data[26:34].hex()}


def test_mixed_null_and_iso2_cipher(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    actual = wire(tc, env, "null-iso2", bytes.fromhex("000000010002c023"))
    tc.assert_eq({"alert_level": 2, "alert": 40}, actual)
    env.record("mixed NULL/ISO2 offer rejects with handshake_failure on dual-version service", actual)


def test_iso2_only_trust_indication(tc: TestContext):
    tc.set_test_timeout(180)
    env = validation.environment
    actual = wire(tc, env, "iso2-trust", bytes.fromhex("13021303c023"))
    tc.assert_eq("TLSv1.3", actual["version"])
    tc.assert_eq(0x1302, actual["cipher"])
    # OpenSSL sends certificate_authorities and validates the actual returned
    # chain with ISO-2-only trust. Its CLI cannot send trusted_ca_keys; the raw
    # observation above includes both trust extensions and status_request_v2.
    address, port, _, _ = endpoint(tc, env)
    command = ["openssl", "s_client", "-connect", f"[{address}%{env.iface}]:{port}",
               "-min_protocol", "TLSv1.2", "-max_protocol", "TLSv1.3",
               "-groups", "P-521:P-256", "-cipher", "ECDHE-ECDSA-AES128-SHA256",
               "-ciphersuites", "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256",
               "-requestCAfile", str(env.work / "iso2-root.pem"),
               "-CAfile", str(env.work / "iso2-root.pem"), "-no-CApath", "-no-CAstore",
               "-verify_return_error", "-showcerts", "-status", "-no_ign_eof"]
    result = subprocess.run(command, input="", capture_output=True, text=True, timeout=60)
    output = result.stdout + result.stderr
    (env.work / "mixed-iso2-only-trust.log").write_text(output)
    tc.assert_(result.returncode != 0)
    tc.assert_("certificate verify failed" in output)
    leaf = (env.work / "iso20-secc.pem").read_text().strip()
    tc.assert_(leaf in output)
    env.record("ISO2-only trust indication retains alternative ISO20 chain; EV rejects untrusted chain", actual)
    time.sleep(2)


def test_missing_iso20_identity_caps_service_to_tls12(tc: TestContext):
    tc.set_test_timeout(180)
    env = validation.environment
    # This is the last case: only the suite's temporary ISO20 identity is removed.
    from cryptography import x509
    leaf = x509.load_pem_x509_certificate((env.work / "iso20-secc.pem").read_bytes())
    entry = next(e for e in env.inventory() if int(e["certificateHashData"]["serialNumber"], 16) == leaf.serial_number)
    tc.assert_eq("Accepted", env.csms.call("DeleteCertificate", {"certificateHashData": entry["certificateHashData"]})["status"])
    time.sleep(3)
    actual = wire(tc, env, "capped-null-iso2", bytes.fromhex("000000010002c023"))
    tc.assert_eq("TLSv1.2", actual["version"])
    tc.assert_eq(0xC023, actual["cipher"])
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    context.minimum_version = ssl.TLSVersion.TLSv1_2
    context.maximum_version = ssl.TLSVersion.TLSv1_3
    context.check_hostname = False
    context.load_verify_locations(cadata=env.pki["iso2"][1].decode())
    context.set_ciphers("ECDHE-ECDSA-AES128-SHA256")
    with socket.socket(socket.AF_INET6, socket.SOCK_STREAM) as raw:
        raw.settimeout(45)
        raw.connect(endpoint(tc, env))
        with context.wrap_socket(raw) as tls:
            tc.assert_eq("TLSv1.2", tls.version())
            tc.assert_eq("ECDHE-ECDSA-AES128-SHA256", tls.cipher()[0])
            expected = x509.load_pem_x509_certificate((env.work / "iso2-secc.pem").read_bytes())
            tc.assert_eq(expected.public_bytes(serialization.Encoding.DER), tls.getpeercert(binary_form=True))
            from iso15118.shared.messages.enums import Namespace
            result = vehicle.exchange(tls, "supportedAppProtocolReq", {"AppProtocol": [common.ISO2]}, Namespace.SAP, 0x8001)
            tc.assert_eq("OK_SuccessfulNegotiation", result["supportedAppProtocolRes"]["ResponseCode"])
    env.record("missing ISO20 identity allows authenticated mixed-client TLS12 and ISO2 SAP", actual)


if __name__ == "__main__":
    run_testsuite(locals())
