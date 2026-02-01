# ISO 15118-2 Certificate Generation for WARP4 Charger

Note: These certificates are for **testing purposes only**.

```bash
./generate_certs.sh
```

This generates certificates with the default password `12345`.

To use a custom password:
```bash
./generate_certs.sh mypassword
```

## Generated Files

After running the script, the `output/` directory will contain:

### For WARP4 Charger (SECC)
- `certs/cpoCertChain.pem` - Certificate chain (SECC leaf + CPO Sub-CA 2 + CPO Sub-CA 1)
- `private_keys/seccLeaf_unencrypted.key` - Private key (unencrypted, for ESP32)

### For EV Simulator (EVCC)
- `certs/v2gRootCACert.pem` - V2G Root CA certificate (must be trusted by EV)

## Certificate Chain Structure

```
V2G Root CA (self-signed, 10 years validity)
└── CPO Sub-CA 1 (4 years validity, pathlen:1)
    └── CPO Sub-CA 2 (1 year validity, pathlen:0)
        └── SECC Leaf Certificate (60 days validity)
```

## ISO 15118-2 TLS Requirements

| Parameter | Value |
|-----------|-------|
| TLS Version | TLS 1.2 |
| Cipher Suites | ECDHE-ECDSA-AES128-SHA256, ECDH-ECDSA-AES128-SHA256 |
| Elliptic Curve | secp256r1 (NIST P-256) |
| Signature Algorithm | ECDSA with SHA-256 |

## Using with EV Simulator

1. Generate certificates: `./generate_certs.sh`
2. Copy `output/certs/v2gRootCACert.pem` to the EV simulator's trust store
3. Configure the WARP Charger to use:
   - Certificate chain: `output/certs/cpoCertChain.pem`
   - Private key: `output/private_keys/seccLeaf_unencrypted.key`

## Verification

To verify the certificate chain:
```bash
openssl verify -CAfile output/certs/v2gRootCACert.pem \
    -untrusted output/certs/cpoCertChain.pem \
    output/certs/seccLeafCert.pem
```

To view certificate details:
```bash
openssl x509 -in output/certs/seccLeafCert.pem -text -noout
```
