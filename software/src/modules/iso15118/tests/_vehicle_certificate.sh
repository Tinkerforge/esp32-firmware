#!/bin/bash
# Production vehicle policy in real Mbed TLS TLS 1.3 mutual-auth handshakes.
set -eu
cd "$(dirname "$0")"
BUILD=$(mktemp -d "${TMPDIR:-/tmp}/iso15118-vehicle.XXXXXXXX")
trap 'if [ "${KEEP_BUILD:-0}" = 1 ]; then printf "%s\n" "$BUILD"; else rm -rf -- "$BUILD"; fi' EXIT
if [ -n "${MBEDTLS_SRC:-}" ]; then
    cp -r "$MBEDTLS_SRC" "$BUILD/mbedtls"
else
    git clone --quiet --depth 1 --branch v3.6.6 --recurse-submodules --shallow-submodules \
        https://github.com/Mbed-TLS/mbedtls "$BUILD/mbedtls"
fi
PATCH_ROOT=$(realpath ../../../../patches/lib-builder/esp-idf/components/mbedtls/mbedtls)
git -C "$BUILD/mbedtls" apply --ignore-space-change "$PATCH_ROOT/0021-Fix-TLS-1.3-server-rejection-alert-keys.patch"
make -C "$BUILD/mbedtls" lib -j"$(nproc)" > /dev/null
# Compile the production verification callback/worker with a small RTOS/OCPP
# adapter. Only the post-handshake OCPP handoff is excluded in this host harness.
python3 - "$BUILD/verification.inc" <<'PY'
from pathlib import Path
import sys
text = Path('../isotls/isotls_verification.cpp').read_text()
text = '\n'.join(line for line in text.splitlines() if not line.startswith('#include'))
start = text.index('void ISOTLS::hand_off_vehicle_chain()')
end = text.index('void ISOTLS::verify_intermediate_certs()', start)
text = text[:start] + text[end:]
Path(sys.argv[1]).write_text('#define MODULE_OCPP_AVAILABLE() 1\n' + text)
PY
g++ -Wall -Wextra -Werror -std=c++17 -O1 -g \
    -I "$BUILD/mbedtls/include" -I ../isotls -I "$BUILD" \
    -o "$BUILD/server" -x c++ _vehicle_certificate_server.cpp.inc ../isotls/vehicle_certificate.cpp -x none \
    "$BUILD/mbedtls/library/libmbedtls.a" "$BUILD/mbedtls/library/libmbedx509.a" "$BUILD/mbedtls/library/libmbedcrypto.a"
python3 _vehicle_certificate.py "$BUILD/server"
VEHICLE_BACKPRESSURE=1 python3 _vehicle_certificate.py "$BUILD/server"
