/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "../../options";
let x = {
    "iso15118": {
        "content": {
            "configuration": "Konfiguration",
            "autocharge": "Autocharge",
            "autocharge_desc": "Automatische Ladefreigabe per Fahrzeugidentifikation aktivieren",
            "autocharge_help": <><p>Wenn aktiviert, kommuniziert die Wallbox kurz mit dem Fahrzeug, um dessen eindeutige Netzwerkkennung (MAC-Adresse) auszulesen. Diese Kennung kann dann zur automatischen Ladefreigabe verwendet werden.</p><p>Der Vorgang dauert je nach Fahrzeug etwa 5 bis 20 Sekunden, danach wird die Kommunikationssitzung beendet.</p></>,
            "read_soc": "SoC auslesen",
            "read_soc_desc": "Ladestand vom Fahrzeug per ISO 15118 auslesen",
            "read_soc_help": <><p>Wenn aktiviert, liest die Wallbox den aktuellen Ladestand (State of Charge) einmalig zu Beginn eines Ladevorgangs vom Fahrzeug aus. Dies dauert je nach Fahrzeug etwa 10 bis 45 Sekunden.</p><p>Nach der ersten Messung wird der Ladestand während des Ladens mithilfe des eingebauten Stromzählers fortgeschrieben. {options.PRODUCT_NAME.startsWith("WARP") ? <span>Diese Funktion ist nur beim {options.PRODUCT_NAME} Pro verfügbar, da dieser über den erforderlichen eingebauten Stromzähler verfügt.</span> : <span>Diese Funktion erfordert eine Wallbox mit eingebautem Stromzähler.</span>}</p></>,
            "charge_via_iso15118": "Laden per ISO 15118-20",
            "charge_via_iso15118_desc": "Laden per ISO 15118-20 aktivieren",
            "charge_via_iso15118_help": <><p>Diese Funktion ist noch nicht verfügbar. Sie wird mit einem zukünftigen Firmware-Update freigeschaltet, sobald die ISO 15118-20 Zertifizierung abgeschlossen ist.</p><p>Mit ISO 15118-20 kann die Wallbox das Fahrzeug identifizieren, den Ladestand kontinuierlich auslesen und den Ladevorgang direkt über das ISO 15118-20 Protokoll steuern.</p></>,
            "min_charge_current": "Minimaler Ladestrom",
            "min_charge_current_help": <><p>Der standardmäßige minimale Ladestrom, der Fahrzeugen beim Laden über ISO 15118-20 angeboten wird. Dieser Wert gilt für alle Fahrzeuge, sofern er nicht durch eine fahrzeugspezifische Konfiguration überschrieben wird.</p><p>Manche Fahrzeuge können bei niedrigen Strömen effizient laden, während andere dabei Energie verschwenden oder nur sehr langsam laden. Falls ein Fahrzeug bei niedriger Leistung ineffizient lädt, sollte dieser Wert erhöht werden.</p></>,
            "advanced_settings": "Experteneinstellungen",
            "pib_download": "PIB herunterladen",
            "pib_download_desc": "Parameter Information Block herunterladen",
            "pib_upload": "PIB hochladen",
            "pib_upload_desc": "PIB-Datei auf das PLC-Modem hochladen",
            "pib_upload_button": "PIB flashen",
            "pib_uploading": "PIB wird geflasht..."
        },
        "script": {
            "save_failed": "Speichern der ISO15118-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "ISO15118-Einstellungen",
            "pib_download_failed": "PIB-Download fehlgeschlagen",
            "pib_upload_failed": "PIB-Upload fehlgeschlagen",
            "pib_upload_success": "PIB erfolgreich geflasht. Das PLC-Modem wird zurückgesetzt.",
            "pib_write_failed": "PIB-Schreibvorgang fehlgeschlagen"
        }
    }
}
