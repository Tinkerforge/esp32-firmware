export interface state {
    contactor: boolean,
    led_rgb: Uint8Array,
    gpio_input_state: boolean[],
    gpio_output_state: boolean,
    gpio_input_configuration: Uint8Array,
    input_voltage: number,
    contactor_check_state: number,
    energy_meter_type: number,
    energy_meter_power: number,
    energy_meter_energy_rel: number,
    energy_meter_energy_abs: number,
    energy_meter_phases_active: boolean[],
    energy_meter_phases_connected: boolean[]
}

export interface config {
    excess_charging_enable: boolean,
    phase_switching: number,
    mains_power_reception: number,
    minimum_charging: number,
    relay_config: number,
    relay_config_if: number,
    relay_config_is: number,
    relay_config_then: number,
    input3_config: number,
    input3_config_if: number,
    input3_config_then: number,
    input4_config: number,
    input4_config_if: number,
    input4_config_then: number

}

export type debug_header = string;
export type debug = string;