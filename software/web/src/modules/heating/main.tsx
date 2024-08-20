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
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { InputNumber } from "../../ts/components/input_number";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Trello } from "react-feather";
import { InputTime } from "../../ts/components/input_time";
import { Collapse } from "react-bootstrap";
import { InputSelect } from "../../ts/components/input_select";

export function HeatingNavbar() {
    return <NavbarItem name="heating" title={__("heating.navbar.heating")} symbol={<Trello />} hidden={false} />;
}

type HeatingConfig = API.getType["heating/config"];

export class Heating extends ConfigComponent<'heating/config'> {
    summer_start_day:   number;
    summer_start_month: number;
    summer_end_day:     number;
    summer_end_month:   number;

    static days: [string, string][] = [
        ["1", "1"],
        ["2", "2"],
        ["3", "3"],
        ["4", "4"],
        ["5", "5"],
        ["6", "6"],
        ["7", "7"],
        ["8", "8"],
        ["9", "9"],
        ["10", "10"],
        ["11", "11"],
        ["12", "12"],
        ["13", "13"],
        ["14", "14"],
        ["15", "15"],
        ["16", "16"],
        ["17", "17"],
        ["18", "18"],
        ["19", "19"],
        ["20", "20"],
        ["21", "21"],
        ["22", "22"],
        ["23", "23"],
        ["24", "24"],
        ["25", "25"],
        ["26", "26"],
        ["27", "27"],
        ["28", "28"],
        ["29", "29"],
        ["30", "30"],
        ["31", "31"]
    ];

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

        return (
            <SubPage name="heating">
                <ConfigForm id="heating_config_form"
                            title={__("heating.content.heating")}
                            isModified={this.isModified()}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
                    <FormRow label="Steuersignal">
                        <div>
                            Als Steuersignal für die Heizung wird "SG Ready" verwendet.
                            Der SEB hat dafür zwei potentialfreie Schaltausgänge:
                        </div>
                        <div>
                            <ul>
                                <li>Ausgang 0 wird für den blockierenden Betrieb verwendet (SG Ready Zustand 1).</li>
                                <li>Ausgang 1 wird für die Einschaltempfehlung verwendet (SG Ready Zustand 3).</li>
                                <li>Wenn beide Ausgänge nicht geschaltet sind findet der Normalbetrieb statt (SG Ready Zustand 2).</li>
                            </ul>
                        </div>
                        <div>
                            Die intelligente Winter-/Sommermodussteuerung nutzt den Ausgang 1 und
                            die Steuerung nach §14 EnWG den Ausgang 0. Es ist möglich nur einen der beiden Ausgänge anzuschließen
                            wenn nur eine der beiden Steuerungen verwendet werden soll.
                        </div>
                    </FormRow>
                    <FormRow label="Mindesthaltezeit" label_muted="Wie lange soll ein Anlaufbefehl mindestens gelten">
                        <InputNumber
                            unit="Minuten"
                            value={state.minimum_control_holding_time}
                            onValue={this.set("minimum_control_holding_time")}
                            min={0}
                            max={60}
                        />
                    </FormRow>
                    <FormSeparator heading="Wintermodus"/>

                    <FormRow label="Winter Start" label_muted="">
                        <div class="row no-gutters">
                            <div class="col-md-6">
                                <div class="input-group">
                                    <div class="input-group-prepend heating-input-group-append"><span class="heating-fixed-size input-group-text">Monat</span></div>
                                    <InputSelect
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
                                    <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">Tag</span></div>
                                    <InputSelect
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
                    <FormRow label="Winter Ende" label_muted="">
                        <div class="row no-gutters">
                            <div class="col-md-6">
                                <div class="input-group">
                                    <div class="input-group-prepend heating-input-group-append"><span class="heating-fixed-size input-group-text">Monat</span></div>
                                    <InputSelect
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
                                    <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">Tag</span></div>
                                    <InputSelect
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
                    <FormRow label="Dynamische Preissteuerung">
                        <Switch desc="Aktiviert die Optimierung der Heizungssteuerung anhand des dynamischen Strompreises"
                                checked={state.winter_dynamic_price_control_active}
                                onClick={this.toggle('winter_dynamic_price_control_active')}
                        />
                    </FormRow>
                    <Collapse in={state.winter_dynamic_price_control_active}>
                        <div>
                            <FormRow label="Durchschnitspreis-Schwelle" label_muted="Liegt der Tagesdurchschnittspreis unter der Schwelle, dann wird der Heizung ein Anlaufbefehl gegeben">
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
                    <FormRow label="PV-Überschuss-Steuerung">
                        <Switch desc="Aktiviert die Optimierung der Heizungssteuerung anhand des PV-Überschuss"
                                checked={state.winter_pv_excess_control_active}
                                onClick={this.toggle('winter_pv_excess_control_active')}
                        />
                    </FormRow>
                    <Collapse in={state.winter_pv_excess_control_active}>
                        <div>
                            <FormRow label="PV-Überschuss-Schwelle" label_muted="Ab wieviel Watt PV-Überschuss soll der Heizung ein Anlaufbefehl gegeben werden">
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

                    <FormSeparator heading="Sommermodus"/>
                    <FormRow label="Sommer Start" label_muted="">
                        <div class="row no-gutters">
                            <div class="col-md-6">
                                <div class="input-group">
                                    <div class="input-group-prepend heating-input-group-append"><span class="heating-fixed-size input-group-text">Monat</span></div>
                                    <InputSelect
                                        items={Heating.months}
                                        value={this.summer_start_month}
                                    />
                                </div>
                            </div>
                            <div class="col-md-6">
                                <div class="input-group">
                                    <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">Tag</span></div>
                                    <InputSelect
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
                                    <div class="input-group-prepend heating-input-group-append"><span class="heating-fixed-size input-group-text">Monat</span></div>
                                    <InputSelect
                                        items={Heating.months}
                                        value={this.summer_end_month}
                                    />
                                </div>
                            </div>
                            <div class="col-md-6">
                                <div class="input-group">
                                    <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">Tag</span></div>
                                    <InputSelect
                                        items={days_summer_end}
                                        value={this.summer_end_day}
                                    />
                                </div>
                            </div>
                        </div>
                    </FormRow>
                    <FormRow label="Blockierzeit">
                        <Switch desc="Aktiviert den täglichen Blockierzeitraum"
                                checked={state.summer_block_time_active}
                                onClick={this.toggle('summer_block_time_active')}
                        />
                    </FormRow>
                    <Collapse in={state.summer_block_time_active}>
                        <div>
                            <FormRow label="Morgens">
                                <div class="row no-gutters">
                                    <div class="col-md-6">
                                        <div class="input-group">
                                            <div class="input-group-prepend heating-input-group-append"><span class="heating-fixed-size input-group-text">Von</span></div>
                                            <InputTime
                                                className={"form-control-md"}
                                                date={new Date(0, 0, 1, 0, 0)}
                                                showSeconds={false}
                                            />
                                        </div>
                                    </div>
                                    <div class="col-md-6">
                                        <div class="input-group">
                                            <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">Bis</span></div>
                                                <InputTime
                                                className={"form-control-md"}
                                                date={this.get_date_from_minutes(state.summer_block_time_morning)}
                                                showSeconds={false}
                                                onDate={(d: Date) => this.setState({summer_block_time_morning: this.get_minutes_from_date(d)})}
                                            />
                                        </div>
                                    </div>
                                </div>
                            </FormRow>
                            <FormRow label="Abends">
                                <div class="row no-gutters">
                                    <div class="col-md-6">
                                        <div class="input-group">
                                            <div class="input-group-prepend heating-input-group-append"><span class="heating-fixed-size input-group-text">Von</span></div>
                                                <InputTime
                                                className={"form-control-md"}
                                                date={this.get_date_from_minutes(state.summer_block_time_evening)}
                                                showSeconds={false}
                                                onDate={(d: Date) => this.setState({summer_block_time_evening: this.get_minutes_from_date(d)})}
                                            />
                                        </div>
                                    </div>
                                    <div class="col-md-6">
                                        <div class="input-group">
                                            <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">Bis</span></div>
                                            <InputTime
                                                className={"form-control-md"}
                                                date={new Date(0, 0, 1, 23, 59)}
                                                showSeconds={false}
                                            />
                                        </div>
                                    </div>
                                </div>
                            </FormRow>
                        </div>
                    </Collapse>
                    <FormRow label="PV-Ertragsprognose">
                        <Switch desc="Blockiere nur wenn erwarteter Ertrag über konfigurierter Blockier-Schwelle"
                                checked={state.summer_yield_forecast_active}
                                onClick={this.toggle('summer_yield_forecast_active')}
                        />
                    </FormRow>
                    <Collapse in={state.summer_yield_forecast_active}>
                        <div>
                            <FormRow label="Blockier-Schwelle" label_muted="Ab wieviel kWh Ertragsprognose soll der Anlaufbefehl anhand Uhrzeit blockiert werden">
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
                    <FormRow label="Dynamische Preissteuerung">
                        <Switch desc="Aktiviert die Optimierung der Heizungssteuerung anhand des dynamischen Strompreises"
                                checked={state.summer_dynamic_price_control_active}
                                onClick={this.toggle('summer_dynamic_price_control_active')}
                        />
                    </FormRow>
                    <Collapse in={state.summer_dynamic_price_control_active}>
                        <div>
                            <FormRow label="Durchschnitspreis-Schwelle" label_muted="Liegt der Tagesdurchschnittspreis unter der Schwelle, dann wird der Heizung ein Anlaufbefehl gegeben">
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
                    <FormRow label="PV-Überschuss-Steuerung">
                        <Switch desc="Aktiviert die Optimierung der Heizungssteuerung anhand des PV-Überschuss"
                                checked={state.summer_pv_excess_control_active}
                                onClick={this.toggle('summer_pv_excess_control_active')}
                        />
                    </FormRow>
                    <Collapse in={state.summer_pv_excess_control_active}>
                        <div>
                            <FormRow label="PV-Überschuss-Schwelle" label_muted="Ab wieviel Watt PV-Überschuss soll der Heizung ein Anlaufbefehl gegeben werden">
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
                        <Switch desc="Aktiviert die Kontrolle der Heizung anhand §14 EnWG"
                                checked={state.p14enwg_active}
                                onClick={this.toggle('p14enwg_active')}
                        />
                    </FormRow>
                    <Collapse in={state.p14enwg_active}>
                        <div>
                            <FormRow label="Eingang">
                                <InputSelect
                                    items={[
                                        ["0", "Eingang 0"],
                                        ["1", "Eingang 1"],
                                        ["2", "Eingang 2"],
                                        ["3", "Eingang 3"],
                                    ]}
                                    value={state.p14enwg_input}
                                    onValue={(v) => this.setState({p14enwg_input: parseInt(v)})}
                                />
                            </FormRow>
                            <FormRow label="Gedrosselt wenn Eingang">
                                <InputSelect
                                    items={[
                                        ["0", "Geschlossen"],
                                        ["1", "Geöffnet"]
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
