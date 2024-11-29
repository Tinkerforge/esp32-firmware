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

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { h, Fragment, Component, RefObject } from "preact";
import { __ } from "../../ts/translation";
import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { InputText } from "../../ts/components/input_text";
import { InputDate } from "../../ts/components/input_date";
import { Button, Collapse, ListGroup, ListGroupItem, Spinner } from "react-bootstrap";
import { InputSelect } from "../../ts/components/input_select";
import { getAllUsernames } from "../users/main";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { InputFloat } from "../../ts/components/input_float";
import { SubPage } from "../../ts/components/sub_page";
import { useMemo } from "preact/hooks";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { BatteryCharging, Calendar, Clock, Download, User, List } from "react-feather";

export function ChargeTrackerNavbar() {
    return <NavbarItem name="charge_tracker" module="charge_tracker" title={__("charge_tracker.navbar.charge_tracker")} symbol={<List />} />;
}

const MAX_TRACKED_CHARGES = 7680;

type Charge = API.getType["charge_tracker/last_charges"][0];
type ChargeTrackerConfig = API.getType["charge_tracker/config"];

interface S {
    user_filter: string;
    user_filter_items: [string, string][];
    start_date: Date;
    end_date: Date;
    file_type: string;
    pdf_text: string;
    csv_flavor: "excel" | "rfc4180";
    show_spinner: boolean;
    last_charges: Readonly<Charge[]>;
}

type ChargeTrackerState = S & API.getType['charge_tracker/state'];

let wallet_icon = <svg width="24" height="24" fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="2" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><rect x="1" y="6.0999" width="22" height="16" rx="2" ry="2"/><path d="m2.9474 6.0908 15.599-4.8048s0.59352-0.22385 0.57647 0.62527c-0.02215 1.1038-0.01535 3.6833-0.01535 3.6833"/></svg>

function TrackedCharge(props: {charge: Charge, users: API.getType['users/config']['users'], electricity_price: number}) {
    const display_name = useMemo(
        () => {
            let result = __("charge_tracker.script.unknown_user");

            let filtered = props.users.filter(x => x.id == props.charge.user_id);

            if (props.charge.user_id != 0 || filtered[0].display_name != "Anonymous") {
                result = __("charge_tracker.script.deleted_user")
                if (filtered.length == 1)
                    result = filtered[0].display_name
            }
            return result;
        },
        [props.users, props.charge.user_id]
    );

    let have_charge_cost = props.electricity_price > 0 && props.charge.energy_charged != null;
    let price_div = have_charge_cost ? <div>{wallet_icon}<span class="ml-1" style="vertical-align: middle;">{util.toLocaleFixed(props.electricity_price / 100 * props.charge.energy_charged / 100, 2)} €</span></div> : <></>

    return <ListGroupItem>
        <div class="row">
            <div class="col">
                <div class="mb-2"><User/><span class="ml-1" style="vertical-align: middle;">{display_name}</span></div>
                <div><Calendar/><span class="ml-1" style="vertical-align: middle;">{util.timestamp_min_to_date(props.charge.timestamp_minutes, __("charge_tracker.script.unknown_charge_start"))}</span></div>
            </div>
            <div class="col-auto">
                <div class="mb-2"><BatteryCharging/><span class="ml-1" style="vertical-align: middle;">{props.charge.energy_charged === null ? "N/A" : util.toLocaleFixed(props.charge.energy_charged, 3)} kWh</span></div>
                <div class={have_charge_cost ? "mb-2" : ""}><Clock/><span class="ml-1" style="vertical-align: middle;">{util.format_timespan(props.charge.charge_duration)}</span></div>
                {price_div}
            </div>
        </div>
    </ListGroupItem>
}

export class ChargeTracker extends ConfigComponent<'charge_tracker/config', {status_ref?: RefObject<ChargeTrackerStatus>}, ChargeTrackerState> {
    constructor() {
        super('charge_tracker/config',
              () => __("charge_tracker.script.save_failed"),
              () => __("charge_tracker.script.reboot_content_changed"), {
                  user_filter: "-2",
                  file_type: "0",
                  csv_flavor: 'excel',
                  start_date: new Date(NaN),
                  end_date: new Date(NaN),
              });

        util.addApiEventListener('users/config', () => {
            let user_filter_items: [string, string][] = API.get('users/config').users.map(x => [x.id.toString(), (x.display_name == "Anonymous" && x.id == 0) ? __("charge_tracker.script.unknown_users") : x.display_name]);
            user_filter_items.unshift(["-1",  __("charge_tracker.script.deleted_users")]);
            user_filter_items.unshift(["-2", __("charge_tracker.script.all_users")]);
            this.setState({user_filter_items: user_filter_items});
        });

        util.addApiEventListener('charge_tracker/state', () => {
            this.setState({...API.get('charge_tracker/state')});
        });

        util.addApiEventListener('charge_tracker/last_charges', () => {
            this.setState({last_charges: API.get('charge_tracker/last_charges')});
        });

        util.addApiEventListener('charge_tracker/config', () => {
            let conf = API.get('charge_tracker/config');
            this.setState({electricity_price: conf.electricity_price});
        });
    }

    get_last_charges(charges: Readonly<Charge[]>, price: number) {
        let users_config = API.get('users/config');

        return charges.map(c => <TrackedCharge charge={c} users={users_config.users} electricity_price={price}/>)
                      .reverse();
    }

    to_csv_line(vals: string[], flavor: 'excel' | 'rfc4180') {
        let line = vals.map(entry => '"' + entry.replace(/\"/, '""') + '"');

        if (flavor == 'excel')
            return line.join(";") + "\r\n";

        return line.join(",") + "\n";
    }

    async downloadChargeLog(flavor: 'excel' | 'rfc4180', user_filter: number, start_date: Date, end_date: Date, price?: number) {
        const [usernames, display_names] = await getAllUsernames()
            .catch(err => {
                util.add_alert("download-usernames", "danger", () => __("charge_tracker.script.download_usernames_failed"), err);
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

                let result = "";
                if (flavor == 'excel')
                    result += "sep=;\r\n";

                let header = this.to_csv_line(line, flavor);
                result += header;
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
                };

                if (start <= end) {
                    for(let i = 0; i < buffer.byteLength; i += 16) {
                        let view = new DataView(buffer, i, 16);

                        let timestamp_minutes = view.getUint32(0, true);
                        let meter_start = view.getFloat32(4, true);
                        let user_id = view.getUint8(8);
                        let charge_duration = view.getUint32(9, true) & 0x00FFFFFF;
                        let meter_end = view.getFloat32(12, true);

                        if (timestamp_minutes != 0 && timestamp_minutes < start) {
                            // We know when this charge started and it was before the requested start date.
                            // This means that all charges before and including this one can't be relevant.
                            result = header;
                            continue;
                        }

                        if (timestamp_minutes != 0 && timestamp_minutes > end)
                            // This charge started after the requested end date. We are done searching.
                            break;

                        if (user_filtered(user_id))
                            continue;

                        let filtered = users_config.users.filter(x => x.id == user_id);

                        let display_name = "";
                        let username = "";
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

                        let charged = (Number.isNaN(meter_start) || Number.isNaN(meter_end) || meter_end < meter_start) ? NaN : (meter_end - meter_start);
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

                        result += this.to_csv_line(line, flavor);
                    }
                }

                if (flavor == 'excel')
                    util.downloadToFile(util.win1252Encode(result), __("charge_tracker.content.charge_log_file"), "csv", "text/csv; charset=windows-1252; header=present");
                else
                    util.downloadToFile(result, __("charge_tracker.content.charge_log_file"), "csv", "text/csv; charset=utf-8; header=present");
            })
            .catch(err => util.add_alert("download-charge-log", "danger", () => __("charge_tracker.script.download_charge_log_failed"), err));
    }

    override async isSaveAllowed(cfg: ChargeTrackerConfig) {
        return cfg.electricity_price == 0 || cfg.electricity_price >= 100
    }

    render(props: {}, state: Readonly<ChargeTrackerState> & ChargeTrackerConfig) {
        if (!util.render_allowed())
            return <SubPage name="charge_tracker" />;

        return (
            <SubPage name="charge_tracker">
                <ConfigForm id="charge_tracker_config_form" title={__("charge_tracker.content.charge_tracker")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("charge_tracker.content.price")}>
                        <InputFloat class={state.electricity_price == 0 || state.electricity_price >= 100 ? "" : "is-invalid"} value={state.electricity_price} onValue={this.set('electricity_price')} digits={2} unit={'ct/kWh'} max={65535} min={0}/>
                        <div class="invalid-feedback">{__("charge_tracker.content.price_invalid")}</div>
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

                <FormRow label={__("charge_tracker.content.file_type")} label_muted={__("charge_tracker.content.file_type_muted")}>
                    <InputSelect
                        value={state.file_type}
                        onValue={this.set("file_type")}
                        items={[
                            ["0", __("charge_tracker.content.file_type_pdf")],
                            ["1", __("charge_tracker.content.file_type_csv")]
                        ]}
                    />
                </FormRow>

                <Collapse in={state.file_type == "0"}>
                    <div>
                        <FormRow label={__("charge_tracker.content.pdf_text")} label_muted={__("charge_tracker.content.pdf_text_muted")}>
                            <textarea name="test" class="text-monospace form-control" id="test" value={state.pdf_text} onInput={(e) => {
                                let value = (e.target as HTMLInputElement).value;
                                if (new Blob([value]).size < 500)
                                    this.setState({pdf_text: value});
                                else
                                    this.setState({pdf_text: state.pdf_text});
                            }} cols={30} rows={6}/>
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

                                end.setHours(23, 59);

                                try {
                                    let pdf = await API.call("charge_tracker/pdf", {
                                        api_not_final_acked: true,
                                        english: navigator.languages.indexOf("de") < 0 || (navigator.languages.indexOf("de") > navigator.languages.indexOf("en")),
                                        start_timestamp_min: start.getTime() / 1000 / 60,
                                        end_timestamp_min: end.getTime() / 1000 / 60,
                                        user_filter: parseInt(state.user_filter),
                                        letterhead: state.pdf_text
                                    }, () => __("charge_tracker.script.download_charge_log_failed"), undefined, 2 * 60 * 1000);
                                    util.downloadToFile(pdf, __("charge_tracker.content.charge_log_file"), "pdf", "application/pdf");
                                } finally {
                                    this.setState({show_spinner: false});
                                }
                            }}>
                                <span class="mr-2">{__("charge_tracker.content.download_btn_pdf")}</span>
                                <Download/>
                                <Spinner animation="border" size="sm" as="span" className="ml-2" hidden={!state.show_spinner}/>
                            </Button>
                        </FormRow>
                    </div>
                </Collapse>

                <Collapse in={state.file_type == "1"}>
                    <div>
                        <FormRow label={__("charge_tracker.content.csv_flavor")} label_muted={__("charge_tracker.content.csv_flavor_muted")}>
                            <InputSelect
                                value={state.csv_flavor}
                                onValue={(v) => this.setState({csv_flavor: v as any})}
                                items={[
                                    ["excel", __("charge_tracker.content.csv_flavor_excel")],
                                    ["rfc4180", __("charge_tracker.content.csv_flavor_rfc4180")]
                                ]}
                            />
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
                                    await this.downloadChargeLog(state.csv_flavor, parseInt(state.user_filter), start ,end, state.electricity_price);
                                } finally {
                                    this.setState({show_spinner: false});
                                }
                            }}>
                                <span class="mr-2">{__("charge_tracker.content.download_btn")}</span>
                                <Download/>
                                <Spinner animation="border" size="sm" as="span" className="ml-2" hidden={!state.show_spinner}/>
                            </Button>
                        </FormRow>
                    </div>
                </Collapse>

                <FormSeparator heading={__("charge_tracker.content.tracked_charges")}/>

                <FormRow label={__("charge_tracker.content.tracked_charges")} label_muted={__("charge_tracker.content.tracked_charges_muted")}>
                    <InputText value={__("charge_tracker.script.tracked_charge_count")(state.tracked_charges, MAX_TRACKED_CHARGES)}/>
                </FormRow>

                <FormRow label={__("charge_tracker.content.first_charge_timestamp")} label_muted={__("charge_tracker.content.first_charge_timestamp_muted")}>
                    <InputText value={util.timestamp_min_to_date(state.first_charge_timestamp, __("charge_tracker.script.unknown_charge_start"))}/>
                </FormRow>

                <FormRow label={__("charge_tracker.content.remove")} label_muted={__("charge_tracker.content.remove_desc")}>
                    <Button variant="danger" className="form-control" onClick={async () => {
                        const modal = util.async_modal_ref.current;
                        if (!await modal.show({
                                title: () => __("charge_tracker.content.remove"),
                                body: () => __("charge_tracker.content.charge_tracker_remove_modal_text"),
                                no_text: () => __("charge_tracker.content.abort_remove"),
                                yes_text: () => __("charge_tracker.content.confirm_remove"),
                                no_variant: "secondary",
                                yes_variant: "danger"
                            }))
                            return;

                            await API.call('charge_tracker/remove_all_charges', {
                                    "do_i_know_what_i_am_doing": true
                                }, () => __("charge_tracker.script.remove_failed"));

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
            </SubPage>
        );
    }
}

export class ChargeTrackerStatus extends Component {
    render() {
        if (!util.render_allowed())
            return <StatusSection name="charge_tracker" />;

        let last_charges = API.get('charge_tracker/last_charges');
        let cc = API.get('charge_tracker/current_charge');
        let evse_uptime = API.get('evse/low_level_state').uptime;
        let energy_abs = API.get('meter/values').energy_abs;
        let users = API.get('users/config').users;
        let electricity_price = API.get('charge_tracker/config').electricity_price;

        let current_charge = <></>;

        if (cc.user_id != -1) {
            let charge_duration = evse_uptime - cc.evse_uptime_start
            if (evse_uptime < cc.evse_uptime_start)
                charge_duration += 0xFFFFFFFF;

            charge_duration = Math.floor(charge_duration / 1000);

            let charge: Charge = {
                charge_duration: charge_duration,
                energy_charged: (energy_abs === null || cc.meter_start === null) ? null : (energy_abs - cc.meter_start),
                timestamp_minutes: cc.timestamp_minutes,
                user_id: cc.user_id
            };

            current_charge = <FormRow label={__("charge_tracker.status.current_charge")}>
                <ListGroup>
                    <TrackedCharge charge={charge}
                                    users={users}
                                    electricity_price={electricity_price}
                                    />
                </ListGroup>
            </FormRow>;
        }

        let last_charges_list = last_charges.length == 0 ? <></>
            : <FormRow label={__("charge_tracker.status.last_charges")}>
                <ListGroup>
                    {last_charges.slice(-3).map(c =>
                        <TrackedCharge charge={c}
                                        users={users}
                                        electricity_price={electricity_price}
                                        />
                    ).reverse()}
                </ListGroup>
            </FormRow>;

        return <StatusSection name="charge_tracker">
                {current_charge}
                {last_charges_list}
            </StatusSection>;
    }
}

export function init() {
}
