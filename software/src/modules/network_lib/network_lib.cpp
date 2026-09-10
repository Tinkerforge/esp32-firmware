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

#include "network_lib.h"

#include <TFNetwork.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "tools/dns.h"

#include "gcc_warnings.h"

[[gnu::format(__printf__, 1, 0)]]
void TFNetwork::vlogfln(const char *fmt, va_list args)
{
    logger.vprintfln(fmt, args);
}

void TFNetwork::resolve(const char *host, std::function<void(ip_addr_t *address, int error_number)> &&callback)
{
    dns_gethostbyname_addrtype_lwip_ctx_async(host, [callback](dns_gethostbyname_addrtype_lwip_ctx_async_data *data) {
        if (data->err != ERR_OK) {
            callback(nullptr, err_to_errno(data->err));
        }
        else if (data->err == ERR_CONN) {
            callback(nullptr, -1); // no address available for this host
        }
        else {
            callback(&data->addr, -1);
        }
    }, LWIP_DNS_ADDRTYPE_DEFAULT);
}

uint16_t TFNetwork::get_random_uint16()
{
    uint32_t r = esp_random();

    return static_cast<uint16_t>((r >> 16) ^ r);
}

void TFNetwork::abort(const char *message)
{
    esp_system_abort(message);
}

void NetworkLib::setup()
{
    initialized = true;
}
