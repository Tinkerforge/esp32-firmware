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
            "test_mode": "Für Speichermodus",
            "test_start": "Starten",
            "test_stop": /*SFN*/(mode: string) => mode ? "Stoppen: " + mode : "Stoppen"/*NF*/,
            "display_name": "Anzeigename",
            "host": "Host",
            "host_invalid": "Host ist ungültig",
            "port": "Port",
            "port_muted": "typischerweise 502",
            "table": "Registertabelle",
            "table_custom": "Benutzerdefiniert",
            "table_victron_energy_gx": "Victron Energy GX",
            "table_deye_hybrid_inverter": "Deye Hybrid-Wechselrichter",
            "table_alpha_ess_hybrid_inverter": "Alpha ESS Hybrid-Wechselrichter",
            "table_hailei_hybrid_inverter": "Hailei Hybrid-Wechselrichter (TPH-Serie)",
            "table_sungrow_hybrid_inverter": "Sungrow Hybrid-Wechselrichter (SH-Serie)",
            "table_sma_hybrid_inverter": "SMA Hybrid-Wechselrichter",
            "sma_forecast_based_charging_warning": <>Das <a href="https://manuals.sma.de/HM-20/de-DE/8644254859.html">SMA prognosebasierte Batterieladen</a> kann die Batteriesteuerung beinträchtigen. Bitte sicherstellen, dass das prognosebasierte Batterieladen im SMA Portal deaktiviert ist.</>,
            "device_address": "Geräteadresse",
            "device_address_muted": /*SFN*/(device_address: number) => "typischerweise " + device_address/*NF*/,
            "repeat_interval": "Wiederholungs-Interval",
            "repeat_interval_muted": "0 deaktivert Wiederholung",
            "battery_mode_block": "Laden blockieren, Entladen blockieren",
            "battery_mode_normal": "Normal Laden, normal Entladen",
            "battery_mode_charge_from_excess": "Normal Laden, Entladen blockieren",
            "battery_mode_charge_from_grid": "Laden erzwingen, Entladen blockieren",
            "battery_mode_discharge_to_load": "Laden blockieren, normal Entladen",
            "battery_mode_discharge_to_grid": "Laden blockieren, Entladen erzwingen",
            "grid_draw_setpoint": "Sollwert Netzbezug",
            "grid_draw_setpoint_muted_normal": "normal",
            "grid_draw_setpoint_muted_force_charge": "Laden erzwungen",
            "grid_draw_setpoint_muted_force_discharge": "Entladen erzwungen",
            "force_charge_current": "Erzwungener Ladestrom",
            "force_discharge_current": "Erzwungener Entladestrom",
            "force_charge_power": "Erzwungene Ladeleistung",
            "force_discharge_power": "Erzwungene Entladeleistung",
            "max_normal_charge_current": "Maximaler normaler Ladestrom",
            "max_normal_discharge_current": "Maximaler normaler Entladestrom",
            "max_normal_charge_power": "Maximale normale Ladeleistung",
            "max_normal_discharge_power": "Maximale normale Entladeleistung",
            "register_title": /*SFN*/(mode: string) => "Speichermodus: " + mode/*NF*/,
            "register_address_mode": "Adressmodus",
            "register_address_mode_address": "Registeradresse (beginnt bei 0)",
            "register_address_mode_number": "Registernummer (beginnt bei 1)",
            "register_blocks": "Registerblöcke",
            "register_blocks_add_title": "Registerblock hinzufügen",
            "register_blocks_add_message_register_blocks": /*SFN*/(have: number, max: number) => `${have} von ${max} Registerblöcken konfiguriert`/*NF*/,
            "register_blocks_add_message_total_values": /*SFN*/(have: number, max: number) => `${have} von ${max} Werten konfiguriert`/*NF*/,
            "register_blocks_add_select_address_mode": "Zuerst Adressmodus auswählen...",
            "register_blocks_edit_title": "Registerblock bearbeiten",
            "register_blocks_desc": "Beschreibung",
            "register_blocks_desc_muted": "optional",
            "register_blocks_function_code": "Funktionscode",
            "register_blocks_function_code_write_single_coil": "5 – Einzelne Coil schreiben",
            "register_blocks_function_code_write_single_register": "6 – Einzelnes Register schreiben",
            "register_blocks_function_code_write_multiple_coils": "15 – Mehrere Coils schreiben",
            "register_blocks_function_code_write_multiple_registers": "16 – Mehrere Register schreiben",
            "register_blocks_function_code_mask_write_register": "22 – Register maskiert schreiben",
            "register_blocks_function_code_read_mask_write_single_register": "3 + 6 – Einzelnes Register lesen und maskiert schreiben",
            "register_blocks_function_code_read_mask_write_multiple_registers": "3 + 16 – Mehrere Register lesen und maskiert schreiben",
            "register_blocks_start_address": "Startadresse",
            "register_blocks_start_address_muted": "beginnt bei 0",
            "register_blocks_start_number": "Startnummer",
            "register_blocks_start_number_muted": "beginnt bei 1",
            "register_blocks_value": "Wert",
            "register_blocks_value_muted": "",
            "register_blocks_value_invalid": "Der Wert muss eine Dezimalzahl sein.",
            "register_blocks_values": "Werte",
            "register_blocks_values_muted": "kommagetrennt",
            "register_blocks_values_invalid": "Die Werte müssen eine kommagetrennte Liste von Dezimalzahlen sein.",
            "register_blocks_mask": "Bitmaske",
            "register_blocks_mask_muted": "",
            "register_blocks_mask_invalid": "Die Bitmaske muss eine Folge von 0, 1 oder x sein.",
            "register_blocks_masks": "Bitmasken",
            "register_blocks_masks_muted": "kommagetrennt",
            "register_blocks_masks_invalid": "Die Bitmasken müssen eine kommagetrennte Liste von Folgen von 0, 1 oder x sein.",
            "register_blocks_values_desc": /*SFN*/(func: number, addr: number, values: string) => {
                let rtyp = "Unbekannt";

                switch (func) {
                case 5:
                case 15:
                    rtyp = values.indexOf(",") >= 0 ? "Coils" : "Coil";
                    break;

                case 6:
                case 16:
                case 22:
                case 9:
                case 19:
                    rtyp = "Register";
                    break;
                }

                return "Setze " + (values.indexOf(",") >= 0 ? "ab" : "") + " " + rtyp + " " + addr + " auf " + values;
            }/*NF*/
        },
        "script": {
        }
    }
}
