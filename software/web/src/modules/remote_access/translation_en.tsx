
/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "remote_access": {
        "status": {
            "remote_access": "Remote Access",
            "label_muted": /*SFN*/ (since: string) => `since ${since}`/*NF*/,
            "since_start": "since reboot",
            "connected": "Connected to server",
            "connected_to_clients": /*SFN*/ (clients: number) => `${clients} client${clients == 1 ? '' : 's'} connected` /*NF*/,
            "disconnected": "Disconnected",
            "disabled": "Disabled"
        },
        "navbar": {
            "remote_access": "Remote Access"
        },
        "content": {
            "remote_access": "Remote Access",
            "email": "Email address",
            "password": "Password",
            "password_muted": "only used for the registration process and will not be stored",
            "relay_host": "Relay server hostname or IP address",
            "relay_host_muted": "To switch to another relay server all users all user accounts must be removed",
            "relay_port": "Relay server port",
            "enable": "Remote access enabled",
            "enable_desc": /*FFN*/ (host: string) => <>A <a href={`https://${host}/`}>{host}</a> user account is required for the remote access.</> /*NF*/,
            "cert": "TLS certificate",
            "not_used": "Embedded certificate bundle",
            "status_modal_header": "Registration progess",
            "advanced_settings": "Advanced settings",
            "prepare_login": "Preparing login",
            "logging_in": "Logging in user",
            "prepare_encryption": "Preparing encryption",
            "registration": <>Registering {__("device")}</>,
            "login_failed": "Login failed:",
            "wrong_credentials": "Wrong username or password",
            "add_user": "Add user",
            "user": "Users",
            "user_add_message": /*SFN*/(have: number, max: number) => `${have} of ${max} users configured`/*NF*/,
            "note": "Note",
            "note_muted": /*FFN*/(host: string) => <>will be shown at <a href={`https://${host}/`} target="_blank">{host}</a>. Optional</>/*NF*/,
            "user_exists": "This user already exists",
            "token_corrupted": "Authorization token corrupted. Please remove and re-add.",
            "auth_token": "Authorization token",
            "auth_token_invalid": "Invalid authorization token",
            "auth_method": "Authorization method",
            "ping": "Ping",
            "start_ping": "Start ping",
            "stop_ping": "Stop ping",
            "start_ping_failed": "Starting ping failed",
            "stop_ping_failed": "Stopping ping failed",
            "packets_sent": "Packets sent",
            "packets_received": "Packets received",
            "time_elapsed": "Elapsed time"
        },
        "script": {
            "save_failed": "Failed to save the remote access settings.",
            "reboot_content_changed": "remote access settings",
            "http_client_error_10": <>Can't reach relay server. Is {__("the_device")} connected to the internet? Is the relay server configuration correct?</>
        }
    }
}
