# ISO 15118 Certificate Generation for WARP4 Charger

Note: These certificates are for **testing purposes only**.

```bash
./generate_certs.sh
```

This generates certificates with the default password `12345`.

To use a custom password:
```bash
./generate_certs.sh mypassword
```

The script generates TLS certificates for both ISO 15118-2 and ISO 15118-20,
and automatically updates the embedded certificates in the firmware source code
(`dev_certs.cpp`).

Two certificate chains are generated per ISO version:
- **SECC chain** (charger/server): V2G Root CA -> CPO Sub-CA 1 -> CPO Sub-CA 2 -> SECC Leaf
- **OEM chain** (EV simulator/client): OEM Root CA -> OEM Sub-CA 1 -> OEM Sub-CA 2 -> OEM Leaf

The OEM chain is required by the iso15118 Python library (evsim) for TLS 1.3
mutual authentication.

## Generated Files

After running the script, the `output/` directory will contain:

### ISO 15118-2 (`output/iso2/`, secp256r1, TLS 1.2)

| File | Description |
|------|-------------|
| `certs/v2gRootCACert.pem` | V2G Root CA (must be in EV trust store) |
| `certs/cpoCertChain.pem` | SECC cert chain (Leaf + Sub-CA 2 + Sub-CA 1) |
| `certs/oemCertChain.pem` | OEM cert chain (for evsim client auth) |
| `private_keys/seccLeaf_unencrypted.key` | SECC private key (for ESP32) |
| `private_keys/oemLeaf.key` | OEM private key (encrypted) |

### ISO 15118-20 (`output/iso20/`, secp521r1, TLS 1.3)

| File | Description |
|------|-------------|
| `certs/v2gRootCACert.pem` | V2G Root CA (must be in EV trust store) |
| `certs/cpoCertChain.pem` | SECC cert chain (Leaf + Sub-CA 2 + Sub-CA 1) |
| `certs/oemCertChain.pem` | OEM cert chain (for evsim client auth) |
| `private_keys/seccLeaf_unencrypted.key` | SECC private key (for ESP32) |
| `private_keys/oemLeaf.key` | OEM private key (encrypted) |

### Firmware

The script automatically updates `src/modules/iso15118/dev_certs.cpp` with the
generated SECC certificate chains and private keys for both ISO versions.
This ensures the embedded certificates always match the generated files.

## Certificate Chain Structure

```
V2G Root CA (self-signed, 10 years)
└── CPO Sub-CA 1 (4 years, pathlen:1)
    └── CPO Sub-CA 2 (1 year, pathlen:0)
        └── SECC Leaf Certificate (60 days)

OEM Root CA (self-signed, 10 years)
└── OEM Sub-CA 1 (4 years, pathlen:1)
    └── OEM Sub-CA 2 (1 year, pathlen:0)
        └── OEM Leaf Certificate (60 days)
```

## Verification

To verify the ISO 15118-2 certificate chain:
```bash
openssl verify -CAfile output/iso2/certs/v2gRootCACert.pem \
    -untrusted output/iso2/certs/cpoCertChain.pem \
    output/iso2/certs/seccLeafCert.pem
```

To verify the ISO 15118-20 certificate chain:
```bash
openssl verify -CAfile output/iso20/certs/v2gRootCACert.pem \
    -untrusted output/iso20/certs/cpoCertChain.pem \
    output/iso20/certs/seccLeafCert.pem
```

To view certificate details:
```bash
openssl x509 -in output/iso2/certs/seccLeafCert.pem -text -noout
```
