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
            "check_for_updates": "Nach Aktualisierungen suchen",
            "check_for_updates_timestamp": "Letzte Aktualisierungssuche",
            "check_for_updates_error": "Letzter Fehler",
            "available_beta_update": "Verfügbare Beta-Aktualisierung",
            "available_release_update": "Verfügbare Release-Aktualisierung",
            "available_stable_update": "Verfügbare Stable-Aktualisierung",
            "no_update": "Keine Aktualisierung verfügbar"
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
            "build_time": /*SFN*/ (build_time: string) => ` (erstellt ${build_time})`/*NF*/
        }
    }
}
