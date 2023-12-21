/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "nfc": {
        "status": {
        },
        "navbar": {
            "nfc": "NFC Tags"
        },
        "content": {
            "nfc": "NFC Tags",

            "table_tag_id": "Tag ID",
            "table_user_id": "Assigned user",
            "table_tag_type": "Tag type",
            "table_last_seen": "Last seen",

            "add_tag_title": "Add tag",
            "add_tag_seen_tags": "Last seen tags",
            "add_tag_user_id": "Assigned user",
            "add_tag_tag_id": "Tag ID",
            "add_tag_tag_type": "Tag type",
            "add_tag_prefix": "",
            "add_tag_infix": " of ",
            "add_tag_suffix": " tags configured",
            "add_tag_description": "Hold the new tag to the right side of the charger. Tag ID and type can also be typed in manually.",

            "edit_tag_title": "Edit tag",
            "edit_tag_tag_id": "Tag ID",
            "edit_tag_user_id": "Assigned user",
            "edit_tag_tag_type": "Tag type",

            "last_seen": "",
            "last_seen_suffix": " ago",
            "select_type": "Select...",
            "type_0": "Mifare Classic",
            "type_1": "NFC Forum Type 1",
            "type_2": "NFC Forum Type 2",
            "type_3": "NFC Forum Type 3",
            "type_4": "NFC Forum Type 4",
            "tag_id_invalid_feedback": "The BSSID must have three to ten groups separated by a colon, each with a two-digit hexadecimal number. For example 01:23:45:67:89:AB"
        },
        "automation": {
            "nfc": "NFC-Tags",
            "add_tag_description": "Hold the new tag to the right side of the charger. Tag ID and type can also be typed in manually.",
            "trigger_charge_any": "Charge start/stop",
            "trigger_charge_start": "Charge start",
            "trigger_charge_stop": "Charge stop",
            "tag_action": "Tag action",
            "tag_id_invalid_feedback": "The BSSID must have three to ten groups separated by a colon, each with a two-digit hexadecimal number. For example 01:23:45:67:89:AB",
            "table_tag_id": "Tag ID",
            "table_user_id": "Assigned user",
            "table_tag_type": "Tag type",
            "type_0": "Mifare Classic",
            "type_1": "NFC Forum Type 1",
            "type_2": "NFC Forum Type 2",
            "type_3": "NFC Forum Type 3",
            "type_4": "NFC Forum Type 4",
            "last_seen": "",
            "last_seen_suffix": " ago",
            "last_seen_and_known_tags": "Last seen tags and known tags",

            "automation_action_text": /*FFN*/(tag_id: string, tag_type: string, tag_action: number) => {
                let action = <></>;
                if (tag_action !== 0) {
                    action = <> to {tag_action == 1 ? "start a charge" : tag_action == 2 ? "stop a charge" : ""}</>
                }
                return <>
                    fake the presence of the <b>NFC</b> tag "<b>{tag_id}</b>" ({tag_type}).
                </>
            }/*NF*/,
            "automation_trigger_text": /*FFN*/(tag_id: string, tag_type: string) => {
                return <>
                    If <b>NFC</b> tag "<b>{tag_id}</b>" ({tag_type}) is detected,{" "}
                </>
            }/*NF*/,
            "automation_trigger_nfc": "NFC tag seen"
        },
        "script": {
            "not_seen": "Unknown",
            "save_failed": "Failed to save the NFC settings.",
            "reboot_content_changed": "NFC settings",
            "not_assigned": "Not assigned",
            "not_assigned_desc": "This tag will not be able to start charges."
        }
    }
}
