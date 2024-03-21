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
import timezones from "./timezones";
import { InputSelect } from "../../ts/components/input_select";
import { Button } from "react-bootstrap";
import { InputText } from "../../ts/components/input_text";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { Clock } from "react-feather";

export function NTPNavbar() {
    return <NavbarItem name="ntp" module="ntp" title={__("ntp.navbar.ntp")} symbol={<Clock />} />;
}

type NTPConfig = API.getType["ntp/config"];

export class NTP extends ConfigComponent<'ntp/config', {status_ref?: RefObject<NTPStatus>}> {
    constructor() {
        super('ntp/config',
              __("ntp.script.save_failed"),
              __("ntp.script.reboot_content_changed"));
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

    render(props: {}, state: Readonly<NTPConfig>) {
        if (!util.render_allowed())
            return <SubPage name="ntp" />;

        let splt = state.timezone.split("/");

        return (
            <SubPage name="ntp">
                <ConfigForm id="ntp_config_form"
                            title={__("ntp.content.ntp")}
                            isModified={this.isModified()}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("ntp.content.enable")}>
                        <Switch desc={__("ntp.content.enable_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}/>
                    </FormRow>

                    <FormRow label={__("ntp.content.use_dhcp")}>
                        <Switch desc={__("ntp.content.use_dhcp_desc")}
                                checked={state.use_dhcp}
                                onClick={this.toggle('use_dhcp')}/>
                    </FormRow>

                    <FormRow label={__("ntp.content.timezone")}>
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
                        <Button variant="primary" className="form-control" onClick={() => this.setState({timezone: Intl.DateTimeFormat().resolvedOptions().timeZone})}>{__("ntp.content.use_browser_timezone")}</Button>
                    </FormRow>

                    <FormRow label={__("ntp.content.server")}>
                        <InputText required
                                   maxLength={64}
                                   value={state.server}
                                   onValue={this.set("server")}/>
                    </FormRow>

                    <FormRow label={__("ntp.content.server2")} label_muted={__("ntp.content.server2_muted")}>
                        <InputText maxLength={64}
                                   value={state.server2}
                                   onValue={this.set("server2")}/>
                    </FormRow>
                </ConfigForm>


            </SubPage>
        );
    }
}

interface NTPStatusState {
    state: API.getType["ntp/state"];
    config: API.getType["ntp/config"];
}

export class NTPStatus extends Component<{}, NTPStatusState> {
    constructor() {
        super();

        util.addApiEventListener('ntp/state', () => {
            this.setState({state: API.get('ntp/state')})
        });

        util.addApiEventListener('ntp/config', () => {
            this.setState({config: API.get('ntp/config')})
        });
    }

    render(props: {}, state: NTPStatusState) {
        if (!util.render_allowed() || !state.config.enable)
            return <StatusSection name="ntp" />;

        return <StatusSection name="ntp">
                <FormRow label={__("ntp.status.ntp")} label_muted={util.timestamp_min_to_date(state.state.time, "")}>
                    <IndicatorGroup
                        style="width: 100%"
                        class="flex-wrap"
                        value={!state.config.enable ? 0 : (state.state.synced ? 2 : 1)}
                        items={[
                            ["primary", __("ntp.status.deactivated")],
                            ["danger", __("ntp.status.not_synced")],
                            ["success", __("ntp.status.synced")],
                        ]}/>
                </FormRow>
            </StatusSection>;
    }
}

export function init() {
}
