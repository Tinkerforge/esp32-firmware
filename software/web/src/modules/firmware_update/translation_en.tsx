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
            "firmware_update": "Firmware Update",
            "current_version": "Installed version",
            "manual_update": "Manual update",
            "manual_update_muted": <><a href="{{{firmware_url}}}">manual download</a></>,
            "browse": "Browse",
            "select_file": "Select firmware file...",
            "update": "Upload",
            "downgrade": "Firmware downgrade",
            "abort_downgrade": "Abort",
            "confirm_downgrade": "Downgrade firmware",
            "check_for_updates": "Check for updates",
            "check_for_updates_timestamp": "Last update check",
            "available_beta_update": "Available beta update",
            "available_release_update": "Available release update",
            "available_stable_update": "Available stable update",
            "no_update": "No update available"
        },
        "script": {
            "update_success": "Successfully updated; restarting...",
            "update_fail": "Failed to update",
            "vehicle_connected": "Can't update the firmware while a vehicle is connected.",
            "no_info_page": null,
            "info_page_corrupted": "Firmware file corrupted (checksum error)",
            "wrong_firmware_type": null,
            "downgrade": "Firmware file contains a downgrade to version %fw%. Installed is version %installed%.",
            "build_time": /*SFN*/ (build_time: string) => ` (created ${build_time})`/*NF*/
        }
    }
}
