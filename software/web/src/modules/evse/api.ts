export interface state {
    iec61851_state: number,
    vehicle_state: number,
    contactor_state: number,
    contactor_error: number,
    charge_release: number,
    allowed_charging_current: number,
    error_state: number,
    lock_state: number,
    time_since_state_change: number
    uptime: number
}

export interface hardware_configuration {
    jumper_configuration: number,
    has_lock_switch: boolean,
}

export interface low_level_state {
    low_level_mode_enabled: boolean,
    led_state: number,
    cp_pwm_duty_cycle: number,
    adc_values: Uint16Array,
    voltages: Int16Array,
    resistances: Uint32Array,
    gpio: boolean[],
    hardware_version: number,
    charging_time: number
}

export interface max_charging_current {
    max_current_configured: number,
    max_current_incoming_cable: number,
    max_current_outgoing_cable: number,
    max_current_managed: number
}

export interface auto_start {
    auto_start_charging: boolean
}

export interface user_calibration {
    user_calibration_active: boolean,
    voltage_diff: number,
    voltage_mul: number,
    voltage_div: number,
    resistance_2700: number,
    resistance_880: number[],
}

export interface managed {
    managed: boolean;
}
