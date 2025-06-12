/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include <functional>
#include <lwip/dns.h>

// Helper function to execute dns_gethostbyname in lwIP's TCPIP context.
[[gnu::nonnull(1, 2, 3)]]
err_t dns_gethostbyname_lwip_ctx(const char *host, ip_addr_t *addr, dns_found_callback found, void *callback_arg);

// Helper function to execute dns_gethostbyname_addrtype in lwIP's TCPIP context.
[[gnu::nonnull(1, 2, 3)]]
err_t dns_gethostbyname_addrtype_lwip_ctx(const char *host, ip_addr_t *addr, dns_found_callback found, void *callback_arg, u8_t dns_addrtype);

struct dns_gethostbyname_addrtype_lwip_ctx_async_data
{
    err_t err; // output
    ip_addr_t addr; // internal
    ip_addr_t *addr_ptr; // output
    std::function<void(dns_gethostbyname_addrtype_lwip_ctx_async_data *callback_arg)> found_callback; // input
    void *user; // input/output
};

[[gnu::nonnull]]
void dns_gethostbyname_addrtype_lwip_ctx_async(const char *host,
                                               std::function<void(dns_gethostbyname_addrtype_lwip_ctx_async_data *callback_arg)> &&found_callback,
                                               u8_t dns_addrtype);
