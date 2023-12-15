export interface low_level_state {
    power_at_meter: number;
    power_at_meter_filtered: number;
    power_available: number;
    power_available_filtered: number;
    overall_min_power: number;
    threshold_3to1: number;
    threshold_1to3: number;
    charge_manager_available_current: number;
    charge_manager_allocated_current: number;
    max_current_limited: number;
    uptime_past_hysteresis: boolean;
    is_3phase: boolean;
    wants_3phase: boolean;
    wants_3phase_last: boolean;
    is_on_last: boolean;
    wants_on_last: boolean;
    phase_state_change_blocked: boolean;
    phase_state_change_delay: number;
    on_state_change_blocked: boolean;
    on_state_change_delay: number;
    charging_blocked: number;
    switching_state: number;
}

export interface config {
    default_mode: number;
    excess_charging_enable: boolean;
    meter_slot_grid_power: number;
    target_power_from_grid: number;
    guaranteed_power: number;
    cloud_filter_mode: number;
}

export interface debug_config {
    hysteresis_time: number;
}

export interface charge_mode {
    mode: number;
}
