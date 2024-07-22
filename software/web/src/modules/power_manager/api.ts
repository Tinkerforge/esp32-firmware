export interface state {
    config_error_flags: number;
    external_control: number;
}

export interface low_level_state {
    power_at_meter: number;
    power_available: number;
    overall_min_power: number;
    charge_manager_available_current: number;
    charge_manager_allocated_current: number;
    max_current_limited: number;
    is_3phase: boolean;
    charging_blocked: number;
}

export interface config {
    enabled: boolean;
    phase_switching_mode: number;
    default_mode: number;
    excess_charging_enable: boolean;
    meter_slot_grid_power: number;
    target_power_from_grid: number;
    guaranteed_power: number;
    cloud_filter_mode: number;
}

export interface dynamic_load_config {
    enabled: boolean;
    meter_slot_grid_currents: number;
    current_limit: number;
    largest_consumer_current: number;
    safety_margin_pct: number;
}

export interface charge_mode {
    mode: number;
}

export interface external_control {
    phases_wanted: number;
}
