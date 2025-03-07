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

#include "dns.h"
#include "main_dependencies.h"

#include "gcc_warnings.h"

struct gethostbyname_parameters {
    const char *host;
    ip_addr_t *addr;
    dns_found_callback found_callback;
    void *callback_arg;
    u8_t dns_addrtype;
};

static esp_err_t gethostbyname_lwip_ctx(void *ctx)
{
    gethostbyname_parameters *parameters = static_cast<gethostbyname_parameters *>(ctx);
    return dns_gethostbyname(parameters->host, parameters->addr, parameters->found_callback, parameters->callback_arg);
}

err_t dns_gethostbyname_lwip_ctx(const char *host, ip_addr_t *addr, dns_found_callback found_callback, void *callback_arg)
{
    gethostbyname_parameters parameters;
    parameters.host = host;
    parameters.addr = addr;
    parameters.found_callback = found_callback;
    parameters.callback_arg = callback_arg;

    return static_cast<err_t>(esp_netif_tcpip_exec(gethostbyname_lwip_ctx, &parameters));
}

static esp_err_t gethostbyname_addrtype_lwip_ctx(void *ctx)
{
    gethostbyname_parameters *parameters = static_cast<gethostbyname_parameters *>(ctx);
    return dns_gethostbyname_addrtype(parameters->host, parameters->addr, parameters->found_callback, parameters->callback_arg, parameters->dns_addrtype);
}

err_t dns_gethostbyname_addrtype_lwip_ctx(const char *host, ip_addr_t *addr, dns_found_callback found_callback, void *callback_arg, u8_t dns_addrtype)
{
    gethostbyname_parameters parameters;
    parameters.host = host;
    parameters.addr = addr;
    parameters.found_callback = found_callback;
    parameters.callback_arg = callback_arg;
    parameters.dns_addrtype = dns_addrtype;

    return static_cast<err_t>(esp_netif_tcpip_exec(gethostbyname_addrtype_lwip_ctx, &parameters));
}

static void gethostbyname_addrtype_lwip_ctx_async(const char */*host*/, const ip_addr_t *addr, void *callback_arg)
{
    dns_gethostbyname_addrtype_lwip_ctx_async_data *data = static_cast<dns_gethostbyname_addrtype_lwip_ctx_async_data *>(callback_arg);

    data->err = ERR_OK; // ERR_OK because we got a response. Response might be negative and ipaddr a nullptr, though.

    if (addr != nullptr) {
        data->addr = *addr;
        data->addr_ptr = &data->addr;
    }
    else {
        data->addr_ptr = nullptr;
    }

    task_scheduler.scheduleOnce([data]() {
        data->found_callback(data);
    });
}

void dns_gethostbyname_addrtype_lwip_ctx_async(const char *host,
                                               std::function<void(dns_gethostbyname_addrtype_lwip_ctx_async_data *callback_arg)> &&found_callback,
                                               dns_gethostbyname_addrtype_lwip_ctx_async_data *callback_arg,
                                               u8_t dns_addrtype)
{
    callback_arg->found_callback = std::move(found_callback);
    err_t err = dns_gethostbyname_addrtype_lwip_ctx(host, &callback_arg->addr, gethostbyname_addrtype_lwip_ctx_async, callback_arg, dns_addrtype);

    // Don't set the callback_arg's err if the result is not available yet.
    // The callback handler might be executed before dns_gethostbyname_addrtype_lwip_ctx returns.
    if (err == ERR_INPROGRESS)
        return;

    callback_arg->err = err;
    callback_arg->addr_ptr = &callback_arg->addr;

    callback_arg->found_callback(callback_arg); // Can't call local found_callback anymore because it has been std::forward'ed.
}
