/* esp32-firmware
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
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

#include <math.h>

#include "modules/meters/imeter.h"
#include "config.h"
#include "modules/meters/meter_defs.h"
#include "meter_defs.h"
#include "bindings/bricklet_rs485.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    //#include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MetersRS485Bricklet;

class MeterRS485Bricklet final : public IMeter
{
public:
    MeterRS485Bricklet(uint32_t slot_, MetersRS485Bricklet *generator_, TF_RS485* rs485_, Config *state_, Config *errors_, ConfigRoot *sdm630_reset_) : slot(slot_), generator(generator_), rs485(rs485_), state(state_), errors(errors_), sdm630_reset(sdm630_reset_) {}

    // IMeter
    MeterClassID get_class() const override;
    void setup(const Config &ephemeral_config) override;
    void register_urls(const String &base_url) override;
    bool supports_power()         override {return true;}
    bool supports_energy_import() override {return true;}
    bool supports_energy_imexsum()override {return true;}
    bool supports_energy_export() override {return true;}
    bool supports_currents()      override {return true;}
    bool supports_reset()         override {return true;}
    bool reset() override;

    void setupMeter();
    void tick();

    void changeMeterType(size_t supported_meter_idx);

    enum class UserDataDone {
        NOT_DONE,
        DONE,
        ERROR
    };

    struct UserData {
        uint16_t *value_to_write;
        uint8_t expected_request_id;
        UserDataDone done;
    };

private:
    void update_all_values();

    uint32_t slot;
    MetersRS485Bricklet *generator;
    TF_RS485 *rs485;
    Config *state;
    uint8_t type_override;

public:
    Config *errors;
    ConfigRoot *sdm630_reset;
    MeterInfo *meter_in_use = nullptr;

private:
    void cb_read_meter_type(TF_RS485 *rs485, uint8_t request_id, int8_t exception_code, uint16_t *holding_registers, uint16_t holding_registers_length);
    void cb_read_values(TF_RS485 *device, uint8_t request_id, int8_t exception_code, uint16_t *regs, uint16_t regs_len);
    void cb_write_reset(TF_RS485 *device, uint8_t request_id, int8_t exception_code);

    uint32_t value_index_power      = UINT32_MAX;
    uint32_t value_index_energy_rel = UINT32_MAX;
    uint32_t value_index_energy_abs = UINT32_MAX;
    uint32_t value_index_current_l1 = UINT32_MAX;
    uint32_t value_index_voltage_l1 = UINT32_MAX;

    bool meter_change_warning_printed = false;

    const RegRead *getNextRead(bool *trigger_fast_read_done, bool *trigger_slow_read_done);

    size_t modbus_read_state_fast = 0;
    size_t modbus_read_state_slow = 0;
    bool last_read_was_fast = false;

    UserData callback_data;

    bool reset_requested = false;

    uint32_t callback_deadline_ms = 0;
    uint32_t next_read_deadline_ms = 0;

    uint16_t meter_type = METER_TYPE_NONE;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
