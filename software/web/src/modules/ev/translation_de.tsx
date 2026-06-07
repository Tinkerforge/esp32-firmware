/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "ev": {
        "navbar": {
            "ev": "Elektrofahrzeuge"
        },
        "status": {
            "ev": "Elektrofahrzeug",
            "no_ev": "Kein Fahrzeug verbunden",
            "soc": "Ladezustand",
            "capacity": "Batteriekapazität",
            "efficiency": "Ladeeffizienz"
        },
        "content": {
            "ev": "Elektrofahrzeuge",
            "ev_profiles": "Fahrzeug-Profile",
            "ev_profiles_desc": "Bekannte Elektrofahrzeuge anhand ihrer MAC-Adresse für Autocharge konfigurieren. Batteriekapazität und Ladeeffizienz werden zur Schätzung des Ladezustands verwendet.",
            "table_name": "Name",
            "table_mac": "MAC-Adresse",
            "table_capacity": "Kapazität",
            "table_efficiency": "Effizienz",
            "add_ev_title": "Fahrzeug-Profil hinzufügen",
            "add_ev_message": /*SFN*/(current: number, max: number) => `${current} von ${max} Fahrzeug-Profilen konfiguriert.`/*NF*/,
            "add_ev_name": "Name",
            "add_ev_name_desc": "Ein Name zur Identifikation dieses Fahrzeugs.",
            "add_ev_mac": "MAC-Adresse",
            "add_ev_mac_desc": "MAC-Adresse des Fahrzeugs.",
            "add_ev_mac_invalid": "Ungültiges MAC-Adressformat.",
            "add_ev_rotating_mac": "Wechselnde Adresse",
            "add_ev_rotating_mac_desc": "Aktivieren, wenn das Fahrzeug einen Teil seiner Adresse zwischen Ladevorgängen ändert. Dies ist bei einigen Fahrzeugen üblich (z.B. VW ID-Modelle). Nur der feste Teil der Adresse wird zur Identifikation verwendet.",
            "add_ev_capacity": "Batteriekapazität",
            "add_ev_capacity_desc": "Gesamte Batteriekapazität in kWh.",
            "add_ev_efficiency": "Ladeeffizienz",
            "add_ev_efficiency_desc": "Verhältnis von gespeicherter zu verbrauchter Energie.",
            "edit_ev_title": "Fahrzeug-Profil bearbeiten",
            "active_ev": "Aktives Fahrzeug",
            "currently_connected": "Aktuell verbunden",
            "active_ev_name": "Name",
            "active_ev_mac": "MAC-Adresse",
            "active_ev_soc": "Ladezustand",
            "active_ev_capacity": "Batteriekapazität",
            "active_ev_efficiency": "Ladeeffizienz",
            "no_active_ev": "Kein Fahrzeug derzeit verbunden.",
            "seen_macs": "Zuletzt gesehene Fahrzeug-MACs",
            "seen_macs_desc": "MAC-Adressen von kürzlich verbundenen Fahrzeugen.",
            "no_seen_macs": "Noch keine MAC-Adressen gesehen.",
            "seen_mac_last_seen": /*SFN*/(time: string) => `Zuletzt gesehen vor ${time}`/*NF*/,
            "seen_mac_charger": /*SFN*/(charger: string) => `Gesehen an Ladecontroller „${charger}“`/*NF*/,
            "mac_duplicate": "Diese MAC-Adresse wird bereits von einem anderen Profil verwendet.",
            "mac_already_configured": /*SFN*/(name: string) => `Bereits konfiguriert: ${name}`/*NF*/,
            "add_ev_seen_macs": "Zuletzt gesehene MACs"
        },
        "script": {
            "save_failed": "Speichern der Fahrzeug-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "Fahrzeug-Einstellungen"
        }
    }
}
