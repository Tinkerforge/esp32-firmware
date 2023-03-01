export interface low_level_state {
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
    phase_switched: number,
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
    contactor_installed: boolean,
    phase_switching_mode: number,
    guaranteed_power: number,
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
    target_power_from_grid: number,
}

export interface charge_mode {
    mode: number
}

export type debug_header = string;
export type debug = string;
