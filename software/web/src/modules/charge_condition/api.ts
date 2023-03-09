export interface config {
    duration_limit: number
    energy_limit_kwh: number
}

export interface live_config {
    duration_limit: number
    energy_limit_kwh: number
}

export interface state {
    start_timestamp_mil: number
    start_energy_kwh: number
    target_timestamp_mil: number
    target_energy_kwh: number
}

export interface override_duration {
    duration: number
}

export interface override_energy {
    energy: number
}
