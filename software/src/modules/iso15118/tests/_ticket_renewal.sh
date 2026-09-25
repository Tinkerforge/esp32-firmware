#!/bin/bash
set -eu
cd "$(dirname "$0")"
BUILD=$(mktemp -d "${TMPDIR:-/tmp}/iso15118-tickets.XXXXXXXX")
trap 'if [ "${KEEP_BUILD:-0}" = 1 ]; then printf "%s\n" "$BUILD"; else rm -rf -- "$BUILD"; fi' EXIT
PATCH_ROOT=$(realpath ../../../../patches/lib-builder/esp-idf/components/mbedtls/mbedtls)
if [ -n "${MBEDTLS_SRC:-}" ]; then
    cp -r "$MBEDTLS_SRC" "$BUILD/mbedtls"
else
    git clone --quiet --depth 1 --branch v3.6.6 --recurse-submodules --shallow-submodules \
        https://github.com/Mbed-TLS/mbedtls "$BUILD/mbedtls"
fi
patch -d "$BUILD/mbedtls" -p1 --forward < "$PATCH_ROOT/0001-ssl_tls-Add-work-around-for-broken-asymmetric-buffer.patch"
for patch_file in "$PATCH_ROOT"/library/001[4-9]-*.rawpatch; do
    patch -d "$BUILD/mbedtls/library" --forward < "$patch_file"
done
patch -d "$BUILD/mbedtls" -p1 --fuzz=0 --forward < "$PATCH_ROOT/0020-Add-post-handshake-TLS-1.3-ticket-issuance.patch"
git -C "$BUILD/mbedtls" apply --ignore-space-change "$PATCH_ROOT/0021-Fix-TLS-1.3-server-rejection-alert-keys.patch"
make -C "$BUILD/mbedtls" clean > /dev/null
make -C "$BUILD/mbedtls" lib -j"$(nproc)" > /dev/null
gcc -Wall -Wextra -Werror -O1 -I "$BUILD/mbedtls/include" -I "$BUILD/mbedtls/library" \
    -c -o "$BUILD/server.o" -x c _ticket_renewal_server.c.inc
g++ -Wall -Wextra -Werror -O1 -I "$BUILD/mbedtls/include" -I ../isotls \
    -o "$BUILD/server" -x c++ _ticket_policy.cpp.inc ../isotls/ticket_policy.cpp -x none "$BUILD/server.o" \
    "$BUILD/mbedtls/library/libmbedtls.a" "$BUILD/mbedtls/library/libmbedx509.a" "$BUILD/mbedtls/library/libmbedcrypto.a"
python3 _ticket_renewal.py "$BUILD/server" ../tools/certs/output/iso20
