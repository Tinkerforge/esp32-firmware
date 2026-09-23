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
#include <functional>
#include <TFModbusTCPClient.h>

#include "modules/modbus_tcp_client/modbus_tcp_tools.h"
#include "language.h"

struct SunSpecResolverCommonModel {
    char Mn[32 + 1];
    char Md[32 + 1];
    char Opt[16 + 1];
    char Vr[16 + 1];
    char SN[32 + 1];
};

typedef std::function<void(const char *fmt, va_list args)> SunSpecResolverVLogFLnCallback;
typedef std::function<void(const char *buf, size_t len)> SunSpecResolverTracePlainCallback;
typedef std::function<void(SunSpecResolverCommonModel *common_model, uint16_t start_address, uint16_t block_length)> SunSpecResolverResultCallback;
typedef std::function<void()> SunSpecResolverTimeoutCallback;

bool sun_spec_is_solar_edge(const char *manufacturer);
bool sun_spec_is_kostal(const char *manufacturer);
bool sun_spec_is_kostal_smart_energy_meter(const char *model);

class SunSpecResolver final
{
private:
    SunSpecResolver() {}
    ~SunSpecResolver() {}

public:
    static SunSpecResolver *create(const char *print_prefix,
                                   SunSpecResolverVLogFLnCallback &&vprintfln_callback,
                                   const char *trace_prefix,
                                   SunSpecResolverVLogFLnCallback &&vtracefln_callback,
                                   SunSpecResolverTracePlainCallback &&trace_plain_callback,
                                   TFGenericTCPSharedClient *shared_client,
                                   uint8_t device_address,
                                   const char *manufacturer_name,
                                   const char *model_name,
                                   const char *serial_number,
                                   uint16_t model_id,
                                   uint16_t model_instance,
                                   SunSpecResolverResultCallback &&result_callback,
                                   SunSpecResolverTimeoutCallback &&timeout_callback,
                                   Language language = Language::English);

    void destroy();

private:
    [[gnu::format(__printf__, 2, 3)]]
    void printfln_(const char *fmt, ...);

    [[gnu::format(__printf__, 2, 3)]]
    void tracefln_(const char *fmt, ...);

    void read();
    void next_base_address();
    void next();
    void report_result(SunSpecResolverCommonModel *common_model, uint16_t start_address, uint16_t block_length);

    enum class State {
        Idle,
        ReadSunSpecID,
        ReadModelHeader,
        ReadModel,
    };

    const char *print_prefix;
    SunSpecResolverVLogFLnCallback vprintfln_callback;
    const char *trace_prefix;
    SunSpecResolverVLogFLnCallback vtracefln_callback;
    SunSpecResolverTracePlainCallback trace_plain_callback;
    TFGenericTCPSharedClient *shared_client;
    uint8_t device_address;
    const char *manufacturer_name;
    const char *model_name;
    const char *serial_number;
    uint16_t model_id;
    uint16_t model_instance;
    SunSpecResolverResultCallback result_callback;
    SunSpecResolverTimeoutCallback timeout_callback;
    Language language;

    size_t error_counter = 0;
    size_t base_address_index = 0;
    State state = State::Idle;
    State state_next = State::ReadSunSpecID;
    bool device_found = false;
    uint16_t model_counter;
    uint16_t buffer[67]; // buffer must be big enough for the common model
    ModbusDeserializer deserializer;
    uint16_t start_address;
    uint16_t data_count;
    SunSpecResolverCommonModel common_model;

    uint64_t read_task_id = 0;
    bool read_pending = false;
    bool destroy_requested = false;
    bool log_read_errors = false;
    TFModbusTCPClientTransactionResult last_read_result = TFModbusTCPClientTransactionResult::Success;
    size_t last_read_result_burst_length = 0;
};
