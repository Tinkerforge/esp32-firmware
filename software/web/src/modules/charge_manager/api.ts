interface Charger {
    name: string,
    uid: number,
    last_update: number,
    uptime: number,
    supported_current: number,
    allowed_current: number,
    requested_current: number,
    wants_to_charge: boolean,
    wants_to_charge_low_priority: boolean,
    is_charging: boolean,
    cp_disconnect_supported: boolean,
    cp_disconnect_state: boolean,
    cp_disconnect: boolean,
    last_sent_config: number,
    allocated_current: number,
    state: number,
    error: number,
    charger_state: number,
    power_total: number,
    energy_abs: number
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
    chargers: Charger[]
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
