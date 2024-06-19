/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "firmware_update": {
        "status": {
        },
        "navbar": {
            "firmware_update": "Firmware-Aktualisierung"
        },
        "content": {
            "firmware_update": "Firmware-Aktualisierung",
            "current_version": "Installierte Version",
            "manual_update": "Manuelle Aktualisierung",
            "manual_update_muted": <><a href="{{{firmware_url}}}">manueller Download</a></>,
            "browse": "Durchsuchen",
            "select_file": "Firmware-Datei auswählen...",
            "update": "Hochladen",
            "downgrade": "Firmware-Downgrade",
            "abort_downgrade": "Abbrechen",
            "confirm_downgrade": "Downgrade durchführen",
            "wrong_signature_title": "Signatur-Abweichung",
            "wrong_signature_body": /*FFN*/(actual_publisher: string, expected_publisher: string) => actual_publisher == null ? <>Firmware-Datei wurde <strong>nicht</strong> von <span style="font-style: italic;">{expected_publisher}</span> bereitgestellt und ist <strong>nicht</strong> vertrauenswürdig!</> : <>Firmware-Datei wurde wider Erwarten von <span style="font-style: italic;">{actual_publisher}</span>, anstelle von <span style="font-style: italic;">{expected_publisher}</span>, bereitgestellt!</>/*NF*/,
            "abort_update": "Abbrechen",
            "confirm_override": "Firmware trotzdem installieren",
            "check_for_update": "Nach Aktualisierung suchen",
            "check_for_update_timestamp": "Letzte Aktualisierungssuche",
            "check_for_update_error": "Letzter Fehler",
            "beta_update": "Verfügbare Beta-Aktualisierung",
            "release_update": "Verfügbare Release-Aktualisierung",
            "stable_update": "Verfügbare Stable-Aktualisierung",
            "no_update": "Keine Aktualisierung verfügbar",
            "install_update": "Installieren"
        },
        "script": {
            "update_success": "Erfolgreich aktualisiert; starte neu...",
            "update_fail": "Aktualisierung fehlgeschlagen",
            "vehicle_connected": "Es kann keine Aktualisierung vorgenommen werden, während ein Fahrzeug verbunden ist.",
            "no_info_page": null,
            "info_page_corrupted": "Firmware-Datei ist beschädigt (Checksummenfehler)",
            "wrong_firmware_type": null,
            "downgrade": "Firmware-Datei beinhaltet ein Downgrade auf Version %firmware%. Installiert ist Version %installed%.",
            "no_update_url": "Kein Aktualisierungs-URL konfiguriert",
            "no_cert": "HTTPS-Zertifikat nicht vorhanden",
            "download_error": "Fehler beim Download aufgetreten",
            "no_response": "Keine Antwort vom Aktualisierungs-Server",
            "list_malformed": "Aktualisierungsliste ist beschädigt",
            "build_time": /*SFN*/ (build_time: string) => ` (erstellt ${build_time})`/*NF*/,
            "install_failed": "Installation fehlgeschlagen"
        }
    }
}
