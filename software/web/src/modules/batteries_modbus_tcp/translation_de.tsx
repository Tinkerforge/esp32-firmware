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
            "forbid_charge": "Laden verbieten",
            "revoke_charge_override": "Verbot zum Laden zurücknehmen",
            "register_address_mode": "Adressmodus",
            "register_address_mode_address": "Registeradresse (beginnt bei 0)",
            "register_address_mode_number": "Registernummer (beginnt bei 1)",
            "register_blocks": "Registerblöcke",
            "register_blocks_add_title": "Registerblock hinzufügen",
            "register_blocks_add_message_register_blocks": /*SFN*/(have: number, max: number) => `${have} von ${max} Registerblöcken konfiguriert`/*NF*/,
            "register_blocks_add_message_total_values": /*SFN*/(have: number, max: number) => `${have} von ${max} Werten konfiguriert`/*NF*/,
            "register_blocks_add_select_address_mode": "Zuerst Adressmodus auswählen...",
            "register_blocks_edit_title": "Registerblock bearbeiten",
            "register_blocks_register_desc": "Beschreibung",
            "register_blocks_register_desc_muted": "optional",
            "register_blocks_register_type": "Registertyp",
            "register_blocks_register_type_holding_register": "Holding Register",
            "register_blocks_register_type_holding_register_desc": "Holding Register",
            "register_blocks_register_type_input_register": "Input Register",
            "register_blocks_register_type_coil": "Coil",
            "register_blocks_register_type_coil_desc": "Coil",
            "register_blocks_register_type_discrete_input": "Discrete Input",
            "register_blocks_start_address": "Startadresse",
            "register_blocks_start_address_muted": "beginnt bei 0",
            "register_blocks_start_number": "Startnummer",
            "register_blocks_start_number_muted": "beginnt bei 1",
            "register_blocks_values": "Werte",
            "register_blocks_values_muted": "kommagetrennt",
            "register_blocks_values_invalid": "Die Werte müssen eine kommagetrennte Liste von Dezimalzahlen sein.",
            "register_blocks_values_desc": /*SFN*/(rtype: string, addr: number, values: number[]) => "Setze " + rtype + (values.length > 1 ? " ab " : " ") + addr + " auf " + values.join(", ")/*NF*/
        },
        "script": {
        }
    }
}
