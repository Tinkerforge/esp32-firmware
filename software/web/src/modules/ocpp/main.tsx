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

import * as API from "../../ts/api";
import * as util from "../../ts/util";

import { h, render, Component, Fragment, VNode} from "preact";
import { __ } from "../../ts/translation";

import { Switch } from "../../ts/components/switch";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { ConfigForm } from "../../ts/components/config_form";
import { ConfigComponent } from "../../ts/components/config_component";
import { Button } from "react-bootstrap";

type OcppConfig = API.getType['ocpp/config']

export class Ocpp extends ConfigComponent<'ocpp/config'> {
    constructor() {
        super('ocpp/config',
              __("ocpp.script.save_failed"),
              __("ocpp.script.reboot_content_changed"));
    }

    override async sendSave(t: "ocpp/config", cfg: OcppConfig) {
        await API.save_maybe('evse/ocpp_enabled', {enabled: this.state.enable}, __("evse.script.save_failed"));
        await super.sendSave(t, cfg);
    }

    render(props: {}, state: Readonly<OcppConfig>) {
        if (!state)
            return (<></>);

        return (
            <>
                <ConfigForm id="ocpp_config_form" title={__("ocpp.content.ocpp")} onSave={this.save} onDirtyChange={(d) => this.ignore_updates = d}>
                    <FormRow label={__("ocpp.content.enable_ocpp")}>
                        <Switch desc={__("ocpp.content.enable_ocpp_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}/>
                    </FormRow>
                    <FormRow label={__("ocpp.content.endpoint_url")}>
                        <InputText required
                                   maxLength={128}
                                   value={state.url}
                                   onValue={this.set("url")}/>
                    </FormRow>
                    <FormRow label={__("ocpp.content.reset")} label_muted={__("ocpp.content.reset_muted")}>
                        <Button variant="danger" className="form-control" onClick={async () =>{
                            const modal = util.async_modal_ref.current;
                            if (!await modal.show({
                                    title: __("ocpp.content.reset_title"),
                                    body: __("ocpp.content.reset_title_text"),
                                    no_text: __("ocpp.content.abort_reset"),
                                    yes_text: __("ocpp.content.confirm_reset"),
                                    no_variant: "secondary",
                                    yes_variant: "danger"
                                }))
                                return;

                            API.call("ocpp/reset", null, __("ocpp.content.reset_failed"), __("ocpp.script.reboot_content_changed"));
                            }}>
                            {__("ocpp.content.reset")}
                        </Button>
                    </FormRow>
                </ConfigForm>
            </>
        );
    }
}

render(<Ocpp/>, $('#ocpp')[0])

export function add_event_listeners(source: API.APIEventTarget) {}

export function init() {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-ocpp').prop('hidden', !module_init.ocpp);
}
