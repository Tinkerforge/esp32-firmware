/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "generic_modbus_tcp_client.h"

#include <errno.h>
#include <string.h>

#include "event_log.h"
#include "task_scheduler.h"

void GenericModbusTCPClient::start_connection()
{
    host_data.user = this;

    dns_gethostbyname_addrtype_lwip_ctx_async(host_name.c_str(), [](dns_gethostbyname_addrtype_lwip_ctx_async_data *data) {
        GenericModbusTCPClient *gmbtc = static_cast<GenericModbusTCPClient *>(data->user);

        gmbtc->check_ip(data->addr_ptr, data->err);
    }, &host_data, LWIP_DNS_ADDRTYPE_IPV4);
}

void GenericModbusTCPClient::check_ip(const ip_addr_t *ip, int err)
{
    if (err != ERR_OK || !ip || ip->type != IPADDR_TYPE_V4) {
        if (!resolve_error_printed) {
            if (err == ERR_VAL) {
                logger.printfln("generic_modbus_tcp_client: Meter configured with hostname '%s', but no DNS server is configured!", host_name.c_str());
            } else {
                logger.printfln("generic_modbus_tcp_client: Couldn't resolve hostname '%s', error %i", host_name.c_str(), err);
            }
            resolve_error_printed = true;
        }

        task_scheduler.scheduleOnce([this](){
            this->start_connection();
        }, 10 * 1000);

        return;
    }
    resolve_error_printed = false;

    host_ip = ip->u_addr.ip4.addr;

    errno = ENOTRECOVERABLE; // Set to something known because connect() might leave errno unchanged on some errors.
    if (!mb->connect(host_ip)) {
        if (!connect_error_printed) {
            if (errno == EINPROGRESS) { // WiFiClient::connect() doesn't set errno and incorrectly returns EINPROGRESS despite being blocking.
                logger.printfln("generic_modbus_tcp_client: Connection to '%s' failed.", host_name.c_str());
            } else {
                logger.printfln("generic_modbus_tcp_client: Connection to '%s' failed: %s (%i)", host_name.c_str(), strerror(errno), errno);
            }
            connect_error_printed = true;
        }

        task_scheduler.scheduleOnce([this](){
            this->start_connection();
        }, connect_backoff_ms);

        connect_backoff_ms *= 2;
        if (connect_backoff_ms > 16000) {
            connect_backoff_ms = 16000;
        }
    } else {
        connect_backoff_ms = 1000;
        connect_error_printed = false;
        logger.printfln("generic_modbus_tcp_client: Connected to '%s'", host_name.c_str());
    }

    connect_callback();
}
