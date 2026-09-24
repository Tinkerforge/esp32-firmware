#!/bin/bash
# Host regression for production group policy and TLS 1.3 selection patch 0019.
set -eu
cd "$(dirname "$0")"
BUILD=$(mktemp -d "${TMPDIR:-/tmp}/iso15118-groups.XXXXXXXX")
trap 'rm -rf -- "$BUILD"' EXIT
PATCH_ROOT=$(realpath ../../../../patches/lib-builder/esp-idf/components/mbedtls/mbedtls)
if [ -n "${MBEDTLS_SRC:-}" ]; then
    cp -r "$MBEDTLS_SRC" "$BUILD/mbedtls"
else
    git clone --quiet --depth 1 --branch v3.6.6 --recurse-submodules --shallow-submodules \
        https://github.com/Mbed-TLS/mbedtls "$BUILD/mbedtls"
fi
patch -d "$BUILD/mbedtls" -p1 --forward < "$PATCH_ROOT/0001-ssl_tls-Add-work-around-for-broken-asymmetric-buffer.patch"
for patch_file in "$PATCH_ROOT"/library/001[4-8]-*.rawpatch; do
    patch -d "$BUILD/mbedtls/library" --forward < "$patch_file"
done
if [ "${APPLY_GROUP_PATCH:-1}" = 1 ]; then
    patch -d "$BUILD/mbedtls/library" --forward < "$PATCH_ROOT/library/0019-Honor-TLS-1.3-server-group-preference.rawpatch"
fi
make -C "$BUILD/mbedtls" lib -j"$(nproc)" > /dev/null
# Exercise the production policy function, not a test copy of its mapping.
python3 - "$BUILD/group_policy.inc" <<'PY'
import pathlib, sys
source = pathlib.Path('../isotls/isotls_policy.cpp').read_text()
start = source.index('static void make_group_policy(')
end = source.index('\n}', start) + 2
pathlib.Path(sys.argv[1]).write_text(source[start:end] + '\n')
PY
g++ -Wall -Wextra -Werror -O1 -I "$BUILD" -I "$BUILD/mbedtls/include" -I "$BUILD/mbedtls/library" \
    -o "$BUILD/server" -x c++ _named_group_server.cpp.inc -x none \
    "$BUILD/mbedtls/library/libmbedtls.a" "$BUILD/mbedtls/library/libmbedx509.a" "$BUILD/mbedtls/library/libmbedcrypto.a"
python3 _named_group_preference.py "$BUILD/server" ../tools/certs/output/iso20
