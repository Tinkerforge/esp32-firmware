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

#include "modules/meters/imeter.h"
#include "modules/modbus_tcp_client/generic_modbus_tcp_client.h"
#include "modules/modbus_tcp_client/modbus_tcp_tools.h"
#include "config.h"
#include "model_parser.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MeterSunSpec final : protected GenericModbusTCPClient, public IMeter
{
public:
    MeterSunSpec(uint32_t slot_, Config *state_, Config *errors_, TFModbusTCPClientPool *pool_, size_t trace_buffer_index_) :
        GenericModbusTCPClient("meters_sun_spec", format_meter_slot(slot_), pool_), slot(slot_), state(state_), errors(errors_), trace_buffer_index(trace_buffer_index_) {}

    [[gnu::const]] MeterClassID get_class() const override;
    void setup(Config *ephemeral_config) override;
    void register_events() override;
    void pre_reboot() override;

    bool supports_power()         override {return true;}
    bool supports_energy_import() override {return true;}
    bool supports_energy_imexsum()override {return true;}
    bool supports_energy_export() override {return true;}
    bool supports_currents()      override {return true;}

    void read_done_callback();

private:
    enum class ScanState {
        Idle,
        ReadSunSpecID,
        ReadModelHeader,
        ReadModel,
    };

    void connect_callback() override;
    void disconnect_callback() override;

    bool alloc_read_buffer(size_t model_regcount);
    void trace_response();
    void read_start(size_t model_regcount);

    void scan_start_delay();
    void scan_start();
    void scan_read_delay();
    void scan_next_base_address();
    void scan_next();

    uint32_t slot;
    Config *state;
    Config *errors;
    size_t trace_buffer_index;

    bool read_allowed = false;
    bool values_declared = false;

    String manufacturer_name;
    String model_name;
    String serial_number;
    uint16_t model_id;
    uint16_t model_instance;
    DCPortType dc_port_type;
    size_t scan_base_address_index;
    ScanState scan_state;
    ScanState scan_state_next;
    ModbusDeserializer scan_deserializer;
    bool scan_device_found;
    uint16_t scan_model_counter;

    uint32_t quirks = 0;
    IMetersSunSpecParser *model_parser;

    bool check_phase_voltages = false;
    uint32_t phase_voltage_index_cache[3];

    micros_t last_successful_parse = 0_us;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
