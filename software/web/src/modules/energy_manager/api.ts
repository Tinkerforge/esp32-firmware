export interface state {
    error_flags: number,
    contactor: boolean,
    led_rgb: number[],
    gpio_input_state: boolean[],
    gpio_output_state: boolean,
    input_voltage: number,
    contactor_check_state: number,
    energy_meter_type: number,
    energy_meter_power: number,
    energy_meter_energy_import: number,
    energy_meter_energy_export: number,
}

export interface config {
    default_mode: number,
    auto_reset_mode: boolean,
    auto_reset_time: number,
    excess_charging_enable: boolean,
    contactor_installed: boolean,
    phase_switching_mode: number,
    target_power_from_grid: number,
    guaranteed_power: number,
    hysteresis_time: number,
    hysteresis_wear_accepted: boolean,
    relay_config: number,
    relay_config_when: number,
    relay_config_is: number,
    input3_config: number,
    input3_config_limit: number,
    input3_config_when: number,
    input3_config_rising_mode: number,
    input3_config_falling_mode: number,
    input4_config: number,
    input4_config_limit: number,
    input4_config_when: number,
    input4_config_rising_mode: number,
    input4_config_falling_mode: number
}

export interface runtime_config {
    mode: number
}

export type debug_header = string;
export type debug = string;
