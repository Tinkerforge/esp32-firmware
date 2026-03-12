/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "ethernet": {
        "status": {
            "ethernet_connection": "LAN-Verbindung",
            "not_configured": "Deaktiviert",
            "not_connected": "Getrennt",
            "connecting": "Verbinde",
            "connected": "Verbunden",
            "disabled_after_reboot": "Aktiv bis Neustart"
        },
        "navbar": {
            "ethernet": "LAN-Verbindung"
        },
        "content": {
            "ethernet": "LAN-Verbindung",
            "mac": "MAC-Adresse",
            "mac_none": "LAN-Verbindung nicht aktiv",
            "status_ip": "IP-Adresse",
            "status_ip_none": "Keine IP-Adresse",
            "status_link": "Verbindung",
            "status_link_none": "Keine Verbindung",
            "full_duplex": "Vollduplex",
            "half_duplex": "Halbduplex",
            "enable": "LAN-Verbindung aktiviert",
            "enable_desc": "Verbindung wird automatisch beim Start oder wenn ein Kabel eingesteckt wird aufgebaut.",
            "disabled_but_active": "Die LAN-Verbindung wurde deaktiviert und wird nach dem nächsten Neustart nicht mehr zur Verfügung stehen. Dies wird nicht empfohlen, da es dazu führen kann, dass das Gerät nicht mehr erreichbar ist.",
            "disabled_auto_revert": /*SFN*/(seconds: number) => `Die Einstellung wird automatisch in ${seconds} Sekunden zurückgesetzt.`/*NF*/,
            "reboot_now": "Jetzt neustarten.",
            "reboot_body": "Die LAN-Verbindung wurde deaktiviert. Jetzt neustarten um die Änderung anzuwenden?",
            "reboot_with_lan_disabled": "Neustart mit deaktiviertem LAN",
            "cannot_disable": "Mindestens eine Netzwerkschnittstelle muss aktiviert bleiben. Aktiviere zuerst die WLAN-Verbindung oder den WLAN-Access-Point.",
            "cannot_disable_no_wifi": "Die LAN-Verbindung kann nicht deaktiviert werden, da keine andere Netzwerkschnittstelle verfügbar ist."
        },
        "script": {
            "save_failed": "Speichern der Verbindungseinstellungen fehlgeschlagen."
        }
    }
}
