export interface low_level_state {
    power_at_meter: number,
    power_at_meter_filtered: number,
    power_available: number,
    power_available_filtered: number,
    overall_min_power: number,
    threshold_3to1: number,
    threshold_1to3: number,
    charge_manager_allocated_current: number,
    max_current_limited: number,
    uptime_past_hysteresis: boolean,
    is_3phase: boolean,
    wants_3phase: boolean,
    wants_3phase_last: boolean,
    is_on_last: boolean,
    wants_on_last: boolean,
    phase_state_change_blocked: boolean,
    phase_state_change_delay: number,
    on_state_change_blocked: boolean,
    on_state_change_delay: number,
    charging_blocked: number,
    switching_state: number,
    consecutive_bricklet_errors: number,

    contactor: boolean,
    led_rgb: number[],
    input_voltage: number,
    contactor_check_state: number,
}

export interface meter_state {
    energy_meter_type: number,
    energy_meter_power: number,
    energy_meter_energy_import: number,
    energy_meter_energy_export: number,
}

export interface state {
    phases_switched: number,
    error_flags: number,
    input3_state: boolean,
    input4_state: boolean,
    relay_state: boolean,
}

export interface config {
    default_mode: number,
    auto_reset_mode: boolean,
    auto_reset_time: number,
    excess_charging_enable: boolean,
    target_power_from_grid: number,
    guaranteed_power: number,
    cloud_filter_mode: number,
    contactor_installed: boolean,
    phase_switching_mode: number,
    relay_config: number,
    relay_rule_when: number,
    relay_rule_is: number,
    input3_rule_then: number,
    input3_rule_then_limit: number,
    input3_rule_is: number,
    input3_rule_then_on_high: number,
    input3_rule_then_on_low: number,
    input4_rule_then: number,
    input4_rule_then_limit: number,
    input4_rule_is: number,
    input4_rule_then_on_high: number,
    input4_rule_then_on_low: number
}

export interface debug_config {
    hysteresis_time: number,
}

export interface charge_mode {
    mode: number
}

export interface history_wallbox_5min_changed {
    uid: number,
    year: number,
    month: number,
    day: number,
    hour: number,
    minute: number,
    flags: number,
    power: number
}

export interface history_wallbox_daily_changed {
    uid: number,
    year: number,
    month: number,
    day: number,
    energy: number
}

export interface history_energy_manager_5min_changed {
    year: number,
    month: number,
    day: number,
    hour: number,
    minute: number,
    flags: number,
    power_grid: number,
    power_general: number[]
}

export interface history_energy_manager_daily_changed {
    year: number,
    month: number,
    day: number,
    energy_grid_in: number,
    energy_grid_out: number,
    energy_general_in: number[],
    energy_general_out: number[]
}

export type debug_header = string;
export type debug = string;
