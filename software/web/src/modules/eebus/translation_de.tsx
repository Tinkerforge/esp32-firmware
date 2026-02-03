/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "eebus": {
        "content": {
            "enable_eebus": "EEBUS aktivieren",
            "enable_eebus_desc": "Erlaubt die Kommunikation mit EEBUS Geräten",
            "enable_eebus_help": <>
                <p>EEBUS ist ein Kommunikationsstandard für intelligentes Energiemanagement. Er ermöglicht die Kommunikation
                zwischen Energiegeräten wie Wallboxen, Wärmepumpen, PV-Anlagen und Energiemanagementsystemen (HEMS).</p>
                <p>Folgende Use Cases werden unterstützt:</p>
                <ul>
                    <li><b>LPC (Limitation of Power Consumption)</b>: Begrenzung der Leistungsaufnahme. Wird von Steuerboxen
                    zur Netzsteuerung gemäß §14a EnWG verwendet. Ermöglicht dem Netzbetreiber die Ladeleistung zu begrenzen.</li>
                    <li><b>MPC (Monitoring of Power Consumption)</b>: Überwachung des Stromverbrauchs. Stellt Leistungs- und
                    Energiemesswerte für externe Systeme bereit.</li>
                    <li><b>CEVC (Coordinated EV Charging)</b>: Koordiniertes Laden. Ermöglicht einem Energiemanager die
                    Steuerung des Ladevorgangs basierend auf Ladeplänen und Anreizen.</li>
                    <li><b>EVCC (EV Commissioning and Configuration)</b>: Fahrzeugkonfiguration. Meldet Informationen über
                    das angeschlossene Elektrofahrzeug wie Kommunikationsstandard und Leistungsgrenzen.</li>
                    <li><b>EVCEM (EV Charging Electricity Measurement)</b>: Lademessung. Stellt Messwerte wie Strom,
                    Leistung und geladene Energie pro Phase bereit.</li>
                    <li><b>EVSECC (EVSE Commissioning and Configuration)</b>: Wallbox-Status. Meldet den Betriebszustand
                    der Ladestation und eventuelle Fehlerzustände.</li>
                    <li><b>EVCS (EV Charging Summary)</b>: Ladeübersicht. Stellt Zusammenfassungen abgeschlossener
                    Ladevorgänge mit Energiemenge und Kosten bereit.</li>
                </ul>
            </>,
            "cert": "Zertifikat",
            "key": "Key",
            "ski": "SKI",
            "ski_muted": "Subject Key Identifier dieses Geräts",
            "ski_help": <>
                <p>Der Subject Key Identifier (SKI) ist ein eindeutiger kryptographischer Bezeichner für dieses Gerät.</p>
                <p>Wenn dieses Gerät mit z.B. einer Steuerbox kommunizieren soll, kann es sein dass der SKI bei der Einrichtung
                der Steuerbox benötigt wird.</p>
            </>,
            "no_cert": "Nicht verwendet",
            "unknown": "Unbekannt",
            "search_peers": "Suche",
            "searching_peers": "Suche im Gange...",
            "search_completed": "Neue Suche",
            "search_failed": "Suche fehlgeschlagen",
            "add_peers": "Gerät Hinzufügen",
            "peer_info": {
                "peers": "EEBUS-Partner",
                "dns_name": "DNS Adresse",
                "device_ip": "Adressen",
                "device_port": "Port",
                "device_trusted": "Verbindung erlauben",
                "model_brand": "Hersteller",
                "model_model": "Gerät",
                "model_type": "Gerätetyp",
                "trusted_no": "Nein",
                "trusted_yes": "Ja",
                "edit_peer_title": "Bearbeiten",
                "wss_path": "WSS Pfad",
                "state": "Status",
                "state_connected": "Verbunden",
                "state_eebus_connected": "EEBUS Aktiv",
                "state_disconnected": "Getrennt",
                "state_discovered": "Entdeckt",
                "state_loaded_from_config": "Aus Konfiguration geladen",
                "autoregister": "Autoregistrierung aktiv",
                "overwrite_notice": "Diese Einstellungen werden überschrieben wenn ein EEBUS Gerät mit der SKI gefunden wird.",
                "persistent": "In Konfiguration gespeichert",
                "persistent_remove_error": "Nur gespeicherte Peers können entfernt werden. Entdeckte Peers können nicht entfernt werden."
            },
            "show_usecase_details": "EEBUS Details anzeigen",
            "yes": "Ja",
            "no": "Nein",
            "devices": "Partner",
            "connected": "Verbunden",
            "discovered": "Entdeckt",
            "commands": "Befehle",
            "received": "Empfangen",
            "sent": "Gesendet",
            "usecases": "Use Cases"
        },
        "navbar": {
            "eebus": "EEBUS"
        },
        "status": {
            "peer_degraded": "Peer Degradiert",
            "lpc_failsafe": "LPC Failsafe",
            "evse_failure": "EVSE Fehler",
            "heartbeat_timeout": "Heartbeat Timeout",
            "discovery_failed": "Suche Fehlgeschlagen"
        },
        "script": {
            "save_failed": "Speichern der EEBUS-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "EEBUS-Einstellungen"
        }
    }
}
