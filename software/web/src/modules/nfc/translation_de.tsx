/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "nfc": {
        "status": {
        },
        "navbar": {
            "nfc": "NFC-Tags"
        },
        "content": {
            "nfc": "NFC-Tags",

            "deadtime": "Stoppe Ladevorgang mit Tag",
            "deadtime_muted": "Erlaubt das Stoppen eines Ladevorgangs mit einem NFC-Tag erst nach der konfigurierten Zeit, um Fehlbedienung zu vermeiden.",
            "deadtime_min": "Sofort",
            "deadtime_3": "Frühestens 3 Sekunden nach Ladefreigabe",
            "deadtime_10": "Frühestens 10 Sekunden nach Ladefreigabe",
            "deadtime_30": "Frühestens 30 Sekunden nach Ladefreigabe",
            "deadtime_60": "Frühestens 60 Sekunden nach Ladefreigabe",
            "deadtime_max": "Nie",

            "tags": "Konfigurierte Tags",
            "table_tag_id": "Tag-ID",
            "table_user_id": "Zugeordneter Benutzer",
            "table_tag_type": "Tag-Typ",
            "table_last_seen": "Zuletzt erkannt",

            "add_tag_title": "Tag hinzufügen",
            "add_tag_seen_tags": "Zuletzt erkannte Tags",
            "add_tag_user_id": "Zugeordneter Benutzer",
            "add_tag_tag_id": "Tag-ID",
            "add_tag_tag_type": "Tag-Typ",
            "add_tag_prefix": "",
            "add_tag_infix": " von ",
            "add_tag_suffix": " Tags konfiguriert",
            "add_tag_description": "Das neue Tag kann zum Anlernen an die rechte Seite der Wallbox gehalten werden. Alternativ können Tag-ID und Tag-Typ manuell angegeben werden.",

            "edit_tag_title": "Tag bearbeiten",
            "edit_tag_tag_id": "Tag-ID",
            "edit_tag_user_id": "Zugeordneter Benutzer",
            "edit_tag_tag_type": "Tag-Typ",

            "last_seen": "Vor ",
            "last_seen_suffix": "",
            "select_type": "Auswählen...",
            "type_0": "Mifare Classic",
            "type_1": "NFC Forum Typ 1",
            "type_2": "NFC Forum Typ 2",
            "type_3": "NFC Forum Typ 3",
            "type_4": "NFC Forum Typ 4",
            "tag_id_invalid_feedback": "Die Tag-ID muss aus vier bis zehn Gruppen mit jeweils einer zweistelligen Hexadezimalzahl, getrennt durch einen Doppelpunkt, bestehen. Zum Beispiel 01:23:45:67:89:AB"
        },
        "automation": {
            "nfc": "Simuliere NFC-Tag",
            "add_tag_description": "Das neue Tag kann zum Anlernen an die rechte Seite der Wallbox gehalten werden. Alternativ können Tag-ID und Tag-Typ manuell angegeben werden.",
            "trigger_charge_any": "Ladevorgang Start/Stopp",
            "trigger_charge_start": "Ladevorgang Start",
            "trigger_charge_stop": "Ladevorgang Stopp",
            "action": "Aktion",
            "tag_id_invalid_feedback": "Die Tag-ID muss aus vier bis zehn Gruppen mit jeweils einer zweistelligen Hexadezimalzahl, getrennt durch einen Doppelpunkt, bestehen. Zum Beispiel 01:23:45:67:89:AB",
            "table_tag_id": "Tag-ID",
            "table_user_id": "Zugeordneter Benutzer",
            "table_tag_type": "Tag-Typ",
            "type_0": "Mifare Classic",
            "type_1": "NFC Forum Typ 1",
            "type_2": "NFC Forum Typ 2",
            "type_3": "NFC Forum Typ 3",
            "type_4": "NFC Forum Typ 4",
            "last_seen": "Vor ",
            "last_seen_suffix": "",
            "last_seen_and_known_tags": "Zuletzt erkannte und bekannte Tags",
            "automation_action_text": /*FFN*/(tag_id: string, tag_type: string, action: number) => {
                let start_stop = <></>;
                if (action !== 0) {
                    start_stop = <> um einen Ladevorgang zu {action == 1 ? "starten" : action == 2 ? "stoppen" : ""}</>;
                }
                return <>
                    simuliere das <b>NFC</b>-Tag "<b>{tag_id}</b>" ({tag_type}){start_stop}.
                </>
            }/*NF*/,
            "automation_trigger_text": /*FFN*/(tag_id: string, tag_type: string) => {
                return (
                <>
                    Wenn das <b>NFC</b>-Tag "<b>{tag_id}</b>" ({tag_type}) erkannt wird,{" "}
                </>
                )
            }/*NF*/,
            "automation_trigger_nfc": "NFC-Tag erkannt"
        },
        "script": {
            "not_seen": "Unbekannt",
            "save_failed": "Speichern der NFC-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "NFC-Einstellungen",
            "not_assigned": "Nicht zugeordnet",
            "not_assigned_desc": "Dieses Tag kann keinen Ladevorgang freigeben."
        }
    }
}
