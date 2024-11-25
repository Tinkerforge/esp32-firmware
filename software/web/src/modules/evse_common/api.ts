//APIPath:evse/

interface Slot {
    max_current: number;
    active: boolean;
    clear_on_disconnect: boolean;
}

export const EVSE_SLOT_INCOMING_CABLE = 0;
export const EVSE_SLOT_OUTGOING_CABLE = 1;
export const EVSE_SLOT_SHUTDOWN_INPUT = 2;
export const EVSE_SLOT_GP_INPUT = 3;
export const EVSE_SLOT_AUTOSTART_BUTTON = 4;
export const EVSE_SLOT_GLOBAL = 5;
export const EVSE_SLOT_USER = 6;
export const EVSE_SLOT_CHARGE_MANAGER = 7;
export const EVSE_SLOT_EXTERNAL = 8;
export const EVSE_SLOT_MODBUS_TCP = 9;
export const EVSE_SLOT_MODBUS_TCP_ENABLE = 10;
export const EVSE_SLOT_OCPP = 11;
export const EVSE_SLOT_CHARGE_LIMITS = 12;
export const EVSE_SLOT_REQUIRE_METER = 13;
export const EVSE_SLOT_AUTOMATION = 14;

export type slots = Slot[];

export interface button_state {
    button_press_time: number;
    button_release_time: number;
    button_pressed: boolean;
}

export interface indicator_led {
    indication: number;
    duration: number;
}

export interface low_level_state {
    led_state: number;
    cp_pwm_duty_cycle: number;
    adc_values: number[];
    voltages: number[];
    resistances: number[];
    gpio: boolean[];
    charging_time: number;
    time_since_state_change: number;
    uptime: number;
    time_since_dc_fault_check: number;
    temperature?: number;
    phases_current?: number;
    phases_requested?: number;
    phases_state?: number;
    dc_fault_pins?: number;
    dc_fault_sensor_type?: number;
}

export interface external_current {
    current: number;
}

export interface external_clear_on_disconnect {
    clear_on_disconnect: boolean;
}

export interface management_current {
    current: number;
}

export interface auto_start_charging {
    auto_start_charging: boolean;
}

export interface global_current {
    current: number;
}

export interface management_enabled {
    enabled: boolean;
}

export interface user_enabled {
    enabled: boolean;
}

export interface external_defaults {
    current: number,
    clear_on_disconnect: boolean
}

export interface automation_current {
    current: number;
}

export interface start_charging {

}

export interface stop_charging {

}

export interface reflash {

}

export interface reset {

}

export interface boost_mode {
    enabled: boolean;
}

export interface identity {
    uid: string;
    connected_uid: string;
    position: string;
    hw_version: string;
    fw_version: string;
    device_identifier: number;
}

export interface led_configuration {
    enable_api: boolean;
}

export interface state {
    iec61851_state: number;
    charger_state: number;
    contactor_state: number;
    contactor_error: number;
    allowed_charging_current: number;
    error_state: number;
    lock_state: number;
    dc_fault_current_state?: number;
}

export interface hardware_configuration {
    jumper_configuration: number;
    has_lock_switch: boolean;
    evse_version: number;
    energy_meter_type?: number;
}

export interface gpio_configuration {
    shutdown_input: number;
    input: number;
    output: number;
}

export interface button_configuration {
    button: number;
}

export interface ev_wakeup {
    enabled: boolean;
}

export interface phase_auto_switch {
    enabled: boolean;
}

export interface phases_connected {
    phases: number;
}

export interface control_pilot_disconnect {
    disconnect: boolean;
}

export interface reset_dc_fault_current_state {
    password: number;
}

export interface trigger_dc_fault_test {}

export interface user_calibration {
    user_calibration_active: boolean;
    voltage_diff: number;
    voltage_mul: number;
    voltage_div: number;
    resistance_2700: number;
    resistance_880: number[];
}

export interface debug_switch_to_one_phase {}
export interface debug_switch_to_three_phases {}
