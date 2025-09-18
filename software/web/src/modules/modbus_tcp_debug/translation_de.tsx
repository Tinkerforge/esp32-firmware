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
            "meter": "Stromzähler",
            "meter_none": "Kein Stromzähler",
            "host": "Hostname oder IP-Adresse",
            "port": "Port",
            "port_muted": "typischerweise 502",
            "device_address": "Geräteadresse",
            "function_code": "Funktionscode",
            "function_code_read_coils": "1 – Coils lesen",
            "function_code_read_discrete_inputs": "2 – Discrete Inputs lesen",
            "function_code_read_holding_registers": "3 – Holding Register lesen",
            "function_code_read_input_registers": "4 – Input Register lesen",
            "function_code_write_single_register": "6 – Einzelnes Register schreiben",
            "function_code_write_multiple_registers": "16 – Mehrere Register schreiben",
            "function_code_mask_write_register": "22 – Register maskiert schreiben",
            "function_code_read_mask_write_single_register": "3 + 6 – Einzelnes Register lesen und maskiert schreiben",
            "function_code_read_mask_write_multiple_register": "3 + 16 – Mehrere Register lesen und maskiert schreiben",
            "register_address_mode": "Adressmodus",
            "register_address_mode_address": "Registeradresse (beginnt bei 0)",
            "register_address_mode_number": "Registernummer (beginnt bei 1)",
            "start_address_dec": "Startadresse (dezimal)",
            "start_address_hex": "Startadresse (hexadezimal)",
            "start_address_hex_invalid": "Muss aus 1 bis 4 hexadezimale Ziffern bestehen",
            "start_number_dec": "Startnummer (dezimal)",
            "start_number_hex": "Startnummer (hexadezimal)",
            "start_number_hex_invalid": "Muss aus 1 bis 4 hexadezimale Ziffern bestehen",
            "data_count": "Anzahl",
            "write_data_single_value": "Wert",
            "write_data_single_value_invalid": "Der Wert muss eine Dezimalzahl sein.",
            "write_data_multiple_values": "Werte",
            "write_data_multiple_values_muted": "kommagetrennt",
            "write_data_multiple_values_invalid": "Die Werte müssen eine kommagetrennte Liste von Dezimalzahlen sein.",
            "write_data_single_mask": "Bitmaske",
            "write_data_single_mask_invalid": "Die Bitmaske muss eine Folge von 0, 1 oder x sein.",
            "write_data_multiple_masks": "Bitmasken",
            "write_data_multiple_masks_muted": "kommagetrennt",
            "write_data_multiple_masks_invalid": "Die Bitmasken müssen eine kommagetrennte Liste von Folgen von 0, 1 oder x sein.",
            "transact_timeout": "Ausführungs-Timeout",
            "execute": "Ausführen",
            "result": "Ergebnis",
            "transfer": "Übertragung"
        },
        "script": {
        }
    }
}
