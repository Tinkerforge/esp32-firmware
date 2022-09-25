//APIPath:evse/

interface Slot {
    max_current: number,
    active: boolean,
    clear_on_disconnect: boolean
}

export type slots = Slot[];

export interface button_state {
    button_press_time: number,
    button_release_time: number,
    button_pressed: boolean
}

export interface indicator_led {
    indication: number,
    duration: number
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

export interface external_current {
    current: number
}

export interface external_clear_on_disconnect {
    clear_on_disconnect: boolean
}

export interface management_current {
    current: number
}

export interface auto_start_charging {
    auto_start_charging: boolean
}

export interface global_current {
    current: number
}

export interface management_enabled {
    enabled: boolean;
}

export interface user_enabled {
    enabled: boolean;
}

export interface external_enabled {
    enabled: boolean;
}

export interface external_defaults {
    current: number,
    clear_on_disconnect: boolean
}

export interface start_charging {

}

export interface stop_charging {

}

export type debug_header = string;
export type debug = string;

export interface reflash {

}

export interface reset {

}
