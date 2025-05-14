/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "network": {
        "status": {
            "sta_and_ethernet": "Wifi and Ethernet active",
            "subnet_conflict": "Subnet conflict",
            "sta": "Wifi-STA",
            "ethernet": "Ethernet",
            "ap": "Wifi-AP",
            "wireguard": "Wireguard",
            "remote_access": "Remote Access",
            "status_help": /*FFN*/(val: [{network: string, name: string}, {network: string, name: string}][]) => {
                return <>
                    <p>Following interfaces have conflicting networks:</p>
                    {
                        val.length > 0 ? <><ul class="mb-0">{val.map(v => <li>{v[0].name} ({v[0].network}) und {v[1].name} ({v[1].network})</li>)}</ul></> : <></>
                    }
                </>
            }/*NF*/
        },
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
