#!/usr/bin/env python3
"""V2G2-871 ISO 15118-2 certificate-chain selection and OCSP multi.

Provisions two independently rooted P-256 V2G chains through OCPP, then
examines raw TLS 1.2 server flights. This intentionally does not use OpenSSL:
stock OpenSSL clients cannot send trusted_ca_keys and status_request_v2.
"""

from __future__ import annotations

import argparse
import base64
import hashlib
import os
import shutil
import struct
import sys
import tempfile
import time
from dataclasses import dataclass
from datetime import datetime, timedelta, timezone
from pathlib import Path

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509 import ocsp
from cryptography.x509.oid import AuthorityInformationAccessOID, NameOID

import _common as common
from _common import CSMSSim as Csms


OCSP_BASE_URL = "http://ocsp.v2g2-871.test/"
TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 = 0xC023
EXT_TRUSTED_CA_KEYS = 3
EXT_STATUS_REQUEST_V2 = 17


@dataclass
class Chain:
    tag: str
    root_key: ec.EllipticCurvePrivateKey
    root: x509.Certificate
    sub1_key: ec.EllipticCurvePrivateKey
    sub1: x509.Certificate
    sub2_key: ec.EllipticCurvePrivateKey
    sub2: x509.Certificate
    leaf: x509.Certificate | None = None
    ocsp_der: list[bytes] | None = None

    @property
    def certificates(self) -> list[x509.Certificate]:
        assert self.leaf is not None
        return [self.leaf, self.sub2, self.sub1]

    @property
    def ders(self) -> list[bytes]:
        return [cert.public_bytes(serialization.Encoding.DER) for cert in self.certificates]

    @property
    def root_der(self) -> bytes:
        return self.root.public_bytes(serialization.Encoding.DER)


@dataclass
class ServerFlight:
    messages: list[tuple[int, bytes]]
    extensions: dict[int, bytes]
    certificates: list[bytes]


def name(common_name: str) -> x509.Name:
    return x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, "DE"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "V2G2-871 Test"),
        x509.NameAttribute(NameOID.COMMON_NAME, common_name),
    ])


def aia(uri: str) -> x509.AuthorityInformationAccess:
    return x509.AuthorityInformationAccess([
        x509.AccessDescription(
            AuthorityInformationAccessOID.OCSP,
            x509.UniformResourceIdentifier(uri),
        )
    ])


def ca_key_usage() -> x509.KeyUsage:
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


def leaf_key_usage() -> x509.KeyUsage:
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


def signed_ca(
    subject: x509.Name,
    key: ec.EllipticCurvePrivateKey,
    issuer_cert: x509.Certificate,
    issuer_key: ec.EllipticCurvePrivateKey,
    path_length: int,
    ocsp_uri: str,
    not_before: datetime,
    not_after: datetime,
) -> x509.Certificate:
    return (
        x509.CertificateBuilder()
        .subject_name(subject)
        .issuer_name(issuer_cert.subject)
        .public_key(key.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(not_before)
        .not_valid_after(not_after)
        .add_extension(x509.BasicConstraints(ca=True, path_length=path_length), critical=True)
        .add_extension(ca_key_usage(), critical=True)
        .add_extension(x509.SubjectKeyIdentifier.from_public_key(key.public_key()), critical=False)
        .add_extension(x509.AuthorityKeyIdentifier.from_issuer_public_key(issuer_key.public_key()), critical=False)
        .add_extension(aia(ocsp_uri), critical=False)
        .sign(issuer_key, hashes.SHA256())
    )


def build_chain(workdir: Path, tag: str) -> Chain:
    directory = workdir / tag
    directory.mkdir()
    now = datetime.now(timezone.utc)
    not_before = now - timedelta(days=1)
    not_after = now + timedelta(days=60)
    ocsp_uri = OCSP_BASE_URL + tag

    root_key = ec.generate_private_key(ec.SECP256R1())
    root_name = name(f"V2G Root {tag}")
    root = (
        x509.CertificateBuilder()
        .subject_name(root_name)
        .issuer_name(root_name)
        .public_key(root_key.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(not_before)
        .not_valid_after(not_after)
        .add_extension(x509.BasicConstraints(ca=True, path_length=2), critical=True)
        .add_extension(ca_key_usage(), critical=True)
        .add_extension(x509.SubjectKeyIdentifier.from_public_key(root_key.public_key()), critical=False)
        .add_extension(x509.AuthorityKeyIdentifier.from_issuer_public_key(root_key.public_key()), critical=False)
        .sign(root_key, hashes.SHA256())
    )
    sub1_key = ec.generate_private_key(ec.SECP256R1())
    sub1 = signed_ca(name(f"CPO SubCA 1 {tag}"), sub1_key, root, root_key, 1,
                     ocsp_uri, not_before, not_after)
    sub2_key = ec.generate_private_key(ec.SECP256R1())
    sub2 = signed_ca(name(f"CPO SubCA 2 {tag}"), sub2_key, sub1, sub1_key, 0,
                     ocsp_uri, not_before, not_after)
    chain = Chain(tag, root_key, root, sub1_key, sub1, sub2_key, sub2)

    for cert_name, cert in (("root", root), ("sub1", sub1), ("sub2", sub2)):
        (directory / f"{cert_name}.pem").write_bytes(cert.public_bytes(serialization.Encoding.PEM))
    for key_name, key in (("root", root_key), ("sub1", sub1_key), ("sub2", sub2_key)):
        (directory / f"{key_name}.key").write_bytes(key.private_bytes(
            serialization.Encoding.PEM,
            serialization.PrivateFormat.PKCS8,
            serialization.NoEncryption(),
        ))
    return chain


def sign_device_csr(chain: Chain, csr_pem: str, workdir: Path) -> None:
    csr = x509.load_pem_x509_csr(csr_pem.encode())
    assert csr.is_signature_valid, f"invalid device CSR for chain {chain.tag}"
    public_key = csr.public_key()
    assert isinstance(public_key, ec.EllipticCurvePublicKey)
    assert isinstance(public_key.curve, ec.SECP256R1), public_key.curve.name
    now = datetime.now(timezone.utc)
    chain.leaf = (
        x509.CertificateBuilder()
        .subject_name(csr.subject)
        .issuer_name(chain.sub2.subject)
        .public_key(public_key)
        .serial_number(x509.random_serial_number())
        .not_valid_before(now - timedelta(days=1))
        .not_valid_after(now + timedelta(days=60))
        .add_extension(x509.BasicConstraints(ca=False, path_length=None), critical=True)
        .add_extension(leaf_key_usage(), critical=True)
        .add_extension(x509.SubjectKeyIdentifier.from_public_key(public_key), critical=False)
        .add_extension(x509.AuthorityKeyIdentifier.from_issuer_public_key(chain.sub2_key.public_key()), critical=False)
        .add_extension(aia(OCSP_BASE_URL + chain.tag), critical=False)
        .sign(chain.sub2_key, hashes.SHA256())
    )
    (workdir / chain.tag / "leaf.pem").write_bytes(
        chain.leaf.public_bytes(serialization.Encoding.PEM)
    )


def good_ocsp(
    certificate: x509.Certificate,
    issuer: x509.Certificate,
    issuer_key: ec.EllipticCurvePrivateKey,
) -> bytes:
    now = datetime.now(timezone.utc)
    response = (
        ocsp.OCSPResponseBuilder()
        .add_response(
            cert=certificate,
            issuer=issuer,
            algorithm=hashes.SHA256(),
            cert_status=ocsp.OCSPCertStatus.GOOD,
            this_update=now,
            next_update=now + timedelta(days=7),
            revocation_time=None,
            revocation_reason=None,
        )
        .responder_id(ocsp.OCSPResponderEncoding.HASH, issuer)
        .certificates([issuer])
        .sign(issuer_key, hashes.SHA256())
    )
    return response.public_bytes(serialization.Encoding.DER)


def make_ocsp_responses(chain: Chain, workdir: Path) -> None:
    assert chain.leaf is not None
    chain.ocsp_der = [
        good_ocsp(chain.leaf, chain.sub2, chain.sub2_key),
        good_ocsp(chain.sub2, chain.sub1, chain.sub1_key),
        good_ocsp(chain.sub1, chain.root, chain.root_key),
    ]
    for index, response in enumerate(chain.ocsp_der):
        (workdir / chain.tag / f"ocsp-{index}.der").write_bytes(response)


def pem(cert: x509.Certificate) -> str:
    return cert.public_bytes(serialization.Encoding.PEM).decode("ascii")


def provision_chain(csms: Csms, chain: Chain, workdir: Path) -> None:
    trigger = csms.call("TriggerMessage", {"requestedMessage": "SignV2GCertificate"})
    assert trigger["status"] == "Accepted", trigger
    request, message_id = csms.expect("SignCertificate", timeout=60)
    csms.respond(message_id, {"status": "Accepted"})
    assert request["certificateType"] == "V2GCertificate", request
    sign_device_csr(chain, request["csr"], workdir)
    make_ocsp_responses(chain, workdir)

    result = csms.call("CertificateSigned", {
        "certificateChain": "".join(pem(cert) for cert in chain.certificates),
        "certificateType": "V2GCertificate",
        "requestId": request["requestId"],
    }, timeout=90)
    assert result["status"] == "Accepted", result

    assert chain.ocsp_der is not None
    responses = {cert.serial_number: der for cert, der in zip(chain.certificates, chain.ocsp_der)}
    requested = set()
    for _ in range(3):
        status_request, status_message_id = csms.expect("GetCertificateStatus", timeout=120)
        request_data = status_request["ocspRequestData"]
        serial = int(request_data["serialNumber"], 16)
        assert serial in responses, (chain.tag, request_data)
        assert serial not in requested, (chain.tag, request_data)
        assert request_data["responderURL"] == OCSP_BASE_URL + chain.tag, request_data
        requested.add(serial)
        csms.respond(status_message_id, {
            "status": "Accepted",
            "ocspResult": base64.b64encode(responses[serial]).decode("ascii"),
        })
    assert requested == set(responses), (chain.tag, requested, set(responses))


def u24(value: int) -> bytes:
    assert 0 <= value <= 0xFFFFFF
    return value.to_bytes(3, "big")


def read_u24(data: bytes, offset: int) -> int:
    if offset + 3 > len(data):
        raise ValueError("truncated uint24")
    return int.from_bytes(data[offset:offset + 3], "big")


def extension(extension_type: int, data: bytes) -> bytes:
    return struct.pack("!HH", extension_type, len(data)) + data


def trusted_ca_keys(identifier: bytes) -> bytes:
    return struct.pack("!H", len(identifier)) + identifier


def status_request_v2() -> bytes:
    ocsp_multi_request = b"\x00\x00\x00\x00"  # empty responder IDs and request extensions
    item = b"\x02" + struct.pack("!H", len(ocsp_multi_request)) + ocsp_multi_request
    return struct.pack("!H", len(item)) + item


def client_hello(trusted: bytes | None, request_status_v2: bool) -> bytes:
    extensions = [
        extension(10, b"\x00\x02\x00\x17"),  # supported_groups: secp256r1
        extension(11, b"\x01\x00"),          # ec_point_formats: uncompressed
        extension(13, b"\x00\x02\x04\x03"), # signature_algorithms: ecdsa_secp256r1_sha256
    ]
    optional = []
    if request_status_v2:
        optional.append(extension(EXT_STATUS_REQUEST_V2, status_request_v2()))
    if trusted is not None:
        optional.append(extension(EXT_TRUSTED_CA_KEYS, trusted_ca_keys(trusted)))
    extensions.extend(optional)  # status_request_v2 deliberately precedes trusted_ca_keys
    extension_block = b"".join(extensions)
    body = (
        b"\x03\x03" + os.urandom(32) + b"\x00"
        + b"\x00\x02" + struct.pack("!H", TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256)
        + b"\x01\x00"
        + struct.pack("!H", len(extension_block)) + extension_block
    )
    handshake = b"\x01" + u24(len(body)) + body
    return b"\x16\x03\x01" + struct.pack("!H", len(handshake)) + handshake


def recv_exact(sock, size: int) -> bytes:
    data = bytearray()
    while len(data) < size:
        chunk = sock.recv(size - len(data))
        if not chunk:
            raise ConnectionError(f"TLS connection closed after {len(data)} of {size} bytes")
        data.extend(chunk)
    return bytes(data)


def parse_server_hello(body: bytes) -> dict[int, bytes]:
    if len(body) < 38:
        raise ValueError("truncated ServerHello")
    assert body[:2] == b"\x03\x03", body[:2].hex()
    offset = 34
    session_id_len = body[offset]
    offset += 1 + session_id_len
    if offset + 3 > len(body):
        raise ValueError("truncated ServerHello session or cipher suite")
    cipher_suite = int.from_bytes(body[offset:offset + 2], "big")
    assert cipher_suite == TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256, hex(cipher_suite)
    offset += 2
    assert body[offset] == 0, body[offset]
    offset += 1
    if offset == len(body):
        return {}
    if offset + 2 > len(body):
        raise ValueError("truncated ServerHello extensions length")
    total = int.from_bytes(body[offset:offset + 2], "big")
    offset += 2
    assert offset + total == len(body), (offset, total, len(body))
    extensions = {}
    end = offset + total
    while offset < end:
        if offset + 4 > end:
            raise ValueError("truncated ServerHello extension")
        extension_type, length = struct.unpack("!HH", body[offset:offset + 4])
        offset += 4
        if offset + length > end:
            raise ValueError("truncated ServerHello extension data")
        assert extension_type not in extensions, extension_type
        extensions[extension_type] = body[offset:offset + length]
        offset += length
    return extensions


def parse_certificate(body: bytes) -> list[bytes]:
    if len(body) < 3:
        raise ValueError("truncated Certificate message")
    total = read_u24(body, 0)
    assert total == len(body) - 3, (total, len(body))
    certificates = []
    offset = 3
    while offset < len(body):
        length = read_u24(body, offset)
        offset += 3
        if length == 0 or offset + length > len(body):
            raise ValueError("invalid TLS certificate-list entry")
        certificates.append(body[offset:offset + length])
        offset += length
    return certificates


def receive_server_flight(sock) -> ServerFlight:
    messages = []
    handshake_data = bytearray()
    while True:
        header = recv_exact(sock, 5)
        content_type, major, minor, length = struct.unpack("!BBBH", header)
        assert major == 3 and minor <= 3, (major, minor)
        assert length <= 18432, length
        payload = recv_exact(sock, length)
        if content_type == 21:
            alerts = [tuple(payload[i:i + 2]) for i in range(0, len(payload), 2)]
            raise RuntimeError(f"TLS alert before ServerHelloDone: {alerts}")
        if content_type != 22:
            continue
        handshake_data.extend(payload)
        while len(handshake_data) >= 4:
            message_length = read_u24(handshake_data, 1)
            if len(handshake_data) < 4 + message_length:
                break
            message_type = handshake_data[0]
            body = bytes(handshake_data[4:4 + message_length])
            del handshake_data[:4 + message_length]
            messages.append((message_type, body))
            if message_type == 14:
                server_hellos = [item for item in messages if item[0] == 2]
                certificate_messages = [item for item in messages if item[0] == 11]
                assert len(server_hellos) == 1, [item[0] for item in messages]
                assert len(certificate_messages) == 1, [item[0] for item in messages]
                return ServerFlight(
                    messages,
                    parse_server_hello(server_hellos[0][1]),
                    parse_certificate(certificate_messages[0][1]),
                )


def probe(charger: str, iface: str, trusted: bytes | None = None,
          request_status_v2: bool = False) -> ServerFlight:
    sock = common.connect_secc(charger, iface)
    try:
        sock.settimeout(30)
        sock.sendall(client_hello(trusted, request_status_v2))
        return receive_server_flight(sock)
    finally:
        sock.close()
        time.sleep(1)


def assert_base_flight(flight: ServerFlight, chains: list[Chain]) -> None:
    assert len(flight.certificates) == 3, len(flight.certificates)
    assert flight.certificates in [chain.ders for chain in chains]
    roots = [chain.root_der for chain in chains]
    assert not any(cert in roots for cert in flight.certificates), "root transmitted in TLS Certificate"


def assert_no_status(flight: ServerFlight) -> None:
    assert EXT_STATUS_REQUEST_V2 not in flight.extensions, flight.extensions
    assert 22 not in [message_type for message_type, _ in flight.messages], [
        message_type for message_type, _ in flight.messages
    ]


def assert_trusted_ack(flight: ServerFlight) -> None:
    assert flight.extensions.get(EXT_TRUSTED_CA_KEYS) == b"", flight.extensions


def parse_ocsp_multi(body: bytes) -> list[bytes]:
    if len(body) < 4:
        raise ValueError("truncated CertificateStatus")
    assert body[0] == 2, body[0]
    total = read_u24(body, 1)
    assert total == len(body) - 4, (total, len(body))
    responses = []
    offset = 4
    while offset < len(body):
        length = read_u24(body, offset)
        offset += 3
        if length == 0 or offset + length > len(body):
            raise ValueError("invalid OCSPResponseList entry")
        responses.append(body[offset:offset + length])
        offset += length
    return responses


def chain_for_der(chains: list[Chain], ders: list[bytes]) -> Chain:
    return next(chain for chain in chains if chain.ders == ders)


def run_checks(charger: str, iface: str, chains: list[Chain]) -> None:
    baseline = probe(charger, iface)
    assert_base_flight(baseline, chains)
    assert EXT_TRUSTED_CA_KEYS not in baseline.extensions, baseline.extensions
    assert_no_status(baseline)
    default = chain_for_der(chains, baseline.certificates)
    target = next(chain for chain in chains if chain is not default)
    print(f"ok  default chain discovered as {default.tag}; target is {target.tag}")

    target_root_point = target.root.public_key().public_bytes(
        serialization.Encoding.X962,
        serialization.PublicFormat.UncompressedPoint,
    )
    target_identifiers = [
        ("cert_sha1_hash", b"\x03" + hashlib.sha1(target.root_der).digest()),
        ("key_sha1_hash", b"\x01" + hashlib.sha1(target_root_point).digest()),
        ("x509_name", b"\x02" + struct.pack("!H", len(target.root.subject.public_bytes()))
         + target.root.subject.public_bytes()),
    ]
    for identifier_name, identifier in target_identifiers:
        flight = probe(charger, iface, trusted=identifier)
        assert_base_flight(flight, chains)
        assert flight.certificates == target.ders, identifier_name
        assert_trusted_ack(flight)
        assert_no_status(flight)
        print(f"ok  {identifier_name} selected exact target chain without status v2")

    pre_agreed = probe(charger, iface, trusted=b"\x00")
    assert_base_flight(pre_agreed, chains)
    assert pre_agreed.certificates == default.ders
    assert_trusted_ack(pre_agreed)
    assert_no_status(pre_agreed)
    print("ok  pre_agreed selected and acknowledged the default chain")

    installed_root_hashes = {hashlib.sha1(chain.root_der).digest() for chain in chains}
    no_match_hash = os.urandom(20)
    while no_match_hash in installed_root_hashes:
        no_match_hash = os.urandom(20)
    no_match = probe(charger, iface, trusted=b"\x03" + no_match_hash)
    assert_base_flight(no_match, chains)
    assert_trusted_ack(no_match)
    assert_no_status(no_match)
    print("ok  unmatched cert hash completed with an installed root-excluded fallback chain")

    status_flight = probe(
        charger,
        iface,
        trusted=b"\x03" + hashlib.sha1(target.root_der).digest(),
        request_status_v2=True,
    )
    assert_base_flight(status_flight, chains)
    assert status_flight.certificates == target.ders
    assert_trusted_ack(status_flight)
    assert status_flight.extensions.get(EXT_STATUS_REQUEST_V2) == b"", status_flight.extensions
    message_types = [message_type for message_type, _ in status_flight.messages]
    certificate_index = message_types.index(11)
    assert message_types[certificate_index + 1] == 22, message_types
    status_messages = [body for message_type, body in status_flight.messages if message_type == 22]
    assert len(status_messages) == 1, message_types
    assert target.ocsp_der is not None
    assert parse_ocsp_multi(status_messages[0]) == target.ocsp_der
    print("ok  status_request_v2 before trusted_ca_keys returned three exact ordered OCSP responses")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--charger", required=True)
    parser.add_argument("--iface")
    parser.add_argument("--port", type=int, default=9500)
    args = parser.parse_args()

    common.require_iso_tls_config(args.charger)
    iface = args.iface or common.route_interface(args.charger)
    local_ip = common.local_ip_towards(args.charger)
    saved_config = common.api_get(args.charger, "ocpp/config")
    saved_debug = common.api_get(args.charger, "iso15118/debug")
    workdir = Path(tempfile.mkdtemp(prefix="iso2_chain_selection_"))
    csms_tls = None
    csms = None
    cleanup_errors = []

    try:
        chains = [build_chain(workdir, "A"), build_chain(workdir, "B")]
        csms_tls = common.LocalCSMSTls(args.charger, local_ip)
        csms = Csms(
            args.port,
            interactive=("SignCertificate", "GetCertificateStatus"),
            certfile=str(csms_tls.certfile),
            keyfile=str(csms_tls.keyfile),
        )

        # Reset makes the live OCPP store authoritative and isolates the two test chains.
        common.api_put(args.charger, "ocpp/reset", None)
        test_config = dict(saved_config)
        test_config.update({
            "enable": True,
            "protocol": 1,
            "url": f"wss://{local_ip}:{args.port}",
            "enable_auth": True,
            "pass": "iso15118-test-password",
            "cert_id": csms_tls.cert_id,
        })
        common.api_put(args.charger, "ocpp/config_update", test_config)
        if not csms.connected.wait(timeout=60):
            raise RuntimeError("charger did not connect to the embedded CSMS")
        time.sleep(2)

        common.enable_debug_mode(args.charger)
        public_result = csms.call("SetVariables", {"setVariableData": [{
            "component": {"name": "ISO15118Ctrlr"},
            "variable": {"name": "PrivateEnviromentEnabled"},
            "attributeValue": "false",
        }]})["setVariableResult"][0]
        assert public_result["attributeStatus"] == "Accepted", public_result

        for chain in chains:
            result = csms.call("InstallCertificate", {
                "certificateType": "V2GRootCertificate",
                "certificate": pem(chain.root),
            })
            assert result["status"] == "Accepted", (chain.tag, result)
        for chain in chains:
            provision_chain(csms, chain, workdir)
            print(f"ok  provisioned chain {chain.tag} and all three Good OCSP responses")

        time.sleep(5)
        run_checks(args.charger, iface, chains)
        print("PASS")
    finally:
        # This mirrors the certificate tests: discard the isolated test store,
        # restore the exact OCPP configuration, and restore the debug state.
        try:
            disabled = dict(saved_config)
            disabled["enable"] = False
            common.api_put(args.charger, "ocpp/config_update", disabled)
            time.sleep(2)
            common.api_put(args.charger, "ocpp/reset", None)
            common.api_put(args.charger, "ocpp/config_update", saved_config)
        except Exception as error:
            cleanup_errors.append(f"OCPP config/store: {error}")
        try:
            common.api_put(args.charger, "iso15118/debug_update", saved_debug)
        except Exception as error:
            cleanup_errors.append(f"ISO 15118 debug state: {error}")
        if csms is not None:
            try:
                csms.stop()
            except Exception as error:
                cleanup_errors.append(f"CSMS: {error}")
        if csms_tls is not None:
            try:
                csms_tls.close()
            except Exception as error:
                cleanup_errors.append(f"CSMS TLS certificate: {error}")
        shutil.rmtree(workdir, ignore_errors=True)
        if cleanup_errors:
            print("cleanup failed: " + "; ".join(cleanup_errors), file=sys.stderr)
            if sys.exc_info()[0] is None:
                raise RuntimeError("; ".join(cleanup_errors))


if __name__ == "__main__":
    main()
