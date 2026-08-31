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

#include "ed448_bench.h"

#include <cstring>
#include <memory>

#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <mbedtls/ed448.h>
#include <mbedtls/pk.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/x509_csr.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "tf_ed448.h"

namespace {

constexpr uint32_t stack_size = 12288;

const uint8_t private_seed[TF_ED448_PRIVATE_SEED_SIZE] = {
    0x6c, 0x82, 0xa5, 0x62, 0xcb, 0x80, 0x8d, 0x10,
    0xd6, 0x32, 0xbe, 0x89, 0xc8, 0x51, 0x3e, 0xbf,
    0x6c, 0x92, 0x9f, 0x34, 0xdd, 0xfa, 0x8c, 0x9f,
    0x63, 0xc9, 0x96, 0x0e, 0xf6, 0xe3, 0x48, 0xa3,
    0x52, 0x8c, 0x8a, 0x3f, 0xcc, 0x2f, 0x04, 0x4e,
    0x39, 0xa3, 0xfc, 0x5b, 0x94, 0x49, 0x2f, 0x8f,
    0x03, 0x2e, 0x75, 0x49, 0xa2, 0x00, 0x98, 0xf9,
    0x5b,
};

const uint8_t expected_public_key[TF_ED448_PUBLIC_KEY_SIZE] = {
    0x5f, 0xd7, 0x44, 0x9b, 0x59, 0xb4, 0x61, 0xfd,
    0x2c, 0xe7, 0x87, 0xec, 0x61, 0x6a, 0xd4, 0x6a,
    0x1d, 0xa1, 0x34, 0x24, 0x85, 0xa7, 0x0e, 0x1f,
    0x8a, 0x0e, 0xa7, 0x5d, 0x80, 0xe9, 0x67, 0x78,
    0xed, 0xf1, 0x24, 0x76, 0x9b, 0x46, 0xc7, 0x06,
    0x1b, 0xd6, 0x78, 0x3d, 0xf1, 0xe5, 0x0f, 0x6c,
    0xd1, 0xfa, 0x1a, 0xbe, 0xaf, 0xe8, 0x25, 0x61,
    0x80,
};

const uint8_t expected_signature[TF_ED448_SIGNATURE_SIZE] = {
    0x53, 0x3a, 0x37, 0xf6, 0xbb, 0xe4, 0x57, 0x25,
    0x1f, 0x02, 0x3c, 0x0d, 0x88, 0xf9, 0x76, 0xae,
    0x2d, 0xfb, 0x50, 0x4a, 0x84, 0x3e, 0x34, 0xd2,
    0x07, 0x4f, 0xd8, 0x23, 0xd4, 0x1a, 0x59, 0x1f,
    0x2b, 0x23, 0x3f, 0x03, 0x4f, 0x62, 0x82, 0x81,
    0xf2, 0xfd, 0x7a, 0x22, 0xdd, 0xd4, 0x7d, 0x78,
    0x28, 0xc5, 0x9b, 0xd0, 0xa2, 0x1b, 0xfd, 0x39,
    0x80, 0xff, 0x0d, 0x20, 0x28, 0xd4, 0xb1, 0x8a,
    0x9d, 0xf6, 0x3e, 0x00, 0x6c, 0x5d, 0x1c, 0x2d,
    0x34, 0x5b, 0x92, 0x5d, 0x8d, 0xc0, 0x0b, 0x41,
    0x04, 0x85, 0x2d, 0xb9, 0x9a, 0xc5, 0xc7, 0xcd,
    0xda, 0x85, 0x30, 0xa1, 0x13, 0xa0, 0xf4, 0xdb,
    0xb6, 0x11, 0x49, 0xf0, 0x5a, 0x73, 0x63, 0x26,
    0x8c, 0x71, 0xd9, 0x58, 0x08, 0xff, 0x2e, 0x65,
    0x26, 0x00,
};

const uint8_t expected_shake256[64] = {
    0x46, 0xb9, 0xdd, 0x2b, 0x0b, 0xa8, 0x8d, 0x13,
    0x23, 0x3b, 0x3f, 0xeb, 0x74, 0x3e, 0xeb, 0x24,
    0x3f, 0xcd, 0x52, 0xea, 0x62, 0xb8, 0x1b, 0x82,
    0xb5, 0x0c, 0x27, 0x64, 0x6e, 0xd5, 0x76, 0x2f,
    0xd7, 0x5d, 0xc4, 0xdd, 0xd8, 0xc0, 0xf2, 0x00,
    0xcb, 0x05, 0x01, 0x9d, 0x67, 0xb5, 0x92, 0xf6,
    0xfc, 0x82, 0x1c, 0x49, 0x47, 0x9a, 0xb4, 0x86,
    0x40, 0x29, 0x2e, 0xac, 0xb3, 0xb7, 0xc4, 0xbe,
};

const uint8_t ed448_order[TF_ED448_PRIVATE_SEED_SIZE] = {
    0xf3, 0x44, 0x58, 0xab, 0x92, 0xc2, 0x78, 0x23,
    0x55, 0x8f, 0xc5, 0x8d, 0x72, 0xc2, 0x6c, 0x21,
    0x90, 0x36, 0xd6, 0xae, 0x49, 0xdb, 0x4e, 0xc4,
    0xe9, 0x23, 0xca, 0x7c, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f,
    0x00,
};

struct TaskContext {
    Ed448Bench *bench;
    uint32_t iterations;
};

struct Measurements {
    uint32_t iterations;
    uint64_t derive_us;
    uint64_t sign_us;
    uint64_t verify_us;
    uint64_t shake_us;
    uint32_t stack_hwm;
    int32_t error;
    int32_t mbedtls_error;
    bool kat_passed;
    bool mbedtls_passed;
};

int test_rng(void *, unsigned char *output, size_t len)
{
    memset(output, 0x5a, len);
    return 0;
}

size_t count_bytes(const uint8_t *data, size_t data_len, const uint8_t *needle, size_t needle_len)
{
    size_t count = 0;
    for (size_t i = 0; i + needle_len <= data_len; ++i) {
        count += memcmp(data + i, needle, needle_len) == 0;
    }
    return count;
}

bool run_mbedtls_kat(int32_t *error)
{
    constexpr size_t der_size = 4096;
    static const uint8_t message[] = "Mbed TLS Ed448 target KAT";
    uint8_t leaf_seed[sizeof(private_seed)];
    uint8_t signature[MBEDTLS_ED448_SIGNATURE_SIZE];
    uint8_t serial = 1;
    size_t signature_len = 0;
    uint32_t verify_flags = 0;
    bool ok = true;

    std::unique_ptr<uint8_t[]> der{new (std::nothrow) uint8_t[der_size]};
    if (!der) {
        *error = MBEDTLS_ERR_X509_ALLOC_FAILED;
        return false;
    }

    mbedtls_pk_context root_key;
    mbedtls_pk_context leaf_key;
    mbedtls_pk_context parsed_key;
    mbedtls_pk_context parsed_public;
    mbedtls_x509write_csr csr_writer;
    mbedtls_x509write_cert cert_writer;
    mbedtls_x509_crt root_cert;
    mbedtls_x509_crt leaf_cert;
    mbedtls_pk_init(&root_key);
    mbedtls_pk_init(&leaf_key);
    mbedtls_pk_init(&parsed_key);
    mbedtls_pk_init(&parsed_public);
    mbedtls_x509write_csr_init(&csr_writer);
    mbedtls_x509write_crt_init(&cert_writer);
    mbedtls_x509_crt_init(&root_cert);
    mbedtls_x509_crt_init(&leaf_cert);

    auto check = [&](int rc) {
        if (rc != 0) {
            if (*error == 0) {
                *error = rc;
            }
            ok = false;
        }
    };

    const mbedtls_pk_info_t *ed448_info = mbedtls_pk_info_from_type(MBEDTLS_PK_ED448);
    check(ed448_info == nullptr ? MBEDTLS_ERR_PK_UNKNOWN_PK_ALG : mbedtls_pk_setup(&root_key, ed448_info));
    if (ok) {
        check(mbedtls_ed448_import_private(mbedtls_pk_ed448(root_key), private_seed, sizeof(private_seed)));
    }

    int der_len = 0;
    if (ok) {
        der_len = mbedtls_pk_write_key_der(&root_key, der.get(), der_size);
        check(der_len > 0 ? 0 : der_len);
    }
    if (ok) {
        check(mbedtls_pk_parse_key(&parsed_key, der.get() + der_size - der_len, static_cast<size_t>(der_len), nullptr, 0, test_rng, nullptr));
        check(mbedtls_pk_check_pair(&root_key, &parsed_key, test_rng, nullptr));
    }

    if (ok) {
        der_len = mbedtls_pk_write_pubkey_der(&root_key, der.get(), der_size);
        check((der_len > 0) ? 0 : der_len);
    }
    if (ok) {
        check(mbedtls_pk_parse_public_key(&parsed_public, der.get() + der_size - der_len, static_cast<size_t>(der_len)));
        check(mbedtls_pk_sign(&parsed_key, MBEDTLS_MD_NONE, message, sizeof(message) - 1, signature, sizeof(signature), &signature_len, test_rng, nullptr));
        check((signature_len == sizeof(signature)) ? 0 : MBEDTLS_ERR_PK_BAD_INPUT_DATA);
        check(mbedtls_pk_verify(&parsed_public, MBEDTLS_MD_NONE, message, sizeof(message) - 1, signature, signature_len));
    }

    if (ok) {
        mbedtls_x509write_csr_set_key(&csr_writer, &root_key);
        mbedtls_x509write_csr_set_md_alg(&csr_writer, MBEDTLS_MD_NONE);
        check(mbedtls_x509write_csr_set_subject_name(&csr_writer, "C=DE,O=Tinkerforge,CN=Ed448 Target KAT"));
    }
    if (ok) {
        der_len = mbedtls_x509write_csr_der(&csr_writer, der.get(), der_size, test_rng, nullptr);
        check(der_len > 0 ? 0 : der_len);
    }
    if (ok) {
        static const uint8_t ed448_algorithm[] = {0x30, 0x05, 0x06, 0x03,
                                                  0x2b, 0x65, 0x71};
        static const uint8_t ed448_algorithm_with_null[] = {
            0x30, 0x07, 0x06, 0x03, 0x2b, 0x65, 0x71, 0x05, 0x00,
        };
        const uint8_t *csr_der = der.get() + der_size - der_len;
        const size_t csr_der_len = static_cast<size_t>(der_len);
        check(count_bytes(csr_der, csr_der_len, ed448_algorithm, sizeof(ed448_algorithm)) == 2
                  ? 0
                  : MBEDTLS_ERR_X509_INVALID_FORMAT);
        check(count_bytes(csr_der, csr_der_len, ed448_algorithm_with_null, sizeof(ed448_algorithm_with_null)) == 0
                  ? 0
                  : MBEDTLS_ERR_X509_INVALID_FORMAT);
    }

    if (ok) {
        mbedtls_x509write_crt_set_subject_key(&cert_writer, &root_key);
        mbedtls_x509write_crt_set_issuer_key(&cert_writer, &root_key);
        mbedtls_x509write_crt_set_md_alg(&cert_writer, MBEDTLS_MD_NONE);
        check(mbedtls_x509write_crt_set_subject_name(&cert_writer, "CN=Ed448 Target Root"));
        check(mbedtls_x509write_crt_set_issuer_name(&cert_writer, "CN=Ed448 Target Root"));
        check(mbedtls_x509write_crt_set_serial_raw(&cert_writer, &serial, 1));
        check(mbedtls_x509write_crt_set_validity(&cert_writer, "20260101000000", "20360101000000"));
        check(mbedtls_x509write_crt_set_basic_constraints(&cert_writer, 1, 0));
        check(mbedtls_x509write_crt_set_key_usage(&cert_writer, MBEDTLS_X509_KU_KEY_CERT_SIGN | MBEDTLS_X509_KU_CRL_SIGN));
    }
    if (ok) {
        der_len = mbedtls_x509write_crt_der(&cert_writer, der.get(), der_size, test_rng, nullptr);
        check(der_len > 0 ? 0 : der_len);
    }
    if (ok) {
        check(mbedtls_x509_crt_parse_der(&root_cert, der.get() + der_size - der_len, static_cast<size_t>(der_len)));
    }

    memcpy(leaf_seed, private_seed, sizeof(leaf_seed));
    leaf_seed[0] ^= 0x80;
    if (ok) {
        check(mbedtls_pk_setup(&leaf_key, ed448_info));
        check(mbedtls_ed448_import_private(mbedtls_pk_ed448(leaf_key), leaf_seed, sizeof(leaf_seed)));
    }
    mbedtls_x509write_crt_free(&cert_writer);
    mbedtls_x509write_crt_init(&cert_writer);
    serial = 2;
    if (ok) {
        mbedtls_x509write_crt_set_subject_key(&cert_writer, &leaf_key);
        mbedtls_x509write_crt_set_issuer_key(&cert_writer, &root_key);
        mbedtls_x509write_crt_set_md_alg(&cert_writer, MBEDTLS_MD_NONE);
        check(mbedtls_x509write_crt_set_subject_name(&cert_writer, "CN=Ed448 Target Leaf"));
        check(mbedtls_x509write_crt_set_issuer_name(&cert_writer, "CN=Ed448 Target Root"));
        check(mbedtls_x509write_crt_set_serial_raw(&cert_writer, &serial, 1));
        check(mbedtls_x509write_crt_set_validity(&cert_writer, "20260101000000", "20300101000000"));
        check(mbedtls_x509write_crt_set_basic_constraints(&cert_writer, 0, -1));
        check(mbedtls_x509write_crt_set_key_usage(&cert_writer, MBEDTLS_X509_KU_DIGITAL_SIGNATURE));
    }
    if (ok) {
        der_len = mbedtls_x509write_crt_der(&cert_writer, der.get(), der_size, test_rng, nullptr);
        check((der_len > 0) ? 0 : der_len);
    }
    if (ok) {
        check(mbedtls_x509_crt_parse_der(&leaf_cert, der.get() + der_size - der_len, static_cast<size_t>(der_len)));
        check(mbedtls_pk_check_pair(&leaf_cert.pk, &leaf_key, test_rng, nullptr));
        check(mbedtls_x509_crt_verify(&leaf_cert, &root_cert, nullptr, nullptr, &verify_flags, nullptr, nullptr));
        check((verify_flags == 0) ? 0 : MBEDTLS_ERR_X509_CERT_VERIFY_FAILED);
    }

    memset(leaf_seed, 0, sizeof(leaf_seed));
    memset(der.get(), 0, der_size);
    mbedtls_x509_crt_free(&leaf_cert);
    mbedtls_x509_crt_free(&root_cert);
    mbedtls_x509write_crt_free(&cert_writer);
    mbedtls_x509write_csr_free(&csr_writer);
    mbedtls_pk_free(&parsed_public);
    mbedtls_pk_free(&parsed_key);
    mbedtls_pk_free(&leaf_key);
    mbedtls_pk_free(&root_key);
    return ok;
}

} // namespace

void Ed448Bench::pre_setup()
{
    command = Config::Object({
        {"iterations", Config::Uint(3, 1, 100)},
    });
    result = Config::Object({
        {"status", Config::Uint(0, 0, 3)},
        {"iterations", Config::Uint32(0)},
        {"kat_passed", Config::Bool(false)},
        {"error", Config::Int32(0)},
        {"mbedtls_passed", Config::Bool(false)},
        {"mbedtls_error", Config::Int32(0)},
        {"derive_us", Config::Uint53(0)},
        {"sign_us", Config::Uint53(0)},
        {"verify_us", Config::Uint53(0)},
        {"shake_us", Config::Uint53(0)},
        {"stack_size", Config::Uint32(stack_size)},
        {"stack_hwm", Config::Uint32(0)},
        {"stack_used", Config::Uint32(0)},
    });
}

void Ed448Bench::setup()
{
    initialized = true;
}

void Ed448Bench::register_urls()
{
    api.addState("ed448_bench/result", &result);
    api.addCommand("ed448_bench/run", &command, {}, [this](Language, String &errmsg) {
        if (running) {
            errmsg = "Ed448 benchmark is already running";
            return;
        }
        run(command.get("iterations")->asUint());
    }, true);
}

void Ed448Bench::run(uint32_t iterations)
{
    auto *context = new TaskContext{this, iterations};
    running = true;
    result.get("status")->updateUint(1);

    const BaseType_t rc = xTaskCreatePinnedToCore(task, "ed448_bench", stack_size, context, 10, nullptr, 0);
    if (rc != pdPASS) {
        delete context;
        running = false;
        result.get("status")->updateUint(3);
        result.get("error")->updateInt(rc);
    }
}

void Ed448Bench::task(void *opaque)
{
    auto *context = static_cast<TaskContext *>(opaque);
    Ed448Bench *bench = context->bench;
    Measurements measurements{};
    measurements.iterations = context->iterations;
    delete context;

    uint8_t public_key[TF_ED448_PUBLIC_KEY_SIZE];
    uint8_t signature[TF_ED448_SIGNATURE_SIZE];
    uint8_t shake[sizeof(expected_shake256)];
    static const uint8_t message[] = "ISO 15118-20 Ed448 target benchmark";

    int rc = tf_shake256(nullptr, 0, shake, sizeof(shake));
    rc = rc == TF_ED448_SUCCESS ? tf_ed448_derive_public(private_seed, sizeof(private_seed), public_key, sizeof(public_key)) : rc;
    rc = rc == TF_ED448_SUCCESS ? tf_ed448_sign(private_seed, sizeof(private_seed), public_key, sizeof(public_key), nullptr, 0, signature, sizeof(signature)) : rc;
    rc = rc == TF_ED448_SUCCESS ? tf_ed448_verify(public_key, sizeof(public_key), nullptr, 0, signature, sizeof(signature)) : rc;
    measurements.kat_passed = (rc == TF_ED448_SUCCESS)                                           &&
                              (memcmp(shake, expected_shake256, sizeof(shake)) == 0)             &&
                              (memcmp(public_key, expected_public_key, sizeof(public_key)) == 0) &&
                              (memcmp(signature, expected_signature, sizeof(signature)) == 0);

    uint8_t invalid[TF_ED448_SIGNATURE_SIZE];
    uint8_t small_order[4][TF_ED448_PUBLIC_KEY_SIZE] = {{1}, {0}, {0}, {0}};
    small_order[1][0] = 0xfe;
    memset(small_order[1] + 1, 0xff, 27);
    small_order[1][28] = 0xfe;
    memset(small_order[1] + 29, 0xff, 27);
    small_order[2][TF_ED448_PUBLIC_KEY_SIZE - 1] = 0x80;
    for (size_t i = 0; i < 4 && measurements.kat_passed; ++i) {
        measurements.kat_passed = tf_ed448_verify(small_order[i], sizeof(small_order[i]), nullptr, 0, expected_signature, sizeof(expected_signature)) == TF_ED448_INVALID_PUBLIC_KEY;
        memcpy(invalid, expected_signature, sizeof(invalid));
        memcpy(invalid, small_order[i], sizeof(small_order[i]));
        measurements.kat_passed = measurements.kat_passed && tf_ed448_verify(expected_public_key, sizeof(expected_public_key), nullptr, 0, invalid, sizeof(invalid)) == TF_ED448_INVALID_SIGNATURE;
    }

    memcpy(invalid, expected_signature, sizeof(invalid));
    memcpy(invalid + TF_ED448_PUBLIC_KEY_SIZE, ed448_order, sizeof(ed448_order));
    measurements.kat_passed = measurements.kat_passed && tf_ed448_verify(expected_public_key, sizeof(expected_public_key), nullptr, 0, invalid, sizeof(invalid)) == TF_ED448_INVALID_SIGNATURE;
    invalid[TF_ED448_PUBLIC_KEY_SIZE]++;
    measurements.kat_passed = measurements.kat_passed && tf_ed448_verify(expected_public_key, sizeof(expected_public_key), nullptr, 0, invalid, sizeof(invalid)) == TF_ED448_INVALID_SIGNATURE;

    memcpy(invalid, expected_signature, sizeof(invalid));
    for (size_t i = 0, carry = 1; i < sizeof(ed448_order); ++i) {
        size_t index = TF_ED448_PUBLIC_KEY_SIZE + i;
        unsigned sum = invalid[index] + ed448_order[i] + carry;
        invalid[index] = static_cast<uint8_t>(sum);
        carry = sum >> 8;
    }
    measurements.kat_passed = measurements.kat_passed && tf_ed448_verify(expected_public_key, sizeof(expected_public_key), nullptr, 0, invalid, sizeof(invalid)) == TF_ED448_INVALID_SIGNATURE;

    memcpy(invalid, expected_signature, sizeof(invalid));
    invalid[0] ^= 1;
    measurements.kat_passed = measurements.kat_passed && tf_ed448_verify(expected_public_key, sizeof(expected_public_key), nullptr, 0, invalid, sizeof(invalid)) == TF_ED448_INVALID_SIGNATURE;
    measurements.error = rc;
    measurements.mbedtls_passed = measurements.kat_passed && run_mbedtls_kat(&measurements.mbedtls_error);

    if (measurements.kat_passed) {
        for (uint32_t i = 0; i < measurements.iterations; ++i) {
            int64_t start = esp_timer_get_time();
            rc = tf_ed448_derive_public(private_seed, sizeof(private_seed), public_key, sizeof(public_key));
            measurements.derive_us += static_cast<uint64_t>(esp_timer_get_time() - start);
            vTaskDelay(1);
        }

        for (uint32_t i = 0; i < measurements.iterations; ++i) {
            int64_t start = esp_timer_get_time();
            rc = rc == TF_ED448_SUCCESS ? tf_ed448_sign(private_seed, sizeof(private_seed), public_key, sizeof(public_key), message, sizeof(message) - 1, signature, sizeof(signature)) : rc;
            measurements.sign_us += static_cast<uint64_t>(esp_timer_get_time() - start);
            vTaskDelay(1);
        }

        for (uint32_t i = 0; i < measurements.iterations; ++i) {
            int64_t start = esp_timer_get_time();
            rc = rc == TF_ED448_SUCCESS  ? tf_ed448_verify(public_key, sizeof(public_key), message, sizeof(message) - 1, signature, sizeof(signature)) : rc;
            measurements.verify_us += static_cast<uint64_t>(esp_timer_get_time() - start);
            vTaskDelay(1);
        }

        for (uint32_t i = 0; i < measurements.iterations; ++i) {
            int64_t start = esp_timer_get_time();
            rc = rc == TF_ED448_SUCCESS ? tf_shake256(message, sizeof(message) - 1, shake, sizeof(shake)) : rc;
            measurements.shake_us += static_cast<uint64_t>(esp_timer_get_time() - start);
            vTaskDelay(1);
        }
        measurements.error = rc;
    }

    measurements.stack_hwm = uxTaskGetStackHighWaterMark(nullptr);
    task_scheduler.scheduleOnce([bench, measurements]() {
        bench->result.get("iterations")->updateUint(measurements.iterations);
        bench->result.get("kat_passed")->updateBool(measurements.kat_passed);
        bench->result.get("error")->updateInt(measurements.error);
        bench->result.get("mbedtls_passed")->updateBool(measurements.mbedtls_passed);
        bench->result.get("mbedtls_error")->updateInt(measurements.mbedtls_error);
        bench->result.get("derive_us")->updateUint53(measurements.derive_us);
        bench->result.get("sign_us")->updateUint53(measurements.sign_us);
        bench->result.get("verify_us")->updateUint53(measurements.verify_us);
        bench->result.get("shake_us")->updateUint53(measurements.shake_us);
        bench->result.get("stack_hwm")->updateUint(measurements.stack_hwm);
        bench->result.get("stack_used")->updateUint(stack_size - measurements.stack_hwm);
        bench->result.get("status")->updateUint((measurements.kat_passed && measurements.mbedtls_passed && (measurements.error == TF_ED448_SUCCESS)) ? 2 : 3);
        bench->running = false;
    });
    vTaskDelete(nullptr);
}
