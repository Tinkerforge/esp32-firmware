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
            "protocol": "Protokoll",
            "protocol_mqtt": "MQTT (unverschlüsselt)",
            "protocol_mqtts": "MQTTS (MQTT über TLS; verschlüsselt)",
            "protocol_ws": "WS (MQTT über WebSocket; unverschlüsselt)",
            "protocol_wss": "WSS (MQTT über WebSocket Secure; verschlüsselt)",
            "cert": "Server-Zertifikat",
            "client_cert": "Client-Zertifikat",
            "client_key": "Client-Key",
            "use_cert_bundle": "Eingebettetes Zertifikats-Bundle",
            "no_cert": "Nicht verwendet",
            "path": "Broker-Pfad",
            "path_muted": "optional; Teil der MQTT-über-WS-URL hinter dem Hostnamen. Muss mit / beginnen.",
            "path_invalid": "Muss mit / beginnen.",
            "broker_host": "Broker-Hostname oder -IP-Adresse",
            "port": "Broker-Port",
            "port_muted": /*SFN*/ (default_port: number) => `typischerweise ${default_port}` /*NF*/,
            "username": "Broker-Benutzername",
            "username_muted": "optional",
            "password": "Broker-Passwort",
            "password_muted": "optional",
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
            "auto_discovery_prefix_invalid": "Der Topic-Präfix darf nicht mit $ beginnen, ein # oder + enthalten oder leer sein."
        },
        "automation": {
            "mqtt": "Sende MQTT-Nachricht",
            "match_any": "Jede Nachricht wird akzeptiert",
            "send_topic": "An Topic",
            "send_payload": "Nachricht",
            "topic": "Überwachtes Topic",
            "payload": "Erwartete Nachricht",
            "retain": "Nachricht speichern (retained)",
            "accept_retain": "Erlaube gespeicherte (retained) Nachrichten",
            "use_topic_prefix": "Topic-Präfix benutzen",
            "use_topic_prefix_muted": "Konfigurierter Topic-Präfix: ",
            "use_topic_prefix_invalid": "Das Topic darf nicht mit dem konfigurierten Topic-Präfix beginnen, es sei denn \"Topic-Präfix benutzen\" ist aktiviert.",
            "automation_action_text": /*FFN*/(topic: string, payload: string, retain: boolean) => {
                if (retain && payload.length == 0) {
                    return <>
                        <b>lösche</b> gespeicherte Nachrichten von Topic "<b>{topic}</b>".
                    </>
                }
                return <>
                    sende MQTT-Nachricht "<b>{payload}</b>" an Topic "<b>{topic}</b>"{retain ? " und speichere sie." : "."}
                </>
            }/*NF*/,
            "automation_trigger_text": /*FFN*/(topic: string, payload: string, retained: boolean) => {
                let ret;
                if (payload.length == 0) {
                    ret = <>Wenn eine beliebige MQTT-Nachricht</>;
                } else {
                    ret = <>Wenn MQTT-Nachricht "<b>{payload}</b>"</>;
                }
                return <>
                    {ret} an Topic "<b>{topic}</b>" empfangen wird{retained ? " (Gespeicherte Nachrichten werden akzeptiert)" : ""}{", "}
                </>
            }/*NF*/,
            "automation_trigger_mqtt": "MQTT-Nachricht empfangen",
            "delete_reatianed_message": "Gespeicherte Nachricht löschen"
        },
        "script": {
            "save_failed": "Speichern der MQTT-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "MQTT-Einstellungen"
        }
    }
}
