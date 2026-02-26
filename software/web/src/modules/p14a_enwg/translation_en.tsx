/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "p14a_enwg": {
        "status": {
            "status": "Status",
            "active": "Active",
            "inactive": "Inactive",
            "current_limit": "Current limit",
            "alert_active": /*SFN*/(limit: number) => "Power limited to " + limit + " W"/*NF*/
        },
        "navbar": {
            "p14a_enwg": "14a EnWG"
        },
        "content": {
            "p14a_enwg": "§14a EnWG",
            "enable_p14a_enwg": "§14a EnWG enabled",
            "enable_p14a_enwg_desc": "Enable §14a EnWG control",
            "enable_p14a_enwg_help": <>
                <p>§14a of the German Energy Industry Act (EnWG) allows grid operators to temporarily reduce the power consumption of controllable consumer devices (heat pumps, wallboxes, battery storage, air conditioning) to prevent local grid overload. The devices are never fully shut off. A minimum of 4200 W always remains available.</p>
                <p>This regulation is primarily relevant for the German market.</p>
            </>,
            "source": "Signal source",
            "source_shutdown_input": "Charger shutdown input",
            "source_input": /*SFN*/(device_name: string) => device_name + " input"/*NF*/,
            "source_eebus": "EEBUS",
            "source_api": "API",
            "input": "Input",
            "limit": "Device count",
            "limit_muted": "Number of controllable consumer devices",
            "limit_help": <>
                <p>The default minimum power for a single controllable consumer device is 4200 W.</p>
                <p>For multiple devices controlled via an energy management system (EMS), the minimum power is calculated as:</p>
                <p><strong>4200 W + (number of devices - 1) &times; simultaneity factor &times; 4200 W</strong></p>
                <p>Simultaneity factors by number of devices:</p>
                <ul>
                    <li>2 devices: 0.80</li>
                    <li>3 devices: 0.75</li>
                    <li>4 devices: 0.70</li>
                    <li>5 devices: 0.65</li>
                    <li>6 devices: 0.60</li>
                    <li>7 devices: 0.55</li>
                    <li>8 devices: 0.50</li>
                    <li>9+ devices: 0.45</li>
                </ul>
                <p>Example for 3 devices: 4200 W + (3 - 1) &times; 0.75 &times; 4200 W = 10500 W</p>
            </>,
            "device_count_unit": /*SFN*/(n: number, limit: number) => n + (n === 1 ? " device" : " devices") + " (" + limit + " W)"/*NF*/,
            "active_on": "Active on",
            "active_on_close": "Closed",
            "active_on_open": "Open",
            "this_charger": "This charger",
            "this_charger_desc": "Apply §14a power limit to this charger",
            "managed_chargers": "Managed chargers",
            "managed_chargers_desc": "Apply §14a power limit to managed chargers",
            "heating": "Heating",
            "heating_desc": "Apply §14a power limit to heating",
            "heating_max_power": "Heating max. power",
            "heating_max_power_muted": "Maximum power draw of the heating system"
        },
        "script": {
            "save_failed": "Failed to save the §14a EnWG settings"
        }
    }
}
