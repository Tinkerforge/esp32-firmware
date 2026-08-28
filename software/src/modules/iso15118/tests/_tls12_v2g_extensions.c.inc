#define MBEDTLS_ALLOW_PRIVATE_ACCESS

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/pk.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_ciphersuites.h"
#include "mbedtls/x509_crt.h"
#include "ssl_misc.h"

static const unsigned char trusted_ca_payload[] = {
    0x00, 0x15, 0x01, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23
};
static const unsigned char status_v2_payload[] = {
    0x00, 0x07, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00
};
static const unsigned char response_list[] = {
    0x00, 0x00, 0x05, 0x30, 0x03, 0x02, 0x01, 0x01,
    0x00, 0x00, 0x05, 0x30, 0x03, 0x02, 0x01, 0x02
};

enum request_mode {
    REQUEST_NONE,
    REQUEST_VALID,
    REQUEST_DUPLICATE_TRUSTED_CA,
    REQUEST_DUPLICATE_STATUS_V2,
};

typedef struct {
    enum request_mode request_mode;
    int client_fd;
    int server_fd;
    int trusted_calls;
    int status_calls;
    int availability_calls;
    int response_calls;
    int cert_calls;
    int hooks_before_cert;
    int availability_after_cert;
    int response_after_cert;
    int selected_response;
    int status_want_write;
    int status_records;
    int status_after_certificate;
    int last_handshake_type;
    int server_ccs_seen;
    int result;
    mbedtls_ssl_context *client_ssl;
    unsigned char pending[MBEDTLS_SSL_IN_CONTENT_LEN + 5];
    size_t pending_len;
    size_t pending_off;
    unsigned char server_hello[1024];
    size_t server_hello_len;
    unsigned char certificate_status[1024];
    size_t certificate_status_len;
} test_context;

static test_context *active_test;
static pthread_mutex_t rng_mutex = PTHREAD_MUTEX_INITIALIZER;

static int locked_random(void *ctx, unsigned char *output, size_t len)
{
    int ret;
    pthread_mutex_lock(&rng_mutex);
    ret = mbedtls_ctr_drbg_random(ctx, output, len);
    pthread_mutex_unlock(&rng_mutex);
    return ret;
}

static uint32_t get_u24(const unsigned char *p)
{
    return ((uint32_t) p[0] << 16) | ((uint32_t) p[1] << 8) | p[2];
}

static void put_u24(unsigned char *p, size_t value)
{
    p[0] = (unsigned char) (value >> 16);
    p[1] = (unsigned char) (value >> 8);
    p[2] = (unsigned char) value;
}

static int write_all(int fd, const unsigned char *buf, size_t len)
{
    while (len != 0) {
        ssize_t ret = send(fd, buf, len, 0);
        if (ret < 0 && errno == EINTR) {
            continue;
        }
        if (ret <= 0) {
            return -1;
        }
        buf += ret;
        len -= (size_t) ret;
    }
    return 0;
}

static int read_all(int fd, unsigned char *buf, size_t len)
{
    while (len != 0) {
        ssize_t ret = recv(fd, buf, len, 0);
        if (ret < 0 && errno == EINTR) {
            continue;
        }
        if (ret <= 0) {
            return -1;
        }
        buf += ret;
        len -= (size_t) ret;
    }
    return 0;
}

int mbedtls_ssl_tls12_trusted_ca_keys_cb(mbedtls_ssl_context *ssl,
                                         const unsigned char *data,
                                         size_t data_len)
{
    (void) ssl;
    active_test->trusted_calls++;
    return data_len == sizeof(trusted_ca_payload) &&
           memcmp(data, trusted_ca_payload, data_len) == 0 ? 0 : -1;
}

int mbedtls_ssl_tls12_status_request_v2_cb(
    mbedtls_ssl_context *ssl, const unsigned char *data, size_t data_len)
{
    (void) ssl;
    active_test->status_calls++;
    return data_len == sizeof(status_v2_payload) &&
           memcmp(data, status_v2_payload, data_len) == 0 ? 0 : -1;
}

int mbedtls_ssl_tls12_status_request_v2_available_cb(mbedtls_ssl_context *ssl)
{
    (void) ssl;
    active_test->availability_calls++;
    active_test->availability_after_cert =
        active_test->cert_calls == 1 && active_test->selected_response == 2;
    return active_test->availability_after_cert ? 0 : -1;
}

int mbedtls_ssl_tls12_status_request_v2_response_cb(
    mbedtls_ssl_context *ssl, const unsigned char **responses,
    size_t *responses_len)
{
    (void) ssl;
    active_test->response_calls++;
    active_test->response_after_cert =
        active_test->availability_calls == 1 &&
        active_test->availability_after_cert &&
        active_test->selected_response == 2;
    if (!active_test->response_after_cert) {
        return -1;
    }
    *responses = response_list;
    *responses_len = sizeof(response_list);
    return 0;
}

static int cert_cb(mbedtls_ssl_context *ssl)
{
    (void) ssl;
    active_test->cert_calls++;
    active_test->hooks_before_cert =
        active_test->trusted_calls == 1 && active_test->status_calls == 1;
    active_test->selected_response = 2;
    return 0;
}

static int server_send(void *ctx_ptr, const unsigned char *buf, size_t len)
{
    test_context *ctx = ctx_ptr;
    int is_status = len >= 9 && buf[0] == MBEDTLS_SSL_MSG_HANDSHAKE &&
                    buf[5] == MBEDTLS_SSL_HS_CERTIFICATE_STATUS;

    if (is_status && !ctx->status_want_write) {
        ctx->status_want_write = 1;
        return MBEDTLS_ERR_SSL_WANT_WRITE;
    }
    if (is_status) {
        ctx->status_records++;
    }
    if (write_all(ctx->server_fd, buf, len) != 0) {
        return MBEDTLS_ERR_NET_SEND_FAILED;
    }
    return (int) len;
}

static int server_recv(void *ctx_ptr, unsigned char *buf, size_t len)
{
    test_context *ctx = ctx_ptr;
    ssize_t ret;
    do {
        ret = recv(ctx->server_fd, buf, len, 0);
    } while (ret < 0 && errno == EINTR);
    if (ret == 0) {
        return MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY;
    }
    if (ret < 0) {
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }
    return (int) ret;
}

static int client_send(void *ctx_ptr, const unsigned char *buf, size_t len)
{
    test_context *ctx = ctx_ptr;
    unsigned char record[MBEDTLS_SSL_OUT_CONTENT_LEN + 128];
    const unsigned char *to_send = buf;
    size_t send_len = len;

    if (ctx->request_mode != REQUEST_NONE && len >= 9 &&
        buf[0] == MBEDTLS_SSL_MSG_HANDSHAKE &&
        buf[5] == MBEDTLS_SSL_HS_CLIENT_HELLO) {
        size_t hs_len = get_u24(buf + 6);
        size_t body_end = 9 + hs_len;
        size_t p = 9 + 2 + 32;
        size_t ext_len_off;
        size_t old_ext_len;
        size_t vector_len;
        size_t added = 8 + sizeof(trusted_ca_payload) + sizeof(status_v2_payload);

        if (ctx->request_mode == REQUEST_DUPLICATE_TRUSTED_CA) {
            added += 4 + sizeof(trusted_ca_payload);
        } else if (ctx->request_mode == REQUEST_DUPLICATE_STATUS_V2) {
            added += 4 + sizeof(status_v2_payload);
        }

        if (body_end != len || len + added > sizeof(record)) {
            return MBEDTLS_ERR_NET_SEND_FAILED;
        }
        p += 1 + buf[p];
        vector_len = ((size_t) buf[p] << 8) + buf[p + 1];
        p += 2 + vector_len;
        p += 1 + buf[p];
        ext_len_off = p;
        old_ext_len = ((size_t) buf[p] << 8) + buf[p + 1];
        if (p + 2 + old_ext_len != body_end) {
            return MBEDTLS_ERR_NET_SEND_FAILED;
        }

        memcpy(record, buf, len);
        p = len;
        record[p++] = 0x00;
        record[p++] = MBEDTLS_TLS_EXT_STATUS_REQUEST_V2;
        record[p++] = 0x00;
        record[p++] = sizeof(status_v2_payload);
        memcpy(record + p, status_v2_payload, sizeof(status_v2_payload));
        p += sizeof(status_v2_payload);
        record[p++] = 0x00;
        record[p++] = MBEDTLS_TLS_EXT_TRUSTED_CA_KEYS;
        record[p++] = 0x00;
        record[p++] = sizeof(trusted_ca_payload);
        memcpy(record + p, trusted_ca_payload, sizeof(trusted_ca_payload));
        p += sizeof(trusted_ca_payload);
        if (ctx->request_mode == REQUEST_DUPLICATE_TRUSTED_CA) {
            record[p++] = 0x00;
            record[p++] = MBEDTLS_TLS_EXT_TRUSTED_CA_KEYS;
            record[p++] = 0x00;
            record[p++] = sizeof(trusted_ca_payload);
            memcpy(record + p, trusted_ca_payload, sizeof(trusted_ca_payload));
            p += sizeof(trusted_ca_payload);
        } else if (ctx->request_mode == REQUEST_DUPLICATE_STATUS_V2) {
            record[p++] = 0x00;
            record[p++] = MBEDTLS_TLS_EXT_STATUS_REQUEST_V2;
            record[p++] = 0x00;
            record[p++] = sizeof(status_v2_payload);
            memcpy(record + p, status_v2_payload, sizeof(status_v2_payload));
            p += sizeof(status_v2_payload);
        }

        record[3] = (unsigned char) ((len - 5 + added) >> 8);
        record[4] = (unsigned char) (len - 5 + added);
        put_u24(record + 6, hs_len + added);
        old_ext_len += added;
        record[ext_len_off] = (unsigned char) (old_ext_len >> 8);
        record[ext_len_off + 1] = (unsigned char) old_ext_len;
        send_len = p;
        to_send = record;

        if (mbedtls_ssl_reset_checksum(ctx->client_ssl) != 0 ||
            ctx->client_ssl->handshake->update_checksum(
                ctx->client_ssl, record + 5, send_len - 5) != 0) {
            return MBEDTLS_ERR_NET_SEND_FAILED;
        }
    }

    if (write_all(ctx->client_fd, to_send, send_len) != 0) {
        return MBEDTLS_ERR_NET_SEND_FAILED;
    }
    return (int) len;
}

static int load_server_record(test_context *ctx)
{
    for (;;) {
        size_t record_len;
        unsigned char *record = ctx->pending;

        if (read_all(ctx->client_fd, record, 5) != 0) {
            fprintf(stderr, "client capture: failed reading record header after handshake type %d\n",
                    ctx->last_handshake_type);
            return -1;
        }
        record_len = ((size_t) record[3] << 8) + record[4];
        if (record_len + 5 > sizeof(ctx->pending) ||
            read_all(ctx->client_fd, record + 5, record_len) != 0) {
            fprintf(stderr, "client capture: failed reading record body type %u len %zu\n",
                    record[0], record_len);
            return -1;
        }

        if (record[0] == MBEDTLS_SSL_MSG_CHANGE_CIPHER_SPEC) {
            ctx->server_ccs_seen = 1;
        } else if (!ctx->server_ccs_seen &&
                   record[0] == MBEDTLS_SSL_MSG_HANDSHAKE && record_len >= 4) {
            size_t hs_len = get_u24(record + 6) + 4;
            if (hs_len != record_len) {
                fprintf(stderr, "client capture: handshake %u has %zu bytes in %zu-byte record\n",
                        record[5], hs_len, record_len);
                return -1;
            }
            if (record[5] == MBEDTLS_SSL_HS_SERVER_HELLO) {
                memcpy(ctx->server_hello, record + 5, hs_len);
                ctx->server_hello_len = hs_len;
            } else if (record[5] == MBEDTLS_SSL_HS_CERTIFICATE_STATUS) {
                ctx->status_after_certificate =
                    ctx->last_handshake_type == MBEDTLS_SSL_HS_CERTIFICATE;
                memcpy(ctx->certificate_status, record + 5, hs_len);
                ctx->certificate_status_len = hs_len;
                if (ctx->client_ssl->handshake->update_checksum(
                        ctx->client_ssl, record + 5, hs_len) != 0) {
                    fprintf(stderr, "client capture: transcript update failed\n");
                    return -1;
                }
                continue;
            }
            ctx->last_handshake_type = record[5];
        }

        ctx->pending_len = record_len + 5;
        ctx->pending_off = 0;
        return 0;
    }
}

static int client_recv(void *ctx_ptr, unsigned char *buf, size_t len)
{
    test_context *ctx = ctx_ptr;
    size_t available;

    if (ctx->pending_off == ctx->pending_len && load_server_record(ctx) != 0) {
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }
    available = ctx->pending_len - ctx->pending_off;
    if (len > available) {
        len = available;
    }
    memcpy(buf, ctx->pending + ctx->pending_off, len);
    ctx->pending_off += len;
    return (int) len;
}

static int parse_server_hello_extensions(const test_context *ctx,
                                         int *trusted, int *status)
{
    const unsigned char *msg = ctx->server_hello;
    size_t msg_len = ctx->server_hello_len;
    size_t p = 4 + 2 + 32;
    size_t end;

    *trusted = 0;
    *status = 0;
    if (msg_len < p + 1) {
        return -1;
    }
    p += 1 + msg[p];
    p += 3;
    if (p == msg_len) {
        return 0;
    }
    if (p + 2 > msg_len) {
        return -1;
    }
    end = p + 2 + ((size_t) msg[p] << 8) + msg[p + 1];
    p += 2;
    if (end != msg_len) {
        return -1;
    }
    while (p < end) {
        unsigned type;
        size_t ext_len;
        if (end - p < 4) {
            return -1;
        }
        type = ((unsigned) msg[p] << 8) + msg[p + 1];
        ext_len = ((size_t) msg[p + 2] << 8) + msg[p + 3];
        p += 4;
        if (ext_len > end - p) {
            return -1;
        }
        if (type == MBEDTLS_TLS_EXT_TRUSTED_CA_KEYS && ext_len == 0) {
            *trusted = 1;
        }
        if (type == MBEDTLS_TLS_EXT_STATUS_REQUEST_V2 && ext_len == 0) {
            *status = 1;
        }
        p += ext_len;
    }
    return 0;
}

static int check_status_message(const test_context *ctx)
{
    const unsigned char *msg = ctx->certificate_status;
    size_t len = ctx->certificate_status_len;
    return len == 8 + sizeof(response_list) &&
           msg[0] == MBEDTLS_SSL_HS_CERTIFICATE_STATUS &&
           get_u24(msg + 1) == 4 + sizeof(response_list) &&
           msg[4] == 2 && get_u24(msg + 5) == sizeof(response_list) &&
           memcmp(msg + 8, response_list, sizeof(response_list)) == 0;
}

typedef struct {
    test_context *test;
    mbedtls_x509_crt *cert;
    mbedtls_pk_context *key;
    mbedtls_ctr_drbg_context *drbg;
} server_args;

static void *server_thread(void *arg_ptr)
{
    server_args *args = arg_ptr;
    test_context *test = args->test;
    mbedtls_ssl_config conf;
    mbedtls_ssl_context ssl;
    const int suites[] = { MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256, 0 };
    int ret;

    mbedtls_ssl_config_init(&conf);
    mbedtls_ssl_init(&ssl);
    ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret == 0) {
        mbedtls_ssl_conf_min_tls_version(&conf, MBEDTLS_SSL_VERSION_TLS1_2);
        mbedtls_ssl_conf_max_tls_version(&conf, MBEDTLS_SSL_VERSION_TLS1_2);
        mbedtls_ssl_conf_ciphersuites(&conf, suites);
        mbedtls_ssl_conf_rng(&conf, locked_random, args->drbg);
        mbedtls_ssl_conf_cert_cb(&conf, cert_cb);
        ret = mbedtls_ssl_conf_own_cert(&conf, args->cert, args->key);
    }
    if (ret == 0) {
        ret = mbedtls_ssl_setup(&ssl, &conf);
    }
    if (ret == 0) {
        mbedtls_ssl_set_bio(&ssl, test, server_send, server_recv, NULL);
        do {
            ret = mbedtls_ssl_handshake(&ssl);
        } while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
                 ret == MBEDTLS_ERR_SSL_WANT_WRITE);
    }
    test->result = ret;
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    close(test->server_fd);
    return NULL;
}

static int run_handshake(enum request_mode request_mode, mbedtls_x509_crt *cert,
                         mbedtls_pk_context *key,
                         mbedtls_ctr_drbg_context *drbg)
{
    int fds[2];
    int ret;
    int trusted_echo;
    int status_echo;
    pthread_t thread;
    test_context test;
    server_args args;
    mbedtls_ssl_config conf;
    mbedtls_ssl_context ssl;
    const int suites[] = { MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256, 0 };

    memset(&test, 0, sizeof(test));
    test.request_mode = request_mode;
    test.selected_response = 0; /* The default/nonselected chain is unavailable. */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
        return -1;
    }
    test.client_fd = fds[0];
    test.server_fd = fds[1];
    active_test = &test;
    args.test = &test;
    args.cert = cert;
    args.key = key;
    args.drbg = drbg;

    mbedtls_ssl_config_init(&conf);
    mbedtls_ssl_init(&ssl);
    ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret == 0) {
        mbedtls_ssl_conf_min_tls_version(&conf, MBEDTLS_SSL_VERSION_TLS1_2);
        mbedtls_ssl_conf_max_tls_version(&conf, MBEDTLS_SSL_VERSION_TLS1_2);
        mbedtls_ssl_conf_ciphersuites(&conf, suites);
        mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
        mbedtls_ssl_conf_rng(&conf, locked_random, drbg);
        ret = mbedtls_ssl_setup(&ssl, &conf);
    }
    if (ret != 0) {
        return ret;
    }
    test.client_ssl = &ssl;
    mbedtls_ssl_set_bio(&ssl, &test, client_send, client_recv, NULL);

    if (pthread_create(&thread, NULL, server_thread, &args) != 0) {
        return -1;
    }
    ret = mbedtls_ssl_handshake(&ssl);
    close(fds[0]);
    pthread_join(thread, NULL);

    if (request_mode == REQUEST_DUPLICATE_TRUSTED_CA ||
        request_mode == REQUEST_DUPLICATE_STATUS_V2) {
        int duplicate_calls = request_mode == REQUEST_DUPLICATE_TRUSTED_CA ?
                              test.trusted_calls : test.status_calls;
        if (test.result != MBEDTLS_ERR_SSL_DECODE_ERROR || ret == 0 ||
            duplicate_calls != 1 || test.cert_calls != 0 ||
            test.availability_calls != 0 || test.response_calls != 0 ||
            test.server_hello_len != 0 || test.certificate_status_len != 0) {
            fprintf(stderr,
                    "duplicate rejection failed: mode=%d client=%d server=%d callbacks=%d\n",
                    request_mode, ret, test.result, duplicate_calls);
            ret = -1;
            goto cleanup;
        }
        printf("ok   duplicate %s rejected with decode_error before second callback\n",
               request_mode == REQUEST_DUPLICATE_TRUSTED_CA ?
               "trusted_ca_keys" : "status_request_v2");
        ret = 0;
        goto cleanup;
    }

    if (ret != 0 || test.result != 0 ||
        parse_server_hello_extensions(&test, &trusted_echo, &status_echo) != 0) {
        fprintf(stderr, "handshake failed: client=%d server=%d\n", ret, test.result);
        ret = -1;
        goto cleanup;
    }

    if (request_mode == REQUEST_VALID) {
        if (test.trusted_calls != 1 || test.status_calls != 1 ||
            test.availability_calls != 1 || test.response_calls != 1 ||
            test.cert_calls != 1 || !test.hooks_before_cert ||
            !test.availability_after_cert || !test.response_after_cert ||
            !trusted_echo || !status_echo || !check_status_message(&test) ||
            test.status_records != 1 || !test.status_want_write ||
            !test.status_after_certificate) {
            fprintf(stderr, "requested extension assertions failed\n");
            ret = -1;
            goto cleanup;
        }
        printf("ok   status-before-trusted order has exact payloads and accepted echoes\n");
        printf("ok   selected chain changes OCSP from unavailable to available\n");
        printf("ok   availability gates echo before the later response provider\n");
        printf("ok   CertificateStatus survived WANT_WRITE and both Finished checks\n");
    } else {
        if (test.trusted_calls != 0 || test.status_calls != 0 ||
            test.availability_calls != 0 || test.response_calls != 0 ||
            trusted_echo || status_echo || test.certificate_status_len != 0 ||
            test.status_records != 0) {
            fprintf(stderr, "no-request assertions failed\n");
            ret = -1;
            goto cleanup;
        }
        printf("ok   no request produces no echo and no CertificateStatus\n");
    }
    ret = 0;

cleanup:
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    return ret;
}

int main(int argc, char **argv)
{
    int ret;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context drbg;
    mbedtls_x509_crt cert;
    mbedtls_pk_context key;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <chain.pem> <key.pem>\n", argv[0]);
        return 2;
    }
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&drbg);
    mbedtls_x509_crt_init(&cert);
    mbedtls_pk_init(&key);

    ret = mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &entropy,
                                (const unsigned char *) "tls12-v2g", 9);
    if (ret == 0) {
        ret = mbedtls_x509_crt_parse_file(&cert, argv[1]);
    }
    if (ret == 0) {
        ret = mbedtls_pk_parse_keyfile(&key, argv[2], NULL,
                                       mbedtls_ctr_drbg_random, &drbg);
    }
    if (ret == 0) {
        ret = run_handshake(REQUEST_VALID, &cert, &key, &drbg);
    }
    if (ret == 0) {
        ret = run_handshake(REQUEST_NONE, &cert, &key, &drbg);
    }
    if (ret == 0) {
        ret = run_handshake(REQUEST_DUPLICATE_TRUSTED_CA, &cert, &key, &drbg);
    }
    if (ret == 0) {
        ret = run_handshake(REQUEST_DUPLICATE_STATUS_V2, &cert, &key, &drbg);
    }

    mbedtls_pk_free(&key);
    mbedtls_x509_crt_free(&cert);
    mbedtls_ctr_drbg_free(&drbg);
    mbedtls_entropy_free(&entropy);
    if (ret == 0) {
        puts("PASS");
        return 0;
    }
    return 1;
}
