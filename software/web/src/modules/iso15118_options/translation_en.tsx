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
            "ignore_soc_compatibility_help": "Ignore vehicle compatibility exclusions for SoC reading. The normal Read state of charge setting must still be enabled.",
            "prefer_din": "Prefer DIN 70121",
            "prefer_din_desc": "Use DIN 70121 instead of ISO 15118-2 to read SoC",
            "prefer_din_help": "If the vehicle supports both protocols, read the state of charge via DIN 70121 instead of ISO 15118-2. Only applies if Read state of charge is enabled and Charge via ISO 15118 is disabled.",
            "shutdown_omit_schedule": "No schedule on stop",
            "shutdown_omit_schedule_desc": "Omit SAScheduleList when ending the SoC session",
            "shutdown_omit_schedule_help": "Do not send an SAScheduleList (power schedule) in the response that ends the DC state-of-charge session. With DIN 70121 an empty schedule element is sent instead, because the protocol requires one.",
            "shutdown_nonzero_pmax": "Non-zero power in schedule",
            "shutdown_nonzero_pmax_desc": "Use 22 kW instead of 0 W as PMax when ending the SoC session",
            "shutdown_nonzero_pmax_help": "Set the maximum power (PMax) in the SAScheduleList of the response that ends the DC state-of-charge session to 22 kW instead of 0 W. Some vehicles may treat 0 W as charging not being allowed for 24 hours. Has no effect if No schedule on stop is enabled."
        },
        "script": {
            "experimental_save_failed": "Failed to save the experimental ISO15118 settings."
        }
    }
}
