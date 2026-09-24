#include <cstdio>
#include <cstring>
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "ticket_policy.h"

#define REQUIRE(x) do { if (!(x)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #x); return 1; } } while (0)

// Shared by the wire server, so replacement-ticket resumption also exercises
// the production authenticated origin envelope rather than just library tickets.
static ISOTLSTicketPolicy policy;
extern "C" int policy_setup(int (*rng)(void *, unsigned char *, size_t), void *ctx)
{ return policy.setup(rng, ctx); }
extern "C" void policy_reset(void) { policy.reset(); }
extern "C" int policy_write(void *ctx, const mbedtls_ssl_session *session, unsigned char *start,
                              const unsigned char *end, size_t *len, uint32_t *lifetime)
{ return policy.write(static_cast<mbedtls_ssl_ticket_context *>(ctx), session, start, end, len, lifetime, 1000); }
extern "C" int policy_parse(void *ctx, mbedtls_ssl_session *session, unsigned char *buf, size_t len)
{ return policy.parse(static_cast<mbedtls_ssl_ticket_context *>(ctx), session, buf, len, 1000); }

extern "C" int policy_test(void)
{
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context rng;
    mbedtls_ssl_ticket_context ctx;
    mbedtls_ssl_session session, loaded;
    mbedtls_entropy_init(&entropy); mbedtls_ctr_drbg_init(&rng);
    mbedtls_ssl_ticket_init(&ctx); mbedtls_ssl_session_init(&session); mbedtls_ssl_session_init(&loaded);
    REQUIRE(mbedtls_ctr_drbg_seed(&rng, mbedtls_entropy_func, &entropy, nullptr, 0) == 0);
    REQUIRE(mbedtls_ssl_ticket_setup(&ctx, mbedtls_ctr_drbg_random, &rng, MBEDTLS_CIPHER_AES_256_GCM, 3600) == 0);
    ISOTLSTicketPolicy p;
    REQUIRE(p.setup(mbedtls_ctr_drbg_random, &rng) == 0);
    session.MBEDTLS_PRIVATE(tls_version) = MBEDTLS_SSL_VERSION_TLS1_3;
    session.MBEDTLS_PRIVATE(endpoint) = MBEDTLS_SSL_IS_SERVER;
    session.MBEDTLS_PRIVATE(ciphersuite) = 0x1302;
    session.MBEDTLS_PRIVATE(ticket_creation_time) = mbedtls_ms_time();
    session.MBEDTLS_PRIVATE(resumption_key_len) = 48;
    unsigned char ticket[4096], copy[4096];
    size_t len;
    uint32_t lifetime;
    const uint64_t origin = 1000;
    REQUIRE(!p.due(origin));
    REQUIRE(p.write(&ctx, &session, ticket, ticket + sizeof(ticket), &len, &lifetime, origin) == 0);
    REQUIRE(lifetime == 3600);
    REQUIRE(!p.due(origin + p.renewal_interval_ms - 1));
    REQUIRE(p.due(origin + p.renewal_interval_ms));
    memcpy(copy, ticket, len);
    copy[0] ^= 1;
    REQUIRE(p.parse(&ctx, &loaded, copy, len, origin) == MBEDTLS_ERR_SSL_INVALID_MAC);
    memcpy(copy, ticket, len);
    copy[15] ^= 1;
    REQUIRE(p.parse(&ctx, &loaded, copy, len, origin) == MBEDTLS_ERR_SSL_INVALID_MAC);
    REQUIRE(p.parse(&ctx, &loaded, ticket, 39, origin) == MBEDTLS_ERR_SSL_INVALID_MAC);
    p.reset();
    memcpy(copy, ticket, len);
    REQUIRE(p.parse(&ctx, &loaded, copy, len, origin + p.renewal_interval_ms) == 0);
    REQUIRE(p.write(&ctx, &session, ticket, ticket + sizeof(ticket), &len, &lifetime,
                    origin + p.renewal_interval_ms) == 0);
    REQUIRE(!p.due(origin + 2 * p.renewal_interval_ms - 1));
    REQUIRE(p.due(origin + 2 * p.renewal_interval_ms));
    // Renewal and resumption must not move the original full-handshake cutoff.
    p.reset();
    memcpy(copy, ticket, len);
    REQUIRE(p.parse(&ctx, &loaded, copy, len, origin + p.maximum_age_ms - 1) == 0);
    REQUIRE(p.expired(origin + p.maximum_age_ms));
    REQUIRE(!p.due(origin + p.maximum_age_ms));
    REQUIRE(p.write(&ctx, &session, copy, copy + sizeof(copy), &len, &lifetime,
                    origin + p.maximum_age_ms) == MBEDTLS_ERR_SSL_SESSION_TICKET_EXPIRED);
    // Recreate a valid envelope for cutoff, rollback and new-key checks.
    p.reset();
    REQUIRE(p.write(&ctx, &session, ticket, ticket + sizeof(ticket), &len, &lifetime, origin) == 0);
    REQUIRE(p.parse(&ctx, &loaded, ticket, len, origin + p.maximum_age_ms) == MBEDTLS_ERR_SSL_SESSION_TICKET_EXPIRED);
    REQUIRE(p.parse(&ctx, &loaded, ticket, len, origin - 1) == MBEDTLS_ERR_SSL_SESSION_TICKET_EXPIRED);
    REQUIRE(p.setup(mbedtls_ctr_drbg_random, &rng) == 0);
    REQUIRE(p.parse(&ctx, &loaded, ticket, len, origin) == MBEDTLS_ERR_SSL_INVALID_MAC);
    mbedtls_ssl_session_free(&loaded); mbedtls_ssl_session_free(&session);
    mbedtls_ssl_ticket_free(&ctx); mbedtls_ctr_drbg_free(&rng); mbedtls_entropy_free(&entropy);
    puts("policy timing, tamper, resumption origin, cutoff and key reset PASS");
    return 0;
}
