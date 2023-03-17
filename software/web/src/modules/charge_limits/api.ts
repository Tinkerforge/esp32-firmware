export interface default_limits {
    duration: number
    energy_wh: number
}

export interface active_limits {
    duration: number
    energy_wh: number
}

export interface state {
    start_timestamp_ms: number
    start_energy_kwh: number
    target_timestamp_ms: number
    target_energy_kwh: number
}

export interface override_duration {
    duration: number
}

export interface override_energy {
    energy_wh: number
}
