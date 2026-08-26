#!/usr/bin/env bash
# Host-only test for classify_client_hello in isotls.cpp.
#
# Extracts the parser from the module source (fails to compile if the source
# markers drift), captures real ClientHellos from the local OpenSSL via
# capture_hellos.py and runs the checks in client_hello_parser_main.cpp.inc under
# ASan/UBSan.
set -eu
cd "$(dirname "$0")"

BUILD=$(mktemp -d)
trap 'rm -rf "$BUILD"' EXIT

# enum ClientHelloVersion up to (excluding) the apply_group_policy comment
sed -n '/^enum class ClientHelloVersion/,/^\/\/ Peeks the ClientHello/p' ../../isotls.cpp | head -n -1 > "$BUILD/classify.inc"
if ! grep -q "classify_client_hello" "$BUILD/classify.inc"; then
    echo "extraction of classify_client_hello from isotls.cpp failed, markers drifted"
    exit 1
fi

python3 capture_hellos.py "$BUILD/hellos.inc"

g++ -Wall -Wextra -O1 -fsanitize=address,undefined -I"$BUILD" \
    -o "$BUILD/client_hello_parser" -x c++ client_hello_parser_main.cpp.inc

"$BUILD/client_hello_parser"
