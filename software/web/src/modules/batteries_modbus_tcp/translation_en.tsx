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
            "registers": "Registers",
            "registers_add_title": "Add register",
            "registers_add_count": /*SFN*/(x: number, max: number) => x + " of " + max + " registers configured"/*NF*/,
            "registers_add_select_address_mode": "Select address mode first...",
            "registers_edit_title": "Edit value",
            "registers_register_desc": "Description",
            "registers_register_desc_muted": "optional",
            "registers_register_type": "Register type",
            "registers_register_type_holding_register": "Holding register",
            "registers_register_type_holding_register_desc": "holding register",
            "registers_register_type_input_register": "Input register",
            "registers_register_type_coil": "Coil",
            "registers_register_type_coil_desc": "coil",
            "registers_register_type_discrete_input": "Discrete input",
            "registers_start_address": "Start address",
            "registers_start_address_muted": "begins at 0",
            "registers_start_number": "Start number",
            "registers_start_number_muted": "begins at 1",
            "registers_values": "Values",
            "registers_values_muted": "comma-separated",
            "registers_values_invalid": "The values must be a comma-separated list of decimal numbers.",
            "registers_values_desc": /*SFN*/(rtype: string, addr: number, values: number[]) => "Set " + rtype + (values.length > 1 ? " from " : " ") + addr + " to " + values.join(", ")/*NF*/
        },
        "script": {
        }
    }
}
