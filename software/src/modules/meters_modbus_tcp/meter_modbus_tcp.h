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

#include "config.h"
#include "modules/meters/imeter.h"
#include "modules/meters/meter_value_id.h"
#include "modules/meters/meter_location.enum.h"
#include "modules/modbus_tcp_client/generic_modbus_tcp_client.h"
#include "modules/modbus_tcp_client/modbus_register_type.enum.h"
#include "modules/modbus_tcp_client/modbus_value_type.enum.h"
#include "meter_modbus_tcp_table_id.enum.h"
#include "meter_modbus_tcp_virtual_meter_enums.h"
#include "shelly_pro_3em_device_profile.enum.h"
#include "shelly_em_monophase_channel.enum.h"
#include "shelly_em_monophase_mapping.enum.h"
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

    struct TableSpec {
        MeterLocation default_location;
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
    bool is_sungrow_hybrid_inverter_meter() const;
    bool is_sungrow_hybrid_inverter_grid_meter() const;
    bool is_sungrow_hybrid_inverter_battery_meter() const;
    bool is_sungrow_hybrid_inverter_pv_meter() const;
    bool is_sungrow_string_inverter_meter() const;
    bool is_sungrow_string_inverter_grid_meter() const;
    bool is_sungrow_string_inverter_pv_meter() const;
    bool is_victron_energy_gx_load_meter() const;
    bool is_victron_energy_gx_pv_meter() const;
    bool is_deye_hybrid_inverter_battery_meter() const;
    bool is_deye_hybrid_inverter_pv_meter() const;
    bool is_alpha_ess_hybrid_inverter_pv_meter() const;
    bool is_shelly_pro_xem_monophase() const;
    bool is_goodwe_hybrid_inverter_battery_meter() const;
    bool is_goodwe_hybrid_inverter_pv_meter() const;
    bool is_solax_hybrid_inverter_pv_meter() const;
    bool is_fronius_gen24_plus_battery_meter() const;
    bool is_hailei_hybrid_inverter_pv_meter() const;
    bool is_fox_ess_h3_ac3_hybrid_inverter_pv_meter() const;
    bool is_carlo_gavazzi_em100_or_et100() const;
    bool is_carlo_gavazzi_em510() const;
    bool is_solaredge_inverter_battery_meter() const;
    bool is_huawei_sun2000_battery_meter() const;
    bool is_huawei_sun2000_pv_meter() const;
    bool is_huawei_sun2000_smart_dongle_battery_meter() const;
    bool is_huawei_emma_load_meter() const;
    bool is_solax_string_inverter_meter() const;
    bool is_solax_string_inverter_pv_meter() const;
    bool is_fox_ess_h3_smart_hybrid_inverter_battery_1_meter() const;
    bool is_fox_ess_h3_smart_hybrid_inverter_battery_2_meter() const;
    bool is_fox_ess_h3_smart_hybrid_inverter_battery_1_and_2_meter() const;
    bool is_fox_ess_h3_smart_hybrid_inverter_pv_meter() const;
    bool is_fox_ess_h3_pro_hybrid_inverter_battery_1_meter() const;
    bool is_fox_ess_h3_pro_hybrid_inverter_battery_2_meter() const;
    bool is_fox_ess_h3_pro_hybrid_inverter_battery_1_and_2_meter() const;
    bool is_fox_ess_h3_pro_hybrid_inverter_pv_meter() const;
    bool is_sma_hybrid_inverter_battery_meter() const;
    bool is_varta_flex_grid_meter() const;
    bool is_varta_flex_battery_meter() const;
    bool is_chisage_ess_hybrid_inverter_pv_meter() const;

    uint32_t slot;
    Config *state;
    Config *errors;
    size_t trace_buffer_index;

    MeterModbusTCPTableID table_id;
    const TableSpec *table = nullptr;

    bool read_allowed = false;
    bool values_declared = false;
    size_t read_index = 0;
    size_t max_register_count = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

    uint16_t register_buffer[METER_MODBUS_TCP_REGISTER_BUFFER_SIZE];
    size_t register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;
    size_t register_start_address;

    union {
        // Sungrow hybrid inverter
        struct {
            SungrowHybridInverterVirtualMeter virtual_meter;
            int output_type;
            uint16_t running_state;
            int16_t battery_current;
            float mppt1_voltage;
            float mppt1_current;
            float mppt2_voltage;
            float mppt2_current;
        } sungrow_hybrid_inverter;

        // Sungrow string inverter
        struct {
            SungrowStringInverterVirtualMeter virtual_meter;
            int output_type;
            float mppt1_voltage;
            float mppt1_current;
            float mppt2_voltage;
            float mppt2_current;
        } sungrow_string_inverter;

        // Solarmax Max Storage
        struct {
            SolarmaxMaxStorageVirtualMeter virtual_meter;
        } solarmax_max_storage;

        // Victron Energy GX
        struct {
            VictronEnergyGXVirtualMeter virtual_meter;
            float ac_consumption_l1;
            float ac_consumption_l2;
            float ac_consumption_l3;
            float ac_coupled_pv_power_on_output_l1;
            float ac_coupled_pv_power_on_output_l2;
            float ac_coupled_pv_power_on_output_l3;
            float ac_coupled_pv_power_on_input_l1;
            float ac_coupled_pv_power_on_input_l2;
            float ac_coupled_pv_power_on_input_l3;
            float ac_coupled_pv_power_on_generator_l1;
            float ac_coupled_pv_power_on_generator_l2;
            float ac_coupled_pv_power_on_generator_l3;
            float dc_coupled_pv_power;
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
            float pv1_voltage;
            float pv1_current;
            float pv1_power;
            float pv2_voltage;
            float pv2_current;
            float pv2_power;
            float pv3_voltage;
            float pv3_current;
            float pv3_power;
            float pv4_voltage;
            float pv4_current;
            float pv4_power;
            float pv5_voltage;
            float pv5_current;
            float pv5_power;
            float pv6_voltage;
            float pv6_current;
            float pv6_power;
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
            int battery_1_mode;
            int battery_2_mode;
            float battery_1_voltage;
            float battery_1_current;
            float battery_1_power;
            float bms_1_pack_temperature;
            float battery_1_capacity;
            float battery_2_voltage;
            float battery_2_current;
            float battery_2_power;
            float bms_2_pack_temperature;
            float battery_2_capacity;
            uint32_t pv_mode;
            float pv1_voltage;
            float pv1_current;
            float pv1_power;
            float pv2_voltage;
            float pv2_current;
            float pv2_power;
            float pv3_voltage;
            float pv3_current;
            float pv3_power;
            float pv4_voltage;
            float pv4_current;
            float pv4_power;
        } goodwe_hybrid_inverter;

        // Solax hybrid inverter
        struct {
            SolaxHybridInverterVirtualMeter virtual_meter;
            float pv1_voltage;
            float pv2_voltage;
            float pv1_current;
            float pv2_current;
            float pv1_power;
            float pv2_power;
        } solax_hybrid_inverter;

        // Fronius GEN24 Plus
        struct {
            FroniusGEN24PlusVirtualMeter virtual_meter;
            int input_id_or_model_id;
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
            float pv1_voltage;
            float pv1_current;
            float pv1_power;
            float pv2_voltage;
            float pv2_current;
            float pv2_power;
            float pv3_voltage;
            float pv3_current;
            float pv3_power;
            float pv4_voltage;
            float pv4_current;
            float pv4_power;
            float pv5_voltage;
            float pv5_current;
            float pv5_power;
            float pv6_voltage;
            float pv6_current;
            float pv6_power;
        } hailei_hybrid_inverter;

        // Fox ESS H3 AC3 hybrid inverter
        struct {
            FoxESSH3AC3HybridInverterVirtualMeter virtual_meter;
            float pv1_voltage;
            float pv1_current;
            float pv1_power;
            float pv2_voltage;
            float pv2_current;
            float pv2_power;
        } fox_ess_h3_ac3_hybrid_inverter;

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

        // SolarEdge inverter
        struct {
            SolaredgeInverterVirtualMeter virtual_meter;
            float battery_1_temperature;
            float battery_1_voltage;
            float battery_1_current;
            float battery_1_power;
            float battery_1_export_energy;
            float battery_1_import_energy;
            float battery_1_state_of_charge;
        } solaredge_inverter;

        // SAX Power Home basic mode
        struct {
            SAXPowerHomeBasicModeVirtualMeter virtual_meter;
        } sax_power_home_basic_mode;

        // SAX Power Home extended mode
        struct {
            SAXPowerHomeExtendedModeVirtualMeter virtual_meter;
        } sax_power_home_extended_mode;

        // E3/DC Hauskraftwerk
        struct {
            E3DCHauskraftwerkVirtualMeter virtual_meter;
        } e3dc_hauskraftwerk;

        // Huawei SUN2000
        struct {
            HuaweiSUN2000VirtualMeter virtual_meter;
            int energy_storage_product_model;
            int number_of_pv_strings;
        } huawei_sun2000;

        // Huawei SUN2000 Smart Dongle
        struct {
            HuaweiSUN2000SmartDongleVirtualMeter virtual_meter;
            int energy_storage_product_model;
        } huawei_sun2000_smart_dongle;

        // Huawei EMMA
        struct {
            HuaweiEMMAVirtualMeter virtual_meter;
        } huawei_emma;

        // Solax string inverter
        struct {
            SolaxStringInverterVirtualMeter virtual_meter;
            float pv1_voltage;
            float pv2_voltage;
            float pv1_current;
            float pv2_current;
            float pv1_power;
            float pv2_power;
            float pv3_voltage;
            float pv3_current;
            float pv3_power;
        } solax_string_inverter;

        // Fox ESS H3 Smart hybrid inverter
        struct {
            FoxESSH3SmartHybridInverterVirtualMeter virtual_meter;
            uint16_t bms_1_connection_status;
            float bms_1_voltage;
            float bms_1_current;
            float bms_1_temperature;
            float bms_1_soc;
            float bms_1_design_energy;
            uint16_t bms_2_connection_status;
            float pv1_voltage;
            float pv1_current;
            float pv2_voltage;
            float pv2_current;
            float pv3_voltage;
            float pv3_current;
            float pv4_voltage;
            float pv4_current;
        } fox_ess_h3_smart_hybrid_inverter;

        // Fox ESS H3 Pro hybrid inverter
        struct {
            FoxESSH3ProHybridInverterVirtualMeter virtual_meter;
            uint16_t bms_1_connection_status;
            float bms_1_voltage;
            float bms_1_current;
            float bms_1_temperature;
            float bms_1_soc;
            float bms_1_design_energy;
            uint16_t bms_2_connection_status;
            float pv1_voltage;
            float pv1_current;
            float pv2_voltage;
            float pv2_current;
            float pv3_voltage;
            float pv3_current;
            float pv4_voltage;
            float pv4_current;
            float pv5_voltage;
            float pv5_current;
            float pv6_voltage;
            float pv6_current;
        } fox_ess_h3_pro_hybrid_inverter;

        // SMA hybrid inverter
        struct {
            SMAHybridInverterVirtualMeter virtual_meter;
            float battery_charge_power;
            float battery_discharge_power;
        } sma_hybrid_inverter;

        // VARTA element
        struct {
            VARTAElementVirtualMeter virtual_meter;
        } varta_element;

        // VARTA flex
        struct {
            VARTAFlexVirtualMeter virtual_meter;
            float grid_power;
            int16_t grid_power_sf;
            float battery_active_power;
            float battery_apparent_power;
            float battery_total_charge_energy;
            int16_t battery_active_power_sf;
            int16_t battery_apparent_power_sf;
            int16_t battery_total_charge_energy_sf;
        } varta_flex;

        // Chisage ESS hybrid inverter
        struct {
            ChisageESSHybridInverterVirtualMeter virtual_meter;
            float pv1_voltage;
            float pv1_current;
            float pv2_voltage;
            float pv2_current;
            float pv1_power;
            float pv2_power;
        } chisage_ess_hybrid_inverter;

        // Huawei Smart Logger 3000
        struct {
            HuaweiSmartLogger3000VirtualMeter virtual_meter;
        } huawei_smart_logger_3000;
    };
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
