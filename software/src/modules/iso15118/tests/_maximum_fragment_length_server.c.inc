// TLS 1.2/1.3 test server for maximum_fragment_length negotiation.
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

static int stall_next_send;
static int handshake_backpressure;
static int send_phase;
static size_t record_remaining;

static int debug_send(void *context, const unsigned char *buffer, size_t length)
{
    if (handshake_backpressure && send_phase++ % 2 == 0) {
        puts("handshake WANT_WRITE");
        return MBEDTLS_ERR_SSL_WANT_WRITE;
    }
    if (stall_next_send) {
        stall_next_send = 0;
        puts("simulated WANT_WRITE");
        return MBEDTLS_ERR_SSL_WANT_WRITE;
    }
    if (record_remaining == 0 && length >= 5) {
        printf("wire record: type %u length %u\n", buffer[0],
               ((unsigned int) buffer[3] << 8) | buffer[4]);
        fflush(stdout);
        record_remaining = 5 + ((size_t) buffer[3] << 8) + buffer[4];
        if (handshake_backpressure && length != record_remaining) {
            fputs("multiple queued records during handshake\n", stderr);
            return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
        }
    }
    if (handshake_backpressure && length > 37) {
        length = 37;
        puts("handshake short write");
    }
    int ret = mbedtls_net_send(context, buffer, length);
    if (ret > 0 && record_remaining >= (size_t) ret) {
        record_remaining -= (size_t) ret;
    }
    return ret;
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

    if (argc != 4 && argc != 6) {
        fprintf(stderr, "usage: %s <port> <chain.pem> <key.pem> [12|13 receive|send]\n", argv[0]);
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

    const mbedtls_ssl_protocol_version version = argc == 6 && strcmp(argv[4], "12") == 0
        ? MBEDTLS_SSL_VERSION_TLS1_2 : MBEDTLS_SSL_VERSION_TLS1_3;
    mbedtls_ssl_conf_min_tls_version(&config, version);
    mbedtls_ssl_conf_max_tls_version(&config, version);
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
    handshake_backpressure = getenv("MFL_BACKPRESSURE") != NULL &&
        strcmp(getenv("MFL_BACKPRESSURE"), "0") != 0;
    do {
        ret = mbedtls_ssl_handshake(&ssl);
        if (ret == MBEDTLS_ERR_SSL_WANT_WRITE && ssl.handshake->outgoing_hs != NULL) {
            printf("fragment retry: %zu/%zu\n", ssl.handshake->outgoing_hs_offset,
                   ssl.handshake->outgoing_hs_len);
            if (strcmp(getenv("MFL_BACKPRESSURE"), "abort") == 0) {
                if (mbedtls_ssl_session_reset(&ssl) != 0 ||
                    ssl.handshake->outgoing_hs != NULL ||
                    ssl.handshake->outgoing_hs_len != 0 ||
                    ssl.handshake->outgoing_hs_offset != 0) {
                    return 2;
                }
                mbedtls_ssl_free(&ssl);
                mbedtls_net_free(&client);
                puts("aborted fragmented handshake reset successfully");
                return 0;
            }
        }
    } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE || ret == MBEDTLS_ERR_SSL_WANT_READ);
    handshake_backpressure = 0;
    if (ret != 0) {
        mbedtls_strerror(ret, error, sizeof(error));
        printf("handshake failed: %d -0x%04x %s\n", ret, (unsigned int) -ret, error);
        return 2;
    }
    if (ssl.handshake != NULL && ssl.handshake->outgoing_hs != NULL) {
        fputs("handshake plaintext retained after successful flush\n", stderr);
        return 2;
    }

    printf("maximum input fragment: %zu\n", mbedtls_ssl_get_input_max_frag_len(&ssl));
    printf("maximum output fragment: %zu\n", mbedtls_ssl_get_output_max_frag_len(&ssl));
    fflush(stdout);

    if (argc == 6 && strcmp(argv[5], "receive") == 0) {
        unsigned char incoming[16384];
        stall_next_send = 1;
        do {
            ret = mbedtls_ssl_read(&ssl, incoming, sizeof(incoming));
        } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE || ret == MBEDTLS_ERR_SSL_WANT_READ);
        printf("received: %d\n", ret);
        if (ret > 0) {
            stall_next_send = 0;
            mbedtls_ssl_close_notify(&ssl);
            return 0;
        }
        return ret == MBEDTLS_ERR_SSL_INVALID_RECORD ? 3 : 4;
    }

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
