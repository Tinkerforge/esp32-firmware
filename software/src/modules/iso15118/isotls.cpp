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
#include "tools/freertos.h"
#include "tools/malloc.h"

#include <sys/socket.h>
#include <errno.h>
#include <string.h>

#include "mbedtls/error.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ssl_ciphersuites.h"
#include "mbedtls/version.h"
#include "mbedtls/x509_crt.h"

#ifndef USE_EMBEDDED_TLS_CERTS
#include <LittleFS.h>
// Certificate paths for ISO 15118-2
#define ISO15118_2_CERT_CHAIN_PATH "/iso15118/iso2/secc_cert_chain.pem"
#define ISO15118_2_PRIVATE_KEY_PATH "/iso15118/iso2/secc_key.pem"
// Certificate paths for ISO 15118-20
#define ISO15118_20_CERT_CHAIN_PATH "/iso15118/iso20/secc_cert_chain.pem"
#define ISO15118_20_PRIVATE_KEY_PATH "/iso15118/iso20/secc_key.pem"
// Trusted root CA paths for mutual TLS (ISO 15118-20)
#define ISO15118_20_OEM_ROOT_CA_PATH "/iso15118/iso20/oem_root_ca.pem"
#define ISO15118_20_V2G_ROOT_CA_PATH "/iso15118/iso20/v2g_root_ca.pem"
#endif

#include "gcc_warnings.h"

// =============================================================================
// ISO 15118 cipher suites (TLS 1.2 + TLS 1.3)
// =============================================================================
// TLS 1.3: [V2G20-2458] Table 6
// TLS 1.2: [V2G2-602] Table 7
static const int iso15118_ciphersuites[] = {
    // TLS 1.3 cipher suites (preferred)
    MBEDTLS_TLS1_3_CHACHA20_POLY1305_SHA256,          // [V2G20-2458]
    MBEDTLS_TLS1_3_AES_256_GCM_SHA384,                // [V2G20-2458]
    // TLS 1.2 cipher suites (fallback)
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,  // [V2G2-602] IETF RFC 5289
    MBEDTLS_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256,   // [V2G2-602] IETF RFC 5289
    0
};

// =============================================================================
// ISO 15118 named groups (ISO 15118-2 + ISO 15118-20)
// =============================================================================
// [V2G20-2674] secp521r1 (ISO 15118-20 primary signature curve)
// [V2G20-2319] x448 (ISO 15118-20 alternative)
// [V2G2-006]   secp256r1 (ISO 15118-2)
// X25519 is listed first for ECDHE key exchange by OpenSSL.
// Offer X25519 key_share by default in their initial
// ClientHello. Without it, the server must send a HelloRetryRequest (HRR)
// adding an extra network round-trip. X25519 is only used for
// ephemeral key exchange. Certificate signatures remain secp521r1.
static const uint16_t iso15118_curves[] = {
    MBEDTLS_SSL_IANA_TLS_GROUP_X25519,     // Fast ECDHE, avoids HRR with most clients
    MBEDTLS_SSL_IANA_TLS_GROUP_X448,       // ISO 15118-20 alternative, faster than secp256r1
    MBEDTLS_SSL_IANA_TLS_GROUP_SECP256R1,  // ISO 15118-2
    MBEDTLS_SSL_IANA_TLS_GROUP_SECP521R1,  // ISO 15118-20 primary, extremely slow
    MBEDTLS_SSL_IANA_TLS_GROUP_NONE
};

// =============================================================================
// Certificate verification profile
// =============================================================================
// This is a copy of the default profile, except that certificate signature
// verification will be skipped so that it can be done manually in parallel.
static const mbedtls_x509_crt_profile mbedtls_x509_crt_profile_custom =
{
    /* Hashes from SHA-256 and above. Note that this selection
     * should be aligned with ssl_preset_default_hashes in ssl_tls.c. */
    MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA384) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA512),

    /* Any PK alg */
    0xFFFFFFF,

    /* Curves at or above 128-bit security level. Note that this selection
     * should be aligned with ssl_preset_default_curves in ssl_tls.c. */
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_SECP256R1) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_SECP384R1) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_SECP521R1) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_BP256R1) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_BP384R1) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_BP512R1) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_CURVE25519) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_CURVE448) |
    0,

    2048, // rsa_min_bitlen doesn't matter because ISO15118 doesn't use RSA
    1,    // skip_signature_verification
};

// =============================================================================
// Certificate selection callback (TLS version-based)
// =============================================================================
// Both ISO 15118-2 (secp256r1, TLS 1.2) and ISO 15118-20 (secp521r1, TLS 1.3)
// certificates are loaded. This callback is invoked after ClientHello processing,
// when the negotiated TLS version is known. It selects the appropriate certificate:
//   - TLS 1.3 -> ISO 15118-20 cert (secp521r1)
//   - TLS 1.2 -> ISO 15118-2  cert (secp256r1)
// =============================================================================

// File-static pointer to the single ISOTLS instance, used by the callback.
// Only safe as long as there is exactly one ISOTLS instance in the firmware.
static ISOTLS *s_isotls_instance = nullptr;

static int tls_cert_selection_callback(mbedtls_ssl_context *ssl)
{
    if (s_isotls_instance == nullptr) {
        return -1;
    }
    return s_isotls_instance->select_certificate_for_handshake(ssl);
}

// EWOULDBLOCK and EAGAIN can be the same value depending on compiler version
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlogical-op"

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

#pragma GCC diagnostic pop

bool ISOTLS::load_certificates()
{
#ifdef USE_EMBEDDED_TLS_CERTS
    logger.printfln("ISOTLS: Using embedded dev certificates");

    // Load ISO 15118-2 certificates (secp256r1)
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

    // Load ISO 15118-20 certificates (secp521r1)
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

    // Load trusted root CA certificates for mutual TLS (ISO 15118-20)
    // [V2G20-2400] SECC shall request EVCC certificate via CertificateRequest
    // [V2G20-2338] SECC shall have at least one V2G or OEM root CA certificate
    oem_root_ca_pem_len_iso20 = strlen(dev_oem_root_ca_pem_iso20) + 1;
    oem_root_ca_pem_iso20 = static_cast<uint8_t*>(calloc_psram_or_dram(oem_root_ca_pem_len_iso20, 1));
    if (oem_root_ca_pem_iso20 == nullptr) {
        logger.printfln("ISOTLS: Failed to allocate memory for ISO20 OEM Root CA");
        return false;
    }
    memcpy(oem_root_ca_pem_iso20, dev_oem_root_ca_pem_iso20, oem_root_ca_pem_len_iso20);

    v2g_root_ca_pem_len_iso20 = strlen(dev_v2g_root_ca_pem_iso20) + 1;
    v2g_root_ca_pem_iso20 = static_cast<uint8_t*>(calloc_psram_or_dram(v2g_root_ca_pem_len_iso20, 1));
    if (v2g_root_ca_pem_iso20 == nullptr) {
        logger.printfln("ISOTLS: Failed to allocate memory for ISO20 V2G Root CA");
        return false;
    }
    memcpy(v2g_root_ca_pem_iso20, dev_v2g_root_ca_pem_iso20, v2g_root_ca_pem_len_iso20);

    logger.printfln("ISOTLS: Loaded trusted root CAs for ISO 15118-20 mutual TLS: OEM=%zu bytes, V2G=%zu bytes",
                    oem_root_ca_pem_len_iso20 - 1, v2g_root_ca_pem_len_iso20 - 1);

    return true;

#else // !USE_EMBEDDED_TLS_CERTS
    // Load ISO 15118-2 certificate chain from LittleFS
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

    // Load ISO 15118-20 certificate chain from LittleFS
    if (!LittleFS.exists(ISO15118_20_CERT_CHAIN_PATH)) {
        logger.printfln("ISOTLS: ISO20 certificate chain file not found: %s", ISO15118_20_CERT_CHAIN_PATH);
        return false;
    }

    cert_file = LittleFS.open(ISO15118_20_CERT_CHAIN_PATH, "r");
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

    bytes_read = cert_file.read(cert_chain_pem_iso20, cert_chain_pem_len_iso20 - 1);
    cert_chain_pem_iso20[bytes_read] = 0;
    cert_file.close();

    if (!LittleFS.exists(ISO15118_20_PRIVATE_KEY_PATH)) {
        logger.printfln("ISOTLS: ISO20 private key file not found: %s", ISO15118_20_PRIVATE_KEY_PATH);
        return false;
    }

    key_file = LittleFS.open(ISO15118_20_PRIVATE_KEY_PATH, "r");
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

    // Load trusted root CA certificates for mutual TLS (ISO 15118-20)
    // [V2G20-2400] SECC shall request EVCC certificate via CertificateRequest
    // These are optional - if not present, mutual auth will be disabled
    if (LittleFS.exists(ISO15118_20_OEM_ROOT_CA_PATH)) {
        File ca_file = LittleFS.open(ISO15118_20_OEM_ROOT_CA_PATH, "r");
        if (ca_file) {
            oem_root_ca_pem_len_iso20 = ca_file.size() + 1;
            oem_root_ca_pem_iso20 = static_cast<uint8_t*>(calloc_psram_or_dram(oem_root_ca_pem_len_iso20, 1));
            if (oem_root_ca_pem_iso20 != nullptr) {
                bytes_read = ca_file.read(oem_root_ca_pem_iso20, oem_root_ca_pem_len_iso20 - 1);
                oem_root_ca_pem_iso20[bytes_read] = 0;
                logger.printfln("ISOTLS: Loaded OEM Root CA from LittleFS");
            }
            ca_file.close();
        }
    } else {
        logger.printfln("ISOTLS: OEM Root CA not found at %s (mutual TLS will use V2G Root CA only)", ISO15118_20_OEM_ROOT_CA_PATH);
    }

    if (LittleFS.exists(ISO15118_20_V2G_ROOT_CA_PATH)) {
        File ca_file = LittleFS.open(ISO15118_20_V2G_ROOT_CA_PATH, "r");
        if (ca_file) {
            v2g_root_ca_pem_len_iso20 = ca_file.size() + 1;
            v2g_root_ca_pem_iso20 = static_cast<uint8_t*>(calloc_psram_or_dram(v2g_root_ca_pem_len_iso20, 1));
            if (v2g_root_ca_pem_iso20 != nullptr) {
                bytes_read = ca_file.read(v2g_root_ca_pem_iso20, v2g_root_ca_pem_len_iso20 - 1);
                v2g_root_ca_pem_iso20[bytes_read] = 0;
                logger.printfln("ISOTLS: Loaded V2G Root CA from LittleFS");
            }
            ca_file.close();
        }
    } else {
        logger.printfln("ISOTLS: V2G Root CA not found at %s (mutual TLS will use OEM Root CA only)", ISO15118_20_V2G_ROOT_CA_PATH);
    }

    if (oem_root_ca_pem_iso20 == nullptr && v2g_root_ca_pem_iso20 == nullptr) {
        logger.printfln("ISOTLS: WARNING: No trusted root CAs found for ISO 15118-20 mutual TLS");
    }

    return true;
#endif // USE_EMBEDDED_TLS_CERTS
}

bool ISOTLS::setup()
{
    if (initialized) {
        return true;
    }

    logger.printfln("ISOTLS: Setting up TLS for ISO 15118 (TLS 1.2 + 1.3)");

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
    cert_chain_iso2 = static_cast<mbedtls_x509_crt*>(calloc_psram_or_dram(1, sizeof(mbedtls_x509_crt)));
    private_key_iso2 = static_cast<mbedtls_pk_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_pk_context)));
    cert_chain_iso20 = static_cast<mbedtls_x509_crt*>(calloc_psram_or_dram(1, sizeof(mbedtls_x509_crt)));
    private_key_iso20 = static_cast<mbedtls_pk_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_pk_context)));

    if (ssl == nullptr || ssl_conf == nullptr || entropy == nullptr || ctr_drbg == nullptr ||
        cert_chain_iso2 == nullptr || private_key_iso2 == nullptr ||
        cert_chain_iso20 == nullptr || private_key_iso20 == nullptr) {
        logger.printfln("ISOTLS: Failed to allocate mbedTLS contexts");
        cleanup();
        return false;
    }

    // Initialize mbedTLS contexts
    mbedtls_ssl_init(ssl);
    mbedtls_ssl_config_init(ssl_conf);
    mbedtls_entropy_init(entropy);
    mbedtls_ctr_drbg_init(ctr_drbg);
    mbedtls_x509_crt_init(cert_chain_iso2);
    mbedtls_pk_init(private_key_iso2);
    mbedtls_x509_crt_init(cert_chain_iso20);
    mbedtls_pk_init(private_key_iso20);

    int ret;

    // Seed the random number generator
    const char *pers = "iso15118_secc";
    ret = mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy,
                                reinterpret_cast<const unsigned char *>(pers), strlen(pers));
    if (ret != 0) {
        logger.printfln("ISOTLS: mbedtls_ctr_drbg_seed failed: -0x%04x", static_cast<unsigned>(-ret));
        cleanup();
        return false;
    }

    // Parse ISO 15118-2 certificates (secp256r1)
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

    // Parse ISO 15118-20 certificates (secp521r1)
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

    // Parse trusted root CA certificates for mutual TLS (ISO 15118-20)
    // [V2G20-2400] SECC shall request EVCC certificate via CertificateRequest
    // [V2G20-2338] SECC shall have at least one V2G or OEM root CA certificate
    trusted_ca_iso20 = static_cast<mbedtls_x509_crt*>(calloc_psram_or_dram(1, sizeof(mbedtls_x509_crt)));
    if (trusted_ca_iso20 == nullptr) {
        logger.printfln("ISOTLS: Failed to allocate trusted CA context");
        cleanup();
        return false;
    }
    mbedtls_x509_crt_init(trusted_ca_iso20);

    int trusted_ca_count = 0;
    if (oem_root_ca_pem_iso20 != nullptr) {
        ret = mbedtls_x509_crt_parse(trusted_ca_iso20, oem_root_ca_pem_iso20, oem_root_ca_pem_len_iso20);
        if (ret != 0) {
            logger.printfln("ISOTLS: OEM Root CA parse failed: -0x%04x", static_cast<unsigned>(-ret));
        } else {
            trusted_ca_count++;
            logger.printfln("ISOTLS: OEM Root CA parsed successfully");
        }
    }

    if (v2g_root_ca_pem_iso20 != nullptr) {
        ret = mbedtls_x509_crt_parse(trusted_ca_iso20, v2g_root_ca_pem_iso20, v2g_root_ca_pem_len_iso20);
        if (ret != 0) {
            logger.printfln("ISOTLS: V2G Root CA parse failed: -0x%04x", static_cast<unsigned>(-ret));
        } else {
            trusted_ca_count++;
            logger.printfln("ISOTLS: V2G Root CA parsed successfully");
        }
    }

    if (trusted_ca_count > 0) {
        logger.printfln("ISOTLS: %d trusted root CA(s) loaded for ISO 15118-20 mutual TLS", trusted_ca_count);
    } else {
        logger.printfln("ISOTLS: WARNING: No trusted root CAs loaded - mutual TLS authentication disabled");
        mbedtls_x509_crt_free(trusted_ca_iso20);
        free_any(trusted_ca_iso20);
        trusted_ca_iso20 = nullptr;
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
    // TLS Version, Cipher Suite and Named Group Configuration
    // =========================================================================
    // Allow both TLS 1.2 and 1.3, let client negotiate
    // [V2G20-2356] If TLS 1.2 negotiated, SECC shall not select ISO 15118-20
    mbedtls_ssl_conf_min_tls_version(ssl_conf, MBEDTLS_SSL_VERSION_TLS1_2);
    mbedtls_ssl_conf_max_tls_version(ssl_conf, MBEDTLS_SSL_VERSION_TLS1_3);
    mbedtls_ssl_conf_ciphersuites(ssl_conf, iso15118_ciphersuites);
    mbedtls_ssl_conf_groups(ssl_conf, iso15118_curves);

    // =========================================================================
    // Authentication Mode
    // =========================================================================
    // Default: VERIFY_NONE (safe fallback for TLS 1.2 / ISO 15118-2)
    // For TLS 1.3 / ISO 15118-20, mutual authentication is enabled
    // per-handshake in the cert_cb callback using:
    //   mbedtls_ssl_set_hs_authmode(MBEDTLS_SSL_VERIFY_REQUIRED)
    //   mbedtls_ssl_set_hs_ca_chain(trusted_ca_iso20)
    // This way TLS 1.2 stays unilateral and TLS 1.3 gets mutual auth.
    mbedtls_ssl_conf_authmode(ssl_conf, MBEDTLS_SSL_VERIFY_NONE);

    mbedtls_ssl_conf_cert_profile(ssl_conf, &mbedtls_x509_crt_profile_custom);

    // Set random number generator
    mbedtls_ssl_conf_rng(ssl_conf, mbedtls_ctr_drbg_random, ctr_drbg);

    // =========================================================================
    // Certificate selection callback
    // =========================================================================
    // A certificate selection callback (f_cert_cb) fires after ClientHello
    // processing. At that point, the negotiated TLS version is known, so the
    // callback:
    //   - TLS 1.3 -> ISO 15118-20 cert (secp521r1) + mutual auth with trusted CAs
    //   - TLS 1.2 -> ISO 15118-2 cert  (secp256r1) + unilateral auth (no client cert)
    // This is deterministic and independent of client signature_algorithms ordering.
    s_isotls_instance = this;
    mbedtls_ssl_conf_cert_cb(ssl_conf, tls_cert_selection_callback);
    logger.printfln("ISOTLS: Registered certificate selection callback");

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

    if (s_isotls_instance == this) {
        s_isotls_instance = nullptr;
    }

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

    if (trusted_ca_iso20 != nullptr) {
        mbedtls_x509_crt_free(trusted_ca_iso20);
        free_any(trusted_ca_iso20);
        trusted_ca_iso20 = nullptr;
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

    if (oem_root_ca_pem_iso20 != nullptr) {
        free_any(oem_root_ca_pem_iso20);
        oem_root_ca_pem_iso20 = nullptr;
        oem_root_ca_pem_len_iso20 = 0;
    }

    if (v2g_root_ca_pem_iso20 != nullptr) {
        free_any(v2g_root_ca_pem_iso20);
        v2g_root_ca_pem_iso20 = nullptr;
        v2g_root_ca_pem_len_iso20 = 0;
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

// Don't inline error logger, keeps the buffer off the stack.
[[gnu::noinline]]
static void log_mbedtls_error(int error, const char *msg)
{
    char error_buf[128];
    mbedtls_strerror(error, error_buf, sizeof(error_buf));
    logger.printfln("ISOTLS: %s: -0x%04x (%s)", msg, static_cast<unsigned>(-error), error_buf);
}

bool ISOTLS::do_handshake()
{
    if (!initialized || ssl == nullptr) {
        logger.printfln("ISOTLS: TLS not initialized");
        return false;
    }

    int ret = mbedtls_ssl_handshake(ssl);

    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
        // Handshake in progress, need to call again
        return false;
    }

    if (verification_context != nullptr && verification_context->async_started) {
        xQueueSemaphoreTake(verification_context->sem_handle, portMAX_DELAY_nowarn);
        verification_context->async_started = false;
    }

    if (ret == 0) {
        if (verification_context->intermediates_valid) {
            // Handshake completed successfully
            handshake_state = TlsHandshakeState::COMPLETED;
            session_active = true;

            const char *tls_version = get_tls_version_string();
            const char *cipher = get_cipher_suite();

            logger.printfln("ISOTLS: Handshake successful: %s, using %s", tls_version ? tls_version : "TLS version unknown", cipher ? cipher : "unknown cipher suite");

            if (!verification_context->leaf_cert_cached) {
                cache_leaf_cert();
            }

            // [V2G20-2356] If TLS 1.2 or lower, SECC shall not select ISO 15118-20
            if (is_tls13_active()) {
                logger.printfln("ISOTLS: TLS 1.3 negotiated - ISO 15118-20 allowed");

                // Log mutual authentication result
                if (mutual_auth_enabled && trusted_ca_iso20 != nullptr) {
                    uint32_t verify_flags = mbedtls_ssl_get_verify_result(ssl);
                    if (verify_flags == 0) {
                        logger.printfln("ISOTLS: Mutual TLS: EVCC certificate verified successfully");
                    } else {
                        // This should not happen with VERIFY_REQUIRED (handshake would have failed),
                        // but log it for completeness
                        char vrfy_buf[256];
                        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", verify_flags);
                        logger.printfln("ISOTLS: Mutual TLS: EVCC certificate verification issues:\n%s", vrfy_buf);
                    }
                }
            } else {
                logger.printfln("ISOTLS: TLS 1.2 negotiated - ISO 15118-20 NOT allowed per [V2G20-2356]");
            }
        } else {
            logger.printfln("Intermediate certificate validation failed");
            ret = MBEDTLS_ERR_X509_CERT_VERIFY_FAILED;

            // end_session() won't send a session close alert because the session isn't marked as active, so send an appropriate alert here.
            mbedtls_ssl_send_alert_message(ssl, MBEDTLS_SSL_ALERT_LEVEL_FATAL, MBEDTLS_SSL_ALERT_MSG_BAD_CERT);
        }
    }

    if (verification_context != nullptr) {
        vQueueDelete(static_cast<QueueHandle_t>(verification_context->sem_handle));
        free(verification_context);
        verification_context = nullptr;
    }

    if (ret == 0) {
        return true;
    } else {
        // Handshake failed
        log_mbedtls_error(ret, "Handshake failed");
        handshake_state = TlsHandshakeState::FAILED;

        return false;
    }
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

    log_mbedtls_error(ret, "Read error");
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

    log_mbedtls_error(ret, "Write error");
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

    return mbedtls_ssl_get_version_number(ssl) == MBEDTLS_SSL_VERSION_TLS1_3;
}

bool ISOTLS::leaf_cert_is_cached()
{
    const mbedtls_x509_crt *leaf_cert = verification_context->certs[0];
    mbedtls_sha256(leaf_cert->raw.p, leaf_cert->raw.len, verification_context->leaf_sha256, 0);

    for (cert_cache_entry *entry = peer_cert_cache; entry != nullptr; entry = entry->next) {
        if (memcmp(verification_context->leaf_sha256, entry->sha256, sizeof(entry->sha256)) == 0) {
            entry->last_seen = now_us();
            logger.printfln("ISOTLS: Found cached certificate of peer '%s'", entry->dn);

            return true;
        }
    }

    return false;
}

void ISOTLS::cache_leaf_cert()
{
    size_t entry_count = 0;
    micros_t lru = std::numeric_limits<micros_t>::max();

    cert_cache_entry **lru_entry_ptr = nullptr;
    cert_cache_entry *lru_entry = nullptr;

    cert_cache_entry **entry_ptr = &peer_cert_cache;
    cert_cache_entry *entry = peer_cert_cache;

    while (entry != nullptr) {
        entry_count++;

        if (entry->last_seen < lru) {
            lru = entry->last_seen;
            lru_entry_ptr = entry_ptr;
            lru_entry = entry;
        }

        entry_ptr = &entry->next;
        entry = entry->next;
    }

    if (entry_count < 10) {
        // Allocate new entry
        entry = static_cast<cert_cache_entry *>(perm_aligned_alloc_prefer(alignof(cert_cache_entry), sizeof(cert_cache_entry), RAM::PSRAM, RAM::DRAM));
    } else {
        // Reuse existing entry
        *lru_entry_ptr = lru_entry->next;
        entry = lru_entry;
        free(entry->dn);
    }

    // Insert at the beginning
    entry->next = peer_cert_cache;
    peer_cert_cache = entry;

    entry->last_seen = now_us();
    memcpy(entry->sha256, verification_context->leaf_sha256, sizeof(entry->sha256));

    const mbedtls_x509_crt *leaf_cert = verification_context->certs[0];
    const mbedtls_asn1_buf &subject = leaf_cert->subject.val;
    const size_t subject_len = subject.len;

    entry->dn = static_cast<char *>(malloc_psram_or_dram(subject_len + 1));
    memcpy(entry->dn, subject.p, subject_len);
    entry->dn[subject_len] = 0;

    logger.printfln("ISOTLS: Caching certificate of peer '%s'", entry->dn);
}

static bool cert_signature_is_valid(const mbedtls_x509_crt *child, mbedtls_x509_crt *parent)
{
    const int sig_result = x509_crt_check_signature(child, parent, nullptr);

    if (sig_result == 0) {
        return true;
    }

    log_mbedtls_error(sig_result, "Certificate signature check failed");
    return false;
}

void ISOTLS::verify_intermediate_certs()
{
    mbedtls_x509_crt **certs = verification_context->certs;
    bool success = true;

    // Check all intermediate certificates; first is leaf, last is root
    for (size_t i = 1; i < CERTS_MAX_VERIFY - 1; i++) {
        mbedtls_x509_crt *child  = certs[i];
        mbedtls_x509_crt *parent = certs[i + 1];

        if (parent == nullptr) {
            // End of chain, don't check root signature
            break;
        }

        if (!cert_signature_is_valid(child, parent)) {
            logger.printfln("ISOTLS: Intermediate certificate %zu failed verification", i);
            success = false;
            break;
        }
    }

    verification_context->intermediates_valid = success;
}

void ISOTLS::verify_certs_task(void *ctx)
{
    ISOTLS *isotls = static_cast<ISOTLS *>(ctx);
    isotls->verify_intermediate_certs();

    // Wake main task
    xSemaphoreGive_nowarn(isotls->verification_context->sem_handle);

    // Exit RTOS task
    vTaskDelete(NULL);
}

int ISOTLS::cert_verify(void *ctx, mbedtls_x509_crt *cert, int index, uint32_t *flags)
{
    if (index < 0) {
        return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
    }

    if (static_cast<size_t>(index) > CERTS_MAX_VERIFY - 1) {
        logger.printfln("Too many certificates in chain: %i/%zu", index + 1, CERTS_MAX_VERIFY);
        return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
    }

    // Log peer certificate issuer -> subject for debugging
    logger.printfln("ISOTLS: EVCC certificate: %.*s -> %.*s", static_cast<int>(cert->issuer.val.len), cert->issuer.val.p, static_cast<int>(cert->subject.val.len), cert->subject.val.p);

    ISOTLS *isotls = static_cast<ISOTLS *>(ctx);
    verification_context_t *verify_ctx = isotls->verification_context;
    verify_ctx->certs[index] = cert;

    if (index > 0) {
        // Leaf not reached, more certs to come.
        return 0;
    }

    // Leaf reached, verify certificates.

    mbedtls_x509_crt *leaf_cert = verify_ctx->certs[0];
    mbedtls_x509_crt *ca_cert   = verify_ctx->certs[1];

    if (ca_cert == nullptr) {
        logger.printfln("ISOTLS: Rejecting self-signed peer certificate");
        *flags |= MBEDTLS_X509_BADCERT_NOT_TRUSTED;

        return 0; // No error; verification failure is not an error
    }

    if (isotls->leaf_cert_is_cached()) {
        verify_ctx->leaf_cert_cached = true;
        verify_ctx->intermediates_valid = true;

        return 0;
    }

    if (verify_ctx->certs[2] == nullptr) {
        // No intermediate certificates to check
        verify_ctx->intermediates_valid = true;
    } else {
        const BaseType_t ret = xTaskCreatePinnedToCore(verify_certs_task, "verify_certs", 3072, ctx, 10, nullptr, 0); // Priority above httpd but below all other core 0 tasks.

        if (ret == pdPASS_nowarn) {
            verify_ctx->async_started = true;
        } else {
            logger.printfln("ISOTLS: verify_certs task could not be created");

            // Verify certs now. This will probably cause the peer to time out.
            isotls->verify_intermediate_certs();
        }
    }

    // Verify leaf certificate
    if (!cert_signature_is_valid(leaf_cert, ca_cert)) {
        logger.printfln("ISOTLS: Leaf certificate failed verification");
        *flags |= MBEDTLS_X509_BADCERT_NOT_TRUSTED;
    }

    return 0; // No error; verification failure is not an error
}

int ISOTLS::select_certificate_for_handshake(mbedtls_ssl_context *ssl_ctx)
{
    size_t sni_len;
    const unsigned char *sni = mbedtls_ssl_get_hs_sni(ssl_ctx, &sni_len);

    if (sni_len > 0) {
        logger.printfln("ISOTLS: cert_cb: EVCC thinks we are '%.*s'", static_cast<int>(sni_len), sni);
    } else {
        logger.printfln("ISOTLS: cert_cb: No SNI from EVCC");
    }

    mbedtls_ssl_protocol_version ver = mbedtls_ssl_get_version_number(ssl_ctx);

    if (ver == MBEDTLS_SSL_VERSION_TLS1_3 && cert_chain_iso20 != nullptr && private_key_iso20 != nullptr) {
        logger.printfln("ISOTLS: cert_cb: TLS 1.3 negotiated, selecting ISO 15118-20 cert (secp521r1)");
        int ret = mbedtls_ssl_set_hs_own_cert(ssl_ctx, cert_chain_iso20, private_key_iso20);
        if (ret != 0) {
            logger.printfln("ISOTLS: cert_cb: Failed to set own cert: -0x%04x", static_cast<unsigned>(-ret));
            return ret;
        }

        // [V2G20-2400] SECC shall request EVCC certificate via CertificateRequest
        // Enable mutual authentication for TLS 1.3 (ISO 15118-20) if enabled
        if (mutual_auth_enabled) {
            if (trusted_ca_iso20 != nullptr) {
                mbedtls_ssl_set_hs_ca_chain(ssl_ctx, trusted_ca_iso20, nullptr);
                mbedtls_ssl_set_hs_authmode(ssl_ctx, MBEDTLS_SSL_VERIFY_REQUIRED);

                assert(verification_context == nullptr);

                verification_context = static_cast<decltype(verification_context)>(heap_caps_calloc(1, sizeof(*verification_context), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));

                if (verification_context == nullptr) {
                    return MBEDTLS_ERR_SSL_ALLOC_FAILED;
                }

                verification_context->sem_handle = xSemaphoreCreateBinaryStatic_nowarn(&verification_context->sem_buf);

                if (verification_context->sem_handle == nullptr) {
                    free(verification_context);
                    verification_context = nullptr;

                    return MBEDTLS_ERR_SSL_ALLOC_FAILED;
                }

                mbedtls_ssl_set_verify(ssl_ctx, &cert_verify, this);

                logger.printfln("ISOTLS: cert_cb: Mutual TLS enabled - EVCC certificate will be verified");
            } else {
                logger.printfln("ISOTLS: cert_cb: WARNING: No trusted CAs loaded, mutual TLS disabled");
            }
        } else {
            mbedtls_ssl_set_hs_authmode(ssl_ctx, MBEDTLS_SSL_VERIFY_NONE);
            logger.printfln("ISOTLS: cert_cb: Mutual TLS disabled by configuration");
        }

        return 0;
    } else if (cert_chain_iso2 != nullptr && private_key_iso2 != nullptr) {
        logger.printfln("ISOTLS: cert_cb: TLS 1.2 negotiated, selecting ISO 15118-2 cert (secp256r1)");
        // ISO 15118-2: Unilateral authentication only (no client cert)
        mbedtls_ssl_set_hs_authmode(ssl_ctx, MBEDTLS_SSL_VERIFY_NONE);
        return mbedtls_ssl_set_hs_own_cert(ssl_ctx, cert_chain_iso2, private_key_iso2);
    }

    logger.printfln("ISOTLS: cert_cb: No matching certificate available for TLS version 0x%04x",
                    static_cast<unsigned>(ver));
    return -1;
}

void ISOTLS::set_mutual_auth_enabled(bool enabled)
{
    mutual_auth_enabled = enabled;
    logger.printfln("ISOTLS: Mutual TLS authentication %s", enabled ? "enabled" : "disabled");
}

bool ISOTLS::is_mutual_auth_enabled() const
{
    return mutual_auth_enabled;
}
