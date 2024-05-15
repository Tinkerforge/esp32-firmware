/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "firmware_update": {
        "status": {
        },
        "navbar": {
            "firmware_update": "Firmware Update"
        },
        "content": {
            "current_firmware": "Firmware version",
            "firmware_update": "Firmware Update",
            "firmware_update_label": "Firmware update",
            "firmware_update_desc": <><a href="{{{firmware_url}}}">firmware download</a></>,
            "browse": "Browse",
            "select_file": "Select firmware file...",
            "update": "Upload",
            "downgrade": "Firmware downgrade",
            "abort_downgrade": "Abort",
            "confirm_downgrade": "Downgrade firmware"
        },
        "script": {
            "update_success": "Successfully updated; restarting...",
            "update_fail": "Failed to update",
            "vehicle_connected": "Can't update the firmware while a vehicle is connected.",
            "no_info_page": null,
            "info_page_corrupted": "Firmware file corrupted (checksum error)",
            "wrong_firmware_type": null,
            "downgrade": "Firmware file contains a downgrade to version %fw%. Installed is version %installed%.",
            "build_time_prefix": " (created ",
            "build_time_suffix": ")"
        }
    }
}
