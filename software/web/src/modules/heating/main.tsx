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
import { InputDate } from "../../ts/components/input_date";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Trello } from "react-feather";
import { InputTime } from "src/ts/components/input_time";
import { Collapse } from "react-bootstrap";
import { InputSelect } from "src/ts/components/input_select";

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
        ["0", "1"],
        ["1", "2"],
        ["2", "3"],
        ["3", "4"],
        ["4", "5"],
        ["5", "6"],
        ["6", "7"],
        ["7", "8"],
        ["8", "9"],
        ["9", "10"],
        ["10", "11"],
        ["11", "12"],
        ["12", "13"],
        ["13", "14"],
        ["14", "15"],
        ["15", "16"],
        ["16", "17"],
        ["17", "18"],
        ["18", "19"],
        ["19", "20"],
        ["20", "21"],
        ["21", "22"],
        ["22", "23"],
        ["23", "24"],
        ["24", "25"],
        ["25", "26"],
        ["26", "27"],
        ["27", "28"],
        ["28", "29"],
        ["29", "30"],
        ["30", "31"]
    ];

    static months: [string, string][] = [
        ["0", "Januar"],
        ["1", "Februar"],
        ["2", "März"],
        ["3", "April"],
        ["4", "Mai"],
        ["5", "Juni"],
        ["6", "Juli"],
        ["7", "August"],
        ["8", "September"],
        ["9", "Oktober"],
        ["10", "November"],
        ["11", "Dezember"]
    ];

    constructor() {
        super('heating/config',
              __("heating.script.save_failed"),
              __("heating.script.reboot_content_changed"));

        // TODO: This is just for testing, such that summer days and months will be auto-filled
        this.setState({
            winter_start_day: 0,
            winter_start_month: 10,
            winter_end_day: 14,
            winter_end_month: 2,
        });
    }

    month_to_days(month: number): [string, string][] {
        switch(month) {
            case 0: case 2: case 4: case 6: case 7: case 9: case 11: return Heating.days.slice(0, 31);
            case 3: case 5: case 8: case 10:                         return Heating.days.slice(0, 30);
            case 1:                                                  return Heating.days.slice(0, 28);
        }
        return Heating.days.slice(0, 31);
    }

    recalculateSummer(state: Readonly<HeatingConfig>) {
        this.summer_start_day = state.winter_end_day + 1;
        this.summer_start_month = state.winter_end_month;
        if(this.summer_start_day > this.month_to_days(state.winter_end_month).length -1) {
            this.summer_start_day = 0;
            this.summer_start_month = state.winter_end_month + 1 > 11 ? 0 : state.winter_end_month + 1;
        }

        this.summer_end_day = state.winter_start_day - 1;
        this.summer_end_month = state.winter_start_month;
        if(this.summer_end_day < 0) {
            this.summer_end_month = state.winter_start_month - 1 < 0 ? 11 : state.winter_start_month - 1;
            this.summer_end_day = this.month_to_days(this.summer_end_month).length - 1;
        }
    }

    render(props: {}, state: Readonly<HeatingConfig>) {
        if (!util.render_allowed())
            return <SubPage name="heating" />;

        let days_winter_start = this.month_to_days(state.winter_start_month);
        let days_winter_end = this.month_to_days(state.winter_end_month);
        let days_summer_start = this.month_to_days(0);
        let days_summer_end = this.month_to_days(0);

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
                            <FormRow label="Mindesthaltezeit" label_muted="Wie lange soll der PV-Überschuss-Anlaufbefehl mindestens gelten">
                                <InputNumber
                                    unit="Minuten"
                                    value={state.winter_pv_excess_control_holding_time}
                                    onValue={this.set("winter_pv_excess_control_holding_time")}
                                    min={0}
                                    max={60}
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
                    <FormRow label="Blockierzeit Morgens">
                        <Switch desc="Aktiviert den ersten Blockierzeit-Zeitraum"
                                checked={state.summer_block_time1_active}
                                onClick={this.toggle('summer_block_time1_active')}
                        />
                    </FormRow>
                    <Collapse in={state.summer_block_time1_active}>
                        <div>
                            <FormRow label="Start-Uhrzeit">
                                <InputTime
                                    className={"form-control-sm"}
                                    date={new Date("2011-04-20T00:00")}
                                    showSeconds={false}
                                    onDate={(d: Date) => null}
                                />
                            </FormRow>
                            <FormRow label="End-Uhrzeit">
                                <InputTime
                                    className={"form-control-sm"}
                                    date={new Date("2011-04-20T11:00")}
                                    showSeconds={false}
                                    onDate={(d: Date) => null}
                                />
                            </FormRow>
                        </div>
                    </Collapse>
                    <FormRow label="Blockierzeit Abends">
                        <Switch desc="Aktiviert den zweiten Blockierzeit-Zeitraum"
                                checked={state.summer_block_time2_active}
                                onClick={this.toggle('summer_block_time2_active')}
                        />
                    </FormRow>
                    <Collapse in={state.summer_block_time2_active}>
                        <div>
                            <FormRow label="Start-Uhrzeit">
                                <InputTime
                                    className={"form-control-sm"}
                                    date={new Date("2011-04-20T18:00")}
                                    showSeconds={false}
                                    onDate={(d: Date) => null}
                                />
                            </FormRow>
                            <FormRow label="End-Uhrzeit">
                                <InputTime
                                    className={"form-control-sm"}
                                    date={new Date("2011-04-20T23:59")}
                                    showSeconds={false}
                                    onDate={(d: Date) => null}
                                />
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
                            <FormRow label="Mindesthaltezeit" label_muted="Wie lange soll der PV-Überschuss-Anlaufbefehl mindestens gelten">
                                <InputNumber
                                    unit="Minuten"
                                    value={state.summer_pv_excess_control_holding_time}
                                    onValue={this.set("summer_pv_excess_control_holding_time")}
                                    min={0}
                                    max={60}
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
