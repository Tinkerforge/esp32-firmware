/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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
import { h, Fragment } from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import { InputNumber } from "../../ts/components/input_number";
import { InputText } from "../../ts/components/input_text";
import { OveR37State } from "./generated/ove_r37_state.enum";
import { MeterValueID } from "../meters/generated/meter_value_id";
import { OVE_R37_TRIP_REASON_UNDERVOLTAGE, OVE_R37_TRIP_REASON_OVERVOLTAGE, OVE_R37_TRIP_REASON_FREQUENCY, OVE_R37_FLAG_VOLTAGE_IN_RANGE, OVE_R37_FLAG_FREQUENCY_IN_RANGE, OVE_R37_FLAG_VOLTAGE_VALID, OVE_R37_FLAG_FREQUENCY_VALID } from "./api";

export function OveR37Navbar() {
    return (
        <NavbarItem
            name="ove_r37"
            module="ove_r37"
            title={__("ove_r37.navbar.ove_r37")}
            symbol={
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="currentColor" stroke="none">
                    <text x="12" y="18" font-size="20" text-anchor="middle">&#167;</text>
                </svg>
            }
        />
    );
}

function state_name(state: OveR37State): string {
    switch (state) {
        case OveR37State.Disabled: return __("ove_r37.content.state_disabled");
        case OveR37State.Normal:   return __("ove_r37.content.state_normal");
        case OveR37State.Tripped:  return __("ove_r37.content.state_tripped");
        case OveR37State.Wait:     return __("ove_r37.content.state_wait");
        case OveR37State.Ramp:     return __("ove_r37.content.state_ramp");
        default:                   return "" + state;
    }
}

function trip_reason_name(trip_reason: number): string {
    if (trip_reason === 0)
        return __("ove_r37.content.trip_none");

    let reasons: string[] = [];
    if (trip_reason & OVE_R37_TRIP_REASON_UNDERVOLTAGE) reasons.push(__("ove_r37.content.trip_undervoltage"));
    if (trip_reason & OVE_R37_TRIP_REASON_OVERVOLTAGE)  reasons.push(__("ove_r37.content.trip_overvoltage"));
    if (trip_reason & OVE_R37_TRIP_REASON_FREQUENCY)    reasons.push(__("ove_r37.content.trip_frequency"));
    return reasons.join(", ");
}

function get_charger_meter_value(value_id: number): number {
    const slot = API.get_unchecked('evse/meter_config')?.slot;
    if (slot === undefined || slot === null)
        return NaN;

    const value_ids = API.get_unchecked(`meters/${slot}/value_ids`) as Readonly<number[]>;
    const values = API.get_unchecked(`meters/${slot}/values`) as Readonly<number[]>;
    const idx = value_ids ? value_ids.indexOf(value_id) : -1;

    if (idx < 0 || !values || idx >= values.length)
        return NaN;

    return values[idx];
}

type OveR37Config = API.getType["ove_r37/config"];

interface OveR37PageState {
    password: string;
    config_enabled: boolean;
    ove_r37_state: API.getType["ove_r37/state"];
}

export class OveR37 extends ConfigComponent<'ove_r37/config', {}, OveR37PageState> {
    constructor() {
        super('ove_r37/config',
              () => __("ove_r37.script.save_failed"),
              undefined,
              {password: ""});

        util.addApiEventListener('ove_r37/config', () => {
            this.setState({config_enabled: API.get('ove_r37/config').enabled});
        });

        util.addApiEventListener('ove_r37/state', () => {
            this.setState({ove_r37_state: API.get('ove_r37/state')});
        });
    }

    override async sendSave(topic: "ove_r37/config", cfg: OveR37Config) {
        await API.call('ove_r37/config_update', {
            enabled: cfg.enabled,
            undervoltage_threshold: cfg.undervoltage_threshold,
            undervoltage_observation_time: cfg.undervoltage_observation_time,
            reconnect_wait_time: cfg.reconnect_wait_time,
            start_delay: cfg.start_delay,
            password: this.state.password,
        }, () => __("ove_r37.script.save_failed"));

        this.setState({password: "", config_enabled: cfg.enabled});
    }

    render(props: {}, state: OveR37Config & OveR37PageState) {
        if (!util.render_allowed())
            return <SubPage name="ove_r37" />;

        const s = state.ove_r37_state;

        const voltage_l1 = get_charger_meter_value(MeterValueID.VoltageL1N);
        const voltage_l2 = get_charger_meter_value(MeterValueID.VoltageL2N);
        const voltage_l3 = get_charger_meter_value(MeterValueID.VoltageL3N);
        const frequency  = get_charger_meter_value(MeterValueID.FrequencyLAvg);

        const in_range_text = (valid_flag: number, in_range_flag: number) =>
            s !== undefined && (s.flags & valid_flag)
                ? ((s.flags & in_range_flag) ? __("ove_r37.status.yes") : __("ove_r37.status.no"))
                : __("ove_r37.status.no_data");

        const voltages_text   = [voltage_l1, voltage_l2, voltage_l3].map((v) => isNaN(v) ? "-" : util.toLocaleFixed(v, 1)).join(" / ");
        const voltage_value   = in_range_text(OVE_R37_FLAG_VOLTAGE_VALID, OVE_R37_FLAG_VOLTAGE_IN_RANGE) + (isNaN(voltage_l1) && isNaN(voltage_l2) && isNaN(voltage_l3) ? "" : ` (${voltages_text} V)`);
        const frequency_value = in_range_text(OVE_R37_FLAG_FREQUENCY_VALID, OVE_R37_FLAG_FREQUENCY_IN_RANGE) + (isNaN(frequency) ? "" : ` (${util.toLocaleFixed(frequency, 3)} Hz)`);

        return (
            <SubPage name="ove_r37" title={__("ove_r37.content.ove_r37")}>
                <SubPage.Status collapsed={!state.config_enabled}>
                    <FormRow label={__("ove_r37.status.state")}>
                        <InputText value={s !== undefined ? state_name(s.state) : ""}/>
                    </FormRow>
                    {s !== undefined && (s.state == OveR37State.Tripped || s.trip_reason != 0) ?
                        <FormRow label={__("ove_r37.status.trip_reason")}>
                            <InputText value={trip_reason_name(s.trip_reason)}/>
                        </FormRow>
                        : undefined}
                    <FormRow label={__("ove_r37.status.voltage_in_range")}>
                        <InputText value={voltage_value}/>
                    </FormRow>
                    <FormRow label={__("ove_r37.status.frequency_in_range")}>
                        <InputText value={frequency_value}/>
                    </FormRow>
                </SubPage.Status>

                <SubPage.Config
                    id="ove_r37_config_form"
                    isDirty={this.isDirty()}
                    onSave={this.save}
                    onDirtyChange={this.setDirty}>

                    <FormRow label={__("ove_r37.content.enabled")} label_muted={__("ove_r37.content.enabled_muted")}>
                        <Switch
                            desc={__("ove_r37.content.enabled_desc")}
                            checked={state.enabled}
                            onClick={() => this.setState({enabled: !state.enabled})}/>
                    </FormRow>

                    <FormRow label={__("ove_r37.content.undervoltage_threshold")} label_muted={__("ove_r37.content.undervoltage_threshold_muted")}>
                        <InputNumber
                            unit="pu/1000"
                            value={state.undervoltage_threshold}
                            onValue={(v) => this.setState({undervoltage_threshold: v})}
                            min={0} max={1000}/>
                    </FormRow>

                    <FormRow label={__("ove_r37.content.undervoltage_observation_time")} label_muted={__("ove_r37.content.undervoltage_observation_time_muted")}>
                        <InputNumber
                            unit="ms"
                            value={state.undervoltage_observation_time}
                            onValue={(v) => this.setState({undervoltage_observation_time: v})}
                            min={0} max={65535}/>
                    </FormRow>

                    <FormRow label={__("ove_r37.content.reconnect_wait_time")} label_muted={__("ove_r37.content.reconnect_wait_time_muted")}>
                        <InputNumber
                            unit="s"
                            value={state.reconnect_wait_time}
                            onValue={(v) => this.setState({reconnect_wait_time: v})}
                            min={0} max={300}/>
                    </FormRow>

                    <FormRow label={__("ove_r37.content.start_delay")} label_muted={__("ove_r37.content.start_delay_muted")}>
                        <InputNumber
                            unit="s"
                            value={state.start_delay}
                            onValue={(v) => this.setState({start_delay: v})}
                            min={0} max={300}/>
                    </FormRow>

                    <FormRow label={__("ove_r37.content.password")} label_muted={__("ove_r37.content.password_muted")}>
                        <InputText
                            value={state.password}
                            onValue={(v) => this.setState({password: v})}
                            placeholder={__("ove_r37.content.password_placeholder")}
                            maxLength={64}/>
                    </FormRow>
                </SubPage.Config>
            </SubPage>
        );
    }
}

export function pre_init() {
}

export function init() {
}
