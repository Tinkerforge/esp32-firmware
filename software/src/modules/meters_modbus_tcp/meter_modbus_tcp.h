/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "generic_modbus_tcp_client.h"
#include "modules/meters/imeter.h"
#include "modules/meters/meter_value_id.h"
#include "config.h"
#include "meters_modbus_tcp.h"
#include "modbus_register_type.enum.h"
#include "modbus_value_type.enum.h"
#include "meter_modbus_tcp_table_id.enum.h"
#include "sungrow_hybrid_inverter_virtual_meter.enum.h"
#include "sungrow_string_inverter_virtual_meter.enum.h"
#include "solarmax_max_storage_virtual_meter.enum.h"
#include "victron_energy_gx_virtual_meter.enum.h"
#include "deye_hybrid_inverter_virtual_meter.enum.h"
#include "alpha_ess_hybrid_inverter_virtual_meter.enum.h"
#include "shelly_pro_3em_device_profile.enum.h"
#include "shelly_em_monophase_channel.enum.h"
#include "shelly_em_monophase_mapping.enum.h"
#include "goodwe_hybrid_inverter_virtual_meter.enum.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

#define METER_MODBUS_TCP_REGISTER_BUFFER_SIZE 32

class MeterModbusTCP final : protected GenericModbusTCPClient, public IMeter
{
public:
    struct ValueSpec {
        const char *name;
        ModbusRegisterType register_type;
        size_t start_address;
        ModbusValueType value_type;
        float offset;
        float scale_factor;
    };

    struct ValueTable {
        const ValueSpec *specs;
        size_t specs_length;
        const MeterValueID *ids;
        size_t ids_length;
        const uint32_t *index;
    };

    MeterModbusTCP(uint32_t slot_, Config *state_, Config *errors_, ModbusTCP *modbus_) : GenericModbusTCPClient(modbus_), slot(slot_), state(state_), errors(errors_) {}

    [[gnu::const]] MeterClassID get_class() const override;
    void setup(const Config &ephemeral_config) override;
    void pre_reboot() override;

    bool supports_power()         override {return true;}
    bool supports_energy_import() override {return true;}
    bool supports_energy_imexsum()override {return true;}
    bool supports_energy_export() override {return true;}
    bool supports_currents()      override {return true;}

    void read_done_callback();

private:
    void connect_callback() override;
    void disconnect_callback() override;
    bool prepare_read();
    void read_next();
    bool is_sungrow_inverter_meter() const;
    bool is_sungrow_grid_meter() const;
    bool is_sungrow_battery_meter() const;
    bool is_victron_energy_gx_inverter_meter() const;
    bool is_victron_energy_gx_load_meter() const;
    bool is_deye_hybrid_inverter_battery_meter() const;

    uint32_t slot;
    Config *state;
    Config *errors;

    MeterModbusTCPTableID table_id;
    const ValueTable *table = nullptr;

    bool read_allowed = false;
    bool values_declared = false;
    size_t read_index = 0;

    uint16_t register_buffer[METER_MODBUS_TCP_REGISTER_BUFFER_SIZE];
    size_t register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;
    size_t register_start_address;

    // custom
    ValueTable *custom_table;

    // Sungrow
    SungrowHybridInverterVirtualMeter sungrow_hybrid_inverter_virtual_meter;
    SungrowStringInverterVirtualMeter sungrow_string_inverter_virtual_meter;
    int sungrow_inverter_output_type = -1;
    uint16_t sungrow_hybrid_inverter_running_state;

    // Solarmax
    SolarmaxMaxStorageVirtualMeter solarmax_max_storage_virtual_meter;

    // Victron Energy
    VictronEnergyGXVirtualMeter victron_energy_gx_virtual_meter;
    float victron_energy_gx_ac_coupled_pv_on_output_l1_power;
    float victron_energy_gx_ac_coupled_pv_on_output_l2_power;
    float victron_energy_gx_ac_coupled_pv_on_output_l3_power;
    float victron_energy_gx_ac_consumption_l1_power;
    float victron_energy_gx_ac_consumption_l2_power;
    float victron_energy_gx_ac_consumption_l3_power;

    // Deye
    DeyeHybridInverterVirtualMeter deye_hybrid_inverter_virtual_meter;
    int deye_hybrid_inverter_device_type = -1;

    // Alpha ESS
    AlphaESSHybridInverterVirtualMeter alpha_ess_hybrid_inverter_virtual_meter;

    // Shelly Pro EM
    ShellyEMMonophaseChannel shelly_pro_em_monophase_channel;
    ShellyEMMonophaseMapping shelly_pro_em_monophase_mapping;

    // Shelly Pro 3EM
    ShellyPro3EMDeviceProfile shelly_pro_3em_device_profile;
    ShellyEMMonophaseChannel shelly_pro_3em_monophase_channel;
    ShellyEMMonophaseMapping shelly_pro_3em_monophase_mapping;

    // GoodWe
    GoodweHybridInverterVirtualMeter goodwe_hybrid_inverter_virtual_meter;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
