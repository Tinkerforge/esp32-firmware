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

#pragma once

#include <stdint.h>

#include "config.h"
#include "module.h"
#include "battery_modbus_tcp.h"
#include "battery_modbus_tcp_table_id.enum.h"
#include "modules/batteries/ibattery_generator.h"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#include "gcc_warnings.h"
#pragma GCC diagnostic ignored "-Weffc++"
#endif

class BatteriesModbusTCP final : public IModule, public IBatteryGenerator
{
public:
    // for IModule
    void pre_setup() override;
    void register_urls() override;
    void loop() override;

    // for IBatteryGenerator
    [[gnu::const]] BatteryClassID get_class() const override;
    virtual IBattery *new_battery(uint32_t slot, Config *state, Config *errors) override;
    [[gnu::const]] virtual const Config *get_config_prototype() override;
    [[gnu::const]] virtual const Config *get_state_prototype() override;
    [[gnu::const]] virtual const Config *get_errors_prototype() override;
    virtual String validate_config(Config &update, ConfigSource source) override;

    void trace_timestamp();

    size_t trace_buffer_index;

private:
    enum class TestState : uint8_t {
        Connect,
        Connecting,
        Disconnect,
        Done,
        CreateTableWriter,
        DestroyTableWriter,
        TableWriting,
    };

    void test_flush_log();
    [[gnu::format(__printf__, 2, 0)]] void test_vprintfln(const char *fmt, va_list args);
    [[gnu::format(__printf__, 2, 3)]] void test_printfln(const char *fmt, ...);

    Config config_prototype;
    Config table_custom_register_block_prototype;
    std::vector<ConfUnionPrototype<BatteryModbusTCPTableID>> table_prototypes;
    //Config errors_prototype;

    ConfigRoot test_config;
    std::vector<ConfUnionPrototype<BatteryModbusTCPTableID>> test_table_prototypes;
    ConfigRoot test_continue_config;
    ConfigRoot test_stop_config;

    struct Test {
        TFGenericTCPSharedClient *client = nullptr;
        micros_t last_keep_alive = 0_us;
        uint32_t slot;
        String host;
        uint16_t port;
        uint32_t cookie;
        BatteryMode mode;
        uint8_t device_address;
        uint16_t repeat_interval; // seconds
        BatteryModbusTCP::TableSpec *table;
        BatteryModbusTCP::TableWriter *writer;
        TestState state = TestState::Connect;
        char printfln_buffer[512] = "";
        micros_t printfln_last_flush = 0_us;
        size_t printfln_buffer_used = 0;
        bool reconnect = false;
        bool stop = false;
    };

    Test *test = nullptr;

    BatteryModbusTCP *instances[OPTIONS_BATTERIES_MAX_SLOTS()];

    micros_t last_trace_timestamp = -1_us;
};

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
