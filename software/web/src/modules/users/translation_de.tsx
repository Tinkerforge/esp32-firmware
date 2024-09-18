/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "users": {
        "status": {
        },
        "navbar": {
            "users": "Benutzerverwaltung"
        },
        "content": {
            "users": "Benutzerverwaltung",

            "enable_authentication": "Anmeldung aktiviert",
            "enable_authentication_desc": "Beim Aufrufen des Webinterfaces oder bei Verwendung der HTTP-API muss eine Anmeldung als einer der konfigurieren Benutzer durchgeführt werden",
            "enable_authentication_invalid": "Damit die Anmeldung aktiviert sein kann, muss mindestens ein Benutzer mit einem konfigurierten Passwort vorhanden sein.",

            "authorized_users": "Berechtigte Benutzer",

            "unknown_username": "Anzeigename des unbekannten Benutzers",

            "table_username": "Benutzername",
            "table_display_name": "Anzeigename",
            "table_current": "Maximaler Ladestrom",
            "table_password": "Passwort",

            "add_user_title": "Benutzer hinzufügen",
            "add_user_username": "Benutzername",
            "add_user_username_desc": "Zur Anmeldung im Webinterface",
            "add_user_display_name": "Anzeigename",
            "add_user_display_name_desc": "Zur Anzeige im Ladeprotokoll und Webinterface",
            "add_user_current": "Maximaler Ladestrom",
            "add_user_password": "Passwort",
            "add_user_password_desc": "Anmeldung deaktiviert",
            "add_user_prefix": "",
            "add_user_infix": " von ",
            "add_user_suffix": " Benutzern konfiguriert",
            "add_user_user_ids_exhausted": "Die maximale Anzahl an Benutzern hat Ladevorgänge aufgezeichnet. Damit ein neuer Benutzer angelegt werden kann, müssen aufgezeichnete Ladevorgänge oder ein existierender Benutzer ohne aufgezeichnete Ladevorgänge gelöscht werden.",

            "edit_user_title": "Benutzer bearbeiten",
            "edit_user_username": "Benutzername",
            "edit_user_display_name": "Anzeigename",
            "edit_user_current": "Maximaler Ladestrom",
            "edit_user_password": "Passwort",

            "evse_user_description": "Ladefreigabe",
            "evse_user_description_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,

            "evse_user_enable": "Verlangt eine Freigabe des Ladevorgangs durch einen Benutzer zum Laden (z.B. per NFC-Tag)",
            "evse_user_enable_invalid": "Für die Ladefreigabe ist mindestens ein Benutzer notwendig."
        },
        "script": {
            "reboot_content_changed": "Benutzereinstellungen",
            "login_disabled": "Anmeldung deaktiviert",
            "save_failed": "Speichern der Benutzereinstellungen fehlgeschlagen.",
            "username_already_tracked": "Benutzername ist bereits in aufgezeichneten Ladevorgängen",
            "username_already_used": "Benutzername ist bereits vergeben ",
            "all_user_ids_in_use": "Keine freien Benutzer-IDs mehr."
        }
    }
}
