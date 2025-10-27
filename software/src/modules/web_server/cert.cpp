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

#include <mbedtls/base64.h>
#include <mbedtls/entropy.h>
#include <mbedtls/oid.h>
#include <mbedtls/pem.h>
#include <LittleFS.h>

#include "build.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"

#include "gcc_warnings.h"

bool Cert::load_external(const cert_load_info *load_info)
{
    if (load_info->cert_id < 0 || load_info->cert_id > std::numeric_limits<uint8_t>::max() ||
        load_info->key_id  < 0 || load_info->key_id  > std::numeric_limits<uint8_t>::max()) {
            return false;
    }

#if MODULE_CERTS_AVAILABLE()
    size_t cert_crt_len = 0;
    crt = certs.get_cert(static_cast<uint8_t>(load_info->cert_id), &cert_crt_len);
    if (crt == nullptr) {
        logger.printfln("Certificate with ID %i is not available", load_info->cert_id);
        return false;
    }
    logger.printfln("Certificate with ID %i loaded", load_info->cert_id);

    size_t cert_key_len = 0;
    key = certs.get_cert(static_cast<uint8_t>(load_info->key_id), &cert_key_len);
    if (key == nullptr) {
        logger.printfln("Key with ID %i is not available", load_info->key_id);
        return false;
    }
    logger.printfln("Key with ID %i loaded", load_info->key_id);

    crt_length = static_cast<uint16_t>(cert_crt_len);
    key_length = static_cast<uint16_t>(cert_key_len);

    return true;
#else
    return false;
#endif
}

bool Cert::load_internal_file(const char *path, std::unique_ptr<uint8_t[]> *uniq_buf, uint16_t *length)
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

bool Cert::load_internal_files(const cert_load_info *load_info)
{
    if (!load_internal_file(load_info->cert_path, &crt, &crt_length)) {
        return false;
    }

    return load_internal_file(load_info->key_path, &key, &key_length);
}

bool Cert::load_internal(const cert_load_info *load_info)
{
    if (load_internal_files(load_info)) {
        return true;
    }

    if (!generate(load_info)) {
        return false;
    }

    return load_internal_files(load_info);
}

bool Cert::load_external_with_internal_fallback(const cert_load_info *load_info)
{
    if (load_external(load_info)) {
        return true;
    }

    return load_internal(load_info);
}

void Cert::get_data(const uint8_t **crt_out, size_t *crt_len_out, const uint8_t **key_out, size_t *key_len_out)
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

[[gnu::noinline]]
bool Cert::generate_cert_and_key(const cert_load_info *load_info)
{
    constexpr size_t MAX_CRT_SIZE = 768;
    constexpr size_t MAX_KEY_SIZE = 512;

    uint8_t *crt_buf;
    uint8_t *key_buf;
    bool retval;

    mbedtls_x509write_cert mbed_cert = {};
    mbedtls_pk_context mbed_key = {};

    // Initialize entropy
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    const char *pers = "web_cert";
    int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, reinterpret_cast<const unsigned char *>(pers), strlen(pers));
    if (ret != 0) {
        logger.printfln("mbedtls_ctr_drbg_seed failed: 0x%04x", static_cast<unsigned>(ret));
        retval = false;
        goto err_out;
    }

    if (!load_info->generator_fn(&mbed_cert, &mbed_key, &ctr_drbg)) {
        retval = false;
        goto err_out;
    }

    crt = heap_alloc_array<uint8_t>(MAX_CRT_SIZE);
    key = heap_alloc_array<uint8_t>(MAX_KEY_SIZE);

    crt_buf = crt.get();
    ret = mbedtls_x509write_crt_der(&mbed_cert, crt_buf, MAX_CRT_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret < 0) {
        logger.printfln("mbedtls_x509write_crt_der failed: 0x%04x", static_cast<unsigned>(ret));
        retval = false;
        goto err_out;
    }
    crt_length = static_cast<uint16_t>(ret);
    memmove(crt_buf, crt_buf + MAX_CRT_SIZE - crt_length, crt_length);
    logger.printfln("Cert (DER) with length %d generated", crt_length);

    key_buf = key.get();
    ret = mbedtls_pk_write_key_der(&mbed_key, key_buf, MAX_KEY_SIZE);
    if (ret < 0) {
        logger.printfln("mbedtls_pk_write_key_der failed: 0x%04x", static_cast<unsigned>(ret));
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
        logger.printfln("Base64 encode failed: 0x%04x", static_cast<unsigned>(ret));
    } else {
        file_cert.print("-----BEGIN CERTIFICATE-----\n");

        for (size_t i = 0; i < base64_len; i += 64) {
            size_t chunk_size = base64_len - i;
            if (chunk_size > 64) {
                chunk_size = 64;
            }
            file_cert.write(&base64[i], chunk_size);
            file_cert.print("\n");
        }
        file_cert.print("-----END CERTIFICATE-----\n");
    }
    file_cert.close();

    base64_len = 0;
    std::fill_n(base64, sizeof(base64), 0);
    ret = mbedtls_base64_encode(base64, sizeof(base64), &base64_len, key.get(), key_length);
    if (ret != 0) {
        logger.printfln("Base64 encode failed: 0x%04x", static_cast<unsigned>(ret));
    } else {
        file_key.print("-----BEGIN EC PRIVATE KEY-----\n");

        for (size_t i = 0; i < base64_len; i += 64) {
            size_t chunk_size = base64_len - i;
            if (chunk_size > 64) {
                chunk_size = 64;
            }
            file_key.write(&base64[i], chunk_size);
            file_key.print("\n");
        }
        file_key.print("-----END EC PRIVATE KEY-----\n");
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

    if (!generate_cert_and_key(load_info)) { // Will always free mbed_cert
        return false;
    }

    if (!write_cert_and_key(load_info)) {
        return false;
    }

    return true;
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
static int cert_set_san(mbedtls_x509write_cert *mbed_cert)
{
    mbedtls_x509_san_list san_local;
    mbedtls_x509_san_list san_host;
    mbedtls_x509_san_list san_fritz;
    mbedtls_x509_san_list san_ap_ip;

    char hostname_local[] = "warp2-abcd.local"; // TODO Use actual (default) hostname.
    san_local.node.type = MBEDTLS_X509_SAN_DNS_NAME;
    san_local.node.san.unstructured_name.p = reinterpret_cast<unsigned char *>(hostname_local);
    san_local.node.san.unstructured_name.len = strlen(hostname_local);
    san_local.next = nullptr;

    san_host.node.type = MBEDTLS_X509_SAN_DNS_NAME;
    san_host.node.san.unstructured_name.p = reinterpret_cast<unsigned char *>(hostname_local);
    san_host.node.san.unstructured_name.len = strlen(hostname_local) - 6;
    san_host.next = &san_local;

    char hostname_fritz[] = "warp2-abcd.fritz.box";
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
static bool fill_default_cert(mbedtls_x509write_cert *mbed_cert)
{
    mbedtls_x509write_crt_init(mbed_cert);
    mbedtls_x509write_crt_set_md_alg(mbed_cert, MBEDTLS_MD_SHA256);

    // Set serial number, version, issuer name, validity, basic constraints, and subject key identifier
    unsigned char serial[1] = { 0x01 };
    int ret = mbedtls_x509write_crt_set_serial_raw(mbed_cert, serial, sizeof(serial));
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_serial_raw failed: 0x%04x", static_cast<unsigned>(ret));
        return false;
    }

    mbedtls_x509write_crt_set_version(mbed_cert, MBEDTLS_X509_CRT_VERSION_3);

    // Set hostname or IP for CN
    // TODO: Get proper hostname or IP dynamically
    ret = mbedtls_x509write_crt_set_subject_name(mbed_cert, "C=DE,O=Tinkerforge GmbH,CN=warp3-2b6h.local");
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_subject_name failed: 0x%04x", static_cast<unsigned>(ret));
        return false;
    }
    // Issuer == subject for self-signed certificate
    ret = mbedtls_x509write_crt_set_issuer_name(mbed_cert, "C=DE,O=Tinkerforge GmbH,CN=warp3-2b6h.local");
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_issuer_name failed: 0x%04x", static_cast<unsigned>(ret));
        return false;
    }

    ret = mbedtls_x509write_crt_set_validity(mbed_cert, "20250101000000", "21250101000000");
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_validity failed: 0x%04x", static_cast<unsigned>(ret));
        return false;
    }

    // For HTTPS we need CA = FALSE
    ret = mbedtls_x509write_crt_set_basic_constraints(mbed_cert, 0, -1);
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_basic_constraints failed: 0x%04x", static_cast<unsigned>(ret));
        return false;
    }

    // For HTTPS set key usage to digital signature and key encipherment
    ret = mbedtls_x509write_crt_set_key_usage(mbed_cert, MBEDTLS_X509_KU_DIGITAL_SIGNATURE | MBEDTLS_X509_KU_KEY_ENCIPHERMENT);
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_key_usage failed: 0x%04x", static_cast<unsigned>(ret));
        return false;
    }

    ret = cert_set_key_usage(mbed_cert);
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_ext_key_usage failed: 0x%04x", static_cast<unsigned>(ret));
        return false;
    }

    ret = cert_set_san(mbed_cert);
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_subject_alternative_name failed: 0x%04x", static_cast<unsigned>(ret));
        return false;
    }

    return true;
}

[[gnu::noinline]]
static bool key_and_sign_default_cert(mbedtls_x509write_cert *mbed_cert, mbedtls_pk_context *mbed_key, mbedtls_ctr_drbg_context *ctr_drbg)
{
    // Init key
    mbedtls_pk_init(mbed_key);

    int ret = mbedtls_pk_setup(mbed_key, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if (ret != 0) {
        logger.printfln("mbedtls_pk_setup failed: 0x%04x", static_cast<unsigned>(ret));
        return false;
    }

    ret = mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1,
                              mbedtls_pk_ec(*mbed_key),
                              mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        logger.printfln("mbedtls_ecp_gen_key failed: 0x%04x", static_cast<unsigned>(ret));
        return false;
    }

    mbedtls_x509write_crt_set_subject_key(mbed_cert, mbed_key);
    mbedtls_x509write_crt_set_issuer_key(mbed_cert, mbed_key); // self-signed

    return true;
}

bool default_certificate_generator_fn(mbedtls_x509write_cert *mbed_cert, mbedtls_pk_context *mbed_key, mbedtls_ctr_drbg_context *ctr_drbg)
{
    if (!fill_default_cert(mbed_cert)) {
        return false;
    }

    return key_and_sign_default_cert(mbed_cert, mbed_key, ctr_drbg);
}
