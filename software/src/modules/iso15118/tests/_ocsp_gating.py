#!/usr/bin/env python3
"""HUB20-532-002 gating and OCSP staple plumbing

Provisions the certificate store step by step and checks the TLS server behavior after every step:
  1. store live but empty, no TLS handshake possible
  2. -2 and -20 chains provisioned, OCSP unknown: TLS 1.2 works and is
     verified against the dev V2G root, TLS 1.3 refused
  3. PrivateEnviromentEnabled does not waive OCSP because this station
     supports PnC; TLS 1.3 remains unavailable while OCSP is unknown
  4. Good OCSP responses for a -20 leaf and two intermediate CAs are
     delivered via GetCertificateStatus; TLS 1.3 remains unavailable until
     all three responses are present
  5. the decrypted TLS 1.3 Certificate message contains the exact response
     belonging to each CertificateEntry, in leaf-first order (needs firmware
     built against libs with the stapling patch)

Invoked by certificates.py through the firmware test runner.
"""

import argparse
import base64
import hashlib
import hmac
import os
import ssl
import struct
import subprocess
import sys
import tempfile
import shutil
import time
import urllib.request
from dataclasses import dataclass
from datetime import datetime, timedelta, timezone
from pathlib import Path

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
from cryptography.x509 import ocsp
from cryptography.x509.oid import AuthorityInformationAccessOID, NameOID

import _common as common
from _common import CSMSSim as Csms, EVTestClient, ISO2, managed_socket

SCRIPT_DIR = Path(__file__).parent.resolve()
CERTS = SCRIPT_DIR / ".." / "tools" / "certs" / "output"
OCSP_URL = "http://ocsp.test.example/"
TLS_STATUS_REQUEST = 5
TLS_STATUS_REQUEST_V2 = 17
TLS_EARLY_DATA = 42
NEXT_UPDATE_OFFSET_S = 120
SEVEN_DAYS_S = 7 * 24 * 60 * 60
PRE_CAP_OFFSET_S = NEXT_UPDATE_OFFSET_S + SEVEN_DAYS_S - 120
POST_CAP_OFFSET_S = NEXT_UPDATE_OFFSET_S + SEVEN_DAYS_S + 300


@dataclass
class Iso20Chain:
    certificates: list[x509.Certificate]
    issuers: list[x509.Certificate]
    issuer_keys: list[ec.EllipticCurvePrivateKey]
    urls: list[str]

    @property
    def ders(self):
        return [certificate.public_bytes(serialization.Encoding.DER)
                for certificate in self.certificates]

    def response(self, index, lifetime=timedelta(days=7)):
        now = datetime.now(timezone.utc)
        response = (
            ocsp.OCSPResponseBuilder()
            .add_response(
                cert=self.certificates[index],
                issuer=self.issuers[index],
                algorithm=hashes.SHA256(),
                cert_status=ocsp.OCSPCertStatus.GOOD,
                this_update=now,
                next_update=now + lifetime,
                revocation_time=None,
                revocation_reason=None,
            )
            .responder_id(ocsp.OCSPResponderEncoding.HASH, self.issuers[index])
            .certificates([self.issuers[index]])
            .sign(self.issuer_keys[index], hashes.SHA512())
        )
        return response.public_bytes(serialization.Encoding.DER)


@dataclass
class Tls13ServerFlight:
    server_hello_extensions: dict[int, bytes]
    handshake_messages: list[tuple[int, bytes]]
    certificate_entries: list[tuple[bytes, dict[int, bytes]]]


def certificate_hash_data(certificate, issuer):
    public_key = issuer.public_key()
    assert isinstance(public_key, ec.EllipticCurvePublicKey)
    issuer_key = public_key.public_bytes(
        serialization.Encoding.X962, serialization.PublicFormat.UncompressedPoint)
    serial = format(certificate.serial_number, "x")
    if len(serial) % 2:
        serial = "0" + serial
    return {
        "hashAlgorithm": "SHA256",
        "issuerNameHash": hashlib.sha256(certificate.issuer.public_bytes()).hexdigest(),
        "issuerKeyHash": hashlib.sha256(issuer_key).hexdigest(),
        "serialNumber": serial,
    }


def run(cmd, **kwargs):
    return subprocess.run(cmd, check=True, capture_output=True, text=True, **kwargs)


def sign_csr(workdir, csr_pem, iso20, with_aia):
    """Signs a device CSR and returns the transmitted chain PEM."""
    pki = CERTS / ("iso20" if iso20 else "iso2")
    issuer = "v2gRootCACert.pem" if iso20 else "cpoSubCA2Cert.pem"
    issuer_key = "v2gRootCA.key" if iso20 else "cpoSubCA2.key"
    csr = workdir / "device.csr"
    csr.write_text(csr_pem)
    ext = workdir / "ext.cnf"
    ext.write_text("[ext]\n"
                   "basicConstraints = critical,CA:false\n"
                   "keyUsage = critical,digitalSignature,keyAgreement\n"
                   "subjectKeyIdentifier = hash\n"
                   + (f"authorityInfoAccess = OCSP;URI:{OCSP_URL}\n" if with_aia else ""))
    leaf = workdir / "leaf.pem"
    run(["openssl", "x509", "-req", "-in", str(csr),
         "-CA", str(pki / "certs" / issuer),
         "-CAkey", str(pki / "private_keys" / issuer_key), "-passin", "pass:12345",
         "-CAcreateserial", "-days", "60", "-sha512" if iso20 else "-sha256",
         "-extfile", str(ext), "-extensions", "ext", "-out", str(leaf)])
    chain = leaf.read_text()
    if not iso20:
        chain += (pki / "certs" / "cpoSubCA2Cert.pem").read_text()
        chain += (pki / "certs" / "cpoSubCA1Cert.pem").read_text()
    return chain


def ca_key_usage():
    return x509.KeyUsage(
        digital_signature=True,
        content_commitment=False,
        key_encipherment=False,
        data_encipherment=False,
        key_agreement=False,
        key_cert_sign=True,
        crl_sign=True,
        encipher_only=False,
        decipher_only=False,
    )


def leaf_key_usage():
    return x509.KeyUsage(
        digital_signature=True,
        content_commitment=False,
        key_encipherment=False,
        data_encipherment=False,
        key_agreement=True,
        key_cert_sign=False,
        crl_sign=False,
        encipher_only=False,
        decipher_only=False,
    )


def aia(url):
    return x509.AuthorityInformationAccess([
        x509.AccessDescription(
            AuthorityInformationAccessOID.OCSP,
            x509.UniformResourceIdentifier(url),
        )
    ])


def sign_ca(subject, key, issuer, issuer_key, path_length, url, not_before, not_after):
    return (
        x509.CertificateBuilder()
        .subject_name(subject)
        .issuer_name(issuer.subject)
        .public_key(key.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(not_before)
        .not_valid_after(not_after)
        .add_extension(x509.BasicConstraints(ca=True, path_length=path_length), critical=True)
        .add_extension(ca_key_usage(), critical=True)
        .add_extension(x509.SubjectKeyIdentifier.from_public_key(key.public_key()), critical=False)
        .add_extension(x509.AuthorityKeyIdentifier.from_issuer_public_key(issuer_key.public_key()), critical=False)
        .add_extension(aia(url), critical=False)
        .sign(issuer_key, hashes.SHA512())
    )


def sign_iso20_chain(workdir, csr_pem):
    pki = CERTS / "iso20"
    root = x509.load_pem_x509_certificate((pki / "certs" / "v2gRootCACert.pem").read_bytes())
    root_key = serialization.load_pem_private_key(
        (pki / "private_keys" / "v2gRootCA.key").read_bytes(), b"12345")
    assert isinstance(root_key, ec.EllipticCurvePrivateKey)

    now = datetime.now(timezone.utc)
    not_before = now - timedelta(days=1)
    not_after = now + timedelta(days=60)
    sub1_key = ec.generate_private_key(ec.SECP521R1())
    sub2_key = ec.generate_private_key(ec.SECP521R1())
    sub1_name = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, "DE"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "WARP"),
        x509.NameAttribute(NameOID.COMMON_NAME, "OCSP Test CPO Sub-CA 1"),
        x509.NameAttribute(NameOID.DOMAIN_COMPONENT, "V2G"),
    ])
    sub2_name = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, "DE"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "WARP"),
        x509.NameAttribute(NameOID.COMMON_NAME, "OCSP Test CPO Sub-CA 2"),
        x509.NameAttribute(NameOID.DOMAIN_COMPONENT, "V2G"),
    ])
    urls = [OCSP_URL + name for name in ("leaf", "sub2", "sub1")]
    sub1 = sign_ca(sub1_name, sub1_key, root, root_key, 1, urls[2], not_before, not_after)
    sub2 = sign_ca(sub2_name, sub2_key, sub1, sub1_key, 0, urls[1], not_before, not_after)

    csr = x509.load_pem_x509_csr(csr_pem.encode("ascii"))
    assert csr.is_signature_valid
    public_key = csr.public_key()
    assert isinstance(public_key, ec.EllipticCurvePublicKey)
    assert isinstance(public_key.curve, ec.SECP521R1), public_key.curve.name
    leaf = (
        x509.CertificateBuilder()
        .subject_name(csr.subject)
        .issuer_name(sub2.subject)
        .public_key(public_key)
        .serial_number(x509.random_serial_number())
        .not_valid_before(not_before)
        .not_valid_after(not_after)
        .add_extension(x509.BasicConstraints(ca=False, path_length=None), critical=True)
        .add_extension(leaf_key_usage(), critical=True)
        .add_extension(x509.SubjectKeyIdentifier.from_public_key(public_key), critical=False)
        .add_extension(x509.AuthorityKeyIdentifier.from_issuer_public_key(sub2_key.public_key()), critical=False)
        .add_extension(aia(urls[0]), critical=False)
        .sign(sub2_key, hashes.SHA512())
    )

    certificates = [leaf, sub2, sub1]
    for name, certificate in zip(("leaf20", "sub2-20", "sub1-20"), certificates):
        path = workdir / f"{name}.pem"
        path.write_bytes(certificate.public_bytes(serialization.Encoding.PEM))
    return Iso20Chain(
        certificates=certificates,
        issuers=[sub2, sub1, root],
        issuer_keys=[sub2_key, sub1_key, root_key],
        urls=urls,
    )


def sign_iso2_csr_at(csr_pem, not_before):
    pki = CERTS / "iso2"
    sub2 = x509.load_pem_x509_certificate(
        (pki / "certs" / "cpoSubCA2Cert.pem").read_bytes())
    sub1 = x509.load_pem_x509_certificate(
        (pki / "certs" / "cpoSubCA1Cert.pem").read_bytes())
    root = x509.load_pem_x509_certificate(
        (pki / "certs" / "v2gRootCACert.pem").read_bytes())
    sub2_key = serialization.load_pem_private_key(
        (pki / "private_keys" / "cpoSubCA2.key").read_bytes(), b"12345")
    assert isinstance(sub2_key, ec.EllipticCurvePrivateKey)
    csr = x509.load_pem_x509_csr(csr_pem.encode("ascii"))
    assert csr.is_signature_valid
    public_key = csr.public_key()
    assert isinstance(public_key, ec.EllipticCurvePublicKey)
    assert isinstance(public_key.curve, ec.SECP256R1), public_key.curve.name
    leaf = (
        x509.CertificateBuilder()
        .subject_name(csr.subject)
        .issuer_name(sub2.subject)
        .public_key(public_key)
        .serial_number(x509.random_serial_number())
        .not_valid_before(not_before)
        .not_valid_after(not_before + timedelta(days=30))
        .add_extension(x509.BasicConstraints(ca=False, path_length=None), critical=True)
        .add_extension(leaf_key_usage(), critical=True)
        .add_extension(x509.SubjectKeyIdentifier.from_public_key(public_key), critical=False)
        .add_extension(x509.AuthorityKeyIdentifier.from_issuer_public_key(sub2_key.public_key()), critical=False)
        .sign(sub2_key, hashes.SHA256())
    )
    chain = "".join(
        certificate.public_bytes(serialization.Encoding.PEM).decode("ascii")
        for certificate in (leaf, sub2, sub1)
    )
    return leaf, sub2, sub1, root, chain


def start_v2g_certificate_csr(csms):
    assert csms.call("TriggerMessage", {
        "requestedMessage": "SignV2GCertificate",
    })["status"] == "Accepted"
    request, message_id = csms.expect("SignCertificate", timeout=60)
    csms.respond(message_id, {"status": "Accepted"})
    assert request["certificateType"] == "V2GCertificate", request
    return request


def installed_chain_for_leaf(csms, leaf_hash):
    response = csms.call("GetInstalledCertificateIds", {
        "certificateType": ["V2GCertificateChain"],
    })
    if response["status"] != "Accepted":
        return response, []
    matches = [
        entry for entry in response["certificateHashDataChain"]
        if entry["certificateHashData"] == leaf_hash
    ]
    return response, matches


def ocsp_response_b64(workdir, leaf_pem_path, next_update_minutes=None):
    """Good OCSP response for the directly root-signed ISO 15118-20 leaf."""
    pki = CERTS / "iso20"
    serial = run(["openssl", "x509", "-in", str(leaf_pem_path), "-noout", "-serial"]).stdout.strip().split("=")[1]
    enddate = run(["openssl", "x509", "-in", str(leaf_pem_path), "-noout", "-enddate"]).stdout.strip().split("=", 1)[1]
    stamp = run(["date", "-d", enddate, "-u", "+%y%m%d%H%M%SZ"]).stdout.strip()
    index = workdir / "index.txt"
    index.write_text(f"V\t{stamp}\t\t{serial}\tunknown\t/CN=x\n")
    resp = workdir / "resp.der"
    validity = ["-ndays", "7"] if next_update_minutes is None else ["-nmin", str(next_update_minutes)]
    run(["openssl", "ocsp", "-index", str(index),
          "-CA", str(pki / "certs" / "v2gRootCACert.pem"),
         "-rsigner", str(pki / "certs" / "v2gRootCACert.pem"),
         "-rkey", str(pki / "private_keys" / "v2gRootCA.key"), "-passin", "pass:12345",
          "-issuer", str(pki / "certs" / "v2gRootCACert.pem"),
          "-cert", str(leaf_pem_path),
          "-reqout", str(workdir / "req.der"), "-respout", str(resp)] + validity)
    return base64.b64encode(resp.read_bytes()).decode(), resp.read_bytes()


def provision_chain(csms, workdir, iso20, with_aia):
    kind = "V2G20Certificate" if iso20 else "V2GCertificate"
    assert csms.call("TriggerMessage", {"requestedMessage": f"Sign{kind}"})["status"] == "Accepted"
    sign_req, msg_id = csms.expect("SignCertificate", timeout=60)
    csms.respond(msg_id, {"status": "Accepted"})
    assert sign_req["certificateType"] == kind
    chain = sign_csr(workdir, sign_req["csr"], iso20, with_aia)
    # secp521r1 chain validation takes over 10 s on the device.
    res = csms.call("CertificateSigned", {
        "certificateChain": chain,
        "certificateType": kind,
        "requestId": sign_req["requestId"],
    }, timeout=90)
    assert res["status"] == "Accepted", res
    return chain


def provision_iso20_chain(csms, workdir):
    assert csms.call("TriggerMessage", {
        "requestedMessage": "SignV2G20Certificate",
    })["status"] == "Accepted"
    sign_req, msg_id = csms.expect("SignCertificate", timeout=60)
    csms.respond(msg_id, {"status": "Accepted"})
    assert sign_req["certificateType"] == "V2G20Certificate", sign_req
    chain = sign_iso20_chain(workdir, sign_req["csr"])
    result = csms.call("CertificateSigned", {
        "certificateChain": "".join(
            certificate.public_bytes(serialization.Encoding.PEM).decode("ascii")
            for certificate in chain.certificates
        ),
        "certificateType": "V2G20Certificate",
        "requestId": sign_req["requestId"],
    }, timeout=90)
    assert result["status"] == "Accepted", result
    return chain


def try_tls(charger, iface, tls13, mutual=False):
    """Returns the negotiated version string or None when refused."""
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    ctx.check_hostname = False
    if tls13:
        ctx.minimum_version = ssl.TLSVersion.TLSv1_3
        ctx.load_verify_locations(cafile=str(CERTS / "iso20" / "certs" / "v2gRootCACert.pem"))
        if mutual:
            ctx.load_cert_chain(certfile=str(CERTS / "iso20" / "certs" / "oemCertChain.pem"),
                                keyfile=str(CERTS / "iso20" / "private_keys" / "oemLeaf.key"),
                                password="12345")
    else:
        ctx.maximum_version = ssl.TLSVersion.TLSv1_2
        ctx.set_ciphers("ECDHE-ECDSA-AES128-SHA256")
        ctx.load_verify_locations(cafile=str(CERTS / "iso2" / "certs" / "v2gRootCACert.pem"))
    ctx.verify_mode = ssl.CERT_REQUIRED
    try:
        raw = common.connect_secc(charger, iface)
    except Exception:
        return None
    try:
        raw.settimeout(30)
        tls = ctx.wrap_socket(raw)
        ver = tls.version()
        tls.close()
        time.sleep(1)
        return ver
    except (ssl.SSLError, OSError):
        raw.close()
        time.sleep(1)
        return None


def trace_log(charger):
    with urllib.request.urlopen(f"http://{charger}/trace_log", timeout=20) as f:
        return f.read().decode(errors="replace")


def read_u24(data, offset):
    if offset + 3 > len(data):
        raise ValueError("truncated uint24")
    return int.from_bytes(data[offset:offset + 3], "big")


def parse_certificate_message(message):
    if len(message) < 8 or message[0] != 11:
        raise ValueError("invalid TLS Certificate message")
    message_length = read_u24(message, 1)
    if message_length != len(message) - 4:
        raise ValueError("invalid TLS Certificate message length")
    context_length = message[4]
    offset = 5 + context_length
    certificate_list_length = read_u24(message, offset)
    offset += 3
    if offset + certificate_list_length != len(message):
        raise ValueError("invalid TLS certificate_list length")

    entries = []
    while offset < len(message):
        certificate_length = read_u24(message, offset)
        offset += 3
        if certificate_length == 0 or offset + certificate_length > len(message):
            raise ValueError("invalid TLS CertificateEntry certificate length")
        certificate = message[offset:offset + certificate_length]
        offset += certificate_length
        if offset + 2 > len(message):
            raise ValueError("truncated TLS CertificateEntry extensions")
        extensions_length = int.from_bytes(message[offset:offset + 2], "big")
        offset += 2
        extensions_end = offset + extensions_length
        if extensions_end > len(message):
            raise ValueError("invalid TLS CertificateEntry extensions length")
        extensions = {}
        while offset < extensions_end:
            if offset + 4 > extensions_end:
                raise ValueError("truncated TLS CertificateEntry extension")
            extension_type = int.from_bytes(message[offset:offset + 2], "big")
            extension_length = int.from_bytes(message[offset + 2:offset + 4], "big")
            offset += 4
            if extension_type in extensions or offset + extension_length > extensions_end:
                raise ValueError("invalid TLS CertificateEntry extension")
            extensions[extension_type] = message[offset:offset + extension_length]
            offset += extension_length
        entries.append((certificate, extensions))
    return entries


def parse_status_request(extension):
    if len(extension) < 4 or extension[0] != 1:
        raise ValueError("invalid CertificateEntry status_request")
    response_length = read_u24(extension, 1)
    if response_length == 0 or response_length != len(extension) - 4:
        raise ValueError("invalid CertificateEntry OCSP response length")
    return extension[4:]


def tls_extension(extension_type, data):
    return struct.pack("!HH", extension_type, len(data)) + data


def hkdf_extract(salt, key_material):
    return hmac.new(salt, key_material, hashlib.sha384).digest()


def hkdf_expand(secret, info, length):
    output = b""
    block = b""
    for counter in range(1, (length + 47) // 48 + 1):
        block = hmac.new(secret, block + info + bytes([counter]), hashlib.sha384).digest()
        output += block
    return output[:length]


def hkdf_expand_label(secret, label, context, length):
    full_label = b"tls13 " + label
    info = (struct.pack("!H", length) + bytes([len(full_label)]) + full_label
            + bytes([len(context)]) + context)
    return hkdf_expand(secret, info, length)


def receive_record(sock):
    header = bytearray()
    while len(header) < 5:
        chunk = sock.recv(5 - len(header))
        if not chunk:
            raise ConnectionError("TLS connection closed before record header")
        header.extend(chunk)
    length = int.from_bytes(header[3:5], "big")
    payload = bytearray()
    while len(payload) < length:
        chunk = sock.recv(length - len(payload))
        if not chunk:
            raise ConnectionError("TLS connection closed within record")
        payload.extend(chunk)
    return bytes(header), bytes(payload)


def parse_extension_vector(data, offset):
    if offset + 2 > len(data):
        raise ValueError("truncated TLS extension vector")
    extensions_length = int.from_bytes(data[offset:offset + 2], "big")
    offset += 2
    end = offset + extensions_length
    if end != len(data):
        raise ValueError("invalid TLS extension vector length")
    extensions = {}
    while offset < end:
        if offset + 4 > end:
            raise ValueError("truncated TLS extension")
        extension_type, extension_length = struct.unpack("!HH", data[offset:offset + 4])
        offset += 4
        if extension_type in extensions or offset + extension_length > end:
            raise ValueError("invalid TLS extension")
        extensions[extension_type] = data[offset:offset + extension_length]
        offset += extension_length
    return extensions


def parse_server_hello(server_hello):
    if server_hello[0] != 2 or read_u24(server_hello, 1) != len(server_hello) - 4:
        raise ValueError("invalid ServerHello")
    body = server_hello[4:]
    offset = 34
    offset += 1 + body[offset]
    if body[offset:offset + 2] != b"\x13\x02" or body[offset + 2] != 0:
        raise ValueError("server did not select TLS_AES_256_GCM_SHA384")
    offset += 3
    extensions = parse_extension_vector(body, offset)
    key_share = extensions.get(51)
    if key_share is None or len(key_share) < 4 or key_share[:2] != b"\x00\x19":
        raise ValueError("server did not select secp521r1")
    key_length = int.from_bytes(key_share[2:4], "big")
    if key_length != len(key_share) - 4:
        raise ValueError("invalid ServerHello key_share")
    return extensions, key_share[4:]


def status_request_v2():
    ocsp_multi_request = b"\x00\x00\x00\x00"
    item = b"\x02" + struct.pack("!H", len(ocsp_multi_request)) + ocsp_multi_request
    return struct.pack("!H", len(item)) + item


def raw_tls13_client_hello(private_key, request_status=True,
                           request_status_v2=False, request_early_data=False):
    public_key = private_key.public_key().public_bytes(
        serialization.Encoding.X962, serialization.PublicFormat.UncompressedPoint)
    key_share = b"\x00\x19" + struct.pack("!H", len(public_key)) + public_key
    extensions = [
        tls_extension(43, b"\x02\x03\x04"),
        tls_extension(10, b"\x00\x02\x00\x19"),
        tls_extension(51, struct.pack("!H", len(key_share)) + key_share),
        tls_extension(13, b"\x00\x02\x06\x03"),
    ]
    if request_status:
        extensions.append(tls_extension(TLS_STATUS_REQUEST, b"\x01\x00\x00\x00\x00"))
    if request_status_v2:
        extensions.append(tls_extension(TLS_STATUS_REQUEST_V2, status_request_v2()))
    if request_early_data:
        extensions.append(tls_extension(TLS_EARLY_DATA, b""))
    extension_block = b"".join(extensions)
    session_id = os.urandom(32)
    body = (b"\x03\x03" + os.urandom(32)
            + bytes([len(session_id)]) + session_id
            + b"\x00\x02\x13\x02\x01\x00"
            + struct.pack("!H", len(extension_block)) + extension_block)
    return b"\x01" + len(body).to_bytes(3, "big") + body


def capture_tls13_server_flight(charger, iface, request_status=True,
                                request_status_v2=False, request_early_data=False):
    private_key = ec.generate_private_key(ec.SECP521R1())
    client_hello = raw_tls13_client_hello(
        private_key, request_status, request_status_v2, request_early_data)
    sock = common.connect_secc(charger, iface)
    try:
        sock.settimeout(30)
        sock.sendall(b"\x16\x03\x01" + struct.pack("!H", len(client_hello)) + client_hello)
        header, server_hello = receive_record(sock)
        if header[0] != 22 or server_hello[0] != 2:
            raise ValueError("first server record is not ServerHello")
        server_hello_extensions, server_key_share = parse_server_hello(server_hello)
        server_public = ec.EllipticCurvePublicKey.from_encoded_point(
            ec.SECP521R1(), server_key_share)
        shared_secret = private_key.exchange(ec.ECDH(), server_public)

        zero = bytes(48)
        early_secret = hkdf_extract(zero, zero)
        empty_hash = hashlib.sha384(b"").digest()
        derived_secret = hkdf_expand_label(early_secret, b"derived", empty_hash, 48)
        handshake_secret = hkdf_extract(derived_secret, shared_secret)
        transcript_hash = hashlib.sha384(client_hello + server_hello).digest()
        traffic_secret = hkdf_expand_label(
            handshake_secret, b"s hs traffic", transcript_hash, 48)
        key = hkdf_expand_label(traffic_secret, b"key", b"", 32)
        iv = hkdf_expand_label(traffic_secret, b"iv", b"", 12)

        handshake_data = bytearray()
        handshake_messages = []
        sequence = 0
        while True:
            header, ciphertext = receive_record(sock)
            if header[0] == 20:  # Compatibility ChangeCipherSpec.
                continue
            if header[0] != 23:
                raise ValueError(f"unexpected encrypted record type {header[0]}")
            nonce = bytearray(iv)
            encoded_sequence = sequence.to_bytes(12, "big")
            for index in range(12):
                nonce[index] ^= encoded_sequence[index]
            plaintext = AESGCM(key).decrypt(bytes(nonce), ciphertext, header)
            sequence += 1
            while plaintext and plaintext[-1] == 0:
                plaintext = plaintext[:-1]
            if not plaintext or plaintext[-1] != 22:
                raise ValueError("encrypted server record is not a handshake record")
            handshake_data.extend(plaintext[:-1])
            offset = 0
            while len(handshake_data) - offset >= 4:
                message_length = read_u24(handshake_data, offset + 1)
                end = offset + 4 + message_length
                if end > len(handshake_data):
                    break
                message = bytes(handshake_data[offset:end])
                message_type = message[0]
                handshake_messages.append((message_type, message))
                if message_type == 11:
                    return Tls13ServerFlight(
                        server_hello_extensions,
                        handshake_messages,
                        parse_certificate_message(message),
                    )
                offset = end
            if offset:
                del handshake_data[:offset]
    finally:
        sock.close()
        time.sleep(1)


def capture_certificate_entries(charger, iface):
    return capture_tls13_server_flight(charger, iface).certificate_entries


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--charger", required=True)
    p.add_argument("--iface")
    p.add_argument("--port", type=int, default=9500)
    args = p.parse_args()

    common.require_iso_tls_config(args.charger)
    iface = args.iface or common.route_interface(args.charger)
    local_ip = common.local_ip_towards(args.charger)
    pnc_supported = "iso15118_pnc" in common.api_get(args.charger, "info/features")
    workdir = Path(tempfile.mkdtemp(prefix="ocsp_gating_"))

    failures = 0
    csms_tls = common.LocalCSMSTls(args.charger, local_ip)

    def check(name, ok, detail=""):
        nonlocal failures
        print(f'{"ok  " if ok else "FAIL"} {name}{": " + str(detail) if detail else ""}')
        failures += 0 if ok else 1

    saved_config = common.api_get(args.charger, "ocpp/config")
    csms = Csms(args.port, interactive=("SignCertificate", "GetCertificateStatus"),
                certfile=str(csms_tls.certfile), keyfile=str(csms_tls.keyfile))
    try:
        common.api_put(args.charger, "ocpp/reset", None)
        test_config = dict(saved_config)
        test_config.update({"enable": True, "protocol": 1, "url": f"wss://{local_ip}:{args.port}",
                            "enable_auth": True, "pass": "iso15118-test-password",
                            "cert_id": csms_tls.cert_id})
        common.api_put(args.charger, "ocpp/config_update", test_config)
        if not csms.connected.wait(timeout=60):
            raise SystemExit("charger did not connect to the embedded CSMS")
        time.sleep(2)

        defaults = csms.call("GetVariables", {"getVariableData": [
            {"component": {"name": "ISO15118Ctrlr"}, "variable": {"name": "ISO15118EvseId"}},
            {"component": {"name": "ISO15118Ctrlr"}, "variable": {"name": "EnforceTlsEnabled"}},
        ]})["getVariableResult"]
        check("ISO15118EvseId default after reset", defaults[0].get("attributeValue") == "ZZ00000",
              defaults[0].get("attributeValue"))
        check("EnforceTlsEnabled default after reset", defaults[1].get("attributeValue") == "false",
              defaults[1].get("attributeValue"))

        common.enable_debug_mode(args.charger)
        time.sleep(2)

        check("store live and empty, no TLS 1.2 handshake",
              try_tls(args.charger, iface, tls13=False) is None)

        for kind, pem in (("V2GRootCertificate", "iso2"), ("V2GRootCertificate", "iso20"),
                          ("OEMRootCertificate", "iso20")):
            name = "v2gRootCACert.pem" if kind.startswith("V2G") else "oemRootCACert.pem"
            res = csms.call("InstallCertificate", {
                "certificateType": kind,
                "certificate": (CERTS / pem / "certs" / name).read_text()})
            assert res["status"] == "Accepted", (kind, pem, res)

        provision_chain(csms, workdir, iso20=False, with_aia=False)

        # Exact +300/+301 boundaries run deterministically on both host crypto
        # backends. Target margins prove the same acceptance/rejection paths
        # without depending on network and certificate timestamp races.
        request = start_v2g_certificate_csr(csms)
        accepted = sign_iso2_csr_at(
            request["csr"], datetime.now(timezone.utc) + timedelta(seconds=120))
        accepted_result = csms.call("CertificateSigned", {
            "certificateChain": accepted[4],
            "certificateType": "V2GCertificate",
            "requestId": request["requestId"],
        }, timeout=90)
        check("SECC chain starting within 300 seconds is accepted [HUB20-42-001]",
              accepted_result["status"] == "Accepted", accepted_result)
        accepted_hash = certificate_hash_data(accepted[0], accepted[1])
        accepted_response, accepted_matches = installed_chain_for_leaf(csms, accepted_hash)
        accepted_entries = accepted_response.get("certificateHashDataChain", [])
        check("accepted future SECC chain replaces the previous same-root chain",
              len(accepted_matches) == 1 and accepted_entries == accepted_matches,
              accepted_entries)

        request = start_v2g_certificate_csr(csms)
        too_early = sign_iso2_csr_at(
            request["csr"], datetime.now(timezone.utc) + timedelta(seconds=600))
        rejected = csms.call("CertificateSigned", {
            "certificateChain": too_early[4],
            "certificateType": "V2GCertificate",
            "requestId": request["requestId"],
        }, timeout=90)
        check("SECC chain starting beyond 300 seconds is rejected [HUB20-42-001]",
              rejected["status"] == "Rejected"
              and rejected.get("statusInfo", {}).get("reasonCode") == "InvalidChain",
              rejected)
        future_response, matches_after_future_rejection = installed_chain_for_leaf(csms, accepted_hash)
        check("future-validity rejection keeps the accepted chain unchanged",
              len(matches_after_future_rejection) == 1
              and future_response.get("certificateHashDataChain", []) == accepted_entries,
              future_response)

        request = start_v2g_certificate_csr(csms)
        includes_root = sign_iso2_csr_at(
            request["csr"], datetime.now(timezone.utc) - timedelta(days=1))
        root_pem = includes_root[3].public_bytes(serialization.Encoding.PEM).decode("ascii")
        rejected = csms.call("CertificateSigned", {
            "certificateChain": includes_root[4] + root_pem,
            "certificateType": "V2GCertificate",
            "requestId": request["requestId"],
        }, timeout=90)
        check("SECC chain including the V2G root is rejected [HUB20-42-004]",
              rejected["status"] == "Rejected"
              and rejected.get("statusInfo", {}).get("reasonCode") == "ChainIncludesRoot",
              rejected)
        root_response, matches_after_root_rejection = installed_chain_for_leaf(csms, accepted_hash)
        check("root-inclusion rejection keeps the accepted chain unchanged",
              len(matches_after_root_rejection) == 1
              and root_response.get("certificateHashDataChain", []) == accepted_entries,
              root_response)

        # Restore a currently valid -2 identity before EV-side TLS checks.
        provision_chain(csms, workdir, iso20=False, with_aia=False)

        expected_evseid = "DE*TNK*E123456"
        set_result = csms.call("SetVariables", {"setVariableData": [{
            "component": {"name": "ISO15118Ctrlr"},
            "variable": {"name": "ISO15118EvseId"},
            "attributeValue": expected_evseid,
        }]})["setVariableResult"][0]
        check("ISO15118EvseId accepted", set_result["attributeStatus"] == "Accepted", set_result)
        common.disable_debug_mode(args.charger)
        time.sleep(1)
        common.enable_debug_mode(args.charger)
        time.sleep(2)

        from iso15118.shared.messages.enums import Namespace
        client = EVTestClient(args.charger, iface)
        with managed_socket(client.connect_tls(client.tls12_context())) as tls:
            sap_res = client.sap(tls, [ISO2])
            assert sap_res["ResponseCode"] == "OK_SuccessfulNegotiation", sap_res
            response = client.exchange(tls, {
                "V2G_Message": {
                    "Header": {"SessionID": "0000000000000000"},
                    "Body": {"SessionSetupReq": {"EVCCID": "020000000001"}},
                },
            }, Namespace.ISO_V2_MSG_DEF)
        actual_evseid = response["V2G_Message"]["Body"]["SessionSetupRes"]["EVSEID"]
        check("SessionSetupRes carries ISO15118EvseId", actual_evseid == expected_evseid, actual_evseid)
        time.sleep(1)

        check("-2 chain live, TLS 1.2 works",
              try_tls(args.charger, iface, tls13=False) == "TLSv1.2")

        chain20 = provision_iso20_chain(csms, workdir)
        listed = csms.call("GetInstalledCertificateIds", {
            "certificateType": ["V2GCertificateChain"],
        })
        check("M03 accepts the installed V2G certificate chains",
              listed["status"] == "Accepted", listed)
        expected_leaf_hash = certificate_hash_data(
            chain20.certificates[0], chain20.issuers[0])
        matching_entries = [
            entry for entry in listed.get("certificateHashDataChain", [])
            if entry["certificateHashData"] == expected_leaf_hash
        ]
        check("M03 includes the ISO 15118-20 SECC leaf hash [M03.FR.04/05]",
              len(matching_entries) == 1, matching_entries)
        if matching_entries:
            expected_children = [
                certificate_hash_data(chain20.certificates[1], chain20.issuers[1]),
                certificate_hash_data(chain20.certificates[2], chain20.issuers[2]),
            ]
            children = matching_entries[0].get("childCertificateHashData", [])
            check("M03 child hashes are CPO Sub-CA 2 then CPO Sub-CA 1 [HUB20-412-001]",
                  children == expected_children, children)
            root_hash = certificate_hash_data(chain20.issuers[2], chain20.issuers[2])
            check("M03 does not list the V2G root as a child certificate",
                  root_hash not in children)
        time.sleep(3)
        check("OCSP unknown, TLS 1.3 refused [HUB20-532-002]",
              try_tls(args.charger, iface, tls13=True, mutual=True) is None)

        assert csms.call("SetVariables", {"setVariableData": [{
            "component": {"name": "ISO15118Ctrlr"}, "variable": {"name": "PrivateEnviromentEnabled"},
            "attributeValue": "true"}]})["setVariableResult"][0]["attributeStatus"] == "Accepted"
        time.sleep(3)
        private_version = try_tls(args.charger, iface, tls13=True, mutual=True)
        if pnc_supported:
            check("private PnC environment still requires SECC OCSP [3.2.9/HUB20-532-002]",
                  private_version is None, private_version)
        else:
            check("private non-PnC environment may waive SECC OCSP [3.2.9]",
                  private_version == "TLSv1.3", private_version)

        # Every transmitted chain certificate has its own AIA URL and exact
        # response. Keep the final request pending long enough to prove that
        # two Good entries are insufficient to open the TLS 1.3 gate.
        ocsp_responses = []
        for index in range(2):
            status_req, msg_id = csms.expect("GetCertificateStatus", timeout=120)
            request_data = status_req["ocspRequestData"]
            check(f"GetCertificateStatus for -20 chain certificate {index}",
                  request_data["responderURL"] == chain20.urls[index]
                  and int(request_data["serialNumber"], 16)
                  == chain20.certificates[index].serial_number,
                  request_data)
            response = chain20.response(index, lifetime=timedelta(minutes=1))
            ocsp_responses.append(response)
            csms.respond(msg_id, {
                "status": "Accepted",
                "ocspResult": base64.b64encode(response).decode("ascii"),
            })
        final_status_req, final_message_id = csms.expect("GetCertificateStatus", timeout=120)
        final_request_data = final_status_req["ocspRequestData"]
        check("GetCertificateStatus for -20 chain certificate 2",
              final_request_data["responderURL"] == chain20.urls[2]
              and int(final_request_data["serialNumber"], 16)
              == chain20.certificates[2].serial_number,
              final_request_data)
        check("missing intermediate OCSP keeps TLS 1.3 unavailable [V2G20-2388]",
              try_tls(args.charger, iface, tls13=True, mutual=True) is None)
        final_response = chain20.response(2, lifetime=timedelta(minutes=1))
        ocsp_responses.append(final_response)
        csms.respond(final_message_id, {
            "status": "Accepted",
            "ocspResult": base64.b64encode(final_response).decode("ascii"),
        })
        time.sleep(5)

        check("OCSP good, TLS 1.3 works in a private PnC environment",
              try_tls(args.charger, iface, tls13=True, mutual=True) == "TLSv1.3")

        assert csms.call("SetVariables", {"setVariableData": [{
            "component": {"name": "ISO15118Ctrlr"}, "variable": {"name": "PrivateEnviromentEnabled"},
            "attributeValue": "false"}]})["setVariableResult"][0]["attributeStatus"] == "Accepted"
        time.sleep(3)
        check("OCSP-good chain remains available after returning to public mode",
              try_tls(args.charger, iface, tls13=True, mutual=True) == "TLSv1.3")
        entries = capture_certificate_entries(args.charger, iface)
        actual_certificates = [certificate for certificate, _ in entries]
        actual_responses = [
            parse_status_request(extensions[TLS_STATUS_REQUEST])
            if TLS_STATUS_REQUEST in extensions else None
            for _, extensions in entries
        ]
        check("public TLS 1.3 sends the full SECC chain without the root [V2G20-2388]",
              actual_certificates == chain20.ders and len(entries) == 3,
              f"captured {len(entries)} CertificateEntry values")
        check("every public TLS 1.3 CertificateEntry carries its exact OCSP response [V2G20-2388]",
              actual_responses == ocsp_responses)

        # Advance the station clock through nextUpdate. The stale Good results
        # and staples must be dropped while replacements are pending.
        csms.current_time_offset_s = NEXT_UPDATE_OFFSET_S
        assert csms.call("TriggerMessage", {"requestedMessage": "Heartbeat"})["status"] == "Accepted"
        time.sleep(5)
        expired_log = trace_log(args.charger)
        check("SECC OCSP cache expires at nextUpdate [HUB20-431-001/V2G20-1021]",
              expired_log.count("OCSP cache expired for chain certificate") >= 3)
        check("expired SECC OCSP forces TLS 1.3 fallback while refresh is pending [HUB20-532-002]",
              try_tls(args.charger, iface, tls13=True, mutual=True) is None)
        check("TLS 1.2 remains available after SECC OCSP expiry",
              try_tls(args.charger, iface, tls13=False) == "TLSv1.2")

        fresh_responses = [None] * len(chain20.certificates)
        refreshed = set()
        for _ in chain20.certificates:
            refresh_req, refresh_msg = csms.expect("GetCertificateStatus", timeout=30)
            request_data = refresh_req["ocspRequestData"]
            index = next(
                index for index, certificate in enumerate(chain20.certificates)
                if certificate.serial_number == int(request_data["serialNumber"], 16)
            )
            check(f"nextUpdate expiry immediately refreshes chain certificate {index}",
                  index not in refreshed and request_data["responderURL"] == chain20.urls[index],
                  request_data)
            refreshed.add(index)
            fresh_response = chain20.response(index, lifetime=timedelta(days=30))
            fresh_responses[index] = fresh_response
            csms.respond(refresh_msg, {
                "status": "Accepted",
                "ocspResult": base64.b64encode(fresh_response).decode("ascii"),
            })
        check("nextUpdate expiry refreshes the complete SECC chain",
              refreshed == set(range(len(chain20.certificates))))
        time.sleep(5)
        check("fresh SECC OCSP restores TLS 1.3 without reboot",
              try_tls(args.charger, iface, tls13=True, mutual=True) == "TLSv1.3")

        # The refreshed responses have nextUpdate 30 days away. They must
        # remain usable immediately before the independent seven-day cap.
        csms.current_time_offset_s = PRE_CAP_OFFSET_S
        assert csms.call("TriggerMessage", {"requestedMessage": "Heartbeat"})["status"] == "Accepted"
        time.sleep(5)
        check("30-day OCSP responses remain valid before the seven-day cap [V2G20-1021]",
              try_tls(args.charger, iface, tls13=True, mutual=True) == "TLSv1.3")
        entries = capture_certificate_entries(args.charger, iface)
        pre_cap_responses = [
            parse_status_request(extensions[TLS_STATUS_REQUEST])
            if TLS_STATUS_REQUEST in extensions else None
            for _, extensions in entries
        ]
        check("exact refreshed staples remain on the wire before the seven-day cap",
              pre_cap_responses == fresh_responses)

        # Cross the cap while nextUpdate is still more than three weeks away.
        # All cached statuses and DER responses must expire and refresh now.
        expiry_count_before_cap = trace_log(args.charger).count(
            "OCSP cache expired for chain certificate")
        csms.current_time_offset_s = POST_CAP_OFFSET_S
        assert csms.call("TriggerMessage", {"requestedMessage": "Heartbeat"})["status"] == "Accepted"
        time.sleep(5)
        cap_log = trace_log(args.charger)
        check("all SECC OCSP entries expire at the seven-day cap [M06.FR.10/HUB20-431-001/V2G20-1021]",
              cap_log.count("OCSP cache expired for chain certificate")
              >= expiry_count_before_cap + len(chain20.certificates))
        check("seven-day cap removes TLS 1.3 staples while refresh is pending [V2G20-1021]",
              try_tls(args.charger, iface, tls13=True, mutual=True) is None)
        check("TLS 1.2 remains available after the seven-day OCSP cap",
              try_tls(args.charger, iface, tls13=False) == "TLSv1.2")

        cap_refreshed = set()
        for _ in chain20.certificates:
            refresh_req, refresh_msg = csms.expect("GetCertificateStatus", timeout=30)
            request_data = refresh_req["ocspRequestData"]
            index = next(
                index for index, certificate in enumerate(chain20.certificates)
                if certificate.serial_number == int(request_data["serialNumber"], 16)
            )
            check(f"seven-day cap immediately refreshes chain certificate {index}",
                  index not in cap_refreshed and request_data["responderURL"] == chain20.urls[index],
                  request_data)
            cap_refreshed.add(index)
            response = chain20.response(index, lifetime=timedelta(days=30))
            csms.respond(refresh_msg, {
                "status": "Accepted",
                "ocspResult": base64.b64encode(response).decode("ascii"),
            })
        check("seven-day cap refreshes the complete SECC chain",
              cap_refreshed == set(range(len(chain20.certificates))))
        time.sleep(5)
        check("fresh status after the seven-day cap restores TLS 1.3",
              try_tls(args.charger, iface, tls13=True, mutual=True) == "TLSv1.3")

        csms.current_time_offset_s = 0
        assert csms.call("TriggerMessage", {"requestedMessage": "Heartbeat"})["status"] == "Accepted"
        time.sleep(2)
    finally:
        try:
            csms.current_time_offset_s = 0
            if csms.connected.is_set():
                csms.call("TriggerMessage", {"requestedMessage": "Heartbeat"})
            disabled = dict(saved_config)
            disabled["enable"] = False
            common.api_put(args.charger, "ocpp/config_update", disabled)
            time.sleep(2)
            common.api_put(args.charger, "ocpp/reset", None)
            common.api_put(args.charger, "ocpp/config_update", saved_config)
        except Exception as e:
            print(f"cleanup failed, restore the ocpp config manually: {e}")
            failures += 1
        csms.stop()
        csms_tls.close()
        shutil.rmtree(workdir, ignore_errors=True)

    print("PASS" if failures == 0 else f"FAIL ({failures} failures)")
    sys.exit(0 if failures == 0 else 1)


if __name__ == "__main__":
    main()
