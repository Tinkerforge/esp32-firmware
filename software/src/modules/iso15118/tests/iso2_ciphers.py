#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""V2G2-602: force both ISO-2 suites on the device, including static ECDH.

Python/OpenSSL no longer supports static ECDH suites. The small TLS 1.2
client below implements only the CBC/SHA256 record mode needed for this test,
verifies the exact provisioned server certificate and both Finished messages,
and carries an ISO-2 SAP exchange. It is test code, not a general TLS client.
"""

import hashlib
import hmac
import os
import socket
import struct
import time

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes

import vehicle_validation as validation
from vehicle_validation import TestContext, common, vehicle
from software.test_runner.test_context import run_testsuite

suite_setup = validation.suite_setup
suite_teardown = validation.suite_teardown
setup = validation.setup
teardown = validation.teardown


def vector(data):
    return len(data).to_bytes(2, "big") + data


def handshake(kind, data):
    return bytes([kind]) + len(data).to_bytes(3, "big") + data


def prf(secret, label, seed, length):
    seed = label + seed
    a = seed
    result = b""
    while len(result) < length:
        a = hmac.digest(secret, a, "sha256")
        result += hmac.digest(secret, a + seed, "sha256")
    return result[:length]


class TLS12Client:
    def __init__(self, sock):
        self.sock = sock
        self.out_seq = self.in_seq = 0
        self.transcript = b""

    def exact(self, size):
        data = b""
        while len(data) < size:
            chunk = self.sock.recv(size - len(data))
            if not chunk:
                raise EOFError("TLS peer closed")
            data += chunk
        return data

    def record(self):
        header = self.exact(5)
        assert header[1:3] == b"\x03\x03", header.hex()
        length = int.from_bytes(header[3:], "big")
        assert length <= 18432
        return header[0], self.exact(length)

    def send_record(self, kind, data):
        self.sock.sendall(bytes([kind]) + b"\x03\x03" + vector(data))

    def encrypt(self, kind, data):
        header = struct.pack("!Q", self.out_seq) + bytes([kind]) + b"\x03\x03" + vector(data)
        plain = data + hmac.digest(self.client_mac, header, "sha256")
        pad = 15 - len(plain) % 16
        plain += bytes([pad]) * (pad + 1)
        iv = os.urandom(16)
        cipher = Cipher(algorithms.AES(self.client_key), modes.CBC(iv)).encryptor()
        self.send_record(kind, iv + cipher.update(plain) + cipher.finalize())
        self.out_seq += 1

    def decrypt(self):
        kind, data = self.record()
        assert len(data) >= 32 and len(data) % 16 == 0
        cipher = Cipher(algorithms.AES(self.server_key), modes.CBC(data[:16])).decryptor()
        plain = cipher.update(data[16:]) + cipher.finalize()
        pad = plain[-1] + 1
        assert plain[-pad:] == bytes([pad - 1]) * pad
        plain = plain[:-pad]
        payload, mac = plain[:-32], plain[-32:]
        header = struct.pack("!Q", self.in_seq) + bytes([kind]) + b"\x03\x03" + vector(payload)
        assert hmac.compare_digest(mac, hmac.digest(self.server_mac, header, "sha256"))
        self.in_seq += 1
        return kind, payload

    def connect(self, suite, expected_leaf, root, offered=None):
        client_random = os.urandom(32)
        extensions = b"".join(kind.to_bytes(2, "big") + vector(data) for kind, data in [
            (10, vector(b"\x00\x17")), (11, b"\x01\x00"),
            (13, vector(b"\x04\x03")), (0xff01, b"\x00"),
        ])
        message = handshake(1, b"\x03\x03" + client_random + b"\x00" + vector(offered or suite.to_bytes(2, "big"))
                            + b"\x01\x00" + vector(extensions))
        self.transcript = message
        self.send_record(22, message)
        pending = b""
        messages = []
        while not messages or messages[-1][0] != 14:
            kind, data = self.record()
            assert kind == 22, (kind, data.hex())
            pending += data
            while len(pending) >= 4:
                length = int.from_bytes(pending[1:4], "big") + 4
                if len(pending) < length:
                    break
                message, pending = pending[:length], pending[length:]
                self.transcript += message
                messages.append((message[0], message[4:]))
        assert not pending
        assert [kind for kind, _ in messages] == ([2, 11, 12, 14] if suite == 0xC023 else [2, 11, 14])
        server_hello = messages[0][1]
        pos = 35 + server_hello[34]
        assert server_hello[:2] == b"\x03\x03"
        assert int.from_bytes(server_hello[pos:pos + 2], "big") == suite
        assert server_hello[pos + 2] == 0
        server_random = server_hello[2:34]
        cert_message = messages[1][1]
        assert int.from_bytes(cert_message[:3], "big") == len(cert_message) - 3
        length = int.from_bytes(cert_message[3:6], "big")
        assert cert_message[6:6 + length] == expected_leaf.public_bytes(serialization.Encoding.DER)
        assert len(cert_message) == 6 + length  # fixture is directly root-issued
        root.public_key().verify(expected_leaf.signature, expected_leaf.tbs_certificate_bytes,
                                 ec.ECDSA(expected_leaf.signature_hash_algorithm))
        peer = expected_leaf.public_key()
        if suite == 0xC023:
            ske = messages[2][1]
            assert ske[:3] == b"\x03\x00\x17"
            params_len = 4 + ske[3]
            assert ske[params_len:params_len + 2] == b"\x04\x03"
            sig_len = int.from_bytes(ske[params_len + 2:params_len + 4], "big")
            assert len(ske) == params_len + 4 + sig_len
            peer.verify(ske[params_len + 4:], client_random + server_random + ske[:params_len], ec.ECDSA(hashes.SHA256()))
            peer = ec.EllipticCurvePublicKey.from_encoded_point(ec.SECP256R1(), ske[4:params_len])
        else:
            assert expected_leaf.extensions.get_extension_for_class(x509.KeyUsage).value.key_agreement
        key = ec.generate_private_key(ec.SECP256R1())
        premaster = key.exchange(ec.ECDH(), peer)
        point = key.public_key().public_bytes(serialization.Encoding.X962, serialization.PublicFormat.UncompressedPoint)
        cke = handshake(16, bytes([len(point)]) + point)
        self.send_record(22, cke)
        self.transcript += cke
        master = prf(premaster, b"master secret", client_random + server_random, 48)
        block = prf(master, b"key expansion", server_random + client_random, 96)
        self.client_mac, self.server_mac = block[:32], block[32:64]
        self.client_key, self.server_key = block[64:80], block[80:96]
        self.send_record(20, b"\x01")
        finished = handshake(20, prf(master, b"client finished", hashlib.sha256(self.transcript).digest(), 12))
        self.encrypt(22, finished)
        self.transcript += finished
        assert self.record() == (20, b"\x01")
        kind, data = self.decrypt()
        assert kind == 22
        assert hmac.compare_digest(data, handshake(20, prf(master, b"server finished", hashlib.sha256(self.transcript).digest(), 12)))

    def sendall(self, data):
        self.encrypt(23, data)

    def recv(self, size):
        kind, data = self.decrypt()
        assert kind == 23 and len(data) <= size
        return data


def check_suite(tc, suite, offered=None):
    from iso15118.shared.messages.enums import Namespace
    tc.set_test_timeout(120)
    env = validation.environment
    response = common.sdp_request(env.iface, expected_from=env.target_ll)
    tc.assert_(response is not None and response["security"] == common.SDP_SECURITY_TLS)
    with socket.socket(socket.AF_INET6, socket.SOCK_STREAM) as sock:
        sock.settimeout(45)
        sock.connect((response["secc_ll"], response["port"], 0, socket.if_nametoindex(env.iface)))
        tls = TLS12Client(sock)
        tls.connect(suite, x509.load_pem_x509_certificate((env.work / "iso2-secc.pem").read_bytes()), env.pki["iso2"][0], offered)
        result = vehicle.exchange(tls, "supportedAppProtocolReq", {"AppProtocol": [common.ISO2]}, Namespace.SAP, 0x8001)
        tc.assert_eq("OK_SuccessfulNegotiation", result["supportedAppProtocolRes"]["ResponseCode"])
        tls.encrypt(21, b"\x01\x00")
    env.record("ISO2 forced cipher authenticated handshake and encrypted SAP", hex(suite))
    time.sleep(2)


def test_ecdhe_ecdsa(tc: TestContext):
    check_suite(tc, 0xC023)


def test_static_ecdh_ecdsa(tc: TestContext):
    check_suite(tc, 0xC025)


def test_ecdhe_preferred_over_static_ecdh(tc: TestContext):
    check_suite(tc, 0xC023, bytes.fromhex("c025c023"))


if __name__ == "__main__":
    run_testsuite(locals())
