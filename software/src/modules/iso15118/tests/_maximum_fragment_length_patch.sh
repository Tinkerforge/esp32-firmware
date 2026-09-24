#!/bin/bash
# Host-only regression test for TLS 1.3 maximum_fragment_length support.

set -eu

cd "$(dirname "$0")"
readonly BUILD="$(mktemp -d "${TMPDIR:-/tmp}/iso15118-maximum-fragment-length.XXXXXXXX")"
SERVER_PID=
PROXY_PID=
cleanup() {
    [ -z "$SERVER_PID" ] || kill "$SERVER_PID" 2>/dev/null || true
    [ -z "$PROXY_PID" ] || kill "$PROXY_PID" 2>/dev/null || true
    rm -rf -- "$BUILD"
}
trap cleanup EXIT

PATCH="$(realpath ../../../../patches/lib-builder/esp-idf/components/mbedtls/mbedtls/library/0014-Add-TLS-1.3-maximum-fragment-length-server-support.rawpatch)"
PATCH_ROOT="$(dirname "$(dirname "$PATCH")")"
CERTS="$(realpath ../tools/certs/output/iso20)"
PORT=${PORT:-19443}

[ -f "$CERTS/certs/cpoCertChain.pem" ] || { echo "dev PKI missing, run ../certs/generate_certs.sh first"; exit 1; }

if [ -n "${MBEDTLS_SRC:-}" ]; then
    cp -r "$MBEDTLS_SRC" "$BUILD/mbedtls"
    rm -rf -- "$BUILD/mbedtls/.git"
else
    git clone --quiet --depth 1 --branch v3.6.6 --recurse-submodules --shallow-submodules \
        https://github.com/Mbed-TLS/mbedtls "$BUILD/mbedtls"
fi

grep -q "3.6.6" "$BUILD/mbedtls/include/mbedtls/build_info.h" || { echo "mbedTLS source is not 3.6.6"; exit 1; }
# Reproduce the actual interaction, including the old buffer workaround.
patch -d "$BUILD/mbedtls" -p1 --forward < "$PATCH_ROOT/0001-ssl_tls-Add-work-around-for-broken-asymmetric-buffer.patch"
patch -d "$BUILD/mbedtls/library" --forward < "$PATCH"
patch -d "$BUILD/mbedtls/library" --forward < "$PATCH_ROOT/library/0015-Enforce-negotiated-input-fragment-limits.rawpatch"
patch -d "$BUILD/mbedtls/library" --forward < "$PATCH_ROOT/library/0016-Prefer-record-size-limit-over-maximum-fragment-length.rawpatch"
patch -d "$BUILD/mbedtls/library" --forward < "$PATCH_ROOT/library/0017-Resume-fragmented-TLS-1.3-handshake-output.rawpatch"
python3 "$BUILD/mbedtls/scripts/config.py" set MBEDTLS_SSL_RECORD_SIZE_LIMIT
python3 "$BUILD/mbedtls/scripts/config.py" set MBEDTLS_SSL_VARIABLE_BUFFER_LENGTH
python3 "$BUILD/mbedtls/scripts/config.py" set MBEDTLS_SSL_OUT_CONTENT_LEN 4096
make -C "$BUILD/mbedtls" lib -j"$(nproc)" > /dev/null
gcc -Wall -Wextra -Werror -O1 -I "$BUILD/mbedtls/include" -I "$BUILD/mbedtls/library" \
    -o "$BUILD/server" -x c _maximum_fragment_length_server.c.inc -x none \
    "$BUILD/mbedtls/library/libmbedtls.a" \
    "$BUILD/mbedtls/library/libmbedx509.a" \
    "$BUILD/mbedtls/library/libmbedcrypto.a"

start_server() {
    local port=$1
    local log=$2
    "$BUILD/server" "$port" "$CERTS/certs/cpoCertChain.pem" \
        "$CERTS/private_keys/seccLeaf_unencrypted.key" > "$log" 2>&1 &
    SERVER_PID=$!
    sleep 0.2
}

for backpressure in 0 1; do
for length in 512 1024 2048 4096; do
    log="$BUILD/server-$length.log"
    output="$BUILD/client-$length.log"
    MFL_BACKPRESSURE=$backpressure start_server "$PORT" "$log"
    openssl s_client -connect "127.0.0.1:$PORT" -tls1_3 -maxfraglen "$length" \
        -CAfile "$CERTS/certs/v2gRootCACert.pem" -tlsextdebug \
        < /dev/null > "$output" 2>&1 || true
    if ! wait "$SERVER_PID"; then
        cat "$log"
        cat "$output"
        exit 1
    fi
    SERVER_PID=
    if [ "$backpressure" = 1 ]; then
        grep -q "handshake WANT_WRITE" "$log" || { cat "$log"; exit 1; }
        grep -q "handshake short write" "$log" || { cat "$log"; exit 1; }
        if [ "$length" -le 1024 ]; then
            grep -q "fragment retry:" "$log" || { cat "$log"; exit 1; }
        fi
    fi
    grep -q "TLS server extension \"max fragment length\" (id=1), len=1" "$output" || { cat "$output"; exit 1; }
    grep -q "Verify return code: 0 (ok)" "$output" || { cat "$output"; exit 1; }
    grep -q "maximum input fragment: $length" "$log" || { cat "$log"; exit 1; }
    grep -q "maximum output fragment: $length" "$log" || { cat "$log"; exit 1; }
    maximum_payload=$((length - 1))
    expected_records=$(( (1536 + maximum_payload - 1) / maximum_payload ))
    grep -q "response: 1536 bytes in $expected_records records" "$log" || { cat "$log"; exit 1; }
    maximum_wire_record=$((length + 16))
    while read -r _ _ _ _ _ record_length; do
        if [ "$record_length" -gt "$maximum_wire_record" ]; then
            echo "FAIL encrypted record $record_length exceeded negotiated limit $maximum_wire_record"
            cat "$log"
            exit 1
        fi
    done < <(grep "wire record: type 23" "$log")
    echo "ok   TLS 1.3 maximum_fragment_length $length backpressure=$backpressure"
done
done

run_negative() {
    local mode=$1
    local server_port=$2
    local proxy_port=$3
    local log="$BUILD/server-$mode.log"
    start_server "$server_port" "$log"
    python3 _mutate_client_hello.py "$mode" "$proxy_port" "$server_port" &
    PROXY_PID=$!
    sleep 0.2
    openssl s_client -connect "127.0.0.1:$proxy_port" -tls1_3 -maxfraglen 512 \
        < /dev/null > "$BUILD/client-$mode.log" 2>&1 || true
    wait "$PROXY_PID"
    PROXY_PID=
    if wait "$SERVER_PID"; then
        echo "FAIL $mode unexpectedly completed the handshake"
        exit 1
    fi
    SERVER_PID=
    grep -q "handshake failed: -26112 " "$log" || { cat "$log"; exit 1; }
    echo "ok   $mode rejected with illegal_parameter"
}

log="$BUILD/server-abort.log"
MFL_BACKPRESSURE=abort start_server "$PORT" "$log"
timeout 15 openssl s_client -connect "127.0.0.1:$PORT" -tls1_3 -maxfraglen 512 \
    < /dev/null > "$BUILD/client-abort.log" 2>&1 || true
wait "$SERVER_PID"
SERVER_PID=
grep -q "aborted fragmented handshake reset successfully" "$log" || { cat "$log"; exit 1; }
echo "ok   reset during fragmented handshake backpressure"

run_negative invalid-mfl $((PORT + 1)) $((PORT + 2))
python3 _maximum_fragment_length_receive.py "$BUILD/server" "$CERTS"

# Build a client with deliberately unusual but transcript-authenticated offers.
# The server binary above retains the production-only patches and 16 KiB input.
patch -d "$BUILD/mbedtls/library" --forward < _record_size_limit_client.rawpatch
make -C "$BUILD/mbedtls" lib -j"$(nproc)" > /dev/null
gcc -Wall -Wextra -Werror -O1 -I "$BUILD/mbedtls/include" \
    -o "$BUILD/client" -x c _record_size_limit_client.c.inc -x none \
    "$BUILD/mbedtls/library/libmbedtls.a" \
    "$BUILD/mbedtls/library/libmbedx509.a" \
    "$BUILD/mbedtls/library/libmbedcrypto.a"
for code in 0 1 129 5 133 6 134; do
    log="$BUILD/server-precedence-$code.log"
    start_server "$PORT" "$log"
    if ! timeout 15 "$BUILD/client" "$PORT" "$CERTS/certs/v2gRootCACert.pem" "$code"; then
        cat "$log"
        exit 1
    fi
    wait "$SERVER_PID"
    SERVER_PID=
    grep -q "maximum input fragment: 16384" "$log" || { cat "$log"; exit 1; }
    grep -q "response: 1536 bytes in 2 records" "$log" || { cat "$log"; exit 1; }
    # RSL=1024 includes inner content type; GCM adds 16 wire bytes.
    while read -r _ _ _ _ _ record_length; do
        [ "$record_length" -le 1040 ] || { cat "$log"; exit 1; }
    done < <(grep "wire record: type 23" "$log")
    echo "ok   TLS 1.3 record_size_limit precedence, MFL test code $code"
done
echo PASS
