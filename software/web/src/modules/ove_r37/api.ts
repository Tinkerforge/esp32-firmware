import { OveR37State } from "./generated/ove_r37_state.enum";

export interface config {
    enabled: boolean;
    undervoltage_threshold: number;
    undervoltage_observation_time: number;
    reconnect_wait_time: number;
    start_delay: number;
}

export interface config_update {
    enabled: boolean;
    undervoltage_threshold: number;
    undervoltage_observation_time: number;
    reconnect_wait_time: number;
    start_delay: number;
    password: string;
}

export interface state {
    state: OveR37State;
    trip_reason: number;
    flags: number;
}

export const OVE_R37_TRIP_REASON_UNDERVOLTAGE = 1;
export const OVE_R37_TRIP_REASON_OVERVOLTAGE  = 2;
export const OVE_R37_TRIP_REASON_FREQUENCY    = 4;

export const OVE_R37_FLAG_VOLTAGE_IN_RANGE   = 1;
export const OVE_R37_FLAG_FREQUENCY_IN_RANGE = 2;
export const OVE_R37_FLAG_VOLTAGE_VALID      = 4;
export const OVE_R37_FLAG_CURRENT_VALID      = 8;
export const OVE_R37_FLAG_FREQUENCY_VALID    = 16;
