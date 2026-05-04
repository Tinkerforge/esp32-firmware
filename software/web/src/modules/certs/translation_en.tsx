/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "certs": {
        "status": {
        },
        "navbar": {
            "certs": "TLS Certificates"
        },
        "content": {
            "certs": "TLS Certificates",

            "cert_explainer": <>
                <p>These key types are supported for private keys and public keys within certificates:</p>
                <ul>
                    <li>ECDSA with secp256r1 or secp384r1 curve, also known as P-256 and P-384</li>
                    <li>RSA with lengths from 2048 to 4096 bit</li>
                </ul>
            </>,

            "add_cert_title": "Add TLS certificate",

            "edit_cert_title": "Edit TLS certificate",
            "cert_name": "Display name",
            "cert_file": "Certificate file",
            "cert_file_muted": "PEM format",

            "add_cert_message": /*SFN*/(have: number, max: number) => `${have} of ${max} certificates configured`/*NF*/
        },
        "script": {
            "add_cert_failed": "Failed to add certificate",
            "mod_cert_failed": "Failed to modify certificate",
            "del_cert_failed": "Failed to remove certificate",
            "cert_too_large": /*SFN*/(max_length: number) => `Certificate file too large! Maximum file size is ${max_length} bytes.`/*NF*/
        }
    }
}
