/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "modbus_tcp_debug": {
        "status": {
        },
        "navbar": {
            "modbus_tcp": "Modbus/TCP"
        },
        "content": {
            "modbus_tcp": "Modbus/TCP",

            "server_missing": <>__("The_device") verfügt aktuell nicht über einen Modbus/TCP-Server. Dieser wird voraussichtlich mit einer zukünftigen Firmware-Aktualisierung hinzugefügt werden, siehe <a href="https://github.com/Tinker%66orge/esp32-firmware/issues/215">GitHub Issue #215</a>.</>,

            "debug": "Debug-Client",
            "host": "Hostname oder IP-Adresse",
            "port": "Port",
            "port_muted": "typischerweise 502",
            "device_address": "Geräteadresse",
            "function_code": "Funktionscode",
            "function_code_read_holding_registers": "3 – Holding Register lesen",
            "function_code_read_input_registers": "4 – Input Register lesen",
            "start_address": "Startadresse",
            "start_address_muted": "beginnt bei 0",
            "data_count": "Anzahl",
            "transact_timeout": "Ausführungs-Timeout",
            "execute": "Ausführen",
            "response": "Antwort"
        },
        "script": {
        }
    }
}
