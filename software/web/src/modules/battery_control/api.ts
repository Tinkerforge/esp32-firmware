export interface config {
    block_discharge_during_fast_charge: boolean;
}

export interface low_level_config {
    rewrite_period: number;
}

export interface state {
    grid_charge_allowed: boolean;
    discharge_blocked: boolean;
}
