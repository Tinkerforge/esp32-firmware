#!/usr/bin/env python3
"""Validate ISO 15118-20 B.4.1, B.4.2 and B.9.2 certificate profiles."""

from datetime import datetime, timedelta, timezone

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec, ed448
from cryptography.x509 import ocsp
from cryptography.x509.oid import (
    AuthorityInformationAccessOID,
    ExtendedKeyUsageOID,
    NameOID,
    ObjectIdentifier,
    SignatureAlgorithmOID,
)


OCSP_URL = "https://ocsp.cso.example"
POLICY_OID = ObjectIdentifier("1.3.6.1.4.1.55555.15118.20")
PROFILE_TIME = datetime(2051, 1, 1, tzinfo=timezone.utc)
SECC_ID = "DETFCSECC" + "0" * 29 + "1"
ALLOWED_NAME_OIDS = {
    NameOID.COUNTRY_NAME,
    NameOID.ORGANIZATION_NAME,
    NameOID.ORGANIZATIONAL_UNIT_NAME,
    NameOID.COMMON_NAME,
    NameOID.DOMAIN_COMPONENT,
}


def name(common_name, domain_component="TFCERTCSO"):
    attributes = [
        x509.NameAttribute(NameOID.COUNTRY_NAME, "DE"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "Tinkerforge"),
        x509.NameAttribute(NameOID.COMMON_NAME, common_name),
    ]
    if domain_component is not None:
        attributes.append(x509.NameAttribute(NameOID.DOMAIN_COMPONENT, domain_component))
    return x509.Name(attributes)


def key_usage(*, ca, key_agreement=False):
    return x509.KeyUsage(
        digital_signature=not ca,
        content_commitment=False,
        key_encipherment=False,
        data_encipherment=False,
        key_agreement=key_agreement,
        key_cert_sign=ca,
        crl_sign=False,
        encipher_only=False if key_agreement else None,
        decipher_only=False if key_agreement else None,
    )


def certificate(
    subject,
    subject_key,
    issuer,
    issuer_key,
    serial,
    not_before,
    not_after,
    algorithm,
    *,
    path_length=None,
    key_agreement=False,
    server_auth=False,
    ocsp_signing=False,
    aia=True,
):
    ca = path_length is not None
    builder = (
        x509.CertificateBuilder()
        .subject_name(subject)
        .issuer_name(issuer)
        .public_key(subject_key.public_key())
        .serial_number(serial)
        .not_valid_before(not_before)
        .not_valid_after(not_after)
        .add_extension(
            x509.AuthorityKeyIdentifier.from_issuer_public_key(issuer_key.public_key()),
            critical=False,
        )
        .add_extension(
            x509.SubjectKeyIdentifier.from_public_key(subject_key.public_key()),
            critical=False,
        )
        .add_extension(key_usage(ca=ca, key_agreement=key_agreement), critical=True)
        .add_extension(x509.BasicConstraints(ca=ca, path_length=path_length), critical=True)
        .add_extension(
            x509.CertificatePolicies([
                x509.PolicyInformation(POLICY_OID, ["https://cso.example/cps"]),
            ]),
            critical=False,
        )
    )
    if server_auth:
        builder = builder.add_extension(
            x509.ExtendedKeyUsage([ExtendedKeyUsageOID.SERVER_AUTH]), critical=True)
    if ocsp_signing:
        builder = builder.add_extension(
            x509.ExtendedKeyUsage([ExtendedKeyUsageOID.OCSP_SIGNING]), critical=True)
    if aia:
        builder = builder.add_extension(
            x509.AuthorityInformationAccess([
                x509.AccessDescription(
                    AuthorityInformationAccessOID.OCSP,
                    x509.UniformResourceIdentifier(OCSP_URL),
                ),
            ]),
            critical=False,
        )
    return builder.sign(issuer_key, algorithm)


def extension(cert, extension_type, critical):
    value = cert.extensions.get_extension_for_class(extension_type)
    assert value.critical is critical, (cert.subject, extension_type, value.critical)
    return value.value


def validate_name(value, *, cso=False, secc=False):
    assert all(attribute.oid in ALLOWED_NAME_OIDS for rdn in value.rdns for attribute in rdn)
    for oid in (NameOID.COUNTRY_NAME, NameOID.ORGANIZATION_NAME, NameOID.COMMON_NAME):
        assert len(value.get_attributes_for_oid(oid)) == 1, (value, oid)
    country = value.get_attributes_for_oid(NameOID.COUNTRY_NAME)[0].value
    assert len(country) == 2 and country.isalpha() and country.isupper(), country
    if cso:
        components = value.get_attributes_for_oid(NameOID.DOMAIN_COMPONENT)
        assert len(components) == 1 and components[0].value.endswith("CSO"), value
    if secc:
        common_name = value.get_attributes_for_oid(NameOID.COMMON_NAME)[0].value
        assert 39 <= len(common_name) <= 64 and common_name.isalnum(), common_name


def der_tlv(data, offset):
    tag = data[offset]
    length = data[offset + 1]
    header_length = 2
    if length & 0x80:
        length_length = length & 0x7F
        length = int.from_bytes(data[offset + 2:offset + 2 + length_length], "big")
        header_length += length_length
    content_start = offset + header_length
    content_end = content_start + length
    return tag, content_start, content_end, content_end


def validate_generalized_time(cert):
    der = cert.public_bytes(serialization.Encoding.DER)
    _, certificate_start, _, _ = der_tlv(der, 0)
    _, tbs_start, _, _ = der_tlv(der, certificate_start)
    offset = tbs_start
    if der[offset] == 0xA0:
        _, _, _, offset = der_tlv(der, offset)
    for _ in range(3):  # serialNumber, signature, issuer
        _, _, _, offset = der_tlv(der, offset)
    tag, validity_start, validity_end, _ = der_tlv(der, offset)
    assert tag == 0x30
    offset = validity_start
    for _ in range(2):
        tag, time_start, time_end, offset = der_tlv(der, offset)
        value = der[time_start:time_end]
        assert tag == 0x18, (cert.subject, "validity is not GeneralizedTime")
        assert len(value) == 15 and value.endswith(b"Z"), (cert.subject, value)
    assert offset == validity_end


def validate_algorithm(cert, profile):
    if profile == "B.4.1":
        assert cert.signature_algorithm_oid == SignatureAlgorithmOID.ECDSA_WITH_SHA512
        assert isinstance(cert.signature_hash_algorithm, hashes.SHA512)
        public_key = cert.public_key()
        assert isinstance(public_key, ec.EllipticCurvePublicKey)
        assert isinstance(public_key.curve, ec.SECP521R1)
    else:
        assert cert.signature_algorithm_oid == SignatureAlgorithmOID.ED448
        assert cert.signature_hash_algorithm is None
        assert isinstance(cert.public_key(), ed448.Ed448PublicKey)


def validate_link(cert, issuer):
    assert cert.issuer == issuer.subject
    cert.verify_directly_issued_by(issuer)
    authority_key = extension(cert, x509.AuthorityKeyIdentifier, False)
    subject_key = extension(issuer, x509.SubjectKeyIdentifier, False)
    assert authority_key.key_identifier == subject_key.digest
    assert cert.not_valid_before_utc >= issuer.not_valid_before_utc
    assert cert.not_valid_after_utc <= issuer.not_valid_after_utc


def validate_common(cert, profile, *, issuer, ca, path_length, cso=False, secc=False):
    assert cert.version == x509.Version.v3
    assert cert.serial_number > 0
    validate_algorithm(cert, profile)
    validate_name(cert.subject, cso=cso, secc=secc)
    validate_generalized_time(cert)
    validate_link(cert, issuer)

    subject_key = extension(cert, x509.SubjectKeyIdentifier, False)
    expected_key = x509.SubjectKeyIdentifier.from_public_key(cert.public_key())
    assert subject_key.digest == expected_key.digest

    usage = extension(cert, x509.KeyUsage, True)
    assert usage.key_cert_sign is ca
    assert usage.digital_signature is (not ca)
    assert not usage.content_commitment
    assert not usage.key_encipherment
    assert not usage.data_encipherment
    assert not usage.crl_sign

    constraints = extension(cert, x509.BasicConstraints, True)
    assert constraints.ca is ca
    assert constraints.path_length == path_length

    policies = extension(cert, x509.CertificatePolicies, False)
    assert len(policies) == 1
    assert policies[0].policy_identifier == POLICY_OID
    assert policies[0].policy_qualifiers == ["https://cso.example/cps"]


def make_cso_chain(profile):
    now = PROFILE_TIME
    if profile == "B.4.1":
        new_key = lambda: ec.generate_private_key(ec.SECP521R1())
        algorithm = hashes.SHA512()
    else:
        new_key = ed448.Ed448PrivateKey.generate
        algorithm = None

    root_key, ca1_key, ca2_key, leaf_key = (new_key() for _ in range(4))
    root_name = name(f"{profile} V2G root", "ROOTV2G")
    root = certificate(
        root_name, root_key, root_name, root_key, 1,
        now - timedelta(days=1), now + timedelta(days=3650), algorithm,
        path_length=3, aia=False,
    )
    ca1 = certificate(
        name(f"{profile} CSO sub-CA 1"), ca1_key, root.subject, root_key, 2,
        now - timedelta(minutes=3), now + timedelta(days=1095), algorithm,
        path_length=1,
    )
    ca2 = certificate(
        name(f"{profile} CSO sub-CA 2"), ca2_key, ca1.subject, ca1_key, 3,
        now - timedelta(minutes=2), now + timedelta(days=730), algorithm,
        path_length=0,
    )
    leaf = certificate(
        name(SECC_ID), leaf_key, ca2.subject, ca2_key, 4,
        now - timedelta(minutes=1), now + timedelta(days=365), algorithm,
        key_agreement=profile == "B.4.1", server_auth=True,
    )
    return root, ca1, ca2, leaf, ca2_key


def validate_cso_chain(profile, root, ca1, ca2, leaf):
    validate_common(ca1, profile, issuer=root, ca=True, path_length=1, cso=True)
    validate_common(ca2, profile, issuer=ca1, ca=True, path_length=0, cso=True)
    validate_common(
        leaf, profile, issuer=ca2, ca=False, path_length=None, cso=True, secc=True)

    for cert in (ca1, ca2, leaf):
        aia = extension(cert, x509.AuthorityInformationAccess, False)
        assert len(aia) == 1
        assert aia[0].access_method == AuthorityInformationAccessOID.OCSP
        assert isinstance(aia[0].access_location, x509.UniformResourceIdentifier)
        assert aia[0].access_location.value == OCSP_URL

    for cert in (ca1, ca2):
        try:
            cert.extensions.get_extension_for_class(x509.ExtendedKeyUsage)
        except x509.ExtensionNotFound:
            pass
        else:
            raise AssertionError(f"CA certificate has ExtendedKeyUsage: {cert.subject}")

    leaf_usage = extension(leaf, x509.KeyUsage, True)
    assert leaf_usage.key_agreement is (profile == "B.4.1")
    leaf_eku = extension(leaf, x509.ExtendedKeyUsage, True)
    assert list(leaf_eku) == [ExtendedKeyUsageOID.SERVER_AUTH]


def validate_b92(issuer, issuer_key, certified_leaf):
    now = PROFILE_TIME
    signer_key = ed448.Ed448PrivateKey.generate()
    signer = certificate(
        name("B.9.2 OCSP responder", None), signer_key,
        issuer.subject, issuer_key, 5,
        now - timedelta(minutes=1), now + timedelta(days=364), None,
        ocsp_signing=True, aia=False,
    )
    validate_common(
        signer, "B.9.2", issuer=issuer, ca=False, path_length=None)
    assert signer.not_valid_after_utc - signer.not_valid_before_utc <= timedelta(days=365)
    usage = extension(signer, x509.KeyUsage, True)
    assert usage.digital_signature
    assert not usage.key_agreement
    eku = extension(signer, x509.ExtendedKeyUsage, True)
    assert list(eku) == [ExtendedKeyUsageOID.OCSP_SIGNING]
    assert {item.oid for item in signer.extensions} == {
        x509.ExtensionOID.AUTHORITY_KEY_IDENTIFIER,
        x509.ExtensionOID.SUBJECT_KEY_IDENTIFIER,
        x509.ExtensionOID.KEY_USAGE,
        x509.ExtensionOID.BASIC_CONSTRAINTS,
        x509.ExtensionOID.CERTIFICATE_POLICIES,
        x509.ExtensionOID.EXTENDED_KEY_USAGE,
    }

    response = (
        ocsp.OCSPResponseBuilder()
        .add_response(
            cert=certified_leaf,
            issuer=issuer,
            algorithm=hashes.SHA256(),
            cert_status=ocsp.OCSPCertStatus.GOOD,
            this_update=now,
            next_update=now + timedelta(days=7),
            revocation_time=None,
            revocation_reason=None,
        )
        .responder_id(ocsp.OCSPResponderEncoding.HASH, signer)
        .certificates([signer])
        .sign(signer_key, None)
    )
    assert response.response_status == ocsp.OCSPResponseStatus.SUCCESSFUL
    assert response.signature_algorithm_oid == SignatureAlgorithmOID.ED448
    assert response.certificates == [signer]
    signer.public_key().verify(response.signature, response.tbs_response_bytes)


def main():
    chains = {}
    for profile in ("B.4.1", "B.4.2"):
        chain = make_cso_chain(profile)
        validate_cso_chain(profile, *chain[:4])
        chains[profile] = chain
        print(f"PASS {profile}: CSO sub-CA1/sub-CA2/SECC profile and chain")

    _, _, ed448_ca2, ed448_leaf, ed448_ca2_key = chains["B.4.2"]
    validate_b92(ed448_ca2, ed448_ca2_key, ed448_leaf)
    print("PASS B.9.2: delegated Ed448 OCSP responder profile and response signature")


if __name__ == "__main__":
    main()
