export interface config {
    charge_plan_active: boolean;
    mode_after_charge_plan: number;
    charge_below_active: boolean;
    charge_below: number;
    block_above_active: boolean;
    block_above: number;
}

export interface charge_plan {
    enabled: boolean;
    day: number;
    time: number;
    hours: number;
}

export interface state {
    last_charge_plan_save: number;
}
