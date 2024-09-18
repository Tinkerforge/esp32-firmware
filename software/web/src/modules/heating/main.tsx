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
import { h, Fragment } from "preact";
import { __ } from "../../ts/translation";
import { METERS_SLOTS } from "../../build";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { InputNumber } from "../../ts/components/input_number";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Thermometer } from "react-feather";
import { InputTime } from "../../ts/components/input_time";
import { Collapse } from "react-bootstrap";
import { InputSelect } from "../../ts/components/input_select";
import { MeterValueID    } from "../meters/meter_value_id";
import { get_noninternal_meter_slots } from "../power_manager/main";

export function HeatingNavbar() {
    return <NavbarItem name="heating" title={__("heating.navbar.heating")} symbol={<Thermometer />} hidden={false} />;
}

type HeatingConfig = API.getType["heating/config"];

export class Heating extends ConfigComponent<'heating/config'> {
    summer_start_day:   number;
    summer_start_month: number;
    summer_end_day:     number;
    summer_end_month:   number;

    static days: [string, string][] = [...Array(31).keys()].map((i) => [
        (i+1).toString(),
        (i+1).toString()
    ]);

    static months: [string, string][] = [
        ["1", "Januar"],
        ["2", "Februar"],
        ["3", "März"],
        ["4", "April"],
        ["5", "Mai"],
        ["6", "Juni"],
        ["7", "Juli"],
        ["8", "August"],
        ["9", "September"],
        ["10", "Oktober"],
        ["11", "November"],
        ["12", "Dezember"]
    ];

    constructor() {
        super('heating/config',
              __("heating.script.save_failed"));
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
            case 1: case 3: case 5: case 7: case 8: case 10: case 12: return Heating.days.slice(0, 31);
            case 4: case 6: case 9: case 11:                          return Heating.days.slice(0, 30);
            case 2:                                                   return Heating.days.slice(0, 28);
            default: console.log("Invalid month: " + month);
        }
        return Heating.days.slice(0, 31);
    }

    recalculateSummer(state: Readonly<HeatingConfig>) {
        this.summer_start_day = state.winter_end_day + 1;
        this.summer_start_month = state.winter_end_month;
        if(this.summer_start_day > this.month_to_days(state.winter_end_month).length) {
            this.summer_start_day = 1;
            this.summer_start_month = state.winter_end_month + 1 > 12 ? 1 : state.winter_end_month + 1;
        }

        this.summer_end_day = state.winter_start_day - 1;
        this.summer_end_month = state.winter_start_month;
        if(this.summer_end_day < 1) {
            this.summer_end_month = state.winter_start_month - 1 < 1 ? 12 : state.winter_start_month - 1;
            this.summer_end_day = this.month_to_days(this.summer_end_month).length;
        }
    }

    render(props: {}, state: Readonly<HeatingConfig>) {
        if (!util.render_allowed())
            return <SubPage name="heating" />;

        let days_winter_start = this.month_to_days(state.winter_start_month);
        let days_winter_end = this.month_to_days(state.winter_end_month);
        let days_summer_start = this.month_to_days(1);
        let days_summer_end = this.month_to_days(1);

        this.recalculateSummer(state);

        const meter_slots = get_noninternal_meter_slots([MeterValueID.PowerActiveLSumImExDiff], __("power_manager.content.meter_slot_grid_power_missing_value"));

        return (
            <SubPage name="heating">
                <ConfigForm id="heating_config_form"
                            title={__("heating.content.heating")}
                            isModified={this.isModified()}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("heating.content.control_signal")}>
                        {__("heating.content.control_signal_description")}
                    </FormRow>
                    <FormRow label={__("heating.content.minimum_holding_time")} label_muted={__("heating.content.minimum_holding_time_description")}>
                        <InputNumber
                            unit={__("heating.content.minutes")}
                            value={state.minimum_control_holding_time}
                            onValue={this.set("minimum_control_holding_time")}
                            min={0}
                            max={60}
                        />
                    </FormRow>
                    <FormRow label={__("heating.content.meter_slot_grid_power")} label_muted={__("heating.content.meter_slot_grid_power_muted")}>
                        <InputSelect
                            placeholder={meter_slots.length > 0 ? __("heating.content.meter_slot_grid_power_select") : __("heating.content.meter_slot_grid_power_none")}
                            items={meter_slots}
                            value={state.meter_slot_grid_power}
                            onValue={(v) => this.setState({meter_slot_grid_power: parseInt(v)})}
                        />
                    </FormRow>
                    <FormRow label={__("heating.content.extended_logging")} label_muted={__("heating.content.extended_logging_description")}>
                        <Switch desc={__("heating.content.extended_logging_activate")}
                                checked={state.extended_logging_active}
                                onClick={this.toggle('extended_logging_active')}
                        />
                    </FormRow>
                    <FormSeparator heading={__("heating.content.winter_mode")}/>

                    <FormRow label={__("heating.content.winter_start")} label_muted="">
                        <div class="row no-gutters">
                            <div class="col-md-6">
                                <div class="input-group">
                                    <div class="input-group-prepend heating-input-group-prepend"><span class="heating-fixed-size input-group-text">{__("heating.content.month")}</span></div>
                                    <InputSelect
                                        className="heating-input-group-prepend"
                                        items={Heating.months}
                                        value={state.winter_start_month}
                                        onValue={(v) => {
                                            this.setState({winter_start_month: parseInt(v)});
                                            days_winter_start = this.month_to_days(parseInt(v));
                                            this.recalculateSummer(state);
                                        }}
                                    />
                                </div>
                            </div>
                            <div class="col-md-6">
                                <div class="input-group">
                                    <div class="input-group-prepend heating-input-group-append"><span class="heating-fixed-size input-group-text">{__("heating.content.day")}</span></div>
                                    <InputSelect
                                        className="heating-input-group-prepend"
                                        items={days_winter_start}
                                        value={state.winter_start_day}
                                        onValue={(v) => {
                                            this.setState({winter_start_day: parseInt(v)})
                                            this.recalculateSummer(state);
                                        }}
                                    />
                                </div>
                            </div>
                        </div>
                    </FormRow>
                    <FormRow label={__("heating.content.winter_end")} label_muted="">
                        <div class="row no-gutters">
                            <div class="col-md-6">
                                <div class="input-group">
                                    <div class="input-group-prepend heating-input-group-prepand"><span class="heating-fixed-size input-group-text">{__("heating.content.month")}</span></div>
                                    <InputSelect
                                        className="heating-input-group-prepend"
                                        items={Heating.months}
                                        value={state.winter_end_month}
                                        onValue={(v) => {
                                            this.setState({winter_end_month: parseInt(v)});
                                            days_winter_end = this.month_to_days(parseInt(v));
                                            this.recalculateSummer(state);
                                        }}
                                    />
                                </div>
                            </div>
                            <div class="col-md-6">
                                <div class="input-group">
                                    <div class="input-group-prepend heating-input-group-append"><span class="heating-fixed-size input-group-text">{__("heating.content.day")}</span></div>
                                    <InputSelect
                                        className="heating-input-group-prepend"
                                        items={days_winter_end}
                                        value={state.winter_end_day}
                                        onValue={(v) => {
                                            this.setState({winter_end_day: parseInt(v)})
                                            this.recalculateSummer(state);
                                        }}
                                    />
                                </div>
                            </div>
                        </div>
                    </FormRow>
                    <FormRow label={__("heating.content.dynamic_price_control")}>
                        <Switch desc={__("heating.content.dynamic_price_control_activate")}
                                checked={state.winter_dynamic_price_control_active}
                                onClick={this.toggle('winter_dynamic_price_control_active')}
                        />
                    </FormRow>
                    <Collapse in={state.winter_dynamic_price_control_active}>
                        <div>
                            <FormRow label={__("heating.content.average_price_threshold")} label_muted={__("heating.content.average_price_threshold_description")}>
                                <InputNumber
                                    unit="%"
                                    value={state.winter_dynamic_price_control_threshold}
                                    onValue={this.set("winter_dynamic_price_control_threshold")}
                                    min={0}
                                    max={100}
                                />
                            </FormRow>
                        </div>
                    </Collapse>
                    <FormRow label={__("heating.content.pv_excess_control")}>
                        <Switch desc={__("heating.content.pv_excess_control_activate")}
                                checked={state.winter_pv_excess_control_active}
                                onClick={this.toggle('winter_pv_excess_control_active')}
                        />
                    </FormRow>
                    <Collapse in={state.winter_pv_excess_control_active}>
                        <div>
                            <FormRow label={__("heating.content.pv_excess_threshold")} label_muted={__("heating.content.pv_excess_threshold_description")}>
                                <InputNumber
                                    unit="Watt"
                                    value={state.winter_pv_excess_control_threshold}
                                    onValue={this.set("winter_pv_excess_control_threshold")}
                                    min={0}
                                    max={100000}
                                />
                            </FormRow>
                        </div>
                    </Collapse>

                    <FormSeparator heading={__("heating.content.summer_mode")}/>
                    <FormRow label="Sommer Start" label_muted="">
                        <div class="row no-gutters">
                            <div class="col-md-6">
                                <div class="input-group">
                                    <div class="input-group-prepend heating-input-group-prepand"><span class="heating-fixed-size input-group-text">{__("heating.content.month")}</span></div>
                                    <InputSelect
                                        className="heating-input-group-prepend"
                                        items={Heating.months}
                                        value={this.summer_start_month}
                                    />
                                </div>
                            </div>
                            <div class="col-md-6">
                                <div class="input-group">
                                    <div class="input-group-prepend heating-input-group-append"><span class="heating-fixed-size input-group-text">{__("heating.content.day")}</span></div>
                                    <InputSelect
                                        className="heating-input-group-prepend"
                                        items={days_summer_start}
                                        value={this.summer_start_day}
                                    />
                                </div>
                            </div>
                        </div>
                    </FormRow>
                    <FormRow label="Sommer Ende" label_muted="">
                        <div class="row no-gutters">
                            <div class="col-md-6">
                                <div class="input-group">
                                    <div class="input-group-prepend heating-input-group-prepend"><span class="heating-fixed-size input-group-text">{__("heating.content.month")}</span></div>
                                    <InputSelect
                                        className="heating-input-group-prepend"
                                        items={Heating.months}
                                        value={this.summer_end_month}
                                    />
                                </div>
                            </div>
                            <div class="col-md-6">
                                <div class="input-group">
                                    <div class="input-group-prepend heating-input-group-append"><span class="heating-fixed-size input-group-text">{__("heating.content.day")}</span></div>
                                    <InputSelect
                                        className="heating-input-group-prepend"
                                        items={days_summer_end}
                                        value={this.summer_end_day}
                                    />
                                </div>
                            </div>
                        </div>
                    </FormRow>
                    <FormRow label={__("heating.content.block_time")}>
                        <Switch desc={__("heating.content.enable_daily_block_period")}
                                checked={state.summer_block_time_active}
                                onClick={this.toggle('summer_block_time_active')}
                        />
                    </FormRow>
                    <Collapse in={state.summer_block_time_active}>
                        <div>
                            <FormRow label={__("heating.content.morning")}>
                                <div class="row no-gutters">
                                    <div class="col-md-6">
                                        <div class="input-group">
                                            <div class="input-group-prepend heating-input-group-prepend"><span class="heating-fixed-size input-group-text">{__("heating.content.from")}</span></div>
                                            <InputTime
                                                className={"form-control-md heating-input-group-prepend"}
                                                date={new Date(0, 0, 1, 0, 0)}
                                                showSeconds={false}
                                            />
                                        </div>
                                    </div>
                                    <div class="col-md-6">
                                        <div class="input-group">
                                            <div class="input-group-prepend heating-input-group-append"><span class="heating-fixed-size input-group-text">{__("heating.content.to")}</span></div>
                                                <InputTime
                                                className={"form-control-md heating-input-group-prepend"}
                                                date={this.get_date_from_minutes(state.summer_block_time_morning)}
                                                showSeconds={false}
                                                onDate={(d: Date) => this.setState({summer_block_time_morning: this.get_minutes_from_date(d)})}
                                            />
                                        </div>
                                    </div>
                                </div>
                            </FormRow>
                            <FormRow label={__("heating.content.evening")}>
                                <div class="row no-gutters">
                                    <div class="col-md-6">
                                        <div class="input-group">
                                            <div class="input-group-prepend heating-input-group-prepend"><span class="heating-fixed-size input-group-text">{__("heating.content.from")}</span></div>
                                                <InputTime
                                                className={"form-control-md heating-input-group-prepend"}
                                                date={this.get_date_from_minutes(state.summer_block_time_evening)}
                                                showSeconds={false}
                                                onDate={(d: Date) => this.setState({summer_block_time_evening: this.get_minutes_from_date(d)})}
                                            />
                                        </div>
                                    </div>
                                    <div class="col-md-6">
                                        <div class="input-group">
                                            <div class="input-group-prepend heating-input-group-append"><span class="heating-fixed-size input-group-text">{__("heating.content.to")}</span></div>
                                            <InputTime
                                                className={"form-control-md heating-input-group-prepend"}
                                                date={new Date(0, 0, 1, 23, 59)}
                                                showSeconds={false}
                                            />
                                        </div>
                                    </div>
                                </div>
                            </FormRow>
                        </div>
                    </Collapse>
                    <FormRow label={__("heating.content.pv_yield_forecast")}>
                        <Switch desc={__("heating.content.pv_yield_forecast_activate")}
                                checked={state.summer_yield_forecast_active}
                                onClick={this.toggle('summer_yield_forecast_active')}
                        />
                    </FormRow>
                    <Collapse in={state.summer_yield_forecast_active}>
                        <div>
                            <FormRow label={__("heating.content.blocking_threshold")} label_muted={__("heating.content.blocking_threshold_description")}>
                                <InputNumber
                                    unit="kWh"
                                    value={state.summer_yield_forecast_threshold}
                                    onValue={this.set("summer_yield_forecast_threshold")}
                                    min={0}
                                    max={1000}
                                />
                            </FormRow>
                        </div>
                    </Collapse>
                    <FormRow label={__("heating.content.dynamic_price_control")}>
                        <Switch desc={__("heating.content.dynamic_price_control_activate")}
                                checked={state.summer_dynamic_price_control_active}
                                onClick={this.toggle('summer_dynamic_price_control_active')}
                        />
                    </FormRow>
                    <Collapse in={state.summer_dynamic_price_control_active}>
                        <div>
                            <FormRow label={__("heating.content.average_price_threshold")} label_muted={__("heating.content.average_price_threshold_description")}>
                                <InputNumber
                                    unit="%"
                                    value={state.summer_dynamic_price_control_threshold}
                                    onValue={this.set("summer_dynamic_price_control_threshold")}
                                    min={0}
                                    max={100}
                                />
                            </FormRow>
                        </div>
                    </Collapse>
                    <FormRow label={__("heating.content.pv_excess_control")}>
                        <Switch desc={__("heating.content.pv_excess_control_activate")}
                                checked={state.summer_pv_excess_control_active}
                                onClick={this.toggle('summer_pv_excess_control_active')}
                        />
                    </FormRow>
                    <Collapse in={state.summer_pv_excess_control_active}>
                        <div>
                            <FormRow label={__("heating.content.pv_excess_threshold")} label_muted={__("heating.content.pv_excess_threshold_description")}>
                                <InputNumber
                                    unit="Watt"
                                    value={state.summer_pv_excess_control_threshold}
                                    onValue={this.set("summer_pv_excess_control_threshold")}
                                    min={0}
                                    max={100000}
                                />
                            </FormRow>
                        </div>
                    </Collapse>

                    <FormSeparator heading="§14 EnWG"/>
                    <FormRow label="§14 EnWG">
                        <Switch desc={__("heating.content.p14_enwg_control_activate")}
                                checked={state.p14enwg_active}
                                onClick={this.toggle('p14enwg_active')}
                        />
                    </FormRow>
                    <Collapse in={state.p14enwg_active}>
                        <div>
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
                            <FormRow label={__("heating.content.throttled_if_input")}>
                                <InputSelect
                                    items={[
                                        ["0", __("heating.content.closed")],
                                        ["1", __("heating.content.opened")]
                                    ]}
                                    value={state.p14enwg_active_type}
                                    onValue={(v) => this.setState({p14enwg_active_type: parseInt(v)})}
                                />
                            </FormRow>
                        </div>
                    </Collapse>
                </ConfigForm>
            </SubPage>
        );
    }
}

export function init() {
}
