//APIPath:evse/

export interface state {
    iec61851_state: number,
    charger_state: number,
    contactor_state: number,
    contactor_error: number,
    allowed_charging_current: number,
    error_state: number,
    lock_state: number,
    dc_fault_current_state: number,
}

export interface hardware_configuration {
    jumper_configuration: number,
    has_lock_switch: boolean,
    evse_version: number,
    energy_meter_type: number,
}

export interface gpio_configuration {
    shutdown_input: number,
    input: number,
    output: number
}

export interface button_configuration {
    button: number
}

export interface control_pilot_configuration {
    control_pilot: number
}

export interface control_pilot_connected {
    connected: boolean
}

export interface reset_dc_fault_current_state {
    password: number
}
