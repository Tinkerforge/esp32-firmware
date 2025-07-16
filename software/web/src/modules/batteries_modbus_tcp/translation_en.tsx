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
            "execute": "Execute",
            "execute_response": "Response",
            "execute_waiting": "Waiting...",
            "execute_done": "Done",
            "execute_error_prefix": "Error: ",
            "display_name": "Display name",
            "host": "Host",
            "host_invalid": "Host is invalid",
            "port": "Port",
            "port_muted": "typically 502",
            "table": "Register table",
            "table_custom": "Custom",
            "device_address": "Device address",
            "permit_grid_charge": "Permit grid charge",
            "revoke_grid_charge_override": "Revoke grid charge permission",
            "forbid_discharge": "Forbid discharge",
            "revoke_discharge_override": "Revoke discharge forbiddance",
            "forbid_charge": "Forbid charge",
            "revoke_charge_override": "Revoke charge forbiddance",
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
            "register_blocks_start_address": "Start address",
            "register_blocks_start_address_muted": "begins at 0",
            "register_blocks_start_number": "Start number",
            "register_blocks_start_number_muted": "begins at 1",
            "register_blocks_value": "Value",
            "register_blocks_value_muted": "",
            "register_blocks_values": "Values",
            "register_blocks_values_muted": "comma-separated",
            "register_blocks_values_invalid": "The values must be a comma-separated list of decimal numbers.",
            "register_blocks_values_desc": /*SFN*/(func: number, addr: number, values: number[]) => {
                let rtyp = "unknown";

                switch (func) {
                case 5:
                case 15:
                    rtyp = values.length > 1 ? "coils" : "coil";
                    break;

                case 6:
                case 16:
                    rtyp = values.length > 1 ? "registers" : "register";
                    break;
                }

                return "Set " + rtyp + (values.length > 1 ? " from " : " ") + addr + " to " + values.join(", ");
            }/*NF*/
        },
        "script": {
        }
    }
}
