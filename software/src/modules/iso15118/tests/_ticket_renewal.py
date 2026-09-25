import socket
import ssl
import subprocess
import sys
from pathlib import Path

server, certs = sys.argv[1], Path(sys.argv[2]).resolve()
proc = subprocess.Popen([server, str(certs / 'certs/cpoCertChain.pem'),
                         str(certs / 'private_keys/seccLeaf_unencrypted.key')],
                        stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
try:
    line = proc.stdout.readline()
    assert line.startswith('PORT '), (line, proc.communicate(timeout=10))
    port = int(line.split()[1])
    context = ssl.create_default_context(cafile=str(certs / 'certs/v2gRootCACert.pem'))
    context.minimum_version = context.maximum_version = ssl.TLSVersion.TLSv1_3
    messages = []

    def callback(conn, direction, version, content_type, message_type, data):
        if direction == 'read' and int(content_type) == 22 and int(message_type) == 4:
            messages.append(bytes(data))

    context._msg_callback = callback
    session = None
    for connection in range(3):
        before = len(messages)
        with socket.create_connection(('127.0.0.1', port), timeout=5) as sock:
            with context.wrap_socket(sock, server_hostname='SECCCert',
                                     session=session if connection == 1 else None) as tls:
                assert tls.session_reused == (connection == 1)
                tls.sendall(b'x')
                data = b''
                while len(data) < (2 if connection == 2 else 3):
                    chunk = tls.recv(3 - len(data))
                    assert chunk
                    data += chunk
                assert data == (b'AB' if connection == 2 else b'ABC'), data
                if connection != 2:
                    assert len(messages) - before == 3, len(messages) - before
                    session = tls.session
                    assert session.has_ticket
                    tls.sendall(b'x')
        print('ok', 'reset' if connection == 2 else 'resumed renewal' if connection else 'full handshake renewal')
    nonces, tickets = [], []
    for message in messages:
        body = message[4:]
        assert int.from_bytes(body[:4], 'big') == 3600
        n = body[8]
        assert n == 32
        nonces.append(body[9:9+n])
        pos = 9+n
        length = int.from_bytes(body[pos:pos+2], 'big')
        tickets.append(body[pos+2:pos+2+length])
        assert body[pos+2+length:] == b'\x00\x00'  # no early_data extension
    assert len(set(nonces)) == len(nonces)
    assert len(set(tickets)) == len(tickets)
    out, err = proc.communicate(timeout=10)
    assert proc.returncode == 0 and 'PASS' in out, (out, err)
    print(out, end='')
finally:
    if proc.poll() is None:
        proc.terminate()
    remaining_out, remaining_err = proc.communicate(timeout=10)
    if remaining_out or remaining_err:
        print(remaining_out, remaining_err, file=sys.stderr)
