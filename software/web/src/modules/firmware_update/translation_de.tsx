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
            "manual_update_muted": <><a href="{{{firmware_url}}}">neueste Firmwares</a></>,
            "browse": "Durchsuchen",
            "select_file": "Firmware-Datei auswählen...",
            "install_update": "Installieren",
            "installing_update": "wird installiert...",
            "downgrade_title": "Firmware-Downgrade",
            "downgrade_body": /*SFN*/(firmware_version: string, installed_version: string) => `Firmware-Datei beinhaltet ein Downgrade auf Version ${firmware_version}. Installiert ist Version ${installed_version}.`/*NF*/,
            "confirm_downgrade": "Downgrade durchführen",
            "signature_verify_failed_title": "Signatur-Abweichung",
            "signature_verify_failed_body": /*FFN*/(actual_publisher: string, expected_publisher: string) =>
                <>Die Firmware-Datei wurde <strong>nicht</strong> von <span class="text-success">{expected_publisher}</span>
                {actual_publisher == null ? <> herausgegeben und ist <strong>nicht</strong> vertrauenswürdig</> : <>, sondern angeblich von <span class="text-danger">{actual_publisher}</span> herausgegeben</>}!
                Der Herausgeber <span class="text-success">{expected_publisher}</span> garantiert <strong>nicht</strong> für die Sicherheit und Funktionalität dieser unbekannten Firmware!</>/*NF*/,
            "abort_update": "Abbrechen",
            "confirm_override": "Firmware trotzdem installieren",
            "check_for_update": "Nach Aktualisierung suchen",
            "check_for_update_timestamp": "Letzte Aktualisierungssuche",
            "check_for_update_error": "Letzter Fehler",
            "available_update": "Verfügbare Aktualisierung",
            "no_update": "Keine Aktualisierung verfügbar",
            "install_progress": "Installations-Fortschritt",
            "install_complete": "Installation abgeschlossen"
        },
        "script": {
            "update_success": "Erfolgreich aktualisiert; starte neu...",
            "update_fail": "Aktualisierung fehlgeschlagen",
            "check_state_2": "Eine andere Abfrage läuft bereits",
            "check_state_3": "Interner Fehler aufgetreten",
            "check_state_4": "Kein Aktualisierungs-URL konfiguriert",
            "check_state_5": "HTTPS-Zertifikat nicht vorhanden",
            "check_state_6": "HTTPS-Client-Initialisierung fehlgeschlagen",
            "check_state_7": "Keine Antwort vom Aktualisierungs-Server",
            "check_state_8": "Fehler beim Download aufgetreten",
            "check_state_9": "Versionsnummer ist beschädigt",
            "check_state_10": "Firmware-Index ist beschädigt",
            "check_state_11": "Abgebrochen",
            "check_state_12": "Teil-Download aufgetreten",
            "check_state_13": "Aktualisierungssuche wird nicht unterstüzt",
            "install_state_2": "Eine andere Abfrage läuft bereits",
            "install_state_3": "Interner Fehler aufgetreten",
            "install_state_4": "Kein Aktualisierungs-URL konfiguriert",
            "install_state_5": "HTTPS-Zertifikat nicht vorhanden",
            "install_state_6": "HTTPS-Client-Initialisierung fehlgeschlagen",
            "install_state_7": "Keine Antwort vom Aktualisierungs-Server",
            "install_state_8": "Fehler beim Download aufgetreten",
            "install_state_9": "Versionsnummer ist beschädigt",
            "install_state_10": "Firmware-Datei is zu klein",
            "install_state_11": "Flash-Initialisierung fehlgeschlagen",
            "install_state_12": "Flash-Teil-Aktualisierung aufgetreten",
            "install_state_13": "Flash-Übernahme fehlgeschlagen",
            "install_state_14": "Signatur-Initialisierung fehlgeschlagen",
            "install_state_15": "Signatur-Berechnung fehlgeschlagen",
            "install_state_16": "Signatur-Verifikation fehlgeschlagen",
            "install_state_17": "Während ein Fahrzeug verbunden ist, kann keine Aktualisierung vorgenommen werden",
            "install_state_18": "Firmware-Info-Seite ist zu groß",
            "install_state_19": null,
            "install_state_20": "Firmware-Datei ist beschädigt (Checksummenfehler)",
            "install_state_21": null,
            "install_state_23": "Abgebrochen",
            "install_state_24": "Starte neu",
            "install_state_25": "Teil-Download aufgetreten",
            "install_state_26": "Installation wird nicht unterstüzt",
            "install_state_27": "Größe der Firmware-Datei ist unbekannt",
            "build_time": /*SFN*/(build_time: string) => `erstellt ${build_time}`/*NF*/,
            "publisher": /*SFN*/(publisher: string) => `von ${publisher}`/*NF*/,
            "install_failed": "Installation fehlgeschlagen"
        }
    }
}
