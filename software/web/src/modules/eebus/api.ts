interface Peer {
    ip: string;
    port: number;
    trusted: boolean;
    dns_name: string;
    id: string;
    wss_path: string;
    ski: string;
    autoregister: boolean;
    model_brand: string;
    model_model: string;
    model_type: string;
    state: number;
}

interface Connection {
    ski: string;
    ship_state: number;
}

export interface add {
    ski: string;
    trusted: boolean;
    ip: string;
    port: number;
    dns_name: string;
    wss_path: string;
}

export interface remove {
    ski: string;
}

export interface config {
    enable: boolean;
    cert_id: number;
    key_id: number;
    peers: Peer[];
}

export interface ChargeProcesses {
    id: number;
    charged_kwh: number;
    start_time: number;
    duration: number;
    cost: number;
    percent_self_produced_energy: number;
    percent_self_produced_cost: number;
}

export interface state {
    ski: string;
    discovery_state: number;
    connections: Connection[];
}

export interface usecases {
    commands_received: number;
    commands_sent: number;
    charging_summary: ChargeProcesses[];
    power_consumption_limitation: {
        usecase_state: number;
        limit_active: boolean;
        current_limit: number;
        failsafe_limit_power_w: number;
        failsafe_limit_duration_s: number;
        constraints_power_maximum: number;
    };
    ev_commissioning_and_configuration: {
        ev_connected: boolean;
        communication_standard: string;
        asymmetric_charging_supported: boolean;
        mac_address: string;
        minimum_power: number;
        maximum_power: number;
        standby_power: number;
        standby_mode: number;
    }
    evse_commissioning_and_configuration: {
        evse_failure: boolean;
        evse_failure_description: string;
    }

}

export interface scan {
}
