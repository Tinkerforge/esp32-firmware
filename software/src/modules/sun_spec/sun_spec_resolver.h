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

struct SunSpecResolverCommonModel {
    char Mn[32 + 1];
    char Md[32 + 1];
    char Opt[16 + 1];
    char Vr[16 + 1];
    char SN[32 + 1];
};

typedef std::function<void()> SunSpecResolverTimeoutCallback;
typedef std::function<void(SunSpecResolverCommonModel *common_model, uint16_t start_address, uint16_t block_length)> SunSpecResolverResultCallback;

bool sun_spec_is_solar_edge(const char *manufacturer);
bool sun_spec_is_kostal(const char *manufacturer);
bool sun_spec_is_kostal_smart_energy_meter(const char *model);

class SunSpecResolver final
{
private:
    SunSpecResolver() {}
    ~SunSpecResolver() {}

public:
    static SunSpecResolver *create(const char *event_log_prefix_override,
                                   const char *event_log_message_prefix,
                                   std::function<void(void)> &&trace_timestamp_callback,
                                   size_t trace_buffer_index,
                                   const char *trace_log_message_prefix,
                                   TFGenericTCPSharedClient *shared_client,
                                   uint8_t device_address,
                                   const char *manufacturer_name,
                                   const char *model_name,
                                   const char *serial_number,
                                   uint16_t model_id,
                                   uint16_t model_instance,
                                   SunSpecResolverResultCallback &&result_callback,
                                   SunSpecResolverTimeoutCallback &&timeout_callback);

    void destroy();

private:
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

    const char *event_log_prefix_override;
    size_t event_log_prefix_override_len;
    const char *event_log_message_prefix;
    std::function<void(void)> trace_timestamp_callback;
    size_t trace_buffer_index;
    const char *trace_log_message_prefix;
    TFGenericTCPSharedClient *shared_client;
    uint8_t device_address;
    const char *manufacturer_name;
    const char *model_name;
    const char *serial_number;
    uint16_t model_id;
    uint16_t model_instance;
    SunSpecResolverResultCallback result_callback;
    SunSpecResolverTimeoutCallback timeout_callback;

    size_t error_counter = 0;
    size_t base_address_index = 0;
    State state = State::Idle;
    State state_next = State::ReadSunSpecID;
    bool device_found = false;
    uint16_t model_counter;
    uint16_t buffer[68]; // buffer must be big enough for the Common model
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
