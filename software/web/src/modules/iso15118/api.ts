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

export interface state_din70121 {
    state: number;
    session_id: number[];
    supported_protocols: string[];
    protocol: string;
    evcc_id: number[];
    soc: number;
    ev_ready: number;
    ev_cabin_conditioning: number;
    ev_cabin_conditioning_is_used: boolean;
    ev_ress_conditioning: number;
    ev_ress_conditioning_is_used: boolean;
    ev_error_code: number;
    ev_maximum_current_limit_val: number;
    ev_maximum_current_limit_mul: number;
    ev_maximum_power_limit_val: number;
    ev_maximum_power_limit_mul: number;
    ev_maximum_power_limit_is_used: boolean;
    ev_maximum_voltage_limit_val: number;
    ev_maximum_voltage_limit_mul: number;
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
}

export interface state_iso20 {
}