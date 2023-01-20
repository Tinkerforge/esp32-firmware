export interface state {
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
    excess_charging_enable: boolean,
    contactor_installed: boolean,
    phase_switching_mode: number,
    target_power_from_grid: number,
    maximum_available_current: number,
    minimum_current: number,
    guaranteed_power: number,
    hysteresis_time: number,
    hysteresis_wear_accepted: boolean,
    relay_config: number,
    relay_config_when: number,
    relay_config_is: number,
    input3_config: number,
    input3_config_limit: number,
    input3_config_when: number,
    input4_config: number,
    input4_config_limit: number,
    input4_config_when: number
}

export type debug_header = string;
export type debug = string;
