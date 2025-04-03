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
#include <TFModbusTCPClient.h>
#include <TFTools/Micros.h>
#include <lwip/ip_addr.h>

#include "module.h"
#include "modules/meters/imeter_generator.h"
#include "modules/modbus_tcp_client/modbus_tcp_tools.h"
#include "config.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MetersSunSpec final : public IModule, public IMeterGenerator
{
public:
    MetersSunSpec() {}

    // for IModule
    void pre_setup() override;
    void register_urls() override;
    void loop() override;

    // for IMeterGenerator
    [[gnu::const]] MeterClassID get_class() const override;
    virtual IMeter *new_meter(uint32_t slot, Config *state, Config *errors) override;
    [[gnu::const]] virtual const Config *get_config_prototype() override;
    [[gnu::const]] virtual const Config *get_state_prototype()  override;
    [[gnu::const]] virtual const Config *get_errors_prototype() override;

    void trace_timestamp();

private:
    enum class ScanState : uint8_t {
        Connect,
        Connecting,
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
        ReadModelID,
        ReadModelIDDone,
        ReadModelBlockLength,
        ReadModelBlockLengthDone,
        ReportModelResult,
        ReportError,
    };

    ScanState scan_get_next_state_after_read_error();
    void scan_flush_log();
    [[gnu::format(__printf__, 2, 3)]] void scan_printfln(const char *fmt, ...);

    Config config_prototype;
    Config errors_prototype;

    ConfigRoot scan_config;
    ConfigRoot scan_continue_config;
    ConfigRoot scan_abort_config;

    struct Scan {
        Scan() : client(TFModbusTCPByteOrder::Host) {}

        TFModbusTCPClient client;
        micros_t last_keep_alive = 0_us;
        String host;
        uint16_t port;
        uint8_t device_address_first;
        uint8_t device_address_last;
        uint8_t device_address;
        ScanState state = ScanState::Connect;
        uint16_t model_id;
        uint32_t cookie;
        size_t base_address_index = 0;
        size_t read_address;
        size_t read_size;
        size_t read_retries;
        size_t read_index;
        micros_t read_delay_deadline;
        uint16_t read_buffer[68];
        micros_t read_timeout = 1_s;
        uint16_t read_timeout_burst = 0;
        ScanState read_state;
        ScanState error_state;
        bool abort = false;
        ModbusDeserializer deserializer;
        TFModbusTCPClientTransactionResult read_result;
        std::unordered_map<uint16_t, uint16_t> model_instances;
        size_t block_length;
        char printfln_buffer[512] = "";
        micros_t printfln_last_flush = 0_us;
        size_t printfln_buffer_used = 0;
        char common_manufacturer_name[32 + 1];
        char common_model_name[32 + 1];
        char common_serial_number[32 + 1];
    };

    Scan *scan = nullptr;

    size_t trace_buffer_index;
    micros_t last_trace_timestamp = -1_us;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
