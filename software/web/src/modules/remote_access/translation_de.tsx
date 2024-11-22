
/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "remote_access": {
        "navbar": {
            "remote_access": "Fernzugriff"
        },
        "content": {
            "remote_access": "Fernzugriff",
            "email": "E-Mail-Adresse",
            "password": "Passwort",
            "password_muted": "wird nur zur Registrierung verwendet und nicht gespeichert",
            "relay_host": "Relay-Server-Hostname",
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
            "registration": null,
            "login_failed": "Login Fehlgeschlagen:",
            "wrong_credentials": "Nutzername oder Passwort falsch",
            "add_user": "Nutzer hinzufügen",
            "user": "Account",
            "user_add_message": /*SFN*/ (user_count: number, max_users: number) => `${user_count} von ${max_users} Nutzern konfiguriert.`/*NF*/,
            "all_users_in_use": "Alle Accounts in benutzung",
            "note": "Notiz",
            "note_muted": /*FFN*/(host: string) => <>wird auf <a href={`https://${host}/`} target="_blank">{host}</a> angezeigt. Optional</>/*NF*/
        },
        "script": {
            "save_failed": "Speichern der Fernzugriffs-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "Fernzugriffs-Einstellungen"
        }
    }
}
