"""Wire group selection plus authenticated OpenSSL handshakes under suite policy."""
import contextlib
import os
from pathlib import Path
import socket
import subprocess
import sys

from cryptography.hazmat.primitives.asymmetric import ec, x448
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat

SERVER, CERTS = sys.argv[1], Path(sys.argv[2]).resolve()
P256, P521, X448, X25519 = 23, 25, 30, 29
HRR = bytes.fromhex('cf21ad74e59a6111be1d8c021e65b891c2a211167abb8c5e079e09e2c8a8339c')


def u16(n):
    return n.to_bytes(2, 'big')


def vector(data):
    return u16(len(data)) + data


def ext(kind, data):
    return u16(kind) + vector(data)


@contextlib.contextmanager
def server(policy, version=13):
    certs = CERTS if version == 13 else CERTS.parent / 'iso2'
    p = subprocess.Popen([SERVER, str(certs / 'certs/cpoCertChain.pem'),
                          str(certs / 'private_keys/seccLeaf_unencrypted.key'), str(version),
                          str(int(policy[0])), str(int(policy[1]))],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    try:
        line = p.stdout.readline()
        assert line.startswith('PORT '), (line, p.communicate(timeout=10))
        yield int(line.split()[1]), p
    finally:
        if p.poll() is None:
            p.terminate()
        p.communicate(timeout=10)


def share(group):
    if group == X448:
        return x448.X448PrivateKey.generate().public_key().public_bytes(Encoding.Raw, PublicFormat.Raw)
    curve = ec.SECP521R1() if group == P521 else ec.SECP256R1()
    return ec.generate_private_key(curve).public_key().public_bytes(Encoding.X962, PublicFormat.UncompressedPoint)


def hello(groups, shares, reverse=False, malformed=False):
    group_ext = ext(10, vector(b''.join(u16(g) for g in groups)))
    share_ext = ext(51, vector(b''.join(u16(g) + vector(share(g)) for g in shares)))
    if malformed:
        share_ext = ext(51, b'\x00\x04\x00\x19\x00')
    extensions = ext(43, b'\x02\x03\x04') + ext(13, vector(b'\x06\x03'))
    extensions += share_ext + group_ext if reverse else group_ext + share_ext
    body = b'\x03\x03' + os.urandom(32) + b'\x00' + vector(b'\x13\x02') + b'\x01\x00' + vector(extensions)
    msg = b'\x01' + len(body).to_bytes(3, 'big') + body
    return b'\x16\x03\x01' + vector(msg)


def read_exact(sock, n):
    data = b''
    while len(data) < n:
        chunk = sock.recv(n - len(data))
        assert chunk, 'unexpected EOF'
        data += chunk
    return data


def response(sock):
    header = read_exact(sock, 5)
    body = read_exact(sock, int.from_bytes(header[3:], 'big'))
    if header[0] == 21:
        return 'alert', body[1]
    assert header[0] == 22 and body[0] == 2, (header, body)
    retry = body[6:38] == HRR
    pos = 39 + body[38] + 3
    length = int.from_bytes(body[pos:pos + 2], 'big')
    pos += 2
    end = pos + length
    while pos < end:
        kind = int.from_bytes(body[pos:pos + 2], 'big')
        size = int.from_bytes(body[pos + 2:pos + 4], 'big')
        pos += 4
        if kind == 51:
            return 'hrr' if retry else 'share', int.from_bytes(body[pos:pos + 2], 'big')
        pos += size
    raise AssertionError('no server key_share')


count = 0


def wire(name, policy, groups, shares, expected, reverse=False, malformed=False):
    global count
    with server(policy) as (port, proc):
        with socket.create_connection(('127.0.0.1', port), timeout=5) as sock:
            sock.sendall(hello(groups, shares, reverse, malformed))
            actual = response(sock)
            assert actual == expected, (name, actual, expected)
    count += 1
    print('ok', name)


for reverse in (False, True):
    for groups in ([P521, X448], [X448, P521]):
        suffix = f'groups={groups} key_share_first={reverse}'
        wire('server preference ' + suffix, (1, 1), groups, groups, ('share', P521), reverse)
        wire('preferred share absent ' + suffix, (1, 1), groups, [X448], ('hrr', P521), reverse)
        wire('empty shares ' + suffix, (1, 1), groups, [], ('hrr', P521), reverse)
        wire('Ed448-only ' + suffix, (0, 1), groups, groups, ('share', X448), reverse)
        wire('P521-only ' + suffix, (1, 0), groups, groups, ('share', P521), reverse)
    wire('malformed key_share', (1, 1), [P521], [], ('alert', 50), reverse, True)
wire('only mutual X448', (1, 1), [X448], [X448], ('share', X448))
wire('P521 disabled', (0, 1), [P521], [P521], ('alert', 40))
wire('X448 disabled', (1, 0), [X448], [X448], ('alert', 40))
wire('all ISO20 groups disabled', (0, 0), [P521, X448], [P521, X448], ('alert', 40))
wire('P256 forbidden in TLS13', (1, 1), [P256], [P256], ('alert', 40))
wire('unsupported initial share', (1, 1), [P256, X448, P521], [P256], ('hrr', P521))

# OpenSSL verifies chain, hostname and Finished; the server checks application
# data in both directions. X448-first exercises HRR to the preferred P-521.
for policy, groups, expected, version in [
    ((1, 1), 'X448:P-521', 'secp521r1', 13),
    ((1, 1), 'P-521:X448', 'secp521r1', 13),
    ((0, 1), 'P-521:X448', 'X448', 13),
    ((1, 0), 'X448:P-521', 'secp521r1', 13),
    ((1, 1), 'X448', 'X448', 13),
    ((0, 0), 'P-256', 'prime256v1', 12),
]:
    with server(policy, version) as (port, proc):
        result = subprocess.run(['openssl', 's_client', '-connect', f'127.0.0.1:{port}',
                                 f'-tls1_{version - 10}', '-groups', groups,
                                 '-CAfile', str((CERTS if version == 13 else CERTS.parent / 'iso2') / 'certs/v2gRootCACert.pem'),
                                 '-verify_hostname', 'SECCCert', '-verify_return_error', '-ign_eof'],
                                input='ping', capture_output=True, text=True, timeout=15)
        assert result.returncode == 0, (result.stdout, result.stderr)
        output = result.stdout + result.stderr
        assert 'pong' in output and expected in output and 'Verify return code: 0 (ok)' in output, output
        out, err = proc.communicate(timeout=10)
        assert proc.returncode == 0 and 'COMPLETE' in out, (out, err)
    count += 1
    print('ok authenticated', policy, groups, version)
print(f'PASS {count} scenarios')
