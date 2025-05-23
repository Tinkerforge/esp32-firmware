/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { h, Fragment, createRef, Component, RefObject } from "preact";
import { effect } from "@preact/signals-core";
import { __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { SwitchableInputNumber } from "../../ts/components/switchable_input_number";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { InputNumber } from "../../ts/components/input_number";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Thermometer } from "react-feather";
import { InputSelect } from "../../ts/components/input_select";
import { MeterValueID    } from "../meters/meter_value_id";
import { get_noninternal_meter_slots, NoninternalMeterSelector } from "../power_manager/main";
import { UplotLoader } from "../../ts/components/uplot_loader";
import { UplotData, UplotWrapperB, UplotPath } from "../../ts/components/uplot_wrapper_2nd";
import { InputText } from "../../ts/components/input_text";
import { is_solar_forecast_enabled, get_kwh_today, get_kwh_tomorrow } from  "../solar_forecast/main";
import { is_day_ahead_prices_enabled, get_average_price_today, get_average_price_tomorrow, get_price_from_index } from "../day_ahead_prices/main";
import { StatusSection } from "../../ts/components/status_section";
import { Button } from "react-bootstrap";
import { ControlPeriod } from "./control_period.enum";
import { sgr_blocking_override, state } from "./api";

export function HeatingNavbar() {
    return <NavbarItem name="heating" module="heating" title={__("heating.navbar.heating")} symbol={<Thermometer />} />;
}

type HeatingConfig = API.getType["heating/config"];

interface HeatingState {
    heating_state: API.getType["heating/state"];
    dap_config: API.getType["day_ahead_prices/config"];
    active_sgr_blocking_type: number;
    active_sgr_extended_type: number;
}

export class Heating extends ConfigComponent<'heating/config', {status_ref?: RefObject<HeatingStatus>}, HeatingState> {
    uplot_loader_ref        = createRef();
    uplot_wrapper_ref       = createRef();

    summer_start_day:   number;
    summer_start_month: number;
    summer_end_day:     number;
    summer_end_month:   number;

    static days(): [string, string][] {
        return [...Array(31).keys()].map((i) => [
            (i+1).toString(),
            (i+1).toString()
        ]);
    }

    static months(): [string, string][] {
        return [
            ["1",  __("heating.content.january")],
            ["2",  __("heating.content.february")],
            ["3",  __("heating.content.march")],
            ["4",  __("heating.content.april")],
            ["5",  __("heating.content.may")],
            ["6",  __("heating.content.june")],
            ["7",  __("heating.content.july")],
            ["8",  __("heating.content.august")],
            ["9",  __("heating.content.september")],
            ["10", __("heating.content.october")],
            ["11", __("heating.content.november")],
            ["12", __("heating.content.december")]
        ];
    }

    constructor() {
        super('heating/config',
              () => __("heating.script.save_failed"));

        util.addApiEventListener("heating/state", () => {
            this.setState({heating_state: API.get("heating/state")});
        });

        util.addApiEventListener("heating/config", () => {
            let config = API.get("heating/config");

            this.setState({active_sgr_blocking_type: config.sgr_blocking_type, active_sgr_extended_type: config.sgr_extended_type});
        });

        util.addApiEventListener("day_ahead_prices/prices", () => {
            // Update chart every time new price data comes in
            this.update_uplot();
        });

        // Update vertical "now" line on time change
        effect(() => this.update_uplot());
    }

    get_price_timeframe() {
        const dap_prices = API.get("day_ahead_prices/prices");

        let time = new Date(util.get_date_now_1m_update_rate());
        let s = ""
        if(dap_prices.resolution == 0) {
            time.setMilliseconds(Math.floor(time.getMilliseconds() / 1000) * 1000);
            time.setSeconds(Math.floor(time.getSeconds() / 60) * 60);
            time.setMinutes(Math.floor(time.getMinutes() / 15) * 15);
            s += time.toLocaleTimeString() + '-';
            time.setMinutes(time.getMinutes() + 15);
            s += time.toLocaleTimeString()
        } else {
            time.setMilliseconds(Math.floor(time.getMilliseconds() / 1000) * 1000);
            time.setSeconds(Math.floor(time.getSeconds() / 60) * 60);
            time.setMinutes(Math.floor(time.getMinutes() / 60) * 60);
            s += time.toLocaleTimeString() + '-';
            time.setMinutes(time.getMinutes() + 60);
            s += time.toLocaleTimeString()
        }

        return s
    }

    get_control_period_hours() {
        const control_period = this.state.control_period;
        switch(control_period) {
            case ControlPeriod.Hours24: return 24;
            case ControlPeriod.Hours12: return 12;
            case ControlPeriod.Hours8:  return 8;
            case ControlPeriod.Hours6:  return 6;
            case ControlPeriod.Hours4:  return 4;
            default: console.log("Invalid control period: " + control_period); return 24;
        }
    }

    update_uplot() {
        // Use signal here to make effect() record its use, even
        // if this function might exit early on its first call
        let date_now = util.get_date_now_1m_update_rate();

        if (this.uplot_wrapper_ref.current == null) {
            return;
        }

        let extended_hours = this.state.extended_hours
        let blocking_hours = this.state.blocking_hours

        const control_period_hours = this.get_control_period_hours();
        if (this.state.extended && this.state.blocking) {
            if (blocking_hours + extended_hours > control_period_hours) {
                this.setState({blocking_hours: Math.max(0, control_period_hours - extended_hours)});
                blocking_hours = Math.max(0, control_period_hours - extended_hours);
                if (this.state.extended_hours > control_period_hours) {
                    this.setState({extended_hours: control_period_hours});
                    extended_hours = control_period_hours;
                }
            }
        } else if (this.state.extended) {
            if (this.state.extended_hours > control_period_hours) {
                this.setState({extended_hours: control_period_hours});
                extended_hours = control_period_hours;
            }
        } else if (this.state.blocking) {
            if (this.state.blocking_hours > control_period_hours) {
                this.setState({blocking_hours: control_period_hours});
                blocking_hours = control_period_hours;
            }
        }

        const dap_prices = API.get("day_ahead_prices/prices");
        const dap_config = API.get("day_ahead_prices/config");
        let data: UplotData;

        // If we have not got any prices yet, use empty data
        if (dap_prices.prices.length == 0) {
            data = {
                keys: [null],
                names: [null],
                values: [null],
                stacked: [null],
                paths: [null],
            }
        // Else fill with time and the three different prices we want to show
        } else {
            data = {
                keys: [null, 'price'],
                names: [null, __("day_ahead_prices.content.electricity_price")],
                values: [[], [], [], []],
                stacked: [null, true],
                paths: [null, UplotPath.Step],
                // Only enable the electricity price by default.
                // The chart with only electricity price is the most useful in most cases.
                default_visibilty: [null, true],
                lines_vertical: []
            }
            const resolution_multiplier = dap_prices.resolution == 0 ? 15 : 60
            const grid_costs_and_taxes_and_supplier_markup = dap_config.grid_costs_and_taxes / 1000.0 + dap_config.supplier_markup / 1000.0;
            for (let i = 0; i < dap_prices.prices.length; i++) {
                data.values[0].push(dap_prices.first_date * 60 + i * 60 * resolution_multiplier);
                data.values[1].push(get_price_from_index(i) / 1000.0 + grid_costs_and_taxes_and_supplier_markup);
            }

            data.values[0].push(dap_prices.first_date * 60 + dap_prices.prices.length * 60 * resolution_multiplier - 1);
            data.values[1].push(get_price_from_index(dap_prices.prices.length - 1) / 1000.0 + grid_costs_and_taxes_and_supplier_markup);

            const hour_multiplier = 60/resolution_multiplier;
            const num_per_day     = 24*hour_multiplier;
            const extended        = this.state.extended ? extended_hours*hour_multiplier : 0;
            const blocking        = this.state.blocking ? blocking_hours*hour_multiplier : 0;

            let num_per_period = 0;
            switch(this.state.control_period) {
                case ControlPeriod.Hours24: num_per_period = num_per_day; break;
                case ControlPeriod.Hours12: num_per_period = num_per_day/2; break;
                case ControlPeriod.Hours8:  num_per_period = num_per_day/3; break;
                case ControlPeriod.Hours6:  num_per_period = num_per_day/4; break;
                case ControlPeriod.Hours4:  num_per_period = num_per_day/6; break;
                default: console.log("Invalid control period: " + this.state.control_period); num_per_period = num_per_day; break;
            }

            for (let start = 0; start < dap_prices.prices.length; start += num_per_period) {
                const cheap_hours = data.values[1]
                    .slice(start, start + num_per_period)
                    .map((price, index) => ({ price, index }))
                    .sort((a, b) => a.price - b.price)
                    .slice(0, extended)
                    .map(item => item.index + start);
                const expensive_hours = data.values[1]
                    .slice(start, start + num_per_period)
                    .map((price, index) => ({ price, index }))
                    .sort((a, b) => b.price - a.price)
                    .slice(0, blocking)
                    .map(item => item.index + start);

                cheap_hours.forEach(index => {
                    data.lines_vertical.push({'index': index, 'text': '', 'color': [40, 167, 69, 0.5]});
                });
                expensive_hours.forEach(index => {
                    data.lines_vertical.push({'index': index, 'text': '', 'color': [220, 53, 69, 0.5]});
                });
            }

            // Add vertical line at current time
            const resolution_divisor = dap_prices.resolution == 0 ? 15 : 60;
            const diff = Math.floor(date_now / 60000) - dap_prices.first_date;
            const index = Math.floor(diff / resolution_divisor);
            data.lines_vertical.push({'index': index, 'text': __("day_ahead_prices.content.now"), 'color': [64, 64, 64, 0.2]});
        }

        // Show loader or data depending on the availability of data
        this.uplot_loader_ref.current.set_data(data && data.keys.length > 1);
        this.uplot_wrapper_ref.current.set_data(data);
    }

    get_date_from_minutes(minutes: number) {
        const h = Math.floor(minutes / 60);
        const m = minutes - h * 60;
        return new Date(0, 0, 1, h, m);
    }

    get_minutes_from_date(date: Date) {
        return date.getMinutes() + date.getHours()*60;
    }

    month_to_days(month: number): [string, string][] {
        switch(month) {
            case 1: case 3: case 5: case 7: case 8: case 10: case 12: return Heating.days().slice(0, 31);
            case 4: case 6: case 9: case 11:                          return Heating.days().slice(0, 30);
            case 2:                                                   return Heating.days().slice(0, 28);
            default: console.log("Invalid month: " + month);
        }
        return Heating.days().slice(0, 31);
    }

    render(props: {}, state: HeatingState & HeatingConfig) {
        if (!util.render_allowed())
            return <SubPage name="heating" />;

        const meter_slots = get_noninternal_meter_slots([MeterValueID.PowerActiveLSumImExDiff], NoninternalMeterSelector.AllValues, __("power_manager.content.meter_slot_grid_power_missing_value"));

        const solar_forecast_enabled   = is_solar_forecast_enabled();
        const day_ahead_prices_enabled = is_day_ahead_prices_enabled();
        const meter_available          = meter_slots.length > 0;

        function get_remaining_minutes() {
            if (state.heating_state.next_update == 0) {
                return __("util.not_yet_known");
            } else {
                const remaining_holding_time = Math.round(Math.max(0, state.heating_state.next_update - util.get_date_now_1m_update_rate() / 60000));
                if (remaining_holding_time == 1) {
                    return "1 " + __("heating.content.minute");
                } else {
                    return remaining_holding_time + " " + __("heating.content.minutes");
                }
            }
        }

        return (
            <SubPage name="heating">
                <ConfigForm id="heating_config_form"
                            title={__("heating.content.heating")}
                            isModified={false}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("heating.content.meter_slot_grid_power")} label_muted={__("heating.content.meter_slot_grid_power_muted")}>
                        <InputSelect
                            placeholder={meter_available ? __("select") : __("heating.content.meter_slot_grid_power_none")}
                            items={meter_slots}
                            value={state.meter_slot_grid_power}
                            onValue={(v) => this.setState({meter_slot_grid_power: parseInt(v)})}
                        />
                    </FormRow>
                    <FormRow label={__("heating.content.minimum_holding_time")} label_muted={__("heating.content.minimum_holding_time_muted")}>
                        <InputNumber
                            unit={__("heating.content.minutes")}
                            value={state.min_hold_time}
                            onValue={this.set("min_hold_time")}
                            min={10}
                            max={60}
                        />
                    </FormRow>
                    <FormRow label={__("heating.content.remaining_holding_time")} label_muted={__("heating.content.remaining_holding_time_muted")}>
                        <InputText
                            readonly={true}
                            value={get_remaining_minutes()}>
                            <div class="input-group-append">
                                <Button
                                    className="form-control rounded-right"
                                    variant="primary"
                                    onClick={() => API.call('heating/reset_holding_time', {}, () => "")}>
                                    {__("heating.content.update_now")}
                                </Button>
                            </div>
                        </InputText>
                    </FormRow>
                    <FormRow label={__("heating.content.sg_ready_output") + " 1"} label_muted={__("heating.content.sg_ready_output1_muted")} help={__("heating.content.sg_ready_output1_help")}>
                        <div class="row mx-n1">
                            <div class="col px-1">
                                <InputSelect
                                    items={[
                                        ["0", __("heating.content.output_active_closed")],
                                        ["1", __("heating.content.output_active_open")]
                                    ]}
                                    value={state.sgr_blocking_type}
                                    onValue={(v) => this.setState({sgr_blocking_type: parseInt(v)})}
                                />
                            </div>
                            {state.active_sgr_blocking_type !== undefined ?
                                <div class="col-auto px-1">
                                    <Button variant="primary"
                                            onClick={() => API.call("heating/toggle_sgr_blocking", {}, () => __("heating.content.toogle_now_failed")(1))}>
                                        {state.heating_state.sgr_blocking === (state.active_sgr_blocking_type === 0) ? __("heating.content.open_now") : __("heating.content.close_now")}
                                    </Button>
                                </div> : undefined}
                        </div>
                    </FormRow>
                    <FormRow label={__("heating.content.sg_ready_output") + " 2"} label_muted={__("heating.content.sg_ready_output2_muted")} help={__("heating.content.sg_ready_output2_help")}>
                        <div class="row mx-n1">
                            <div class="col px-1">
                                <InputSelect
                                    items={[
                                        ["0", __("heating.content.output_active_closed")],
                                        ["1", __("heating.content.output_active_open")]
                                    ]}
                                    value={state.sgr_extended_type}
                                    onValue={(v) => this.setState({sgr_extended_type: parseInt(v)})}
                                />
                            </div>
                            {state.active_sgr_extended_type !== undefined ?
                                <div class="col-auto px-1">
                                    <Button variant="primary"
                                            onClick={() => API.call("heating/toggle_sgr_extended", {}, () => __("heating.content.toogle_now_failed")(2))}>
                                        {state.heating_state.sgr_extended === (state.active_sgr_extended_type === 0) ? __("heating.content.open_now") : __("heating.content.close_now")}
                                    </Button>
                                </div> : undefined}
                        </div>
                    </FormRow>
                    <FormRow label={__("heating.content.control_period")} label_muted={__("heating.content.control_period_muted")} help={__("heating.content.control_period_help")}>
                        <InputSelect
                            items={[
                                ["0", __("heating.content.hours")(24)],
                                ["1", __("heating.content.hours")(12)],
                                ["2", __("heating.content.hours")(8)],
                                ["3", __("heating.content.hours")(6)],
                                ["4", __("heating.content.hours")(4)]
                            ]}
                            value={state.control_period}
                            onValue={(v) => this.setState({control_period: parseInt(v)}, this.update_uplot)}
                        />
                    </FormRow>
                    <FormRow label={__("heating.content.extended_logging")} label_muted={__("heating.content.extended_logging_description")}>
                        <Switch desc={__("heating.content.extended_logging_enabled")}
                                checked={state.extended_logging}
                                onClick={this.toggle('extended_logging')}
                        />
                    </FormRow>
                    <FormSeparator heading={__("heating.content.extended_operation")} help={__("heating.content.extended_operation_help")}/>
                    <FormRow label={__("heating.content.pv_excess_control")}
                             error={__("heating.content.meter_needs_activation")}
                             show_error={state.pv_excess_control && !meter_available}
                             class="mb-2 mb-lg-0">
                        <SwitchableInputNumber
                            switch_label_active={__("heating.content.active")}
                            switch_label_inactive={__("heating.content.inactive")}
                            unit={__("heating.content.watt")}
                            checked={state.pv_excess_control}
                            onClick={this.toggle('pv_excess_control')}
                            value={state.pv_excess_control_threshold}
                            onValue={this.set("pv_excess_control_threshold")}
                            min={0}
                            max={100000}
                            switch_label_min_width="110px"
                        />
                    </FormRow>
                    <FormRow label={__("heating.content.or")} small={true} class="mb-0">
                        <div></div>
                    </FormRow>
                    <FormRow label={__("heating.content.dpc_low")}
                             error={__("heating.content.day_ahead_prices_needs_activation")}
                             show_error={state.extended && !day_ahead_prices_enabled}
                             class="mb-2 mb-lg-0">
                        <SwitchableInputNumber
                            switch_label_active={__("heating.content.active")}
                            switch_label_inactive={__("heating.content.inactive")}
                            unit={__("heating.content.h_per_x")(this.get_control_period_hours())}
                            checked={state.extended}
                            onClick={this.toggle('extended', this.update_uplot)}
                            value={state.extended_hours}
                            onValue={(v) => {this.setState({extended_hours: v}, this.update_uplot)}}
                            min={0}
                            max={this.get_control_period_hours() - state.blocking_hours}
                            switch_label_min_width="110px"
                        />
                    </FormRow>
                    <FormRow label={__("heating.content.but_only_if")} small={true} class="mb-0">
                        <div></div>
                    </FormRow>
                    <FormRow label={__("heating.content.pv_yield_forecast")}
                             error={__("heating.content.solar_forecast_needs_activation")}
                             show_error={state.yield_forecast && state.extended && !solar_forecast_enabled}>
                        <SwitchableInputNumber
                            disabled={!state.extended}
                            switch_label_active={__("heating.content.active")}
                            switch_label_inactive={__("heating.content.inactive")}
                            unit={__("heating.content.kwh_per_day")}
                            checked={state.yield_forecast && state.extended}
                            onClick={this.toggle('yield_forecast', this.update_uplot)}
                            value={state.yield_forecast_threshold}
                            onValue={this.set("yield_forecast_threshold", this.update_uplot)}
                            min={0}
                            max={1000}
                            switch_label_min_width="110px"
                        />
                    </FormRow>
                    <FormSeparator heading={__("heating.content.blocking_operation")} help={__("heating.content.blocking_operation_help")}/>
                    <FormRow label={__("heating.content.for_the_most_expensive")}
                             error={__("heating.content.day_ahead_prices_needs_activation")}
                             show_error={state.blocking && !day_ahead_prices_enabled}>
                        <SwitchableInputNumber
                            switch_label_active={__("heating.content.active")}
                            switch_label_inactive={__("heating.content.inactive")}
                            unit={__("heating.content.h_per_x")(this.get_control_period_hours())}
                            checked={state.blocking}
                            onClick={this.toggle('blocking', this.update_uplot)}
                            value={state.blocking_hours}
                            onValue={(v) => {this.setState({blocking_hours: v}, this.update_uplot)}}
                            min={0}
                            max={this.get_control_period_hours() - state.extended_hours}
                            switch_label_min_width="110px"
                        />
                    </FormRow>
                    <FormSeparator heading={__("heating.content.status")} help={__("heating.content.status_help")}/>
                    <FormRow label={__("heating.content.price_based_heating_plan")} label_muted={__("heating.content.price_based_heating_plan_muted")}>
                    <div class="card">
                        <div style="position: relative;"> {/* this plain div is necessary to make the size calculation stable in safari. without this div the height continues to grow */}
                            <UplotLoader
                                ref={this.uplot_loader_ref}
                                show={true}
                                marker_class={'h4'}
                                no_data={__("day_ahead_prices.content.no_data")}
                                loading={__("day_ahead_prices.content.loading")}>
                                <UplotWrapperB
                                    ref={this.uplot_wrapper_ref}
                                    class="heating-chart"
                                    sub_page="heating"
                                    color_cache_group="heating.default"
                                    show={true}
                                    on_mount={() => this.update_uplot()}
                                    legend_time_label={__("day_ahead_prices.content.time")}
                                    legend_time_with_minutes={true}
                                    aspect_ratio={3}
                                    x_format={{hour: '2-digit', minute: '2-digit'}}
                                    x_padding_factor={0}
                                    x_include_date={true}
                                    y_unit="ct/kWh"
                                    y_label={__("day_ahead_prices.content.price_ct_per_kwh")}
                                    y_digits={3}
                                    only_show_visible={true}
                                    padding={[30, 15, null, 5]}
                                />
                            </UplotLoader>
                        </div>
                    </div>
                    </FormRow>
                    <FormRow label={__("heating.content.average_price")}>
                        <div class="row mx-n1">
                            <div class="col-md-6 px-1">
                                <div class="input-group">
                                    <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("today")}</span></div>
                                    <InputText
                                        value={util.get_value_with_unit(get_average_price_today(), "ct/kWh", 2, 1000)}
                                    />
                                </div>
                            </div>
                            <div class="col-md-6 px-1">
                                <div class="input-group">
                                    <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("tomorrow")}</span></div>
                                    <InputText
                                        value={util.get_value_with_unit(get_average_price_tomorrow(), "ct/kWh", 2, 1000)}
                                    />
                                </div>
                            </div>
                        </div>
                    </FormRow>
                    <FormRow label={__("heating.content.solar_forecast")}>
                        <div class="row mx-n1">
                            <div class="col-md-6 px-1">
                                <div class="input-group">
                                    <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("today")}</span></div>
                                    <InputText
                                        value={util.get_value_with_unit(get_kwh_today(), "kWh", 2)}
                                    />
                                </div>
                            </div>
                            <div class="col-md-6 px-1">
                                <div class="input-group">
                                    <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("tomorrow")}</span></div>
                                    <InputText
                                        value={util.get_value_with_unit(get_kwh_tomorrow(), "kWh", 2)}
                                    />
                                </div>
                            </div>
                        </div>
                    </FormRow>
                    <FormRow label={__("heating.content.sg_ready")}>
                        <div class="row mx-n1">
                            <div class="col-md-6 px-1">
                                <div class="input-group">
                                    <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("heating.content.output") + " 1"}</span></div>
                                    <InputText
                                        value={state.heating_state.sgr_blocking ? __("heating.content.active") : __("heating.content.inactive")}
                                    />
                                </div>
                            </div>
                            <div class="col-md-6 px-1">
                                <div class="input-group">
                                    <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("heating.content.output") + " 2"}</span></div>
                                    <InputText
                                        value={state.heating_state.sgr_extended ? __("heating.content.active") : __("heating.content.inactive")}
                                    />
                                </div>
                            </div>
                        </div>
                    </FormRow>

                    <FormSeparator heading={__("heating.content.p14_enwg")} help={__("heating.content.p14_enwg_help")}/>
                    <FormRow label={__("heating.content.p14_enwg")}>
                        <Switch desc={__("heating.content.p14_enwg_control_enabled")}
                                checked={state.p14enwg}
                                onClick={this.toggle('p14enwg')}
                        />
                    </FormRow>
                    <FormRow label={__("heating.content.input")}>
                        <InputSelect
                            items={[
                                ["0", __("heating.content.input") + " 1"],
                                ["1", __("heating.content.input") + " 2"],
                                ["2", __("heating.content.input") + " 3"],
                                ["3", __("heating.content.input") + " 4"],
                            ]}
                            value={state.p14enwg_input}
                            onValue={(v) => this.setState({p14enwg_input: parseInt(v)})}
                        />
                    </FormRow>
                    <FormRow label={__("heating.content.blocking_operation")}>
                        <InputSelect
                            items={[
                                ["0", __("heating.content.input_active_closed")],
                                ["1", __("heating.content.input_active_open")]
                            ]}
                            value={state.p14enwg_type}
                            onValue={(v) => this.setState({p14enwg_type: parseInt(v)})}
                        />
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

const MIN_OVERRIDE_DURATION = 15; // minutes

export class HeatingStatus extends Component<{}, state & sgr_blocking_override & {override_duration: number, override_remaining: number, blocking: boolean}>
{
    override_remaining_interval_id: number = undefined;

    constructor() {
        super();

        this.state = {
            sgr_blocking: false,
            sgr_extended: false,
            p14enwg: false,
            next_update: 0,
            override_until: 0,
            override_duration: MIN_OVERRIDE_DURATION,
            override_remaining: null,
            blocking: false,
        };

        util.addApiEventListener("heating/config", () => {
            let config = API.get("heating/config");

            this.setState({blocking: config.blocking});
        });

        util.addApiEventListener("heating/state", () => {
            this.setState({...API.get("heating/state")});
        });

        util.addApiEventListener("heating/sgr_blocking_override", () => {
            let sgr_blocking_override = API.get("heating/sgr_blocking_override");
            let override_remaining: number = null;

            if (sgr_blocking_override.override_until != 0) {
                override_remaining = this.get_override_remaining();

                if (this.override_remaining_interval_id === undefined) {
                    this.override_remaining_interval_id = setInterval(() => this.setState({override_remaining: this.get_override_remaining()}), 5000);
                }
            }
            else if (this.override_remaining_interval_id !== undefined) {
                clearInterval(this.override_remaining_interval_id);
                this.override_remaining_interval_id = undefined;
            }

            this.setState({override_until: sgr_blocking_override.override_until, override_remaining: override_remaining});
        });
    }

    get_override_remaining() {
        return Math.round(Math.max(0, this.state.override_until - util.get_date_now_1m_update_rate() / 60000));
    }

    render() {
        if (!util.render_allowed()) {
            return <StatusSection name="heating" />;
        }

        return <StatusSection name="heating">
            <FormRow hidden={!this.state.blocking} label={__("heating.content.override_blocking")}>
                <form onSubmit={async (e) => {
                    e.preventDefault();
                    e.stopPropagation();

                    let override_until = 0;
                    let override_remaining = 0;

                    if (this.state.override_until == 0) {
                        override_until = Math.round(util.get_date_now_1m_update_rate() / 60000 + this.state.override_duration);
                        override_remaining = this.state.override_duration;
                    }

                    await API.save("heating/sgr_blocking_override", {override_until: override_until}, () => "Override heating failed");

                    this.setState({override_until: override_until, override_remaining: override_remaining});
                }}>
                    <div className="row mx-n1">
                        <div className="col px-1">
                            {this.state.override_until != 0 ?
                                <InputNumber
                                    unit={__("heating.content.minutes")}
                                    value={this.state.override_remaining}
                                    readonly
                                /> :
                                <InputNumber
                                    disabled={!this.state.sgr_blocking}
                                    unit={__("heating.content.minutes")}
                                    value={this.state.override_duration}
                                    onValue={(v) => this.setState({override_duration: v})}
                                    min={MIN_OVERRIDE_DURATION}
                                    max={60}
                                />}
                        </div>
                        <div className="col-auto px-1">
                            <Button
                                disabled={!this.state.sgr_blocking && this.state.override_until == 0}
                                variant={this.state.override_until != 0 ? "warning" : "primary"}
                                type="submit">
                                {this.state.override_until != 0 ? __("heating.content.discard_override") : __("heating.content.override")}
                            </Button>
                        </div>
                    </div>
                    {this.state.sgr_extended/*p14enwg*/ ?
                        <div class="mt-1 mb-0">{__("heating.content.override_blocked_by_p14enwg")}</div>
                        : undefined}
                </form>
            </FormRow>
            <FormRow label={__("heating.content.sg_ready")} label_muted={__("heating.content.sg_ready_muted")}>
                <div class="row mx-n1">
                    <div class="col-md-6 px-1">
                        <div class="input-group">
                            <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("heating.content.blocked")}</span></div>
                            <InputText
                                value={this.state.sgr_blocking ? __("heating.content.active") : __("heating.content.inactive")}
                            />
                        </div>
                    </div>
                    <div class="col-md-6 px-1">
                        <div class="input-group">
                            <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("heating.content.extended")}</span></div>
                            <InputText
                                value={this.state.sgr_extended ? __("heating.content.active") : __("heating.content.inactive")}
                            />
                        </div>
                    </div>
                </div>
            </FormRow>
        </StatusSection>;
    }
}

export function init() {
}
