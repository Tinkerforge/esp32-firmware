/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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
#include <unordered_map>
#include <ModbusTCP.h>
#include <lwip/ip_addr.h>

#include "module.h"
#include "modules/meters/meter_generator.h"
#include "modules/meters_modbus_tcp/modbus_tcp_tools.h"
#include "config.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MetersSunSpec final : public IModule, public MeterGenerator
{
public:
    // for IModule
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void loop() override;

    // for MeterGenerator
    [[gnu::const]] MeterClassID get_class() const override;
    virtual IMeter *new_meter(uint32_t slot, Config *state, Config *errors) override;
    [[gnu::const]] virtual const Config *get_config_prototype() override;
    [[gnu::const]] virtual const Config *get_state_prototype()  override;
    [[gnu::const]] virtual const Config *get_errors_prototype() override;

private:
    enum class ScanState {
        Idle,
        Resolve,
        Resolving,
        Connect,
        Disconnect,
        Done,
        NextDeviceAddress,
        NextBaseAddress,
        Read,
        Reading,
        ReadDelay,
        ReadNext,
        ReadSunSpecID,
        ReadSunSpecIDDone,
        ReadCommonModelBlock,
        ReadCommonModelBlockDone,
        ReadModelHeader,
        ReadModelHeaderDone,
        ReportModelResult,
    };

    ScanState scan_get_next_state_after_read_error();
    void scan_flush_log();
    [[gnu::format(__printf__, 2, 3)]] void scan_printfln(const char *fmt, ...);

    Config config_prototype;

    ModbusTCP modbus;
    ConfigRoot scan_config;
    ConfigRoot scan_continue_config;
    ConfigRoot scan_abort_config;

    bool scan_new = false;
    String scan_new_host;
    uint16_t scan_new_port;
    uint8_t scan_new_device_address_first;
    uint8_t scan_new_device_address_last;
    uint32_t scan_new_cookie;

    micros_t scan_last_keep_alive = 0_us;
    bool scan_abort = false;
    ScanState scan_state = ScanState::Idle;
    String scan_host;
    dns_gethostbyname_addrtype_lwip_ctx_async_data scan_host_data;
    IPAddress scan_host_address;
    uint16_t scan_port;
    uint8_t scan_device_address_first;
    uint8_t scan_device_address_last;
    uint32_t scan_cookie;
    uint8_t scan_device_address;
    size_t scan_base_address_index;
    size_t scan_read_address;
    size_t scan_read_size;
    size_t scan_read_retries;
    micros_t scan_read_delay_deadline;
    uint16_t scan_read_buffer[68];
    uint16_t scan_read_timeout_burst;
    uint32_t scan_read_cookie = 0;
    ModbusDeserializer scan_deserializer;
    size_t scan_read_index;
    Modbus::ResultCode scan_read_result;
    ScanState scan_read_state;
    uint16_t scan_model_id;
    std::unordered_map<uint16_t, uint16_t> scan_model_instances;
    size_t scan_block_length;
    char scan_printfln_buffer[512] = "";
    size_t scan_printfln_buffer_used = 0;
    micros_t scan_printfln_last_flush = 0_us;
    char scan_common_manufacturer_name[32 + 1];
    char scan_common_model_name[32 + 1];
    char scan_common_serial_number[32 + 1];
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
