export interface config {
    sgr_blocking_type: number;
    sgr_extended_type: number;
    min_hold_time: number;
    meter_slot_grid_power: number;
    control_period: number;
    extended_logging: boolean;
    yield_forecast: boolean;
    yield_forecast_threshold: number;
    extended: boolean;
    extended_hours: number;
    blocking: boolean;
    blocking_hours: number;
    pv_excess_control: boolean;
    pv_excess_control_threshold: number;
    p14enwg: boolean;
    p14enwg_input: number;
    p14enwg_type: number;
}

export interface state {
    sgr_blocking: boolean;
    sgr_extended: boolean;
    p14enwg: boolean;
    next_update: number;
}

export interface reset_holding_time {}

export interface sgr_blocking_override {
    override_until: number;
}

export type toggle_sgr_blocking = {};
export type toggle_sgr_extended = {};
