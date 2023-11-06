interface ChargerState {
    state: number,
    error: number,
    allocated_current: number,
    supported_current: number,
    last_update: number,
    name: string,
    uid: number
}

interface ServCharger {
    hostname: string;
    ip: string;
    display_name: string;
    error: number;
}

export type scan_result = ServCharger[] | string;

export interface state {
    state: number,
    uptime: number,
    chargers: ChargerState[]
}

interface ChargerConfig {
    host: string,
    name: string
}

export interface config {
    enable_charge_manager: boolean,
    enable_watchdog: boolean,
    verbose: boolean,
    default_available_current: number,
    maximum_available_current: number,
    minimum_current_auto: boolean,
    minimum_current: number,
    minimum_current_1p: number,
    minimum_current_vehicle_type: number,
    requested_current_threshold: number,
    requested_current_margin: number,
    chargers: ChargerConfig[]
}

export interface available_current {
    current: number
}

export interface scan {
}
