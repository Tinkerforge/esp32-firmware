/** @jsxImportSource preact */
import { h } from "preact";
import { removeUnicodeHacks } from "../../ts/translation";
let x = {
    "modbus_tcp_debug": {
        "status": {
        },
        "navbar": {
            "modbus_tcp": "Modbus/TCP"
        },
        "content": {
            "modbus_tcp": "Modbus/TCP",

            "server_missing": <>__("The_device") currently has no Modbus/TCP server. Presumably it will be added by a future firmware update__("modbus_tcp_debug.content.server_missing_github").</>,
            "server_missing_github": <>, see <a href={removeUnicodeHacks("https://github.com/Tinkerforge/esp32-firmware/issues/215")}>GitHub issue #215</a></>,

            "debug": "Debug client",
            "host": "Hostname or IP address",
            "port": "Port",
            "port_muted": "typically 502",
            "device_address": "Device address",
            "function_code": "Function code",
            "function_code_read_coils": "1 – Read coils",
            "function_code_read_discrete_inputs": "2 – Read discrete inputs",
            "function_code_read_holding_registers": "3 – Read holding registers",
            "function_code_read_input_registers": "4 – Read input registers",
            "function_code_write_single_register": "6 – Write single register",
            "function_code_write_multiple_registers": "16 – Write multiple registers",
            "function_code_mask_write_register": "22 – Mask write register",
            "register_address_mode": "Address mode",
            "register_address_mode_address": "Register address (begins at 0)",
            "register_address_mode_number": "Register number (begins at 1)",
            "start_address_dec": "Start address (decimal)",
            "start_address_hex": "Start address (hexadecimal)",
            "start_address_hex_invalid": "Must consist of 1 to 4 hexadecimal digits",
            "data_count": "Count",
            "write_data_single_value": "Value",
            "write_data_single_value_invalid": "The value must be a decimal number.",
            "write_data_multiple_values": "Values",
            "write_data_multiple_values_muted": "comma-separated",
            "write_data_multiple_values_invalid": "The values must be a comma-separated list of decimal numbers.",
            "write_data_mask": "Bitmask",
            "write_data_mask_invalid": "The bitmask must be a sequence of 0, 1 or X.",
            "transact_timeout": "Execution timeout",
            "execute": "Execute",
            "response": "Response"
        },
        "script": {
        }
    }
}
