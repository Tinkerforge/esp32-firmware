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
            "manual_update_muted": <><a href="{{{firmware_url}}}">recent firmwares</a></>,
            "browse": "Browse",
            "select_file": "Select firmware file...",
            "install_update": "Install",
            "installing_update": "installing...",
            "downgrade": "Firmware downgrade",
            "abort_downgrade": "Abort",
            "confirm_downgrade": "Downgrade firmware",
            "wrong_signature_title": "Signature mismatch",
            "wrong_signature_body": /*FFN*/(actual_publisher: string, expected_publisher: string) =>
                <>The firmware file was <strong>not</strong> published by <span class="text-success">{expected_publisher}</span>
                {actual_publisher == null ? <> and is <strong>not</strong>to be trusted</> : <>, but supposedly by <span class="text-danger">{actual_publisher}</span> instead</>}!
                The publisher <span class="text-success">{expected_publisher}</span> does not <strong>warrant</strong> the security and functionality of this unknown firmware!</>/*NF*/,
            "abort_update": "Abort",
            "confirm_override": "Install firmware anyway",
            "check_for_update": "Check for update",
            "check_for_update_timestamp": "Last update check",
            "check_for_update_error": "Last error",
            "available_update": "Available update",
            "no_update": "No update available"
        },
        "script": {
            "update_success": "Successfully updated; restarting...",
            "update_fail": "Failed to update",
            "vehicle_connected": "Can't update the firmware while a vehicle is connected.",
            "no_info_page": null,
            "info_page_corrupted": "Firmware file corrupted (checksum error)",
            "wrong_firmware_type": null,
            "downgrade": "Firmware file contains a downgrade to version %firmware%. Installed is version %installed%.",
            "no_update_url": "No update URL configured",
            "no_cert": "HTTPS certificate not available",
            "download_error": "Error occurred during download",
            "no_response": "Update server did not respond",
            "list_malformed": "Update list is malformed",
            "build_time": /*SFN*/ (build_time: string) => ` (created ${build_time})`/*NF*/,
            "install_failed": "Install failed"
        }
    }
}
