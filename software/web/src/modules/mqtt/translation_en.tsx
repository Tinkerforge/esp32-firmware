/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "mqtt": {
        "status": {
            "connection": "MQTT connection",
            "not_configured": "Deactivated",
            "not_connected": "Not connected",
            "connected": "Connected",
            "error": "Error"
        },
        "navbar": {
            "mqtt": "MQTT"
        },
        "content": {
            "mqtt": "MQTT",
            "enable_mqtt_desc": null,
            "enable_mqtt": "MQTT enabled",
            "broker_host": "Broker hostname or IP address",
            "port": "Broker port",
            "port_muted": "typically 1883",
            "username": "Broker username",
            "username_muted": "optional",
            "password": "Broker password",
            "password_muted": "optional",
            "topic_prefix": "Topic prefix",
            "topic_prefix_muted": "optional",
            "topic_prefix_invalid": "The topic prefix can not start with $ or contain a # or +.",
            "client_name": "Client ID",
            "interval": "Maximum send interval",
            "interval_muted": "Messages are only sent if the payload has changed.",
            "auto_discovery_mode": "Discovery mode",
            "auto_discovery_mode_muted": "Support auto discovery by home automation.",
            "auto_discovery_mode_disabled": "Disabled",
            "auto_discovery_mode_generic": "Generic",
            "auto_discovery_mode_homeassistant": "Home Assistant",
            "auto_discovery_prefix": "Discovery topic prefix",
            "auto_discovery_prefix_invalid": "The topic prefix can not start with $, contain a # or +, or be empty."
        },
        "cron": {
            "mqtt": "MQTT",
            "match_all": "Accept all messages",
            "full_topic": "Full Topic",
            "send_topic": "To topic",
            "send_payload": "Message",
            "topic": "Topic",
            "payload": "Message",
            "retain": "Retain message",
            "accept_retain": "Accept retained messages",
            "use_topic_prefix": "Use topic prefix",
            "use_topic_prefix_muted": "The topic prefix is ",
            "use_topic_prefix_invalid": "The topic prefix must not be a part of the topic",
            "cron_action_text": /*FFN*/(topic: string, payload: string, retain: boolean) => {
                if (retain && payload.length == 0) {
                    return <>
                        <b>delete</b> retained messages from topic "<b>{topic}</b>".
                    </>
                }
                return <>
                    send MQTT message "<b>{payload}</b>" to topic "<b>{topic}</b>"{retain ? " and retain it." : "."}
                </>
            }/*NF*/,
            "cron_trigger_text": /*FFN*/(topic: string, payload: string, retained: boolean) => {
                let ret = <></>;
                if (payload.length == 0) {
                    ret = <>If any MQTT message</>;
                } else {
                    ret = <>If  MQTT message "<b>{payload}</b>"</>;
                }
                return <>
                    {ret} is received on topic "<b>{topic}</b>"{retained ? " (Retained messages are accepted)" : ""}{", "}
                </>
            }/*NF*/,
            "cron_trigger_mqtt": "MQTT message received",
            "delete_reatianed_message": "Delete retained message"
        },
        "script": {
            "save_failed": "Failed to save the MQTT settings.",
            "reboot_content_changed": "MQTT settings"
        }
    }
}
