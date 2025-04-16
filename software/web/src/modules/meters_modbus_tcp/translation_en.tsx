/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "meters_modbus_tcp": {
        "status": {
        },
        "navbar": {
        },
        "content": {
            "meter_class": "Modbus/TCP",

            "display_name": "Display name",
            "host": "Hostname or IP address",
            "port": "Port",
            "port_muted": "typically 502 or 1502",
            "table": "Register table",
            "table_custom": "Custom",
            "table_sungrow_hybrid_inverter": "Sungrow hybrid inverter (SH series)",
            "table_sungrow_string_inverter": "Sungrow string inverter (SG series)",
            "table_solarmax_max_storage": "Solarmax Max.Storage",
            "table_victron_energy_gx": "Victron Energy GX",
            "table_deye_hybrid_inverter": "Deye hybrid inverter",
            "table_alpha_ess_hybrid_inverter": "Alpha ESS hybrid inverter",
            "table_shelly_pro_em": "Shelly Pro EM",
            "table_shelly_pro_3em": "Shelly Pro 3EM",
            "table_goodwe_hybrid_inverter": "Goodwe hybrid inverter (EH, ET, BH and BT series)",
            "table_solax_hybrid_inverter": "Solax hybrid inverter",
            "table_fronius_gen24_plus": "Fronius GEN24 Plus",
            "table_hailei_hybrid_inverter": "Hailei hybrid inverter (TPH series)",
            "table_fox_ess_h3_hybrid_inverter": "Fox ESS H3 hybrid inverter",
            "table_siemens_pac2200": "Siemens PAC2200",
            "table_siemens_pac3120": "Siemens PAC3120",
            "table_siemens_pac3200": "Siemens PAC3200",
            "table_siemens_pac3220": "Siemens PAC3220",
            "table_siemens_pac4200": "Siemens PAC4200",
            "table_siemens_pac4220": "Siemens PAC4220",
            "table_carlo_gavazzi_em24_din": "Carlo Gavazzi EM24-DIN",
            "table_carlo_gavazzi_em24_e1": "Carlo Gavazzi EM24-E1",
            "table_carlo_gavazzi_em100": "Carlo Gavazzi EM100",
            "table_carlo_gavazzi_et100": "Carlo Gavazzi ET100",
            "table_carlo_gavazzi_em210": "Carlo Gavazzi EM210",
            "table_carlo_gavazzi_em270": "Carlo Gavazzi EM270",
            "table_carlo_gavazzi_em280": "Carlo Gavazzi EM280",
            "table_carlo_gavazzi_em300": "Carlo Gavazzi EM300",
            "table_carlo_gavazzi_et300": "Carlo Gavazzi ET300",
            "table_carlo_gavazzi_em510": "Carlo Gavazzi EM510",
            "table_carlo_gavazzi_em530": "Carlo Gavazzi EM530",
            "table_carlo_gavazzi_em540": "Carlo Gavazzi EM540",
            "table_solaredge": "Solaredge",
            "table_eastron_sdm630_tcp": "Eastron SDM630 TCP",
            "table_tinkerforge_warp_charger": "Tinkerforge WARP Charger",
            "table_sax_power_home_basic_mode": "SAX Power Home (basic mode)",
            "table_sax_power_home_extended_mode": "SAX Power Home (extended mode)",
            "table_e3dc": "E3/DC",
            "virtual_meter": "Virtual meter",
            "virtual_meter_inverter": "Inverter",
            "virtual_meter_grid": "Grid",
            "virtual_meter_battery": "Battery",
            "virtual_meter_load": "Load",
            "virtual_meter_backup_load": "Backup load",
            "virtual_meter_pv": "PV",
            "virtual_meter_meter": "Meter",
            "virtual_meter_current_transformer_1": "Current transformer 1",
            "virtual_meter_current_transformer_2": "Current transformer 2",
            "virtual_meter_additional_generation": "Additional generation",
            "location": "Location",
            "location_matching": "Matching",
            "location_different": "Different",
            "device_address": "Device address",
            "device_address_muted": /*SFN*/(device_address: number) => "typically " + device_address/*NF*/,
            "shelly_pro_3em_device_profile": "Device profile",
            "shelly_pro_3em_device_profile_triphase": "Triphase",
            "shelly_pro_3em_device_profile_monophase": "Monophase",
            "shelly_em_monophase_channel": "Channel",
            "shelly_em_monophase_channel_1": "1",
            "shelly_em_monophase_channel_2": "2",
            "shelly_em_monophase_channel_3": "3",
            "shelly_em_monophase_mapping": "Mapping",
            "shelly_em_monophase_mapping_l1": "L1",
            "shelly_em_monophase_mapping_l2": "L2",
            "shelly_em_monophase_mapping_l3": "L3",
            "carlo_gavazzi_phase": "Phase",
            "carlo_gavazzi_phase_l1": "L1",
            "carlo_gavazzi_phase_l2": "L2",
            "carlo_gavazzi_phase_l3": "L3",
            "register_address_mode": "Address mode",
            "register_address_mode_address": "Register address (begins at 0)",
            "register_address_mode_number": "Register number (begins at 1)",
            "registers": "Registers",
            "registers_add_title": "Add register",
            "registers_add_count": /*SFN*/(x: number, max: number) => x + " of " + max + " registers configured"/*NF*/,
            "registers_add_select_address_mode": "Select address mode first.",
            "registers_edit_title": "Edit register",
            "registers_register": /*SFN*/(start_address: number, value_id_name: string) => start_address + " as " + value_id_name/*NF*/,
            "registers_register_type": "Register type",
            "registers_register_type_holding_register": "Holding register",
            "registers_register_type_input_register": "Input register",
            "registers_start_address": "Start address",
            "registers_start_address_muted": "begins at 0",
            "registers_start_number": "Start number",
            "registers_start_number_muted": "begins at 1",
            "registers_value_type": "Value type",
            "registers_value_type_u16": "One register, 16-bit, integer, unsigned",
            "registers_value_type_s16": "One register, 16-bit, integer, signed",
            "registers_value_type_u32be": "Two registers, 32-bit, integer, unsigned, big-endian",
            "registers_value_type_u32le": "Two registers, 32-bit, integer, unsigned, little-endian",
            "registers_value_type_s32be": "Two registers, 32-bit, integer, signed, big-endian",
            "registers_value_type_s32le": "Two registers, 32-bit, integer, signed, little-endian",
            "registers_value_type_f32be": "Two registers, 32-bit, float, big-endian",
            "registers_value_type_f32le": "Two registers, 32-bit, float, little-endian",
            "registers_value_type_u64be": "Four registers, 64-bit, integer, unsigned, big-endian",
            "registers_value_type_u64le": "Four registers, 64-bit, integer, unsigned, little-endian",
            "registers_value_type_s64be": "Four registers, 64-bit, integer, signed, big-endian",
            "registers_value_type_s64le": "Four registers, 64-bit, integer, signed, little-endian",
            "registers_value_type_f64be": "Four registers, 64-bit, float, big-endian",
            "registers_value_type_f64le": "Four registers, 64-bit, float, little-endian",
            "registers_offset": "Offset",
            "registers_scale_factor": "Scale factor",
            "registers_value_id": "Value"
        },
        "script": {
        }
    }
}
