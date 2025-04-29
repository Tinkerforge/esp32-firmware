/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "wifi": {
        "status": {
            "wifi_connection": "WLAN-Verbindung",
            "not_configured": "Deaktiviert",
            "not_connected": "Getrennt",
            "connecting": "Verbinde",
            "connected": "Verbunden",
            "wifi_ap": "WLAN-Access-Point",
            "disabled": "Deaktiviert",
            "enabled": "Aktiviert",
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
            "sta_enable_sta_desc": <>{__("The_device")} verbindet sich beim Start automatisch zum konfigurierten Netzwerk.</>,
            "sta_ssid": "Netzwerkname (SSID)",
            "sta_scan": "Netzwerksuche",
            "sta_bssid": "BSSID",
            "sta_bssid_lock": "BSSID-Sperre",
            "sta_bssid_invalid": "Die BSSID muss aus sechs Gruppen mit jeweils einer zweistelligen Hexadezimalzahl, getrennt durch einen Doppelpunkt, bestehen. Zum Beispiel 01:23:45:67:89:AB",
            "sta_bssid_lock_desc": "Verbindet sich nur zur konfigurierten BSSID. Bei Einsatz mehrerer Access Points und/oder Repeater mit demselben Netzwerknamen wird so immer derselbe AP oder Repeater verwendet.",
            "sta_enable_11b": "Empfangsoptimierung",
            "sta_enable_11b_desc": <>Verwendet den veralteten, langsamen 802.11b-Modus für etwas höhere Reichweite. <strong>Dieser verlangsamt auch andere Geräte, die zum WLAN verbunden sind</strong>, kann aber die Empfangsqualität und Robustheit der Verbindung leicht verbessern.</>,
            "sta_passphrase": "Passphrase",

            "ap_settings": "WLAN-Access-Point",
            "ap_enable": "Access Point",
            "ap_enable_help": <>
                <p>Der Access-Point kann in einem von zwei Modi betrieben werden: Entweder kann er immer aktiv sein oder nur dann, wenn die Verbindung zu einem anderen WLAN bzw. zu einem LAN nicht konfiguriert oder fehlgeschlagen ist. </p>
                <p><strong>Wir empfehlen, den Access-Point nie komplett zu deaktivieren, da sonst bei einer fehlgeschlagenen Verbindung zu einem anderen Netzwerk das Webinterface nicht mehr erreicht werden kann.</strong></p>
            </>,
            "ap_enabled": "Aktiviert",
            "ap_fallback_only": "Nur als Fallback",
            "ap_disabled": "Deaktiviert",
            "ap_ssid": "Netzwerkname (SSID)",
            "ap_hide_ssid": "Netzwerkname versteckt",
            "ap_hide_ssid_desc": "Einige Geräte können versteckte WLANs auf Kanal 12 und 13 nicht finden.",
            "ap_passphrase": "Passphrase",
            "ap_channel": "Kanal",
            "ap_channel_muted": "ignoriert, wenn WLAN-Verbindung aktiv ist",
            "ap_channel_auto_select": "Automatische Auswahl",

            "confirm_title": "WLAN-Access-Point deaktivieren?",
            "confirm_content": <>Soll der WLAN-Access-Point wirklich komplett deaktiviert werden? Falls zukünftig keine Verbindung zu einem konfigurierten Netzwerk möglich ist, <b>kann nicht mehr auf das Webinterface zugegriffen werden</b>. {__("The_device")} muss dann auf den Werkszustand zurückgesetzt werden. <a href="{{{manual_url}}}">Siehe Betriebsanleitung für Details.</a></>,
            "confirm_abort": "Abbrechen",
            "confirm_confirm": "Deaktivieren",

            "wpa_personal": "WPA2/3 Personal",
            "wpa_auth_type": "Authentifizierung",
            "eap_tls": "WPA2 Enterprise - EAP-TLS",
            "eap_peap_ttls": "WPA2 Enterprise - EAP-PEAP/TTLS",
            "eap_identity": "Identität",
            "eap_username": "Benutzername",
            "eap_password": "Passwort",
            "eap_ca_cert": "CA-Zertifikat",
            "eap_client_cert": "Client-Zertifikat",
            "eap_client_key": "Client-Schlüssel",
            "eap_cert_placeholder": "Kein Zertifikat",
            "eap_key_placeholder": "Kein Schlüssel",
            "optional_eap_cert_muted": /*FFN*/ (is_key: boolean) => {
                return <><a href="#certs">{is_key ? "Schlüssel" : "Zertifikat"} hinzufügen</a>; optional</>
            } /*NF*/,
            "eap_cert_muted": /*FFN*/ (is_key: boolean) => {
                return <a href="#certs">{is_key ? "Schlüssel" : "Zertifikat"} hinzufügen</a>
            } /*NF*/,
            "optional": "optional"
        },
        "script": {
            "scan_wifi_init_failed": "Suche nach Netzwerken fehlgeschlagen. Konnte Scan nicht starten.",
            "scan_wifi_results_failed": "Suche nach Netzwerken fehlgeschlagen. Konnte Ergebnisse nicht abholen.",
            "no_ap_found": "Kein Netzwerk gefunden.",
            "hidden_ap": "[Versteckter AP]",

            "sta_save_failed": "Speichern der Verbindungseinstellungen fehlgeschlagen.",
            "ap_save_failed": "Speichern der Access Point-Einstellungen fehlgeschlagen.",
            "sta_reboot_content_changed": "Verbindungseinstellungen",
            "ap_reboot_content_changed": "Access-Point-Einstellungen"
        }
    }
}
