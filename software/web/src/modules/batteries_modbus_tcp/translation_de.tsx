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
            "execute": "Ausführen",
            "execute_response": "Antwort",
            "execute_waiting": "Warte...",
            "execute_done": "Fertig",
            "execute_error_prefix": "Fehler: ",
            "display_name": "Anzeigename",
            "host": "Host",
            "host_invalid": "Host ist ungültig",
            "port": "Port",
            "port_muted": "typischerweise 502",
            "table": "Registertabelle",
            "table_custom": "Benutzerdefiniert",
            "device_address": "Geräteadresse",
            "permit_grid_charge": "Laden vom Netz erlauben",
            "revoke_grid_charge_override": "Erlaubnis zum Laden vom Netz zurücknehmen",
            "forbid_discharge": "Entladen verbieten",
            "revoke_discharge_override": "Verbot zum Entladen zurücknehmen",
            "register_address_mode": "Adressmodus",
            "register_address_mode_address": "Registeradresse (beginnt bei 0)",
            "register_address_mode_number": "Registernummer (beginnt bei 1)",
            "registers": "Register",
            "registers_add_title": "Register hinzufügen",
            "registers_add_count": /*SFN*/(x: number, max: number) => x + " von " + max + " Registern konfiguriert"/*NF*/,
            "registers_add_select_address_mode": "Zuerst Adressmodus auswählen...",
            "registers_edit_title": "Register bearbeiten",
            "registers_register_desc": "Beschreibung",
            "registers_register_desc_muted": "optional",
            "registers_register_type": "Registertyp",
            "registers_register_type_holding_register": "Holding Register",
            "registers_register_type_holding_register_desc": "Holding Register",
            "registers_register_type_input_register": "Input Register",
            "registers_register_type_coil": "Coil",
            "registers_register_type_coil_desc": "Coil",
            "registers_register_type_discrete_input": "Discrete Input",
            "registers_start_address": "Startadresse",
            "registers_start_address_muted": "beginnt bei 0",
            "registers_start_number": "Startnummer",
            "registers_start_number_muted": "beginnt bei 1",
            "registers_values": "Werte",
            "registers_values_muted": "kommagetrennt",
            "registers_values_invalid": "Die Werte müssen eine kommagetrennte Liste von Dezimalzahlen sein.",
            "registers_values_desc": /*SFN*/(rtype: string, addr: number, values: number[]) => "Setze " + rtype + (values.length > 1 ? " ab " : " ") + addr + " auf " + values.join(", ")/*NF*/
        },
        "script": {
        }
    }
}
