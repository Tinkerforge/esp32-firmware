export interface config {
    duration_limit: number
    energy_limit_kwh: number
    time_restriction_enabled: boolean
    allowed_times: boolean[][]
}

export interface state {
    start_time_mil: number
    start_energy_kwh: number
    duration_left_sec: number
    energy_left_kwh: number
}