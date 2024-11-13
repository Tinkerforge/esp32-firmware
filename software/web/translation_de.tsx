/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "main": {
        "title": "{{{display_name}}} Webinterface",
        "navbar_login": "Anmeldung",
        "content_login": "Anmeldung",
        "login_username": "Benutzername",
        "login_password": "Passwort",
        "login_button": "Anmelden",
        "navbar_status": "Status",
        "reboot_title": "Neu starten um Konfiguration anzuwenden",
        "reboot_content": /*SFN*/ (name: string) => `Die geänderten ${name} werden nur nach einem Neustart angewendet. Jetzt neu starten?`/*NF*/,
        "abort": "Abbrechen",
        "reboot": "Neu starten"
    },
    "component": {
        "config_form": {
            "save": "Speichern",
            "reset": "Zurücksetzen"
        },
        "input_password": {
            "to_be_cleared": "Wird beim Speichern geleert",
            "unchanged": "Unverändert",
            "required": "Notwendig",
            "not_set": "Nicht gesetzt",
            "capslock_enabled": "Feststelltaste ist aktiviert"
        },
        "ip_configuration": {
            "ip_configuration": "IP-Konfiguration",
            "dhcp": "Automatisch (DHCP)",
            "static": "Statisch",

            "static_ip": "IP-Adresse",
            "subnet": "Subnetzmaske",
            "subnet_placeholder": "Auswählen...",
            "gateway": "Gateway",
            "dns": "DNS-Server",
            "dns_muted": "optional",
            "dns2": "Alternativer DNS-Server",
            "dns2_muted": "optional",
            "static_ip_invalid": "Die IP-Adresse muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrennt durch einen Punkt, bestehen. Zum Beispiel 10.0.0.2",
            "gateway_invalid": "Die IP des Gateways muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrennt durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1",
            "dns_invalid": "Die IP des DNS-Servers muss entweder leer bleiben, oder aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrennt durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1.",
            "dns2_invalid": "Die IP des alternativen DNS-Servers muss entweder leer bleiben, oder aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrennt durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1.",
            "gateway_out_of_subnet": "Gateway-Adresse liegt außerhalb des konfigurierten Subnetzes!",
            "subnet_captures_prefix": "Kombination aus IP-Adresse und Subnetzmaske nicht erlaubt, da sie ",
            "subnet_captures_suffix": " übernehmen würde.",
            "ip_is_broadcast": "Die IP-Adresse darf nicht die Broadcast-Adresse des Subnetzes sein!",
            "ip_is_network": "Die IP-Adresse darf nicht die Netzwerk-Adresse des Subnetzes sein!",
            "wifi_ap": "den WLAN-Access-Point",
            "wifi_sta": "die WLAN-Verbindung",
            "ethernet": "die LAN-Verbindung",
            "wireguard": "die WireGuard-Verbindung"
        },
        "collapsed_section": {
            "heading": "Details",
            "show": "Anzeigen",
            "hide": "Verstecken"
        },
        "input_text": {
            "min_only": /*SFN*/ (min: string) => `Mindestens ${min} Zeichen erforderlich`/*NF*/,
            "max_only": /*SFN*/ (max: string) => `Maximal ${max} Zeichen erlaubt`/*NF*/,
            "min_max": /*SFN*/ (min: string, max: string) => `Mindestens ${min}, aber maximal ${max} Zeichen erforderlich`/*NF*/,
            "required": "Feld darf nicht leer sein"
        },
        "input_number": {
            "min_only": /*SFN*/ (min: string, unit: string) => `Mindestens ${min} ${unit} erforderlich`/*NF*/,
            "max_only": /*SFN*/ (max: string, unit: string) => `Maximal ${max} ${unit} erlaubt`/*NF*/,
            "min_max": /*SFN*/ (min: string, max: string, unit: string) => `Mindestens ${min}, aber maximal ${max} ${unit} erforderlich`/*NF*/,
            "required": "Feld darf nicht leer sein"
        },
        "input_file": {
            "uploading": "wird hochgeladen..."
        },
        "table": {
            "abort": "Verwerfen",
            "add": "Hinzufügen",
            "apply": "Übernehmen"
        },
        "remote_close_button": {
            "close": "Fernzugriff schließen"
        }
    },
    "util": {
        "reboot_title": "Starte neu...",
        "reboot_text": "Das Webinterface wird nach dem Neustart automatisch neu geladen.",
        "event_connection_lost_title": null,
        "event_connection_lost": "Verbindung wird wiederhergestellt...",

        "upload_abort": "Upload abgebrochen",
        "upload_error": "Fehler beim Hochladen",
        "upload_timeout": "Timeout beim Hochladen",
        "download_abort": "Download abgebrochen",
        "download_error": "Fehler beim Herunterladen",
        "download_timeout": "Timeout beim Herunterladen",
        "not_yet_known": "Noch unbekannt"
    },
    "reset": {
        "reset_modal": "Zurücksetzen",
        "reset_modal_body": "Konfiguration wirklich zurücksetzen?",
        "reset_modal_body_prefix": "",
        "reset_modal_body_postfix": " wirklich zurücksetzen?",
        "reset_modal_abort": "Abbrechen",
        "reset_modal_confirm": "Zurücksetzen"
    },
    "days": "Tage",
    "day": "Tag",
    "today": "Heute",
    "tomorrow": "Morgen"
}
