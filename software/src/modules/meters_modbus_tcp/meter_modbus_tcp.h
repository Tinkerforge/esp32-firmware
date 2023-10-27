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

#include <IPAddress.h>
#include <stdint.h>
#include "lwip/ip_addr.h"

// Connect attempts are blocking. Use a low timeout that should usually work and just try again if it doesn't.
#define MODBUSIP_CONNECT_TIMEOUT 500

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wvla"
#endif
#include "ModbusTCP.h"
#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

#include "config.h"
#include "modules/meters/imeter.h"
#include "tools.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MeterModbusTCP final : public IMeter
{
public:
    MeterModbusTCP(uint32_t slot_, Config *config_, Config *state_, Config *errors_, ModbusTCP *mb_) : slot(slot_), config(config_), state(state_), errors(errors_), mb(mb_) {}

    MeterClassID get_class() const override;
    void setup() override;

    bool supports_power()         override {return true;}
    bool supports_energy_import() override {return true;}
    bool supports_energy_export() override {return true;}
    bool supports_currents()      override {return true;}

    void check_ip(const ip_addr_t *ip, int err);

private:
    void start_connection();
    void connect_to_ip();
    void poll_next();
    void handle_data();

    uint32_t slot;
    Config *config;
    Config *state;
    Config *errors;

    ModbusTCP *mb;

    String host_name;
    IPAddress host_ip;
    uint16_t port = 0;
    uint8_t address = 0;

    uint32_t connect_backoff_ms = 1000;
    bool resolve_error_printed = false;
    bool connect_error_printed = false;

    enum PollState {
        Single,
        Combined,
        Done,
    };
    PollState poll_state;
    uint16_t poll_count;
    uint16_t *register_buffer;
    uint32_t register_buffer_size = 178;

    micros_t request_start;
    micros_t all_start;
    uint32_t worst_runtime;
    uint32_t best_runtime;
};

const char* get_modbus_result_code_name(Modbus::ResultCode event) _ATTRIBUTE((const));

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
