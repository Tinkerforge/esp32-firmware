// TLS 1.3 test server for the mbedTLS OCSP stapling patch. Serves one
// handshake per connection with the -20 dev SECC chain and staples the
// OCSP response file for the leaf via the strong hook override.
// Usage: stapling_server <port> <chain.pem> <key.pem> <leaf_ocsp.der>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"

static unsigned char *ocsp_der;
static size_t ocsp_der_len;
static int hook_calls;

int mbedtls_ssl_tls13_ocsp_response_cb(mbedtls_ssl_context *ssl, size_t index,
                                       const unsigned char **der, size_t *der_len);

int mbedtls_ssl_tls13_ocsp_response_cb(mbedtls_ssl_context *ssl, size_t index,
                                       const unsigned char **der, size_t *der_len)
{
    (void) ssl;
    hook_calls++;
    if (index != 0) {
        return -1;
    }
    *der = ocsp_der;
    *der_len = ocsp_der_len;
    return 0;
}

static unsigned char *read_file(const char *path, size_t *len, int pem)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, "cannot open %s\n", path);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *buf = calloc(1, (size_t) size + 1);
    if (fread(buf, 1, (size_t) size, f) != (size_t) size) {
        exit(1);
    }
    fclose(f);
    *len = (size_t) size + (pem ? 1 : 0);
    return buf;
}

int main(int argc, char **argv)
{
    if (argc != 5) {
        fprintf(stderr, "usage: %s <port> <chain.pem> <key.pem> <ocsp.der>\n", argv[0]);
        return 1;
    }

    size_t chain_len, key_len;
    unsigned char *chain_pem = read_file(argv[2], &chain_len, 1);
    unsigned char *key_pem = read_file(argv[3], &key_len, 1);
    ocsp_der = read_file(argv[4], &ocsp_der_len, 0);

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context drbg;
    mbedtls_x509_crt crt;
    mbedtls_pk_context pk;
    mbedtls_ssl_config conf;
    mbedtls_ssl_context ssl;
    mbedtls_net_context listen_fd, client_fd;

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&drbg);
    mbedtls_x509_crt_init(&crt);
    mbedtls_pk_init(&pk);
    mbedtls_ssl_config_init(&conf);
    mbedtls_ssl_init(&ssl);
    mbedtls_net_init(&listen_fd);
    mbedtls_net_init(&client_fd);

    if (mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &entropy, NULL, 0) != 0 ||
        mbedtls_x509_crt_parse(&crt, chain_pem, chain_len) != 0 ||
        mbedtls_pk_parse_key(&pk, key_pem, key_len, NULL, 0, mbedtls_ctr_drbg_random, &drbg) != 0) {
        fprintf(stderr, "setup failed\n");
        return 1;
    }

    if (mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER,
                                    MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT) != 0) {
        return 1;
    }
    mbedtls_ssl_conf_min_tls_version(&conf, MBEDTLS_SSL_VERSION_TLS1_3);
    mbedtls_ssl_conf_max_tls_version(&conf, MBEDTLS_SSL_VERSION_TLS1_3);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &drbg);
    if (mbedtls_ssl_conf_own_cert(&conf, &crt, &pk) != 0) {
        return 1;
    }
    if (mbedtls_ssl_setup(&ssl, &conf) != 0) {
        return 1;
    }

    if (mbedtls_net_bind(&listen_fd, "127.0.0.1", argv[1], MBEDTLS_NET_PROTO_TCP) != 0) {
        fprintf(stderr, "bind failed\n");
        return 1;
    }
    printf("listening on %s\n", argv[1]);
    fflush(stdout);

    for (;;) {
        if (mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL) != 0) {
            continue;
        }
        mbedtls_ssl_session_reset(&ssl);
        hook_calls = 0;
        mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

        int ret = mbedtls_ssl_handshake(&ssl);
        if (ret == 0) {
            printf("handshake ok, version %s, hook calls %d\n",
                   mbedtls_ssl_get_version(&ssl), hook_calls);
            mbedtls_ssl_close_notify(&ssl);
        } else {
            char err[128];
            mbedtls_strerror(ret, err, sizeof(err));
            printf("handshake failed: -0x%04x %s, hook calls %d\n",
                   (unsigned) -ret, err, hook_calls);
        }
        fflush(stdout);
        mbedtls_net_free(&client_fd);
    }
}
