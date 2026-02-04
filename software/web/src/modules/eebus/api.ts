import {Usecases} from "./usecases.enum";

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
    persistent: boolean;
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
    persistent: boolean;
}

export interface remove {
    ski: string;
}

export interface config {
    enable: boolean;
    cert_id: number;
    key_id: number;
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
    peers: Peer[];
}

export interface usecases {
    commands_received: number;
    commands_sent: number;
    usecases_supported: Usecases[],
    charging_summary: ChargeProcesses[];
    power_consumption_limitation: {
        usecase_state: number;
        limit_active: boolean;
        current_limit: number;
        failsafe_limit_power_w: number;
        failsafe_limit_duration_s: number;
        constraints_power_maximum: number;
        outstanding_duration_s: number;
    };
    power_production_limitation: {
        usecase_state: number;
        limit_active: boolean;
        current_limit: number;
        failsafe_limit_power_w: number;
        failsafe_limit_duration_s: number;
        constraints_power_maximum: number;
        outstanding_duration_s: number;
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
    };
    evse_commissioning_and_configuration: {
        evse_failure: boolean;
        evse_failure_description: string;
    }
    ev_charging_electricity_measurement: {
        amps_phase_1: number;
        amps_phase_2: number;
        amps_phase_3: number;
        power_phase_1: number;
        power_phase_2: number;
        power_phase_3: number;
        charged_wh: number;
        charged_valuesource_measured: boolean;
    };
    monitoring_of_power_consumption: {
        total_power_w: number;
        power_phase_1_w: number;
        power_phase_2_w: number;
        power_phase_3_w: number;
        energy_consumed_wh: number;
        energy_produced_wh: number;
        current_phase_1_ma: number;
        current_phase_2_ma: number;
        current_phase_3_ma: number;
        voltage_phase_1_v: number;
        voltage_phase_2_v: number;
        voltage_phase_3_v: number;
        voltage_phase_1_2_v: number;
        voltage_phase_2_3_v: number;
        voltage_phase_3_1_v: number;
        frequency_mhz: number;
    };
    coordinated_ev_charging: {
        has_charging_plan: boolean;
        charging_plan_start_time: number;
        target_power_w: number;
        has_incentives: boolean;
        energy_broker_connected: boolean;
        energy_broker_heartbeat_ok: boolean;
    };

}

export interface scan {
}
