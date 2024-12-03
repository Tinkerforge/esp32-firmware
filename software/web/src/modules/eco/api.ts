interface ChargerState {
    amount: number
}

export interface config {
    charge_plan_active: boolean;
    mode_after_charge_plan: number;
    service_life_active: boolean;
    service_life: number;
    charge_below_active: boolean;
    charge_below: number;
    block_above_active: boolean;
    block_above: number;
    yield_forecast_active: boolean;
    yield_forecast: number;
}

export interface charge_plan {
    enabled: boolean;
    depature: number;
    time: number;
    amount: number;
}

export interface state {
    last_charge_plan_save: number;
    chargers: ChargerState[],
}
