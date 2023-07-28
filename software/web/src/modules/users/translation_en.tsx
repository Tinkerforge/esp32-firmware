/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "users": {
        "status": {
        },
        "navbar": {
            "users": "User Management"
        },
        "content": {
            "users": "User Management",

            "enable_authentication": "Enable login",
            "enable_authentication_desc": "A successful login as one of the configured users is required to open the Web interface or call the HTTP API",
            "enable_authentication_invalid": "To enable the login at least one user with a configured password is required.",

            "authorized_users": "Authorized users",

            "unknown_username": "Unknown user display name",

            "table_username": "Username",
            "table_display_name": "Display name",
            "table_current": "Maximum charge current",
            "table_password": "Password",

            "add_user_title": "Add user",
            "add_user_username": "Username",
            "add_user_username_desc": "To log into the web interface",
            "add_user_display_name": "Display name",
            "add_user_display_name_desc": "Shown in web interface and charge log",
            "add_user_current": "Maximum charge current",
            "add_user_password": "Password",
            "add_user_password_desc": "Login disabled",
            "add_user_prefix": "",
            "add_user_infix": " of ",
            "add_user_suffix": " users configured",
            "add_user_user_ids_exhausted": "The maximum number of users has tracked charges. To be able to add a new user, tracked charges or a user without tracked charges have to be removed.",

            "edit_user_title": "Edit user",
            "edit_user_username": "Username",
            "edit_user_display_name": "Display name",
            "edit_user_current": "Maximum charge current",
            "edit_user_password": "Password",

            "evse_user_description": "Charge release",
            "evse_user_description_muted": <><a href="{{{manual_url}}}">see manual for details</a></>,

            "evse_user_enable": "Requires a user authorization (via NFC card) to charge",
            "evse_user_enable_invalid": "At least one user is required for the charge_release."
        },
        "script": {
            "reboot_content_changed": "User configuration",
            "login_disabled": "Login disabled",
            "save_failed": "Failed to save the user configuration.",
            "username_already_tracked": "Username is already in tracked charges",
            "username_already_used": "Username is already in use",
            "all_user_ids_in_use": "All user-ids in use."
        }
    }
}
