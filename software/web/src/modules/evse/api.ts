export interface state {
    iec61851_state: number,
    charger_state: number,
    contactor_state: number,
    contactor_error: number,
    allowed_charging_current: number,
    error_state: number,
    lock_state: number
}

export interface hardware_configuration {
    jumper_configuration: number,
    has_lock_switch: boolean,
    evse_version: number
}

export interface low_level_state {
    led_state: number,
    cp_pwm_duty_cycle: number,
    adc_values: Uint16Array,
    voltages: Int16Array,
    resistances: Uint32Array,
    gpio: boolean[],
    charging_time: number,
    time_since_state_change: number,
    uptime: number,
}

export interface auto_start_charging {
    auto_start_charging: boolean
}

export interface management_enabled {
    enabled: boolean;
}

export interface user_slot_enabled {
    enabled: boolean;
}

export interface external_enabled {
    enabled: boolean;
}

interface Slot {
    max_current: number,
    active: boolean,
    clear_on_disconnect: boolean
}

export type slots = Slot[];

export type debug_header = string;
export type debug = string;

export interface user_calibration {
    user_calibration_active: boolean,
    voltage_diff: number,
    voltage_mul: number,
    voltage_div: number,
    resistance_2700: number,
    resistance_880: number[],
}
