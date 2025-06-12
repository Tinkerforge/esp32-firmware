import { RuleConfig } from "./types";

export interface config {
    forbid_discharge_during_fast_charge: boolean;
}

export type rules_permit_grid_charge = RuleConfig[];
export type rules_forbid_discharge   = RuleConfig[];
export type rules_forbid_charge      = RuleConfig[];

export interface low_level_config {
    rewrite_period: number;
}

export interface state {
    grid_charge_permitted: boolean;
    discharge_forbidden:   boolean;
    charge_forbidden:      boolean;
}
