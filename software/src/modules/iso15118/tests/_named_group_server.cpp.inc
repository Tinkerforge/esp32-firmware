#define MBEDTLS_ALLOW_PRIVATE_ACCESS
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/x509_crt.h"
#include "group_policy.inc"

#define CHECK(call) do { int r = (call); if (r != 0) { \
    fprintf(stderr, "%s: -0x%x\n", #call, -r); return 1; } } while (0)

int main(int argc, char **argv)
{
    if (argc != 6) return 2;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cert;
    mbedtls_pk_context key;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context rng;
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_x509_crt_init(&cert);
    mbedtls_pk_init(&key);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&rng);
    CHECK(mbedtls_ctr_drbg_seed(&rng, mbedtls_entropy_func, &entropy,
                               reinterpret_cast<const unsigned char *>("groups"), 6));
    CHECK(mbedtls_x509_crt_parse_file(&cert, argv[1]));
    CHECK(mbedtls_pk_parse_keyfile(&key, argv[2], nullptr, mbedtls_ctr_drbg_random, &rng));
    CHECK(mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER,
                                      MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT));
    const bool tls13 = strcmp(argv[3], "13") == 0;
    mbedtls_ssl_conf_min_tls_version(&conf, tls13 ? MBEDTLS_SSL_VERSION_TLS1_3 : MBEDTLS_SSL_VERSION_TLS1_2);
    mbedtls_ssl_conf_max_tls_version(&conf, tls13 ? MBEDTLS_SSL_VERSION_TLS1_3 : MBEDTLS_SSL_VERSION_TLS1_2);
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &rng);
    mbedtls_ssl_conf_session_tickets_cb(&conf, nullptr, nullptr, nullptr);
    mbedtls_ssl_conf_new_session_tickets(&conf, 0);
    CHECK(mbedtls_ssl_conf_own_cert(&conf, &cert, &key));
    uint16_t groups[4] = {};
    make_group_policy(groups, true, true, true);
    mbedtls_ssl_conf_groups(&conf, groups);
    CHECK(mbedtls_ssl_setup(&ssl, &conf));
    // Match production: setup/reset precedes ClientHello-based policy selection.
    CHECK(mbedtls_ssl_session_reset(&ssl));
    make_group_policy(groups, tls13, atoi(argv[4]) != 0, atoi(argv[5]) != 0);

    int listener = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(listener, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0 || listen(listener, 1) != 0) return 4;
    socklen_t addr_len = sizeof(addr);
    if (getsockname(listener, reinterpret_cast<sockaddr *>(&addr), &addr_len) != 0) return 4;
    printf("PORT %u\n", ntohs(addr.sin_port));
    fflush(stdout);
    mbedtls_net_context peer;
    mbedtls_net_init(&peer);
    peer.fd = accept(listener, nullptr, nullptr);
    close(listener);
    timeval timeout = {5, 0};
    setsockopt(peer.fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    mbedtls_ssl_set_bio(&ssl, &peer, mbedtls_net_send, mbedtls_net_recv, nullptr);
    int ret = mbedtls_ssl_handshake(&ssl);
    if (ret == 0) {
        unsigned char request[4];
        CHECK(mbedtls_ssl_read(&ssl, request, sizeof(request)) == 4 && memcmp(request, "ping", 4) == 0 ? 0 : -1);
        CHECK(mbedtls_ssl_write(&ssl, reinterpret_cast<const unsigned char *>("pong"), 4) == 4 ? 0 : -1);
        mbedtls_ssl_close_notify(&ssl);
        puts("COMPLETE");
    } else {
        printf("handshake result -0x%x\n", -ret);
    }
    mbedtls_net_free(&peer);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_x509_crt_free(&cert);
    mbedtls_pk_free(&key);
    mbedtls_ctr_drbg_free(&rng);
    mbedtls_entropy_free(&entropy);
    return 0;
}
