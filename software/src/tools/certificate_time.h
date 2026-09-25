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

#include <ctime>
#include <mbedtls/x509_crt.h>

// Explicit, connection-scoped validity checks. Do not use Mbed TLS's
// time_is_past/future helpers: They are no-ops without HAVE_TIME_DATE.
// Certificate validity endpoints are inclusive (RFC 5280 section 4.1.2.5).
inline uint32_t certificate_time_flags(const mbedtls_x509_crt &cert, time_t now)
{
    struct tm utc;
    if ((now <= 0) || (gmtime_r(&now, &utc) == nullptr) || (utc.tm_year < 120)) {
        return MBEDTLS_X509_BADCERT_OTHER; // No usable wall clock (e.g. boot epoch).
    }

    const int current[] = {utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday, utc.tm_hour, utc.tm_min, utc.tm_sec};
    const auto compare = [&current](const mbedtls_x509_time &date) {
        const int fields[] = {date.year, date.mon, date.day, date.hour, date.min, date.sec};
        for (size_t i = 0; i < 6; ++i) {
            if (fields[i] != current[i]) {
                return fields[i] < current[i] ? -1 : 1;
            }
        }
        return 0;
    };

    uint32_t flags = 0;
    if (compare(cert.valid_from) > 0) {
        flags |= MBEDTLS_X509_BADCERT_FUTURE;
    }

    if (compare(cert.valid_to) < 0) {
        flags |= MBEDTLS_X509_BADCERT_EXPIRED;
    }

    return flags;
}

inline int certificate_time_verify(void *, mbedtls_x509_crt *cert, int, uint32_t *flags)
{
    *flags |= certificate_time_flags(*cert, time(nullptr));
    return 0;
}
