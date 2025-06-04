/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "./src/options";
import { __ } from "./src/ts/translation";
let x = {
    "main": {
        "title": `${options.PRODUCT_NAME} Webinterface`,
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
            "wireguard": "die WireGuard-Verbindung",
            "remote_access": "den Fernzugriff"
        },
        "collapsed_section": {
            "heading": "Details",
            "show": "Anzeigen",
            "hide": "Verstecken"
        },
        "input_text": {
            "min_only": /*SFN*/ (min: string) => `Mindestens ${min} Zeichen erforderlich`/*NF*/,
            "max_only": /*SFN*/ (max: string) => `Maximal ${max} Zeichen erlaubt`/*NF*/,
            "min_max": /*SFN*/ (min: string, max: string) => `Mindestens ${min} Zeichen erforderlich, aber maximal ${max} Zeichen erlaubt`/*NF*/,
            "required": "Feld darf nicht leer sein"
        },
        "input_number": {
            "min_only": /*SFN*/ (min: string, unit: string) => `Mindestens ${min} ${unit} erforderlich`/*NF*/,
            "max_only": /*SFN*/ (max: string, unit: string) => `Maximal ${max} ${unit} erlaubt`/*NF*/,
            "min_max": /*SFN*/ (min: string, max: string, unit: string) => `Mindestens ${min} ${unit} erforderlich, aber maximal ${max} ${unit} erlaubt`/*NF*/,
            "required": "Feld darf nicht leer sein"
        },
        "input_any_float": {
            "min_only": /*SFN*/ (min: number) => `Mindestens ${min} erforderlich`/*NF*/,
            "max_only": /*SFN*/ (max: number) => `Maximal ${max} erlaubt`/*NF*/,
            "min_max": /*SFN*/ (min: number, max: number) => `Mindestens ${min} erforderlich, aber maximal ${max} erlaubt`/*NF*/
        },
        "input_file": {
            "uploading": "wird hochgeladen..."
        },
        "input_host": {
            "invalid_feedback": "Hostname oder IP-Address ist ungültig"
        },
        "item_modal": {
            "error_button": "Schließen",
            "import_button": "Importieren",
            "import_error_title": "Fehler beim Importieren",
            "import_error_no_file": "Es wurde keine Datei ausgewählt.",
            "import_error_wrong_file_type": "Ausgewählte Datei ist keine JSON Datei.",
            "import_error_file_read_error": "Datei konnte nicht gelesen werden.",
            "export_button": "Exportieren",
            "export_error_title": "Fehler beim Exportieren"
        },
        "table": {
            "abort": "Verwerfen",
            "add": "Hinzufügen",
            "apply": "Übernehmen"
        },
        "remote_close_button": {
            "close": "Fernzugriff schließen"
        },
        "debug_logger": {
            "loading_debug_report": "Lade Debug-Report",
            "loading_debug_report_failed": "Laden des Debug-Reports fehlgeschlagen",
            "loading_event_log": "Lade Event-Log",
            "loading_event_log_failed": "Laden des Event-Logs fehlgeschlagen",
            "tab_close_warning": /*SFN*/ (debug_module: string) => `Die Aufzeichnung des ${debug_module}-Logs wird abgebrochen, wenn der Tab geschlossen wird.`/*NF*/,
            "starting_debug": /*SFN*/ (debug_module: string) => `Aktiviere Aufzeichnung des ${debug_module}-Logs.`/*NF*/,
            "starting_debug_failed": /*SFN*/ (debug_module: string) => `Aktivierung der Aufzeichnung des ${debug_module}-Logs fehlgeschlagen.`/*NF*/,
            "debug_running": "Aufzeichnung läuft. Tab nicht schließen!",
            "debug_stop_failed": /*SFN*/ (debug_module: string) => `Stoppen der Aufzeichnung des ${debug_module}-Logs fehlgeschlagen.`/*NF*/,
            "debug_stopped": /*SFN*/ (debug_module: string) => `Aufzeichnung des ${debug_module}-Logs gestoppt.`/*NF*/,
            "debug_done": "Abgeschlossen.",
            "debug_start": "Start",
            "debug_stop": "Stop + Download"
        }
    },
    "util": {
        "reboot_title": "Starte neu...",
        "reboot_text": "Das Webinterface wird nach dem Neustart automatisch neu geladen.",
        "event_connection_lost_title": <>Verbindung {__("to_the_device")} verloren!</>,
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
    "tomorrow": "Morgen",
    "select": "Auswählen...",
    "branding": {
        "device": null,
        "the": null,
        "The": null,
        "this": null,
        "This": null,
        "to_the": null,
        "from_the": null
    },
    "device": <>{__("branding.device")}</>,
    "the_device": <>{__("branding.the")} {__("branding.device")}</>,
    "The_device": <>{__("branding.The")} {__("branding.device")}</>,
    "this_device": <>{__("branding.this")} {__("branding.device")}</>,
    "This_device": <>{__("branding.This")} {__("branding.device")}</>,
    "to_the_device": <>{__("branding.to_the")} {__("branding.device")}</>,
    "from_the_device": <>{__("branding.from_the")} {__("branding.device")}</>
}
