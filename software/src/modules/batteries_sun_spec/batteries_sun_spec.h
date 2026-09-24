/* esp32-firmware
 * Copyright (C) 2026 Matthias Bolte <matthias@tinkerforge.com>
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
#include "language.h"
#include "battery_sun_spec.h"
#include "modules/batteries/ibattery_generator.h"
#include "modules/sun_spec/sun_spec_resolver.h"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#include "gcc_warnings.h"
#pragma GCC diagnostic ignored "-Weffc++"
#endif

class BatteriesSunSpec final : public IModule, public IBatteryGenerator
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

    void trace_timestamp();

    size_t trace_buffer_index;

private:
    enum class TestState : uint8_t {
        Start,
        Connect,
        Connecting,
        Disconnect,
        Done,
        CreateResolver,
        DestroyResolver,
        Resolving,
        CreateWriter,
        DestroyWriter,
        Writing,
    };

    void test_resolve_result(SunSpecResolverCommonModel *common_model, uint16_t start_address, uint16_t block_length);
    void test_flush_log();
    [[gnu::format(__printf__, 2, 0)]] void test_vprintfln(const char *fmt, va_list args);
    [[gnu::format(__printf__, 2, 3)]] void test_printfln(const char *fmt, ...);

    Config config_prototype;
    Config state_prototype;
    //Config errors_prototype;

    ConfigRoot test_config;
    ConfigRoot test_continue_config;
    ConfigRoot test_stop_config;

    struct Test {
        Language language;
        TFGenericTCPSharedClient *shared_client = nullptr;
        micros_t last_keep_alive = 0_us;
        uint32_t slot;
        String host;
        uint16_t port;
        uint8_t device_address;
        String manufacturer_name;
        String model_name;
        String serial_number;
        uint16_t model_instance;
        int8_t force_charge_rate;
        int8_t force_discharge_rate;
        BatteryMode mode;
        uint32_t cookie;
        char trace_prefix[8 + 1]; // strlen("bXYr t1 ") == 8
        SunSpecResolver *resolver = nullptr;
        uint16_t resolved_address;
        BatterySunSpec::WriterContext *writer_ctx = nullptr;
        TestState state;
        TestState state_after_destroy_resolver;
        char printfln_buffer[512] = "";
        micros_t printfln_last_flush = 0_us;
        size_t printfln_buffer_used = 0;
        bool reconnect = false;
        bool stop = false;
    };

    Test *test = nullptr;

    BatterySunSpec *instances[OPTIONS_BATTERIES_MAX_SLOTS()];

    micros_t last_trace_timestamp = -1_us;
};

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
