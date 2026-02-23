/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "eebus": {
        "content": {
            "enable_eebus": "EEBUS aktivieren",
            "enable_eebus_desc": "Erlaubt die Kommunikation mit EEBUS Geräten",
            "enable_eebus_help_intro": <>
                <p>EEBUS ist ein Kommunikationsstandard für intelligentes Energiemanagement. Er ermöglicht die Kommunikation
                zwischen Energiegeräten wie Wallboxen, Wärmepumpen, PV-Anlagen und Energiemanagementsystemen (HEMS).</p>
            </>,
            "enable_eebus_help_usecases_intro": "Folgende Use Cases werden unterstützt:",
            "usecase_lpc_desc": "Begrenzung der Leistungsaufnahme. Wird von Steuerboxen zur Netzsteuerung gemäß §14a EnWG verwendet. Ermöglicht dem Netzbetreiber die Leistungsaufnahme von Großverbrauchern zu begrenzen zu begrenzen.",
            "usecase_lpp_desc": "Begrenzung der Leistungsabgabe. Ermöglicht dem Netzbetreiber die Begrenzung der Netzeinspeisung.",
            "usecase_mpc_desc": "Überwachung des Stromverbrauchs. Stellt Leistungs- und Energiemesswerte für externe Systeme bereit.",
            "usecase_mgcp_desc": "Netzanschlusspunkt-Überwachung. Überwacht den Leistungsfluss am Netzanschlusspunkt einschließlich Einspeisung und Bezug.",
            "usecase_cevc_desc": "Koordiniertes Laden. Ermöglicht einem Energiemanager die Steuerung des Ladevorgangs basierend auf Ladeplänen und Anreizen.",
            "usecase_evcc_desc": "Fahrzeugkonfiguration. Meldet Informationen über das angeschlossene Elektrofahrzeug wie Kommunikationsstandard und Leistungsgrenzen.",
            "usecase_evcem_desc": "Lademessung. Stellt Messwerte wie Strom, Leistung und geladene Energie bereit.",
            "usecase_evsecc_desc": "Wallbox-Status. Meldet den Betriebszustand der Ladestation und eventuelle Fehlerzustände.",
            "usecase_evcs_desc": "Ladeübersicht. Stellt Zusammenfassungen abgeschlossener Ladevorgänge mit Energiemenge und Kosten bereit.",
            "usecase_opev_desc": "Überlastschutz. Ermöglicht die Begrenzung des Ladestroms.",
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
                "state_connected": "SHIP Aktiv",
                "state_eebus_connected": "EEBUS Aktiv",
                "state_disconnected": "Getrennt",
                "state_discovered": "Entdeckt",
                "state_loaded_from_config": "Aus Konfiguration geladen",
                "autoregister": "Autoregistrierung aktiv",
                "overwrite_notice": "Diese Einstellungen werden überschrieben wenn ein EEBUS Gerät mit der SKI gefunden wird.",
                "persistent": "In Konfiguration gespeichert",
                "persistent_remove_error": "Nur gespeicherte Peers können entfernt werden. Entdeckte Peers können nicht entfernt werden."
            },
            "usecase_details": "Details",
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
