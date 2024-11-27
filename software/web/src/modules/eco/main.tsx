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
import { Thermometer } from "react-feather";
import { StatusSection } from "../../ts/components/status_section";
import { FormRow } from "../../ts/components/form_row";
import { is_solar_forecast_enabled } from  "../solar_forecast/main";
import { is_day_ahead_prices_enabled } from "../day_ahead_prices/main";
import { Switch } from "../../ts/components/switch";
import { InputSelect } from "../../ts/components/input_select";
import { InputNumber } from "../../ts/components/input_number";
import { InputTime } from "../../ts/components/input_time";
import { Button } from "react-bootstrap";

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
              __("eco.script.save_failed"));
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
    timeout: number = undefined;

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

        console.log(charge_plan);

        this.timeout = setTimeout(() => API.save("eco/charge_plan", charge_plan), 1000);
    }

    render(props: {}, state: EcoStatusState) {
        if (!util.render_allowed()) {
            return <StatusSection name="eco" />
        }

        let charge_plan_text = () => {
            let day = "bis Heute um";
            if (state.charge_plan.day === 1) {
                day = "bis Morgen um";
            } else if (state.charge_plan.day === 2) {
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
                        <div class="input-group-prepend"><span class="eco-fixed-size input-group-text">Tag</span></div>
                        <InputSelect
                            disabled={state.charge_plan.enabled}
                            items={[
                                ["0", "Heute"],
                                ["1", "Morgen"],
                                ["2", "Täglich"],
                            ]}
                            value={state.charge_plan.day}
                            onValue={(v) => this.setState({charge_plan: {...state.charge_plan, day: parseInt(v)}}, () => this.update_charge_plan({...state.charge_plan, day: parseInt(v)}))}
                        />
                    </div>
                </div>
                <div class="col-md-16 mt-1">
                    <div class="input-group">
                        <div class="input-group-prepend"><span class="eco-fixed-size input-group-text">Uhrzeit</span></div>
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
