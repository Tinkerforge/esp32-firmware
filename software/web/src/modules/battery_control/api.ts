export interface config {
    forbid_discharge_during_fast_charge: boolean;
}

interface RuleConfig {
    name:          string;
    soc_cond:      number;
    soc_th:        number;
    price_cond:    number;
    price_th:      number;
    forecast_cond: number;
    forecast_th:   number;
}

export type rules_forbid_discharge   = RuleConfig[];
export type rules_permit_grid_charge = RuleConfig[];

export interface low_level_config {
    rewrite_period: number;
}

export interface state {
    grid_charge_permitted: boolean;
    discharge_forbidden:   boolean;
}
