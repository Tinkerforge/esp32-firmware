
/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "remote_access": {
        "status": {
            "remote_access": "Fernzugriff",
            "label_muted": /*SFN*/ (since: string) => `seit ${since}`/*NF*/,
            "since_start": "seit Neustart",
            "connected": "Verbunden zum Server",
            "connected_to_clients": /*SFN*/ (clients: number) => `${clients} Client${clients == 1 ? "" : "s"} verbunden` /*NF*/,
            "disconnected": "Getrennt",
            "disabled": "Deaktiviert"
        },
        "navbar": {
            "remote_access": "Fernzugriff"
        },
        "content": {
            "remote_access": "Fernzugriff",
            "email": "E-Mail-Adresse",
            "password": "Passwort",
            "password_muted": "wird nur zur Registrierung verwendet und nicht gespeichert",
            "relay_host": "Relay-Server-Hostname oder -IP-Adresse",
            "relay_host_muted": "Um einen anderen Relay-Server zu verwenden müssen alle Accounts entfernt werden",
            "relay_port": "Relay-Server-Port",
            "enable": "Fernzugriff aktiviert",
            "enable_desc": /*FFN*/ (host: string) => <>Für den Fernzugriff wird ein <a href={`https://${host}/`}>{host}</a>-Account benötigt.</> /*NF*/,
            "cert": "TLS-Zertifikat",
            "not_used": "Eingebettetes Zertifikatsbundle",
            "status_modal_header": "Registrierung wird durchgeführt",
            "advanced_settings": "Experteneinstellungen",
            "prepare_login": "Bereite Login vor",
            "logging_in": "Logge Benutzer ein",
            "prepare_encryption": "Bereite Verschlüsselung vor",
            "registration": <>Registriere {__("device")}</>,
            "login_failed": "Login Fehlgeschlagen:",
            "wrong_credentials": "Benutzername oder Passwort falsch",
            "add_user": "Benutzer hinzufügen",
            "user": "Account",
            "user_add_message": /*SFN*/ (user_count: number, max_users: number) => `${user_count} von ${max_users} Benutzern konfiguriert.`/*NF*/,
            "all_users_in_use": "Alle Accounts in benutzung",
            "note": "Notiz",
            "note_muted": /*FFN*/(host: string) => <>wird auf <a href={`https://${host}/`} target="_blank">{host}</a> angezeigt. Optional</>/*NF*/,
            "user_exists": "Account wurde bereits hinzugefügt",
            "token_corrupted": "Autorisierungstoken beschädigt. Bitte entfernen und neu hinzufügen.",
            "auth_token": "Autorisierungstoken",
            "auth_token_invalid": "Ungültiges Autorisierungstoken",
            "auth_method": "Autorisierungsmethode"
        },
        "script": {
            "save_failed": "Speichern der Fernzugriffs-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "Fernzugriffs-Einstellungen",
            "http_client_error_10": <>Relay-Server kann nicht erreicht werden. Verfügt {__("the_device")} über eine Internetverbindung? Ist der Relay-Server korrekt konfiguriert?</>
        }
    }
}
