export interface config {
}

export interface state_slac {
    state: number;
    modem_found: boolean;
    modem_initialization_tries: number;
    atten_char_indication_tries: number;
    attenuation_profile: number[];
    received_aag_lists: number;
    received_sounds: number;
    nmk: number[];
    nid: number[];
    evse_mac: number[];
    evse_mac_modem: number[];
    pev_mac: number[];
    pev_mac_modem: number[];
    pev_run_id: number[];
}

export interface state_sdp {
    state: number;
    evse_ip_address: number[];
    evse_port: number;
    evse_security: number;
    evse_tranport_protocol: number;
    ev_security: number;
    ev_tranport_protocol: number;
}

export interface state_common {
    state: number;
    supported_protocols: string[];
    protocol: string;
}

export interface state_din70121 {
    state: number;
    session_id: number[];
    evcc_id: number[];
    soc: number;
    ev_ready: number;
    ev_cabin_conditioning: number;
    ev_cabin_conditioning_is_used: boolean;
    ev_ress_conditioning: number;
    ev_ress_conditioning_is_used: boolean;
    ev_error_code: number;
    ev_max_current_limit_val: number;
    ev_max_current_limit_mul: number;
    ev_max_power_limit_val: number;
    ev_max_power_limit_mul: number;
    ev_max_power_limit_is_used: boolean;
    ev_max_voltage_limit_val: number;
    ev_max_voltage_limit_mul: number;
    ev_energy_capacity_val: number;
    ev_energy_capacity_mul: number;
    ev_energy_capacity_is_used: boolean;
    ev_energy_request_val: number;
    ev_energy_request_mul: number;
    ev_energy_request_is_used: boolean;
    full_soc: number;
    full_soc_is_used: boolean;
    bulk_soc: number;
    bulk_soc_is_used: boolean;
}

export interface state_iso2 {
    state: number;
    session_id: number[];
    evcc_id: number[];
    max_entries_sa_schedule_tuple: number;
    requested_energy_transfer_mode: number;
    'ac': {
        is_used: boolean;
        departure_time: number;
        departure_time_is_used: boolean;
        eamount_val: number;
        eamount_mul: number;
        ev_max_voltage_val: number;
        ev_max_voltage_mul: number;
        ev_min_current_val: number;
        ev_min_current_mul: number;
    };
    'dc': {
        is_used: boolean;
        departure_time: number;
        departure_time_is_used: boolean;
        soc: number;
        ev_ready: number;
        ev_error_code: number;
        ev_max_current_limit_val: number;
        ev_max_current_limit_mul: number;
        ev_max_power_limit_val: number;
        ev_max_power_limit_mul: number;
        ev_max_power_limit_is_used: boolean;
        ev_max_voltage_limit_val: number;
        ev_max_voltage_limit_mul: number;
        ev_energy_capacity_val: number;
        ev_energy_capacity_mul: number;
        ev_energy_capacity_is_used: boolean;
        ev_energy_request_val: number;
        ev_energy_request_mul: number;
        ev_energy_request_is_used: boolean;
        full_soc: number;
        full_soc_is_used: boolean;
        bulk_soc: number;
        bulk_soc_is_used: boolean;
    };
}

export interface state_iso20 {
    state: number;
}