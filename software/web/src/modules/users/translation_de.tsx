/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "users": {
        "status": {},
        "navbar": {
            "users": "Benutzerverwaltung"
        },
        "content": {
            "users": "Benutzerverwaltung",

            "enable_authentication": "Anmeldung aktiviert",
            "enable_authentication_desc":
                "Beim Aufrufen des Webinterfaces oder bei Verwendung der HTTP-API muss eine Anmeldung als einer der konfigurieren Benutzer durchgeführt werden",
            "enable_authentication_invalid":
                "Damit die Anmeldung aktiviert sein kann, muss mindestens ein Benutzer mit einem konfigurierten Passwort vorhanden sein.",

            "authorized_users": "Berechtigte Benutzer",

            "unknown_username": "Anzeigename des unbekannten Benutzers",

            "table_username": "Benutzername",
            "table_display_name": "Anzeigename",
            "table_current": "Maximaler Ladestrom",
            "table_password": "Passwort",

            "add_user_title": "Benutzer hinzufügen",
            "add_user_username": "Benutzername",
            "add_user_username_desc": "zur Anmeldung beim Webinterface",
            "add_user_display_name": "Anzeigename",
            "add_user_display_name_desc":
                "zur Anzeige im Ladelog und Webinterface",
            "add_user_current": "Maximaler Ladestrom",
            "add_user_password": "Passwort",
            "add_user_password_desc": "Anmeldung deaktiviert",
            "add_user_message": /*SFN*/ (have: number, max: number) =>
                `${have} von ${max} Benutzern konfiguriert` /*NF*/,
            "add_user_user_ids_exhausted":
                "Die maximale Anzahl an Benutzern hat Ladevorgänge aufgezeichnet. Damit ein neuer Benutzer angelegt werden kann, müssen aufgezeichnete Ladevorgänge oder ein existierender Benutzer ohne aufgezeichnete Ladevorgänge gelöscht werden.",

            "edit_user_title": "Benutzer bearbeiten",
            "edit_user_username": "Benutzername",
            "edit_user_username_desc": "zur Anmeldung beim Webinterface",
            "edit_user_display_name": "Anzeigename",
            "edit_user_display_name_desc":
                "zur Anzeige im Ladelog und Webinterface",
            "edit_user_current": "Maximaler Ladestrom",
            "edit_user_password": "Passwort",

            "evse_user_description": "Ladefreigabe",

            "evse_user_enable": <>
                    Verlangt eine Freigabe des Ladevorgangs durch einen Benutzer
                    zum Laden (z.B. per NFC-Tag)
                </>,
            "evse_user_enable_invalid":
                "Für die Ladefreigabe ist mindestens ein Benutzer notwendig.",
            "evse_user_enable_central_auth_warning": /*FFN*/(manager_ip: string) => <>Deaktiviert, weil die <a href={`http://${manager_ip}/#charge_manager_chargers`}>zentrale Ladefreigabe</a> im Lastmanager aktiviert ist.</>/*NF*/,

            "nfc_tags": "NFC-Tags",
            "nfc_tag_id": "Tag-ID",
            "nfc_tag_type": "Tag-Typ",
            "nfc_no_tags_assigned": "Keine NFC-Tags zugeordnet",
            "nfc_no_seen_tags":
                "In letzter Zeit wurden keine NFC-Tags erkannt. Halte ein Tag an die Wallbox, um es zu erkennen.",
            "nfc_tag_already_assigned": /*SFN*/ (other_name: string) =>
                `Bereits Benutzer "${other_name}" zugeordnet` /*NF*/,
            "nfc_add_tag": "NFC-Tag hinzufügen",
            "nfc_add_tag_message": /*SFN*/ (
                have: number,
                max: number,
                haveForUser: number,
                maxPerUser: number,
            ) =>
                `${have} von ${max} Tags konfiguriert (${haveForUser} von ${maxPerUser} für diesen Benutzer)` /*NF*/,
            "nfc_seen_tags": "Erkannte Tags",
            "nfc_last_seen": "Zuletzt erkannt",
            "nfc_add_tag_manually": "Tag manuell hinzuf\u00fcgen",
            "nfc_tag_id_placeholder": "z.B. 01:23:45:67",
            "nfc_tag_id_invalid":
                "Die Tag-ID muss aus vier bis zehn Gruppen mit jeweils einer zweistelligen Hexadezimalzahl, getrennt durch einen Doppelpunkt, bestehen. Zum Beispiel 01:23:45:67:89:AB"
        },
        "script": {
            "reboot_content_changed": "Benutzereinstellungen",
            "login_disabled": "Anmeldung deaktiviert",
            "save_failed": "Speichern der Benutzereinstellungen fehlgeschlagen.",
            "username_already_tracked":
                "Benutzername ist bereits in aufgezeichneten Ladevorgängen",
            "username_already_used": "Benutzername ist bereits vergeben ",
            "all_user_ids_in_use": "Keine freien Benutzer-IDs mehr."
        }
    }
}
