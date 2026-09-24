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

#include <stdint.h>
#include "mbedtls/ssl_ticket.h"

class ISOTLSTicketPolicy {
public:
    static constexpr uint32_t lifetime_s = 3600;
    // Leave one extra second for task scheduling before the required 20 s lead.
    static constexpr uint64_t renewal_interval_ms = (lifetime_s - 21) * 1000ULL;
    static constexpr uint64_t maximum_age_ms = 604800000ULL;

    ~ISOTLSTicketPolicy();

    int setup(int (*rng)(void *, unsigned char *, size_t), void *ctx);
    void reset();
    void clear();
    bool due(uint64_t now) const;
    bool expired(uint64_t now) const;

    int write(mbedtls_ssl_ticket_context *ctx, const mbedtls_ssl_session *session,
              unsigned char *start, const unsigned char *end, size_t *len,
              uint32_t *lifetime, uint64_t now);
    int parse(mbedtls_ssl_ticket_context *ctx, mbedtls_ssl_session *session,
              unsigned char *buf, size_t len, uint64_t now);

    bool pending = false;

private:
    unsigned char key[32] = {};
    uint64_t origin = 0;
    uint64_t next_renewal = 0;
    bool have_origin = false;
};
