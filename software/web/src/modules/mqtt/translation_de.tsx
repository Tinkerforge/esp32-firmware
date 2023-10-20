/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "mqtt": {
        "status": {
            "connection": "MQTT-Verbindung",
            "not_configured": "Deaktiviert",
            "not_connected": "Getrennt",
            "connected": "Verbunden",
            "error": "Fehler"
        },
        "navbar": {
            "mqtt": "MQTT"
        },
        "content": {
            "mqtt": "MQTT",
            "enable_mqtt_desc": null,
            "enable_mqtt": "MQTT aktiviert",
            "broker_host": "Broker-Hostname oder -IP-Adresse",
            "port": "Broker-Port",
            "port_muted": "typischerweise 1883",
            "username": "Broker-Benutzername",
            "username_muted": "optional",
            "password": "Broker-Passwort",
            "password_muted": "optional",
            "topic": "Überwachtes Topic",
            "payload": "Erwartete Nachricht",
            "retain": "Nachricht Speichern",
            "accept_retain": "Erlaube gespeicherte (retained) Nachrichten",
            "use_topic_prefix": "Topic-Präfix benutzen",
            "use_topic_prefix_muted": "Konfigurierter Topic-Präfix: ",
            "use_topic_prefix_invalid": "Das Topic darf nicht mit dem konfigurierten Topic-Präfix beginnen, es sei denn \"Topic-Präfix benutzen\" ist aktiviert.",
            "topic_prefix": "Topic-Präfix",
            "topic_prefix_muted": "optional",
            "topic_prefix_invalid": "Der Topic-Präfix darf nicht mit $ beginnen, oder ein # oder + enthalten.",
            "client_name": "Client-ID",
            "interval": "Maximales Sendeintervall",
            "interval_muted": "Daten werden nur bei Änderung übertragen.",
            "auto_discovery_mode": "Discovery-Modus",
            "auto_discovery_mode_muted": "Unterstützt automatische Erkennung durch eine Hausautomatisierung.",
            "auto_discovery_mode_disabled": "Deaktiviert",
            "auto_discovery_mode_generic": "Generisch",
            "auto_discovery_mode_homeassistant": "Home Assistant",
            "auto_discovery_prefix": "Discovery-Topic-Präfix",
            "auto_discovery_prefix_invalid": "Der Topic-Präfix darf nicht mit $ beginnen, ein # oder + enthalten oder leer sein.",
            "yes": "Ja",
            "no": "Nein",

            "cron_action_text": /*FFN*/(topic: string, payload: string, retain: boolean) => {
                return <>
                    sende MQTT-Nachricht '<b>{payload}</b>' an Topic '<b>{topic}</b>'{retain ? " und speichere sie." : "."}
                </>
            }/*NF*/,
            "cron_trigger_text": /*FFN*/(topic: string, payload: string, retained: boolean) => {
                return <>
                    Wenn MQTT-Nachricht '<b>{payload}</b>' an Topic '<b>{topic}</b>' empfangen wird {retained ? "(Gespeicherte Nachrichten werden akzeptiert)" : ""} {", "}
                </>
            }/*NF*/,
            "cron_trigger_mqtt": "MQTT-Nachricht empfangen"

        },
        "script": {
            "save_failed": "Speichern der MQTT-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "MQTT-Einstellungen"
        }
    }
}
