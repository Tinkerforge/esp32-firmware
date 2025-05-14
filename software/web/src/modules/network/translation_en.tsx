/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "network": {
        "status": {
            "subnet_conflict": "Subnet conflict",
            "dns_not_configured": "DNS not configured",
            "dns_not_configured_text": /*FFN*/(val: string[]) => {
                return <>
                    <p>Following interfaces do not have a DNS configured:</p>
                    <ul class="mb-0">
                        {val.map(v => <li>{v}</li>)}
                    </ul>
                </>
            }/*NF*/,
            "subnet_text": /*FFN*/(val: [{network: string, name: string, href: string, dhcp?: boolean}, {network: string, name: string, href: string, dhcp?: boolean}][]) => {
                return <>
                    <p>There are interfaces with colliding networks. This can cause problems when trying to connect to {__("the_device")}. Please ensure that there are no overlapping networks configured.</p>
                    <p>Following configurations may cause problems:</p>
                    {
                        val.length > 0 ? <><ul class="mb-0">{val.map(v => <li><a href={v[0].href}>{v[0].name}</a> ({v[0].network}{v[0].dhcp ? " via DHCP" : ""}) and <a href={v[1].href}>{v[1].name}</a> ({v[1].network}{v[1].dhcp ? " via DHCP" : ""})</li>)}</ul></> : <></>
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
