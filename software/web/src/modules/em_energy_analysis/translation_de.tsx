/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "em_energy_analysis": {
        "status": {
            "power_history": "Leistungsverlauf",
            "current_power": "Leistung"
        },
        "navbar": {
            "em_energy_analysis": "Energiebilanz"
        },
        "content": {
            "em_energy_analysis": "Energiebilanz",
            "no_data": "Keine Daten vorhanden!",
            "loading": "Lade Daten...",
            "data_type_5min": "Tag",
            "data_type_daily": "Monat",
            "state_phase": "Phasen",
            "state_input3": "Eingang 3",
            "state_input4": "Eingang 4",
            "state_input_low": "Offen",
            "state_input_high": "Geschlossen",
            "state_relay": "Relais",
            "state_relay_open": "Offen",
            "state_relay_closed": "Geschlossen",
            "state_not_connected": "Getrennt",
            "state_waiting_for_charge_release": "Warte auf Freigabe",
            "state_ready_to_charge": "Ladebereit",
            "state_charging": "Lädt",
            "state_error": "Fehler",
            "state_single_phase": "Einphasig",
            "state_three_phase": "Dreiphasig",
            "import": "Bezug",
            "export": "Einspeisung"
        },
        "script": {
            "time_5min": "Uhrzeit",
            "time_daily": "Datum",
            "power": "Leistung",
            "energy": "Energie",
            "total_energy": "Gesamt-Energie",
            "meter": /*SFN*/(slot: number|"?") => "Zähler #" + slot/*NF*/
        }
    }
}
