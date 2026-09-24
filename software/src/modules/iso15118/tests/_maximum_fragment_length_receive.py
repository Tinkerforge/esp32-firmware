#!/usr/bin/env python3
"""Use real OpenSSL handshakes, then replace one authenticated application record.

The proxy knows test-only traffic secrets from OpenSSL's key log. Re-encryption
distinguishes record_overflow from a bad-MAC rejection, for TLS 1.2 and 1.3.
"""

import hmac
from pathlib import Path
import selectors
import socket
import subprocess
import sys
import tempfile
import time

from cryptography.hazmat.primitives.ciphers.aead import AESGCM
from cryptography.exceptions import InvalidTag


def expand(secret, label, size):
    info = size.to_bytes(2, "big") + bytes([len(label) + 6]) + b"tls13 " + label + b"\0"
    output = block = b""
    while len(output) < size:
        block = hmac.digest(secret, block + info + bytes([len(output) // 32 + 1]), "sha256")
        output += block
    return output[:size]


def prf(secret, seed, size):
    output = b""
    a = seed
    while len(output) < size:
        a = hmac.digest(secret, a, "sha256")
        output += hmac.digest(secret, a + seed, "sha256")
    return output[:size]


def receive(connection, size):
    result = b""
    while len(result) < size:
        chunk = connection.recv(size - len(result))
        if not chunk:
            raise EOFError
        result += chunk
    return result


def record(connection):
    header = receive(connection, 5)
    return header, receive(connection, int.from_bytes(header[3:], "big"))


def secrets(path):
    return {parts[0]: bytes.fromhex(parts[2])
            for line in path.read_text().splitlines()
            if len(parts := line.split()) == 3 and not line.startswith("#")}


def run_case(server, certs, version, length, size, header_only=False, padding=0):
    with tempfile.TemporaryDirectory(prefix="mfl-receive-") as temporary:
        directory = Path(temporary)
        keylog = directory / "keys.log"
        chain = certs / "certs/cpoCertChain.pem"
        private_key = certs / "private_keys/seccLeaf_unencrypted.key"
        ca = certs / "certs/v2gRootCACert.pem"
        if version == 12:
            # Keep this receive regression independent of the TLS 1.2 writer's
            # lack of fragmentation for a multi-certificate handshake message.
            chain = ca = directory / "cert.pem"
            private_key = directory / "key.pem"
            subprocess.run([
                "openssl", "req", "-x509", "-newkey", "ec", "-pkeyopt",
                "ec_paramgen_curve:P-256", "-nodes", "-subj", "/CN=mfl",
                "-days", "1", "-keyout", str(private_key), "-out", str(chain),
            ], check=True, capture_output=True)
        with socket.create_server(("127.0.0.1", 0)) as listener:
            proxy_port = listener.getsockname()[1]
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                server_port = reservation.getsockname()[1]
            with (directory / "server.log").open("w+") as log:
                target = subprocess.Popen([
                    server, str(server_port), str(chain), str(private_key), str(version), "receive",
                ], stdout=log, stderr=log)
                client = None
                try:
                    # Retry only while the listener is starting, with a fixed deadline.
                    deadline = time.monotonic() + 10
                    while True:
                        try:
                            upstream = socket.create_connection(("127.0.0.1", server_port), timeout=1)
                            break
                        except ConnectionRefusedError:
                            if time.monotonic() >= deadline or target.poll() is not None:
                                raise
                            time.sleep(0.02)
                    client = subprocess.Popen([
                        "openssl", "s_client", "-connect", f"127.0.0.1:{proxy_port}",
                        f"-tls1_{version - 10}",
                        *(["-maxfraglen", str(length)] if length != 16384 else []),
                        "-cipher", "ECDHE-ECDSA-AES128-GCM-SHA256",
                        "-ciphersuites", "TLS_AES_128_GCM_SHA256",
                        "-keylogfile", str(keylog), "-quiet", "-ign_eof",
                        "-CAfile", str(ca),
                    ], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    client.stdin.write(b"probe\n")
                    client.stdin.flush()
                    downstream, _ = listener.accept()
                    with upstream, downstream, selectors.DefaultSelector() as selector:
                        upstream.settimeout(10)
                        downstream.settimeout(10)
                        selector.register(upstream, selectors.EVENT_READ)
                        selector.register(downstream, selectors.EVENT_READ)
                        client_random = server_random = None
                        tls12_encrypted = False
                        sequence = 0
                        replaced = False
                        while not replaced:
                            events = selector.select(10)
                            assert events, "proxy timed out"
                            for key, _ in events:
                                source = key.fileobj
                                header, body = record(source)
                                if source is upstream:
                                    if header[0] == 22 and body[0] == 2:
                                        server_random = body[6:38]
                                    downstream.sendall(header + body)
                                    continue
                                if header[0] == 22 and not tls12_encrypted and body[0] == 1:
                                    client_random = body[6:38]
                                if version == 12 and header[0] == 20:
                                    tls12_encrypted = True
                                    upstream.sendall(header + body)
                                    continue
                                if version == 12 and tls12_encrypted:
                                    seq = sequence.to_bytes(8, "big")
                                    sequence += 1
                                    if header[0] == 23:
                                        master = secrets(keylog)["CLIENT_RANDOM"]
                                        keys = prf(master, b"key expansion" + server_random + client_random, 40)
                                        cipher = AESGCM(keys[:16])
                                        iv = keys[32:36] + body[:8]
                                        aad = seq + header[:3] + (len(body) - 24).to_bytes(2, "big")
                                        assert cipher.decrypt(iv, body[8:], aad) == b"probe\n"
                                        plaintext = b"P" * size
                                        aad = seq + header[:3] + size.to_bytes(2, "big")
                                        body = body[:8] + cipher.encrypt(iv, plaintext, aad)
                                        replaced = True
                                elif version == 13 and header[0] == 23:
                                    traffic = secrets(keylog)
                                    if "CLIENT_TRAFFIC_SECRET_0" in traffic:
                                        secret = traffic["CLIENT_TRAFFIC_SECRET_0"]
                                        cipher = AESGCM(expand(secret, b"key", 16))
                                        iv = expand(secret, b"iv", 12)  # first application record, sequence 0
                                        try:
                                            plain = cipher.decrypt(iv, body, header)
                                        except InvalidTag:
                                            pass  # client Finished uses the handshake traffic key
                                        else:
                                            assert plain.rstrip(b"\0") == b"probe\n\x17"
                                            plaintext = b"P" * size + b"\x17" + b"\0" * padding
                                            header = header[:3] + (len(plaintext) + 16).to_bytes(2, "big")
                                            body = cipher.encrypt(iv, plaintext, header)
                                            replaced = True
                                if replaced:
                                    if header_only:
                                        header = header[:3] + (length + 2048).to_bytes(2, "big")
                                        body = b""
                                    else:
                                        header = header[:3] + len(body).to_bytes(2, "big")
                                upstream.sendall(header + body)
                        # Relay the server's encrypted alert so OpenSSL checks its exact type.
                        while True:
                            try:
                                header, body = record(upstream)
                            except (EOFError, ConnectionResetError):
                                break
                            downstream.sendall(header + body)
                    _, error = client.communicate(timeout=10)
                    result = target.wait(timeout=10)
                    log.seek(0)
                    output = log.read()
                    rejected = header_only or size > length
                    assert result == (3 if rejected else 0), (result, output, error)
                    if rejected:
                        assert b"alert record overflow" in error.lower(), error
                        assert "simulated WANT_WRITE" in output, output
                    else:
                        assert f"received: {size}" in output, output
                    assert f"maximum input fragment: {length}" in output, output
                    print(f"PASS TLS1.{version - 10} MFL={length} content={size} padding={padding} header_only={header_only}")
                except Exception:
                    log.seek(0)
                    print(log.read(), file=sys.stderr)
                    if client is not None:
                        client.kill()
                        print(client.communicate(timeout=10)[1].decode(), file=sys.stderr)
                    raise
                finally:
                    for process in (client, target):
                        if process is not None and process.poll() is None:
                            process.kill()
                            process.wait()


if __name__ == "__main__":
    for version in (12, 13):
        for size in (511, 512, 513):
            run_case(sys.argv[1], Path(sys.argv[2]), version, 512, size)
        run_case(sys.argv[1], Path(sys.argv[2]), version, 512, 512, header_only=True)
    # MFL limits content, unlike record_size_limit which includes type/padding.
    run_case(sys.argv[1], Path(sys.argv[2]), 13, 512, 512, padding=32)
    # Asymmetric buffer regression: output capacity is 4096, but absent MFL
    # the peer may still send an 8192-byte record to the 16384-byte input buffer.
    for version in (12, 13):
        run_case(sys.argv[1], Path(sys.argv[2]), version, 16384, 8192)
