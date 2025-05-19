/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "charge_tracker": {
        "status": {
            "last_charges": "Last charges",
            "current_charge": "Current charge"
        },
        "navbar": {
            "charge_tracker": "Charge Tracker"
        },
        "content": {
            "charge_tracker": "Charge Tracker",
            "charge_log_file": "charge-log",
            "download": "Download charge log",
            "download_desc": "",
            "download_btn": "Download charge log as CSV",
            "download_btn_pdf": "Download charge log as PDF",

            "pdf_letterhead": "PDF letterhead",
            "pdf_letterhead_muted": "",

            "tracked_charges": "Tracked charges",
            "tracked_charges_muted": "",
            "first_charge_timestamp": "Oldest tracked charge",
            "first_charge_timestamp_muted": "",
            "user_filter": "User filter",
            "user_filter_muted": "downloaded file will only contain charges of the selected user",

            "date_filter": "Time filter",
            "date_filter_muted": "downloaded file will only contain charges in the selected date range",
            "from": "from",
            "to": "to",
            "last_charges": "Last charges",
            "last_charges_desc": "up to 30 charges are shown",
            "remove": "Remove tracked charges",
            "remove_desc": "",
            "remove_btn": "Remove all tracked charges",

            "charge_tracker_remove_modal_text": <>All tracked charges and the username history will be removed. <b>This action cannot be undone!</b> {__("the_device")} then will restart.</>,
            "abort_remove": "Abort",
            "confirm_remove": "Delete",
            "price": "Electricity price",
            "price_invalid": "The electricity price must be set in cent per kWh, not euro per kWh.",
            "price_not_dynamic_yet": /*FFN*/(dap_enabled: boolean) => <>
                Charges are always tracked with the electricity price configured here.<br/>
                {dap_enabled ?
                    <>The dynamic pricing is <strong>not</strong> tracked!</>:
                    <>A dynamic pricing (currently not configured) will <strong>not</strong> be tracked.</>}<br/>
                <>The charged energy is always tracked completely. <strong>No</strong> distinction is
                  made based on its origin (grid, battery storage, PV, etc.)!</>
            </>/*NF*/,

            "file_type": "File format",
            "file_type_muted": "",
            "file_type_pdf": "PDF",
            "file_type_csv": "CSV",

            "csv_flavor": "CSV flavor",
            "csv_flavor_muted": "",
            "csv_flavor_excel": "Compatible to Excel (; as field separator, Windows-1252 encoded)",
            "csv_flavor_rfc4180": "According to RFC4180 (, as field separator, UTF-8 encoded)"
        },
        "automation": {
            "charge_tracker_reset": "Remove tracked charges",
            "automation_action_text": "remove all tracked charges and reboot."
        },
        "script": {
            "tracked_charge_count": /*SFN*/ (tracked: number, max_: number) => `${tracked} of maximum ${max_} (${Math.floor(100 * tracked / max_)} %)` /*NF*/,
            "csv_header_start": "Start time",
            "csv_header_display_name": "Display name",
            "csv_header_username": "Username",
            "csv_header_energy": "Charged energy in kWh",
            "csv_header_duration": "Charge duration in s",
            "csv_header_meter_start": "Meter reading start",
            "csv_header_meter_end": "Meter reading end",
            "csv_header_price": "Charging costs in € Working price ",

            "unknown_user": "Unknown user",
            "unknown_users": "Unknown users",
            "deleted_user": "Deleted user",
            "deleted_users": "Deleted users",
            "all_users": "All users",
            "unknown_charge_start": "Unknown start time",

            "remove_failed": "Failed to remove tracked charges",
            "remove_init": "Removing tracked charges and rebooting...",
            "download_charge_log_failed": "Failed to download tracked charges",
            "download_usernames_failed": "Failed to download the usernames used in tracked charges",

            "save_failed": "Failed to save the charge manager settings.",
            "reboot_content_changed": "charge tracker settings"
        }
    }
}
