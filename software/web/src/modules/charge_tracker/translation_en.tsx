/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
import { toLocaleFixed } from "../../ts/util";
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
            "download": "Download charge-log",
            "download_btn": "Download charge-log",

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

            "charge_tracker_remove_modal_text": <>All tracked charges and the username history will be removed. <b>This action cannot be undone!</b> {__("the_device")} then will reboot.</>,
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
            "csv_flavor_rfc4180": "According to RFC4180 (, as field separator, UTF-8 encoded)",

            "user_filter_label": "User filter",
            "file_type_label": "File type",
            "language_label": "Language",
            "language_german": "German",
            "language_english": "English",
            "letterhead_label": "Letterhead",
            "target_user": "Remote access user",
            "charge_log_email_send_config": "Email delivery",
            "charge_log_email_send_add_modal_title": "Add email delivery",
            "charge_log_email_send_edit_modal_title": "Edit email delivery",
            "charge_log_email_send_to_user": "Send via email to",
            "charge_log_send_add_message": /*SFN*/(have: number, max: number) => `${have} of ${max} users configured`/*NF*/,
            "next_send": "Next send",
            "never": "Never",
            "test_send": "Test Send",
            "sending": "Sending...",
            "charge_log_email_send_config_help": "Email sending provides the ability to send the charge-logs on the 1st of each month to remote access users. Due to technical limitations, the charge-logs are not end-to-end encrypted."
        },
        "automation": {
            "charge_tracker_reset": "Remove tracked charges",
            "automation_action_text": "remove all tracked charges and reboot."
        },
        "script": {
            "tracked_charge_count": /*SFN*/ (tracked: number, max_: number) => `${tracked} of maximum ${max_} (${toLocaleFixed(100 * tracked / max_)} %)` /*NF*/,

            "unknown_user": "Unknown user",
            "unknown_users": "Unknown users",
            "deleted_user": "Deleted user",
            "deleted_users": "Deleted users",
            "all_users": "All users",
            "unknown_charge_start": "Unknown start time",

            "remove_failed": "Failed to remove tracked charges",
            "remove_init": "Removing tracked charges and rebooting...",
            "download_charge_log_failed": "Failed to download tracked charges",
            "upload_charge_log_success": "Charge-log sending was successful",
            "upload_charge_log_failed": "Failed to send charge-log",
            "test_charge_log_upload_started": "Test charge log upload started",

            "save_failed": "Failed to save the charge tracker settings."
        }
    }
}
