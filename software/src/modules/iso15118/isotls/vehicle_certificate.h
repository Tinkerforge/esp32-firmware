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

#pragma once

#include "mbedtls/x509_crt.h"

namespace ISOVehicleCertificate {
    // V2G20-2443 / RFC 8446: reject certificate-policy failures with
    // certificate_unknown, not BADCERT_OTHER's access_denied. Mbed TLS 3.6
    // preserves unknown callback bits and uses its default certificate alert.
    // Recheck bit allocation and alert mapping when upgrading Mbed TLS.
    constexpr uint32_t POLICY_FAILURE = 0x00100000;

    bool ocsp_url(const mbedtls_x509_crt &cert, char *url, size_t capacity);
    uint32_t verify(const mbedtls_x509_crt &cert, bool leaf, bool require_ocsp);
    bool issuer_key_matches(const mbedtls_x509_crt &cert, const mbedtls_x509_crt &issuer);
}
