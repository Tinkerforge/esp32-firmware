/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "ocpp": {
        "status": {
            "ocpp": "OCPP connection",
            "connecting": "Connecting",
            "connected": "Connected",
            "error": "Error",
            "status": "OCPP state",
            "waiting_for_tag": "Waiting for NFC tag ",
            "waiting_for_cable": "Waiting for Cable connection "
        },
        "navbar": {
            "ocpp": "OCPP"
        },
        "content": {
            "ocpp": "OCPP",
            "enable_ocpp": "OCPP enabled",
            "enable_ocpp_desc": "Enables the configured OCPP server to control this charger",
            "endpoint_url": "Endpoint URL",
            "endpoint_url_invalid": "The endpoint URL has to start with either ws:// (unencrypted) or wss:// (encrypted) and must not end with a /",
            "identity": "Charge point identity",
            "enable_auth": "Authorization enabled",
            "enable_auth_desc": "Sends the configured password or 40 character long hex key be to authorized by the OCPP server",
            "pass": "Password or hex key",
            "tls_cert": "TLS Certificate",
            "use_cert_bundle": "Embedded certificate bundle",

            "reset": "Reset OCPP state",
            "reset_muted": "",
            "reset_title": "Reset OCPP state?",
            "reset_title_text": <>This resets the OCPP state completely. <strong>Transaction information not transmitted up to now will be lost!</strong>.</>,
            "abort_reset": "Abort",
            "confirm_reset": "Reset",
            "reset_failed": "Failed to reset OCPP state",

            "charge_point_state": "Charge point state",
            "charge_point_status": "Charge point OCPP status",
            "next_profile_eval": "Next charge profile evaluation",
            "connector_state": "Connector state",
            "connector_status": "Connector OCPP status",
            "tag_id": "Tag ID",
            "parent_tag_id": "Tag group ID",
            "tag_expiry_date": "Tag expiry date",
            "tag_timeout": "Tag timeout",
            "cable_timeout": "Cable timeout",
            "txn_id": "Transaction ID",
            "txn_start_time": "Transaction start time",
            "current": "Charge current in transaction",
            "txn_with_invalid_id": "Transaction with invalid ID",
            "unavailable_requested": "Change to \"unavailable\" requested",
            "message_in_flight_type": "Type of message in flight",
            "message_in_flight_id": "ID of message in flight",
            "message_in_flight_len": "Length of message in flight",
            "message_timeout": "Message timeout",
            "txn_msg_retry_timeout": "Transaction message retry timeout",
            "message_queue_depth": "Messages enqueued",
            "status_queue_depth": "StatusNotifications enqueued",
            "txn_msg_queue_depth": "Transaction messages enqueued",
            "is_connected": "Connection state",
            "last_ping_sent": "Last WS ping sent",
            "pong_timeout": "WS pong timeout",
            "connection_state_since": /*SFN*/ (connected: boolean, date: string) => (connected ? "Connected" : "Not connected") + " since " + date /*NF*/,

            "not_waiting_for_tag": "No tag expected",
            "not_waiting_for_cable": "No cable expected",
            "no_message_in_flight": "No message in flight",
            "no_ping_sent": "No WS ping sent",
            "no_tag_seen": "No tag seen",
            "no_transaction_running": "No transaction running",
            "never_connected_since_reboot": "reboot",

            "charge_point_state_0": "Power on",
            "charge_point_state_1": "Flush persistent messages",
            "charge_point_state_2": "Available",
            "charge_point_state_3": "Server registration pending",
            "charge_point_state_4": "Server registration rejected",
            "charge_point_state_5": "Unavailable",
            "charge_point_state_6": "Faulted",
            "charge_point_state_7": "Soft reset",
            "charge_point_state_8": "Hard reset",

            "status_0": "Available",
            "status_1": "Preparing",
            "status_2": "Charging",
            "status_3": "Suspended by vehicle",
            "status_4": "Suspended by charger",
            "status_5": "Finishing",
            "status_6": "Reserved",
            "status_7": "Unavailable",
            "status_8": "Faulted",
            "status_9": "None",

            "connector_state_0": "Idle",
            "connector_state_1": "Not connected, not authorized",
            "connector_state_2": "Connected, not authorized",
            "connector_state_3": "Not connected, authorizing",
            "connector_state_4": "Plug detected, authorizing",
            "connector_state_5": "Angeschlossen, authorizing",
            "connector_state_6": "Not connected, authorized",
            "connector_state_7": "Plug detected, authorized",
            "connector_state_8": "Transaction in progress",
            "connector_state_9": "Authorizing for stop",
            "connector_state_10": "Finishing. Waiting for disconnect",
            "connector_state_11": "Finishing. Waiting for unplug",
            "connector_state_12": "Finishing. Waiting for unlock authentication",
            "connector_state_13": "Finishing. Waiting for cable owner's tag",
            "connector_state_14": "Unavailable",

            "message_in_flight_type_0": "Authorize",
            "message_in_flight_type_1": "BootNotification",
            "message_in_flight_type_2": "ChangeAvailabilityResponse",
            "message_in_flight_type_3": "ChangeConfigurationResponse",
            "message_in_flight_type_4": "ClearCacheResponse",
            "message_in_flight_type_5": "DataTransfer",
            "message_in_flight_type_6": "DataTransferResponse",
            "message_in_flight_type_7": "GetConfigurationResponse",
            "message_in_flight_type_8": "Heartbeat",
            "message_in_flight_type_9": "MeterValues",
            "message_in_flight_type_10": "RemoteStartTransactionResponse",
            "message_in_flight_type_11": "RemoteStopTransactionResponse",
            "message_in_flight_type_12": "ResetResponse",
            "message_in_flight_type_13": "StartTransaction",
            "message_in_flight_type_14": "StatusNotification",
            "message_in_flight_type_15": "StopTransaction",
            "message_in_flight_type_16": "UnlockConnectorResponse",
            "message_in_flight_type_17": "AuthorizeResponse",
            "message_in_flight_type_18": "BootNotificationResponse",
            "message_in_flight_type_19": "ChangeAvailability",
            "message_in_flight_type_20": "ChangeConfiguration",
            "message_in_flight_type_21": "ClearCache",
            "message_in_flight_type_22": "GetConfiguration",
            "message_in_flight_type_23": "HeartbeatResponse",
            "message_in_flight_type_24": "MeterValuesResponse",
            "message_in_flight_type_25": "RemoteStartTransaction",
            "message_in_flight_type_26": "RemoteStopTransaction",
            "message_in_flight_type_27": "Reset",
            "message_in_flight_type_28": "StartTransactionResponse",
            "message_in_flight_type_29": "StatusNotificationResponse",
            "message_in_flight_type_30": "StopTransactionResponse",
            "message_in_flight_type_31": "UnlockConnector",
            "message_in_flight_type_32": "GetDiagnosticsResponse",
            "message_in_flight_type_33": "DiagnosticsStatusNotification",
            "message_in_flight_type_34": "FirmwareStatusNotification",
            "message_in_flight_type_35": "UpdateFirmwareResponse",
            "message_in_flight_type_36": "GetDiagnostics",
            "message_in_flight_type_37": "DiagnosticsStatusNotificationResponse",
            "message_in_flight_type_38": "FirmwareStatusNotificationResponse",
            "message_in_flight_type_39": "UpdateFirmware",
            "message_in_flight_type_40": "GetLocalListVersionResponse",
            "message_in_flight_type_41": "SendLocalListResponse",
            "message_in_flight_type_42": "GetLocalListVersion",
            "message_in_flight_type_43": "SendLocalList",
            "message_in_flight_type_44": "CancelReservationResponse",
            "message_in_flight_type_45": "ReserveNowResponse",
            "message_in_flight_type_46": "CancelReservation",
            "message_in_flight_type_47": "ReserveNow",
            "message_in_flight_type_48": "ClearChargingProfileResponse",
            "message_in_flight_type_49": "GetCompositeScheduleResponse",
            "message_in_flight_type_50": "SetChargingProfileResponse",
            "message_in_flight_type_51": "ClearChargingProfile",
            "message_in_flight_type_52": "GetCompositeSchedule",
            "message_in_flight_type_53": "SetChargingProfile",
            "message_in_flight_type_54": "TriggerMessageResponse",
            "message_in_flight_type_55": "TriggerMessage",

            "configuration": "Configuration",

            "last_rejected_tag_reason": "Reason of last tag rejection",

            "last_rejected_tag": /*SFN*/(last_tag: string, last_tag_reason: number) => {
                if (last_tag == "")
                    return "";

                let result = `Last tag ${last_tag}`;
                switch(last_tag_reason) {
                    case 0: //Blocked
                        result += " blocked by server";
                        break;
                    case 1: //Expired
                        result += " expired";
                        break;
                    case 2: //Invalid
                        result += " not known";
                        break;
                    case 3: //ConcurrentTx
                        result += " being used for another transaction";
                        break;
                }
                return result;
            }/*NF*/
        },
        "script": {
            "save_failed": "Failed to save the OCPP settings.",
            "reboot_content_changed": "OCPP settings"
        }
    }
}
