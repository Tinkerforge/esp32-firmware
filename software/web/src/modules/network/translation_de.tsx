/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "network": {
        "status": {
            "sta_and_ethernet": "STA und Ethernet aktiv",
            "subnet_conflict": "Subnetzkonflikt",
            "sta": "WLAN-Verbindung",
            "ethernet": "LAN-Verbindung",
            "ap": "WLAN-Access-Point",
            "wireguard": "Wireguard",
            "remote_access": "Fernzugriff",
            "status_help": /*FFN*/(val: {network: number, name: string}[]) => {
                return <>
                    <p>Zeigt Netzwerkkonfigurationen, welche möglicherweise zu Problemen führen können an</p>
                    <ul>
                        <li><strong>{__("network.status.sta_and_ethernet")}</strong>: Wifi-STA und Ethernet ist gleichzeitig aktiv</li>
                        <li><strong>{__("network.status.subnet_conflict")}</strong>: Mehrere Netzwerkinterface sind im Selben Subnetz
                    {
                        val.length > 0 ? <><br/>Folgende Konflikte:<ul>{val.map(v => <li>{v.name}</li>)}</ul></> : <></>
                    }</li>
                    </ul>
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
