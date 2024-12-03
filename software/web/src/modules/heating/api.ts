export interface config {
    sg_ready_blocking_active_type: number;
    sg_ready_extended_active_type: number;
    minimum_holding_time: number;
    meter_slot_grid_power: number;
    control_period: number;
    extended_logging_active: boolean;
    yield_forecast_active: boolean;
    yield_forecast_threshold: number;
    extended_active: boolean;
    extended_hours: number;
    blocking_active: boolean;
    blocking_hours: number;
    pv_excess_control_active: boolean;
    pv_excess_control_threshold: number;
    p14enwg_active: boolean;
    p14enwg_input: number;
    p14enwg_active_type: number;
}

export interface state {
    sg_ready_blocking_active: boolean;
    sg_ready_extended_active: boolean;
    p14ewng_active: boolean;
    remaining_holding_time: number;
}

export interface reset_holding_time {}