export interface low_level_state {
    consecutive_bricklet_errors: number;

    contactor: boolean;
    led_rgb: number[];
    input_voltage: number;
    contactor_check_state: number;
}

export interface state {
    phases_switched: number;
    error_flags: number;
    config_error_flags: number;
    input3_state: boolean;
    input4_state: boolean;
    relay_state: boolean;
}

export interface config {
    contactor_installed: boolean;
}

export interface reflash {
}

export interface reset {
}
