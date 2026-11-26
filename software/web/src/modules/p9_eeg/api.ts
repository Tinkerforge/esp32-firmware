import { P9EegSource } from "./generated/p9_eeg_source.enum";

export interface config {
    enable: boolean;
    source: P9EegSource;
    inverter_power_w: number;
    relays: number[];
    default_level_pct: number;
}

export interface state {
    limit_active: boolean;
    limit_w: number;
    level_pct: number;
    default_active: boolean;
    limit_reachable: boolean;
}

export interface control {
    active: boolean;
    limit_w: number;
}

export interface control_update {
    active: boolean;
    limit_w: number;
}
