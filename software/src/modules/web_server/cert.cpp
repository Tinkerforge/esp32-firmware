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

#include <esp_https_server.h>
#include <mbedtls/base64.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/oid.h>
#include <mbedtls/pem.h>
#include <LittleFS.h>

#include "build.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"

#define KEY_FILE  "/web_server/key"
#define CERT_FILE "/web_server/cert"


void Cert::log()
{
    logger.printfln("Cert:");
    logger.print_plain((char*)crt, crt_length);
    logger.printfln("Key:");
    logger.print_plain((char*)key, key_length);
}

bool Cert::read()
{
    if (!LittleFS.exists(KEY_FILE) || !LittleFS.exists(CERT_FILE)) {
        logger.printfln("Cert or key file does not exist, generating new self signed certificates");
        // Generate will create new self-signed certs and also fill the cert and key buffers
        if (!generate()) {
            return false;
        }
    }

    File file_key = LittleFS.open(KEY_FILE, "r");
    if (!file_key) {
        logger.printfln("Failed to open key file");
        return false;
    }

    File file_cert = LittleFS.open(CERT_FILE, "r");
    if (!file_cert) {
        logger.printfln("Failed to open cert file");
        return false;
    }

    key_length = file_key.readBytes(reinterpret_cast<char*>(key), sizeof(key));
    key[key_length] = 0;
    logger.printfln("Key (PEM) with length %d read (first byte: %x)", key_length, key[0]);
    file_key.close();

    crt_length = file_cert.readBytes(reinterpret_cast<char*>(crt), sizeof(crt));
    crt[crt_length] = 0;
    logger.printfln("Cert (PEM) with length %d read (first byte: %x)", crt_length, crt[0]);
    file_cert.close();

    return true;
}

bool Cert::generate()
{
    if (LittleFS.exists(KEY_FILE)) {
        LittleFS.remove(KEY_FILE);
    }

    if (LittleFS.exists(CERT_FILE)) {
        LittleFS.remove(CERT_FILE);
    }

    // Temporarily use key and crt for DER format, will be converted to PEM later
    std::fill_n(key, sizeof(key), 0);
    std::fill_n(crt, sizeof(crt), 0);
    key_length = 0;
    crt_length = 0;

    // Initialize entropy
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    const char *pers = "web_cert";
    int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, reinterpret_cast<const unsigned char *>(pers), strlen(pers));
    if (ret != 0) {
        logger.printfln("mbedtls_ctr_drbg_seed failed: 0x%04x", ret);
        return false;
    }

    // Init key and cert writer
    mbedtls_pk_context mbed_key;
    mbedtls_pk_init(&mbed_key);

    ret = mbedtls_pk_setup(&mbed_key, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if (ret != 0) {
        logger.printfln("mbedtls_pk_setup failed: 0x%04x", ret);
        return false;
    }

    ret = mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1,
                              mbedtls_pk_ec(mbed_key),
                              mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        logger.printfln("mbedtls_ecp_gen_key failed: 0x%04x", ret);
        return false;
    }

    // Create self-signed certificate
    mbedtls_x509write_cert mbed_cert;
    mbedtls_x509write_crt_init(&mbed_cert);
    mbedtls_x509write_crt_set_md_alg(&mbed_cert, MBEDTLS_MD_SHA256);
    mbedtls_x509write_crt_set_subject_key(&mbed_cert, &mbed_key);
    mbedtls_x509write_crt_set_issuer_key(&mbed_cert, &mbed_key); // self-signed

    // Set serial number, version, issuer name, validity, basic constraints, and subject key identifier
    unsigned char serial[1] = { 0x01 };
    ret = mbedtls_x509write_crt_set_serial_raw(&mbed_cert, serial, sizeof(serial));
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_serial_raw failed: 0x%04x", ret);
        return false;
    }

    mbedtls_x509write_crt_set_version(&mbed_cert, 2);

    // CHANGE: Use your HTTPS hostname or IP for CN!
    ret = mbedtls_x509write_crt_set_subject_name(&mbed_cert, "C=DE,O=Tinkerforge GmbH,CN=warp3-2b6h.local");
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_subject_name failed: 0x%04x", ret);
        return false;
    }
    ret = mbedtls_x509write_crt_set_issuer_name(&mbed_cert, "C=DE,O=Tinkerforge GmbH,CN=warp3-2b6h.local");
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_issuer_name failed: 0x%04x", ret);
        return false;
    }

    ret = mbedtls_x509write_crt_set_validity(&mbed_cert, "20240803120000", "21230803120000");
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_validity failed: 0x%04x", ret);
        return false;
    }

    // CHANGE: For HTTPS, CA = FALSE
    ret = mbedtls_x509write_crt_set_basic_constraints(&mbed_cert, 0, -1);
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_basic_constraints failed: 0x%04x", ret);
        return false;
    }

    // CHANGE: For HTTPS, set key usage to digital signature and key encipherment
    ret = mbedtls_x509write_crt_set_key_usage(&mbed_cert, MBEDTLS_X509_KU_DIGITAL_SIGNATURE | MBEDTLS_X509_KU_KEY_ENCIPHERMENT);
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_key_usage failed: 0x%04x", ret);
        return false;
    }

    // CHANGE: For HTTPS, set extended key usage to serverAuth
    const unsigned char ext_key_usage_der[] = {0x30, 0x0a, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x01};
    ret = mbedtls_x509write_crt_set_extension(
        &mbed_cert,
        MBEDTLS_OID_EXTENDED_KEY_USAGE,
        MBEDTLS_OID_SIZE(MBEDTLS_OID_EXTENDED_KEY_USAGE),
        0,
        ext_key_usage_der, sizeof(ext_key_usage_der)
    );
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_extension failed: 0x%04x", ret);
        return false;
    }

#if 0
    // OPTIONAL: Add Subject Alternative Name (SAN) for browsers
    // Example for DNS SAN "mydevice.local"
    const unsigned char san_der[] = {0x30, 0x11, 0x82, 0x0f, 'm','y','d','e','v','i','c','e','.','l','o','c','a','l'};
    ret = mbedtls_x509write_crt_set_extension(
        &mbed_cert,
        MBEDTLS_OID_SUBJECT_ALT_NAME,
        MBEDTLS_OID_SIZE(MBEDTLS_OID_SUBJECT_ALT_NAME),
        0,
        san_der, sizeof(san_der)
    );
#endif

    ret = mbedtls_x509write_crt_set_subject_key_identifier(&mbed_cert);
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_subject_key_identifier failed: 0x%04x", ret);
        return false;
    }

    crt_length = mbedtls_x509write_crt_der(&mbed_cert, crt, sizeof(crt), mbedtls_ctr_drbg_random, &ctr_drbg);
    memmove(crt, crt + sizeof(crt) - crt_length, crt_length);
    logger.printfln("Cert (DER) with length %d generated (first byte: %x)", crt_length, crt[0]);

    key_length = mbedtls_pk_write_key_der(&mbed_key, key, sizeof(key));
    memmove(key, key + sizeof(key) - key_length, key_length);
    logger.printfln("Key (DER) with length %d generated (first byte: %x)", key_length, key[0]);

    mbedtls_pk_free(&mbed_key);
    mbedtls_x509write_crt_free(&mbed_cert);


    // Save generated cert and key in DER format as PEM files
    unsigned char base64[768];
    size_t base64_len = 0;

    std::fill_n(base64, sizeof(base64), 0);
    ret = mbedtls_base64_encode(base64, sizeof(base64), &base64_len, crt, crt_length);

    File file_cert = LittleFS.open(CERT_FILE, "w", true);
    if (!file_cert) {
        logger.printfln("Failed to open cert file for writing");
        return false;
    }

    File file_key = LittleFS.open(KEY_FILE, "w", true);
    if (!file_key) {
        logger.printfln("Failed to open key file for writing");
        return false;
    }

    if (ret != 0) {
        logger.printfln("Base64 encode failed: 0x%04x", ret);
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
    ret = mbedtls_base64_encode(base64, sizeof(base64), &base64_len, key, key_length);
    if (ret != 0) {
        logger.printfln("Base64 encode failed: 0x%04x", ret);
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