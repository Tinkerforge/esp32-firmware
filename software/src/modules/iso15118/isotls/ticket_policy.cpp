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

#include "ticket_policy.h"

#include "mbedtls/md.h"
#include "mbedtls/platform_util.h"

#if defined(MBEDTLS_SSL_SESSION_TICKETS) && defined(MBEDTLS_SSL_TICKET_C) && defined(MBEDTLS_SSL_PROTO_TLS1_3)

ISOTLSTicketPolicy::~ISOTLSTicketPolicy()
{
    clear();
}

int ISOTLSTicketPolicy::setup(int (*rng)(void *, unsigned char *, size_t), void *ctx)
{
    clear();
    return rng(ctx, key, sizeof(key));
}

void ISOTLSTicketPolicy::reset()
{
    origin = next_renewal = 0;
    have_origin = false;
    pending = false;
}

void ISOTLSTicketPolicy::clear()
{
    reset();
    mbedtls_platform_zeroize(key, sizeof(key));
}

bool ISOTLSTicketPolicy::due(uint64_t now) const
{
    return have_origin && (now >= origin) && ((now - origin) < maximum_age_ms) && (next_renewal != 0) && (now >= next_renewal);
}

bool ISOTLSTicketPolicy::expired(uint64_t now) const
{
    return have_origin && ((now < origin) || ((now - origin) >= maximum_age_ms));
}

int ISOTLSTicketPolicy::write(mbedtls_ssl_ticket_context *ctx, const mbedtls_ssl_session *session, unsigned char *start, const unsigned char *end, size_t *len, uint32_t *lifetime, uint64_t now)
{
    *len = 0;

    if (!have_origin) {
        origin = now;
        have_origin = true;
    }

    if (expired(now)) {
        return MBEDTLS_ERR_SSL_SESSION_TICKET_EXPIRED;
    }

    // Eight-byte full-handshake origin + opaque Mbed TLS ticket + HMAC.
    if (end < start || static_cast<size_t>(end - start) < 40) {
        return MBEDTLS_ERR_SSL_BUFFER_TOO_SMALL;
    }

    for (size_t i = 0; i < 8; ++i) {
        start[i] = static_cast<unsigned char>(origin >> (56 - 8 * i));
    }

    size_t ticket_len = 0;
    int ret = mbedtls_ssl_ticket_write(ctx, session, start + 8, end - 32, &ticket_len, lifetime);
    if (ret != 0) {
        return ret;
    }

    ret = mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), key, sizeof(key), start, ticket_len + 8, start + ticket_len + 8);
    if (ret == 0) {
        *len = ticket_len + 40;
        next_renewal = now + renewal_interval_ms;
    }

    return ret;
}

int ISOTLSTicketPolicy::parse(mbedtls_ssl_ticket_context *ctx, mbedtls_ssl_session *session, unsigned char *buf, size_t len, uint64_t now)
{
    if (len < 40) {
        return MBEDTLS_ERR_SSL_INVALID_MAC;
    }

    unsigned char tag[32] = {};
    int ret = mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), key, sizeof(key), buf, len - 32, tag);
    unsigned char difference = 0;
    for (size_t i = 0; i < sizeof(tag); ++i) {
        difference |= tag[i] ^ buf[len - 32 + i];
    }

    mbedtls_platform_zeroize(tag, sizeof(tag));

    if (ret != 0 || difference != 0) {
        return MBEDTLS_ERR_SSL_INVALID_MAC;
    }

    uint64_t saved_origin = 0;
    for (size_t i = 0; i < 8; ++i) {
        saved_origin = (saved_origin << 8) | buf[i];
    }

    if (now < saved_origin || now - saved_origin >= maximum_age_ms) {
        return MBEDTLS_ERR_SSL_SESSION_TICKET_EXPIRED;
    }

    ret = mbedtls_ssl_ticket_parse(ctx, session, buf + 8, len - 40);

    if (ret == 0) {
        origin = saved_origin;
        have_origin = true;
    }

    return ret;
}

#endif
