/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
import * as options from "../../options";
let x = {
    "network": {
        "status": {
            "subnet_conflict": "Subnetzkonflikt",
            "dns_not_configured": "DNS nicht konfiguriert",
            "dns_not_configured_text": /*FFN*/(val: string[]) => {
                return <>
                    <p>Folgende Schnittstellen haben keinen DNS-Server konfiguriert:</p>
                    <ul>
                        {val.map(v => <li>{v}</li>)}
                    </ul>
                </>
            }/*NF*/,
            "subnet_text": /*FFN*/(val: [{network: string, name: string, href: string, dhcp?: boolean}, {network: string, name: string, href: string, dhcp?: boolean}][]) => {
                return <>
                    <p>Es gibt Schnittstellen mit kollidierenden Netzwerken. Dies kann zu Problemen bei der Erreichbarkeit des {__("device")}s führen. Bitte stelle sicher, dass es keine Überschneidungen gibt.</p>
                    <p>Folgende Konfigurationen sind problematisch:</p>
                    {
                        val.length > 0 ? <><ul>{val.map(v => <li><a href={v[0].href}>{v[0].name}</a> ({v[0].network}{v[0].dhcp ? " via DHCP" : ""}) und <a href={v[1].href}>{v[1].name}</a> ({v[1].network}{v[1].dhcp ? " via DHCP" : ""})</li>)}</ul></> : <></>
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
            "hostname_invalid": `Der Hostname darf nur aus den Groß- und Kleinbuchstaben A-Z und a-z, sowie den Ziffern 0-9 und Bindestrichen bestehen. Zum Beispiel ${options.HOSTNAME_PREFIX}-A1c`,
            "enable_mdns": "mDNS aktiviert",
            "enable_mdns_desc": <>{__("The_device")} kann von anderen Geräten in diesem Netzwerk per mDNS gefunden werden.</>,
            "web_server_port": "Listen-Port des Webinterfaces (HTTP)",
            "web_server_port_secure": "Sicherer Listen-Port des Webinterfaces (HTTPS)",
            "cert": "Zertifikat für HTTPS",
            "cert_help": "Ein selbstsigniertes Zertifikat im PEM- oder DER-Format, oder eine Zertifikatskette im PEM-Format, beginnend mit dem Gerätezertifikat.",
            "key": "Privater Schlüssel für HTTPS",
            "key_help": "Privater Schlüssel für das gewählte Zertifikat. Aus Performancegründen sollte das ein ECDSA-Schlüssel mit secp256r1-Kurve oder alternativ ein 2048-Bit-RSA-Schlüssel sein. Größere Schlüssellängen sind signifikant langsamer und bieten aktuell keinen signifikanten Sicherheitsgewinn.",
            "no_cert": "Internes selbst-signiertes Zertifikat",
            "no_key": "Interner selbst-generierter privater Schlüssel",
            "transport_mode": "Verbindungsmodus",
            "insecure": "Unsicher (HTTP)",
            "secure": "Sicher (HTTPS)",
            "insecure_and_secure": "Unsicher und sicher gleichzeitig (HTTP und HTTPS)"
        },
        "script": {
            "save_failed": "Speichern der Netzwerkeinstellungen fehlgeschlagen",
            "reboot_content_changed": "Netzwerkeinstellungen"
        }
    }
}
