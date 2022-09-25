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

export interface user_calibration {
    user_calibration_active: boolean,
    voltage_diff: number,
    voltage_mul: number,
    voltage_div: number,
    resistance_2700: number,
    resistance_880: number[],
}
