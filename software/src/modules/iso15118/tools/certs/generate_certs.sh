#!/bin/bash

# ===============================================================================================================
# Certificate generation script for ISO 15118 TLS support on WARP Charger
#
# This generates certificate chains for both ISO 15118-2 and ISO 15118-20:
#   V2G Root CA -> CPO Sub-CA 1 -> CPO Sub-CA 2 -> SECC Leaf Certificate
#
# ISO 15118-2:  Uses secp256r1 (prime256v1) curve with TLS 1.2
# ISO 15118-20: Uses secp521r1 curve with TLS 1.3
#
# For testing, the same V2G Root CA must be trusted by the EV simulator.
#
# Based on: https://github.com/EcoG-io/iso15118 PKI scripts
# ===============================================================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Validity periods (in days)
VALIDITY_V2G_ROOT_CERT=3650      # 10 years
VALIDITY_CPO_SUBCA1_CERT=1460    # 4 years
VALIDITY_CPO_SUBCA2_CERT=365     # 1 year
VALIDITY_SECC_LEAF_CERT=60       # 60 days

# Default password for private keys (for testing only!)
PASSWORD="${1:-12345}"

usage() {
    echo "Usage: $0 [password]"
    echo ""
    echo "Generates ISO 15118 TLS certificates for SECC (charger)"
    echo ""
    echo "Arguments:"
    echo "  password    Password for private key encryption (default: 12345)"
    echo ""
    echo "Output files (in ./output/):"
    echo "  iso2/certs/v2gRootCACert.pem       - V2G Root CA for ISO 15118-2"
    echo "  iso2/certs/cpoCertChain.pem        - SECC cert chain for ISO 15118-2 (secp256r1)"
    echo "  iso2/private_keys/seccLeaf.key     - SECC private key for ISO 15118-2"
    echo ""
    echo "  iso20/certs/v2gRootCACert.pem      - V2G Root CA for ISO 15118-20"
    echo "  iso20/certs/cpoCertChain.pem       - SECC cert chain for ISO 15118-20 (secp521r1)"
    echo "  iso20/private_keys/seccLeaf.key    - SECC private key for ISO 15118-20"
    echo ""
    exit 0
}

if [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    usage
fi

# Function to generate certificate chain for a specific ISO version
# Arguments: $1 = iso_version (iso2 or iso20), $2 = ec_curve, $3 = serial_base
generate_cert_chain() {
    local ISO_VERSION=$1
    local EC_CURVE=$2
    local SERIAL_BASE=$3
    local SYMMETRIC_CIPHER=-aes-128-cbc
    local SHA=-sha256

    echo ""
    echo "=========================================="
    echo "Generating certificates for $ISO_VERSION"
    echo "Curve: $EC_CURVE"
    echo "=========================================="

    # Create output directories
    local OUTPUT_DIR="$SCRIPT_DIR/output/$ISO_VERSION"
    local CERT_PATH="$OUTPUT_DIR/certs"
    local KEY_PATH="$OUTPUT_DIR/private_keys"
    local CSR_PATH="$OUTPUT_DIR/csrs"

    mkdir -p "$CERT_PATH"
    mkdir -p "$KEY_PATH"
    mkdir -p "$CSR_PATH"

    # 1) Create self-signed V2G Root CA certificate
    echo "[$ISO_VERSION] [1/4] Creating V2G Root CA..."
    openssl ecparam -genkey -name $EC_CURVE | \
        openssl ec $SYMMETRIC_CIPHER -passout pass:$PASSWORD -out "$KEY_PATH/v2gRootCA.key"
    openssl req -new -key "$KEY_PATH/v2gRootCA.key" -passin pass:$PASSWORD \
        -config configs/v2gRootCACert.cnf -out "$CSR_PATH/v2gRootCA.csr"
    openssl x509 -req -in "$CSR_PATH/v2gRootCA.csr" \
        -extfile configs/v2gRootCACert.cnf -extensions ext \
        -signkey "$KEY_PATH/v2gRootCA.key" -passin pass:$PASSWORD $SHA \
        -set_serial $((SERIAL_BASE + 0)) -out "$CERT_PATH/v2gRootCACert.pem" -days $VALIDITY_V2G_ROOT_CERT

    # 2) Create CPO Sub-CA 1 certificate (signed by V2G Root CA)
    echo "[$ISO_VERSION] [2/4] Creating CPO Sub-CA 1..."
    openssl ecparam -genkey -name $EC_CURVE | \
        openssl ec $SYMMETRIC_CIPHER -passout pass:$PASSWORD -out "$KEY_PATH/cpoSubCA1.key"
    openssl req -new -key "$KEY_PATH/cpoSubCA1.key" -passin pass:$PASSWORD \
        -config configs/cpoSubCA1Cert.cnf -out "$CSR_PATH/cpoSubCA1.csr"
    openssl x509 -req -in "$CSR_PATH/cpoSubCA1.csr" \
        -extfile configs/cpoSubCA1Cert.cnf -extensions ext \
        -CA "$CERT_PATH/v2gRootCACert.pem" -CAkey "$KEY_PATH/v2gRootCA.key" \
        -passin pass:$PASSWORD -set_serial $((SERIAL_BASE + 1)) \
        -out "$CERT_PATH/cpoSubCA1Cert.pem" -days $VALIDITY_CPO_SUBCA1_CERT

    # 3) Create CPO Sub-CA 2 certificate (signed by CPO Sub-CA 1)
    echo "[$ISO_VERSION] [3/4] Creating CPO Sub-CA 2..."
    openssl ecparam -genkey -name $EC_CURVE | \
        openssl ec $SYMMETRIC_CIPHER -passout pass:$PASSWORD -out "$KEY_PATH/cpoSubCA2.key"
    openssl req -new -key "$KEY_PATH/cpoSubCA2.key" -passin pass:$PASSWORD \
        -config configs/cpoSubCA2Cert.cnf -out "$CSR_PATH/cpoSubCA2.csr"
    openssl x509 -req -in "$CSR_PATH/cpoSubCA2.csr" \
        -extfile configs/cpoSubCA2Cert.cnf -extensions ext \
        -CA "$CERT_PATH/cpoSubCA1Cert.pem" -CAkey "$KEY_PATH/cpoSubCA1.key" \
        -passin pass:$PASSWORD -set_serial $((SERIAL_BASE + 2)) \
        -out "$CERT_PATH/cpoSubCA2Cert.pem" -days $VALIDITY_CPO_SUBCA2_CERT

    # 4) Create SECC leaf certificate (signed by CPO Sub-CA 2)
    echo "[$ISO_VERSION] [4/4] Creating SECC Leaf Certificate..."
    openssl ecparam -genkey -name $EC_CURVE | \
        openssl ec $SYMMETRIC_CIPHER -passout pass:$PASSWORD -out "$KEY_PATH/seccLeaf.key"
    openssl req -new -key "$KEY_PATH/seccLeaf.key" -passin pass:$PASSWORD \
        -config configs/seccLeafCert.cnf -out "$CSR_PATH/seccLeafCert.csr"
    openssl x509 -req -in "$CSR_PATH/seccLeafCert.csr" \
        -extfile configs/seccLeafCert.cnf -extensions ext \
        -CA "$CERT_PATH/cpoSubCA2Cert.pem" -CAkey "$KEY_PATH/cpoSubCA2.key" \
        -passin pass:$PASSWORD -set_serial $((SERIAL_BASE + 3)) \
        -out "$CERT_PATH/seccLeafCert.pem" -days $VALIDITY_SECC_LEAF_CERT

    # 5) Create the certificate chain for SECC TLS
    #    Order: SECC Leaf + CPO Sub-CA 2 + CPO Sub-CA 1
    #    (V2G Root CA is NOT included - it must be in the EV's trust store)
    echo "[$ISO_VERSION] Creating certificate chain..."
    cat "$CERT_PATH/seccLeafCert.pem" "$CERT_PATH/cpoSubCA2Cert.pem" "$CERT_PATH/cpoSubCA1Cert.pem" \
        > "$CERT_PATH/cpoCertChain.pem"

    # 6) Create an unencrypted version of the SECC private key (for easier ESP32 loading)
    echo "[$ISO_VERSION] Creating unencrypted SECC private key..."
    openssl ec -in "$KEY_PATH/seccLeaf.key" -passin pass:$PASSWORD -out "$KEY_PATH/seccLeaf_unencrypted.key"

    # 7) Save the password
    echo "$PASSWORD" > "$KEY_PATH/password.txt"

    echo "[$ISO_VERSION] Certificate generation complete!"
}

echo "=== ISO 15118 Certificate Generation for WARP Charger ==="
echo "Password: $PASSWORD"

# Generate ISO 15118-2 certificates (secp256r1/prime256v1)
# [V2G2-006] ECC-based using secp256r1 curve
# [V2G2-007] Key length for ECC shall be 256 bit
generate_cert_chain "iso2" "prime256v1" 12345

# Generate ISO 15118-20 certificates (secp521r1)
# [V2G20-2674] Primary: secp521r1 with ECDSA signature algorithm
# [V2G20-2675] Key length for ECC shall be 521 bit
generate_cert_chain "iso20" "secp521r1" 22345

echo ""
echo "=== Certificate Generation Complete ==="
echo ""
echo "ISO 15118-2 (TLS 1.2, secp256r1):"
echo "  Certificate chain: output/iso2/certs/cpoCertChain.pem"
echo "  Private key:       output/iso2/private_keys/seccLeaf_unencrypted.key"
echo "  V2G Root CA:       output/iso2/certs/v2gRootCACert.pem"
echo ""
echo "ISO 15118-20 (TLS 1.3, secp521r1):"
echo "  Certificate chain: output/iso20/certs/cpoCertChain.pem"
echo "  Private key:       output/iso20/private_keys/seccLeaf_unencrypted.key"
echo "  V2G Root CA:       output/iso20/certs/v2gRootCACert.pem"
echo ""
echo "To verify ISO 15118-2 certificate chain:"
echo "  openssl verify -CAfile output/iso2/certs/v2gRootCACert.pem -untrusted output/iso2/certs/cpoCertChain.pem output/iso2/certs/seccLeafCert.pem"
echo ""
echo "To verify ISO 15118-20 certificate chain:"
echo "  openssl verify -CAfile output/iso20/certs/v2gRootCACert.pem -untrusted output/iso20/certs/cpoCertChain.pem output/iso20/certs/seccLeafCert.pem"
echo ""
