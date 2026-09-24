// Test-only client linked with a ClientHello extension-construction patch.
#define MBEDTLS_ALLOW_PRIVATE_ACCESS
#include <stdio.h>
#include <stdlib.h>
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"

int main(int argc, char **argv)
{
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context drbg;
    mbedtls_x509_crt ca;
    mbedtls_ssl_config config;
    mbedtls_ssl_context ssl;
    mbedtls_net_context connection;
    unsigned char data[2048];
    size_t received = 0;
    int ret;

    if (argc != 4) {
        return 1;
    }
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&drbg);
    mbedtls_x509_crt_init(&ca);
    mbedtls_ssl_config_init(&config);
    mbedtls_ssl_init(&ssl);
    mbedtls_net_init(&connection);
    if (mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &entropy, NULL, 0) != 0 ||
        mbedtls_x509_crt_parse_file(&ca, argv[2]) != 0 ||
        mbedtls_ssl_config_defaults(&config, MBEDTLS_SSL_IS_CLIENT,
                                   MBEDTLS_SSL_TRANSPORT_STREAM,
                                   MBEDTLS_SSL_PRESET_DEFAULT) != 0) {
        return 1;
    }
    mbedtls_ssl_conf_min_tls_version(&config, MBEDTLS_SSL_VERSION_TLS1_3);
    mbedtls_ssl_conf_max_tls_version(&config, MBEDTLS_SSL_VERSION_TLS1_3);
    mbedtls_ssl_conf_rng(&config, mbedtls_ctr_drbg_random, &drbg);
    mbedtls_ssl_conf_ca_chain(&config, &ca, NULL);
    // The test patch encodes order and malformed values in this private field.
    config.mfl_code = (unsigned char) atoi(argv[3]);
    if (mbedtls_ssl_setup(&ssl, &config) != 0 ||
        mbedtls_ssl_set_hostname(&ssl, "SECCCert") != 0 ||
        mbedtls_net_connect(&connection, "127.0.0.1", argv[1], MBEDTLS_NET_PROTO_TCP) != 0) {
        return 1;
    }
    mbedtls_ssl_set_bio(&ssl, &connection, mbedtls_net_send, mbedtls_net_recv, NULL);
    ret = mbedtls_ssl_handshake(&ssl);
    if (ret != 0) {
        fprintf(stderr, "client handshake failed: %d\n", ret);
        return 2;
    }
    if (mbedtls_ssl_get_verify_result(&ssl) != 0 ||
        ssl.session->mfl_code != MBEDTLS_SSL_MAX_FRAG_LEN_NONE ||
        ssl.session->record_size_limit != 16384) {
        fprintf(stderr, "incorrect negotiated limits or certificate verification\n");
        return 3;
    }
    // A server MFL response is also rejected by the client's extension parser:
    // the test-only offer deliberately does not set its sent-extension mask.
    while ((ret = mbedtls_ssl_read(&ssl, data, sizeof(data))) > 0) {
        for (int i = 0; i < ret; ++i) {
            if (data[i] != 'R') {
                return 4;
            }
        }
        received += (size_t) ret;
    }
    if (ret != MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY || received != 1536) {
        fprintf(stderr, "response failed: %d, %zu bytes\n", ret, received);
        return 5;
    }
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&config);
    mbedtls_net_free(&connection);
    mbedtls_x509_crt_free(&ca);
    mbedtls_ctr_drbg_free(&drbg);
    mbedtls_entropy_free(&entropy);
    return 0;
}
