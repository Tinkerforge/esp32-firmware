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

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { h,  Fragment } from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { SubPage } from "../../ts/components/sub_page";
import { EVSE_SLOT_MODBUS_TCP } from "../evse_common/api";
import { FormSeparator } from "../../ts/components/form_separator";
import { NavbarItem } from "../../ts/components/navbar_item";

export function ModbusTcpNavbar() {
    return (
        <NavbarItem name="modbus_tcp" module="modbus_tcp" title={__("modbus_tcp.navbar.modbus_tcp")} symbol={
            <svg fill="currentColor" viewBox="0 0 24 24" width="24" height="24" xmlns="http://www.w3.org/2000/svg"><g><g stroke="currentColor"><path d="M19.587 12a7.442 7.442 0 1 1-7.442-7.441A7.441 7.441 0 0 1 19.587 12" fill="#fff" stroke-width="1.674"/><path d="M22.192 16.242a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M7.68 16.242a2.79 2.79 0 1 1-2.791-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M7.568 7.814a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M22.192 7.814a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M14.936 20.373a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path d="M14.936 3.628a2.79 2.79 0 1 1-2.79-2.79 2.79 2.79 0 0 1 2.79 2.79" fill="#fff" stroke-width="1.674"/><path class="cls-3" d="M5.533 8.44 11.698 12c0-.595.893-.595.894 0l6.165-3.56-.224-.388-6.165 3.56c-.024.032 0-7.119 0-7.119-.094-.006-.446 0-.446 0s.01 7.146 0 7.12L5.755 8.054c-.067.106-.222.386-.222.386z" stroke-width=".5" fill="#000"/><path class="cls-3" d="M18.758 15.56 12.591 12c0 .595-.893.595-.893 0l-6.165 3.56.226.388s6.174-3.578 6.163-3.562v7.121s.426.002.446 0c0 0-.019-7.138 0-7.12l6.168 3.56s.208-.36.222-.387z" stroke-width=".5"/></g></g></svg>
        } />);
}

type ModbusTcpConfig = API.getType["modbus_tcp/config"];

interface config {
    evse_enable: boolean;
}

export class ModbusTcp extends ConfigComponent<'modbus_tcp/config', {}, config> {
    constructor() {
        super('modbus_tcp/config',
              () => __("modbus_tcp.script.save_failed"));

        util.addApiEventListener('evse/slots', () => {
            this.setState({evse_enable: API.get('evse/slots')[EVSE_SLOT_MODBUS_TCP].active});
        });
    }

    override async sendSave(t: "modbus_tcp/config", cfg: config & ModbusTcpConfig) {
        await API.save_unchecked('evse/modbus_tcp_enabled', {enabled: this.state.evse_enable}, () => __("evse.script.save_failed"));
        await super.sendSave(t, cfg);
    }

    override async sendReset(t: "modbus_tcp/config"){
        await API.save_unchecked('evse/modbus_tcp_enabled', {enabled: false}, () => __("evse.script.save_failed"));
        await super.sendReset(t);
    }

    override getIsModified(t: "modbus_tcp/config"): boolean {
        let evse = API.get_unchecked("evse/modbus_tcp_enabled");
        if (evse != null && evse.enabled)
            return true;
        return super.getIsModified(t);
    }

    render(props: {}, state: ModbusTcpConfig & config) {
        if (!util.render_allowed())
            return <SubPage name="modbus_tcp" />;

        let docu = <>
            <FormSeparator heading={__("modbus_tcp.content.table_docu")} />
            <table class="table table-bordered table-sm">
                {__("modbus_tcp.content.table_content")}
            </table>
        </>;

        return (
            <SubPage name="modbus_tcp">
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
                                max={65535}
                                required/>
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

export function init() {
}
