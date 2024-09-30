export interface config {
    sg_ready_blocking_active_type: number;
    sg_ready_extended_active_type: number;
    minimum_control_holding_time: number;
    meter_slot_grid_power: number;
    extended_logging_active: boolean;
    summer_start_day: number;
    summer_start_month: number;
    summer_end_day: number;
    summer_end_month: number;
    summer_active_time_active: boolean;
    summer_active_time_start: number;
    summer_active_time_end: number;
    summer_yield_forecast_active: boolean;
    summer_yield_forecast_threshold: number;
    dpc_extended_active: boolean;
    dpc_extended_threshold: number;
    dpc_blocking_active: boolean;
    dpc_blocking_threshold: number;
    pv_excess_control_active: boolean;
    pv_excess_control_threshold: number;
    p14enwg_active: boolean;
    p14enwg_input: number;
    p14enwg_active_type: number;
}
