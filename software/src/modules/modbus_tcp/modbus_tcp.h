/* esp32-firmware
 * Copyright (C) 2022 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "module.h"
#include "config.h"
#include <TFModbusTCPServer.h>
#include <memory>

#include <TFTools/Option.h>

class ModbusTcp final : public IModule
{
public:
    ModbusTcp();
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;
    void pre_reboot() override;

private:
    void update_regs();
    void update_bender_regs();
    void update_keba_regs();

    void start_server();
    void stop_server();
    void fillCache();

    TFModbusTCPExceptionCode getWarpCoils(uint16_t start_address, uint16_t data_count, uint8_t *data_values);
    TFModbusTCPExceptionCode getWarpDiscreteInputs(uint16_t start_address, uint16_t data_count, uint8_t *data_values);
    TFModbusTCPExceptionCode getWarpInputRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values);
    TFModbusTCPExceptionCode getWarpHoldingRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values);

    TFModbusTCPExceptionCode setWarpCoils(uint16_t start_address, uint16_t data_count, uint8_t *data_values);
    TFModbusTCPExceptionCode setWarpHoldingRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values);

    TFModbusTCPExceptionCode getKebaHoldingRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values);
    TFModbusTCPExceptionCode setKebaHoldingRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values);

    TFModbusTCPExceptionCode getBenderHoldingRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values);
    TFModbusTCPExceptionCode setBenderHoldingRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values);

    ConfigRoot config;

    bool started = false;

    TFModbusTCPServer server;

    struct Cache {
        const Config *evse_state;
        const Config *evse_slots;
        const Config *evse_ll_state;
        const Config *evse_indicator_led;
        const Config *evse_gp_output;
        const Config *evse_hardware_configuration;
        const Config *current_charge;
        const Config *meter_state;
        const Config *meter_values;
        const Config *meter_phases;
        const Config *meter_all_values;
        const Config *power_manager_state;
        const Config *power_manager_external_control;

        char nfc_tag_injection_buffer[28];

        bool has_feature_evse;
        bool has_feature_meter;
        bool has_feature_meter_phases;
        bool has_feature_meter_all_values;
        bool has_feature_charge_tracker;
        bool has_feature_nfc;
        bool has_feature_phase_switch;
    };

    enum RegisterTable {
        WARP,
        BENDER,
        KEBA
    };

    union TwoRegs {
        uint32_t u;
        float f;
        struct {uint16_t upper; uint16_t lower;} regs;
        char chars[4];
    };
    Option<TwoRegs> getWarpInputRegister(uint16_t address, void *ctx);
    Option<TwoRegs> getWarpHoldingRegister(uint16_t address);

    Option<TwoRegs> getKebaHoldingRegister(uint16_t reg);
    TwoRegs getBenderHoldingRegister(uint16_t reg);

    std::unique_ptr<Cache> cache;
    uint64_t tick_task;

    bool send_illegal_data_address = true;
};
