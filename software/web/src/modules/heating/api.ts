export interface config {
    minimum_control_holding_time: number;
    meter_slot_grid_power: number;
    extended_logging_active: boolean;
    winter_start_day: number;
    winter_start_month: number;
    winter_end_day: number;
    winter_end_month: number;
    winter_dynamic_price_control_active: boolean;
    winter_dynamic_price_control_threshold: number;
    winter_pv_excess_control_active: boolean;
    winter_pv_excess_control_threshold: number;
    summer_block_time_active: boolean;
    summer_block_time_morning: number;
    summer_block_time_evening: number;
    summer_yield_forecast_active: boolean;
    summer_yield_forecast_threshold: number;
    summer_dynamic_price_control_active: boolean;
    summer_dynamic_price_control_threshold: number;
    summer_pv_excess_control_active: boolean;
    summer_pv_excess_control_threshold: number;
    p14enwg_active: boolean;
    p14enwg_input: number;
    p14enwg_active_type: number;
}
