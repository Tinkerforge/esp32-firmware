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

#include "ModbusTCP.h"

#include "config.h"
#include "modules/meters/meter_generator.h"
#include "modules/meters_modbus_tcp/modbus_tcp_tools.h"
#include "module.h"
#include "lwip/ip_addr.h"

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
    MeterClassID get_class() const override _ATTRIBUTE((const));
    virtual IMeter *new_meter(uint32_t slot, Config *state, Config *config, Config *errors) override;
    virtual const Config *get_config_prototype() override _ATTRIBUTE((const));
    virtual const Config *get_state_prototype()  override _ATTRIBUTE((const));
    virtual const Config *get_errors_prototype() override _ATTRIBUTE((const));

//private:
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
        ReadNext,
        ReadSunSpecID,
        ReadSunSpecIDDone,
        ReadCommonModelHeader,
        ReadCommonModelHeaderDone,
        ReadCommonModelBlock,
        ReadCommonModelBlockDone,
        ReadStandardModelHeader,
        ReadStandardModelHeaderDone,
        ReportStandardModelResult,
    };

    void scan_printfln(const char *fmt, ...) __attribute__((__format__(__printf__, 2, 3)));

    Config config_prototype;

    ModbusTCP modbus;
    ConfigRoot scan;

    bool scan_new = false;
    String scan_new_host = "";
    uint16_t scan_new_port = 0;

    bool scan_log_idle = false;
    ScanState scan_state = ScanState::Idle;
    String scan_host = "";
    dns_gethostbyname_addrtype_lwip_ctx_async_data scan_host_data;
    IPAddress scan_host_address;
    uint16_t scan_port;
    uint8_t scan_device_address;
    size_t scan_base_address_index;
    size_t scan_read_address;
    size_t scan_read_size;
    uint16_t scan_read_buffer[68];
    uint32_t scan_read_cookie = 0;
    ModbusDeserializer scan_deserializer;
    size_t scan_read_index;
    Modbus::ResultCode scan_read_result;
    ScanState scan_read_state;
    size_t scan_common_block_length;
    uint16_t scan_standard_model_id;
    size_t scan_standard_block_length;
    char scan_common_manufacturer_name[32 + 1];
    char scan_common_model_name[32 + 1];
    char scan_common_options[16 + 1];
    char scan_common_version[16 + 1];
    char scan_common_serial_number[32 + 1];
    uint16_t scan_common_device_address;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
