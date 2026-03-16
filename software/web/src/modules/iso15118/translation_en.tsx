/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "iso15118": {
        "content": {
            "configuration": "Configuration",
            "autocharge": "Autocharge",
            "autocharge_desc": "Enable automatic charging authorization via EV identification",
            "read_soc": "Read SoC",
            "read_soc_desc": "Read State of Charge from the EV",
            "charge_via_iso15118": "Charge via ISO 15118",
            "charge_via_iso15118_desc": "Enable charging via ISO 15118",
            "min_charge_current": "Minimum Charge Current",
            "advanced_settings": "Advanced Settings",
            "pib_download": "Download PIB",
            "pib_download_desc": "Download Parameter Information Block",
            "pib_upload": "Upload PIB",
            "pib_upload_desc": "Upload a PIB file to the PLC modem",
            "pib_upload_button": "Flash PIB",
            "pib_uploading": "Flashing PIB..."
        },
        "script": {
            "save_failed": "Failed to save the ISO15118 settings.",
            "reboot_content_changed": "ISO15118 settings",
            "pib_download_failed": "Failed to download PIB",
            "pib_upload_failed": "Failed to upload PIB",
            "pib_upload_success": "PIB successfully flashed. The PLC modem is resetting.",
            "pib_write_failed": "PIB write failed"
        }
    }
}
