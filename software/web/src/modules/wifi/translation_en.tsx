/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "../../options";
import { __ } from "../../ts/translation";
let x = {
    "wifi": {
        "status": {
            "wifi_connection": "WiFi connection",
            "not_configured": "Disabled",
            "not_connected": "Not connected",
            "connecting": "Connecting",
            "connected": "Connected",
            "wifi_ap": "WiFi access point",
            "disabled": "Disabled",
            "enabled": "Enabled",
            "fallback_inactive": "Fallback inactive",
            "fallback_active": "Fallback active"
        },
        "navbar": {
            "wifi_sta": "WiFi Connection",
            "wifi_ap": "WiFi Access Point"
        },
        "content": {
            "sta_settings": "WiFi Connection",
            "sta_mac": "Device MAC address",
            "sta_connected_bssid": "Connected BSSID",
            "sta_connected_bssid_none": "Not connected",
            "sta_connection": "WiFi connection",
            "status_sta_ip": "IP address",
            "status_sta_ip_none": "No IP address",
            "status_sta_rssi": "Signal strength",
            "status_sta_rssi_none": "Not connected",
            "status_sta_disconnect_reason": "Disconnect reason",
            "status_sta_disconnect_reason_no_ap": "Access point not found. Is the reception too poor or the SSID incorrect?",
            "status_sta_disconnect_reason_auth": "Authentication failed. Is the passphrase correct?",
            "status_sta_disconnect_reason_ap_full": "Access point full: Too many connected devices.",
            "status_sta_disconnect_reason_no_dhcp": "No IP address received via DHCP.",
            "status_sta_disconnect_reason_poor_reception": "Reception too poor.",
            "status_sta_disconnect_reason_generic": "Connection failed.",
            "sta_enable_sta": "WiFi connection enabled",
            "sta_enable_sta_desc": <>Automatically connects to the configured network on start-up.</>,
            "sta_ssid": "Network name (SSID)",
            "sta_scan": "Search for networks",
            "sta_bssid": "BSSID",
            "sta_bssid_invalid": "The BSSID must have six groups separated by a colon, each with a two-digit hexadecimal number. For example 01:23:45:67:89:AB",
            "sta_bssid_lock": "BSSID lock",
            "sta_bssid_lock_desc": "Connects only to the configured BSSID, even when other access points or repeaters are available on the same network. When disabled, usually the one with the best reception is chosen.",
            "sta_enable_11b": "Optimize reception",
            "sta_enable_11b_desc": <>Use deprecated slow 802.11b mode for slightly better WiFi range. <strong>This also slows down other devices connected to this network</strong> but may improve the reception quality and robustness of the WiFi connection somewhat.</>,
            "sta_passphrase": "Passphrase",

            "ap_settings": "WiFi Access Point",
            "ap_bssid": "BSSID",
            "ap_bssid_none": "Access point not active",
            "ap_state": "Access point",
            "status_ap_ip": "IP address",
            "ap_disabled_but_active": "The WiFi access point has been disabled and will not be available after the next reboot. This is not recommended as it may cause the device to become unreachable.",
            "ap_restart_now": "Restart now.",
            "ap_reboot_body": "The WiFi access point has been disabled. Reboot now to apply the change?",
            "sta_disabled_but_active": "The WiFi connection has been disabled and will not be available after the next reboot.",
            "sta_restart_now": "Restart now.",
            "sta_reboot_body": "The WiFi connection has been disabled. Reboot now to apply the change?",
            "ap_enable": "Access point",
            "ap_enable_help": <>
                <p>The access-point can be run in two different modes: Always on, and only active in case no other network is available.</p>
                <p><strong>We strongly advise to never switch it completely off, since you wont be able to access the device once the default network is unavailable.</strong></p>
            </>,
            "ap_enabled": "Enabled",
            "ap_fallback_only": "As fallback only",
            "ap_disabled": "Disabled",
            "ap_ssid": "Network name (SSID)",
            "ap_hide_ssid": "Network name hidden",
            "ap_hide_ssid_desc": "Some devices cannot find hidden WiFis on channels 12 and 13.",
            "ap_passphrase": "Passphrase",
            "ap_channel": "Channel",
            "ap_channel_muted": "ignored if WiFi connection is active",
            "ap_channel_auto_select": "Selected automatically",

            "confirm_title": "Disable WiFi Access Point?",
            "confirm_content": <>Are you sure you want to disable the WiFi Access Point? If no connection to a configured network can be established in the future, <b>it is not possible to access {__("the_device")} anymore</b>. {__("The_device")} then has to be reset to factory defaults. {options.MANUAL_URL.length > 0 ? <a href={options.MANUAL_URL}>See manual for details.</a> : undefined}</>,
            "confirm_abort": "Abort",
            "confirm_confirm": "Disable",

            "wpa_personal": "WPA2/3 Personal",
            "wpa_auth_type": "Authentication",
            "eap_tls": "WPA2 Enterprise - EAP-TLS",
            "eap_peap_ttls": "WPA2 Enterprise - EAP-PEAP/TTLS",
            "eap_identity": "Identity",
            "eap_username": "Username",
            "eap_password": "Password",
            "eap_ca_cert": "CA certificate",
            "eap_client_cert": "Client certificate",
            "eap_client_key": "Client key",
            "eap_cert_placeholder": "No certificate",
            "eap_key_placeholder": "No key",
            "optional_eap_cert_muted": /*FFN*/ (is_key: boolean) => {
                return <><a href="#certs">Add {is_key ? "key" : "certificate"}</a>; optional</>
            } /*NF*/,
            "eap_cert_muted": /*FFN*/ (is_key: boolean) => {
                return <a href="#certs">Add {is_key ? "key" : "certificate"}</a>
            } /*NF*/,
            "optional": "optional"
        },
        "script": {
            "scan_wifi_init_failed": "Search for networks failed. Failed to start scan.",
            "scan_wifi_results_failed": "Search for networks failed. Failed to fetch scan results.",
            "no_ap_found": "No network found",
            "hidden_ap": "[Hidden AP]",

            "sta_save_failed": "Failed to save connection settings.",
            "ap_save_failed": "Failed to save access point settings."
        }
    }
}
