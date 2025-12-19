/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "batteries_modbus_tcp": {
        "status": {
        },
        "navbar": {
        },
        "content": {
            "battery_class": "Modbus/TCP",
            "export_basename_suffix": "-Modbus-TCP",
            "test": "Test",
            "test_muted": "",
            "test_mode": "For battery mode",
            "test_start": "Start",
            "test_stop": /*SFN*/(mode: string) => mode ? "Stop: " + mode : "Stop"/*NF*/,
            "display_name": "Display name",
            "host": "Host",
            "host_invalid": "Host is invalid",
            "port": "Port",
            "port_muted": "typically 502",
            "table": "Register table",
            "table_custom": "Custom",
            "table_victron_energy_gx": "Victron Energy GX",
            "table_deye_hybrid_inverter": "Deye hybrid inverter",
            "table_alpha_ess_hybrid_inverter": "Alpha ESS hybrid inverter",
            "table_hailei_hybrid_inverter": "Hailei hybrid inverter (TPH series)",
            "table_sungrow_hybrid_inverter": "Sungrow hybrid inverter (SH series)",
            "table_sma_hybrid_inverter": "SMA hybrid inverter",
            "sma_forecast_based_charging_warning": <>SMA's own <a href="https://manuals.sma.de/HM-20/en-US/8644254859.html">forecast-based charging</a> might interfere with the battery control. Please ensure that forecast-based charging is disabled in the SMA Sunny Portal.</>,
            "device_address": "Device address",
            "device_address_muted": /*SFN*/(device_address: number) => "typically " + device_address/*NF*/,
            "repeat_interval": "Repeat interval",
            "repeat_interval_muted": "0 disables repeat",
            "battery_mode_block": "Block charge, block discharge",
            "battery_mode_normal": "Charge normally, discharge normally",
            "battery_mode_charge_from_excess": "Charge normally, block discharge",
            "battery_mode_charge_from_grid": "Force charge, block discharge",
            "battery_mode_discharge_to_load": "Block charge, discharge normally",
            "battery_mode_discharge_to_grid": "Block charge, force discharge",
            "grid_draw_setpoint": "Grid draw setpoint",
            "grid_draw_setpoint_muted_normal": "normal",
            "grid_draw_setpoint_muted_force_charge": "force charge",
            "grid_draw_setpoint_muted_force_discharge": "force discharge",
            "force_charge_current": "Force charge current",
            "force_discharge_current": "Force discharge current",
            "force_charge_power": "Force charge power",
            "force_discharge_power": "Force discharge power",
            "max_normal_charge_current": "Maximum normal charge current",
            "max_normal_discharge_current": "Maximum normal discharge current",
            "max_normal_charge_power": "Maximum normal charge power",
            "max_normal_discharge_power": "Maximum normal discharge power",
            "register_title": /*SFN*/(mode: string) => "Battery mode: " + mode/*NF*/,
            "register_address_mode": "Address mode",
            "register_address_mode_address": "Register address (begins at 0)",
            "register_address_mode_number": "Register number (begins at 1)",
            "register_blocks": "Registers blocks",
            "register_blocks_add_title": "Add register block",
            "register_blocks_add_message_register_blocks": /*SFN*/(have: number, max: number) => `${have} of ${max} register blocks configured`/*NF*/,
            "register_blocks_add_message_total_values": /*SFN*/(have: number, max: number) => `${have} of ${max} values configured`/*NF*/,
            "register_blocks_add_select_address_mode": "Select address mode first...",
            "register_blocks_edit_title": "Edit register block",
            "register_blocks_desc": "Description",
            "register_blocks_desc_muted": "optional",
            "register_blocks_function_code": "Function code",
            "register_blocks_function_code_write_single_coil": "5 – Write single coil",
            "register_blocks_function_code_write_single_register": "6 – Write single register",
            "register_blocks_function_code_write_multiple_coils": "15 – Write multiple coils",
            "register_blocks_function_code_write_multiple_registers": "16 – Write multiple registers",
            "register_blocks_function_code_mask_write_register": "22 – Mask write register",
            "register_blocks_function_code_read_mask_write_single_register": "3 + 6 – Read, mask and write single register",
            "register_blocks_function_code_read_mask_write_multiple_registers": "3 + 16 – Read, mask and write multiple registers",
            "register_blocks_start_address": "Start address",
            "register_blocks_start_address_muted": "begins at 0",
            "register_blocks_start_number": "Start number",
            "register_blocks_start_number_muted": "begins at 1",
            "register_blocks_value": "Value",
            "register_blocks_value_muted": "",
            "register_blocks_value_invalid": "The value must be a decimal number.",
            "register_blocks_values": "Values",
            "register_blocks_values_muted": "comma-separated",
            "register_blocks_values_invalid": "The values must be a comma-separated list of decimal numbers.",
            "register_blocks_mask": "Bitmask",
            "register_blocks_mask_muted": "",
            "register_blocks_mask_invalid": "The bitmask must be a sequence of 0, 1 or x.",
            "register_blocks_masks": "Bitmasks",
            "register_blocks_masks_muted": "comma-separated",
            "register_blocks_masks_invalid": "The bitmasks must be a comma-separated list of sequences of 0, 1 or x.",
            "register_blocks_values_desc": /*SFN*/(func: number, addr: number, values: string) => {
                let rtyp = "unknown";

                switch (func) {
                case 5:
                case 15:
                    rtyp = values.indexOf(",") >= 0 ? "coils" : "coil";
                    break;

                case 6:
                case 16:
                case 22:
                case 9:
                case 19:
                    rtyp = values.indexOf(",") >= 0 ? "registers" : "register";
                    break;
                }

                return "Set " + (values.indexOf(",") >= 0 ? "from" : "") + " " + rtyp + " " + addr + " to " + values;
            }/*NF*/
        },
        "script": {
        }
    }
}
