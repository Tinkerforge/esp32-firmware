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

export interface history_wallbox_5min_changed {
    uid: number;
    year: number;
    month: number;
    day: number;
    hour: number;
    minute: number;
    flags: number;
    power: number;
}

export interface history_wallbox_daily_changed {
    uid: number;
    year: number;
    month: number;
    day: number;
    energy: number;
}

export interface history_energy_manager_5min_changed {
    year: number;
    month: number;
    day: number;
    hour: number;
    minute: number;
    flags: number;
    power: number[];
}

export interface history_energy_manager_daily_changed {
    year: number;
    month: number;
    day: number;
    energy_import: number[];
    energy_export: number[];
}

export type debug_header = string;
export type debug = string;
