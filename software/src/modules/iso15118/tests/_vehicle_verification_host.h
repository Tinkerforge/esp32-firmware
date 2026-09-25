// Minimal platform adapter for compiling the actual ISOTLS verification source
// on host. The RTOS worker runs synchronously; this tests policy/signatures and
// callbacks, not scheduling, ESP32 memory allocation or handshake latency.
#pragma once
#include <cstdint>
#include <cstring>
#include "mbedtls/x509_crt.h"
#include "mbedtls/ssl.h"
#include "mbedtls/md.h"
#include "vehicle_certificate.h"

struct Trace {
    template<typename... Args> void trace(const char *, Args...) {}
};
inline Trace iso15118;
struct OcppPolicy {
    bool required = true;
    bool is_iso20_ocsp_required() const { return required; }
};
inline OcppPolicy ocpp;
using BaseType_t = int;
constexpr int pdPASS_nowarn = 1;
inline int xTaskCreatePinnedToCore(void (*fn)(void *), const char *, int, void *ctx, int, void *, int)
{
    fn(ctx);
    return pdPASS_nowarn;
}
inline void xSemaphoreGive_nowarn(void *) {}
inline void vTaskDelete(void *) {}
constexpr int portMAX_DELAY_nowarn = -1;
inline void xQueueSemaphoreTake(void *, int) {}

// Production's exported Mbed TLS helper is provided by patch 0003. Stock host
// Mbed TLS verifies the same DER signature here and also in its normal verifier.
inline int x509_crt_check_signature(const mbedtls_x509_crt *child, mbedtls_x509_crt *parent, void *)
{
    unsigned char hash[MBEDTLS_MD_MAX_SIZE];
    const auto *md = mbedtls_md_info_from_type(child->MBEDTLS_PRIVATE(sig_md));
    if (md == nullptr) return -1;
    int ret = mbedtls_md(md, child->tbs.p, child->tbs.len, hash);
    if (ret != 0) return ret;
    return mbedtls_pk_verify_ext(child->MBEDTLS_PRIVATE(sig_pk), child->MBEDTLS_PRIVATE(sig_opts),
                                &parent->pk, child->MBEDTLS_PRIVATE(sig_md), hash, mbedtls_md_get_size(md),
                                child->MBEDTLS_PRIVATE(sig).p, child->MBEDTLS_PRIVATE(sig).len);
}

class ISOTLS {
public:
    static constexpr size_t CERTS_MAX_VERIFY = 8;
    struct verification_context_t {
        mbedtls_x509_crt *certs[CERTS_MAX_VERIFY]{};
        void *sem_handle = nullptr;
        bool async_started = false;
        bool intermediates_valid = false;
        mbedtls_x509_crt *anchor_root = nullptr;
    } context;
    verification_context_t *verification_context = &context;
    mbedtls_x509_crt *trusted_ca_iso20 = nullptr;
    void reset() { context = {}; }
    void hand_off_vehicle_chain();
    void verify_intermediate_certs();
    static void verify_certs_task(void *ctx);
    static int cert_verify(void *ctx, mbedtls_x509_crt *cert, int index, uint32_t *flags);
};
