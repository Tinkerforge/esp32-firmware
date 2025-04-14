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

            "server_missing": <>__("The_device") verfügt aktuell nicht über einen Modbus/TCP-Server. Dieser wird voraussichtlich mit einer zukünftigen Firmware-Aktualisierung hinzugefügt werden__("modbus_tcp_debug.content.server_missing_github").</>,
            "server_missing_github": <>, siehe <a href={removeUnicodeHacks("https://github.com/Tinkerforge/esp32-firmware/issues/215")}>GitHub Issue #215</a></>,

            "debug": "Debug-Client",
            "host": "Hostname oder IP-Adresse",
            "port": "Port",
            "port_muted": "typischerweise 502",
            "device_address": "Geräteadresse",
            "function_code": "Funktionscode",
            "function_code_read_holding_registers": "3 – Holding Register lesen",
            "function_code_read_input_registers": "4 – Input Register lesen",
            "function_code_write_single_register": "6 – Einzelnes Register schreiben",
            "function_code_write_multiple_registers": "16 – Mehrere Register schreiben",
            "start_address": "Startadresse",
            "start_address_muted": "beginnt bei 0",
            "data_count": "Anzahl",
            "write_data_single_register": "Wert",
            "write_data_single_register_invalid": "Der Wert muss eine Dezimalzahl sein.",
            "write_data_multiple_registers": "Werte",
            "write_data_multiple_registers_muted": "kommagetrennt",
            "write_data_multiple_registers_invalid": "Die Werte müssen eine kommagetrennte Liste von Dezimalzahlen sein.",
            "transact_timeout": "Ausführungs-Timeout",
            "execute": "Ausführen",
            "response": "Antwort"
        },
        "script": {
        }
    }
}
