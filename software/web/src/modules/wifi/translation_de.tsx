/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "wifi": {
        "status": {
            "wifi_connection": "WLAN-Verbindung",
            "not_configured": "Deaktiviert",
            "not_connected": "Getrennt",
            "connecting": "Verbinde",
            "connected": "Verbunden",
            "wifi_ap": "WLAN-Access-Point",
            "deactivated": "Deaktiviert",
            "activated": "Aktiviert",
            "fallback_inactive": "Fallback inaktiv",
            "fallback_active": "Fallback aktiv"
        },
        "navbar": {
            "wifi_sta": "WLAN-Verbindung",
            "wifi_ap": "WLAN-Access-Point"
        },
        "content": {
            "sta_settings": "WLAN-Verbindung",
            "sta_enable_sta": "WLAN-Verbindung aktiviert",
            "sta_enable_sta_desc": null,
            "sta_ssid": "Netzwerkname (SSID)",
            "sta_scan": "Netzwerksuche",
            "sta_scanning": "Suche Netzwerke...",
            "sta_bssid": "BSSID",
            "sta_bssid_lock": "BSSID-Sperre",
            "sta_bssid_invalid": "Die BSSID muss aus sechs Gruppen mit jeweils einer zweistelligen Hexadezimalzahl, getrennt durch einen Doppelpunkt, bestehen. Zum Beispiel 01:23:45:67:89:AB",
            "sta_bssid_lock_desc": "Verbindet sich nur zur konfigurierten BSSID. Bei Einsatz mehrerer Access Points und/oder Repeater mit demselben Netzwerknamen wird so immer derselbe AP oder Repeater verwendet.",
            "sta_passphrase": "Passphrase",

            "ap_settings": "WLAN-Access-Point",
            "ap_enable": "Access Point",
            "ap_enable_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "ap_enabled": "Aktiviert",
            "ap_fallback_only": "Nur als Fallback",
            "ap_disabled": "Deaktiviert",
            "ap_ssid": "Netzwerkname (SSID)",
            "ap_hide_ssid": "Netzwerkname versteckt",
            "ap_hide_ssid_desc_pre": null,
            "ap_hide_ssid_desc_post": " erreichbar",
            "ap_passphrase": "Passphrase",
            "ap_channel": "Kanal",
            "ap_channel_muted": "ignoriert wenn WLAN-Verbindung aktiv ist",
            "ap_channel_auto_select": "Automatische Auswahl",

            "confirm_title": "WLAN-Access-Point deaktivieren?",
            "confirm_content": null,
            "confirm_abort": "Abbrechen",
            "confirm_confirm": "Deaktivieren"
        },
        "script": {
            "scan_wifi_init_failed": "Suche nach Netzwerken fehlgeschlagen. Konnte Scan nicht starten.",
            "scan_wifi_results_failed": "Suche nach Netzwerken fehlgeschlagen. Konnte Ergebnisse nicht abholen.",
            "select_ap": "Gefundene Netzwerke",
            "no_ap_found": "Kein Netzwerk gefunden.",
            "hidden_ap": "[Versteckter AP]",

            "sta_save_failed": "Speichern der Verbindungseinstellungen fehlgeschlagen.",
            "ap_save_failed": "Speichern der Access Point-Einstellungen fehlgeschlagen.",
            "sta_reboot_content_changed": "Verbindungseinstellungen",
            "ap_reboot_content_changed": "Access-Point-Einstellungen"
        }
    }
}
