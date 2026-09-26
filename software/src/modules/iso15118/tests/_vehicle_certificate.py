#!/usr/bin/env python3
"""Wire regressions for the production vehicle-role / OCSP-source policy.

Synthetic current-time fixtures deliberately isolate policy checks; these are
not complete Annex B or audit certificates. No charger or external PKI is used.
"""
from datetime import datetime, timedelta, timezone
from pathlib import Path
import socket
import ssl
import subprocess
import sys
import tempfile
import importlib.util

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec, ed448
from cryptography.x509.oid import AuthorityInformationAccessOID as AIA, ExtendedKeyUsageOID as EKU, NameOID

URL = "https://ocsp.example/status"
NOW = datetime.now(timezone.utc)


def evccid(unique="123456789ABCDEF", wmi="WVW", *, separators=False):
    # V2G20-2095 / C.6: decimal expansion, then positional modulo-11.
    elements = [wmi, "V", unique]
    significant = "".join(element.lstrip("0") for element in elements).upper()
    digits = "".join(str(int(char, 36)) for char in significant)
    check = sum(int(digit) * 2 ** (index % 28) for index, digit in enumerate(digits)) % 11
    return ("-" if separators else "").join(elements + ["X" if check == 10 else str(check)])


def name(cn, role):
    attrs = [x509.NameAttribute(NameOID.ORGANIZATION_NAME, "Vehicle Test"),
             x509.NameAttribute(NameOID.COMMON_NAME, cn)]
    for dc in role or []:
        attrs.append(x509.NameAttribute(NameOID.DOMAIN_COMPONENT, dc))
    return x509.Name(attrs)


def key_identifier(key, method=2):
    # V2G20-3431/3432: RFC 5280 section 4.2.1.2 methods 1 and 2.
    # Works for EC and Ed448 keys.
    digest = x509.SubjectKeyIdentifier.from_public_key(key.public_key()).digest
    if method == 1:
        return digest
    return bytes([(digest[-8] & 0x0f) | 0x40]) + digest[-7:]


def issue(subject, key, issuer, issuer_key, *, ca=False, path_length=None,
          source=URL, eku=None, usage=True, constraints=True, digital=True,
          key_cert_sign=None, expired=False, future=False, method=AIA.OCSP,
          critical_aia=False, key_agreement=False, signing_hash=None,
          ski=True, aki=True, critical_ids=False, crl_sign=False, identifier_method=2,
          content_commitment=False, key_encipherment=False, data_encipherment=False,
          encipher_only=False, decipher_only=False, extra_extensions=(), critical_eku=True,
          not_before=None, not_after=None, critical_constraints=True, critical_usage=True,
          critical_ski=None, critical_aki=None):
    builder = (x509.CertificateBuilder().subject_name(subject).issuer_name(issuer)
               .public_key(key.public_key()).serial_number(x509.random_serial_number())
               .not_valid_before(not_before if not_before is not None else NOW + timedelta(days=1) if future else NOW - timedelta(days=2))
               .not_valid_after(not_after if not_after is not None else NOW - timedelta(days=1) if expired else NOW + timedelta(days=30)))
    if constraints:
        builder = builder.add_extension(x509.BasicConstraints(ca, path_length), critical_constraints)
    if usage:
        builder = builder.add_extension(x509.KeyUsage(digital, content_commitment, key_encipherment, data_encipherment, key_agreement,
                                                     ca if key_cert_sign is None else key_cert_sign,
                                                     crl_sign, encipher_only, decipher_only), critical_usage)
    if ski is not False:
        builder = builder.add_extension(x509.SubjectKeyIdentifier(key_identifier(key, identifier_method) if ski is True else ski), critical_ids if critical_ski is None else critical_ski)
    if aki is not False:
        builder = builder.add_extension(x509.AuthorityKeyIdentifier(
            key_identifier(issuer_key, identifier_method) if aki is True else aki, None, None), critical_ids if critical_aki is None else critical_aki)
    if eku is not None:
        builder = builder.add_extension(x509.ExtendedKeyUsage(eku), critical_eku)
    if source == "crl":
        builder = builder.add_extension(x509.CRLDistributionPoints([
            x509.DistributionPoint([x509.UniformResourceIdentifier("https://crl.example/list")],
                                   None, None, None)]), False)
    elif source is not None:
        urls = source if isinstance(source, list) else [source]
        builder = builder.add_extension(x509.AuthorityInformationAccess([
            x509.AccessDescription(method, x509.UniformResourceIdentifier(url)) for url in urls
        ]), critical_aia)
    for extension, critical in extra_extensions:
        builder = builder.add_extension(extension, critical)
    return builder.sign(issuer_key, None if isinstance(issuer_key, ed448.Ed448PrivateKey) else signing_hash or hashes.SHA512())


def pem(cert):
    return cert.public_bytes(serialization.Encoding.PEM)


def write_key(path, key):
    path.write_bytes(key.private_bytes(serialization.Encoding.PEM,
                                     serialization.PrivateFormat.PKCS8,
                                     serialization.NoEncryption()))


def run(server):
    with tempfile.TemporaryDirectory(prefix="vehicle-policy-") as tmp:
        work = Path(tmp)
        keys = [ec.generate_private_key(ec.SECP521R1()) for _ in range(5)]
        root_key, sub1_key, sub2_key, leaf_key, server_key = keys
        root_name = name("OEM root", None)
        sub1_name, sub2_name, leaf_name = [name(cn, ["EV"]) for cn in ["Sub1", "Sub2", evccid()]]
        root = issue(root_name, root_key, root_name, root_key, ca=True, path_length=2, source=None)
        server_name = name("localhost", None)
        server_cert = issue(server_name, server_key, root_name, root_key, eku=[EKU.SERVER_AUTH], source=None)
        (work / "root.pem").write_bytes(pem(root))
        (work / "server.pem").write_bytes(pem(server_cert))
        write_key(work / "server.key", server_key)
        write_key(work / "client.key", leaf_key)

        def chain(tag, *, leaf_opts=None, sub1_opts=None, sub2_opts=None, leaf_role=("EV",),
                  sub1_role=("EV",), sub2_role=("EV",), forged=False):
            s1_name, s2_name = name("Sub1", sub1_role), name("Sub2", sub2_role)
            s1 = issue(s1_name, sub1_key, root_name,
                       ec.generate_private_key(ec.SECP521R1()) if forged else root_key,
                       **(dict(ca=True, path_length=1) | (sub1_opts or {})))
            s2 = issue(s2_name, sub2_key, s1_name, sub1_key,
                       **(dict(ca=True, path_length=0) | (sub2_opts or {})))
            leaf = issue(name(evccid(), leaf_role), leaf_key, s2_name, sub2_key,
                         **(dict(eku=[EKU.CLIENT_AUTH]) | (leaf_opts or {})))
            path = work / f"{tag}.pem"
            path.write_bytes(pem(leaf) + pem(s2) + pem(s1))
            return path

        def connect(port, cert):
            context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
            context.minimum_version = context.maximum_version = ssl.TLSVersion.TLSv1_3
            context.load_verify_locations(str(work / "root.pem"))
            if cert is not None:
                context.load_cert_chain(str(cert), str(work / "client.key"))
            try:
                with socket.create_connection(("127.0.0.1", port), timeout=5) as raw:
                    with context.wrap_socket(raw, server_hostname="localhost") as tls:
                        tls.sendall(b"ping")
                        reply = tls.recv(4)
                        assert reply == b"pong", f"Missing authenticated response/alert: {reply!r}"
                        return True
            except ssl.SSLError as exc:
                assert "ALERT" in str(exc).upper(), f"Unreadable rejection: {exc}"
                return False

        count = 0

        def scenario(tag, certs, expected, require=True, expected_url=URL):
            nonlocal count
            proc = subprocess.Popen([server, str(work / "server.pem"), str(work / "server.key"),
                                     str(work / "root.pem"), str(int(require)), str(len(certs))],
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            try:
                line = proc.stdout.readline()
                assert line.startswith("PORT "), (tag, line, proc.communicate(timeout=10))
                results = [connect(int(line.split()[1]), cert) for cert in certs]
                out, err = proc.communicate(timeout=10)
                assert proc.returncode == 0 and results == expected, (tag, results, out, err)
                assert out.count("COMPLETE") == sum(expected), (tag, out)
                assert out.count("REJECTED") == len(expected) - sum(expected), (tag, out)
                if require and any(expected):
                    for depth in range(3):
                        expected_source = expected_url if expected_url is not None else "http://ocsp.vehicle.test/vehicle"
                        assert f"SOURCE {depth} {expected_source}" in out, (tag, out)
                count += 1
                print(f"PASS {tag}")
            finally:
                if proc.poll() is None:
                    proc.kill()
                    proc.communicate()

        good = chain("valid")
        scenario("valid full chain", [good], [True])
        scenario("mandatory EKU omitted", [chain("no-eku", leaf_opts={"eku": None})], [False])
        scenario("client and server EKU", [chain("both-eku", leaf_opts={"eku": [EKU.CLIENT_AUTH, EKU.SERVER_AUTH]})], [True])
        scenario("role suffix", [chain("suffix", leaf_role=("ManufacturerEV",), sub1_role=("MakerEV",))], [True])
        for tag, opts in [
            ("contract leaf", {"leaf_role": ("MSP",)}),
            ("provisioning leaf", {"leaf_role": ("OEM",)}),
            ("contract sub1", {"sub1_role": ("MSP",)}),
            ("contract sub2", {"sub2_role": ("MSP",)}),
            ("missing leaf role", {"leaf_role": ()}),
            ("missing sub role", {"sub1_role": ()}),
            ("ambiguous role", {"leaf_role": ("EV", "MSP")}),
            ("server-only EKU", {"leaf_opts": {"eku": [EKU.SERVER_AUTH]}}),
            ("any EKU", {"leaf_opts": {"eku": [EKU.ANY_EXTENDED_KEY_USAGE]}}),
            ("missing usage", {"leaf_opts": {"usage": False}}),
            ("wrong usage", {"leaf_opts": {"digital": False, "key_agreement": True}}),
            ("leaf signs certificates", {"leaf_opts": {"key_cert_sign": True}}),
            ("missing constraints", {"leaf_opts": {"constraints": False}}),
            ("CA leaf", {"leaf_opts": {"ca": True}}),
            ("expired leaf", {"leaf_opts": {"expired": True}}),
            ("future leaf", {"leaf_opts": {"future": True}}),
            ("expired intermediate", {"sub1_opts": {"expired": True}}),
            ("wrong path length", {"sub1_opts": {"path_length": 0}}),
            ("forged chain", {"forged": True}),
            ("missing leaf source", {"leaf_opts": {"source": None}}),
            ("missing sub1 source", {"sub1_opts": {"source": None}}),
            ("missing sub2 source", {"sub2_opts": {"source": None}}),
            ("CRL-only unsupported", {"leaf_opts": {"source": "crl"}}),
            ("caIssuers is not OCSP", {"leaf_opts": {"method": AIA.CA_ISSUERS}}),
            ("empty URL", {"leaf_opts": {"source": ""}}),
            ("embedded NUL", {"leaf_opts": {"source": "https://ocsp.example/\x00tail"}}),
            ("unsupported URL scheme", {"leaf_opts": {"source": "ldap://ocsp.example/status"}}),
            ("missing authority", {"leaf_opts": {"source": "https:///status"}}),
            ("overlong URL", {"leaf_opts": {"source": "https://ocsp.example/" + "x" * 256}}),
        ]:
            scenario(tag, [chain(tag, **opts)], [False])
        scenario("no vehicle certificate", [None], [False])
        url255 = "https://ocsp.example/" + "x" * (255 - len("https://ocsp.example/"))
        scenario("255-byte URL", [chain("url255", leaf_opts={"source": url255},
                                        sub1_opts={"source": url255}, sub2_opts={"source": url255})],
                 [True], expected_url=url255)
        scenario("select usable OCSP alternative", [chain("alternatives", leaf_opts={"source": ["ldap://invalid", URL]})], [True])
        no_source = chain("waiver", leaf_opts={"source": None}, sub1_opts={"source": None}, sub2_opts={"source": None})
        scenario("private non-PnC source waiver", [no_source], [True], require=False)
        scenario("waiver does not waive role", [chain("waiver-role", leaf_role=("MSP",))], [False], require=False)
        scenario("waiver does not waive validity", [chain("waiver-time", leaf_opts={"expired": True})], [False], require=False)
        # Keep identical leaf DER, change only the signature of its topmost CA.
        forged = chain("forged-after-valid", forged=True)
        good_parts = good.read_bytes().split(b"-----BEGIN CERTIFICATE-----")
        forged_parts = forged.read_bytes().split(b"-----BEGIN CERTIFICATE-----")
        forged.write_bytes(b"-----BEGIN CERTIFICATE-----".join(good_parts[:3] + forged_parts[3:]))
        scenario("same leaf changed intermediate then recovery", [good, forged, good], [True, False, True])
        # Isolate the actual worker: host TLS's normal signature verification
        # must not be the only reason that the forged intermediate is rejected.
        for path, expected in [(good, "VALID"), (forged, "REJECTED")]:
            result = subprocess.run([server, "verify-chain", str(path), str(work / "root.pem")],
                                    capture_output=True, text=True, check=True)
            assert result.stdout.strip() == expected, result
        print("PASS isolated production signature worker (valid/forged)")

        # Exercise the bench fixture generator against a fresh existing OEM root.
        spec = importlib.util.spec_from_file_location("vehicle_fixtures",
            Path(__file__).resolve().parents[1] / "tools/certs/generate_vehicle_certs.py")
        fixtures = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(fixtures)
        (work / "certs").mkdir()
        (work / "private_keys").mkdir()
        (work / "certs/oemRootCACert.pem").write_bytes(pem(root))
        (work / "private_keys/oemRootCA.key").write_bytes(root_key.private_bytes(
            serialization.Encoding.PEM, serialization.PrivateFormat.PKCS8,
            serialization.BestAvailableEncryption(b"12345")))
        fixtures.generate(work, b"12345")
        fixture_key = serialization.load_pem_private_key((work / "private_keys/vehicleLeaf.key").read_bytes(), b"12345")
        write_key(work / "client.key", fixture_key)
        scenario("generated bench vehicle identity", [work / "certs/vehicleCertChain.pem"], [True], expected_url=None)
        write_key(work / "client.key", leaf_key)

        # Feed extension DER directly to the same parser used by TLS and M07.
        def der(tag, content):
            length = len(content)
            size = bytes([length]) if length < 128 else bytes([0x82, length >> 8, length & 255])
            return bytes([tag]) + size + content

        aia_oid = der(6, bytes.fromhex("2b06010505070101"))
        ocsp_oid = der(6, bytes.fromhex("2b06010505073001"))
        desc = der(0x30, ocsp_oid + der(0x86, URL.encode()))
        ext = der(0x30, aia_oid + der(4, der(0x30, desc)))
        valid_ext = der(0x30, ext)
        parser_cases = [("valid", valid_ext, URL),
                        ("duplicate AIA", der(0x30, ext + ext), "REJECTED"),
                        ("trailing outer data", valid_ext + b"\0", "REJECTED"),
                        ("trailing access data", der(0x30, der(0x30, aia_oid + der(4, der(0x30, desc + b"\0")))), "REJECTED"),
                        ("critical AIA", der(0x30, der(0x30, aia_oid + der(1, b"\xff") + der(4, der(0x30, desc)))), URL)]
        parser_cases += [(f"truncated {i}", valid_ext[:i], "REJECTED") for i in range(len(valid_ext))]
        for tag, data, expected in parser_cases:
            (work / "extensions.der").write_bytes(data)
            result = subprocess.run([server, "extensions", str(work / "extensions.der")],
                                    capture_output=True, text=True, check=True)
            assert result.stdout.strip() == expected, (tag, result.stdout, result.stderr)
        print(f"PASS {len(parser_cases)} extension parser cases")
        print(f"Passed {count} TLS scenarios and {len(parser_cases)} parser cases")


if __name__ == "__main__":
    run(sys.argv[1])
