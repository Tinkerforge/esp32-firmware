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

            "cert_explainer": <>
                <p>Diese Schlüsseltypen werden für private Schlüssel und öffentliche Schlüssel in Zertifikaten unterstützt:</p>
                <ul>
                    <li>ECDSA mit secp256r1- oder secp384r1-Kurve, auch bekannt als P-256 und P-384</li>
                    <li>RSA mit Längen von 2048 bis 4096 Bit</li>
                </ul>
            </>,

            "add_cert_title": "TLS-Zertifikat hinzufügen",

            "edit_cert_title": "TLS-Zertifikat bearbeiten",
            "cert_name": "Zertifikatsname",
            "cert_file": "Zertifikatsdatei",
            "cert_file_muted": "im PEM-Format",

            "add_cert_message": /*SFN*/(have: number, max: number) => `${have} von ${max} Zertifikaten konfiguriert`/*NF*/
        },
        "script": {
            "add_cert_failed": "Hinzufügen des Zertifikats fehlgeschlagen",
            "mod_cert_failed": "Modifizieren des Zertifikats fehlgeschlagen",
            "del_cert_failed": "Löschen des Zertifikats fehlgeschlagen",
            "cert_too_large": /*SFN*/(max_length: number) => `Zertifikatsdatei zu groß! Maximale Dateigröße ist ${max_length} Bytes.`/*NF*/
        }
    }
}
