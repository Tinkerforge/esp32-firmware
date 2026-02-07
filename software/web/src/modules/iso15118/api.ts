export interface config {
    autocharge: boolean;
    read_soc: boolean;
    charge_via_iso15118: boolean;
    min_charge_current: number;
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
    encryption: number;
    seen_macs: {
        mac: number[];
        last_seen: number;
    }[];
}

export interface state_din70121 {
    state: number;
    session_id: number[];
    evcc_id: number[];
    soc: number;
}

export interface state_iso2 {
    state: number;
    session_id: number[];
    evcc_id: number[];
    soc: number;
    energy_capacity: number;
}

export interface state_iso20 {
    state: number;
    session_id: string;
    evcc_id: string;
    soc: number;
    active_power: number;
    energy_capacity: number;
}