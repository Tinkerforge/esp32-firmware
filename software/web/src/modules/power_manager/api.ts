export interface state {
    config_error_flags: number;
    external_control: number;
}

export interface low_level_state {
    power_at_meter: number;
    power_at_battery: number;
    power_available: number;
    i_meter: number[];
    i_pp_max: number[];
    i_pp_mavg: number[];
    i_pp: number[];
    overall_min_power: number;
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
    meter_slot_battery_power: number;
    battery_mode: number;
    battery_inverted: boolean;
    battery_deadzone: number;
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
