/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

import $ from "../../ts/jq";

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import feather from "../../ts/feather";

import { h, render, Fragment, Component } from "preact";
import { __ } from "../../ts/translation";

import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { InputText } from "../../ts/components/input_text";
import { InputDate } from "../../ts/components/input_date";
import { Button, ListGroup, ListGroupItem, Spinner } from "react-bootstrap";
import { PageHeader } from "src/ts/components/page_header";
import { InputSelect } from "src/ts/components/input_select";
import { BatteryCharging, Calendar, Clock, Download, User } from "react-feather";
import { getAllUsernames } from "../users/main";
import { ConfigComponent } from "src/ts/components/config_component";
import { ConfigForm } from "src/ts/components/config_form";
import { InputFloat } from "src/ts/components/input_float";

type Charge = API.getType['charge_tracker/last_charges'][0];
type ChargetrackerConfig = API.getType['charge_tracker/config'];

interface S {
    user_filter: string
    user_filter_items: [string, string][]
    start_date: Date
    end_date: Date
    show_spinner: boolean
    last_charges: Readonly<Charge[]>
}

type ChargeTrackerState = S & API.getType['charge_tracker/state'];

export class ChargeTracker extends ConfigComponent<'charge_tracker/config', {}, ChargeTrackerState & ChargetrackerConfig> {
    constructor() {
        super('charge_tracker/config',
                __("charge_tracker.script.save_failed"),
                __("charge_tracker.script.reboot_content_changed"));

        util.eventTarget.addEventListener('users/config', () => {
            let user_filter_items: [string, string][] = API.get('users/config').users.map(x => [x.id.toString(), (x.display_name == "Anonymous" && x.id == 0) ? __("charge_tracker.script.unknown_users") : x.display_name]);
            user_filter_items.unshift(["-1",  __("charge_tracker.script.deleted_users")]);
            user_filter_items.unshift(["-2", __("charge_tracker.script.all_users")]);
            this.setState({user_filter_items: user_filter_items});
        });

        util.eventTarget.addEventListener('charge_tracker/state', () => {
            this.setState({...API.get('charge_tracker/state')});
        });

        util.eventTarget.addEventListener('charge_tracker/last_charges', () => {
            this.setState({last_charges: API.get('charge_tracker/last_charges')});
        });

        util.eventTarget.addEventListener('charge_tracker/config', () => {
            this.setState({electricity_price: API.get('charge_tracker/config').electricity_price});
        });

        this.state = {
            user_filter: "-2"
        } as any
    }

    get_last_charges(charges: typeof this.state.last_charges, price: number) {
        let users_config = API.get('users/config');

        return charges.map(c => {
            let display_name = __("charge_tracker.script.unknown_user")

            let filtered = users_config.users.filter(x => x.id == c.user_id);

            if (c.user_id != 0 || filtered[0].display_name != "Anonymous") {
                display_name = __("charge_tracker.script.deleted_user")
                if (filtered.length == 1)
                    display_name = filtered[0].display_name
            }

            let icon = <svg class="feather feather-wallet mr-1" width="24" height="24" fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="2" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><rect x="1" y="6.0999" width="22" height="16" rx="2" ry="2"/><path d="m2.9474 6.0908 15.599-4.8048s0.59352-0.22385 0.57647 0.62527c-0.02215 1.1038-0.01535 3.6833-0.01535 3.6833"/></svg>
            let price_div = <div>{icon}<span style="vertical-align: middle;">{util.toLocaleFixed(price / 100 * c.energy_charged / 100, 2)} €</span></div>

            return <ListGroupItem>
                <div class="row">
                    <div class="col">
                        <div class="mb-2"><User/><span class="ml-1" style="vertical-align: middle;">{display_name}</span></div>
                        <div><Calendar/><span class="ml-1" style="vertical-align: middle;">{util.timestamp_min_to_date(c.timestamp_minutes, __("charge_tracker.script.unknown_charge_start"))}</span></div>
                    </div>
                    <div class="col-auto">
                        <div class="mb-2"><BatteryCharging/><span class="ml-1" style="vertical-align: middle;">{c.energy_charged === null ? "N/A" : util.toLocaleFixed(c.energy_charged, 3)} kWh</span></div>
                        <div class="mb-2"><Clock/><span class="ml-1" style="vertical-align: middle;">{util.format_timespan(c.charge_duration)}</span></div>
                        {price > 0 && c.energy_charged != null ? price_div : <></>}
                    </div>
                </div>
            </ListGroupItem>}).reverse();
    }

    render(props: {}, state: Readonly<ChargeTrackerState> & ChargetrackerConfig) {
        if (!state)
            return (<></>);

        return (
            <>
                <ConfigForm id="charge_tracker_config_form" title={__("charge_tracker.content.charge_tracker")} onSave={this.save} isModified={this.isModified()} onReset={this.reset} onDirtyChange={(d) => this.ignore_updates = d}>
                    <FormRow label={__("charge_tracker.content.price")}>
                        <InputFloat value={state.electricity_price} onValue={this.set('electricity_price')} digits={2} unit={'ct/kWh'} max={20000} min={0}/>
                    </FormRow>
                </ConfigForm>

                <FormSeparator heading={__("charge_tracker.content.download")}/>

                <FormRow label={__("charge_tracker.content.user_filter")} label_muted={__("charge_tracker.content.user_filter_muted")}>
                    <InputSelect
                        value={state.user_filter}
                        onValue={(v) => this.setState({user_filter: v})}
                        items={state.user_filter_items ?? []}
                    />
                </FormRow>

                <FormRow label={__("charge_tracker.content.date_filter")} label_muted={__("charge_tracker.content.date_filter_muted")}>
                    <div class="row no-gutters">
                        <div class="col-md-6">
                            <div class="input-group">
                                <div class="input-group-prepend"><span class="input-group-text">{__("charge_tracker.content.from")}</span></div>
                                <InputDate className="charge-tracker-input-group-prepend"
                                        date={state.start_date}
                                        onDate={(d: Date) => this.setState({start_date: d})}
                                    />
                            </div>
                        </div>
                        <div class="col-md-6">
                            <div class="input-group">
                                <div class="input-group-prepend charge-tracker-input-group-append"><span class="input-group-text">{__("charge_tracker.content.to")}</span></div>
                                <InputDate className="charge-tracker-input-group-prepend"
                                        date={state.end_date}
                                        onDate={(d: Date) => this.setState({end_date: d})}
                                    />
                            </div>
                        </div>
                    </div>
                </FormRow>

                <FormRow label="" label_muted={__("charge_tracker.content.download_desc")}>
                    <Button variant="primary" className="form-control" onClick={async () => {
                        this.setState({show_spinner: true});

                        let start = state.start_date ?? new Date(0);
                        // Start and end dates are "invalid date" if the user clicks the input's clear button.
                        if (isNaN(start.getTime()))
                            start = new Date(0);

                        let end = state.end_date ?? new Date(Date.now());
                        if (isNaN(end.getTime()))
                            end = new Date(Date.now());

                        try {
                            await downloadChargeLog(parseInt(state.user_filter), start ,end, state.electricity_price);
                        } finally {
                            this.setState({show_spinner: false});
                        }
                    }}>
                        <span class="mr-2">{__("charge_tracker.content.download_btn")}</span>
                        <Download/>
                        <Spinner animation="border" size="sm" as="span" className="ml-2" hidden={!state.show_spinner}/>
                    </Button>
                </FormRow>

                <FormSeparator heading={__("charge_tracker.content.tracked_charges")}/>

                <FormRow label={__("charge_tracker.content.tracked_charges")} label_muted={__("charge_tracker.content.tracked_charges_muted")}>
                    <InputText value={state.tracked_charges}/>
                </FormRow>

                <FormRow label={__("charge_tracker.content.first_charge_timestamp")} label_muted={__("charge_tracker.content.first_charge_timestamp_muted")}>
                    <InputText value={util.timestamp_min_to_date(state.first_charge_timestamp, __("charge_tracker.script.unknown_charge_start"))}/>
                </FormRow>

                <FormRow label={__("charge_tracker.content.remove")} label_muted={__("charge_tracker.content.remove_desc")}>
                    <Button variant="danger" className="form-control" onClick={async () => {
                        const modal = util.async_modal_ref.current;
                        if (!await modal.show({
                                title: __("charge_tracker.content.remove"),
                                body: __("charge_tracker.content.charge_tracker_remove_modal_text"),
                                no_text: __("charge_tracker.content.abort_remove"),
                                yes_text: __("charge_tracker.content.confirm_remove"),
                                no_variant: "secondary",
                                yes_variant: "danger"
                            }))
                            return;

                            await API.call('charge_tracker/remove_all_charges', {
                                    "do_i_know_what_i_am_doing": true
                                }, __("charge_tracker.script.remove_failed"));

                            util.postReboot(__("charge_tracker.script.remove_init"), __("util.reboot_text"));
                    }}>
                        {__("charge_tracker.content.remove_btn")}
                    </Button>
                </FormRow>

                <FormRow label={__("charge_tracker.content.last_charges")} label_muted={__("charge_tracker.content.last_charges_desc")}>
                    <ListGroup>
                        {this.get_last_charges(state.last_charges ?? [], state.electricity_price)}
                    </ListGroup>
                </FormRow>
            </>
        );
    }
}

render(<ChargeTracker/>, $('#charge_tracker')[0]);

let x = <svg class="feather feather-credit-card" width="24" height="24" fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="2" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><rect x="1" y="6.0999" width="22" height="16" rx="2" ry="2"/><path d="m2.9474 6.0908 15.599-4.8048s0.59352-0.22385 0.57647 0.62527c-0.02215 1.1038-0.01535 3.6833-0.01535 3.6833"/></svg>

function show_charge_cost(charged: number)
{
    let price = API.get("charge_tracker/config").electricity_price;
    if (price > 0 && charged != null)
    {
        let icon = '<svg class="feather feather-wallet mr-1" width="24" height="24" fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="2" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><rect x="1" y="6.0999" width="22" height="16" rx="2" ry="2"/><path d="m2.9474 6.0908 15.599-4.8048s0.59352-0.22385 0.57647 0.62527c-0.02215 1.1038-0.01535 3.6833-0.01535 3.6833"/></svg>'

        return `<div>${icon}<span style="vertical-align: middle;">${util.toLocaleFixed(price / 100 * charged / 100, 2)} €</span></div>`;
    }
    return "";


}

function update_last_charges() {
    let charges = API.get('charge_tracker/last_charges');
    let users_config = API.get('users/config')

    let last_charges_html = charges.slice(-3).map((user) => {
        let display_name = __("charge_tracker.script.unknown_user")

        let filtered = users_config.users.filter(x => x.id == user.user_id);

        if (user.user_id != 0 || filtered[0].display_name != "Anonymous") {
            display_name = __("charge_tracker.script.deleted_user")
            if (filtered.length == 1)
                display_name = filtered[0].display_name
        }

        return `<div class="list-group-item">
        <div class="row">
            <div class="col">
                <div class="mb-2"><span class="mr-1" data-feather="user"></span><span style="vertical-align: middle;">${display_name}</span></div>
                <div><span class="mr-1" data-feather="calendar"></span><span style="vertical-align: middle;">${util.timestamp_min_to_date(user.timestamp_minutes, __("charge_tracker.script.unknown_charge_start"))}</span></div>
            </div>
            <div class="col-auto">
                <div class="mb-2"><span class="mr-1" data-feather="battery-charging"></span><span style="vertical-align: middle;">${user.energy_charged === null ? "N/A" : util.toLocaleFixed(user.energy_charged, 3)} kWh</span></div>
                <div class="mb-2"><span class="mr-1" data-feather="clock"></span><span style="vertical-align: middle;">${util.format_timespan(user.charge_duration)}</span></div>
                ${show_charge_cost(user.energy_charged)}
            </div>
        </div>
        </div>`
    }).reverse();

    $('#charge_tracker_status_last_charges').html(last_charges_html.join(""));
    feather.replace();
}

function to_csv_line(vals: string[]) {
    let line = vals.map(entry => '"' + entry.replace(/\"/, '""') + '"');

    return line.join(",") + "\r\n";
}

async function downloadChargeLog(user_filter: number, start_date: Date, end_date: Date, price?: number) {
    const [usernames, display_names] = await getAllUsernames()
        .catch(err => {
            util.add_alert("download-usernames", "danger", __("charge_tracker.script.download_usernames_failed"), err);
            return [null, null];
        });

    if (usernames == null || display_names == null)
        return;

    await util.download('/charge_tracker/charge_log')
        .then(blob => blob.arrayBuffer())
        .then(buffer => {
            let line = [
                __("charge_tracker.script.csv_header_start"),
                __("charge_tracker.script.csv_header_display_name"),
                __("charge_tracker.script.csv_header_energy"),
                __("charge_tracker.script.csv_header_duration"),
                "",
                __("charge_tracker.script.csv_header_meter_start"),
                __("charge_tracker.script.csv_header_meter_end"),
                __("charge_tracker.script.csv_header_username"),
                typeof price == 'number' && price > 0 ? __("charge_tracker.script.csv_header_price") + util.toLocaleFixed(price / 100, 2) + "ct/kWh" : "",
            ];

            let header = to_csv_line(line);
            let result = header;
            let users_config = API.get('users/config');

            let start = start_date.getTime() / 1000 / 60;

            end_date.setHours(23, 59, 59, 999);
            let end = end_date.getTime() / 1000 / 60;

            let known_users = API.get('users/config').users.filter(u => u.id != 0).map(u => u.id);

            let user_filtered = (x: number) => {
                switch(user_filter) {
                    case -2:
                        return false;
                    case -1:
                        return known_users.indexOf(x) < 0;
                    default:
                        return x != user_filter;
                }
            }

            if (start <= end) {
                for(let i = 0; i < buffer.byteLength; i += 16) {
                    let view = new DataView(buffer, i, 16);

                    let timestamp_minutes = view.getUint32(0, true);
                    let meter_start = view.getFloat32(4, true);
                    let user_id = view.getUint8(8);
                    let charge_duration = view.getUint32(9, true) & 0x00FFFFFF;
                    let meter_end = view.getFloat32(12, true);

                    if (timestamp_minutes != 0 && timestamp_minutes < start) {
                        result = header;
                        continue;
                    }

                    if (timestamp_minutes != 0 && timestamp_minutes > end)
                        break;

                    if (user_filtered(user_id))
                        continue;

                    let filtered = users_config.users.filter(x => x.id == user_id);

                    let display_name = "";
                    let username = ""
                    if (user_id == 0) {
                        if (filtered[0].display_name == "Anonymous")
                            display_name = __("charge_tracker.script.unknown_user");
                        else
                            display_name = filtered[0].display_name;
                        username = __("charge_tracker.script.unknown_user");
                    }
                    else if (filtered.length == 1) {
                        display_name = filtered[0].display_name
                        username = filtered[0].username
                    }
                    else {
                        display_name = display_names[user_id];
                        username = usernames[user_id];
                    }

                    let charged = (Number.isNaN(meter_start) || Number.isNaN(meter_end)) ? NaN : (meter_end - meter_start);
                    let charged_string;
                    let charged_price = typeof price == 'number' ? charged / 100 * price / 100 : 0;
                    let charged_price_string;
                    if (Number.isNaN(charged) || charged < 0) {
                        charged_string = 'N/A';
                        charged_price_string = 'N/A';
                    } else {
                        charged_string = util.toLocaleFixed(charged, 3);
                        charged_price_string = util.toLocaleFixed(charged_price, 2);
                    }

                    let line = [
                        util.timestamp_min_to_date(timestamp_minutes, __("charge_tracker.script.unknown_charge_start")),
                        display_name,
                        charged_string,
                        charge_duration.toString(),
                        "",
                        Number.isNaN(meter_start) ? 'N/A' : util.toLocaleFixed(meter_start, 3),
                        Number.isNaN(meter_end) ? 'N/A' : util.toLocaleFixed(meter_end, 3),
                        username,
                        price > 0 ? charged_price_string : ""
                    ];

                    result += to_csv_line(line);
                }
            }

            util.downloadToFile(result, "charge-log", "csv", "text/csv; charset=utf-8; header=present");
        })
        .catch(err => util.add_alert("download-charge-log", "alert-danger", __("charge_tracker.script.download_charge_log_failed"), err));
}

function update_current_charge() {
    let cc = API.get('charge_tracker/current_charge');
    let evse_ll = API.get('evse/low_level_state');
    let mv = API.get('meter/values');
    let uc = API.get('users/config');

    $('#charge_tracker_current_charge').prop("hidden", cc.user_id == -1);

    if (cc.user_id == -1) {
        return;
    }

    let filtered = uc.users.filter((x) => x.id == cc.user_id);
    let user_display_name = __("charge_tracker.script.unknown_user");
    if (filtered.length > 0 && (cc.user_id != 0 || filtered[0].display_name != "Anonymous"))
        user_display_name = filtered[0].display_name;

    let energy_charged = mv.energy_abs - cc.meter_start;
    let time_charging = evse_ll.uptime - cc.evse_uptime_start
    if (evse_ll.uptime < cc.evse_uptime_start)
        time_charging += 0xFFFFFFFF;

    time_charging = Math.floor(time_charging / 1000);

    let price = API.get("charge_tracker/config").electricity_price;

    if (filtered.length == 0)
        $('#users_status_charging_user').html(__("charge_tracker.script.deleted_user"));
    else if (filtered[0].display_name == "Anonymous" && cc.user_id == 0)
        $('#users_status_charging_user').html(__("charge_tracker.script.unknown_user"));
    else
        $('#users_status_charging_user').html(user_display_name);
    $('#users_status_charging_time').html(util.format_timespan(time_charging));
    $('#users_status_charged_energy').html(cc.meter_start == null ? "N/A kWh" : util.toLocaleFixed(energy_charged, 3) + " kWh");
    $('#users_status_charging_start').html(util.timestamp_min_to_date(cc.timestamp_minutes, __("charge_tracker.script.unknown_charge_start")));
    if (price > 0 && cc.meter_start != null)
    {
        $('#current_charge_price').text(util.toLocaleFixed(price / 100 * energy_charged / 100, 2) + " €");
        $('#current_charge_price_div').prop('hidden', false);
    }
    else
        $('#current_charge_price_div').prop('hidden', true);
}

export function init() {

}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('charge_tracker/last_charges', update_last_charges);
    source.addEventListener('charge_tracker/current_charge', update_current_charge);
    source.addEventListener('evse/low_level_state', update_current_charge);
    source.addEventListener('meter/values', update_current_charge);
    source.addEventListener('users/config', update_current_charge);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-charge_tracker').prop('hidden', !module_init.charge_tracker);
}
