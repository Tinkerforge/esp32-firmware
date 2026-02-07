#!/bin/bash

# ===============================================================================================================
# Certificate generation script for ISO 15118 TLS support on WARP Charger
#
# This generates certificate chains for both ISO 15118-2 and ISO 15118-20:
#   SECC (charger) chain: V2G Root CA -> CPO Sub-CA 1 -> CPO Sub-CA 2 -> SECC Leaf Certificate
#   OEM (EV client) chain: OEM Root CA -> OEM Sub-CA 1 -> OEM Sub-CA 2 -> OEM Leaf Certificate
#
# The OEM certificate chain is required by the iso15118 Python library (evsim) for
# TLS 1.3 mutual authentication (ISO 15118-20).
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
VALIDITY_OEM_ROOT_CERT=3650      # 10 years
VALIDITY_OEM_SUBCA1_CERT=1460    # 4 years
VALIDITY_OEM_SUBCA2_CERT=365     # 1 year
VALIDITY_OEM_LEAF_CERT=60        # 60 days

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
    echo "  iso2/certs/oemCertChain.pem        - OEM cert chain for ISO 15118-2 (evsim TLS 1.3)"
    echo "  iso2/private_keys/oemLeaf.key      - OEM private key for ISO 15118-2"
    echo ""
    echo "  iso20/certs/v2gRootCACert.pem      - V2G Root CA for ISO 15118-20"
    echo "  iso20/certs/cpoCertChain.pem       - SECC cert chain for ISO 15118-20 (secp521r1)"
    echo "  iso20/private_keys/seccLeaf.key    - SECC private key for ISO 15118-20"
    echo "  iso20/certs/oemCertChain.pem       - OEM cert chain for ISO 15118-20 (evsim TLS 1.3)"
    echo "  iso20/private_keys/oemLeaf.key     - OEM private key for ISO 15118-20"
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

    echo "[$ISO_VERSION] SECC certificate generation complete!"
}

# Function to generate OEM certificate chain for a specific ISO version.
# The OEM chain is used by the EV simulator (evsim) as the client certificate
# for TLS 1.3 mutual authentication.
#
# Chain: OEM Root CA -> OEM Sub-CA 1 -> OEM Sub-CA 2 -> OEM Leaf
#
# Arguments: $1 = iso_version (iso2 or iso20), $2 = ec_curve, $3 = serial_base
generate_oem_cert_chain() {
    local ISO_VERSION=$1
    local EC_CURVE=$2
    local SERIAL_BASE=$3
    local SYMMETRIC_CIPHER=-aes-128-cbc
    local SHA=-sha256

    echo ""
    echo "[$ISO_VERSION] Generating OEM certificate chain..."

    local OUTPUT_DIR="$SCRIPT_DIR/output/$ISO_VERSION"
    local CERT_PATH="$OUTPUT_DIR/certs"
    local KEY_PATH="$OUTPUT_DIR/private_keys"
    local CSR_PATH="$OUTPUT_DIR/csrs"

    # Directories should already exist from generate_cert_chain

    # 1) Create self-signed OEM Root CA certificate
    echo "[$ISO_VERSION] [OEM 1/4] Creating OEM Root CA..."
    openssl ecparam -genkey -name $EC_CURVE | \
        openssl ec $SYMMETRIC_CIPHER -passout pass:$PASSWORD -out "$KEY_PATH/oemRootCA.key"
    openssl req -new -key "$KEY_PATH/oemRootCA.key" -passin pass:$PASSWORD \
        -config configs/oemRootCACert.cnf -out "$CSR_PATH/oemRootCA.csr"
    openssl x509 -req -in "$CSR_PATH/oemRootCA.csr" \
        -extfile configs/oemRootCACert.cnf -extensions ext \
        -signkey "$KEY_PATH/oemRootCA.key" -passin pass:$PASSWORD $SHA \
        -set_serial $((SERIAL_BASE + 0)) -out "$CERT_PATH/oemRootCACert.pem" -days $VALIDITY_OEM_ROOT_CERT

    # 2) Create OEM Sub-CA 1 (signed by OEM Root CA)
    echo "[$ISO_VERSION] [OEM 2/4] Creating OEM Sub-CA 1..."
    openssl ecparam -genkey -name $EC_CURVE | \
        openssl ec $SYMMETRIC_CIPHER -passout pass:$PASSWORD -out "$KEY_PATH/oemSubCA1.key"
    openssl req -new -key "$KEY_PATH/oemSubCA1.key" -passin pass:$PASSWORD \
        -config configs/oemSubCA1Cert.cnf -out "$CSR_PATH/oemSubCA1.csr"
    openssl x509 -req -in "$CSR_PATH/oemSubCA1.csr" \
        -extfile configs/oemSubCA1Cert.cnf -extensions ext \
        -CA "$CERT_PATH/oemRootCACert.pem" -CAkey "$KEY_PATH/oemRootCA.key" \
        -passin pass:$PASSWORD -set_serial $((SERIAL_BASE + 1)) \
        -out "$CERT_PATH/oemSubCA1Cert.pem" -days $VALIDITY_OEM_SUBCA1_CERT

    # 3) Create OEM Sub-CA 2 (signed by OEM Sub-CA 1)
    echo "[$ISO_VERSION] [OEM 3/4] Creating OEM Sub-CA 2..."
    openssl ecparam -genkey -name $EC_CURVE | \
        openssl ec $SYMMETRIC_CIPHER -passout pass:$PASSWORD -out "$KEY_PATH/oemSubCA2.key"
    openssl req -new -key "$KEY_PATH/oemSubCA2.key" -passin pass:$PASSWORD \
        -config configs/oemSubCA2Cert.cnf -out "$CSR_PATH/oemSubCA2.csr"
    openssl x509 -req -in "$CSR_PATH/oemSubCA2.csr" \
        -extfile configs/oemSubCA2Cert.cnf -extensions ext \
        -CA "$CERT_PATH/oemSubCA1Cert.pem" -CAkey "$KEY_PATH/oemSubCA1.key" \
        -passin pass:$PASSWORD -set_serial $((SERIAL_BASE + 2)) \
        -out "$CERT_PATH/oemSubCA2Cert.pem" -days $VALIDITY_OEM_SUBCA2_CERT

    # 4) Create OEM Leaf certificate (signed by OEM Sub-CA 2)
    echo "[$ISO_VERSION] [OEM 4/4] Creating OEM Leaf Certificate..."
    openssl ecparam -genkey -name $EC_CURVE | \
        openssl ec $SYMMETRIC_CIPHER -passout pass:$PASSWORD -out "$KEY_PATH/oemLeaf.key"
    openssl req -new -key "$KEY_PATH/oemLeaf.key" -passin pass:$PASSWORD \
        -config configs/oemLeafCert.cnf -out "$CSR_PATH/oemLeafCert.csr"
    openssl x509 -req -in "$CSR_PATH/oemLeafCert.csr" \
        -extfile configs/oemLeafCert.cnf -extensions ext \
        -CA "$CERT_PATH/oemSubCA2Cert.pem" -CAkey "$KEY_PATH/oemSubCA2.key" \
        -passin pass:$PASSWORD -set_serial $((SERIAL_BASE + 3)) \
        -out "$CERT_PATH/oemLeafCert.pem" -days $VALIDITY_OEM_LEAF_CERT

    # 5) Create OEM certificate chain
    #    Order: OEM Leaf + OEM Sub-CA 2 + OEM Sub-CA 1
    echo "[$ISO_VERSION] Creating OEM certificate chain..."
    cat "$CERT_PATH/oemLeafCert.pem" "$CERT_PATH/oemSubCA2Cert.pem" "$CERT_PATH/oemSubCA1Cert.pem" \
        > "$CERT_PATH/oemCertChain.pem"

    # 6) Save password files for iso15118 library
    #    The library expects separate password files for SECC and OEM leaf keys
    echo "$PASSWORD" > "$KEY_PATH/seccLeafPassword.txt"
    echo "$PASSWORD" > "$KEY_PATH/oemLeafPassword.txt"

    echo "[$ISO_VERSION] OEM certificate generation complete!"
}

# Function to convert a PEM file to C string literal lines.
# Each line of the PEM file becomes: "line\n"
# Output is written to stdout.
pem_to_c_string() {
    local PEM_FILE=$1
    while IFS= read -r line; do
        # Skip empty lines at end of file
        [ -z "$line" ] && continue
        printf '    "%s\\n"\n' "$line"
    done < "$PEM_FILE"
}

# Function to convert a PEM cert chain file to C string literal lines
# with C++ comments between certificates.
# Chain order: SECC Leaf, CPO Sub-CA 2, CPO Sub-CA 1
cert_chain_to_c_string() {
    local PEM_FILE=$1
    local cert_num=0
    local comments=("SECC Leaf Certificate" "CPO Sub-CA 2" "CPO Sub-CA 1")

    while IFS= read -r line; do
        [ -z "$line" ] && continue
        if [ "$line" = "-----BEGIN CERTIFICATE-----" ]; then
            if [ $cert_num -gt 0 ]; then
                # Add blank comment line between certs
                echo "    // ${comments[$cert_num]}"
            else
                echo "    // ${comments[$cert_num]}"
            fi
            cert_num=$((cert_num + 1))
        fi
        printf '    "%s\\n"\n' "$line"
    done < "$PEM_FILE"
}

# Function to update dev_certs.cpp with freshly generated certificates.
# This ensures the embedded certificates in the firmware always match
# the generated certificate files.
update_dev_certs_cpp() {
    local DEV_CERTS_CPP="$SCRIPT_DIR/../../dev_certs.cpp"

    if [ ! -f "$DEV_CERTS_CPP" ]; then
        echo "Warning: dev_certs.cpp not found at $DEV_CERTS_CPP, skipping update"
        return
    fi

    echo ""
    echo "=== Updating dev_certs.cpp ==="

    local ISO2_CHAIN="$SCRIPT_DIR/output/iso2/certs/cpoCertChain.pem"
    local ISO2_KEY="$SCRIPT_DIR/output/iso2/private_keys/seccLeaf_unencrypted.key"
    local ISO20_CHAIN="$SCRIPT_DIR/output/iso20/certs/cpoCertChain.pem"
    local ISO20_KEY="$SCRIPT_DIR/output/iso20/private_keys/seccLeaf_unencrypted.key"

    for f in "$ISO2_CHAIN" "$ISO2_KEY" "$ISO20_CHAIN" "$ISO20_KEY"; do
        if [ ! -f "$f" ]; then
            echo "Error: Required file not found: $f"
            return 1
        fi
    done

    # Build the C string literals
    local ISO2_CHAIN_C
    ISO2_CHAIN_C=$(cert_chain_to_c_string "$ISO2_CHAIN")
    local ISO2_KEY_C
    ISO2_KEY_C=$(pem_to_c_string "$ISO2_KEY")
    local ISO20_CHAIN_C
    ISO20_CHAIN_C=$(cert_chain_to_c_string "$ISO20_CHAIN")
    local ISO20_KEY_C
    ISO20_KEY_C=$(pem_to_c_string "$ISO20_KEY")

    cat > "$DEV_CERTS_CPP" << CPPEOF
// WARNING: This file is generated by "src/modules/iso15118/tools/certs/generate_certs.sh"

// Embedded test certificates for development/testing
// Certificate chain: SECC Leaf -> CPO Sub-CA2 -> CPO Sub-CA1

#include "dev_certs.h"

#include "gcc_warnings.h"

// =============================================================================
// ISO 15118-2 Certificates (secp256r1/prime256v1, TLS 1.2)
// =============================================================================
// [V2G2-006] ECC-based using secp256r1 curve
// [V2G2-007] Key length for ECC shall be 256 bit

const char dev_cert_chain_pem_iso2[] =
${ISO2_CHAIN_C};

// SECC leaf private key for ISO 15118-2 (unencrypted EC key, secp256r1)
const char dev_private_key_pem_iso2[] =
${ISO2_KEY_C};

// =============================================================================
// ISO 15118-20 Certificates (secp521r1, TLS 1.3)
// =============================================================================
// [V2G20-2674] Primary: secp521r1 with ECDSA signature algorithm
// [V2G20-2675] Key length for ECC shall be 521 bit

const char dev_cert_chain_pem_iso20[] =
${ISO20_CHAIN_C};

// SECC leaf private key for ISO 15118-20 (unencrypted EC key, secp521r1)
const char dev_private_key_pem_iso20[] =
${ISO20_KEY_C};
CPPEOF

    echo "  Updated: $DEV_CERTS_CPP"
}

echo "=== ISO 15118 Certificate Generation for WARP Charger ==="
echo "Password: $PASSWORD"

# Generate ISO 15118-2 certificates (secp256r1/prime256v1)
# [V2G2-006] ECC-based using secp256r1 curve
# [V2G2-007] Key length for ECC shall be 256 bit
generate_cert_chain "iso2" "prime256v1" 12345
generate_oem_cert_chain "iso2" "prime256v1" 32345

# Generate ISO 15118-20 certificates (secp521r1)
# [V2G20-2674] Primary: secp521r1 with ECDSA signature algorithm
# [V2G20-2675] Key length for ECC shall be 521 bit
generate_cert_chain "iso20" "secp521r1" 22345
generate_oem_cert_chain "iso20" "secp521r1" 42345

# Update dev_certs.cpp with the freshly generated certificates
update_dev_certs_cpp

echo ""
echo "=== Certificate Generation Complete ==="
echo ""
echo "ISO 15118-2 (TLS 1.2, secp256r1):"
echo "  Certificate chain: output/iso2/certs/cpoCertChain.pem"
echo "  Private key:       output/iso2/private_keys/seccLeaf_unencrypted.key"
echo "  V2G Root CA:       output/iso2/certs/v2gRootCACert.pem"
echo "  OEM cert chain:    output/iso2/certs/oemCertChain.pem"
echo "  OEM private key:   output/iso2/private_keys/oemLeaf.key"
echo ""
echo "ISO 15118-20 (TLS 1.3, secp521r1):"
echo "  Certificate chain: output/iso20/certs/cpoCertChain.pem"
echo "  Private key:       output/iso20/private_keys/seccLeaf_unencrypted.key"
echo "  V2G Root CA:       output/iso20/certs/v2gRootCACert.pem"
echo "  OEM cert chain:    output/iso20/certs/oemCertChain.pem"
echo "  OEM private key:   output/iso20/private_keys/oemLeaf.key"
echo ""
echo "Firmware:"
echo "  dev_certs.cpp:     src/modules/iso15118/dev_certs.cpp (auto-updated)"
echo ""
echo "To verify ISO 15118-2 certificate chain:"
echo "  openssl verify -CAfile output/iso2/certs/v2gRootCACert.pem -untrusted output/iso2/certs/cpoCertChain.pem output/iso2/certs/seccLeafCert.pem"
echo ""
echo "To verify ISO 15118-20 certificate chain:"
echo "  openssl verify -CAfile output/iso20/certs/v2gRootCACert.pem -untrusted output/iso20/certs/cpoCertChain.pem output/iso20/certs/seccLeafCert.pem"
echo ""
