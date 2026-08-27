#!/bin/bash
# Host-only regression test for the mbedTLS alert-classification patch.

set -eu

cd "$(dirname "$0")"
BUILD=/tmp/opencode/iso15118_tests/alert_classification
PATCH="$(realpath ../../../../patches/lib-builder/esp-idf/components/mbedtls/mbedtls/0007-Preserve-actionable-TLS-fatal-alert-causes.rawpatch)"

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
patch -d "$BUILD/mbedtls" -p1 --forward < "$PATCH"

cat > "$BUILD/test.c" <<'EOF'
#define MBEDTLS_ALLOW_PRIVATE_ACCESS
#include <mbedtls/ssl.h>
#include "ssl_misc.h"

#include <stdio.h>
#include <string.h>

static int check(unsigned char alert, int expected)
{
    unsigned char message[2] = { MBEDTLS_SSL_ALERT_LEVEL_FATAL, alert };
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    int result;

    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    conf.MBEDTLS_PRIVATE(transport) = MBEDTLS_SSL_TRANSPORT_STREAM;
    ssl.MBEDTLS_PRIVATE(conf) = &conf;
    ssl.MBEDTLS_PRIVATE(in_msgtype) = MBEDTLS_SSL_MSG_ALERT;
    ssl.MBEDTLS_PRIVATE(in_msglen) = sizeof(message);
    ssl.MBEDTLS_PRIVATE(in_msg) = message;
    result = mbedtls_ssl_handle_message_type(&ssl);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);

    if (result != expected) {
        fprintf(stderr, "alert %u: got %d, expected %d\n", alert, result, expected);
        return 1;
    }
    return 0;
}

int main(void)
{
    int failed = 0;
    failed |= check(MBEDTLS_SSL_ALERT_MSG_PROTOCOL_VERSION, MBEDTLS_ERR_SSL_ALERT_PROTOCOL_VERSION);
    failed |= check(MBEDTLS_SSL_ALERT_MSG_HANDSHAKE_FAILURE, MBEDTLS_ERR_SSL_ALERT_CIPHER_SUITE);
    failed |= check(MBEDTLS_SSL_ALERT_MSG_INSUFFICIENT_SECURITY, MBEDTLS_ERR_SSL_ALERT_CIPHER_SUITE);
    failed |= check(MBEDTLS_SSL_ALERT_MSG_INTERNAL_ERROR, MBEDTLS_ERR_SSL_FATAL_ALERT_MESSAGE);
    return failed;
}
EOF

make -C "$BUILD/mbedtls" lib -j"$(nproc)" > /dev/null
gcc -Wall -Wextra -Werror -I "$BUILD/mbedtls/include" -I "$BUILD/mbedtls/library" \
    -o "$BUILD/test" "$BUILD/test.c" \
    "$BUILD/mbedtls/library/libmbedtls.a" "$BUILD/mbedtls/library/libmbedx509.a" "$BUILD/mbedtls/library/libmbedcrypto.a"
"$BUILD/test"
echo PASS
