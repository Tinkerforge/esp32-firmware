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
#include "fronius_gen24_plus_virtual_meter.enum.h"
#include "hailei_hybrid_inverter_virtual_meter.enum.h"
#include "fox_ess_h3_hybrid_inverter_virtual_meter.enum.h"
#include "carlo_gavazzi_phase.enum.h"
#include "carlo_gavazzi_em270_virtual_meter.enum.h"
#include "carlo_gavazzi_em280_virtual_meter.enum.h"
#include "solaredge_virtual_meter.enum.h"
#include "sax_power_virtual_meter.enum.h"
#include "e3dc_virtual_meter.enum.h"
#include "huawei_sun2000_virtual_meter.enum.h"

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
        bool f32_negative_max_as_nan;
    };

    MeterModbusTCP(uint32_t slot_, Config *state_, Config *errors_, TFModbusTCPClientPool *pool_, size_t trace_buffer_index_) :
        GenericModbusTCPClient("meters_mbtcp", format_meter_slot(slot_), pool_), slot(slot_), state(state_), errors(errors_), trace_buffer_index(trace_buffer_index_) {}

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
    void parse_next();
    bool is_sungrow_inverter_meter() const;
    bool is_sungrow_grid_meter() const;
    bool is_sungrow_battery_meter() const;
    bool is_victron_energy_gx_inverter_meter() const;
    bool is_victron_energy_gx_load_meter() const;
    bool is_deye_hybrid_inverter_battery_meter() const;
    bool is_deye_hybrid_inverter_pv_meter() const;
    bool is_shelly_pro_xem_monophase() const;
    bool is_goodwe_hybrid_inverter_battery_meter() const;
    bool is_fronius_gen24_plus_battery_meter() const;
    bool is_carlo_gavazzi_em100_or_et100() const;
    bool is_carlo_gavazzi_em510() const;
    bool is_solaredge_battery_meter() const;
    bool is_e3dc_pv_meter() const;
    bool is_huawei_sun2000_battery_meter() const;
    bool is_huawei_sun2000_pv_meter() const;

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

    int sungrow_inverter_output_type;

    union {
        // custom
        struct {
            ValueTable *table;
        } custom;

        // Sungrow hybrid inverter
        struct {
            SungrowHybridInverterVirtualMeter virtual_meter;
            uint16_t running_state;
        } sungrow_hybrid_inverter;

        // Sungrow string inverter
        struct {
            SungrowStringInverterVirtualMeter virtual_meter;
        } sungrow_string_inverter;

        // Solarmax Max Storage
        struct {
            SolarmaxMaxStorageVirtualMeter virtual_meter;
        } solarmax_max_storage;

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

        // Deye hybrid inverter
        struct {
            DeyeHybridInverterVirtualMeter virtual_meter;
            int device_type;
            float pv1_power;
            float pv2_power;
            float pv3_power;
            float pv4_power;
            float pv1_voltage;
            float pv1_current;
            float pv2_voltage;
            float pv2_current;
            float pv3_voltage;
            float pv3_current;
            float pv4_voltage;
            float pv4_current;
        } deye_hybrid_inverter;

        // Alpha ESS hybrid inverter
        struct {
            AlphaESSHybridInverterVirtualMeter virtual_meter;
        } alpha_ess_hybrid_inverter;

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

        // GoodWe hybrid inverter
        struct {
            GoodweHybridInverterVirtualMeter virtual_meter;
            float battery_1_voltage;
            float battery_1_current;
            float battery_1_power;
            uint16_t battery_1_mode;
            float bms_1_pack_temperature;
            float battery_1_capacity;
            float battery_2_voltage;
            float battery_2_current;
            float battery_2_power;
            uint16_t battery_2_mode;
            float bms_2_pack_temperature;
            float battery_2_capacity;
        } goodwe_hybrid_inverter;

        // Solax hybrid inverter
        struct {
            SolaxHybridInverterVirtualMeter virtual_meter;
        } solax_hybrid_inverter;

        // Fronius GEN24 Plus
        struct {
            FroniusGEN24PlusVirtualMeter virtual_meter;
            uint16_t input_id_or_model_id;
            size_t start_address_shift;
            int16_t dca_sf;
            int16_t dcv_sf;
            int16_t dcw_sf;
            int16_t dcwh_sf;
            float charge_dca;
            float charge_dcv;
            float charge_dcw;
            float charge_dcwh;
            float discharge_dca;
            float discharge_dcv;
            float discharge_dcw;
            float discharge_dcwh;
            float chastate;
            int16_t chastate_sf;
        } fronius_gen24_plus;

        // Hailei hybrid inverter
        struct {
            HaileiHybridInverterVirtualMeter virtual_meter;
        } hailei_hybrid_inverter;

        // Fox ESS H3 hybrid inverter
        struct {
            FoxESSH3HybridInverterVirtualMeter virtual_meter;
        } fox_ess_h3_hybrid_inverter;

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

        // SolarEdge
        struct {
            SolaredgeVirtualMeter virtual_meter;
            float battery_1_temperature;
            float battery_1_voltage;
            float battery_1_current;
            float battery_1_power;
            float battery_1_export_energy;
            float battery_1_import_energy;
            float battery_1_state_of_charge;
        } solaredge;

        // SAX Power
        struct {
            SAXPowerVirtualMeter virtual_meter;
        } sax_power;

        // E3/DC
        struct {
            E3DCVirtualMeter virtual_meter;
        } e3dc;

        // Huawei SUN2000
        struct {
            HuaweiSUN2000VirtualMeter virtual_meter;
            int32_t energy_storage_product_model;
            int32_t number_of_pv_strings;
        } huawei_sun2000;
    };
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
