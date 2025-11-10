/* esp32-firmware
 * Copyright (C) 2025 Mattias Schäffersmann <mattias@tinkerforge.com>
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

 #include "cert_generator.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"

[[gnu::noinline]]
static bool fill_ship_cert(mbedtls_x509write_cert *mbed_cert)
{
    mbedtls_x509write_crt_set_md_alg(mbed_cert, MBEDTLS_MD_SHA256);

    // Set serial number, version, issuer name, validity, basic constraints, and subject key identifier
    unsigned char serial[1] = {0x01}; // Any non-zero value
    int ret = mbedtls_x509write_crt_set_serial_raw(mbed_cert, serial, sizeof(serial));
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_serial_raw failed: -0x%04x", static_cast<unsigned>(-ret));
        return false;
    }

    mbedtls_x509write_crt_set_version(mbed_cert, MBEDTLS_X509_CRT_VERSION_3);

    ret = mbedtls_x509write_crt_set_subject_name(mbed_cert, "C=DE,O=Tinkerforge GmBH,CN=WARP_EEBUS");
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_subject_name failed: -0x%04x", static_cast<unsigned>(-ret));
        return false;
    }
    ret = mbedtls_x509write_crt_set_issuer_name(mbed_cert, "C=DE,O=Tinkerforge GmBH,CN=WARP_EEBUS");
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_issuer_name failed: -0x%04x", static_cast<unsigned>(-ret));
        return false;
    }
    ret = mbedtls_x509write_crt_set_validity(mbed_cert, "20250101000000", "21250101000000");
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_validity failed: -0x%04x", static_cast<unsigned>(-ret));
        return false;
    }
    ret = mbedtls_x509write_crt_set_basic_constraints(mbed_cert, 1, -1);
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_basic_constraints failed: -0x%04x", static_cast<unsigned>(-ret));
        return false;
    }
    ret = mbedtls_x509write_crt_set_subject_key_identifier(mbed_cert);
    if (ret != 0) {
        logger.printfln("mbedtls_x509write_crt_set_subject_key_identifier failed: -0x%04x", static_cast<unsigned>(-ret));
        return false;
    }

    return true;
}

bool eebus_ship_certificate_generator_fn(mbedtls_x509write_cert *mbed_cert, mbedtls_pk_context *mbed_key, mbedtls_ctr_drbg_context *ctr_drbg)
{
    if (!Cert::default_key_generator_fn(mbed_cert, mbed_key, ctr_drbg)) {
        return false;
    }

    return fill_ship_cert(mbed_cert);
}
