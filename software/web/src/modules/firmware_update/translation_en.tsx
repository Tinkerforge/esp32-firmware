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
            "downgrade_title": "Firmware downgrade",
            "downgrade_body": /*SFN*/(firmware_version: string, installed_version: string) => `Firmware file contains a downgrade to version ${firmware_version}. Installed is version ${installed_version}.`/*NF*/,
            "confirm_downgrade": "Downgrade firmware",
            "signature_verify_failed_title": "Signature mismatch",
            "signature_verify_failed_body": /*FFN*/(actual_publisher: string, expected_publisher: string) =>
                <>The firmware file was <strong>not</strong> published by <span class="text-success">{expected_publisher}</span>
                {actual_publisher == null ? <> and is <strong>not</strong> to be trusted</> : <>, but supposedly by <span class="text-danger">{actual_publisher}</span> instead</>}!
                The publisher <span class="text-success">{expected_publisher}</span> does <strong>not</strong> warrant the security and functionality of this unknown firmware!</>/*NF*/,
            "abort_update": "Abort",
            "confirm_override": "Install firmware anyway",
            "check_for_update": "Check for update",
            "check_for_update_timestamp": "Last update check",
            "check_for_update_error": "Last error",
            "available_update": "Available update",
            "no_update": "No update available",
            "install_progress": "Install progress",
            "install_complete": "Install complete"
        },
        "script": {
            "update_success": "Successfully updated; restarting...",
            "update_fail": "Failed to update",
            "check_state_2": "Another operation is in progress",
            "check_state_3": "Internal error occurred",
            "check_state_4": "No update URL configured",
            "check_state_5": "HTTPS certificate not available",
            "check_state_6": "HTTPS client initialization failed",
            "check_state_7": "Update server did not respond",
            "check_state_8": "Download error occurred",
            "check_state_9": "Version number is malformed",
            "check_state_10": "Firmware index is malformed",
            "check_state_11": "Aborted",
            "check_state_12": "Download short read occurred",
            "check_state_13": "Check for update is not supported",
            "install_state_2": "Another operation is in progress",
            "install_state_3": "Internal error occurred",
            "install_state_4": "No update URL configured",
            "install_state_5": "HTTPS certificate not available",
            "install_state_6": "HTTPS client initialization failed",
            "install_state_7": "Update server did not respond",
            "install_state_8": "Download error occurred",
            "install_state_9": "Version number is malformed",
            "install_state_10": "Firmware file is too small",
            "install_state_11": "Flash initialization failed",
            "install_state_12": "Flash short write occurred",
            "install_state_13": "Flash apply failed",
            "install_state_14": "Signature initialization failed",
            "install_state_15": "Signature calculation failed",
            "install_state_16": "Signature verification failed",
            "install_state_17": "Can't update the firmware while a vehicle is connected.",
            "install_state_18": "Info page is too big",
            "install_state_19": null,
            "install_state_20": "Firmware file corrupted (checksum error)",
            "install_state_21": null,
            "install_state_23": "Aborted",
            "install_state_24": "Rebooting",
            "install_state_25": "Download short read occurred",
            "install_state_26": "Installation is not supported",
            "install_state_27": "Firmware file size is unknown",
            "build_time": /*SFN*/(build_time: string) => `created ${build_time}`/*NF*/,
            "publisher": /*SFN*/(publisher: string) => `by ${publisher}`/*NF*/,
            "install_failed": "Install failed"
        }
    }
}
