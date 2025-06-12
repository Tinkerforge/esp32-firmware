/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

#define EVENT_LOG_PREFIX "modbus_tcp_clnt"

#include "modbus_tcp_client.h"

#include <TFNetworkUtil.h>
#include <esp_timer.h>

#include "tools/dns.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"

void ModbusTCPClient::setup()
{
    TFNetworkUtil::vlogfln = [](const char *fmt, va_list args) {
        logger.vprintfln(fmt, args);
    };

    TFNetworkUtil::resolve = [this](const char *host, std::function<void(uint32_t address, int error_number)> &&callback) {
        dns_gethostbyname_addrtype_lwip_ctx_async(host, [callback](dns_gethostbyname_addrtype_lwip_ctx_async_data *data) {
            if (data->err != ERR_OK) {
                callback(0, err_to_errno(data->err));
            }
            else if (data->addr_ptr == nullptr || data->addr_ptr->type != IPADDR_TYPE_V4) {
                callback(0, -1); // FIXME: resolve failed but no errno?
            }
            else {
                callback(data->addr_ptr->u_addr.ip4.addr, -1);
            }
        }, LWIP_DNS_ADDRTYPE_IPV4);
    };

    initialized = true;
}

void ModbusTCPClient::loop()
{
    pool.tick();
}

TFModbusTCPClientPool *ModbusTCPClient::get_pool()
{
    return &pool;
}
