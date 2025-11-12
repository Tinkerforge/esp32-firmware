/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "eebus": {
        "content": {
            "enable_eebus": "EEBUS aktivieren",
            "enable_eebus_desc": "Erlaubt die Kommunikation mit EEBUS Geräten",
            "enable_eebus_help": <>
                <p>EEBUS ist ein Kommunikationsstandard der unter anderem zukünftig von Energienetzebereibern genutzt werden
                soll, um die Last für das Stromnetz reduzieren zu können. EEBUS soll daher genutzt werden um
                Verbrauchseinrichtungen (z.B. Wallboxen), die unter den §14a EnWG fallen zu steuern. EEBUS ist standardisiert,
                allerdings stellen sich noch viele Detailfragen hinsichtlich der genauen Implentierung.
                Wir warten aktuell noch auf genauere Informationen und Testmöglichkeiten seitens der Netzbetreiber.</p>
                <p>Hast du bereits eine Steuerbox mit Unterstützung für EEBUS installiert bekommen? Dann
                kontaktiere uns doch bitte unter <a href="mailto:info@tinkerforge.com">info@tinkerforge.com</a>.</p>
                <p>EEBUS definiert verschiedene Use Cases. Für die Steuerung vom Netzbetreiber wird der LPC (Limitation of Power Consumption)
                Use Case benötigt. Diesen Use Case werden wir veröffentlichen sobald die ersten Steuerboxen mit EEBUS-Support ausgerollt werden.</p>
                <p>Für die Steuerung über ein HEMS werden andere Use Cases seitens EEBUS definiert.
                Für den Bereich E-Mobility definiert EEBUS zum Beispiel die Use Cases:
                <ul>
                    <li>Coordinated EV Charging</li>
                    <li>Overload Protection by EV Charging Current Curtailment</li>
                    <li>Optimization of Self-Consumption during EV Charging</li>
                    <li>EV Charging Electricity Measurement</li>
                    <li>EV Charging Summary</li>
                    <li>EV Commissioning and Configuration</li>
                    <li>EVSE Commissioning and Configuration</li>
                </ul>
                In einem zweiten Schritt werden wir relevante Use Cases aus dem E-Mobility Bereich implementieren.</p>
                <p>Aktuell kann bereits nach anderen EEBUS-fähigen Geräten gesucht werden und eine Verbindung zwischen diesem Gerät
                und anderen EEBUS-fähigen Geräten hergestellt werden. Desweiteren zeichnen wir die Kommunikation im Debug-Log auf,
                so dass diese von uns ausgewertet werden kann.</p>
            </>,
            "cert": "Zertifikat",
            "key": "Key",
            "ski": "SKI",
            "ski_muted": "Subject Key Identifier dieses Geräts",
            "ski_help": <>
                <p>Der Subject Key Identifier (SKI) ist ein eindeutiger kryptographischer Bezeichner für dieses Gerät.</p>
                <p>Wenn dieses Gerät mit z.B. einer Steuerbox kommunizieren soll, kann es sein dass der SKI bei der Einrichtung
                der Steuerbox benötigt wird.</p>
            </>,
            "no_cert": "Nicht verwendet",
            "unknown": "Unbekannt",
            "search_peers": "Suche",
            "searching_peers": "Suche im Gange...",
            "search_completed": "Neue Suche",
            "search_failed": "Suche fehlgeschlagen",
            "add_peers": "SKI Hinzufügen",
            "peer_info": {
                "peers": "EEBUS-Partner",
                "dns_name": "DNS Adresse",
                "device_ip": "Adressen",
                "device_port": "Port",
                "device_trusted": "Verbindung erlauben",
                "model_brand": "Hersteller",
                "model_model": "Gerät",
                "model_type": "Gerätetyp",
                "trusted_no": "Nein",
                "trusted_yes": "Ja",
                "edit_peer_title": "Bearbeiten",
                "wss_path": "WSS Pfad",
                "state": "Status",
                "state_connected": "Verbunden",
                "state_disconnected": "Unbekannt",
                "state_discovered": "Entdeckt",
                "autoregister": "Autoregistrierung aktiv",
                "overwrite_notice": "Diese Einstellungen werden überschrieben wenn ein EEBUS Gerät mit der SKI gefunden wird."
            },
            "show_usecase_details": "EEBUS Details anzeigen",
            "yes": "Ja",
            "no": "Nein"
        },
        "script": {
            "save_failed": "Speichern der EEBUS-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "EEBUS-Einstellungen"
        }
    }
}
