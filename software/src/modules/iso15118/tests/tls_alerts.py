#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import socket
import subprocess
import time

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from _common import CERTS_DIR, EVTestClient, IsoTestEnvironment, managed_socket
from software.test_runner.test_context import TestContext, run_testsuite

environment = None
client = None


CASES = {
    "tls13_p256_only": ["-tls1_3", "-groups", "P-256"],
    "tls13_x25519_only": ["-tls1_3", "-groups", "X25519"],
    "tls12_sha1_only": [
        "-tls1_2", "-cipher", "ECDHE-ECDSA-AES128-SHA256:@SECLEVEL=0",
        "-curves", "P-256", "-sigalgs", "ECDSA+SHA1",
    ],
    "tls12_x25519_x448_only": [
        "-tls1_2", "-cipher", "ECDHE-ECDSA-AES128-SHA256",
        "-curves", "X25519:X448",
    ],
    "tls12_disjoint_cipher": ["-tls1_2", "-cipher", "ECDHE-RSA-AES256-GCM-SHA384"],
}


def suite_setup(tc: TestContext):
    global environment, client
    environment = IsoTestEnvironment(tc)
    environment.start()
    client = EVTestClient(environment.host, environment.iface, environment.secc_ll)


def setup(tc: TestContext):
    assert environment is not None
    environment.reset_session()


def suite_teardown(tc: TestContext):
    if environment is not None:
        environment.stop()


def capture_client_hello(arguments):
    with socket.socket() as server:
        server.bind(("127.0.0.1", 0))
        server.listen(1)
        port = server.getsockname()[1]
        process = subprocess.Popen(
            ["openssl", "s_client", "-connect", f"127.0.0.1:{port}", *arguments],
            stdin=subprocess.DEVNULL,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        try:
            connection, _ = server.accept()
            with connection:
                connection.settimeout(2)
                data = bytearray()
                while len(data) < 5 or len(data) < 5 + int.from_bytes(data[3:5], "big"):
                    chunk = connection.recv(4096)
                    if not chunk:
                        break
                    data.extend(chunk)
                return bytes(data)
        finally:
            process.kill()
            process.wait()


def check_alert(tc: TestContext, arguments):
    assert client is not None
    hello = capture_client_hello(arguments)
    tc.assert_ge(5, len(hello))
    with managed_socket(client.connect_raw()) as sock:
        sock.sendall(hello)
        response = sock.recv(4096)
    time.sleep(0.2)
    tc.assert_ge(7, len(response))
    tc.assert_eq(0x15, response[0])
    tc.assert_eq(2, response[5])
    tc.assert_eq(40, response[6])


def corrupt_certificate_authorities_length(hello):
    data = bytearray(hello)
    body = 9
    body += 2 + 32
    body += 1 + data[body]
    cipher_len = int.from_bytes(data[body:body + 2], "big")
    body += 2 + cipher_len
    body += 1 + data[body]
    extensions_len = int.from_bytes(data[body:body + 2], "big")
    body += 2
    extensions_end = body + extensions_len
    while body < extensions_end:
        extension_type = int.from_bytes(data[body:body + 2], "big")
        extension_len = int.from_bytes(data[body + 2:body + 4], "big")
        extension_data = body + 4
        if extension_type == 47:
            authorities_len = int.from_bytes(
                data[extension_data:extension_data + 2], "big")
            data[extension_data:extension_data + 2] = (
                authorities_len - 1).to_bytes(2, "big")
            return bytes(data)
        body = extension_data + extension_len
    raise AssertionError("OpenSSL did not send certificate_authorities")


def test_tls13_malformed_certificate_authorities(tc: TestContext):
    # RFC 8446 framing for the V2G20-2379/3376 chain-selection input.
    assert client is not None
    hello = capture_client_hello([
        "-tls1_3",
        "-requestCAfile", str(CERTS_DIR / "iso20/certs/v2gRootCACert.pem"),
    ])
    hello = corrupt_certificate_authorities_length(hello)
    with managed_socket(client.connect_raw()) as sock:
        sock.sendall(hello)
        response = sock.recv(4096)
    tc.assert_ge(7, len(response))
    tc.assert_eq(0x15, response[0])
    tc.assert_eq(2, response[5])
    tc.assert_eq(50, response[6])


def generate_tests():
    return {
        f"test_{name}": lambda tc, args=args: check_alert(tc, args)
        for name, args in CASES.items()
    }


if __name__ == "__main__":
    run_testsuite(dict(locals(), **generate_tests()))
