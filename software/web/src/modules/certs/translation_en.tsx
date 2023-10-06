/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "certs": {
        "status": {
        },
        "navbar": {
            "certs": "TLS certificates"
        },
        "content": {
            "certs": "TLS certificates",
            "add_cert_title": "Add TLS certificate",

            "edit_cert_title": "Edit TLS certificate",
            "cert_name": "Display name",
            "cert_file": "Certificate file",
            "cert_file_muted": "(PEM format)",

            "add_cert_id": "Certificate ID",

            "add_cert_message": /*SFN*/(have: number, max: number) => `${have} of ${max} certificates configured.`/*NF*/,

            "browse": "Browse",
            "select_file": "Select file"
        },
        "script": {
            "save_failed": "Failed to save certificate configuration",
            "reboot_content_changed": "Certificate configuration",
            "add_cert_failed": "Failed to add certificate",
            "cert_too_large": /*SFN*/(max_length: number) => `Certificate file too large! Maximum file size is ${max_length} bytes.`/*NF*/
        }
    }
}
