#!/bin/bash
# Host only test for the mbedTLS TLS 1.3 OCSP stapling patch
# (patches/lib-builder 0006). Applies the patch to a fresh mbedTLS
# 3.6.6 checkout, builds the library, runs _stapling_server.c with
# the -20 dev PKI and probes with openssl s_client:
#   1. client with -status gets the stapled response, Cert Status good, chain verified
#   2. client without -status gets no extension and the hook stays uncalled

set -eu

cd "$(dirname "$0")"
BUILD=/tmp/opencode/iso15118_tests/stapling
PATCH="$(realpath ../../../../patches/lib-builder/esp-idf/components/mbedtls/mbedtls/0006-Add-server-side-OCSP-stapling-for-TLS-1.3-via-weak-c.patch)"
CERTS="$(realpath ../tools/certs/output/iso20)"
PORT=${PORT:-18443}

[ -f "$CERTS/certs/cpoCertChain.pem" ] || { echo "dev PKI missing, run ../certs/generate_certs.sh first"; exit 1; }

rm -rf "$BUILD"
mkdir -p "$BUILD"

if [ -n "${MBEDTLS_SRC:-}" ]; then
    cp -r "$MBEDTLS_SRC" "$BUILD/mbedtls"
    rm -rf "$BUILD/mbedtls/.git"
else
    git clone --quiet --depth 1 --branch v3.6.6 --recurse-submodules --shallow-submodules \
        https://github.com/Mbed-TLS/mbedtls "$BUILD/mbedtls"
fi

grep -q "3.6.6" "$BUILD/mbedtls/include/mbedtls/build_info.h" || { echo "mbedTLS source is not 3.6.6"; exit 1; }

echo "--- applying patch 0006 ---"
patch -d "$BUILD/mbedtls" -p1 --forward < "$PATCH"

echo "--- building mbedTLS (takes a minute) ---"
make -C "$BUILD/mbedtls" lib -j"$(nproc)" > /dev/null

gcc -Wall -Wextra -O1 -I "$BUILD/mbedtls/include" -o "$BUILD/stapling_server" \
    -x c _stapling_server.c -x none \
    "$BUILD/mbedtls/library/libmbedtls.a" "$BUILD/mbedtls/library/libmbedx509.a" "$BUILD/mbedtls/library/libmbedcrypto.a"

echo "--- generating an OCSP response for the dev leaf ---"
SERIAL=$(openssl x509 -in "$CERTS/certs/seccLeafCert.pem" -noout -serial | cut -d= -f2)
ENDDATE=$(date -d "$(openssl x509 -in "$CERTS/certs/seccLeafCert.pem" -noout -enddate | cut -d= -f2)" -u +%y%m%d%H%M%SZ)
printf 'V\t%s\t\t%s\tunknown\t/CN=x\n' "$ENDDATE" "$SERIAL" > "$BUILD/index.txt"
openssl ocsp -index "$BUILD/index.txt" \
    -CA "$CERTS/certs/cpoSubCA2Cert.pem" \
    -rsigner "$CERTS/certs/cpoSubCA2Cert.pem" \
    -rkey "$CERTS/private_keys/cpoSubCA2.key" -passin pass:12345 \
    -issuer "$CERTS/certs/cpoSubCA2Cert.pem" \
    -cert "$CERTS/certs/seccLeafCert.pem" \
    -reqout "$BUILD/req.der" -respout "$BUILD/leaf_ocsp.der" -ndays 7 > /dev/null 2>&1

"$BUILD/stapling_server" "$PORT" "$CERTS/certs/cpoCertChain.pem" \
    "$CERTS/private_keys/seccLeaf_unencrypted.key" "$BUILD/leaf_ocsp.der" \
    > "$BUILD/server.log" 2>&1 &
SERVER_PID=$!
trap 'kill $SERVER_PID 2>/dev/null' EXIT
sleep 1

fails=0

echo "--- probe with status_request ---"
echo | openssl s_client -connect "127.0.0.1:$PORT" -tls1_3 -status \
    -CAfile "$CERTS/certs/v2gRootCACert.pem" > "$BUILD/with_status.txt" 2>/dev/null || true
if grep -q "OCSP Response Status: successful" "$BUILD/with_status.txt" \
   && grep -q "Cert Status: good" "$BUILD/with_status.txt" \
   && grep -q "Verify return code: 0 (ok)" "$BUILD/with_status.txt"; then
    echo "ok   stapled response delivered and good"
else
    echo "FAIL stapled response missing or bad"; fails=$((fails+1))
fi

echo "--- probe without status_request ---"
echo | openssl s_client -connect "127.0.0.1:$PORT" -tls1_3 \
    -CAfile "$CERTS/certs/v2gRootCACert.pem" > "$BUILD/without_status.txt" 2>/dev/null || true
if grep -q "Verify return code: 0 (ok)" "$BUILD/without_status.txt" \
   && ! grep -q "OCSP Response Status" "$BUILD/without_status.txt"; then
    echo "ok   handshake without staple"
else
    echo "FAIL unexpected staple or failed handshake"; fails=$((fails+1))
fi

if grep -q "hook calls 3" "$BUILD/server.log" && grep -q "hook calls 0" "$BUILD/server.log"; then
    echo "ok   hook called per chain certificate and only on request"
else
    echo "FAIL unexpected hook call counts"; cat "$BUILD/server.log"; fails=$((fails+1))
fi

if [ "$fails" -eq 0 ]; then echo PASS; else echo "FAIL ($fails failures)"; exit 1; fi
