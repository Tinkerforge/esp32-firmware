interface ChargerState {
    start: number;
    amount: number
}

export interface config {
    enable: boolean;
    mode_after: number;
    park_time: boolean;
    park_time_duration: number;
    charge_below: boolean;
    charge_below_threshold: number;
    block_above: boolean;
    block_above_threshold: number;
    yield_forecast: boolean;
    yield_forecast_threshold: number;
}

export interface charge_plan {
    enable: boolean;
    departure: number;
    time: number;
    amount: number;
}

export interface state {
    last_save: number;
    chargers: ChargerState[],
}
