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

#pragma once

#include <cstdint>
#include <memory> // for std::unique_ptr

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"

typedef bool (*certificate_generator_fn)(mbedtls_x509write_cert *mbed_cert, mbedtls_pk_context *mbed_key, mbedtls_ctr_drbg_context *ctr_drbg);

struct cert_load_info {
    int16_t cert_id;
    int16_t key_id;
    const char *cert_path;
    const char *key_path;
    certificate_generator_fn generator_fn;
};

class Cert
{
public:
    Cert() {};

    bool load_external(const cert_load_info *load_info);
    bool load_internal(const cert_load_info *load_info);
    bool load_external_with_internal_fallback(const cert_load_info *load_info);

    void get_data(const uint8_t **crt_out, size_t *crt_len_out, const uint8_t **key_out, size_t *key_len_out);
    bool is_loaded();
    void free();

    static bool default_cert_fill_fn(mbedtls_x509write_cert *mbed_cert);
    static bool default_key_generator_fn(mbedtls_x509write_cert *mbed_cert, mbedtls_pk_context *mbed_key, mbedtls_ctr_drbg_context *ctr_drbg);
    static bool default_certificate_generator_fn(mbedtls_x509write_cert *mbed_cert, mbedtls_pk_context *mbed_key, mbedtls_ctr_drbg_context *ctr_drbg);

private:
    static bool load_internal_file(const char *path, std::unique_ptr<uint8_t[]> *uniq_buf, uint16_t *length);
    bool load_internal_files(const cert_load_info *load_info);

    bool generate_cert_and_key(const cert_load_info *load_info);
    bool write_cert_and_key(const cert_load_info *load_info);
    bool generate(const cert_load_info *load_info);

    std::unique_ptr<uint8_t[]> crt;
    std::unique_ptr<uint8_t[]> key;

    uint16_t crt_length = 0;
    uint16_t key_length = 0;
};
