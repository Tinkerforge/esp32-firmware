import { RuleConfig } from "./types";
import { BatteryMode } from "../batteries/battery_mode.enum";

export interface config {
    cheap_tariff_quarters: number;
    expensive_tariff_quarters: number;
}

export type rules_charge    = RuleConfig[];
export type rules_discharge = RuleConfig[];

export interface state {
    mode: BatteryMode;
    active_charge_rule: number;
    active_discharge_rule: number;
}
