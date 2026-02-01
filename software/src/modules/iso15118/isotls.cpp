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

#include <sys/socket.h>
#include <errno.h>
#include <string.h>

#include "mbedtls/error.h"
#include "mbedtls/debug.h"

#ifndef USE_EMBEDDED_TLS_CERTS
#include <LittleFS.h>
// Certificate paths
#define ISO15118_CERT_CHAIN_PATH "/iso15118/secc_cert_chain.pem"
#define ISO15118_PRIVATE_KEY_PATH "/iso15118/secc_key.pem"
#endif

// =============================================================================
// ISO 15118-2 cipher suites
// =============================================================================
// [V2G2-602] The SECC shall support all cipher suites defined in Table 7
// Table 7 — Supported cipher suites (ISO 15118-2 Section 7.7.3.4)
static const int iso15118_ciphersuites[] = {
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,  // IETF RFC 5289
    MBEDTLS_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256,   // IETF RFC 5289
    0
};

// TODO: ISO 15118-20 cipher suites (for full -20 TLS 1.3 support)
// [V2G20-2458] The SECC shall support all cipher suites defined in Table 6:
// - TLS_AES_256_GCM_SHA384 (IETF RFC 5116)
// - TLS_CHACHA20_POLY1305_SHA256 (IETF RFC 8439)
// static const int iso15118_20_ciphersuites[] = {
//     MBEDTLS_TLS_AES_256_GCM_SHA384,
//     MBEDTLS_TLS_CHACHA20_POLY1305_SHA256,
//     0
// };

// =============================================================================
// ISO 15118-2 curves
// =============================================================================
// [V2G2-006] ECC-based using secp256r1 curve (SECG notation) with ECDSA
// [V2G2-007] Key length for ECC shall be 256 bit
static const uint16_t iso15118_curves[] = {
    MBEDTLS_SSL_IANA_TLS_GROUP_SECP256R1,
    MBEDTLS_SSL_IANA_TLS_GROUP_NONE
};

// TODO: ISO 15118-20 named groups (for full -20 TLS 1.3 support)
// [V2G20-1634] The SECC shall support all named groups defined in Table 7
// [V2G20-2674] Primary: secp521r1 with ECDSA signature algorithm
// [V2G20-2319] Alternative: x448 (Curve448) with EdDSA
// [V2G20-2675] Key length for ECC shall be 521 bit
// static const uint16_t iso15118_20_curves[] = {
//     MBEDTLS_SSL_IANA_TLS_GROUP_SECP521R1,
//     MBEDTLS_SSL_IANA_TLS_GROUP_X448,
//     MBEDTLS_SSL_IANA_TLS_GROUP_NONE
// };

// mbedTLS send callback for non-blocking socket I/O
static int tls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
    int fd = *static_cast<int*>(ctx);
    ssize_t ret = send(fd, buf, len, 0);

    if (ret < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        }
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
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            return MBEDTLS_ERR_SSL_WANT_READ;
        }
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }
    if (ret == 0) {
        return MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY;
    }
    return static_cast<int>(ret);
}

bool ISOTLS::load_certificates()
{
#ifdef USE_EMBEDDED_TLS_CERTS
    // Use embedded test certificates
    logger.printfln("ISOTLS: Using embedded dev certificates");

    // Copy certificate chain
    cert_chain_pem_len = strlen(dev_cert_chain_pem) + 1;
    cert_chain_pem = (uint8_t*)heap_caps_calloc_prefer(cert_chain_pem_len, 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if (cert_chain_pem == nullptr) {
        logger.printfln("ISOTLS: Failed to allocate memory for certificate chain");
        return false;
    }
    memcpy(cert_chain_pem, dev_cert_chain_pem, cert_chain_pem_len);
    logger.printfln("ISOTLS: Loaded embedded certificate chain (%zu bytes)", cert_chain_pem_len - 1);

    // Copy private key
    private_key_pem_len = strlen(dev_private_key_pem) + 1;
    private_key_pem = (uint8_t*)heap_caps_calloc_prefer(private_key_pem_len, 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if (private_key_pem == nullptr) {
        logger.printfln("ISOTLS: Failed to allocate memory for private key");
        return false;
    }
    memcpy(private_key_pem, dev_private_key_pem, private_key_pem_len);
    logger.printfln("ISOTLS: Loaded embedded private key (%zu bytes)", private_key_pem_len - 1);

    return true;

#else // !USE_EMBEDDED_TLS_CERTS
    // Load certificate chain from LittleFS
    if (!LittleFS.exists(ISO15118_CERT_CHAIN_PATH)) {
        logger.printfln("ISOTLS: Certificate chain file not found: %s", ISO15118_CERT_CHAIN_PATH);
        return false;
    }

    File cert_file = LittleFS.open(ISO15118_CERT_CHAIN_PATH, "r");
    if (!cert_file) {
        logger.printfln("ISOTLS: Failed to open certificate chain file");
        return false;
    }

    cert_chain_pem_len = cert_file.size() + 1; // +1 for null terminator
    cert_chain_pem = (uint8_t*)heap_caps_calloc_prefer(cert_chain_pem_len, 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if (cert_chain_pem == nullptr) {
        logger.printfln("ISOTLS: Failed to allocate memory for certificate chain");
        cert_file.close();
        return false;
    }

    size_t bytes_read = cert_file.read(cert_chain_pem, cert_chain_pem_len - 1);
    cert_chain_pem[bytes_read] = 0; // Null terminate
    cert_file.close();

    logger.printfln("ISOTLS: Loaded certificate chain (%zu bytes)", bytes_read);

    // Load private key from LittleFS
    if (!LittleFS.exists(ISO15118_PRIVATE_KEY_PATH)) {
        logger.printfln("ISOTLS: Private key file not found: %s", ISO15118_PRIVATE_KEY_PATH);
        return false;
    }

    File key_file = LittleFS.open(ISO15118_PRIVATE_KEY_PATH, "r");
    if (!key_file) {
        logger.printfln("ISOTLS: Failed to open private key file");
        return false;
    }

    private_key_pem_len = key_file.size() + 1; // +1 for null terminator
    private_key_pem = (uint8_t*)heap_caps_calloc_prefer(private_key_pem_len, 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if (private_key_pem == nullptr) {
        logger.printfln("ISOTLS: Failed to allocate memory for private key");
        key_file.close();
        return false;
    }

    bytes_read = key_file.read(private_key_pem, private_key_pem_len - 1);
    private_key_pem[bytes_read] = 0; // Null terminate
    key_file.close();

    logger.printfln("ISOTLS: Loaded private key (%zu bytes)", bytes_read);

    return true;
#endif // USE_EMBEDDED_TLS_CERTS
}

bool ISOTLS::setup()
{
    if (initialized) {
        return true;
    }

    logger.printfln("ISOTLS: Setting up TLS for ISO 15118...");

    // Load certificates first
    if (!load_certificates()) {
        logger.printfln("ISOTLS: Failed to load certificates");
        return false;
    }

    // Allocate mbedTLS contexts from PSRAM
    ssl = (mbedtls_ssl_context*)heap_caps_calloc_prefer(sizeof(mbedtls_ssl_context), 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    ssl_conf = (mbedtls_ssl_config*)heap_caps_calloc_prefer(sizeof(mbedtls_ssl_config), 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    cert_chain = (mbedtls_x509_crt*)heap_caps_calloc_prefer(sizeof(mbedtls_x509_crt), 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    private_key = (mbedtls_pk_context*)heap_caps_calloc_prefer(sizeof(mbedtls_pk_context), 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    entropy = (mbedtls_entropy_context*)heap_caps_calloc_prefer(sizeof(mbedtls_entropy_context), 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    ctr_drbg = (mbedtls_ctr_drbg_context*)heap_caps_calloc_prefer(sizeof(mbedtls_ctr_drbg_context), 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);

    if (ssl == nullptr || ssl_conf == nullptr || cert_chain == nullptr ||
        private_key == nullptr || entropy == nullptr || ctr_drbg == nullptr) {
        logger.printfln("ISOTLS: Failed to allocate mbedTLS contexts");
        cleanup();
        return false;
    }

    // Initialize mbedTLS contexts
    mbedtls_ssl_init(ssl);
    mbedtls_ssl_config_init(ssl_conf);
    mbedtls_x509_crt_init(cert_chain);
    mbedtls_pk_init(private_key);
    mbedtls_entropy_init(entropy);
    mbedtls_ctr_drbg_init(ctr_drbg);

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

    // Parse certificate chain
    ret = mbedtls_x509_crt_parse(cert_chain, cert_chain_pem, cert_chain_pem_len);
    if (ret != 0) {
        logger.printfln("ISOTLS: mbedtls_x509_crt_parse failed: -0x%04x", static_cast<unsigned>(-ret));
        cleanup();
        return false;
    }
    logger.printfln("ISOTLS: Certificate chain parsed successfully");

    // Parse private key
    ret = mbedtls_pk_parse_key(private_key, private_key_pem, private_key_pem_len,
                               nullptr, 0, mbedtls_ctr_drbg_random, ctr_drbg);
    if (ret != 0) {
        logger.printfln("ISOTLS: mbedtls_pk_parse_key failed: -0x%04x", static_cast<unsigned>(-ret));
        cleanup();
        return false;
    }
    logger.printfln("ISOTLS: Private key parsed successfully");

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
    // TLS Version Configuration
    // =========================================================================
    // [V2G2-067] TLS version 1.2 according to IETF RFC 5246 shall be supported
    // TODO: [V2G20-1264] ISO 15118-20 requires TLS 1.3 (IETF RFC 8446)
    // TODO: [V2G20-1237] If TLS 1.2 or lower, EVCC shall not offer ISO 15118-20
    // TODO: [V2G20-2356] If TLS 1.2 or lower, SECC shall not select ISO 15118-20
    mbedtls_ssl_conf_min_tls_version(ssl_conf, MBEDTLS_SSL_VERSION_TLS1_2);
    mbedtls_ssl_conf_max_tls_version(ssl_conf, MBEDTLS_SSL_VERSION_TLS1_2);

    // =========================================================================
    // Authentication Mode
    // =========================================================================
    // ISO 15118-2: Unilateral authentication (EVCC authenticates SECC, Section 7.7.3.1)
    // No client certificate verification - WARP is not public infrastructure
    // TODO: ISO 15118-20 requires mutual authentication:
    // TODO: [V2G20-2400] SECC shall request EVCC certificate via CertificateRequest
    // TODO: [V2G20-2432] SECC shall validate EVCC certificate chain
    mbedtls_ssl_conf_authmode(ssl_conf, MBEDTLS_SSL_VERIFY_NONE);

    // =========================================================================
    // Cipher Suites
    // =========================================================================
    // [V2G2-602] Set ISO 15118-2 cipher suites (Table 7)
    // TODO: [V2G20-2458] ISO 15118-20 uses different cipher suites (Table 6)
    mbedtls_ssl_conf_ciphersuites(ssl_conf, iso15118_ciphersuites);

    // =========================================================================
    // Elliptic Curves / Named Groups
    // =========================================================================
    // [V2G2-006] Set secp256r1 curve only
    // TODO: [V2G20-1634] ISO 15118-20 uses secp521r1 and x448 (Table 7)
    mbedtls_ssl_conf_groups(ssl_conf, iso15118_curves);

    // Set random number generator
    mbedtls_ssl_conf_rng(ssl_conf, mbedtls_ctr_drbg_random, ctr_drbg);

    // Set own certificate and private key
    ret = mbedtls_ssl_conf_own_cert(ssl_conf, cert_chain, private_key);
    if (ret != 0) {
        logger.printfln("ISOTLS: mbedtls_ssl_conf_own_cert failed: -0x%04x", static_cast<unsigned>(-ret));
        cleanup();
        return false;
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
        heap_caps_free(ssl);
        ssl = nullptr;
    }

    if (ssl_conf != nullptr) {
        mbedtls_ssl_config_free(ssl_conf);
        heap_caps_free(ssl_conf);
        ssl_conf = nullptr;
    }

    if (cert_chain != nullptr) {
        mbedtls_x509_crt_free(cert_chain);
        heap_caps_free(cert_chain);
        cert_chain = nullptr;
    }

    if (private_key != nullptr) {
        mbedtls_pk_free(private_key);
        heap_caps_free(private_key);
        private_key = nullptr;
    }

    if (entropy != nullptr) {
        mbedtls_entropy_free(entropy);
        heap_caps_free(entropy);
        entropy = nullptr;
    }

    if (ctr_drbg != nullptr) {
        mbedtls_ctr_drbg_free(ctr_drbg);
        heap_caps_free(ctr_drbg);
        ctr_drbg = nullptr;
    }

    if (cert_chain_pem != nullptr) {
        heap_caps_free(cert_chain_pem);
        cert_chain_pem = nullptr;
        cert_chain_pem_len = 0;
    }

    if (private_key_pem != nullptr) {
        heap_caps_free(private_key_pem);
        private_key_pem = nullptr;
        private_key_pem_len = 0;
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
        logger.printfln("ISOTLS: Handshake completed successfully");
        logger.printfln("ISOTLS: Cipher suite: %s", mbedtls_ssl_get_ciphersuite(ssl));
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
