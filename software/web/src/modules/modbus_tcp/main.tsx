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

import { h, render, Fragment } from "preact";
import { __ } from "../../ts/translation";

import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { InputPassword } from "../../ts/components/input_password";
import { Switch } from "../../ts/components/switch";
import { InputSelect } from "src/ts/components/input_select";
import { Form } from "react-bootstrap";
import { EVSE_SLOT_MODBUS_TCP } from "../evse_common/api";

type ModbusTCPConfig = API.getType['modbus_tcp/config'];

interface config {
    evse_enable: boolean
}

export class ModbusTCP extends ConfigComponent<'modbus_tcp/config', {}, config> {
    constructor() {
        super('modbus_tcp/config',
                __("modbus_tcp.script.save_failed"),
                __("modbus_tcp.script.reboot_content_changed"));

                util.eventTarget.addEventListener('evse/slots', () => {
                    this.setState({evse_enable: API.get('evse/slots')[EVSE_SLOT_MODBUS_TCP].active});
                });

    }

    override async sendSave(t: "modbus_tcp/config", cfg: config & ModbusTCPConfig) {
        await API.save_maybe('evse/modbus_tcp_enabled', {enabled: this.state.evse_enable}, __("evse.script.save_failed"));
        await super.sendSave(t, cfg);
    }

    render(props: {}, state: ModbusTCPConfig & config) {
        if (!state)
            return (<></>);

        return (
            <>
                <ConfigForm id="modbus_tcp_config_form" title={__("modbus_tcp.content.modbus_tcp")} onSave={() => this.save()} onDirtyChange={(d) => this.ignore_updates = d}>
                        <FormRow label={__("modbus_tcp.content.enable")}>
                            <InputSelect items={[
                                ["0", __("modbus_tcp.content.disabled")],
                                ["1", __("modbus_tcp.content.read_only")],
                                ["2", __("modbus_tcp.content.full_access")],
                            ]}
                            value={this.state.enable && this.state.evse_enable ? "2" : this.state.enable ? "1" : "0"}
                            onValue={(v) => {
                                this.setState({enable: v != "0", evse_enable: v == "2"});
                            }}></InputSelect>
                        </FormRow>
                        <FormRow label={__("modbus_tcp.content.port")} label_muted={__("modbus_tcp.content.port_muted")}>
                            <InputNumber value={state.port}
                                         onValue={this.set("port")}
                                         min={1}
                                         max={65536}/>
                        </FormRow>
                </ConfigForm>
            </>
        );
    }
}

render(<ModbusTCP/>, $('#modbus_tcp')[0])

export function add_event_listeners(source: API.APIEventTarget) {
}

export function init() {
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-modbus_tcp').prop('hidden', !module_init.modbus_tcp);
}
