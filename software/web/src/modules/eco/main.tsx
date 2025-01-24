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
import { Button } from "react-bootstrap";
import { UplotLoader } from "../../ts/components/uplot_loader";
import { UplotData, UplotWrapper, UplotPath } from "../../ts/components/uplot_wrapper_2nd";
import { is_day_ahead_prices_enabled, get_price_from_index, get_prices_as_15min, get_price_from_index_as_15min } from "../day_ahead_prices/main";
import { Departure } from "./departure.enum";
import { Resolution} from "../day_ahead_prices/resolution.enum";

export function EcoNavbar() {
    return (
        <NavbarItem
            name="eco"
            module="eco"
            title={__("eco.navbar.eco")}
            symbol={
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                    <path d="M4.44893 17.009C-0.246384 7.83762 7.34051 0.686125 19.5546 3.61245C20.416 3.81881 21.0081 4.60984 20.965 5.49452C20.5862 13.288 17.0341 17.7048 6.13252 17.9857C"/>
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
                    <FormRow label="Ladeplanung aktivieren" help={<>{!day_ahead_prices_enabled && __("eco.content.day_ahead_prices_needs_activation")} {__("eco.content.charge_plan_enable_help")}</>}>
                        <Switch desc="Ladeplanung anhand von dynamischen Strompreisen und PV-Ertragsprognose."
                            disabled={!day_ahead_prices_enabled}
                            checked={state.enable && day_ahead_prices_enabled}
                            onClick={this.toggle('enable')}
                        />
                    </FormRow>
                    <FormRow label="Modus nach Ablauf des Ladeplans" help={<>{!day_ahead_prices_enabled && __("eco.content.day_ahead_prices_needs_activation")} {__("eco.content.mode_after_help")}</>}>
                        <InputSelect
                            disabled={!day_ahead_prices_enabled || !state.enable}
                            items={[
                                ["0", "Schnell"],
                                ["1", "Eco+PV"],
                                ["2", "PV"],
                                ["3", "Aus"],
                            ]}
                            value={state.mode_after}
                            onValue={(v) => this.setState({mode_after: parseInt(v)})}
                        />
                    </FormRow>
                    <FormRow label="Maximale Standzeit" help={<>{!day_ahead_prices_enabled && __("eco.content.day_ahead_prices_needs_activation")} {__("eco.content.park_time_help")}</>}>
                        <SwitchableInputNumber
                            disabled={!day_ahead_prices_enabled || !state.enable}
                            switch_label_active={__("eco.content.active")}
                            switch_label_inactive={__("eco.content.inactive")}
                            unit="h"
                            checked={state.park_time && day_ahead_prices_enabled && state.enable}
                            onClick={this.toggle('park_time')}
                            value={state.park_time_duration}
                            onValue={this.set("park_time_duration")}
                            min={1}
                            max={48}
                            switch_label_min_width="110px"
                        />
                    </FormRow>
                    <FormRow label="Immer laden wenn Preis unter" help={<>{!day_ahead_prices_enabled && __("eco.content.day_ahead_prices_needs_activation")} {__("eco.content.charge_below_help")}</>}>
                        <SwitchableInputNumber
                            disabled={!day_ahead_prices_enabled}
                            switch_label_active={__("eco.content.active")}
                            switch_label_inactive={__("eco.content.inactive")}
                            unit="ct"
                            checked={state.charge_below && day_ahead_prices_enabled}
                            onClick={this.toggle('charge_below')}
                            value={state.charge_below_threshold}
                            onValue={this.set("charge_below_threshold")}
                            min={-100}
                            max={5000}
                            switch_label_min_width="110px"
                        />
                    </FormRow>
                    <FormRow label="Nie laden wenn Preis über" help={<>{!day_ahead_prices_enabled && __("eco.content.day_ahead_prices_needs_activation")} {__("eco.content.block_above_help")}</>}>
                        <SwitchableInputNumber
                            disabled={!day_ahead_prices_enabled}
                            switch_label_active={__("eco.content.active")}
                            switch_label_inactive={__("eco.content.inactive")}
                            unit="ct"
                            checked={state.block_above && day_ahead_prices_enabled}
                            onClick={this.toggle('block_above')}
                            value={state.block_above_threshold}
                            onValue={this.set("block_above_threshold")}
                            min={-100}
                            max={5000}
                            switch_label_min_width="110px"
                        />
                    </FormRow>
                    <FormRow label="Nur wenn PV-Ertragsprognose unter" help={<>{!solar_forecast_enabled && __("eco.content.solar_forecast_needs_activation")} {__("eco.content.yield_forecast_threshold_help")}</>}>
                    <SwitchableInputNumber
                            disabled={!solar_forecast_enabled}
                            switch_label_active={__("eco.content.active")}
                            switch_label_inactive={__("eco.content.inactive")}
                            unit={"kWh/Tag"}
                            checked={state.yield_forecast && solar_forecast_enabled}
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
        effect(() => this.update_uplot());
    }

    update_uplot() {
        // Use signal here to make effect() record its use, even
        // if this function might exit early on its first call
        let date_now = util.get_date_now_1m_update_rate();

        if (this.uplot_wrapper_ref.current == null) {
            return;
        }

        const eco_state       = API.get("eco/state")
        const eco_charge_plan = API.get("eco/charge_plan")

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
            const eco_chart_data = {
                departure: this.props.departure,
                time: this.props.time,
                amount: this.props.amount,
                current_time: Math.floor(date_now / 60000) // current date in minutues
            };

            try {
                const eco_chart = API.call("eco/chart", eco_chart_data, undefined, undefined, 2 * 60 * 1000);

                // Get array of bools from blob (binary bitfield)
                eco_chart.then((blob) => {
                    blob.arrayBuffer().then((buffer) => {
                        const array = new Uint8Array(buffer);
                        const bool_array = new Array(array.length*8).fill(false).map((_, i) => (array[Math.floor(i/8)] & (1 << (i % 8))) != 0);
                        this.update_uplot_draw(date_now, bool_array);
                    });
                });
            } catch (e) {
                // The errors we can get here are mostly stuff like "no dap data available" or "no time available".
                // We should not annoy the user with error popups for this.
                this.update_uplot_draw(date_now, []);
                console.log("Error while fetching eco chart data:", e);
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
        return <div style="position: relative;"> {/* this plain div is neccessary to make the size calculation stable in safari. without this div the height continues to grow */}
            <UplotLoader
                ref={this.uplot_loader_ref}
                show={true}
                marker_class={'h4'}
                no_data={__("day_ahead_prices.content.no_data")}
                loading={__("day_ahead_prices.content.loading")}>
                <UplotWrapper
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
                    x_height={50}
                    x_format={{hour: '2-digit', minute: '2-digit'}}
                    x_padding_factor={0}
                    x_include_date={true}
                    y_unit={"ct/kWh"}
                    y_label={"ct/kWh"}
                    y_digits={3}
                    y_three_split={true}
                    only_show_visible={true}
                    grid_show={false}
                    padding={[10, 10, null, 5]}
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
            if(this.state.charge_plan === undefined) {
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

        // TODO: This function needs to go into translation_*.tsx
        let charge_plan_text = () => {
            let day = "bis Heute um";
            if (state.charge_plan.departure === 1) {
                day = "bis Morgen um";
            } else if (state.charge_plan.departure === 2) {
                day = "täglich bis";
            }

            const active = state.charge_plan.enable ? "aktiv" : "nicht aktiv";
            const time   = this.get_date_from_minutes(state.charge_plan.time).toLocaleTimeString([], {hour: '2-digit', minute: '2-digit'});
            const plan   = `Aktueller Ladeplan: Nutze die günstigsten ${state.charge_plan.amount} Stunden ${day} ${time} Uhr. Der Ladeplan ist ${active}.`;
            if (!state.charge_plan.enable || state.state.chargers.length == 0) {
                return plan;
            }

            const start  = state.state.chargers[0].start*60*1000;
            const amount = state.state.chargers[0].amount;

            if (start == 0) {
                const status = `Status: Kein Auto angeschlossen.`;
                return <div>{plan}<br/>{status}</div>;
            }

            const today     = new Date().setHours(0, 0, 0, 0);
            const start_day = new Date(start).setHours(0, 0, 0, 0);

            const begin = today == start_day ?
                `Ladebeginn: Heute, ${new Date(start).toLocaleString([], {hour: '2-digit', minute: '2-digit'})}` :
                `Ladebeginn: ${new Date(start).toLocaleString([], {weekday: 'long', hour: '2-digit', minute: '2-digit'})}`;
            const charging_done = `Ladedauer bisher: ${amount} Minuten.`;
            const charging_todo = `Ladedauer ausstehend: ${state.charge_plan.amount*60 - amount} Minuten.`;

            return <div>{plan}<br/>{begin}<br/>{charging_done}<br/>{charging_todo}</div>;
        };

        return <StatusSection name="eco">
            <FormRow label="Ladeplan" label_muted={charge_plan_text()}>
                <div class="col-md-16">
                    <div class="input-group">
                        <div class="input-group-prepend"><span class="eco-fixed-size input-group-text">Abfahrt</span></div>
                        <InputSelect
                            disabled={state.charge_plan.enable}
                            items={[
                                ["0", "Heute"],
                                ["1", "Morgen"],
                                ["2", "Täglich"],
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
                </div>
                <div class="col-md-16 mt-1">
                    <div class="input-group flex-nowrap">
                        <div class="input-group-prepend"><span class="eco-fixed-size input-group-text">Ladedauer</span></div>
                        <InputNumber
                            disabled={state.charge_plan.enable}
                            unit="h"
                            value={state.charge_plan.amount}
                            onValue={(v) => this.setState({charge_plan: {...state.charge_plan, amount: v}}, () => this.update_charge_plan({...state.charge_plan, amount: v}))}
                            min={1}
                            max={48}
                        />
                    </div>
                </div>
                <div class="card mt-1">
                <EcoChart charger_id={-1} ref={this.eco_chart_ref} departure={this.state.charge_plan.departure} time={this.state.charge_plan.time} amount={this.state.charge_plan.amount} enable={this.state.charge_plan.enable}/>
                </div>
                <div class="mt-2">
                    <Button
                        onClick={() => this.setState({charge_plan: {...state.charge_plan, enable: !state.charge_plan.enable}}, () => this.update_charge_plan({...state.charge_plan, enable: !state.charge_plan.enable}))}
                        className="form-control"
                    >
                        <span class="text-nowrap">{state.charge_plan.enable ? "Ladeplan deaktiveren" : "Ladeplan aktivieren"}</span>
                    </Button>
                </div>
            </FormRow>
        </StatusSection>;
    }
}

export function init() {
}
