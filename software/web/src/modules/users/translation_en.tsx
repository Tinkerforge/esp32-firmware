/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "users": {
        "status": {},
        "navbar": {
            "users": "User Management"
        },
        "content": {
            "users": "User Management",

            "enable_authentication": "Login enabled",
            "enable_authentication_desc":
                "A successful login as one of the configured users is required to open the Web interface or call the HTTP API",
            "enable_authentication_invalid":
                "To enable the login at least one user with a configured password is required.",

            "authorized_users": "Authorized users",

            "unknown_username": "Unknown user display name",

            "table_username": "Username",
            "table_display_name": "Display name",
            "table_current": "Maximum charge current",
            "table_password": "Password",

            "add_user_title": "Add user",
            "add_user_username": "Username",
            "add_user_username_desc": "to log in to the web interface",
            "add_user_display_name": "Display name",
            "add_user_display_name_desc": "shown in web interface and charge log",
            "add_user_current": "Maximum charge current",
            "add_user_password": "Password",
            "add_user_password_desc": "Login disabled",
            "add_user_message": /*SFN*/ (have: number, max: number) =>
                `${have} of ${max} users configured` /*NF*/,
            "add_user_user_ids_exhausted":
                "The maximum number of users has tracked charges. To be able to add a new user, tracked charges or a user without tracked charges have to be removed.",

            "edit_user_title": "Edit user",
            "edit_user_username": "Username",
            "edit_user_username_desc": "to log in to the web interface",
            "edit_user_display_name": "Display name",
            "edit_user_display_name_desc":
                "shown in web interface and charge log",
            "edit_user_current": "Maximum charge current",
            "edit_user_password": "Password",

            "evse_user_description": "Charge release",

            "evse_user_enable": <>
                    Requires a user authorization (via NFC tag) to charge
                </>,
            "evse_user_enable_invalid":
                "At least one user is required for the charge release.",
            "evse_user_enable_central_auth_warning": <>Disabled because <a href="#charge_manager_chargers"> central authorization </a> is enabled in the charge manager.
                </>,

            "nfc_tags": "NFC Tags",
            "nfc_tag_id": "Tag ID",
            "nfc_tag_type": "Tag Type",
            "nfc_no_tags_assigned": "No NFC tags assigned",
            "nfc_no_seen_tags": "No NFC tags seen recently. Hold a tag to the charger to detect it.",
            "nfc_tag_already_assigned": /*SFN*/ (other_name: string) =>
                `Already assigned to user "${other_name}"` /*NF*/,
            "nfc_add_tag": "Add NFC Tag",
            "nfc_seen_tags": "Seen Tags",
            "nfc_last_seen": "Last seen",
            "nfc_add_tag_manually": "Add tag manually",
            "nfc_tag_id_placeholder": "e.g. 01:23:45:67",
            "nfc_tag_id_invalid":
                "The tag ID must have four to ten groups separated by colons, each with a two-digit hexadecimal number. For example 01:23:45:67:89:AB"
        },
        "script": {
            "reboot_content_changed": "user settings",
            "login_disabled": "Login disabled",
            "save_failed": "Failed to save the user settings.",
            "username_already_tracked": "Username is already in tracked charges",
            "username_already_used": "Username is already in use",
            "all_user_ids_in_use": "All user-ids in use."
        }
    }
}
