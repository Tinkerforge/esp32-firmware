#!/usr/bin/env python3
"""Add TLS vehicle identities to an existing development PKI.

Keeps OEM provisioning identities/roots intact. These are test fixtures for role,
source and TLS checks, not a declaration of complete Annex B conformance.
"""
import argparse
from datetime import datetime, timedelta, timezone
from pathlib import Path

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import AuthorityInformationAccessOID, ExtendedKeyUsageOID, NameOID


def generate(directory, password):
    certs, keys = directory / "certs", directory / "private_keys"
    root = x509.load_pem_x509_certificate((certs / "oemRootCACert.pem").read_bytes())
    root_key = serialization.load_pem_private_key((keys / "oemRootCA.key").read_bytes(), password)
    issuer, issuer_key = root, root_key
    chain = []
    now = datetime.now(timezone.utc)
    for index, label in enumerate(["vehicleSubCA1", "vehicleSubCA2", "vehicleLeaf"]):
        key = ec.generate_private_key(ec.SECP521R1())
        ca = index < 2
        subject = x509.Name([x509.NameAttribute(NameOID.ORGANIZATION_NAME, "WARP Test"),
                             x509.NameAttribute(NameOID.COMMON_NAME, label if ca else "WVWV123456789ABCDEF7"),
                             x509.NameAttribute(NameOID.DOMAIN_COMPONENT, "EV")])
        builder = (x509.CertificateBuilder().subject_name(subject).issuer_name(issuer.subject)
                   .public_key(key.public_key()).serial_number(x509.random_serial_number())
                   .not_valid_before(now - timedelta(minutes=5))
                   .not_valid_after(min(now + timedelta(days=60), root.not_valid_after_utc))
                   .add_extension(x509.BasicConstraints(ca, 1 - index if ca else None), True)
                   .add_extension(x509.KeyUsage(not ca, False, False, False, False, ca, False, False, False), True)
                   .add_extension(x509.SubjectKeyIdentifier.from_public_key(key.public_key()), False)
                   .add_extension(x509.AuthorityKeyIdentifier.from_issuer_public_key(issuer_key.public_key()), False)
                   .add_extension(x509.AuthorityInformationAccess([x509.AccessDescription(
                       AuthorityInformationAccessOID.OCSP,
                       x509.UniformResourceIdentifier(f"http://ocsp.vehicle.test/{label}"))]), False))
        if not ca:
            builder = builder.add_extension(x509.ExtendedKeyUsage([ExtendedKeyUsageOID.CLIENT_AUTH]), True)
        cert = builder.sign(issuer_key, hashes.SHA512())
        (certs / f"{label}Cert.pem").write_bytes(cert.public_bytes(serialization.Encoding.PEM))
        (keys / f"{label}.key").write_bytes(key.private_bytes(serialization.Encoding.PEM,
            serialization.PrivateFormat.PKCS8, serialization.BestAvailableEncryption(password)))
        chain.insert(0, cert.public_bytes(serialization.Encoding.PEM))
        issuer, issuer_key = cert, key
    (certs / "vehicleCertChain.pem").write_bytes(b"".join(chain))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--directory", type=Path, default=Path(__file__).parent / "output/iso20")
    parser.add_argument("--password", default="12345")
    args = parser.parse_args()
    generate(args.directory, args.password.encode())
