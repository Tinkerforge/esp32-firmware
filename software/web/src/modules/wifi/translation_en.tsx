/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "wifi": {
        "status": {
            "wifi_connection": "WiFi connection",
            "not_configured": "Deactivated",
            "not_connected": "Not connected",
            "connecting": "Connecting",
            "connected": "Connected",
            "wifi_ap": "WiFi access point",
            "deactivated": "Deactivated",
            "activated": "Activated",
            "fallback_inactive": "Fallback inactive",
            "fallback_active": "Fallback active"
        },
        "navbar": {
            "wifi_sta": "WiFi Connection",
            "wifi_ap": "WiFi Access Point"
        },
        "content": {
            "sta_settings": "WiFi Connection",
            "sta_enable_sta": "WiFi connection enabled",
            "sta_enable_sta_desc": "Automatically connects to the configured network on start-up",
            "sta_ssid": "Network name (SSID)",
            "sta_scan": "Search for networks",
            "sta_scanning": "Searching for networks...",
            "sta_bssid": "BSSID",
            "sta_bssid_invalid": "The BSSID must have six groups separated by a colon, each with a two-digit hexadecimal number. For example 01:23:45:67:89:AB",
            "sta_bssid_lock": "BSSID lock",
            "sta_bssid_lock_desc": "Connects only to the network with the configured BSSID. Leave disabled if you use multiple access points or repeaters with with the same network name.",
            "sta_passphrase": "Passphrase",

            "ap_settings": "WiFi Access Point",
            "ap_enable": "Access point",
            "ap_enable_muted": <><a href="{{{manual_url}}}">see manual for details</a></>,
            "ap_enabled": "Activated",
            "ap_fallback_only": "As fallback only",
            "ap_disabled": "Deactivated",
            "ap_ssid": "Network name (SSID)",
            "ap_hide_ssid": "Network name hidden",
            "ap_hide_ssid_desc_pre": null,
            "ap_hide_ssid_desc_post": ".",
            "ap_passphrase": "Passphrase",
            "ap_channel": "Channel",
            "ap_channel_muted": "ignored if WiFi connection is active",
            "ap_channel_auto_select": "Selected automatically",

            "confirm_title": "Disable WiFi Access Point?",
            "confirm_content": null,
            "confirm_abort": "Abort",
            "confirm_confirm": "Disable",

            "wpa_personal": "WPA2/3 Personal",
            "wpa_auth_type": "Authentication",
            "eap_tls": "WPA2 Enterprise - EAP-TLS",
            "eap_peap_ttls": "WPA2 Enterprise - EAP-PEAP/TTLS",
            "eap_identity": "Identity",
            "eap_username": "Username",
            "eap_password": "Password",
            "eap_ca_cert": "CA-Certificate",
            "eap_client_cert": "Client-Certificate",
            "eap_client_key": "Client-Key",
            "eap_client_key_password": "Client-Key-Password",
            "eap_cert_placeholder": "No certificate",
            "eap_key_placeholder": "No key",
            "optional_eap_cert_muted": /*FFN*/ () => {
                return <>optional <a href="#certs">Certificate administration</a></>
            } /*NF*/,
            "eap_cert_muted": /*FFN*/ () => {
                return <a href="#certs">Certificate administration</a>
            } /*NF*/,
            "optional": "optional"
        },
        "script": {
            "scan_wifi_init_failed": "Search for networks failed. Failed to start scan.",
            "scan_wifi_results_failed": "Search for networks failed. Failed to fetch scan results.",
            "select_ap": "Found networks",
            "no_ap_found": "No network found",
            "hidden_ap": "[Hidden AP]",

            "sta_save_failed": "Failed to save connection settings.",
            "ap_save_failed": "Failed to save access point settings.",
            "sta_reboot_content_changed": "connection settings",
            "ap_reboot_content_changed": "access point settings"
        }
    }
}
