/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "isotls.h"
#include "dev_certs.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools/malloc.h"

#include <sys/socket.h>
#include <errno.h>
#include <string.h>

#include "mbedtls/error.h"
#include "mbedtls/ssl_ciphersuites.h"
#include "mbedtls/version.h"

// Check if TLS 1.3 is available in mbedTLS build
#if defined(MBEDTLS_SSL_PROTO_TLS1_3)
#define ISOTLS_TLS13_AVAILABLE 1
#else
#define ISOTLS_TLS13_AVAILABLE 0
#endif

#ifndef USE_EMBEDDED_TLS_CERTS
#include <LittleFS.h>
// Certificate paths for ISO 15118-2
#define ISO15118_2_CERT_CHAIN_PATH "/iso15118/iso2/secc_cert_chain.pem"
#define ISO15118_2_PRIVATE_KEY_PATH "/iso15118/iso2/secc_key.pem"
// Certificate paths for ISO 15118-20
#define ISO15118_20_CERT_CHAIN_PATH "/iso15118/iso20/secc_cert_chain.pem"
#define ISO15118_20_PRIVATE_KEY_PATH "/iso15118/iso20/secc_key.pem"
#endif

// =============================================================================
// ISO 15118-2 cipher suites (TLS 1.2)
// =============================================================================
// [V2G2-602] The SECC shall support all cipher suites defined in Table 7
// Table 7 - Supported cipher suites (ISO 15118-2 Section 7.7.3.4)
static const int iso15118_2_ciphersuites[] = {
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,  // IETF RFC 5289
    MBEDTLS_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256,   // IETF RFC 5289
    0
};

#if ISOTLS_TLS13_AVAILABLE
// =============================================================================
// ISO 15118-20 cipher suites (TLS 1.3)
// =============================================================================
// [V2G20-2458] The SECC shall support all cipher suites defined in Table 6
// Table 6 - TLS 1.3 cipher suites (ISO 15118-20 Section 7.7.3.4)
static const int iso15118_20_ciphersuites[] = {
    MBEDTLS_TLS1_3_AES_256_GCM_SHA384,        // Primary [V2G20-2458]
    MBEDTLS_TLS1_3_CHACHA20_POLY1305_SHA256,  // Alternative [V2G20-2458]
    0
};

// =============================================================================
// Combined cipher suites for AUTO mode (both TLS 1.2 and 1.3)
// =============================================================================
static const int iso15118_auto_ciphersuites[] = {
    // TLS 1.3 cipher suites (preferred)
    MBEDTLS_TLS1_3_AES_256_GCM_SHA384,
    MBEDTLS_TLS1_3_CHACHA20_POLY1305_SHA256,
    // TLS 1.2 cipher suites (fallback)
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256,
    0
};
#endif // ISOTLS_TLS13_AVAILABLE

// =============================================================================
// ISO 15118-2 curves
// =============================================================================
// [V2G2-006] ECC-based using secp256r1 curve (SECG notation) with ECDSA
// [V2G2-007] Key length for ECC shall be 256 bit
static const uint16_t iso15118_2_curves[] = {
    MBEDTLS_SSL_IANA_TLS_GROUP_SECP256R1,
    MBEDTLS_SSL_IANA_TLS_GROUP_NONE
};

#if ISOTLS_TLS13_AVAILABLE
// =============================================================================
// ISO 15118-20 named groups
// =============================================================================
// [V2G20-1634] The SECC shall support all named groups defined in Table 7
// [V2G20-2674] Primary: secp521r1 with ECDSA signature algorithm
// [V2G20-2319] Alternative: x448 (Curve448) with EdDSA
// [V2G20-2675] Key length for ECC shall be 521 bit
static const uint16_t iso15118_20_curves[] = {
    MBEDTLS_SSL_IANA_TLS_GROUP_SECP521R1,  // Primary [V2G20-2674]
    MBEDTLS_SSL_IANA_TLS_GROUP_X448,       // Alternative [V2G20-2319]
    MBEDTLS_SSL_IANA_TLS_GROUP_NONE
};

// =============================================================================
// Combined curves for AUTO mode (both ISO 15118-2 and -20)
// =============================================================================
static const uint16_t iso15118_auto_curves[] = {
    MBEDTLS_SSL_IANA_TLS_GROUP_SECP521R1,  // ISO 15118-20 primary
    MBEDTLS_SSL_IANA_TLS_GROUP_X448,       // ISO 15118-20 alternative
    MBEDTLS_SSL_IANA_TLS_GROUP_SECP256R1,  // ISO 15118-2
    MBEDTLS_SSL_IANA_TLS_GROUP_NONE
};
#endif // ISOTLS_TLS13_AVAILABLE

// mbedTLS send callback for non-blocking socket I/O
static int tls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
    int fd = *static_cast<int*>(ctx);
    ssize_t ret = send(fd, buf, len, 0);

    if (ret < 0) {
        int saved_errno = errno;
        if (saved_errno == EWOULDBLOCK || saved_errno == EAGAIN) {
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        }
        logger.printfln("ISOTLS: send() failed on fd %d: errno %d (%s), len=%zu",
                        fd, saved_errno, strerror(saved_errno), len);
        return MBEDTLS_ERR_NET_SEND_FAILED;
    }
    return static_cast<int>(ret);
}

// mbedTLS receive callback for non-blocking socket I/O
static int tls_net_recv(void *ctx, unsigned char *buf, size_t len)
{
    int fd = *static_cast<int*>(ctx);
    ssize_t ret = recv(fd, buf, len, 0);

    if (ret < 0) {
        int saved_errno = errno;
        if (saved_errno == EWOULDBLOCK || saved_errno == EAGAIN) {
            return MBEDTLS_ERR_SSL_WANT_READ;
        }
        logger.printfln("ISOTLS: recv() failed on fd %d: errno %d (%s), len=%zu",
                        fd, saved_errno, strerror(saved_errno), len);
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }
    if (ret == 0) {
        logger.printfln("ISOTLS: recv() returned 0 on fd %d (peer closed)", fd);
        return MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY;
    }
    return static_cast<int>(ret);
}

bool ISOTLS::load_certificates()
{
    bool load_iso2 = (tls_mode == IsoTlsMode::ISO15118_2 || tls_mode == IsoTlsMode::AUTO);
#if ISOTLS_TLS13_AVAILABLE
    bool load_iso20 = (tls_mode == IsoTlsMode::ISO15118_20 || tls_mode == IsoTlsMode::AUTO);
#else
    bool load_iso20 = false;  // TLS 1.3 not available, skip ISO20 certs
#endif

#ifdef USE_EMBEDDED_TLS_CERTS
    logger.printfln("ISOTLS: Using embedded dev certificates");

    // Load ISO 15118-2 certificates (secp256r1)
    if (load_iso2) {
        cert_chain_pem_len_iso2 = strlen(dev_cert_chain_pem_iso2) + 1;
        cert_chain_pem_iso2 = static_cast<uint8_t*>(calloc_psram_or_dram(cert_chain_pem_len_iso2, 1));
        if (cert_chain_pem_iso2 == nullptr) {
            logger.printfln("ISOTLS: Failed to allocate memory for ISO2 certificate chain");
            return false;
        }
        memcpy(cert_chain_pem_iso2, dev_cert_chain_pem_iso2, cert_chain_pem_len_iso2);

        private_key_pem_len_iso2 = strlen(dev_private_key_pem_iso2) + 1;
        private_key_pem_iso2 = static_cast<uint8_t*>(calloc_psram_or_dram(private_key_pem_len_iso2, 1));
        if (private_key_pem_iso2 == nullptr) {
            logger.printfln("ISOTLS: Failed to allocate memory for ISO2 private key");
            return false;
        }
        memcpy(private_key_pem_iso2, dev_private_key_pem_iso2, private_key_pem_len_iso2);
        logger.printfln("ISOTLS: Loaded ISO 15118-2 certs (secp256r1): chain=%zu bytes, key=%zu bytes",
                        cert_chain_pem_len_iso2 - 1, private_key_pem_len_iso2 - 1);
    }

    // Load ISO 15118-20 certificates (secp521r1)
    if (load_iso20) {
        cert_chain_pem_len_iso20 = strlen(dev_cert_chain_pem_iso20) + 1;
        cert_chain_pem_iso20 = static_cast<uint8_t*>(calloc_psram_or_dram(cert_chain_pem_len_iso20, 1));
        if (cert_chain_pem_iso20 == nullptr) {
            logger.printfln("ISOTLS: Failed to allocate memory for ISO20 certificate chain");
            return false;
        }
        memcpy(cert_chain_pem_iso20, dev_cert_chain_pem_iso20, cert_chain_pem_len_iso20);

        private_key_pem_len_iso20 = strlen(dev_private_key_pem_iso20) + 1;
        private_key_pem_iso20 = static_cast<uint8_t*>(calloc_psram_or_dram(private_key_pem_len_iso20, 1));
        if (private_key_pem_iso20 == nullptr) {
            logger.printfln("ISOTLS: Failed to allocate memory for ISO20 private key");
            return false;
        }
        memcpy(private_key_pem_iso20, dev_private_key_pem_iso20, private_key_pem_len_iso20);
        logger.printfln("ISOTLS: Loaded ISO 15118-20 certs (secp521r1): chain=%zu bytes, key=%zu bytes",
                        cert_chain_pem_len_iso20 - 1, private_key_pem_len_iso20 - 1);
    }

    return true;

#else // !USE_EMBEDDED_TLS_CERTS
    // Load certificate chain from LittleFS
    if (load_iso2) {
        if (!LittleFS.exists(ISO15118_2_CERT_CHAIN_PATH)) {
            logger.printfln("ISOTLS: ISO2 certificate chain file not found: %s", ISO15118_2_CERT_CHAIN_PATH);
            return false;
        }

        File cert_file = LittleFS.open(ISO15118_2_CERT_CHAIN_PATH, "r");
        if (!cert_file) {
            logger.printfln("ISOTLS: Failed to open ISO2 certificate chain file");
            return false;
        }

        cert_chain_pem_len_iso2 = cert_file.size() + 1;
        cert_chain_pem_iso2 = static_cast<uint8_t*>(calloc_psram_or_dram(cert_chain_pem_len_iso2, 1));
        if (cert_chain_pem_iso2 == nullptr) {
            logger.printfln("ISOTLS: Failed to allocate memory for ISO2 certificate chain");
            cert_file.close();
            return false;
        }

        size_t bytes_read = cert_file.read(cert_chain_pem_iso2, cert_chain_pem_len_iso2 - 1);
        cert_chain_pem_iso2[bytes_read] = 0;
        cert_file.close();

        if (!LittleFS.exists(ISO15118_2_PRIVATE_KEY_PATH)) {
            logger.printfln("ISOTLS: ISO2 private key file not found: %s", ISO15118_2_PRIVATE_KEY_PATH);
            return false;
        }

        File key_file = LittleFS.open(ISO15118_2_PRIVATE_KEY_PATH, "r");
        if (!key_file) {
            logger.printfln("ISOTLS: Failed to open ISO2 private key file");
            return false;
        }

        private_key_pem_len_iso2 = key_file.size() + 1;
        private_key_pem_iso2 = static_cast<uint8_t*>(calloc_psram_or_dram(private_key_pem_len_iso2, 1));
        if (private_key_pem_iso2 == nullptr) {
            logger.printfln("ISOTLS: Failed to allocate memory for ISO2 private key");
            key_file.close();
            return false;
        }

        bytes_read = key_file.read(private_key_pem_iso2, private_key_pem_len_iso2 - 1);
        private_key_pem_iso2[bytes_read] = 0;
        key_file.close();

        logger.printfln("ISOTLS: Loaded ISO 15118-2 certs from LittleFS");
    }

    if (load_iso20) {
        if (!LittleFS.exists(ISO15118_20_CERT_CHAIN_PATH)) {
            logger.printfln("ISOTLS: ISO20 certificate chain file not found: %s", ISO15118_20_CERT_CHAIN_PATH);
            return false;
        }

        File cert_file = LittleFS.open(ISO15118_20_CERT_CHAIN_PATH, "r");
        if (!cert_file) {
            logger.printfln("ISOTLS: Failed to open ISO20 certificate chain file");
            return false;
        }

        cert_chain_pem_len_iso20 = cert_file.size() + 1;
        cert_chain_pem_iso20 = static_cast<uint8_t*>(calloc_psram_or_dram(cert_chain_pem_len_iso20, 1));
        if (cert_chain_pem_iso20 == nullptr) {
            logger.printfln("ISOTLS: Failed to allocate memory for ISO20 certificate chain");
            cert_file.close();
            return false;
        }

        size_t bytes_read = cert_file.read(cert_chain_pem_iso20, cert_chain_pem_len_iso20 - 1);
        cert_chain_pem_iso20[bytes_read] = 0;
        cert_file.close();

        if (!LittleFS.exists(ISO15118_20_PRIVATE_KEY_PATH)) {
            logger.printfln("ISOTLS: ISO20 private key file not found: %s", ISO15118_20_PRIVATE_KEY_PATH);
            return false;
        }

        File key_file = LittleFS.open(ISO15118_20_PRIVATE_KEY_PATH, "r");
        if (!key_file) {
            logger.printfln("ISOTLS: Failed to open ISO20 private key file");
            return false;
        }

        private_key_pem_len_iso20 = key_file.size() + 1;
        private_key_pem_iso20 = static_cast<uint8_t*>(calloc_psram_or_dram(private_key_pem_len_iso20, 1));
        if (private_key_pem_iso20 == nullptr) {
            logger.printfln("ISOTLS: Failed to allocate memory for ISO20 private key");
            key_file.close();
            return false;
        }

        bytes_read = key_file.read(private_key_pem_iso20, private_key_pem_len_iso20 - 1);
        private_key_pem_iso20[bytes_read] = 0;
        key_file.close();

        logger.printfln("ISOTLS: Loaded ISO 15118-20 certs from LittleFS");
    }

    return true;
#endif // USE_EMBEDDED_TLS_CERTS
}

bool ISOTLS::setup(IsoTlsMode mode)
{
    if (initialized) {
        return true;
    }

#if !ISOTLS_TLS13_AVAILABLE
    // TLS 1.3 is not available in this mbedTLS build
    // Force ISO 15118-2 mode (TLS 1.2 only)
    if (mode == IsoTlsMode::ISO15118_20) {
        logger.printfln("ISOTLS: ERROR - TLS 1.3 not available, cannot use ISO15118_20 mode");
        logger.printfln("ISOTLS: mbedTLS was compiled without CONFIG_MBEDTLS_SSL_PROTO_TLS1_3");
        return false;
    }
    if (mode == IsoTlsMode::AUTO) {
        logger.printfln("ISOTLS: WARNING - TLS 1.3 not available, falling back to ISO15118_2 mode");
        mode = IsoTlsMode::ISO15118_2;
    }
#endif

    tls_mode = mode;

    const char *mode_str = (mode == IsoTlsMode::ISO15118_2) ? "ISO 15118-2 (TLS 1.2)" :
                           (mode == IsoTlsMode::ISO15118_20) ? "ISO 15118-20 (TLS 1.3)" :
                           "AUTO (TLS 1.2/1.3)";
    logger.printfln("ISOTLS: Setting up TLS for ISO 15118 - Mode: %s", mode_str);

    // Load certificates first
    if (!load_certificates()) {
        logger.printfln("ISOTLS: Failed to load certificates");
        return false;
    }

    // Allocate mbedTLS contexts from PSRAM
    ssl = static_cast<mbedtls_ssl_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_ssl_context)));
    ssl_conf = static_cast<mbedtls_ssl_config*>(calloc_psram_or_dram(1, sizeof(mbedtls_ssl_config)));
    entropy = static_cast<mbedtls_entropy_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_entropy_context)));
    ctr_drbg = static_cast<mbedtls_ctr_drbg_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_ctr_drbg_context)));

    bool need_iso2 = (mode == IsoTlsMode::ISO15118_2 || mode == IsoTlsMode::AUTO);
#if ISOTLS_TLS13_AVAILABLE
    bool need_iso20 = (mode == IsoTlsMode::ISO15118_20 || mode == IsoTlsMode::AUTO);
#else
    bool need_iso20 = false;
#endif

    if (need_iso2) {
        cert_chain_iso2 = static_cast<mbedtls_x509_crt*>(calloc_psram_or_dram(1, sizeof(mbedtls_x509_crt)));
        private_key_iso2 = static_cast<mbedtls_pk_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_pk_context)));
    }

    if (need_iso20) {
        cert_chain_iso20 = static_cast<mbedtls_x509_crt*>(calloc_psram_or_dram(1, sizeof(mbedtls_x509_crt)));
        private_key_iso20 = static_cast<mbedtls_pk_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_pk_context)));
    }

    if (ssl == nullptr || ssl_conf == nullptr || entropy == nullptr || ctr_drbg == nullptr) {
        logger.printfln("ISOTLS: Failed to allocate mbedTLS contexts");
        cleanup();
        return false;
    }

    if (need_iso2 && (cert_chain_iso2 == nullptr || private_key_iso2 == nullptr)) {
        logger.printfln("ISOTLS: Failed to allocate ISO2 cert contexts");
        cleanup();
        return false;
    }

    if (need_iso20 && (cert_chain_iso20 == nullptr || private_key_iso20 == nullptr)) {
        logger.printfln("ISOTLS: Failed to allocate ISO20 cert contexts");
        cleanup();
        return false;
    }

    // Initialize mbedTLS contexts
    mbedtls_ssl_init(ssl);
    mbedtls_ssl_config_init(ssl_conf);
    mbedtls_entropy_init(entropy);
    mbedtls_ctr_drbg_init(ctr_drbg);

    if (need_iso2) {
        mbedtls_x509_crt_init(cert_chain_iso2);
        mbedtls_pk_init(private_key_iso2);
    }

    if (need_iso20) {
        mbedtls_x509_crt_init(cert_chain_iso20);
        mbedtls_pk_init(private_key_iso20);
    }

    int ret;

    // Seed the random number generator
    const char *pers = "iso15118_secc";
    ret = mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy,
                                (const unsigned char *)pers, strlen(pers));
    if (ret != 0) {
        logger.printfln("ISOTLS: mbedtls_ctr_drbg_seed failed: -0x%04x", static_cast<unsigned>(-ret));
        cleanup();
        return false;
    }

    // Parse ISO 15118-2 certificates
    if (need_iso2) {
        ret = mbedtls_x509_crt_parse(cert_chain_iso2, cert_chain_pem_iso2, cert_chain_pem_len_iso2);
        if (ret != 0) {
            logger.printfln("ISOTLS: ISO2 mbedtls_x509_crt_parse failed: -0x%04x", static_cast<unsigned>(-ret));
            cleanup();
            return false;
        }

        ret = mbedtls_pk_parse_key(private_key_iso2, private_key_pem_iso2, private_key_pem_len_iso2,
                                   nullptr, 0, mbedtls_ctr_drbg_random, ctr_drbg);
        if (ret != 0) {
            logger.printfln("ISOTLS: ISO2 mbedtls_pk_parse_key failed: -0x%04x", static_cast<unsigned>(-ret));
            cleanup();
            return false;
        }
        logger.printfln("ISOTLS: ISO 15118-2 certificates parsed successfully (secp256r1)");
    }

    // Parse ISO 15118-20 certificates
    if (need_iso20) {
        ret = mbedtls_x509_crt_parse(cert_chain_iso20, cert_chain_pem_iso20, cert_chain_pem_len_iso20);
        if (ret != 0) {
            logger.printfln("ISOTLS: ISO20 mbedtls_x509_crt_parse failed: -0x%04x", static_cast<unsigned>(-ret));
            cleanup();
            return false;
        }

        ret = mbedtls_pk_parse_key(private_key_iso20, private_key_pem_iso20, private_key_pem_len_iso20,
                                   nullptr, 0, mbedtls_ctr_drbg_random, ctr_drbg);
        if (ret != 0) {
            logger.printfln("ISOTLS: ISO20 mbedtls_pk_parse_key failed: -0x%04x", static_cast<unsigned>(-ret));
            cleanup();
            return false;
        }
        logger.printfln("ISOTLS: ISO 15118-20 certificates parsed successfully (secp521r1)");
    }

    // Configure SSL
    ret = mbedtls_ssl_config_defaults(ssl_conf,
                                      MBEDTLS_SSL_IS_SERVER,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) {
        logger.printfln("ISOTLS: mbedtls_ssl_config_defaults failed: -0x%04x", static_cast<unsigned>(-ret));
        cleanup();
        return false;
    }

    // =========================================================================
    // TLS Version Configuration based on mode
    // =========================================================================
    switch (mode) {
        case IsoTlsMode::ISO15118_2:
            // [V2G2-067] TLS version 1.2 only
            mbedtls_ssl_conf_min_tls_version(ssl_conf, MBEDTLS_SSL_VERSION_TLS1_2);
            mbedtls_ssl_conf_max_tls_version(ssl_conf, MBEDTLS_SSL_VERSION_TLS1_2);
            mbedtls_ssl_conf_ciphersuites(ssl_conf, iso15118_2_ciphersuites);
            mbedtls_ssl_conf_groups(ssl_conf, iso15118_2_curves);
            break;

#if ISOTLS_TLS13_AVAILABLE
        case IsoTlsMode::ISO15118_20:
            // [V2G20-1264] TLS version 1.3 only
            mbedtls_ssl_conf_min_tls_version(ssl_conf, MBEDTLS_SSL_VERSION_TLS1_3);
            mbedtls_ssl_conf_max_tls_version(ssl_conf, MBEDTLS_SSL_VERSION_TLS1_3);
            mbedtls_ssl_conf_ciphersuites(ssl_conf, iso15118_20_ciphersuites);
            mbedtls_ssl_conf_groups(ssl_conf, iso15118_20_curves);
            break;

        case IsoTlsMode::AUTO:
        default:
            // Allow both TLS 1.2 and 1.3, let client negotiate
            // [V2G20-2356] If TLS 1.2 negotiated, SECC shall not select ISO 15118-20
            mbedtls_ssl_conf_min_tls_version(ssl_conf, MBEDTLS_SSL_VERSION_TLS1_2);
            mbedtls_ssl_conf_max_tls_version(ssl_conf, MBEDTLS_SSL_VERSION_TLS1_3);
            mbedtls_ssl_conf_ciphersuites(ssl_conf, iso15118_auto_ciphersuites);
            mbedtls_ssl_conf_groups(ssl_conf, iso15118_auto_curves);
            break;
#else
        // When TLS 1.3 is not available, ISO15118_20 and AUTO are handled
        // at the top of this function (AUTO falls back to ISO15118_2)
        case IsoTlsMode::ISO15118_20:
        case IsoTlsMode::AUTO:
        default:
            // Should not reach here due to earlier checks, but handle gracefully
            mbedtls_ssl_conf_min_tls_version(ssl_conf, MBEDTLS_SSL_VERSION_TLS1_2);
            mbedtls_ssl_conf_max_tls_version(ssl_conf, MBEDTLS_SSL_VERSION_TLS1_2);
            mbedtls_ssl_conf_ciphersuites(ssl_conf, iso15118_2_ciphersuites);
            mbedtls_ssl_conf_groups(ssl_conf, iso15118_2_curves);
            break;
#endif
    }

    // =========================================================================
    // Authentication Mode
    // =========================================================================
    // ISO 15118-2: Unilateral authentication (EVCC authenticates SECC, Section 7.7.3.1)
    // ISO 15118-20: Should use mutual authentication [V2G20-2400]
    // TODO: Implement mutual authentication for ISO 15118-20 when needed
    mbedtls_ssl_conf_authmode(ssl_conf, MBEDTLS_SSL_VERIFY_NONE);

    // Set random number generator
    mbedtls_ssl_conf_rng(ssl_conf, mbedtls_ctr_drbg_random, ctr_drbg);

    // =========================================================================
    // Set own certificate(s) and private key(s)
    // =========================================================================
    // For AUTO mode, we add both certificate chains so mbedTLS can select
    // the appropriate one based on the negotiated TLS version and cipher suite.
    // mbedTLS will automatically select the certificate with matching key type.

    if (need_iso2) {
        ret = mbedtls_ssl_conf_own_cert(ssl_conf, cert_chain_iso2, private_key_iso2);
        if (ret != 0) {
            logger.printfln("ISOTLS: mbedtls_ssl_conf_own_cert (ISO2) failed: -0x%04x", static_cast<unsigned>(-ret));
            cleanup();
            return false;
        }
    }

    if (need_iso20) {
        ret = mbedtls_ssl_conf_own_cert(ssl_conf, cert_chain_iso20, private_key_iso20);
        if (ret != 0) {
            logger.printfln("ISOTLS: mbedtls_ssl_conf_own_cert (ISO20) failed: -0x%04x", static_cast<unsigned>(-ret));
            cleanup();
            return false;
        }
    }

    // Setup SSL context with configuration
    ret = mbedtls_ssl_setup(ssl, ssl_conf);
    if (ret != 0) {
        logger.printfln("ISOTLS: mbedtls_ssl_setup failed: -0x%04x", static_cast<unsigned>(-ret));
        cleanup();
        return false;
    }

    initialized = true;
    logger.printfln("ISOTLS: TLS setup complete");
    return true;
}

void ISOTLS::cleanup()
{
    end_session();

    if (ssl != nullptr) {
        mbedtls_ssl_free(ssl);
        free_any(ssl);
        ssl = nullptr;
    }

    if (ssl_conf != nullptr) {
        mbedtls_ssl_config_free(ssl_conf);
        free_any(ssl_conf);
        ssl_conf = nullptr;
    }

    if (cert_chain_iso2 != nullptr) {
        mbedtls_x509_crt_free(cert_chain_iso2);
        free_any(cert_chain_iso2);
        cert_chain_iso2 = nullptr;
    }

    if (private_key_iso2 != nullptr) {
        mbedtls_pk_free(private_key_iso2);
        free_any(private_key_iso2);
        private_key_iso2 = nullptr;
    }

    if (cert_chain_iso20 != nullptr) {
        mbedtls_x509_crt_free(cert_chain_iso20);
        free_any(cert_chain_iso20);
        cert_chain_iso20 = nullptr;
    }

    if (private_key_iso20 != nullptr) {
        mbedtls_pk_free(private_key_iso20);
        free_any(private_key_iso20);
        private_key_iso20 = nullptr;
    }

    if (entropy != nullptr) {
        mbedtls_entropy_free(entropy);
        free_any(entropy);
        entropy = nullptr;
    }

    if (ctr_drbg != nullptr) {
        mbedtls_ctr_drbg_free(ctr_drbg);
        free_any(ctr_drbg);
        ctr_drbg = nullptr;
    }

    if (cert_chain_pem_iso2 != nullptr) {
        free_any(cert_chain_pem_iso2);
        cert_chain_pem_iso2 = nullptr;
        cert_chain_pem_len_iso2 = 0;
    }

    if (private_key_pem_iso2 != nullptr) {
        free_any(private_key_pem_iso2);
        private_key_pem_iso2 = nullptr;
        private_key_pem_len_iso2 = 0;
    }

    if (cert_chain_pem_iso20 != nullptr) {
        free_any(cert_chain_pem_iso20);
        cert_chain_pem_iso20 = nullptr;
        cert_chain_pem_len_iso20 = 0;
    }

    if (private_key_pem_iso20 != nullptr) {
        free_any(private_key_pem_iso20);
        private_key_pem_iso20 = nullptr;
        private_key_pem_len_iso20 = 0;
    }

    initialized = false;
}

bool ISOTLS::start_session(int fd)
{
    if (!initialized || ssl == nullptr) {
        logger.printfln("ISOTLS: Cannot start session - not initialized");
        return false;
    }

    // Reset SSL state for new connection
    mbedtls_ssl_session_reset(ssl);

    // Store socket fd and set up I/O callbacks
    socket_fd = fd;
    mbedtls_ssl_set_bio(ssl, &socket_fd, tls_net_send, tls_net_recv, nullptr);

    handshake_state = TlsHandshakeState::IN_PROGRESS;
    session_active = false;

    logger.printfln("ISOTLS: Starting TLS session on socket %d", fd);
    return true;
}

void ISOTLS::end_session()
{
    if (session_active && ssl != nullptr) {
        // Send close_notify alert (best effort, ignore errors)
        mbedtls_ssl_close_notify(ssl);
    }

    session_active = false;
    handshake_state = TlsHandshakeState::NOT_STARTED;
    socket_fd = -1;
}

bool ISOTLS::do_handshake()
{
    if (!initialized || ssl == nullptr) {
        logger.printfln("ISOTLS: TLS not initialized");
        return false;
    }

    int ret = mbedtls_ssl_handshake(ssl);

    if (ret == 0) {
        // Handshake completed successfully
        handshake_state = TlsHandshakeState::COMPLETED;
        session_active = true;

        const char *tls_version = get_tls_version_string();
        const char *cipher = get_cipher_suite();
        bool is_tls13 = is_tls13_active();

        logger.printfln("ISOTLS: Handshake completed successfully");
        logger.printfln("ISOTLS: TLS version: %s", tls_version ? tls_version : "unknown");
        logger.printfln("ISOTLS: Cipher suite: %s", cipher ? cipher : "unknown");

        if (tls_mode == IsoTlsMode::AUTO) {
            // [V2G20-2356] If TLS 1.2 or lower, SECC shall not select ISO 15118-20
            if (is_tls13) {
                logger.printfln("ISOTLS: TLS 1.3 negotiated - ISO 15118-20 allowed");
            } else {
                logger.printfln("ISOTLS: TLS 1.2 negotiated - ISO 15118-20 NOT allowed per [V2G20-2356]");
            }
        }

        return true;
    }

    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
        // Handshake in progress, need to call again
        return false;
    }

    // Handshake failed
    char error_buf[128];
    mbedtls_strerror(ret, error_buf, sizeof(error_buf));
    logger.printfln("ISOTLS: Handshake failed: -0x%04x (%s)", static_cast<unsigned>(-ret), error_buf);
    handshake_state = TlsHandshakeState::FAILED;
    return false;
}

ssize_t ISOTLS::read(uint8_t *data, size_t len)
{
    if (!session_active || (ssl == nullptr)) {
        return -1;
    }

    int ret = mbedtls_ssl_read(ssl, data, len);

    if (ret >= 0) {
        return ret;
    }

    if ((ret == MBEDTLS_ERR_SSL_WANT_READ) || (ret == MBEDTLS_ERR_SSL_WANT_WRITE)) {
        errno = EWOULDBLOCK;
        return -1;
    }

    if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
        return 0; // Connection closed
    }

    char error_buf[128];
    mbedtls_strerror(ret, error_buf, sizeof(error_buf));
    logger.printfln("ISOTLS: Read error: -0x%04x (%s)", static_cast<unsigned>(-ret), error_buf);
    return -1;
}

ssize_t ISOTLS::write(const uint8_t *data, size_t len)
{
    if (!session_active || ssl == nullptr) {
        return -1;
    }

    int ret = mbedtls_ssl_write(ssl, data, len);

    if (ret >= 0) {
        return ret;
    }

    if ((ret == MBEDTLS_ERR_SSL_WANT_READ) || (ret == MBEDTLS_ERR_SSL_WANT_WRITE)) {
        errno = EWOULDBLOCK;
        return -1;
    }

    char error_buf[128];
    mbedtls_strerror(ret, error_buf, sizeof(error_buf));
    logger.printfln("ISOTLS: Write error: -0x%04x (%s)", static_cast<unsigned>(-ret), error_buf);
    return -1;
}

const char *ISOTLS::get_cipher_suite() const
{
    if (!session_active || (ssl == nullptr)) {
        return nullptr;
    }
    return mbedtls_ssl_get_ciphersuite(ssl);
}

const char *ISOTLS::get_tls_version_string() const
{
    if (!session_active || (ssl == nullptr)) {
        return nullptr;
    }
    return mbedtls_ssl_get_version(ssl);
}

bool ISOTLS::is_tls13_active() const
{
    if (!session_active || (ssl == nullptr)) {
        return false;
    }

    const char *version = mbedtls_ssl_get_version(ssl);
    if (version == nullptr) {
        return false;
    }

    return (strcmp(version, "TLSv1.3") == 0);
}
