#!/usr/bin/env python3
# Captures real ClientHello bytes from the python ssl stack (OpenSSL) for the
# classify_client_hello host test, written as C arrays to the file given as
# first argument.
import socket
import ssl
import threading
import sys

def capture(configure):
    server = socket.socket()
    server.bind(("127.0.0.1", 0))
    server.listen(1)
    port = server.getsockname()[1]
    data = bytearray()

    def client():
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        ctx.check_hostname = False
        ctx.verify_mode = ssl.CERT_NONE
        configure(ctx)
        try:
            with socket.create_connection(("127.0.0.1", port), timeout=2) as raw:
                with ctx.wrap_socket(raw, server_hostname="secc") as tls:
                    tls.recv(1)
        except Exception:
            pass

    t = threading.Thread(target=client)
    t.start()
    conn, _ = server.accept()
    conn.settimeout(2)
    try:
        while True:
            chunk = conn.recv(4096)
            if not chunk:
                break
            data += chunk
            # One full record is enough
            if len(data) >= 5 and len(data) >= 5 + int.from_bytes(data[3:5], "big"):
                break
    except socket.timeout:
        pass
    conn.close()
    server.close()
    t.join()
    return bytes(data)

def tls13(ctx):
    pass  # default: TLS 1.3 offered

def tls12_only(ctx):
    ctx.maximum_version = ssl.TLSVersion.TLSv1_2

with open(sys.argv[1], "w") as f:
    for name, conf in [("hello_tls13", tls13), ("hello_tls12", tls12_only)]:
        blob = capture(conf)
        print(f"{name}: {len(blob)} bytes")
        f.write(f"static const unsigned char {name}[] = {{")
        f.write(",".join(str(b) for b in blob))
        f.write("};\n")
