/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "p14a_enwg": {
        "status": {
            "status": "Status",
            "active": "Aktiv",
            "inactive": "Inaktiv",
            "current_limit": "Aktuelles Limit",
            "alert_active": /*SFN*/(limit: number) => "Leistung auf " + limit + " W begrenzt"/*NF*/
        },
        "navbar": {
            "p14a_enwg": "14a EnWG"
        },
        "content": {
            "p14a_enwg": "§14a EnWG",
            "enable_p14a_enwg": "§14a EnWG aktiviert",
            "enable_p14a_enwg_desc": "§14a-EnWG-Steuerung aktivieren",
            "enable_p14a_enwg_help": <>
                <p>§14a des Energiewirtschaftsgesetzes (EnWG) ermöglicht es Netzbetreibern, den Strombezug steuerbarer Verbrauchseinrichtungen (Wärmepumpen, Wallboxen, Batteriespeicher, Klimaanlagen) temporär zu reduzieren, um eine Überlastung des lokalen Netzes zu vermeiden. Die Geräte werden dabei nie vollständig abgeschaltet. Eine Mindestleistung von 4200 W bleibt stets verfügbar.</p>
            </>,
            "source": "Signalquelle",
            "source_shutdown_input": "Abschalteingang der Wallbox",
            "source_input": /*SFN*/(device_name: string) => "Eingang des " + device_name/*NF*/,
            "source_eebus": "EEBUS",
            "source_api": "API",
            "input": "Eingang",
            "limit": "Geräteanzahl",
            "limit_muted": "Anzahl der steuerbaren Verbrauchseinrichtungen",
            "limit_help": <>
                <p>Die Mindestleistung für eine einzelne steuerbare Verbrauchseinrichtung beträgt 4200 W.</p>
                <p>Bei mehreren Geräten, die über ein Energiemanagementsystem (EMS) gesteuert werden, berechnet sich die Mindestleistung wie folgt:</p>
                <p><strong>4200 W + (Anzahl der Geräte - 1) &times; Gleichzeitigkeitsfaktor &times; 4200 W</strong></p>
                <p>Gleichzeitigkeitsfaktoren nach Anzahl der Geräte:</p>
                <ul>
                    <li>2 Geräte: 0,80</li>
                    <li>3 Geräte: 0,75</li>
                    <li>4 Geräte: 0,70</li>
                    <li>5 Geräte: 0,65</li>
                    <li>6 Geräte: 0,60</li>
                    <li>7 Geräte: 0,55</li>
                    <li>8 Geräte: 0,50</li>
                    <li>ab 9 Geräte: 0,45</li>
                </ul>
                <p>Beispiel für 3 Geräte: 4200 W + (3 - 1) &times; 0,75 &times; 4200 W = 10500 W</p>
            </>,
            "device_count_unit": /*SFN*/(n: number, limit: number) => n + (n === 1 ? " Gerät" : " Geräte") + " (" + limit + " W)"/*NF*/,
            "active_on": "Aktiv bei",
            "active_on_close": "Geschlossen",
            "active_on_open": "Geöffnet",
            "this_charger": "Diese Wallbox",
            "this_charger_desc": "§14a-Leistungslimit auf diese Wallbox anwenden",
            "managed_chargers": "Verwaltete Wallboxen",
            "managed_chargers_desc": "§14a-Leistungslimit auf verwaltete Wallboxen anwenden",
            "heating": "Heizung",
            "heating_desc": "§14a-Leistungslimit auf Heizung anwenden",
            "heating_max_power": "Max. Leistung Heizung",
            "heating_max_power_muted": "Maximale Leistungsaufnahme der Heizungsanlage"
        },
        "script": {
            "save_failed": "Speichern der §14a-EnWG-Einstellungen fehlgeschlagen"
        }
    }
}
