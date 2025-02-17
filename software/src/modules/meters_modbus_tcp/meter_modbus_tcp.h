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

#include "modules/modbus_tcp_client/generic_modbus_tcp_client.h"
#include "modules/meters/imeter.h"
#include "modules/meters/meter_value_id.h"
#include "config.h"
#include "meters_modbus_tcp.h"
#include "modules/modbus_tcp_client/modbus_register_type.enum.h"
#include "modules/modbus_tcp_client/modbus_value_type.enum.h"
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
#include "solax_hybrid_inverter_virtual_meter.enum.h"
#include "fronius_gen24_plus_hybrid_inverter_virtual_meter.enum.h"
#include "hailei_hybrid_inverter_virtual_meter.enum.h"
#include "fox_ess_h3_hybrid_inverter_virtual_meter.enum.h"
#include "carlo_gavazzi_phase.enum.h"
#include "carlo_gavazzi_em270_virtual_meter.enum.h"
#include "carlo_gavazzi_em280_virtual_meter.enum.h"

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
        bool drop_sign;
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

    MeterModbusTCP(uint32_t slot_, Config *state_, Config *errors_, TFModbusTCPClientPool *pool_, size_t trace_buffer_index_) :
        GenericModbusTCPClient("meters_mbtcp", pool_), slot(slot_), state(state_), errors(errors_), trace_buffer_index(trace_buffer_index_) {}

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
    bool is_deye_hybrid_inverter_pv_meter() const;
    bool is_shelly_pro_xem_monophase() const;
    bool is_fronius_gen24_plus_hybrid_inverter_battery_meter() const;
    bool is_carlo_gavazzi_em100_or_et100() const;
    bool is_carlo_gavazzi_em510() const;

    uint32_t slot;
    Config *state;
    Config *errors;
    size_t trace_buffer_index;

    MeterModbusTCPTableID table_id;
    const ValueTable *table = nullptr;

    bool read_allowed = false;
    bool values_declared = false;
    size_t read_index = 0;
    size_t max_register_count = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

    uint16_t register_buffer[METER_MODBUS_TCP_REGISTER_BUFFER_SIZE];
    size_t register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;
    size_t register_start_address;

    union {
        // custom
        struct {
            ValueTable *table;
        } custom;

        // Sungrow
        struct {
            SungrowHybridInverterVirtualMeter hybrid_inverter_virtual_meter;
            SungrowStringInverterVirtualMeter string_inverter_virtual_meter;
            int inverter_output_type;
            uint16_t hybrid_inverter_running_state;
        } sungrow;

        // Solarmax
        struct {
            SolarmaxMaxStorageVirtualMeter max_storage_virtual_meter;
        } solarmax;

        // Victron Energy GX
        struct {
            VictronEnergyGXVirtualMeter virtual_meter;
            float ac_coupled_pv_on_output_l1_power;
            float ac_coupled_pv_on_output_l2_power;
            float ac_coupled_pv_on_output_l3_power;
            float ac_consumption_l1_power;
            float ac_consumption_l2_power;
            float ac_consumption_l3_power;
        } victron_energy_gx;

        // Deye
        struct {
            DeyeHybridInverterVirtualMeter hybrid_inverter_virtual_meter;
            int hybrid_inverter_device_type;
            float hybrid_inverter_pv1_power;
            float hybrid_inverter_pv2_power;
            float hybrid_inverter_pv3_power;
            float hybrid_inverter_pv4_power;
            float hybrid_inverter_pv1_voltage;
            float hybrid_inverter_pv1_current;
            float hybrid_inverter_pv2_voltage;
            float hybrid_inverter_pv2_current;
            float hybrid_inverter_pv3_voltage;
            float hybrid_inverter_pv3_current;
            float hybrid_inverter_pv4_voltage;
            float hybrid_inverter_pv4_current;
        } deye;

        // Alpha ESS
        struct {
            AlphaESSHybridInverterVirtualMeter hybrid_inverter_virtual_meter;
        } alpha_ess;

        // Shelly Pro EM
        struct {
            ShellyEMMonophaseChannel monophase_channel;
            ShellyEMMonophaseMapping monophase_mapping;
        } shelly_pro_em;

        // Shelly Pro 3EM
        struct {
            ShellyPro3EMDeviceProfile device_profile;
            ShellyEMMonophaseChannel monophase_channel;
            ShellyEMMonophaseMapping monophase_mapping;
        } shelly_pro_3em;

        // Goodwe
        struct {
            GoodweHybridInverterVirtualMeter hybrid_inverter_virtual_meter;
        } goodwe;

        // Solax
        struct {
            SolaxHybridInverterVirtualMeter hybrid_inverter_virtual_meter;
        } solax;

        // Fronius
        struct {
            FroniusGEN24PlusHybridInverterVirtualMeter gen24_plus_hybrid_inverter_virtual_meter;
            uint16_t gen24_plus_hybrid_inverter_input_or_model_id;
            size_t gen24_plus_hybrid_inverter_start_address_shift;
            int16_t gen24_plus_hybrid_inverter_dca_sf;
            int16_t gen24_plus_hybrid_inverter_dcv_sf;
            int16_t gen24_plus_hybrid_inverter_dcw_sf;
            int16_t gen24_plus_hybrid_inverter_dcwh_sf;
            float gen24_plus_hybrid_inverter_charge_dca;
            float gen24_plus_hybrid_inverter_charge_dcv;
            float gen24_plus_hybrid_inverter_charge_dcw;
            float gen24_plus_hybrid_inverter_charge_dcwh;
            float gen24_plus_hybrid_inverter_discharge_dca;
            float gen24_plus_hybrid_inverter_discharge_dcv;
            float gen24_plus_hybrid_inverter_discharge_dcw;
            float gen24_plus_hybrid_inverter_discharge_dcwh;
            float gen24_plus_hybrid_inverter_chastate;
            int16_t gen24_plus_hybrid_inverter_chastate_sf;
        } fronius;

        // Hailei
        struct {
            HaileiHybridInverterVirtualMeter hybrid_inverter_virtual_meter;
        } hailei;

        // Fox ESS H3
        struct {
            FoxESSH3HybridInverterVirtualMeter hybrid_inverter_virtual_meter;
        } fox_ess_h3;

        // Carlo Gavazzi EM100
        struct {
            CarloGavazziPhase phase;
        } carlo_gavazzi_em100;

        // Carlo Gavazzi ET100
        struct {
            CarloGavazziPhase phase;
        } carlo_gavazzi_et100;

        // Carlo Gavazzi EM270
        struct {
            CarloGavazziEM270VirtualMeter virtual_meter;
        } carlo_gavazzi_em270;

        // Carlo Gavazzi EM280
        struct {
            CarloGavazziEM280VirtualMeter virtual_meter;
        } carlo_gavazzi_em280;

        // Carlo Gavazzi EM510
        struct {
            CarloGavazziPhase phase;
        } carlo_gavazzi_em510;
    };
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
