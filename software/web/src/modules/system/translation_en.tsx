/** @jsxImportSource preact */
import { h } from "preact";
import { __, removeUnicodeHacks } from "../../ts/translation";
let x = {
    "system": {
        "status": {
            "system": "System",
            "last_reset": /*FFN*/(reason: number, version: string, display_type: string) => {
                let reason_table = [
                    "due to unknown cause",
                    "due to power-on",
                    "by external pin",
                    "by software reset",
                    "due to software crash",
                    "due to interrupt watchdog",
                    "due to task watchdog",
                    "due to other watchdog",
                    "after exiting deep sleep mode",
                    "due to brownout",
                    "by SDIO",
                    "by USB peripheral",
                    "by JTAG",
                    "due to eFuse error",
                    "due to power glitch detection",
                    "due to CPU lock up",
                ];

                let reason_str = reason_table[reason];

                if (reason_str === undefined) {
                    reason_str = reason_table[0];
                }

                let result = [<>__("The_device") was rebooted unexpectedly {reason_str}.</>];

                if ("{{{support_email}}}".length > 0) {
                    result.push(<> Please download a <a href="#event_log">debug report</a> and send it to <a href={removeUnicodeHacks(`mailto:{{{support_email}}}?subject=${display_type} with firmware ${version} was rebooted unexpectedly`)}>{{{support_email}}}</a>.</>);
                }

                return <>{result}</>;
            }/*NF*/,
            "hide_last_reset_warning_failed": "Clearing the warning failed"
        },
        "navbar": {
            "system": "Settings"
        },
        "content": {
            "system": "System",

            "factory_reset_desc": null,
            "factory_reset": "Factory reset",
            "factory_reset_modal_text": null,

            "config_reset_desc": null,
            "config_reset": "Reset configuration",

            "config_reset_modal_text": null,
            "confirm_config_reset": "Reset configuration",

            "abort_reset": "Abort",
            "confirm_factory_reset": "Reset to factory defaults",
            "reboot": "Reboot",
            "reboot_desc": null,

            "system_language": "System language",
            "system_language_help": "Sets the system language to the specified language. If \"Auto-detect browser language\" is disabled, the browser's language settings are ignored and the specified system language is also used on the website.",
            "german": "German",
            "english": "English",
            "detect_browser_language": "Auto-detect browser language"
        },
        "script": {
            "factory_reset_init": "Formatting configuration partition and restarting...",
            "factory_reset_error": "Initiating factory reset failed",

            "config_reset_init": "Resetting configuration and restarting...",
            "config_reset_error": "Failed to reset configuration",

            "save_failed": "Failed to save the system settings"
        }
    }
}
