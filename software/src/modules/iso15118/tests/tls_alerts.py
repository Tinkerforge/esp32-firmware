#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import socket
import subprocess
import time

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

from _common import EVTestClient, IsoTestEnvironment, managed_socket


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


def generate_tests():
    return {
        f"test_{name}": lambda tc, args=args: check_alert(tc, args)
        for name, args in CASES.items()
    }


if __name__ == "__main__":
    run_testsuite(dict(locals(), **generate_tests()))
