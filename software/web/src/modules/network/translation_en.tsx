/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "network": {
        "status": {},
        "navbar": {
            "network": "Settings"
        },
        "content": {
            "network": "Network",
            "hostname": "Hostname",
            "hostname_invalid": "The hostname must contain only upper and lower case letters A-Z and a-z the digits 0-9 or dashes. For example warp-A1c",
            "enable_mdns": "mDNS enabled",
            "enable_mdns_desc": <>Allows other devices in this network to find {__("the_device")}.</>,
            "web_server_port": "Web interface listen port"
        },
        "script": {
            "save_failed": "Failed to save the network settings",
            "reboot_content_changed": "network settings"
        }
    }
}
