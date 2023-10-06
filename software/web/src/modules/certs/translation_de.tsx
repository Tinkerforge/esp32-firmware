/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "certs": {
        "status": {
        },
        "navbar": {
            "certs": "TLS-Zertifikate"
        },
        "content": {
            "certs": "TLS-Zertifikate",
            "add_cert_title": "TLS-Zertifikat hinzufügen",

            "edit_cert_title": "TLS-Zertifikat bearbeiten",
            "cert_name": "Zertifikatsname",
            "cert_file": "Zertifikatsdatei",
            "cert_file_muted": "(PEM-Format)",

            "add_cert_id": "Zertifikats-ID",

            "add_cert_message": /*SFN*/(have: number, max: number) => `${have} von ${max} Zertifikaten konfiguriert.`/*NF*/,

            "browse": "Durchsuchen",
            "select_file": "Datei auswählen"
        },
        "script": {
            "save_failed": "Speichern der Zertifikatseinstellungen fehlgeschlagen",
            "reboot_content_changed": "Zertifikatseinstellungen",
            "add_cert_failed": "Hinzufügen des Zertifikats fehlgeschlagen",
            "cert_too_large": /*SFN*/(max_length: number) => `Zertifikatsdatei zu groß! Maximale Dateigröße ist ${max_length} Bytes.`/*NF*/
        }
    }
}
