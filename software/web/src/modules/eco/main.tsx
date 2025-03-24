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
import { SwitchableInputNumber } from "../../ts/components/switchable_input_number";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { FormRow } from "../../ts/components/form_row";
import { is_solar_forecast_enabled } from  "../solar_forecast/main";
import { Switch } from "../../ts/components/switch";
import { InputSelect } from "../../ts/components/input_select";
import { InputNumber } from "../../ts/components/input_number";
import { InputTime } from "../../ts/components/input_time";
import { Button, Collapse, InputGroup } from "react-bootstrap";
import { UplotLoader } from "../../ts/components/uplot_loader";
import { UplotData, UplotWrapperB, UplotPath } from "../../ts/components/uplot_wrapper_2nd";
import { is_day_ahead_prices_enabled, get_price_from_index, get_prices_as_15min, get_price_from_index_as_15min } from "../day_ahead_prices/main";
import { Departure } from "./departure.enum";
import { Resolution } from "../day_ahead_prices/resolution.enum";
import { ConfigChargeMode } from "modules/charge_manager/config_charge_mode.enum";
import { get_allowed_charge_modes } from "modules/charge_manager/main";

export function EcoNavbar() {
    return (
        <NavbarItem
            name="eco"
            module="eco"
            title={__("eco.navbar.eco")}
            symbol={
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                    <path d="M4.44893 17.009C-0.246384 7.83762 7.34051 0.686125 19.5546 3.61245C20.416 3.81881 21.0081 4.60984 20.965 5.49452C20.5862 13.288 17.0341 17.7048 6.13252 17.9857"/>
                    <path d="M3.99999 21C5.50005 15.5 6 12.5 12 9.99997"/>
                </svg>
            }
        />
    );
}

type EcoConfig = API.getType["eco/config"];

interface EcoState {
    eco_state: API.getType["eco/state"];
    charge_plan: API.getType["eco/charge_plan"];
}

export class Eco extends ConfigComponent<'eco/config', {status_ref?: RefObject<EcoStatus>}, EcoState> {
    constructor() {
        super('eco/config',
             () => __("eco.script.save_failed"));
    }

    render(props: {}, state: EcoState & EcoConfig) {
        if (!util.render_allowed())
            return <SubPage name="eco" />;

        const solar_forecast_enabled   = is_solar_forecast_enabled();
        const day_ahead_prices_enabled = is_day_ahead_prices_enabled();

        return (
            <SubPage name="eco">
                <ConfigForm id="eco_config_form"
                            title={__("eco.content.eco")}
                            isModified={false}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("eco.content.charge_plan_enable")}
                             help={__("eco.content.charge_plan_enable_help")}
                             error={__("eco.content.day_ahead_prices_needs_activation")}
                             show_error={state.enable && !day_ahead_prices_enabled}>
                        <Switch desc={__("eco.content.charge_plan_enable_desc")}
                            checked={state.enable}
                            onClick={this.toggle('enable')}
                        />
                    </FormRow>
                    <FormRow label={__("eco.content.park_time")} help={__("eco.content.park_time_help")}>
                        <SwitchableInputNumber
                            switch_label_active={__("eco.content.active")}
                            switch_label_inactive={__("eco.content.inactive")}
                            unit="h"
                            checked={state.park_time}
                            onClick={this.toggle('park_time')}
                            value={state.park_time_duration}
                            onValue={this.set("park_time_duration")}
                            min={1}
                            max={48}
                            switch_label_min_width="110px"
                        />
                    </FormRow>
                    <FormRow label={__("eco.content.charge_below")} help={__("eco.content.charge_below_help")}>
                        <SwitchableInputNumber
                            switch_label_active={__("eco.content.active")}
                            switch_label_inactive={__("eco.content.inactive")}
                            unit="ct"
                            checked={state.charge_below}
                            onClick={this.toggle('charge_below')}
                            value={state.charge_below_threshold}
                            onValue={this.set("charge_below_threshold")}
                            min={-100}
                            max={5000}
                            switch_label_min_width="110px"
                        />
                    </FormRow>
                    <FormRow label={__("eco.content.block_above")} help={__("eco.content.block_above_help")}>
                        <SwitchableInputNumber
                            switch_label_active={__("eco.content.active")}
                            switch_label_inactive={__("eco.content.inactive")}
                            unit="ct"
                            checked={state.block_above}
                            onClick={this.toggle('block_above')}
                            value={state.block_above_threshold}
                            onValue={this.set("block_above_threshold")}
                            min={-100}
                            max={5000}
                            switch_label_min_width="110px"
                        />
                    </FormRow>
                    <FormRow label={__("eco.content.yield_forecast_threshold")} help={__("eco.content.yield_forecast_threshold_help")}
                             error={__("eco.content.solar_forecast_needs_activation")}
                             show_error={state.yield_forecast && !solar_forecast_enabled}>
                        <SwitchableInputNumber
                            switch_label_active={__("eco.content.active")}
                            switch_label_inactive={__("eco.content.inactive")}
                            unit={__("eco.content.kwh_per_day")}
                            checked={state.yield_forecast}
                            onClick={this.toggle('yield_forecast')}
                            value={state.yield_forecast_threshold}
                            onValue={this.set("yield_forecast_threshold")}
                            min={0}
                            max={1000}
                            switch_label_min_width="110px"
                        />
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

interface EcoStatusState {
    state: API.getType["eco/state"];
    charge_plan: API.getType["eco/charge_plan"];
}

export class EcoChart extends Component<{charger_id: number, departure?: Departure, time?: number, amount?: number, enable?: boolean}, {}> {
    uplot_loader_ref  = createRef();
    uplot_wrapper_ref = createRef();

    constructor() {
        super();

        util.addApiEventListener("day_ahead_prices/prices", () => {
            // Update chart every time new price data comes in
            this.update_uplot();
        });

        // Update vertical "now" line on time change
        // Calling async functions in effect is not supported
        // because signal accesses will not be tracked under await.
        // However update_uplot does access the 1 minute signal only
        // blocking, so this should be fine (tm)
        // See: https://github.com/preactjs/signals/issues/284#issuecomment-1871915973
        effect(() => this.update_uplot() as unknown as void);
    }

    async update_uplot() {
        // Use signal here to make effect() record its use, even
        // if this function might exit early on its first call
        let date_now = util.get_date_now_1m_update_rate();

        if (this.uplot_wrapper_ref.current == null) {
            return;
        }

        const eco_state = API.get("eco/state")

        // this.props.charger_id is -1 for status page charger, we always show charger 0 on status page
        const charger_id = Math.max(0, this.props.charger_id)
        if (eco_state.chargers.length <= charger_id ) {
            return;
        }

        if (this.props.charger_id >= 0 || (((this.props.enable == undefined) || this.props.enable) && (eco_state.chargers[charger_id].start > 0) && (eco_state.chargers[charger_id].amount > 0) && (eco_state.chargers[charger_id].chart != ""))) {
            const chart_base64 = eco_state.chargers[charger_id].chart;
            const chart_str    = atob(chart_base64);
            const chart_uint8  = Uint8Array.from(chart_str, c => c.charCodeAt(0));
            const chart_bool   = new Array(chart_uint8.length*8).fill(false).map((_, i) => (chart_uint8[Math.floor(i/8)] & (1 << (i % 8))) != 0);
            this.update_uplot_draw(date_now, chart_bool);
        } else {
            if (this.props.departure == undefined || this.props.time == undefined || this.props.amount == undefined) {
                return;
            }

            const charge_mode = API.get('power_manager/charge_mode').mode;
            const visible = charge_mode >= ConfigChargeMode.Eco && charge_mode <= ConfigChargeMode.EcoMinPV;
            // Don't poll eco/chart if the chart is not visible
            if (!visible) {
                return;
            }

            const eco_chart_data = {
                departure: this.props.departure,
                time: this.props.time,
                amount: this.props.amount,
                current_time: Math.floor(date_now / 60000) // current date in minutues
            };

            try {
                const eco_chart = await API.call("eco/chart", eco_chart_data, undefined, undefined, 2 * 60 * 1000);

                // Get array of bools from blob (binary bitfield)
                let buffer = await eco_chart.arrayBuffer();
                const array = new Uint8Array(buffer);
                const bool_array = new Array(array.length*8).fill(false).map((_, i) => (array[Math.floor(i/8)] & (1 << (i % 8))) != 0);
                this.update_uplot_draw(date_now, bool_array);
            } catch (e) {
                this.update_uplot_draw(date_now, []);
                throw e;
            }
        }
    }

    update_uplot_draw(date_now: number, green_blocks: boolean[]) {
        if (this.uplot_wrapper_ref.current == null) {
            return;
        }

        const prices = get_prices_as_15min();
        const first_date = API.get("day_ahead_prices/prices").first_date;
        const dap_config = API.get("day_ahead_prices/config");
        let data: UplotData;

        // If we have not got any prices yet, use empty data
        if (prices.length == 0) {
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
            const grid_costs_and_taxes_and_supplier_markup = dap_config.grid_costs_and_taxes / 1000.0 + dap_config.supplier_markup / 1000.0;
            for (let i = 0; i < prices.length; i++) {
                data.values[0].push(first_date * 60 + i * 60*15);
                data.values[1].push(get_price_from_index_as_15min(i) / 1000.0 + grid_costs_and_taxes_and_supplier_markup);
            }

            data.values[0].push(first_date * 60 + prices.length * 60 * 15 - 1);
            data.values[1].push(get_price_from_index_as_15min(prices.length - 1) / 1000.0 + grid_costs_and_taxes_and_supplier_markup);

            const from = date_now / 60000; // current date in minutues
            const from_index = Math.max(0, Math.floor((from - first_date) / 15))

            for (let i = from_index; i < Math.min(data.values[0].length, green_blocks.length + from_index); i++) {
                if (green_blocks[i-from_index]) {
                    data.lines_vertical.push({'index': i, 'text': '', 'color': [40, 167, 69, 0.5]});
                }
            }

            // Grey out history
            for (let i = 0; i < from_index; i++) {
                data.lines_vertical.push({'index': i, 'text': '', 'color': [64, 64, 64, 0.2]});
            }
        }

        // Show loader or data depending on the availability of data
        this.uplot_loader_ref.current.set_data(data && data.keys.length > 1);
        this.uplot_wrapper_ref.current.set_data(data);
    }

    render(props: {}) {
        return <div style="position: relative;"> {/* this plain div is necessary to make the size calculation stable in safari. without this div the height continues to grow */}
            <UplotLoader
                ref={this.uplot_loader_ref}
                show={true}
                marker_class={'h4'}
                no_data={__("day_ahead_prices.content.no_data")}
                loading={__("day_ahead_prices.content.loading")}>
                <UplotWrapperB
                    ref={this.uplot_wrapper_ref}
                    legend_show={false}
                    class="eco-chart"
                    sub_page="status"
                    color_cache_group="eco.default"
                    show={true}
                    on_mount={() => this.update_uplot()}
                    legend_time_label={__("day_ahead_prices.content.time")}
                    legend_time_with_minutes={true}
                    aspect_ratio={4}
                    x_format={{hour: '2-digit', minute: '2-digit'}}
                    x_padding_factor={0}
                    x_include_date={true}
                    y_unit="ct/kWh"
                    y_label="ct/kWh"
                    y_digits={3}
                    y_three_split={true}
                    only_show_visible={true}
                    grid_show={false}
                    padding={[null, 15, null, 5]}
                    height_min={100}
                />
            </UplotLoader>
        </div>
    }
}


export class EcoStatus extends Component<{}, EcoStatusState> {
    timeout: number                    = undefined;
    eco_chart_ref: RefObject<EcoChart> = createRef();

    constructor() {
        super();

        util.addApiEventListener('eco/state', () => {
            this.setState({state: API.get('eco/state')})
        });

        util.addApiEventListener('eco/charge_plan', () => {
            if((this.state.charge_plan === undefined) || (this.state.charge_plan.enable != API.get('eco/charge_plan').enable)) {
                this.setState({charge_plan: API.get('eco/charge_plan')})
            }
        });
    }

    get_date_from_minutes(minutes: number) {
        const h = Math.floor(minutes / 60);
        const m = minutes - h * 60;
        return new Date(0, 0, 1, h, m);
    }

    get_minutes_from_date(date: Date) {
        return date.getMinutes() + date.getHours()*60;
    }

    update_charge_plan(charge_plan: API.getType["eco/charge_plan"]) {
        if(this.timeout !== undefined) {
            clearTimeout(this.timeout);
        }

        const old_enable = API.get("eco/charge_plan").enable;

        if (charge_plan.enable != old_enable) {
            API.save("eco/charge_plan", charge_plan)
        } else {
            this.timeout = setTimeout(() => API.save("eco/charge_plan", charge_plan), 1000);
        }

        if(!charge_plan.enable) {
            this.eco_chart_ref.current?.update_uplot();
        }
    }

    render(props: {}, state: EcoStatusState) {
        if (!util.render_allowed()) {
            return <StatusSection name="eco" />
        }

        let charge_mode = API.get('power_manager/charge_mode').mode;
        let visible = charge_mode >= ConfigChargeMode.Eco && charge_mode <= ConfigChargeMode.EcoMinPV;

        let cpy = {...state.charge_plan, time: this.get_date_from_minutes(state.charge_plan.time)};

        return <StatusSection name="eco">
            <Collapse in={visible}><div>
            <FormRow label={__("eco.status.charge_plan")}
                     label_muted={__("eco.script.charge_plan")(cpy,
                                                               state.state.chargers.length == 0 ? -1 : state.state.chargers[0].start,
                                                               state.state.chargers.length == 0 ? -1 : state.state.chargers[0].amount)}>
                    <div class="input-group">
                        <div class="input-group-prepend"><span class="eco-fixed-size input-group-text">{__("eco.status.departure")}</span></div>
                        <InputSelect
                            disabled={state.charge_plan.enable}
                            items={[
                                ["0", __("today")],
                                ["1", __("tomorrow")],
                                ["2", __("eco.status.daily")],
                            ]}
                            value={state.charge_plan.departure}
                            onValue={(v) => this.setState({charge_plan: {...state.charge_plan, departure: parseInt(v)}}, () => this.update_charge_plan({...state.charge_plan, departure: parseInt(v)}))}
                        />
                        <InputTime
                            disabled={state.charge_plan.enable}
                            date={this.get_date_from_minutes(state.charge_plan.time)}
                            showSeconds={false}
                            onDate={(d: Date) => this.setState({charge_plan: {...state.charge_plan, time: this.get_minutes_from_date(d)}}, () => this.update_charge_plan({...state.charge_plan, time: this.get_minutes_from_date(d)}))}
                        />
                    </div>

                <div class="input-group flex-nowrap mt-2">
                        <div class="input-group-prepend"><span class="eco-fixed-size input-group-text">{__("eco.status.amount")}</span></div>
                        {/*TODO: fix rounded corners*/}
                        <InputNumber
                            disabled={state.charge_plan.enable}
                            unit="h"
                            value={state.charge_plan.amount}
                            onValue={(v) => this.setState({charge_plan: {...state.charge_plan, amount: v}}, () => this.update_charge_plan({...state.charge_plan, amount: v}))}
                            min={1}
                            max={48}
                        />
                </div>

                <div class="card mt-2">
                <EcoChart charger_id={-1} ref={this.eco_chart_ref} departure={this.state.charge_plan.departure} time={this.state.charge_plan.time} amount={this.state.charge_plan.amount} enable={this.state.charge_plan.enable}/>
                </div>
                <div class="mt-2">
                    <Button
                        onClick={() => this.setState({charge_plan: {...state.charge_plan, enable: !state.charge_plan.enable}}, () => this.update_charge_plan({...state.charge_plan, enable: !state.charge_plan.enable}))}
                        className="form-control"
                    >
                        <span class="text-nowrap">{state.charge_plan.enable ? __("eco.status.disable_charge_plan") : __("eco.status.enable_charge_plan")}</span>
                    </Button>
                </div>
            </FormRow>
            </div></Collapse>
        </StatusSection>;
    }
}

export function init() {
}
