/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "iso15118": {
        "content": {
            "experimental_settings": "Experimental vehicle compatibility",
            "experimental_warning": <><p>These experimental settings and their API are unstable and may change or be removed in future firmware versions.</p></>,
            "experimental_unplug": "Experimental settings are locked until the vehicle is confirmed unplugged.",
            "ef_teardown": "E/F reset",
            "ef_teardown_desc": "Reset CP before AC charging",
            "ef_teardown_help": "Apply an E/F control-pilot reset before offering normal AC charging. This also affects fallback after Autocharge without SoC reading.",
            "nonegotiation_autocharge": "Autocharge variant",
            "nonegotiation_autocharge_desc": "Reject protocol negotiation",
            "nonegotiation_autocharge_help": "When Autocharge is used without SoC reading, including when SoC reading is skipped for compatibility, deliberately reject protocol negotiation instead of stopping after vehicle identification. Also applies to the second communication round when Reconnect is enabled, even without Autocharge.",
            "nonegotiation_after_soc": "Reconnect",
            "nonegotiation_after_soc_desc": "Reconnect after reading SoC",
            "nonegotiation_after_soc_help": "After the DC state-of-charge session, attempt a new communication round to test AC fallback. Stop at SLAC matching without confirming the match. If Autocharge variant is enabled, complete the connection and reply with Failed_NoNegotiation instead.",
            "ignore_soc_compatibility": "Always read SoC",
            "ignore_soc_compatibility_desc": "Ignore compatibility restrictions",
            "ignore_soc_compatibility_help": "Ignore vehicle compatibility exclusions for SoC reading. The normal Read state of charge setting must still be enabled."
        },
        "script": {
            "experimental_save_failed": "Failed to save the experimental ISO15118 settings."
        }
    }
}
