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

#pragma once

#include "generic_modbus_tcp_client.h"

#include <stdint.h>

#include "config.h"
#include "modules/meters/imeter.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MeterModbusTCP final : protected GenericModbusTCPClient, public IMeter
{
public:
    MeterModbusTCP(uint32_t slot_, Config *state_, Config *errors_, ModbusTCP *mb_) : GenericModbusTCPClient(mb_), slot(slot_), state(state_), errors(errors_) {}

    MeterClassID get_class() const override _ATTRIBUTE((const));
    void setup(Config &ephemeral_config) override;

    bool supports_power()         override {return true;}
    bool supports_energy_import() override {return true;}
    bool supports_energy_export() override {return true;}
    bool supports_currents()      override {return true;}

private:
    void connect_callback() override;
    void disconnect_callback() override;
    void poll_next();
    void handle_data();

    const uint32_t slot;
    Config * const state;
    Config * const errors;

    enum PollState {
        Single,
        Combined,
        Done,
    };
    PollState poll_state;
    uint16_t poll_count;
    uint16_t *register_buffer;
    uint32_t register_buffer_size = METERS_MODBUS_TCP_MAX_HREG_WORDS;

    micros_t request_start;
    micros_t all_start;
    uint32_t worst_runtime;
    uint32_t best_runtime;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
