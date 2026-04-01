/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "ethernet": {
        "status": {
            "ethernet_connection": "LAN connection",
            "not_configured": "Disabled",
            "not_connected": "Not connected",
            "connecting": "Connecting",
            "connected": "Connected",
            "disabled_after_reboot": "Active until reboot"
        },
        "navbar": {
            "ethernet": "LAN Connection"
        },
        "content": {
            "ethernet": "LAN Connection",
            "mac": "MAC address",
            "mac_none": "LAN connection not active",
            "status_ip": "IP address",
            "status_ipv6": "IPv6 addresses",
            "status_ip_none": "No IP address",
            "status_link": "Link",
            "status_link_none": "No link",
            "full_duplex": "Full Duplex",
            "half_duplex": "Half Duplex",
            "enable": "LAN connection enabled",
            "enable_desc": "Automatically connects to the configured network on start-up.",
            "disabled_but_active": "The LAN connection has been disabled and will not be available after the next reboot. This is not recommended as it may cause the device to become unreachable.",
            "disabled_auto_revert": /*SFN*/(seconds: number) => `The setting will be automatically reverted in ${seconds} seconds.`/*NF*/,
            "reboot_now": "Reboot now.",
            "reboot_body": "The LAN connection has been disabled. Reboot now to apply the change?",
            "reboot_with_lan_disabled": "Reboot with LAN disabled",
            "cannot_disable": "At least one network interface must remain enabled. Enable WiFi connection or WiFi access point first.",
            "cannot_disable_no_wifi": "LAN connection cannot be disabled because no other network interface is available.",
            "ipv6_switch": "Enable IPv6",
            "ipv6_help": "Only Stateless Address Configuration is supported. The IPv6 DNS has a lower Priority than the IPv4 DNS."
        },
        "script": {
            "save_failed": "Failed to save connection settings."
        }
    }
}
