/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "network": {
        "status": {
            "sta_and_ethernet": "WLAN und Ethernet aktiv",
            "subnet_conflict": "Subnetzkonflikt",
            "sta": "WLAN-Verbindung",
            "ethernet": "LAN-Verbindung",
            "ap": "WLAN-Access-Point",
            "wireguard": "Wireguard",
            "remote_access": "Fernzugriff",
            "status_help": /*FFN*/(val: [{network: string, name: string, href: string, dhcp?: boolean}, {network: string, name: string, href: string, dhcp?: boolean}][]) => {
                return <>
                    <p>Es gibt Schnittstellen mit kollidierenden Netzwerken. Dies kann zu Problemen bei der Erreichbarkeit des {__("device")}s führen. Bitte stelle sicher, dass es keine Überschneidungen gibt.</p>
                    <p>Folgende Konfigurationen sind problematisch:</p>
                    {
                        val.length > 0 ? <><ul class="mb-0">{val.map(v => <li><a href={v[0].href}>{v[0].name}</a> ({v[0].network}{v[0].dhcp ? " via DHCP" : ""}) und <a href={v[1].href}>{v[1].name}</a> ({v[1].network}{v[1].dhcp ? " via DHCP" : ""})</li>)}</ul></> : <></>
                    }
                </>
            }/*NF*/
        },
        "navbar": {
            "network": "Einstellungen"
        },
        "content": {
            "network": "Netzwerk",
            "hostname": "Hostname",
            "hostname_invalid": "Der Hostname darf nur aus den Groß- und Kleinbuchstaben A-Z und a-z, sowie den Ziffern 0-9 und Bindestrichen bestehen. Zum Beispiel warp-A1c",
            "enable_mdns": "mDNS aktiviert",
            "enable_mdns_desc": <>{__("The_device")} kann von anderen Geräten in diesem Netzwerk per mDNS gefunden werden.</>,
            "web_server_port": "Listen-Port des Webinterfaces"
        },
        "script": {
            "save_failed": "Speichern der Netzwerkeinstellungen fehlgeschlagen",
            "reboot_content_changed": "Netzwerkeinstellungen"
        }
    }
}
