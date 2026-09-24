#define MBEDTLS_ALLOW_PRIVATE_ACCESS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_ticket.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/x509_crt.h"
#include "ssl_misc.h"

#define CHECK(x) do { int r = (x); if (r != 0) { fprintf(stderr, "%s: -0x%x\n", #x, -r); return 1; } } while (0)
#define ASSERT(x) CHECK((x) ? 0 : -1)
static int ticket_writes;
int policy_setup(int (*rng)(void *, unsigned char *, size_t), void *ctx);
void policy_reset(void);
int policy_write(void *, const mbedtls_ssl_session *, unsigned char *, const unsigned char *, size_t *, uint32_t *);
int policy_parse(void *, mbedtls_ssl_session *, unsigned char *, size_t);
int policy_test(void);
static int ticket_write(void *ctx, const mbedtls_ssl_session *session, unsigned char *start,
                        const unsigned char *end, size_t *len, uint32_t *lifetime)
{
    ticket_writes++;
    return policy_write(ctx, session, start, end, len, lifetime);
}
typedef struct { mbedtls_net_context net; int pressure; int calls; } transport;
static int send_cb(void *ctx, const unsigned char *buf, size_t len)
{
    transport *t = ctx;
    if (t->pressure) {
        if (++t->calls % 2) return MBEDTLS_ERR_SSL_WANT_WRITE;
        if (len > 17) len = 17;
    }
    return mbedtls_net_send(&t->net, buf, len);
}
static int recv_cb(void *ctx, unsigned char *buf, size_t len)
{
    return mbedtls_net_recv(&((transport *)ctx)->net, buf, len);
}
int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cert;
    mbedtls_pk_context key;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context rng;
    mbedtls_ssl_ticket_context tickets;
    mbedtls_ssl_init(&ssl); mbedtls_ssl_config_init(&conf);
    mbedtls_x509_crt_init(&cert); mbedtls_pk_init(&key);
    mbedtls_entropy_init(&entropy); mbedtls_ctr_drbg_init(&rng);
    mbedtls_ssl_ticket_init(&tickets);
    ASSERT(mbedtls_ssl_send_session_ticket(NULL) == MBEDTLS_ERR_SSL_BAD_INPUT_DATA);
    ASSERT(mbedtls_ssl_send_session_ticket(&ssl) == MBEDTLS_ERR_SSL_BAD_INPUT_DATA);
    CHECK(mbedtls_ctr_drbg_seed(&rng, mbedtls_entropy_func, &entropy, (const unsigned char *)"tickets", 7));
    CHECK(mbedtls_x509_crt_parse_file(&cert, argv[1]));
    CHECK(mbedtls_pk_parse_keyfile(&key, argv[2], NULL, mbedtls_ctr_drbg_random, &rng));
    CHECK(mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT));
    mbedtls_ssl_conf_min_tls_version(&conf, MBEDTLS_SSL_VERSION_TLS1_3);
    mbedtls_ssl_conf_max_tls_version(&conf, MBEDTLS_SSL_VERSION_TLS1_3);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &rng);
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
    CHECK(mbedtls_ssl_conf_own_cert(&conf, &cert, &key));
    CHECK(mbedtls_ssl_ticket_setup(&tickets, mbedtls_ctr_drbg_random, &rng, MBEDTLS_CIPHER_AES_256_GCM, 3600));
    CHECK(policy_setup(mbedtls_ctr_drbg_random, &rng));
    mbedtls_ssl_conf_session_tickets_cb(&conf, ticket_write, policy_parse, &tickets);
    mbedtls_ssl_conf_new_session_tickets(&conf, 1);
    mbedtls_ssl_conf_tls13_key_exchange_modes(&conf, MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_EPHEMERAL | MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_PSK_EPHEMERAL);
    CHECK(mbedtls_ssl_setup(&ssl, &conf));
    ASSERT(mbedtls_ssl_send_session_ticket(&ssl) == MBEDTLS_ERR_SSL_BAD_INPUT_DATA);
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET; addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ASSERT(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) == 0);
    ASSERT(listen(listener, 1) == 0);
    socklen_t addr_len = sizeof(addr);
    ASSERT(getsockname(listener, (struct sockaddr *)&addr, &addr_len) == 0);
    printf("PORT %u\n", ntohs(addr.sin_port)); fflush(stdout);
    for (int connection = 0; connection < 3; ++connection) {
        transport t = {0};
        t.net.fd = accept(listener, NULL, NULL);
        struct timeval timeout = {5, 0};
        setsockopt(t.net.fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        CHECK(mbedtls_ssl_session_reset(&ssl));
        policy_reset();
        ASSERT(!ssl.handshake->ticket_renewal_pending);
        mbedtls_ssl_set_bio(&ssl, &t, send_cb, recv_cb, NULL);
        CHECK(mbedtls_ssl_handshake(&ssl));
        ASSERT(ssl.handshake->resume == (connection == 1));
        unsigned char request;
        ASSERT(mbedtls_ssl_read(&ssl, &request, 1) == 1);
        ASSERT(mbedtls_ssl_write(&ssl, (const unsigned char *)"A", 1) == 1);
        // Disabled configuration cannot be bypassed via the explicit API.
        mbedtls_ssl_conf_new_session_tickets(&conf, 0);
        ASSERT(mbedtls_ssl_send_session_ticket(&ssl) == MBEDTLS_ERR_SSL_BAD_INPUT_DATA);
        mbedtls_ssl_conf_new_session_tickets(&conf, 1);
        // A pending app write must finish before renewal starts.
        t.pressure = 1;
        ASSERT(mbedtls_ssl_write(&ssl, (const unsigned char *)"B", 1) == MBEDTLS_ERR_SSL_WANT_WRITE);
        ASSERT(mbedtls_ssl_send_session_ticket(&ssl) == MBEDTLS_ERR_SSL_BAD_INPUT_DATA);
        int ret;
        do { ret = mbedtls_ssl_write(&ssl, (const unsigned char *)"B", 1); } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);
        ASSERT(ret == 1);
        for (int renewal = 0; renewal < 2; ++renewal) {
            int before = ticket_writes;
            int retries = 0;
            do {
                ret = mbedtls_ssl_send_session_ticket(&ssl);
                if (ret == MBEDTLS_ERR_SSL_WANT_WRITE) retries++;
                if (connection == 2 && retries) break;
            } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);
            ASSERT(ticket_writes == before + 1);
            ASSERT(retries > 0);
            if (connection == 2) break;
            CHECK(ret);
            ASSERT(!ssl.handshake->ticket_renewal_pending);
        }
        if (connection == 2) {
            CHECK(mbedtls_ssl_session_reset(&ssl));
            ASSERT(!ssl.handshake->ticket_renewal_pending && ssl.out_left == 0);
            puts("reset pending renewal OK");
        } else {
            t.pressure = 0;
            ASSERT(mbedtls_ssl_write(&ssl, (const unsigned char *)"C", 1) == 1);
            ASSERT(mbedtls_ssl_read(&ssl, &request, 1) == 1);
            CHECK(mbedtls_ssl_close_notify(&ssl));
        }
        mbedtls_net_free(&t.net);
    }
    close(listener);
    mbedtls_ssl_free(&ssl); mbedtls_ssl_config_free(&conf);
    mbedtls_ssl_ticket_free(&tickets);
    mbedtls_x509_crt_free(&cert); mbedtls_pk_free(&key);
    mbedtls_ctr_drbg_free(&rng); mbedtls_entropy_free(&entropy);
    CHECK(policy_test());
    puts("PASS");
    return 0;
}
