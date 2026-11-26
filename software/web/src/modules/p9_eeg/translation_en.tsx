/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "p9_eeg": {
        "status": {
            "status": "Status",
            "not_limited": "Not limited (100 %)",
            "limited": /*SFN*/(level: number) => "Feed-in limited to " + level + " %"/*NF*/,
            "default_active": /*SFN*/(level: number) => "Default level active (" + level + " %)"/*NF*/,
            "requested_limit": "Requested limit",
            "limit_not_reachable": "The requested limit is below the lowest available level."
        },
        "navbar": {
            "p9_eeg": "9 EEG"
        },
        "content": {
            "p9_eeg": "§9 EEG",
            "enable_p9_eeg": "§9 EEG enabled",
            "enable_p9_eeg_desc": "Power limits of the grid operator are passed to the inverter via the relays.",
            "enable_p9_eeg_help": <>
                <p>§9 of the German Renewable Energy Sources Act (EEG) obliges operators of PV systems to allow the grid operator to reduce the feed-in power.</p>
                <p>The limit of the grid operator is received and mapped to one of the levels 0 %, 30 %, 60 % or 100 % of the inverter's rated power. The level is passed via the relays to the inverter's input for ripple control receivers (often called DRM or RSE). The highest level that complies with the limit is always used.</p>
                <p>This regulation is primarily relevant for the German market.</p>
            </>,
            "source": "Signal source",
            "source_help": <>
                <p><strong>EEBUS</strong>: The limit is received from the grid operator's control box via the EEBUS use case "Limitation of Power Production" (LPP). The control box can change the failsafe limit and the failsafe duration.</p>
                <p><strong>API</strong>: The limit is set via the API (p9_eeg/control_update).</p>
            </>,
            "source_eebus": "EEBUS",
            "source_api": "API",
            "eebus_needs_activation": <>To be able to use EEBUS as signal source, <a href="#eebus">EEBUS</a> must be enabled.</>,
            "inverter_power": "Inverter rated power",
            "inverter_power_muted": "AC rated power",
            "inverter_power_help": <>
                <p>The levels of the inverter refer to its AC rated power, not to the installed module power (kWp). The rated power is listed in the inverter's datasheet.</p>
            </>,
            "relay": "Relay",
            "relay_n": /*SFN*/(n: number) => "Relay " + n/*NF*/,
            "relay_muted": "Level while the relay is closed",
            "relay_help": <>
                <p>Each relay is connected to the inverter's input for one level (for example K3 for 30 % or K4 for 0 %). At most one relay is closed at a time.</p>
                <p>If no relay is closed, 100 % always applies. The inverter must be configured to feed in with full power if no contact is closed.</p>
                <p>Relays used by §9 EEG can no longer be switched by automation rules.</p>
            </>,
            "relay_unused": "Not used",
            "relay_duplicate": "This level is already assigned to another relay.",
            "level_with_power": /*SFN*/(level: number, power_w: number) => level + " % (" + power_w + " W)"/*NF*/,
            "default_level": "Default level",
            "default_level_muted": "As long as the signal source has not provided a limit",
            "default_level_help": <>
                <p>With the API the default level applies after a reboot until the first call of p9_eeg/control_update.</p>
                <p>With EEBUS the default level applies while EEBUS is disabled or still starting. It is also the pre-configured failsafe limit of the LPP use case. This limit applies after a reboot and if the connection to the control box is lost. If the control box has written its own failsafe limit, that limit has priority.</p>
            </>,
            "no_relay_used": "At least one relay must be used."
        },
        "script": {
            "save_failed": "Failed to save the §9 EEG settings"
        },
        "automation": {
            "automation_trigger": "§9 EEG (feed-in limitation)",
            "state": "State",
            "limited": "Limited",
            "not_limited": "Not limited",
            "trigger_disabled": "§9 EEG is disabled",
            "automation_trigger_text": /*FFN*/(limited: boolean) => limited
                ? <>If <b>§9 EEG</b> <b>limits</b> the feed-in,{" "}</>
                : <>If <b>§9 EEG</b> <b>no longer limits</b> the feed-in,{" "}</>/*NF*/
        }
    }
}
