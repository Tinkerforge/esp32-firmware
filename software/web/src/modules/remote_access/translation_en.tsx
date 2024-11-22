
/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "remote_access": {
        "navbar": {
            "remote_access": "Remote Access"
        },
        "content": {
            "remote_access": "Remote Access",
            "email": "Email address",
            "password": "Password",
            "password_muted": "only used for the registation process and will not be stored",
            "relay_host": "Relay server hostname",
            "relay_host_muted": "To switch to another relay-server all users all users must be removed",
            "relay_port": "Relay server port",
            "enable": "Remote access enabled",
            "enable_desc": /*FFN*/ (host: string) => <>A <a href={`https://${host}/`}>{host}</a> account is required for the remote access.</> /*NF*/,
            "cert": "TLS certificate",
            "not_used": "Embedded certificate-bundle",
            "status_modal_header": "Registration progess",
            "advanced_settings": "Advanced settings",
            "prepare_login": "Preparing login",
            "logging_in": "Logging in user",
            "prepare_encryption": "Preparing encryption",
            "registration": null,
            "login_failed": "Login failed:",
            "wrong_credentials": "Wrong username or password",
            "add_user": "Add user",
            "user": "Users",
            "user_add_message": /*SFN*/ (user_count: number, max_users: number) => `${user_count} of ${max_users} users configured.`/*NF*/,
            "all_users_in_use": "All users are currently in use",
            "note": "Note",
            "note_muted": /*FFN*/(host: string) => <>will be shown at <a href={`https://${host}/`} target="_blank">{host}</a>. Optional</>/*NF*/
        },
        "script": {
            "save_failed": "Failed to save the remote access settings.",
            "reboot_content_changed": "remote access settings"
        }
    }
}
