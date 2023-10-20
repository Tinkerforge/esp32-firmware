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
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { SubPage } from "../../ts/components/sub_page";
import { EVSE_SLOT_MODBUS_TCP } from "../evse_common/api";
import { CollapsedSection } from "../../ts/components/collapsed_section";

type ModbusTCPConfig = API.getType["modbus_tcp/config"];

interface config {
    evse_enable: boolean
}

const input_count = 23;
const holding_count = 7 + input_count;
const discrete_count = 12 + holding_count;
const coil_count = 2 + discrete_count;

export class ModbusTCP extends ConfigComponent<'modbus_tcp/config', {}, config> {
    constructor() {
        super('modbus_tcp/config',
                __("modbus_tcp.script.save_failed"),
                __("modbus_tcp.script.reboot_content_changed"));

        util.addApiEventListener('evse/slots', () => {
            this.setState({evse_enable: API.get('evse/slots')[EVSE_SLOT_MODBUS_TCP].active});
        });
    }

    override async sendSave(t: "modbus_tcp/config", cfg: config & ModbusTCPConfig) {
        await API.save_unchecked('evse/modbus_tcp_enabled', {enabled: this.state.evse_enable}, __("evse.script.save_failed"));
        await super.sendSave(t, cfg);
    }

    override async sendReset(t: "modbus_tcp/config"){
        await API.save_unchecked('evse/modbus_tcp_enabled', {enabled: false}, __("evse.script.save_failed"));
        await super.sendReset(t);
    }

    override getIsModified(t: "modbus_tcp/config"): boolean {
        let evse = API.get_unchecked("evse/modbus_tcp_enabled");
        if (evse != null && evse.enabled)
            return true;
        return super.getIsModified(t);
    }

    render(props: {}, state: ModbusTCPConfig & config) {
        if (!util.render_allowed())
            return (<></>);

        let docu = <CollapsedSection label={__("modbus_tcp.content.table_docu")}>
            <table class="table table-bordered table-sm">
                {__("modbus_tcp.content.table_content")}
            </table>
        </CollapsedSection>

        return (
            <SubPage>
                <ConfigForm id="modbus_tcp_config_form" title={__("modbus_tcp.content.modbus_tcp")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("modbus_tcp.content.enable")}>
                        <InputSelect items={[
                            ["0", __("modbus_tcp.content.disabled")],
                            ["1", __("modbus_tcp.content.read_only")],
                            ["2", __("modbus_tcp.content.full_access")],
                        ]}
                        value={this.state.enable && this.state.evse_enable ? "2" : this.state.enable ? "1" : "0"}
                        onValue={(v) => {
                            this.setState({enable: Number(v) != 0, evse_enable: v == "2"});
                        }}/>
                    </FormRow>
                    <FormRow label={__("modbus_tcp.content.port")} label_muted={__("modbus_tcp.content.port_muted")}>
                        <InputNumber value={state.port}
                                onValue={this.set("port")}
                                min={1}
                                max={65535}/>
                    </FormRow>
                    <FormRow label={__("modbus_tcp.content.table")}>
                        <InputSelect items={[
                            ["0", __("modbus_tcp.content.tf")],
                            ["1", __("modbus_tcp.content.bender_emulate")],
                            ["2", __("modbus_tcp.content.keba_emulate")],
                        ]}
                        value={this.state.table}
                        onValue={(v) => {
                            this.setState({table: Number(v)});
                        }}/>
                    </FormRow>
                </ConfigForm>
                {
                    this.state.table == 0 ? docu : <></>
                }
            </SubPage>
        );
    }
}

render(<ModbusTCP />, $("#modbus_tcp")[0]);

export function add_event_listeners(source: API.APIEventTarget) {
}

export function init() {
}

export function update_sidebar_state(module_init: any) {
    $("#sidebar-modbus_tcp").prop("hidden", !module_init.modbus_tcp);
}
