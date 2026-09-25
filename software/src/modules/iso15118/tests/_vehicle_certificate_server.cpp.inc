#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <fstream>
#include <iterator>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/net_sockets.h"
#include "vehicle_certificate.h"
#include "mbedtls/error.h"
#include "_vehicle_verification_host.h"
#include "verification.inc"

struct Output {
    mbedtls_net_context *peer;
    unsigned calls = 0;
    unsigned retries = 0;
    bool backpressure;
};

static int send_output(void *ctx, const unsigned char *buf, size_t len)
{
    auto *out = static_cast<Output *>(ctx);
    if (out->backpressure) {
        if (++out->calls % 2 == 1) {
            ++out->retries;
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        }
        if (len > 17) len = 17;
    }
    return mbedtls_net_send(out->peer, buf, len);
}

static int receive_input(void *ctx, unsigned char *buf, size_t len)
{
    return mbedtls_net_recv(static_cast<Output *>(ctx)->peer, buf, len);
}

#define CHECK(call) do { int r = (call); if (r != 0) { \
    fprintf(stderr, "%s: -0x%x\n", #call, -r); return 1; } } while (0)

static int verify(void *ctx, mbedtls_x509_crt *cert, int depth, uint32_t *flags)
{
    int ret = ISOTLS::cert_verify(ctx, cert, depth, flags);
    char url[256];
    if (ISOVehicleCertificate::ocsp_url(*cert, url, sizeof(url))) {
        printf("SOURCE %d %s\n", depth, url);
    }
    return ret;
}

int main(int argc, char **argv)
{
    if (argc == 4 && strcmp(argv[1], "verify-chain") == 0) {
        mbedtls_x509_crt chain, root;
        mbedtls_x509_crt_init(&chain);
        mbedtls_x509_crt_init(&root);
        CHECK(mbedtls_x509_crt_parse_file(&chain, argv[2]));
        CHECK(mbedtls_x509_crt_parse_file(&root, argv[3]));
        ISOTLS policy;
        policy.trusted_ca_iso20 = &root;
        std::vector<mbedtls_x509_crt *> certs;
        for (auto *cert = &chain; cert != nullptr; cert = cert->next) certs.push_back(cert);
        certs.push_back(&root);
        uint32_t all_flags = 0;
        for (size_t i = certs.size(); i-- > 0;) {
            uint32_t flags = 0;
            CHECK(ISOTLS::cert_verify(&policy, certs[i], static_cast<int>(i), &flags));
            all_flags |= flags;
        }
        puts(all_flags == 0 && policy.context.intermediates_valid ? "VALID" : "REJECTED");
        mbedtls_x509_crt_free(&chain);
        mbedtls_x509_crt_free(&root);
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "extensions") == 0) {
        std::ifstream in(argv[2], std::ios::binary);
        std::vector<unsigned char> der{std::istreambuf_iterator<char>(in), {}};
        mbedtls_x509_crt cert{};
        cert.v3_ext.p = der.data();
        cert.v3_ext.len = der.size();
        char url[256];
        if (ISOVehicleCertificate::ocsp_url(cert, url, sizeof(url))) {
            puts(url);
        } else {
            puts("REJECTED");
        }
        return 0;
    }
    if (argc != 6) return 2;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cert, root;
    mbedtls_pk_context key;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context rng;
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_x509_crt_init(&cert);
    mbedtls_x509_crt_init(&root);
    mbedtls_pk_init(&key);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&rng);
    CHECK(mbedtls_ctr_drbg_seed(&rng, mbedtls_entropy_func, &entropy,
                               reinterpret_cast<const unsigned char *>("vehicle"), 7));
    CHECK(mbedtls_x509_crt_parse_file(&cert, argv[1]));
    CHECK(mbedtls_pk_parse_keyfile(&key, argv[2], nullptr, mbedtls_ctr_drbg_random, &rng));
    CHECK(mbedtls_x509_crt_parse_file(&root, argv[3]));
    CHECK(mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER,
                                      MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT));
    mbedtls_ssl_conf_min_tls_version(&conf, MBEDTLS_SSL_VERSION_TLS1_3);
    mbedtls_ssl_conf_max_tls_version(&conf, MBEDTLS_SSL_VERSION_TLS1_3);
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&conf, &root, nullptr);
    ISOTLS policy;
    policy.trusted_ca_iso20 = &root;
    ocpp.required = atoi(argv[4]) != 0;
    mbedtls_ssl_conf_verify(&conf, verify, &policy);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &rng);
    mbedtls_ssl_conf_new_session_tickets(&conf, 0);
    CHECK(mbedtls_ssl_conf_own_cert(&conf, &cert, &key));
    CHECK(mbedtls_ssl_setup(&ssl, &conf));
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(listener, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0 || listen(listener, 1) != 0) return 4;
    socklen_t addr_len = sizeof(addr);
    if (getsockname(listener, reinterpret_cast<sockaddr *>(&addr), &addr_len) != 0) return 4;
    printf("PORT %u\n", ntohs(addr.sin_port));
    fflush(stdout);
    for (int i = 0; i < atoi(argv[5]); ++i) {
        mbedtls_net_context peer;
        mbedtls_net_init(&peer);
        peer.fd = accept(listener, nullptr, nullptr);
        timeval timeout{5, 0};
        setsockopt(peer.fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        CHECK(mbedtls_ssl_session_reset(&ssl));
        policy.reset();
        Output output{&peer, 0, 0, getenv("VEHICLE_BACKPRESSURE") != nullptr};
        mbedtls_ssl_set_bio(&ssl, &output, send_output, receive_input, nullptr);
        int ret;
        do {
            ret = mbedtls_ssl_handshake(&ssl);
        } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE || ret == MBEDTLS_ERR_SSL_WANT_READ);
        if (ret == 0 && !policy.context.intermediates_valid) {
            mbedtls_ssl_send_alert_message(&ssl, MBEDTLS_SSL_ALERT_LEVEL_FATAL, MBEDTLS_SSL_ALERT_MSG_BAD_CERT);
            ret = MBEDTLS_ERR_X509_CERT_VERIFY_FAILED;
        }
        if (ret == 0) {
            unsigned char request[4];
            CHECK(mbedtls_ssl_read(&ssl, request, sizeof(request)) == 4 && memcmp(request, "ping", 4) == 0 ? 0 : -1);
            do {
                ret = mbedtls_ssl_write(&ssl, reinterpret_cast<const unsigned char *>("pong"), 4);
            } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);
            CHECK(ret == 4 ? 0 : -1);
            do {
                ret = mbedtls_ssl_close_notify(&ssl);
            } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);
            puts("COMPLETE");
        } else {
            printf("REJECTED %x\n", mbedtls_ssl_get_verify_result(&ssl));
        }
        if (output.backpressure && output.retries == 0) return 5;
        fflush(stdout);
        // Avoid a TCP reset discarding the alert while the client still sends
        // its Finished/application data. Drain until peer closes after alert.
        if (ret != 0) {
            shutdown(peer.fd, SHUT_WR);
            unsigned char discard[4096];
            while (recv(peer.fd, discard, sizeof(discard), 0) > 0) {}
        }
        mbedtls_net_free(&peer);
    }
    close(listener);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_x509_crt_free(&cert);
    mbedtls_x509_crt_free(&root);
    mbedtls_pk_free(&key);
    mbedtls_ctr_drbg_free(&rng);
    mbedtls_entropy_free(&entropy);
    return 0;
}
