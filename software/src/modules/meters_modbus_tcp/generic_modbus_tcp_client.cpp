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

#define EVENT_LOG_PREFIX "gen_mbtcp_client"

#include "generic_modbus_tcp_client.h"

#include <errno.h>
#include <string.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "modbus_tcp_tools.h"

#include "gcc_warnings.h"

void GenericModbusTCPClient::start_connection()
{
    host_data.user = this;

    stop_connection();

    dns_gethostbyname_addrtype_lwip_ctx_async(host_name.c_str(), [](dns_gethostbyname_addrtype_lwip_ctx_async_data *data) {
        GenericModbusTCPClient *gmbtc = static_cast<GenericModbusTCPClient *>(data->user);

        gmbtc->check_ip(data->addr_ptr, data->err);
    }, &host_data, LWIP_DNS_ADDRTYPE_IPV4);
}

void GenericModbusTCPClient::stop_connection()
{
    if (modbus->isConnected(host_ip) && modbus->disconnect(host_ip)) {
        logger.printfln("Disconnecting from '%s'", host_name.c_str());
        disconnect_callback();
    }

    host_ip = IPAddress(0u);
    last_successful_read = 0_us;
}

void GenericModbusTCPClient::check_ip(const ip_addr_t *ip, int err)
{
    if (err != ERR_OK || !ip || ip->type != IPADDR_TYPE_V4) {
        if (!resolve_error_printed) {
            if (err == ERR_VAL) {
                logger.printfln("Meter configured with hostname '%s', but no DNS server is configured!", host_name.c_str());
            } else {
                logger.printfln("Couldn't resolve hostname '%s', error %i", host_name.c_str(), err);
            }
            resolve_error_printed = true;
        }

        task_scheduler.scheduleOnce([this]() {
            this->start_connection();
        }, 10 * 1000);

        return;
    }
    resolve_error_printed = false;

    host_ip = ip->u_addr.ip4.addr;

    errno = ENOTRECOVERABLE; // Set to something known because connect() might leave errno unchanged on some errors.
    if (!modbus->connect(host_ip, port)) {
        if (errno != last_connect_errno) {
            if (errno == EINPROGRESS) { // WiFiClient::connect() doesn't set errno when its select() call times out and incorrectly leaves it at EINPROGRESS despite being blocking.
                logger.printfln("Connection attempt to '%s' timed out", host_name.c_str());
            } else {
                logger.printfln("Connection to '%s' failed: %s (%i)", host_name.c_str(), strerror(errno), errno);
            }
            last_connect_errno = errno;
        }

        task_scheduler.scheduleOnce([this]() {
            this->start_connection();
        }, connect_backoff_ms);

        connect_backoff_ms *= 2;
        if (connect_backoff_ms > 16000) {
            connect_backoff_ms = 16000;
        }
    } else {
        connect_backoff_ms = 1000;
        last_connect_errno = 0;
        logger.printfln("Connected to '%s'", host_name.c_str());
        connect_callback();
    }
}

void GenericModbusTCPClient::start_generic_read()
{
    if (!modbus->isConnected(host_ip)) {
        logger.printfln("Connection lost, reconnecting to '%s'", host_name.c_str());
        start_connection();
        return;
    }

    if (last_successful_read != 0_us && deadline_elapsed(last_successful_read + successful_read_timeout)) {
        logger.printfln("Last successful read occurred too long ago, reconnecting to '%s'", host_name.c_str());
        start_connection();
        return;
    }

    read_buffer_num = 0;
    registers_done_count = 0;

    if (generic_read_request.register_count == 0) {
        generic_read_request.result_code = Modbus::ResultCode::EX_DATA_MISMACH;
        generic_read_request.done_callback();
        return;
    }

    size_t read_blocks = (generic_read_request.register_count + METERS_MODBUS_TCP_MAX_HREG_WORDS - 1) / METERS_MODBUS_TCP_MAX_HREG_WORDS;
    read_block_size = static_cast<uint16_t>((generic_read_request.register_count + read_blocks - 1) / read_blocks);

    read_next();
}

void GenericModbusTCPClient::read_next()
{
    // data is always a nullptr.
    // Return value doesn't matter. The caller discards it.
    cbTransaction read_done_cb = [this](Modbus::ResultCode result_code, uint16_t /*transaction_id*/, void * /*data*/)->bool {
        if (result_code != Modbus::ResultCode::EX_SUCCESS) {
            logger.printfln("read%creg failed: %s (0x%02x) host=%s port=%u device_address=%u start_address=%u register_count=%u",
                            generic_read_request.register_type == ModbusRegisterType::HoldingRegister ? 'H' : 'I',
                            get_modbus_result_code_name(result_code),
                            static_cast<uint32_t>(result_code),
                            host_ip.toString().c_str(),
                            port,
                            device_address,
                            generic_read_request.start_address,
                            generic_read_request.register_count);

            generic_read_request.result_code = result_code;
            generic_read_request.done_callback();
            return false;
        }

        registers_done_count = static_cast<uint16_t>(registers_done_count + read_block_size);

        if (registers_done_count >= generic_read_request.register_count) {
            // buffer done
            if (generic_read_request.read_twice && read_buffer_num == 0) {
                // Two reads requested and first read is done. -> Next buffer.
                read_buffer_num = 1;
                registers_done_count = 0;
            } else {
                // Only one read requested or second buffer done. -> All done.
                last_successful_read = now_us();
                generic_read_request.result_code = Modbus::ResultCode::EX_SUCCESS;
                generic_read_request.done_callback();
                return true;
            }
        }

        read_next();

        return true;
    };

    uint16_t *target_buffer = generic_read_request.data[read_buffer_num] + registers_done_count;
    uint16_t read_start_address = static_cast<uint16_t>(generic_read_request.start_address + registers_done_count);
    uint16_t registers_remaining = static_cast<uint16_t>(generic_read_request.register_count - registers_done_count);
    uint16_t read_count = registers_remaining < read_block_size ? registers_remaining : read_block_size;

    uint16_t ret;
    switch (generic_read_request.register_type) {
    case ModbusRegisterType::HoldingRegister: ret = modbus->readHreg(host_ip, read_start_address, target_buffer, read_count, read_done_cb, device_address); break;
    case ModbusRegisterType::InputRegister:   ret = modbus->readIreg(host_ip, read_start_address, target_buffer, read_count, read_done_cb, device_address); break;
    default:
        esp_system_abort("generic_modbus_tcp_client: Unsupported register type to read.");
    }

    if (ret == 0) {
        logger.printfln("Modbus read failed. host=%s port=%u device_address=%u rtype=%u read_start_address=%u read_count=%u",
            host_ip.toString().c_str(), port, device_address, static_cast<uint8_t>(generic_read_request.register_type), read_start_address, read_count);

        generic_read_request.result_code = Modbus::ResultCode::EX_GENERAL_FAILURE;
        generic_read_request.done_callback();
    }

    // Read successfully dispatched, will continue in callback.
}
