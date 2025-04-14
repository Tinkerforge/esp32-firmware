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
            "function_code_read_holding_registers": "3 – Read holding registers",
            "function_code_read_input_registers": "4 – Read input registers",
            "function_code_write_single_register": "6 – Write single register",
            "function_code_write_multiple_registers": "16 – Write multiple registers",
            "start_address": "Start address",
            "start_address_muted": "begins at 0",
            "data_count": "Count",
            "write_data_single_register": "Value",
            "write_data_single_register_invalid": "The values must be a decimal number.",
            "write_data_multiple_registers": "Values",
            "write_data_multiple_registers_muted": "comma-separated",
            "write_data_multiple_registers_invalid": "The values must be a comma separated list of decimal numbers.",
            "transact_timeout": "Execution timeout",
            "execute": "Execute",
            "response": "Response"
        },
        "script": {
        }
    }
}
