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
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import timezones from "../ntp/timezones";
import { InputSelect } from "../../ts/components/input_select";
import { Button } from "react-bootstrap";
import { InputText } from "../../ts/components/input_text";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { Clock } from "react-feather";
import { FormSeparator } from "../../ts/components/form_separator";
import { OutputDatetime } from "../../ts/components/output_datetime";

export function TimeNavbar() {
    return <NavbarItem name="time" module="ntp" title={__("time.navbar.time")} symbol={<Clock />} />;
}

type TimeConfig = API.getType["ntp/config"] & API.getType["rtc/config"];

type RTCTime = API.getType['rtc/time'];

export class Time extends ConfigComponent<'ntp/config', {status_ref?: RefObject<TimeStatus>}, API.getType['rtc/config']> {
    first_render: boolean = true;

    constructor() {
        super('ntp/config',
              () => __("time.script.save_failed"),
              () => __("time.script.reboot_content_changed"));

        util.addApiEventListener("rtc/config", () => this.setState({...API.get("rtc/config")}));
    }

    updateTimezone(s: string, i: number) {
        let splt = this.state.timezone.split("/");
        splt[i] = s;
        if (i == 0)
            splt[1] = Object.keys(timezones[splt[0]])[0];
        else if (i == 1) {
            if (timezones[splt[0]][splt[1]] != null)
                splt[2] = Object.keys(timezones[splt[0]][splt[1]])[0];
            else
                splt = [splt[0], splt[1]];
        }

        this.setState({timezone: splt.join("/")});
    }

    set_current_time() {
        let date = new Date();
        let time: RTCTime = {
            year: date.getUTCFullYear(),
            month: date.getUTCMonth() + 1,
            day: date.getUTCDate(),
            hour: date.getUTCHours(),
            minute: date.getUTCMinutes(),
            second: date.getUTCSeconds(),
            weekday: date.getUTCDay(),
        };

        API.save("rtc/time", time, () => __("time.script.save_failed"));
    }

    override async sendSave(t: "ntp/config", cfg: API.getType["ntp/config"]) {
        super.sendSave(t, cfg);
        // API.save extracts the rtc config
        API.save("rtc/config", this.state);
    }

    override async sendReset(t: "ntp/config") {
        super.sendReset(t);
        API.reset("rtc/config");
    }

    override getIsModified(t: "ntp/config"): boolean {
        return API.is_modified(t) || API.is_modified("rtc/config");
    }

    render(props: {}, state: Readonly<TimeConfig>) {
        if (!util.render_allowed())
            return <SubPage name="time" />;

        if (this.first_render) {
            this.first_render = false;
            if (API.get("rtc/config").auto_sync && !API.get("ntp/state").synced) {
                this.set_current_time();
            }
        }

        let splt = state.timezone.split("/");

        const p = (i: number) => util.leftPad(i, 0, 2);
        let t = API.get('rtc/time');
        let date = new Date(
            `20${p(t.year)}-${p(t.month)}-${p(t.day)}T${p(t.hour)}:${p(t.minute)}:${p(t.second)}.000Z`);

        return (
            <SubPage name="time">
                <ConfigForm id="time_config_form"
                            title={__("time.content.time")}
                            isModified={this.isModified()}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>

                    <FormRow label={__("time.content.live_date")}>
                        <OutputDatetime date={date}
                                        onClick={() => this.set_current_time()}
                                        buttonText={__("time.content.set_time")}
                                        disabled={this.state.auto_sync}
                                        invalidDateText={__("time.content.time_not_set")}/>
                    </FormRow>

                    <FormRow label={__("time.content.enable_auto_sync")}>
                        <Switch desc={__("time.content.auto_sync_desc")} checked={state.auto_sync} onClick={() => {
                                this.setState({"auto_sync": !state.auto_sync})
                        }}/>
                    </FormRow>

                    <FormRow label={__("time.content.timezone")}>
                        <div class="input-group">
                            <InputSelect
                                required
                                value={splt[0]}
                                onValue={(v) => this.updateTimezone(v, 0)}
                                items={
                                    Object.keys(timezones).map(t => [t, t.replace(/_/g, " ")])
                                }
                            />
                            <InputSelect
                                required
                                value={splt[1]}
                                onValue={(v) => this.updateTimezone(v, 1)}
                                items={
                                    Object.keys(timezones[splt[0]]).map(t => [t, t.replace(/_/g, " ")])
                                }
                            />
                            {
                                timezones[splt[0]][splt[1]] == null ? "" :
                                <InputSelect
                                    required
                                    value={splt[2]}
                                    onValue={(v) => this.updateTimezone(v, 2)}
                                    items={
                                        Object.keys(timezones[splt[0]][splt[1]]).map(t => [t, t.replace(/_/g, " ")])
                                    }
                                />
                            }
                        </div>
                        <br/>
                        <Button variant="primary" className="form-control" onClick={() => this.setState({timezone: Intl.DateTimeFormat().resolvedOptions().timeZone})}>{__("time.content.use_browser_timezone")}</Button>
                    </FormRow>

                    <FormSeparator heading={__("time.content.ntp")}/>

                    <FormRow label={__("time.content.enable")}>
                        <Switch desc={__("time.content.enable_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}/>
                    </FormRow>

                    <FormRow label={__("time.content.use_dhcp")}>
                        <Switch desc={__("time.content.use_dhcp_desc")}
                                checked={state.use_dhcp}
                                onClick={this.toggle('use_dhcp')}/>
                    </FormRow>

                    <FormRow label={__("time.content.server")}>
                        <InputText required
                                   maxLength={64}
                                   value={state.server}
                                   onValue={this.set("server")}/>
                    </FormRow>

                    <FormRow label={__("time.content.server2")} label_muted={__("time.content.server2_muted")}>
                        <InputText maxLength={64}
                                   value={state.server2}
                                   onValue={this.set("server2")}/>
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

interface TimeStatusState {
    state: API.getType["ntp/state"];
    config: API.getType["ntp/config"];
}

export class TimeStatus extends Component<{}, TimeStatusState> {
    constructor() {
        super();

        util.addApiEventListener('ntp/state', () => {
            this.setState({state: API.get('ntp/state')})
        });

        util.addApiEventListener('ntp/config', () => {
            this.setState({config: API.get('ntp/config')})
        });
    }

    render(props: {}, state: TimeStatusState) {
        if (!util.render_allowed() || !state.config.enable)
            return <StatusSection name="time" />;

        return <StatusSection name="time">
                <FormRow label={__("time.status.time")} label_muted={util.timestamp_min_to_date(state.state.time, "")}>
                    <IndicatorGroup
                        style="width: 100%"
                        class="flex-wrap"
                        value={!state.config.enable ? 0 : (state.state.synced ? 2 : 1)}
                        items={[
                            ["primary", __("time.status.deactivated")],
                            ["danger", __("time.status.not_synced")],
                            ["success", __("time.status.synced")],
                        ]}/>
                </FormRow>
            </StatusSection>;
    }
}

export function init() {
}
