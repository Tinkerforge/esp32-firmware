/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "ove_r37": {
        "navbar": {
            "ove_r37": "OVE R 37"
        },
        "content": {
            "ove_r37": "OVE-Richtlinie R 37",
            "enabled": "OVE R 37 enabled",
            "enabled_muted": "Grid support per OVE R 37 (Austria)",
            "enabled_desc": "Enable the OVE R 37 grid support functions (undervoltage trip, reconnect conditions, phase symmetry).",
            "undervoltage_threshold": "Undervoltage trip threshold",
            "undervoltage_threshold_muted": "(at 230 V nominal)",
            "undervoltage_threshold_help": <>
                <p>Charging is stopped (tripped) when a phase voltage stays below this threshold for longer than the observation time (OVE R 37 §5.9.8). The threshold is given in 1/1000 pu of the 230 V nominal voltage, so 800 = 0.80 pu ≈ 184 V.</p>
            </>,
            "undervoltage_observation_time": "Undervoltage observation time",
            "undervoltage_observation_time_muted": "Time below the threshold before tripping (default 3000 ms)",
            "reconnect_wait_time": "Reconnect wait time",
            "reconnect_wait_time_muted": "Wait time before resuming charging after a trip (0…300 s, default 60 s)",
            "password": "Electrician password",
            "password_muted": "Required to change the OVE R 37 configuration",
            "password_placeholder": "Electrician password",
            "password_required": "The electrician password is required to save the configuration.",
            "state_disabled": "Disabled",
            "state_normal": "Normal",
            "state_tripped": "Tripped",
            "state_wait": "Waiting for reconnect",
            "state_ramp": "Ramping up",
            "state_boot": "Waiting after restart",
            "trip_none": "None",
            "trip_undervoltage": "Undervoltage",
            "trip_overvoltage": "Overvoltage",
            "trip_frequency": "Frequency"
        },
        "status": {
            "state": "OVE R 37 state",
            "trip_reason": "Trip reason",
            "start_delay": "Charge start delay",
            "start_delay_active": "Active",
            "start_delay_help": <>
                <p>Charging programs with a preset start time (e.g. an automation rule with a scheduled charge start) start charging with a random delay of 0-300 s (OVE R 37 §5.9.2).</p>
                <p>The delay is currently active, charging will start automatically shortly.</p>
            </>,
            "voltage_in_range": "Voltage in range",
            "voltage_in_range_muted": "0.9-1.1 pu (207-253 V)",
            "voltage_in_range_help": <>
                <p>Whether all connected phase voltages are within the 0.9-1.1 pu continuous-operation band (OVE R 37 §5.9.9), i.e. 207-253 V at 230 V nominal.</p>
                <p>This is independent of the undervoltage trip threshold: a voltage can be out of this range (e.g. 197 V) without tripping. The charge is only stopped once the voltage falls below the trip threshold for the observation time.</p>
            </>,
            "frequency_in_range": "Frequency in range",
            "frequency_in_range_muted": "49.9–50.1 Hz",
            "frequency_in_range_help": <>
                <p>Whether the mains frequency is within the 49.9-50.1 Hz reconnect window (OVE R 37 §5.7.4.2). This band determines whether charging may be (re)connected after a trip.</p>
                <p>The frequency itself never trips an active charge (charging loads have no active frequency-response requirement yet).</p>
            </>,
            "yes": "Yes",
            "no": "No",
            "no_data": "No data"
        },
        "script": {
            "save_failed": "Failed to save the OVE R 37 configuration"
        }
    }
}
