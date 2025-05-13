/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "network": {
        "status": {
            "sta_and_ethernet": "STA and Ethernet active",
            "subnet_conflict": "Subnetzkonflikt",
            "sta": "Wifi-STA",
            "ethernet": "Ethernet",
            "ap": "Wifi-AP",
            "wireguard": "Wireguard",
            "remote_access": "Remote Access",
            "status_help": /*FFN*/(val: {network: number, name: string}[]) => {
                return <>
                    <p>Shows if network configurations exists that probably interfere with each other</p>
                    <ul>
                        <li><strong>{__("network.status.sta_and_ethernet")}</strong>: Wifi Connection and LAN Connection is active at the same time</li>
                        <li><strong>{__("network.status.subnet_conflict")}</strong>: Multiple network interfaces are in the same subnet
                    {
                        val.length > 0 ? <><br/>Conflicting interfaces:<ul>{val.map(v => <li>{v.name}</li>)}</ul></> : <></>
                    }</li>
                    </ul>
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
