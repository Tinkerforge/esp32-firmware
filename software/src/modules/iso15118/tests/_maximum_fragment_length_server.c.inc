// TLS 1.3 test server for maximum_fragment_length negotiation.
// Usage: maximum_fragment_length_server <port> <chain.pem> <key.pem>

#define MBEDTLS_ALLOW_PRIVATE_ACCESS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "ssl_misc.h"

static unsigned char *read_file(const char *path, size_t *len)
{
    FILE *file = fopen(path, "rb");
    long size;
    unsigned char *buffer;

    if (file == NULL) {
        fprintf(stderr, "cannot open %s\n", path);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer = calloc(1, (size_t) size + 1);
    if (buffer == NULL || fread(buffer, 1, (size_t) size, file) != (size_t) size) {
        exit(1);
    }
    fclose(file);
    *len = (size_t) size + 1;
    return buffer;
}

static int debug_send(void *context, const unsigned char *buffer, size_t length)
{
    if (length >= 5) {
        printf("wire record: type %u length %u\n", buffer[0],
               ((unsigned int) buffer[3] << 8) | buffer[4]);
        fflush(stdout);
    }
    return mbedtls_net_send(context, buffer, length);
}

int main(int argc, char **argv)
{
    unsigned char response[1536];
    unsigned char *chain_pem;
    unsigned char *key_pem;
    size_t chain_len;
    size_t key_len;
    size_t written = 0;
    int records = 0;
    int ret;
    char error[128];
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context drbg;
    mbedtls_x509_crt certificate;
    mbedtls_pk_context key;
    mbedtls_ssl_config config;
    mbedtls_ssl_context ssl;
    mbedtls_net_context listener;
    mbedtls_net_context client;

    if (argc != 4) {
        fprintf(stderr, "usage: %s <port> <chain.pem> <key.pem>\n", argv[0]);
        return 1;
    }

    chain_pem = read_file(argv[2], &chain_len);
    key_pem = read_file(argv[3], &key_len);
    memset(response, 'R', sizeof(response));

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&drbg);
    mbedtls_x509_crt_init(&certificate);
    mbedtls_pk_init(&key);
    mbedtls_ssl_config_init(&config);
    mbedtls_ssl_init(&ssl);
    mbedtls_net_init(&listener);
    mbedtls_net_init(&client);

    if (mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &entropy, NULL, 0) != 0 ||
        mbedtls_x509_crt_parse(&certificate, chain_pem, chain_len) != 0 ||
        mbedtls_pk_parse_key(&key, key_pem, key_len, NULL, 0,
                             mbedtls_ctr_drbg_random, &drbg) != 0 ||
        mbedtls_ssl_config_defaults(&config, MBEDTLS_SSL_IS_SERVER,
                                    MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT) != 0) {
        fprintf(stderr, "setup failed\n");
        return 1;
    }

    mbedtls_ssl_conf_min_tls_version(&config, MBEDTLS_SSL_VERSION_TLS1_3);
    mbedtls_ssl_conf_max_tls_version(&config, MBEDTLS_SSL_VERSION_TLS1_3);
    mbedtls_ssl_conf_rng(&config, mbedtls_ctr_drbg_random, &drbg);
    if (mbedtls_ssl_conf_own_cert(&config, &certificate, &key) != 0 ||
        mbedtls_ssl_setup(&ssl, &config) != 0 ||
        mbedtls_net_bind(&listener, "127.0.0.1", argv[1],
                         MBEDTLS_NET_PROTO_TCP) != 0 ||
        mbedtls_net_accept(&listener, &client, NULL, 0, NULL) != 0) {
        fprintf(stderr, "listener setup failed\n");
        return 1;
    }

    mbedtls_ssl_set_bio(&ssl, &client, debug_send, mbedtls_net_recv, NULL);
    ret = mbedtls_ssl_handshake(&ssl);
    if (ret != 0) {
        mbedtls_strerror(ret, error, sizeof(error));
        printf("handshake failed: %d -0x%04x %s\n", ret, (unsigned int) -ret, error);
        return 2;
    }

    printf("maximum input fragment: %zu\n", mbedtls_ssl_get_input_max_frag_len(&ssl));
    printf("maximum output fragment: %zu\n", mbedtls_ssl_get_output_max_frag_len(&ssl));

    while (written < sizeof(response)) {
        ret = mbedtls_ssl_write(&ssl, response + written, sizeof(response) - written);
        if (ret <= 0) {
            mbedtls_strerror(ret, error, sizeof(error));
            fprintf(stderr, "write failed: %s\n", error);
            return 1;
        }
        written += (size_t) ret;
        records++;
    }
    printf("response: %zu bytes in %d records\n", written, records);
    fflush(stdout);
    mbedtls_ssl_close_notify(&ssl);
    return 0;
}
