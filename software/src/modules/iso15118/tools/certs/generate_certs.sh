#!/bin/bash

# ===============================================================================================================
# Certificate generation script for ISO 15118-2 TLS support on WARP Charger
#
# This generates the certificate chain needed for TLS between SECC (charger) and EVCC (vehicle):
#   V2G Root CA -> CPO Sub-CA 1 -> CPO Sub-CA 2 -> SECC Leaf Certificate
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

# Cryptographic parameters for ISO 15118-2
SYMMETRIC_CIPHER=-aes-128-cbc
SHA=-sha256
# Note: OpenSSL uses 'prime256v1' instead of 'secp256r1'
EC_CURVE=prime256v1

# Default password for private keys (for testing only!)
PASSWORD="${1:-12345}"

usage() {
    echo "Usage: $0 [password]"
    echo ""
    echo "Generates ISO 15118-2 TLS certificates for SECC (charger)"
    echo ""
    echo "Arguments:"
    echo "  password    Password for private key encryption (default: 12345)"
    echo ""
    echo "Output files (in ./output/):"
    echo "  certs/v2gRootCACert.pem     - V2G Root CA (give to EV simulator)"
    echo "  certs/cpoCertChain.pem      - SECC cert chain (for charger TLS)"
    echo "  private_keys/seccLeaf.key   - SECC private key (for charger TLS)"
    echo ""
    exit 0
}

if [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    usage
fi

echo "=== ISO 15118-2 Certificate Generation for WARP Charger ==="
echo "Password: $PASSWORD"
echo ""

# Create output directories
OUTPUT_DIR="$SCRIPT_DIR/output"
CERT_PATH="$OUTPUT_DIR/certs"
KEY_PATH="$OUTPUT_DIR/private_keys"
CSR_PATH="$OUTPUT_DIR/csrs"

mkdir -p "$CERT_PATH"
mkdir -p "$KEY_PATH"
mkdir -p "$CSR_PATH"

# 1) Create self-signed V2G Root CA certificate
echo "[1/4] Creating V2G Root CA..."
openssl ecparam -genkey -name $EC_CURVE | \
    openssl ec $SYMMETRIC_CIPHER -passout pass:$PASSWORD -out "$KEY_PATH/v2gRootCA.key"
openssl req -new -key "$KEY_PATH/v2gRootCA.key" -passin pass:$PASSWORD \
    -config configs/v2gRootCACert.cnf -out "$CSR_PATH/v2gRootCA.csr"
openssl x509 -req -in "$CSR_PATH/v2gRootCA.csr" \
    -extfile configs/v2gRootCACert.cnf -extensions ext \
    -signkey "$KEY_PATH/v2gRootCA.key" -passin pass:$PASSWORD $SHA \
    -set_serial 12345 -out "$CERT_PATH/v2gRootCACert.pem" -days $VALIDITY_V2G_ROOT_CERT

# 2) Create CPO Sub-CA 1 certificate (signed by V2G Root CA)
echo "[2/4] Creating CPO Sub-CA 1..."
openssl ecparam -genkey -name $EC_CURVE | \
    openssl ec $SYMMETRIC_CIPHER -passout pass:$PASSWORD -out "$KEY_PATH/cpoSubCA1.key"
openssl req -new -key "$KEY_PATH/cpoSubCA1.key" -passin pass:$PASSWORD \
    -config configs/cpoSubCA1Cert.cnf -out "$CSR_PATH/cpoSubCA1.csr"
openssl x509 -req -in "$CSR_PATH/cpoSubCA1.csr" \
    -extfile configs/cpoSubCA1Cert.cnf -extensions ext \
    -CA "$CERT_PATH/v2gRootCACert.pem" -CAkey "$KEY_PATH/v2gRootCA.key" \
    -passin pass:$PASSWORD -set_serial 12346 \
    -out "$CERT_PATH/cpoSubCA1Cert.pem" -days $VALIDITY_CPO_SUBCA1_CERT

# 3) Create CPO Sub-CA 2 certificate (signed by CPO Sub-CA 1)
echo "[3/4] Creating CPO Sub-CA 2..."
openssl ecparam -genkey -name $EC_CURVE | \
    openssl ec $SYMMETRIC_CIPHER -passout pass:$PASSWORD -out "$KEY_PATH/cpoSubCA2.key"
openssl req -new -key "$KEY_PATH/cpoSubCA2.key" -passin pass:$PASSWORD \
    -config configs/cpoSubCA2Cert.cnf -out "$CSR_PATH/cpoSubCA2.csr"
openssl x509 -req -in "$CSR_PATH/cpoSubCA2.csr" \
    -extfile configs/cpoSubCA2Cert.cnf -extensions ext \
    -CA "$CERT_PATH/cpoSubCA1Cert.pem" -CAkey "$KEY_PATH/cpoSubCA1.key" \
    -passin pass:$PASSWORD -set_serial 12347 \
    -out "$CERT_PATH/cpoSubCA2Cert.pem" -days $VALIDITY_CPO_SUBCA2_CERT

# 4) Create SECC leaf certificate (signed by CPO Sub-CA 2)
echo "[4/4] Creating SECC Leaf Certificate..."
openssl ecparam -genkey -name $EC_CURVE | \
    openssl ec $SYMMETRIC_CIPHER -passout pass:$PASSWORD -out "$KEY_PATH/seccLeaf.key"
openssl req -new -key "$KEY_PATH/seccLeaf.key" -passin pass:$PASSWORD \
    -config configs/seccLeafCert.cnf -out "$CSR_PATH/seccLeafCert.csr"
openssl x509 -req -in "$CSR_PATH/seccLeafCert.csr" \
    -extfile configs/seccLeafCert.cnf -extensions ext \
    -CA "$CERT_PATH/cpoSubCA2Cert.pem" -CAkey "$KEY_PATH/cpoSubCA2.key" \
    -passin pass:$PASSWORD -set_serial 12348 \
    -out "$CERT_PATH/seccLeafCert.pem" -days $VALIDITY_SECC_LEAF_CERT

# 5) Create the certificate chain for SECC TLS
#    Order: SECC Leaf + CPO Sub-CA 2 + CPO Sub-CA 1
#    (V2G Root CA is NOT included - it must be in the EV's trust store)
echo ""
echo "Creating certificate chain..."
cat "$CERT_PATH/seccLeafCert.pem" "$CERT_PATH/cpoSubCA2Cert.pem" "$CERT_PATH/cpoSubCA1Cert.pem" \
    > "$CERT_PATH/cpoCertChain.pem"

# 6) Create an unencrypted version of the SECC private key (for easier ESP32 loading)
echo "Creating unencrypted SECC private key..."
openssl ec -in "$KEY_PATH/seccLeaf.key" -passin pass:$PASSWORD -out "$KEY_PATH/seccLeaf_unencrypted.key"

# 7) Save the password
echo "$PASSWORD" > "$KEY_PATH/password.txt"

echo ""
echo "=== Certificate Generation Complete ==="
echo ""
echo "Files for WARP Charger (SECC):"
echo "  Certificate chain: $CERT_PATH/cpoCertChain.pem"
echo "  Private key:       $KEY_PATH/seccLeaf_unencrypted.key"
echo ""
echo "Files for EV Simulator (EVCC):"
echo "  V2G Root CA:       $CERT_PATH/v2gRootCACert.pem"
echo ""
echo "To verify the certificate chain:"
echo "  openssl verify -CAfile $CERT_PATH/v2gRootCACert.pem -untrusted $CERT_PATH/cpoCertChain.pem $CERT_PATH/seccLeafCert.pem"
echo ""
echo "To view certificate details:"
echo "  openssl x509 -in $CERT_PATH/seccLeafCert.pem -text -noout"
