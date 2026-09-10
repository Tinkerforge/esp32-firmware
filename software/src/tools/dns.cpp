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

#include <esp_netif.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/tcpip.h>

#include "dns.h"
#include "event_log_prefix.h"
#include "main_dependencies.h"
#include "tools/net.h"

#include "gcc_warnings.h"

// Note: LWIP_DNS_ADDRTYPE != IPADDR_TYPE
static int ipaddr_aton_addrtype(const char *host, ip_addr_t *addr, u8_t dns_addrtype)
{
    if (dns_addrtype == LWIP_DNS_ADDRTYPE_IPV4) {
        const int success = ip4addr_aton(host, &addr->u_addr.ip4);
        if (success) {
            addr->type = IPADDR_TYPE_V4;
        }
        return success;
    }
    if (dns_addrtype == LWIP_DNS_ADDRTYPE_IPV6) {
        const int success = ip6addr_aton(host, &addr->u_addr.ip6);
        if (success) {
            addr->type = IPADDR_TYPE_V6;
        }
        return success;
    }

    // If neither only IPv4 or IPv6 addresses were requested, accept any valid IP address.
    return ipaddr_aton(host, addr);
}

[[gnu::noinline]]
[[gnu::nonnull]]
static err_t validate_ip_result(const char *name, const ip_addr *addr)
{
    bool valid = true;

    if (addr->type == IPADDR_TYPE_V4) {
        const char *errmsg = nullptr;
        const uint32_t ip4_u32h = addr->u_addr.ip4.addr;

        if (ip4_u32h == 0) { // 0.0.0.0
            errmsg = "blocked by DNS server";
            valid = false;
        } else {
            const uint8_t ip4_first_octet = ip4_u32h & 0xFF;

            if (ip4_first_octet == 0) { // 0.0.0.0/8
                errmsg = "resolved to invalid local IPv4 address";
                valid = false;
            } else if (ip4_first_octet == 127) { // 127.0.0.0/8
                errmsg = "resolved to IPv4 localhost :-? ";
                // considered valid
            } else if ((ip4_first_octet & 0xE0) == 0xE0) { // 224.0.0.0/3
                errmsg = "resolved to invalid multicast or broadcast IPv4 address";
                valid = false;
            }
        }

        if (errmsg != nullptr) {
            task_scheduler.scheduleOnce([errmsg, ip4 = addr->u_addr.ip4, failed_name = String{name}]() { // Can't access the logger from lwIP context.
                char ip4_str[INET_ADDRSTRLEN];
                tf_ip4addr_ntoa(&ip4, ip4_str, std::size(ip4_str));

                logger.printfln("Host '%s' %s: %s", failed_name.c_str(), errmsg, ip4_str);
            });
        }
    } else if (addr->type == IPADDR_TYPE_V6) {
        const ip6_addr_t &ip6 = addr->u_addr.ip6;

        if (ip6.addr[0] == 0 && ip6.addr[1] == 0 && ip6.addr[2] == 0 && ip6.addr[3] == 0) {
            valid = false;

            task_scheduler.scheduleOnce([failed_name = String{name}]() { // Can't access the logger from lwIP context.
                logger.printfln("Host '%s' resoved to invalid IPv6 address '::'", failed_name.c_str());
            });
        }
    }

    return valid ? ERR_OK : ERR_RST;
}

// GethostbynameData is filled on demand.
#pragma GCC diagnostic ignored "-Weffc++"

typedef std::function<void(dns_gethostbyname_addrtype_lwip_ctx_async_data *callback_arg)> FoundCallback;

struct GethostbynameData {
    GethostbynameData(FoundCallback &&callback) : found_callback(std::move(callback)) {}

    aligned_storage<Task> task_buf; // Must be first member variable

    dns_gethostbyname_addrtype_lwip_ctx_async_data output;

    FoundCallback found_callback; // Non-POD, must call destructor.
    const char *host; // Passing a pointer should be safe. Called API creates a copy if the query can't be resolved from cache.
    u8_t addrtype;
};

// Called by lwIP
static void gethostbyname_cb(const char *host, const ip_addr_t *addr, void *ctx)
{
    GethostbynameData *data = static_cast<GethostbynameData *>(ctx);

    if (addr != nullptr) {
        data->output.err = validate_ip_result(host, addr);
        data->output.addr = *addr;
    } else {
        data->output.err = ERR_CONN; // Report unresolvable host as error.
        memset(&data->output.addr, 0, sizeof(data->output.addr));
    }

    // Transfer ownership, will free the whole internal data struct.
    task_scheduler.scheduleOnceNoAlloc(&data->task_buf, true, [data]() {
        data->found_callback(&data->output);
        data->found_callback.~FoundCallback(); // Must destroy callback manually.
    });
}

// Called by lwIP
static esp_err_t gethostbyname_wrapper(void *ctx)
{
    GethostbynameData *data = static_cast<GethostbynameData *>(ctx);
    return dns_gethostbyname_addrtype(data->host, &data->output.addr, &gethostbyname_cb, ctx, data->addrtype);
}

bool dns_gethostbyname_addrtype_lwip_ctx_async(const char *host, FoundCallback &&found_callback, u8_t dns_addrtype)
{
    dns_gethostbyname_addrtype_lwip_ctx_async_data static_output;

    const int is_ip = ipaddr_aton_addrtype(host, &static_output.addr, dns_addrtype);

    if (is_ip) {
        static_output.err = ERR_OK;

        found_callback(&static_output);

        return true;
    }

    GethostbynameData *data = new(std::nothrow) GethostbynameData(std::move(found_callback));

    if (data == nullptr) {
        static_output.err = ERR_MEM;
        memset(&static_output.addr, 0, sizeof(static_output.addr));

        found_callback(&static_output);

        return true;
    }

    data->host = host;
    data->addrtype = dns_addrtype;

    const esp_err_t esp_err = esp_netif_tcpip_exec(gethostbyname_wrapper, data);

    err_t err;
    if (esp_err > 0) {
        logger.printfln("esp_netif_tcpip_exec returned 0x%x", static_cast<unsigned>(esp_err));
        err = ERR_VAL;
    } else {
        err = static_cast<err_t>(esp_err);
    }

    // Don't set the data's err if the result is not available yet.
    // The callback handler might be executed before esp_netif_tcpip_exec returns.
    if (err == ERR_INPROGRESS) {
        return false;
    }

    if (err == ERR_OK) {
        data->output.err = validate_ip_result(host, &data->output.addr);
    } else {
        data->output.err = err;
    }

    data->found_callback(&data->output); // Can't call local found_callback anymore because it has been std::move'd.
    delete data;
    return true;
}

static void dns_removehostbyaddr_safe_cb(void *ctx)
{
    const ip_addr_t *addr = static_cast<decltype(addr)>(ctx);
    dns_removehost(nullptr, addr);
}

void dns_removehostbyaddr_safe(const ip_addr_t *addr)
{
    void *cb_ctx = const_cast<void *>(static_cast<const void *>(addr)); // Casting away the const is safe because it is restored immediately inside the callback.
    tcpip_callback(&dns_removehostbyaddr_safe_cb, cb_ctx);              // Doesn't block until completion, returns after posting request.
}

static void dns_removehostbyname_safe_cb(void *ctx)
{
    const char *hostname = static_cast<decltype(hostname)>(ctx);
    dns_removehost(hostname, nullptr);
}

void dns_removehostbyname_safe(const char *hostname)
{
    void *cb_ctx = const_cast<void *>(static_cast<const void *>(hostname)); // Casting away the const is safe because it is restored immediately inside the callback.
    tcpip_callback(&dns_removehostbyname_safe_cb, cb_ctx);                  // Doesn't block until completion, returns after posting request.
}
