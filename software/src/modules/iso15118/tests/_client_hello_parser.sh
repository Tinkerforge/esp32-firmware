#!/usr/bin/env bash
# Host-only test for classify_client_hello in isotls_policy.cpp.
#
# Extracts the parser from the module source captures real ClientHellos from the local OpenSSL via
# _capture_hellos.py and runs the checks in _client_hello_parser_main.cpp.inc under ASan/UBSan.
set -eu
cd "$(dirname "$0")"

BUILD=$(mktemp -d)
trap 'rm -rf "$BUILD"' EXIT

# enum ClientHelloVersion up to (excluding) the apply_group_policy comment
sed -n '/^enum class ClientHelloVersion/,/^\/\/ Peeks the ClientHello/p' ../isotls/isotls_policy.cpp | head -n -1 > "$BUILD/classify.inc"
if ! grep -q "classify_client_hello" "$BUILD/classify.inc"; then
    echo "extraction of classify_client_hello from isotls_policy.cpp failed, markers drifted"
    exit 1
fi

python3 _capture_hellos.py "$BUILD/hellos.inc"

g++ -Wall -Wextra -O1 -fsanitize=address,undefined -I"$BUILD" \
    -o "$BUILD/client_hello_parser" -x c++ _client_hello_parser_main.cpp.inc

"$BUILD/client_hello_parser"
