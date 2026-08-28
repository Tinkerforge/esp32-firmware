#!/bin/bash
# Host-only regression test for mbedTLS patch 0008.

set -eu

cd "$(dirname "$0")"
readonly BUILD="$(mktemp -d "${TMPDIR:-/tmp}/iso15118-tls12-v2g-extensions.XXXXXXXX")"
trap 'rm -rf -- "$BUILD"' EXIT
PATCH="$(realpath ../../../../patches/lib-builder/esp-idf/components/mbedtls/mbedtls/0008-Add-TLS-1.2-trusted-CA-and-OCSP-multi-server-hooks.patch)"
CERTS="$(realpath ../tools/certs/output/iso20)"

[ -f "$CERTS/certs/cpoCertChain.pem" ] || { echo "dev PKI missing, run ../certs/generate_certs.sh first"; exit 1; }

if [ -n "${MBEDTLS_SRC:-}" ]; then
    cp -r "$MBEDTLS_SRC" "$BUILD/mbedtls"
    rm -rf -- "$BUILD/mbedtls/.git"
else
    git clone --quiet --depth 1 --branch v3.6.6 --recurse-submodules --shallow-submodules \
        https://github.com/Mbed-TLS/mbedtls "$BUILD/mbedtls"
fi

grep -q "3.6.6" "$BUILD/mbedtls/include/mbedtls/build_info.h" || { echo "mbedTLS source is not 3.6.6"; exit 1; }

echo "--- applying patch 0008 ---"
patch -d "$BUILD/mbedtls" -p1 --forward < "$PATCH"

echo "--- building mbedTLS and TLS 1.2 host test ---"
make -C "$BUILD/mbedtls" lib -j"$(nproc)" > /dev/null
gcc -Wall -Wextra -Werror -O1 -pthread \
    -I "$BUILD/mbedtls/include" -I "$BUILD/mbedtls/library" \
    -o "$BUILD/tls12_v2g_extensions" -x c _tls12_v2g_extensions.c.inc -x none \
    "$BUILD/mbedtls/library/libmbedtls.a" \
    "$BUILD/mbedtls/library/libmbedx509.a" \
    "$BUILD/mbedtls/library/libmbedcrypto.a"

"$BUILD/tls12_v2g_extensions" "$CERTS/certs/cpoCertChain.pem" \
    "$CERTS/private_keys/seccLeaf_unencrypted.key"
