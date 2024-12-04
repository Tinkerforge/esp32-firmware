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
import { is_day_ahead_prices_enabled, get_average_price_today, get_average_price_tomorrow, get_price_from_index } from "../day_ahead_prices/main";
import { Depature } from "./depature.enum";

export function EcoNavbar() {
    return (
        <NavbarItem
            name="eco"
            title={__("eco.navbar.eco")}
            symbol={
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                    <path d="M4.44893 17.009C-0.246384 7.83762 7.34051 0.686125 19.5546 3.61245C20.416 3.81881 21.0081 4.60984 20.965 5.49452C20.5862 13.288 17.0341 17.7048 6.13252 17.9857C"/>
                    <path d="M3.99999 21C5.50005 15.5 6 12.5 12 9.99997"/>
                </svg>
            }
            hidden={false}
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
                    <FormRow label="Ladeplanung aktivieren" help="">
                        <Switch desc="Die Ladeplanung kann auf der Startseite im Modus Eco+PV aktiviert werden."
                            disabled={!day_ahead_prices_enabled}
                            checked={state.charge_plan_active && day_ahead_prices_enabled}
                            onClick={this.toggle('charge_plan_active')}
                        />
                    </FormRow>
                    <FormRow label="Modus nach Ablauf des Ladeplans">
                        <InputSelect
                            disabled={!day_ahead_prices_enabled || !state.charge_plan_active}
                            items={[
                                ["0", "Schnell"],
                                ["1", "Eco+PV"],
                                ["2", "PV"],
                                ["3", "Aus"],
                            ]}
                            value={state.mode_after_charge_plan}
                            onValue={(v) => this.setState({mode_after_charge_plan: parseInt(v)})}
                        />
                    </FormRow>
                    <FormRow label="Maximale Standzeit" help="">
                        <SwitchableInputNumber
                            disabled={!day_ahead_prices_enabled || !state.charge_plan_active}
                            switch_label_active={__("eco.content.active")}
                            switch_label_inactive={__("eco.content.inactive")}
                            unit="h"
                            checked={state.service_life_active && day_ahead_prices_enabled && state.charge_plan_active}
                            onClick={this.toggle('service_life_active')}
                            value={state.service_life}
                            onValue={this.set("service_life")}
                            min={1}
                            max={48}
                            switch_label_min_width="100px"
                        />
                    </FormRow>
                    <FormRow label="Immer laden wenn Preis unter" help="">
                        <SwitchableInputNumber
                            disabled={!day_ahead_prices_enabled}
                            switch_label_active={__("eco.content.active")}
                            switch_label_inactive={__("eco.content.inactive")}
                            unit="ct"
                            checked={state.charge_below_active && day_ahead_prices_enabled}
                            onClick={this.toggle('charge_below_active')}
                            value={state.charge_below}
                            onValue={this.set("charge_below")}
                            min={-100}
                            max={5000}
                            switch_label_min_width="100px"
                        />
                    </FormRow>
                    <FormRow label="Nie laden wenn Preis über" help="">
                        <SwitchableInputNumber
                            disabled={!day_ahead_prices_enabled}
                            switch_label_active={__("eco.content.active")}
                            switch_label_inactive={__("eco.content.inactive")}
                            unit="ct"
                            checked={state.block_above_active && day_ahead_prices_enabled}
                            onClick={this.toggle('block_above_active')}
                            value={state.block_above}
                            onValue={this.set("block_above")}
                            min={-100}
                            max={5000}
                            switch_label_min_width="100px"
                        />
                    </FormRow>
                    <FormRow label="Nur wenn PV-Ertragsprognose unter" help="">
                    <SwitchableInputNumber
                            disabled={!solar_forecast_enabled}
                            switch_label_active={__("eco.content.active")}
                            switch_label_inactive={__("eco.content.inactive")}
                            unit={"kWh/Tag"}
                            checked={state.yield_forecast_active && solar_forecast_enabled}
                            onClick={this.toggle('yield_forecast_active')}
                            value={state.yield_forecast}
                            onValue={this.set("yield_forecast")}
                            min={0}
                            max={1000}
                            switch_label_min_width="100px"
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

export class EcoStatus extends Component<{}, EcoStatusState> {
    uplot_loader_ref  = createRef();
    uplot_wrapper_ref = createRef();
    timeout: number   = undefined;

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

        util.addApiEventListener("day_ahead_prices/prices", () => {
            // Update chart every time new price data comes in
            this.update_uplot();
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


    update_uplot() {
        if (this.uplot_wrapper_ref.current == null) {
            return;
        }

        console.log("update_uplot");

        const dap_prices = API.get("day_ahead_prices/prices");
        const dap_config = API.get("day_ahead_prices/config");
        let data: UplotData;

        console.log(dap_prices);
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
            console.log(data);

            const last_save = this.state.state.last_charge_plan_save;
            const enabled   = this.state.charge_plan.enabled;
            const hours     = this.state.charge_plan.amount;
            const depature  = this.state.charge_plan.depature;
            const time      = this.state.charge_plan.time;

            // TODO: Use charge start if charge already started
            // TODO: Remove charged hours if charge already started
            const from = util.get_date_now_1m_update_rate()/(1000*60); // current date in minutues
            const from_index = Math.max(0, Math.floor((from - dap_prices.first_date) / resolution_multiplier));

            // Minutes from today 00:00 to end of charge plan
            const minutes_add = (((depature == Depature.Today) || (depature == Depature.Daily)) ? 0 : 24*60) + time;
            let to_date = new Date();
            to_date.setHours(0, 0, 0, 0)
            const to = to_date.getTime()/(60*1000) + minutes_add;
            const to_index = Math.min(dap_prices.prices.length, Math.ceil((to - dap_prices.first_date) / resolution_multiplier));

            console.log("from: " + from + " from_index: " + from_index + " to: " + to + " to_index: " + to_index);

            for (let i = 0; i < from_index; i++) {
                data.lines_vertical.push({'index': i, 'text': '', 'color': [64, 64, 64, 0.2]});
            }

            if (from_index < to_index) {
                const cheap_hours = data.values[1]
                .slice(from_index, to_index)
                .map((price, index) => ({ price, index }))
                .sort((a, b) => a.price - b.price)
                .slice(0, hours)
                .map(item => item.index+from_index);

                cheap_hours.forEach(index => {
                    data.lines_vertical.push({'index': index, 'text': '', 'color': [40, 167, 69, 0.5]});
                });
                console.log(cheap_hours);
            }

            // Add vertical line at current time
            /*const resolution_divisor = dap_prices.resolution == 0 ? 15 : 60;
            const diff = Math.floor(util.get_date_now_1m_update_rate() / 60000) - dap_prices.first_date;
            const index = Math.floor(diff / resolution_divisor);
            data.lines_vertical.push({'index': index, 'text': __("day_ahead_prices.content.now"), 'color': [64, 64, 64, 0.2]});*/
        }

        // Show loader or data depending on the availability of data
        this.uplot_loader_ref.current.set_data(data && data.keys.length > 1);
        this.uplot_wrapper_ref.current.set_data(data);
    }

    update_charge_plan(charge_plan: API.getType["eco/charge_plan"]) {
        if(this.timeout !== undefined) {
            clearTimeout(this.timeout);
        }

        console.log(charge_plan);

        this.timeout = setTimeout(() => API.save("eco/charge_plan", charge_plan), 1000);

        this.update_uplot();
    }

    render(props: {}, state: EcoStatusState) {
        if (!util.render_allowed()) {
            return <StatusSection name="eco" />
        }

        let charge_plan_text = () => {
            let day = "bis Heute um";
            if (state.charge_plan.depature === 1) {
                day = "bis Morgen um";
            } else if (state.charge_plan.depature === 2) {
                day = "täglich bis";
            }

            const active = state.charge_plan.enabled ? "aktiv" : "nicht aktiv";
            const time = this.get_date_from_minutes(state.charge_plan.time).toLocaleTimeString([], {hour: '2-digit', minute: '2-digit'});
            return `Aktueller Ladeplan: Nutze die günstigsten ${state.charge_plan.amount} Stunden ${day} ${time} Uhr. Der Ladeplan ist ${active}.`;
        };

        return <StatusSection name="eco">
            <FormRow label="Ladeplan" label_muted={charge_plan_text()}>
                <div class="col-md-16">
                    <div class="input-group">
                        <div class="input-group-prepend"><span class="eco-fixed-size input-group-text">Abfahrt</span></div>
                        <InputSelect
                            disabled={state.charge_plan.enabled}
                            items={[
                                ["0", "Heute"],
                                ["1", "Morgen"],
                                ["2", "Täglich"],
                            ]}
                            value={state.charge_plan.depature}
                            onValue={(v) => this.setState({charge_plan: {...state.charge_plan, depature: parseInt(v)}}, () => this.update_charge_plan({...state.charge_plan, depature: parseInt(v)}))}
                        />
                        <InputTime
                            disabled={state.charge_plan.enabled}
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
                            disabled={state.charge_plan.enabled}
                            unit="h"
                            value={state.charge_plan.amount}
                            onValue={(v) => this.setState({charge_plan: {...state.charge_plan, amount: v}}, () => this.update_charge_plan({...state.charge_plan, amount: v}))}
                            min={1}
                            max={48}
                        />
                    </div>
                </div>
                <div style="position: relative;"> {/* this plain div is neccessary to make the size calculation stable in safari. without this div the height continues to grow */}
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
                            only_show_visible={true}
                            grid_show={false}
                            y_three_split={true}
                            padding={[10, 10, null, 5]}
                        />
                    </UplotLoader>
                </div>
                <div class="form-group mt-2">
                    <Button
                        onClick={() => this.setState({charge_plan: {...state.charge_plan, enabled: !state.charge_plan.enabled}}, () => this.update_charge_plan({...state.charge_plan, enabled: !state.charge_plan.enabled}))}
                        className="form-control"
                    >
                        <span class="text-nowrap">{state.charge_plan.enabled ? "Ladeplan deaktiveren" : "Ladeplan aktivieren"}</span>
                    </Button>
                </div>
            </FormRow>
        </StatusSection>;
    }
}

export function init() {
}
