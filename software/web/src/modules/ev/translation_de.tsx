/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "ev": {
        "navbar": {
            "ev": "Fahrzeuge"
        },
        "status": {
            "ev": "Fahrzeug",
            "no_ev": "Kein Fahrzeug verbunden",
            "soc": "Ladestand",
            "capacity": "Batteriekapazität",
            "efficiency": "Ladeeffizienz"
        },
        "content": {
            "ev": "Fahrzeuge",
            "ev_profiles": "Fahrzeug-Profile",
            "ev_profiles_desc": "Bekannte Fahrzeuge anhand ihrer MAC-Adresse für Autocharge konfigurieren. Batteriekapazität und Ladeeffizienz werden zur Schätzung des Ladestands verwendet.",
            "table_name": "Anzeigename",
            "table_mac": "MAC-Adresse",
            "table_capacity": "Kapazität",
            "table_efficiency": "Effizienz",
            "add_ev_title": "Fahrzeug-Profil hinzufügen",
            "add_ev_message": /*SFN*/(current: number, max: number) => `${current} von ${max} Fahrzeug-Profilen konfiguriert.`/*NF*/,
            "add_ev_name": "Anzeigename",
            "add_ev_mac": "MAC-Adresse",
            "add_ev_mac_invalid": "Ungültiges MAC-Adressformat.",
            "add_ev_rotating_mac": "Wechselnde MAC-Adresse",
            "add_ev_rotating_mac_desc": "Aktivieren, wenn das Fahrzeug einen Teil seiner MAC-Adresse zwischen Ladevorgängen ändert.",
            "add_ev_rotating_mac_help": "Dies ist bei einigen Fahrzeugen üblich (z.B. VW ID-Modelle). Nur der feste Teil der MAC-Adresse wird zur Identifikation verwendet.",
            "add_ev_capacity": "Batteriekapazität",
            "add_ev_efficiency": "Ladeeffizienz",
            "add_ev_efficiency_help": <><p>Das Laden der Batterie ist nicht 100% effizent. Ein Teil der aufgenommenen Energie wird von der Fahrzeugelektronik verbraucht. Ein weiterer Teil geht bei der Umwandlung von AC/DC im Fahrzeug verloren.</p>
                                        <p>Damit der Ladestand während eines laufenden Ladevorgangs korrekt geschätzt werden kann, muss die Batteriekapazität und die Ladeeffizienz des Fahrzeugs korrekt konfiguriert werden.</p></>,
            "edit_ev_title": "Fahrzeug-Profil bearbeiten",
            "active_ev": "Aktives Fahrzeug",
            "currently_connected": "Aktuell verbunden",
            "active_ev_name": "Anzeigename",
            "active_ev_mac": "MAC-Adresse",
            "active_ev_soc": "Ladestand",
            "active_ev_capacity": "Batteriekapazität",
            "active_ev_efficiency": "Ladeeffizienz",
            "no_active_ev": "Derzeit kein Fahrzeug verbunden.",
            "seen_macs": "Zuletzt gesehene Fahrzeug-MACs",
            "seen_macs_desc": "MAC-Adressen von kürzlich verbundenen Fahrzeugen.",
            "no_seen_macs": "Noch keine MAC-Adressen gesehen.",
            "seen_mac_last_seen": /*SFN*/(time: string) => `Zuletzt gesehen vor ${time}`/*NF*/,
            "seen_mac_charger": /*SFN*/(charger: string) => `Gesehen an Ladecontroller: ${charger}`/*NF*/,
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
