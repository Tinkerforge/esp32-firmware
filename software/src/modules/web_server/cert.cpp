/* esp32-firmware
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
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
#include "cert.h"

#include <entropy_poll.h>
#include <mbedtls/base64.h>
#include "mbedtls/ecp.h"
#include <mbedtls/entropy.h>
#include <mbedtls/oid.h>
#include <mbedtls/pem.h>
#include <LittleFS.h>

#include "build.h"
#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "tools/string_builder.h"

#include "gcc_warnings.h"

[[gnu::noinline]]
static void log_mbedtls_error(int error, const char *msg)
{
    char error_buf[128];
    mbedtls_strerror(error, error_buf, sizeof(error_buf));
    logger.printfln("Cert: %s: -0x%04x (%s)", msg, static_cast<unsigned>(-error), error_buf);
}

[[gnu::noinline]]
static void log_mbedtls_error(int error, String *cert_error)
{
    if (error == MBEDTLS_ERR_X509_INVALID_FORMAT) {
        *cert_error = "Failed to parse DER certificate: Selected certificate doesn't look like a certificate. Private key selected?";
        return;
    } else if (error == MBEDTLS_ERR_PK_KEY_INVALID_FORMAT) {
        *cert_error = "Failed to load private key: Selected key doesn't look like a private key. Certificate selected?";
        return;
    } else if (error == MBEDTLS_ERR_PK_TYPE_MISMATCH || error == MBEDTLS_ERR_ECP_BAD_INPUT_DATA) {
        *cert_error = "Certificate and key mismatch: Private key does not belong to this certificate or certificate chain reversed.";
        return;
    } else if (error == MBEDTLS_ERR_RSA_KEY_CHECK_FAILED) {
        *cert_error = "Invalid RSA key. Key below minimum 2048 bit length?";
        return;
    }

    char buf[256];
    StringWriter sw{buf, sizeof(buf)};

    sw.printf("-0x%04x (", static_cast<unsigned>(-error));

    char *error_start = sw.getRemainingPtr();
    mbedtls_strerror(error, error_start, sw.getRemainingLength());
    sw.setLength(sw.getLength() + strlen(error_start));

    sw.putc(')');

    *cert_error = sw.getPtr();
}

[[gnu::noinline]]
static void log_cert_subject(const mbedtls_x509_crt *crt, const char *origin_cert_type)
{
    char subject[256];
    int ret = mbedtls_x509_dn_gets(subject, std::size(subject), &crt->subject);

    if (ret < 0) {
        log_mbedtls_error(ret, "Failed to parse subject");
        return;
    }

    const char *cn = strstr(subject, "CN=");

    if (cn == nullptr) {
        cn = subject;
    } else {
        cn += 3; // Skip CN=
    }

    logger.printfln("Loaded %s certificate for %s", origin_cert_type, cn);
}

[[gnu::noinline]]
[[gnu::nonnull]]
static int parse_certificate(std::unique_ptr<uint8_t[]> *uniq_cert_buf, size_t *cert_length, mbedtls_x509_crt *crt, bool crt_own_copy)
{
    if (*cert_length > 27 && strncmp(reinterpret_cast<const char *>(uniq_cert_buf->get()), "-----BEGIN CERTIFICATE-----", 27) == 0) {
        // Cert(s) in PEM format, include null termination.
        (*cert_length)++;

        mbedtls_pem_context ctx;
        mbedtls_pem_init(&ctx);

        size_t use_len;
        int ret = mbedtls_pem_read_buffer(
            &ctx,
            "-----BEGIN CERTIFICATE-----",
            "-----END CERTIFICATE-----",
            reinterpret_cast<const unsigned char *>(uniq_cert_buf->get()),
            nullptr, 0, // Encrypted certificate not supported
            &use_len);

        if (ret != 0) {
            mbedtls_pem_free(&ctx);
            log_mbedtls_error(ret, "Failed to decode PEM");
            return ret;
        }

        if (*cert_length - use_len > 1) {
            mbedtls_pem_free(&ctx);

            if (crt_own_copy) {
                ret = mbedtls_x509_crt_parse(crt, uniq_cert_buf->get(), *cert_length);

                if (ret == 0) {
                    mbedtls_x509_crt *c = crt;
                    do {
                        log_cert_subject(c, "PEM");
                        c = c->next;
                    } while (c != nullptr);
                    return 0;
                } else {
                    if (ret < 0) {
                        log_mbedtls_error(ret, "Failed to parse PEM chain");
                        return ret;
                    } else {
                        logger.printfln("Cert: Failed to parse some certificates in chain");
                        return MBEDTLS_ERR_X509_FATAL_ERROR;
                    }
                }
            } else {
                // Can't pass decoded certificate chain to the webserver yet. Keep the PEM version.
                logger.printfln("Loaded PEM certificate chain");
                return 0;
            }
        }

        size_t buflen = 0;
        const uint8_t *buf = mbedtls_pem_get_buffer(&ctx, &buflen);

        if (buf == nullptr) {
            mbedtls_pem_free(&ctx);
            logger.printfln("No PEM buffer :-?");
            return 0;
        }

        if (crt_own_copy) {
            ret = mbedtls_x509_crt_parse_der(crt, buf, buflen);
        } else {
            ret = mbedtls_x509_crt_parse_der_nocopy(crt, buf, buflen);
        }

        if (ret == 0) {
            log_cert_subject(crt, "PEM");

            if (!crt_own_copy) {
                uint8_t *der_buf = static_cast<uint8_t *>(malloc_psram_or_dram(buflen));
                memcpy(der_buf, buf, buflen);

                // Free PEM context before replacing uniq_cert_buf, in case it holds any references
                mbedtls_pem_free(&ctx);

                //logger.printfln("PEM -> DER  %zu -> %zu", *cert_length, buflen);

                // Replace source PEM buffer with decoded DER buffer.
                uniq_cert_buf->reset(der_buf);
                *cert_length = buflen;
            }
        } else {
            log_mbedtls_error(ret, "Failed to parse PEM certificate");
        }

        mbedtls_pem_free(&ctx);

        return ret;
    } else {
        // Try cert in DER format

        int ret;
        if (crt_own_copy) {
            ret = mbedtls_x509_crt_parse_der(crt, uniq_cert_buf->get(), *cert_length);
        } else {
            ret = mbedtls_x509_crt_parse_der_nocopy(crt, uniq_cert_buf->get(), *cert_length);
        }

        if (ret == 0) {
            log_cert_subject(crt, "DER");
        } else {
            log_mbedtls_error(ret, "Failed to parse DER certificate");
        }

        return ret;
    }
}

static int fallback_hw_rng(void *rng_state, unsigned char *output, size_t len)
{
    size_t olen;
    return mbedtls_hardware_poll(rng_state, output, len, &olen);
}

[[gnu::noinline]]
[[gnu::nonnull]]
static int parse_key(std::unique_ptr<uint8_t[]> *uniq_key_buf, size_t *key_length, mbedtls_pk_context *pk, mbedtls_f_rng_t *f_rng, void *p_rng)
{
    bool is_pem;

    if (*key_length > 11 && strncmp(reinterpret_cast<const char *>(uniq_key_buf->get()), "-----BEGIN ", 11) == 0) {
        // Key in PEM format, include null termination.
        (*key_length)++;
        is_pem = true;
    } else {
        is_pem = false;
    }

    int ret = mbedtls_pk_parse_key(pk, uniq_key_buf->get(), *key_length, nullptr, 0, f_rng, p_rng);

    if (ret != 0) {
        log_mbedtls_error(ret, "Failed to load private key");
        return ret;
    }

    const mbedtls_pk_type_t key_type = mbedtls_pk_get_type(pk);

    switch (key_type) {
        case MBEDTLS_PK_RSA:
        case MBEDTLS_PK_RSA_ALT:
        case MBEDTLS_PK_RSASSA_PSS: {
            const char *key_name = mbedtls_pk_get_name(pk);
            const size_t key_bits = mbedtls_pk_get_bitlen(pk);

            if (key_bits < 2048) {
                logger.printfln("Rejecting insecure %s private key with only %zu bit", key_name, key_bits);
                return MBEDTLS_ERR_RSA_KEY_CHECK_FAILED;
            } else {
                logger.printfln("Loaded %s private key (%zu bit)", key_name, key_bits);
            }

            break;
        }

        case MBEDTLS_PK_ECKEY:
        case MBEDTLS_PK_ECKEY_DH:
        case MBEDTLS_PK_ECDSA: {
            mbedtls_ecp_keypair *ecp = static_cast<mbedtls_ecp_keypair *>(pk->private_pk_ctx);
            const mbedtls_ecp_curve_info *curve_info = mbedtls_ecp_curve_info_from_grp_id(ecp->private_grp.id);

            logger.printfln("Loaded ECDSA private key (%s)", curve_info->name);
            break;
        }

        case MBEDTLS_PK_NONE:
        case MBEDTLS_PK_OPAQUE:
        default:
            logger.printfln("Unexpected key type %i", key_type);
            return MBEDTLS_ERR_PK_UNKNOWN_PK_ALG;
    }

    if (is_pem) {
        uint8_t *tmp = static_cast<uint8_t *>(malloc(*key_length));

        if (tmp != nullptr) {
            ret = mbedtls_pk_write_key_der(pk, tmp, *key_length);

            if (ret <= 0) {
                log_mbedtls_error(ret, "mbedtls_pk_write_key_der failed");
            } else {
                const size_t der_length = static_cast<size_t>(ret);
                const uint8_t *der_start = tmp + *key_length - der_length;

                uint8_t *der_buf = static_cast<uint8_t *>(malloc_psram_or_dram(der_length));

                if (der_buf != nullptr) {
                    memcpy(der_buf, der_start, der_length);

                    //logger.printfln("PEM -> DER  %zu -> %zu", *key_length, der_length);

                    // Replace source PEM buffer with decoded DER buffer.
                    uniq_key_buf->reset(der_buf);
                    *key_length = der_length;
                }
            }

            free(tmp);
        }
    }

    return 0;
}

[[gnu::noinline]]
static int parse_cert_and_key(std::unique_ptr<uint8_t[]> *uniq_cert_buf, size_t *cert_length, std::unique_ptr<uint8_t[]> *uniq_key_buf, size_t *key_length, mbedtls_x509_crt **crt_out, mbedtls_pk_context **private_key_out, String *cert_error)
{
    // Initialize entropy
    mbedtls_entropy_context entropy;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ctr_drbg_init(&ctr_drbg);

    constexpr const char *drbg_personalization = "web_cert";

    mbedtls_x509_crt crt_local;
    mbedtls_pk_context prvkey_local;

    mbedtls_x509_crt   *crt    = nullptr;
    mbedtls_pk_context *prvkey = nullptr;

    int ret;
    bool crt_own_copy;

    if (crt_out == nullptr) {
        crt = &crt_local;
        crt_own_copy = false;
    } else {
        crt = static_cast<decltype(crt)>(malloc_psram_or_dram(sizeof(*crt)));

        if (crt == nullptr) {
            ret = MBEDTLS_ERR_X509_ALLOC_FAILED;
            goto out;
        }

        crt_own_copy = true;
    }

    mbedtls_x509_crt_init(crt);

    if (private_key_out == nullptr) {
        prvkey = &prvkey_local;
    } else {
        prvkey = static_cast<decltype(prvkey)>(malloc_psram_or_dram(sizeof(*prvkey)));

        if (prvkey == nullptr) {
            ret = MBEDTLS_ERR_PK_ALLOC_FAILED;
            goto out;
        }
    }

    mbedtls_pk_init(prvkey);

    ret = parse_certificate(uniq_cert_buf, cert_length, crt, crt_own_copy);

    if (ret != 0) {
        goto out;
    }

    mbedtls_f_rng_t *f_rng;
    void *p_rng;

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, reinterpret_cast<const unsigned char *>(drbg_personalization), strlen(drbg_personalization));

    if (ret == 0) {
        f_rng = mbedtls_ctr_drbg_random;
        p_rng = &ctr_drbg;
    } else {
        log_mbedtls_error(ret, "mbedtls_ctr_drbg_seed failed");
        f_rng = fallback_hw_rng;
        p_rng = nullptr;
    }

    ret = parse_key(uniq_key_buf, key_length, prvkey, f_rng, p_rng);

    if (ret != 0) {
        goto out;
    }

    ret = mbedtls_pk_check_pair(&crt->pk, prvkey, f_rng, p_rng);

    if (ret != 0) {
        log_mbedtls_error(ret, "Certificate and key mismatch");
        logger.printfln("Cert Subject %.*s", static_cast<int>(crt->subject.val.len), crt->subject.val.p);
        goto out;
    }

    ret = 0;

out:
    if (ret == 0 && private_key_out != nullptr) {
        // Free old key
        if (*private_key_out != nullptr) {
            mbedtls_pk_free(*private_key_out);
            free(*private_key_out);
        }

        *private_key_out = prvkey;
    } else {
        mbedtls_pk_free(prvkey);

        if (prvkey != &prvkey_local) {
            free(prvkey);
        }
    }
    prvkey = nullptr;

    if (ret == 0 && crt_out != nullptr) {
        // Free old cert
        if (*crt_out != nullptr) {
            mbedtls_x509_crt_free(*crt_out);
            free(*crt_out);
        }

        *crt_out = crt;
    } else {
        mbedtls_x509_crt_free(crt);

        if (crt != &crt_local) {
            free(crt);
        }
    }
    crt = nullptr;

    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    if (ret != 0 && cert_error != nullptr) {
        log_mbedtls_error(ret, cert_error);
    }

    return ret;
}

#if MODULE_CERTS_AVAILABLE()
bool Cert::load_external_impl(const cert_load_info *load_info, mbedtls_x509_crt **crt_out, mbedtls_pk_context **key_out, String *cert_error)
{
    if (load_info->cert_id < 0 || load_info->cert_id > std::numeric_limits<uint8_t>::max() ||
        load_info->key_id  < 0 || load_info->key_id  > std::numeric_limits<uint8_t>::max()) {
            return false;
    }

    size_t cert_crt_len = 0;
    std::unique_ptr<uint8_t[]> cert_crt = certs.get_cert(static_cast<uint8_t>(load_info->cert_id), &cert_crt_len);
    if (cert_crt == nullptr) {
        logger.printfln("Certificate with ID %i is not available", load_info->cert_id);
        return false;
    }

    size_t cert_key_len = 0;
    std::unique_ptr<uint8_t[]> cert_key = certs.get_cert(static_cast<uint8_t>(load_info->key_id), &cert_key_len);
    if (cert_key == nullptr) {
        logger.printfln("Key with ID %i is not available", load_info->key_id);
        return false;
    }

    if (parse_cert_and_key(&cert_crt, &cert_crt_len, &cert_key, &cert_key_len, crt_out, key_out, cert_error) != 0) {
        return false;
    }

    if (crt_out == nullptr) {
        crt = std::move(cert_crt);
        key = std::move(cert_key);

        crt_length = static_cast<uint16_t>(cert_crt_len);
        key_length = static_cast<uint16_t>(cert_key_len);
    }

    return true;
}
#else
[[gnu::const]]
bool Cert::load_external_impl(const cert_load_info *load_info, mbedtls_x509_crt **crt_out, mbedtls_pk_context **key_out)
{
    return false;
}
#endif

bool Cert::load_internal_file(const char *path, std::unique_ptr<uint8_t[]> *uniq_buf, size_t *length)
{
    if (!LittleFS.exists(path)) {
        return false;
    }

    File f = LittleFS.open(path, "r");
    if (!f) {
        logger.printfln("Failed to open %s", path);
        return false;
    }

    const size_t file_size = f.size();
    (*uniq_buf) = heap_alloc_array<uint8_t>(file_size + 1); // Always add null-termination.
    uint8_t *buf = uniq_buf->get();

    const size_t read_bytes = f.read(buf, file_size);

    if (read_bytes != file_size) {
        logger.printfln("Read %zu of %zu bytes of %s", read_bytes, file_size, path);
        return false;
    }

    f.close();

    buf[file_size] = 0;
    *length = static_cast<uint16_t>(file_size);

    //logger.printfln("Loaded %s (%zu B)", path, file_size);

    return true;
}

bool Cert::load_internal_files(const cert_load_info *load_info, mbedtls_x509_crt **crt_out, mbedtls_pk_context **key_out, String *cert_error)
{
    std::unique_ptr<uint8_t[]> cert_crt;
    size_t cert_crt_len = 0;

    if (!load_internal_file(load_info->cert_path, &cert_crt, &cert_crt_len)) {
        if (cert_error != nullptr) {
            *cert_error = "Internal certificate could not be loaded";
        }
        return false;
    }

    std::unique_ptr<uint8_t[]> cert_key;
    size_t cert_key_len = 0;

    if (!load_internal_file(load_info->key_path, &cert_key, &cert_key_len)) {
        if (cert_error != nullptr) {
            *cert_error = "Key for internal certificate could not be loaded";
        }
        return false;
    }

    if (parse_cert_and_key(&cert_crt, &cert_crt_len, &cert_key, &cert_key_len, crt_out, key_out, cert_error) != 0) {
        return false;
    }

    if (crt_out == nullptr) {
        crt = std::move(cert_crt);
        key = std::move(cert_key);

        crt_length = static_cast<uint16_t>(cert_crt_len);
        key_length = static_cast<uint16_t>(cert_key_len);
    }

    return true;
}

bool Cert::load_internal_impl(const cert_load_info *load_info, mbedtls_x509_crt **crt_out, mbedtls_pk_context **key_out, String *cert_error)
{
    if (load_internal_files(load_info, crt_out, key_out, nullptr)) { // Ignore errors from first load attempt
        return true;
    }

    if (!generate(load_info)) {
        if (cert_error != nullptr) {
            *cert_error = "Internal certificate and/or key could not be generated";
        }
        return false;
    }

    return load_internal_files(load_info, crt_out, key_out, cert_error);
}

bool Cert::load_external_with_internal_fallback_impl(const cert_load_info *load_info, mbedtls_x509_crt **crt_out, mbedtls_pk_context **key_out, String *cert_error)
{
    if (load_external_impl(load_info, crt_out, key_out, cert_error)) {
        return true;
    }

    return load_internal_impl(load_info, crt_out, key_out, cert_error);
}

bool Cert::load_external(const cert_load_info *load_info)
{
    return load_external_impl(load_info, nullptr, nullptr, nullptr);
}

bool Cert::load_internal(const cert_load_info *load_info)
{
    return load_internal_impl(load_info, nullptr, nullptr, nullptr);
}

bool Cert::load_external_with_internal_fallback(const cert_load_info *load_info)
{
    return load_external_with_internal_fallback_impl(load_info, nullptr, nullptr, nullptr);
}

bool Cert::load_and_export_external(const cert_load_info *load_info, mbedtls_x509_crt **crt_out, mbedtls_pk_context **key_out, String *cert_error)
{
    Cert cert;
    return cert.load_external_impl(load_info, crt_out, key_out, cert_error);
}

bool Cert::load_and_export_internal(const cert_load_info *load_info, mbedtls_x509_crt **crt_out, mbedtls_pk_context **key_out, String *cert_error)
{
    Cert cert;
    return cert.load_internal_impl(load_info, crt_out, key_out, cert_error);
}

bool Cert::load_and_export_external_with_internal_fallback(const cert_load_info *load_info, mbedtls_x509_crt **crt_out, mbedtls_pk_context **key_out, String *cert_error)
{
    Cert cert;
    return cert.load_external_with_internal_fallback_impl(load_info, crt_out, key_out, cert_error);
}

void Cert::get_data(const uint8_t **crt_out, size_t *crt_len_out, const uint8_t **key_out, size_t *key_len_out) const
{
    if (crt == nullptr) {
        *crt_out     = nullptr;
        *crt_len_out = 0;
    } else {
        *crt_out     = crt.get();
        *crt_len_out = crt_length;
    }

    if (key == nullptr) {
        *key_out     = nullptr;
        *key_len_out = 0;
    } else {
        *key_out     = key.get();
        *key_len_out = key_length;
    }
}

bool Cert::is_loaded() const
{
    return key_length != 0;
}

void Cert::free()
{
    if (crt != nullptr) crt.reset();
    if (key != nullptr) key.reset();

    crt_length = 0;
    key_length = 0;
}

[[gnu::noinline]]
bool Cert::generate_cert_and_key(const cert_load_info *load_info)
{
    constexpr size_t MAX_CRT_SIZE = 768;
    constexpr size_t MAX_KEY_SIZE = 512;

    uint8_t *crt_buf;
    uint8_t *key_buf;
    bool retval;

    // Initialize entropy
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    const char *pers = "web_cert";
    int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, reinterpret_cast<const unsigned char *>(pers), strlen(pers));
    if (ret != 0) {
        log_mbedtls_error(ret, "mbedtls_ctr_drbg_seed failed");
        retval = false;
        goto err_out;
    }

    // Initialize cert and key
    mbedtls_x509write_cert mbed_cert;
    mbedtls_pk_context mbed_key;
    mbedtls_x509write_crt_init(&mbed_cert);
    mbedtls_pk_init(&mbed_key);

    if (!load_info->generator_fn(&mbed_cert, &mbed_key, &ctr_drbg)) {
        retval = false;
        goto err_out;
    }

    crt = heap_alloc_array<uint8_t>(MAX_CRT_SIZE);
    key = heap_alloc_array<uint8_t>(MAX_KEY_SIZE);

    crt_buf = crt.get();
    ret = mbedtls_x509write_crt_der(&mbed_cert, crt_buf, MAX_CRT_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret < 0) {
        log_mbedtls_error(ret, "mbedtls_x509write_crt_der failed");
        retval = false;
        goto err_out;
    }
    crt_length = static_cast<uint16_t>(ret);
    memmove(crt_buf, crt_buf + MAX_CRT_SIZE - crt_length, crt_length);
    logger.printfln("Cert (DER) with length %d generated", crt_length);

    key_buf = key.get();
    ret = mbedtls_pk_write_key_der(&mbed_key, key_buf, MAX_KEY_SIZE);
    if (ret < 0) {
        log_mbedtls_error(ret, "mbedtls_pk_write_key_der failed");
        retval = false;
        goto err_out;
    }

    key_length = static_cast<uint16_t>(ret);
    memmove(key_buf, key_buf + MAX_KEY_SIZE - key_length, key_length);
    logger.printfln("Key (DER) with length %d generated", key_length);

    retval = true;

err_out:
    mbedtls_x509write_crt_free(&mbed_cert);
    mbedtls_pk_free(&mbed_key);

    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);

    return retval;
}

[[gnu::noinline]]
bool Cert::write_cert_and_key(const cert_load_info *load_info)
{
    // Save generated cert and key that are in DER format as PEM files
    unsigned char base64[768];
    size_t base64_len = 0;

    std::fill_n(base64, sizeof(base64), 0);
    int ret = mbedtls_base64_encode(base64, sizeof(base64), &base64_len, crt.get(), crt_length);

    File file_cert = LittleFS.open(load_info->cert_path, "w", true);
    if (!file_cert) {
        logger.printfln("Failed to open cert file for writing");
        return false;
    }

    File file_key = LittleFS.open(load_info->key_path, "w", true);
    if (!file_key) {
        logger.printfln("Failed to open key file for writing");
        return false;
    }

    if (ret != 0) {
        log_mbedtls_error(ret, "Base64 encode failed");
    } else {
        file_cert.print("-----BEGIN CERTIFICATE-----");
        file_cert.print("\n"); // Don't include newline in header so that the header string constant can be deduplicated.

        for (size_t i = 0; i < base64_len; i += 64) {
            size_t chunk_size = base64_len - i;
            if (chunk_size > 64) {
                chunk_size = 64;
            }
            file_cert.write(&base64[i], chunk_size);
            file_cert.print("\n");
        }
        file_cert.print("-----END CERTIFICATE-----");
        file_cert.print("\n");
    }
    file_cert.close();

    base64_len = 0;
    std::fill_n(base64, sizeof(base64), 0);
    ret = mbedtls_base64_encode(base64, sizeof(base64), &base64_len, key.get(), key_length);
    if (ret != 0) {
        log_mbedtls_error(ret, "Base64 encode failed");
    } else {
        file_key.print("-----BEGIN EC PRIVATE KEY-----");
        file_key.print("\n");

        for (size_t i = 0; i < base64_len; i += 64) {
            size_t chunk_size = base64_len - i;
            if (chunk_size > 64) {
                chunk_size = 64;
            }
            file_key.write(&base64[i], chunk_size);
            file_key.print("\n");
        }
        file_key.print("-----END EC PRIVATE KEY-----");
        file_key.print("\n");
    }
    file_key.close();

    return true;
}

bool Cert::generate(const cert_load_info *load_info)
{
    if (LittleFS.exists(load_info->key_path)) {
        LittleFS.remove(load_info->key_path);
    }

    if (LittleFS.exists(load_info->cert_path)) {
        LittleFS.remove(load_info->cert_path);
    }

    key_length = 0;
    crt_length = 0;

    if (!generate_cert_and_key(load_info)) {
        return false;
    }

    if (!write_cert_and_key(load_info)) {
        return false;
    }

    return true;
}

[[gnu::noinline]]
static bool cert_set_subject_issuer(mbedtls_x509write_cert *mbed_cert, const String &hostname)
{
    // Set hostname or IP for CN
    char distinguished_name[80];
    snprintf(distinguished_name, std::size(distinguished_name), "C=DE,O=Tinkerforge GmbH,CN=%s.local", hostname.c_str());

    int ret = mbedtls_x509write_crt_set_subject_name(mbed_cert, distinguished_name);
    if (ret != 0) {
        log_mbedtls_error(ret, "mbedtls_x509write_crt_set_subject_name failed");
        return false;
    }
    // Issuer == subject for self-signed certificate
    ret = mbedtls_x509write_crt_set_issuer_name(mbed_cert, distinguished_name);
    if (ret != 0) {
        log_mbedtls_error(ret, "mbedtls_x509write_crt_set_issuer_name failed");
        return false;
    }

    return true;
}

[[gnu::noinline]]
static int cert_set_validity(mbedtls_x509write_cert *mbed_cert)
{
    const time_t t_build = build_timestamp() - 24*60*60;
    struct tm tm;
    gmtime_r(&t_build, &tm);

    char not_before[24];
    char not_after[24];

    snprintf(not_before, std::size(not_before), "%04i0101000000", tm.tm_year + 1900);
    snprintf(not_after,  std::size(not_after),  "%04i0101000000", tm.tm_year + 2000);

    return mbedtls_x509write_crt_set_validity(mbed_cert, not_before, not_after);
}

[[gnu::noinline]]
static int cert_set_key_usage(mbedtls_x509write_cert *mbed_cert)
{
    mbedtls_asn1_sequence item;

    // For HTTPS set extended key usage to serverAuth
    item.buf.tag = MBEDTLS_ASN1_OID;
    item.buf.p = reinterpret_cast<unsigned char *>(const_cast<char *>(MBEDTLS_OID_SERVER_AUTH));
    item.buf.len = MBEDTLS_OID_SIZE(MBEDTLS_OID_SERVER_AUTH);
    item.next = nullptr;

    return mbedtls_x509write_crt_set_ext_key_usage(mbed_cert, &item);
}

[[gnu::noinline]]
static int cert_set_san(mbedtls_x509write_cert *mbed_cert, const String &hostname)
{
    const char *hname = hostname.c_str();

    char hostname_local[48];
    char hostname_fritz[48];

    snprintf(hostname_local, std::size(hostname_local), "%s.local",     hname);
    snprintf(hostname_fritz, std::size(hostname_fritz), "%s.fritz.box", hname);

    mbedtls_x509_san_list san_local;
    mbedtls_x509_san_list san_host;
    mbedtls_x509_san_list san_fritz;
    mbedtls_x509_san_list san_ap_ip;

    san_local.node.type = MBEDTLS_X509_SAN_DNS_NAME;
    san_local.node.san.unstructured_name.p = reinterpret_cast<unsigned char *>(hostname_local);
    san_local.node.san.unstructured_name.len = strlen(hostname_local);
    san_local.next = nullptr;

    san_host.node.type = MBEDTLS_X509_SAN_DNS_NAME;
    san_host.node.san.unstructured_name.p = reinterpret_cast<unsigned char *>(hostname_local);
    san_host.node.san.unstructured_name.len = strlen(hostname_local) - 6; // Cut off ".local"
    san_host.next = &san_local;

    san_fritz.node.type = MBEDTLS_X509_SAN_DNS_NAME;
    san_fritz.node.san.unstructured_name.p = reinterpret_cast<unsigned char *>(hostname_fritz);
    san_fritz.node.san.unstructured_name.len = strlen(hostname_fritz);
    san_fritz.next = &san_host;

    unsigned char ap_ip[] = {10,0,0,1};
    san_ap_ip.node.type = MBEDTLS_X509_SAN_IP_ADDRESS;
    san_ap_ip.node.san.unstructured_name.p = ap_ip;
    san_ap_ip.node.san.unstructured_name.len = sizeof(ap_ip);
    san_ap_ip.next = &san_fritz;

    return mbedtls_x509write_crt_set_subject_alternative_name(mbed_cert, &san_ap_ip);
}

[[gnu::noinline]]
bool Cert::default_cert_fill_fn(mbedtls_x509write_cert *mbed_cert)
{
    mbedtls_x509write_crt_set_md_alg(mbed_cert, MBEDTLS_MD_SHA256);

    // Set serial number, version, issuer name, validity, basic constraints, and subject key identifier
    unsigned char serial[1] = { 0x01 };
    int ret = mbedtls_x509write_crt_set_serial_raw(mbed_cert, serial, sizeof(serial));
    if (ret != 0) {
        log_mbedtls_error(ret, "mbedtls_x509write_crt_set_serial_raw failed");
        return false;
    }

    mbedtls_x509write_crt_set_version(mbed_cert, MBEDTLS_X509_CRT_VERSION_3);

    const String &hostname = network.get_hostname();

    if (!cert_set_subject_issuer(mbed_cert, hostname)) {
        return false;
    }

    ret = cert_set_validity(mbed_cert);
    if (ret != 0) {
        log_mbedtls_error(ret, "mbedtls_x509write_crt_set_validity failed");
        return false;
    }

    // For HTTPS we need CA = FALSE
    ret = mbedtls_x509write_crt_set_basic_constraints(mbed_cert, 0, -1);
    if (ret != 0) {
        log_mbedtls_error(ret, "mbedtls_x509write_crt_set_basic_constraints failed");
        return false;
    }

    // For HTTPS set key usage to digital signature and key encipherment
    ret = mbedtls_x509write_crt_set_key_usage(mbed_cert, MBEDTLS_X509_KU_DIGITAL_SIGNATURE | MBEDTLS_X509_KU_KEY_ENCIPHERMENT);
    if (ret != 0) {
        log_mbedtls_error(ret, "mbedtls_x509write_crt_set_key_usage failed");
        return false;
    }

    ret = cert_set_key_usage(mbed_cert);
    if (ret != 0) {
        log_mbedtls_error(ret, "mbedtls_x509write_crt_set_ext_key_usage failed");
        return false;
    }

    ret = cert_set_san(mbed_cert, hostname);
    if (ret != 0) {
        log_mbedtls_error(ret, "mbedtls_x509write_crt_set_subject_alternative_name failed");
        return false;
    }

    return true;
}

[[gnu::noinline]]
bool Cert::default_key_generator_fn(mbedtls_x509write_cert *mbed_cert, mbedtls_pk_context *mbed_key, mbedtls_ctr_drbg_context *ctr_drbg)
{
    int ret = mbedtls_pk_setup(mbed_key, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if (ret != 0) {
        log_mbedtls_error(ret, "mbedtls_pk_setup failed");
        return false;
    }

    ret = mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1,
                              mbedtls_pk_ec(*mbed_key),
                              mbedtls_ctr_drbg_random, ctr_drbg);
    if (ret != 0) {
        log_mbedtls_error(ret, "mbedtls_ecp_gen_key failed");
        return false;
    }

    mbedtls_x509write_crt_set_subject_key(mbed_cert, mbed_key);
    mbedtls_x509write_crt_set_issuer_key(mbed_cert, mbed_key); // self-signed

    return true;
}

bool Cert::default_certificate_generator_fn(mbedtls_x509write_cert *mbed_cert, mbedtls_pk_context *mbed_key, mbedtls_ctr_drbg_context *ctr_drbg)
{
    if (!Cert::default_key_generator_fn(mbed_cert, mbed_key, ctr_drbg)) {
        return false;
    }

    return Cert::default_cert_fill_fn(mbed_cert);
}
