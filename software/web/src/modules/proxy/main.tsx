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
import { h, Fragment } from "preact";
import { __ } from "../../ts/translation";
import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { Table } from "react-bootstrap";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { InputPassword } from "../../ts/components/input_password";
import { InputNumber } from "../../ts/components/input_number";
import { InputIP } from "../../ts/components/input_ip";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Cpu, Slash } from "react-feather";

export function ProxyNavbar() {
    return <NavbarItem name="proxy" module="proxy" title={__("proxy.navbar.proxy")} symbol={<Cpu />} />;
}

interface ProxyState {
    devices: Readonly<API.getType['proxy/devices']>;
    error_counters: Readonly<API.getType['proxy/error_counters']>;
}

export class Proxy extends ConfigComponent<'proxy/config', {}, ProxyState> {
    constructor() {
        super('proxy/config',
            __("proxy.script.save_failed"),
            __("proxy.script.reboot_content_changed"));

        util.addApiEventListener('proxy/devices', () => {
            this.setState({devices: API.get('proxy/devices')});
        });

        util.addApiEventListener('proxy/error_counters', () => {
            this.setState({error_counters: API.get('proxy/error_counters')});
        });

    }

    render(props: {}, state: Readonly<API.getType['proxy/config'] & ProxyState>) {
        if (!util.render_allowed())
            return <SubPage name="proxy" />;

        return (
            <SubPage name="proxy">
                <ConfigForm id="proxy_config_form" title={__("proxy.content.proxy")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("proxy.content.auth_secret")} label_muted={__("proxy.content.auth_secret_muted")}>
                        <InputPassword maxLength={64}
                                       value={state.authentication_secret}
                                       onValue={this.set("authentication_secret")}
                                       clearPlaceholder={__("proxy.content.auth_disabled")}
                                       clearSymbol={<Slash/>}
                                       allowAPIClear
                                       />
                    </FormRow>

                    <FormRow label={__("proxy.content.listen_address")} label_muted={__("proxy.content.listen_address_muted")}>
                        <InputIP invalidFeedback={__("component.ip_configuration.static_ip_invalid")}
                                 required
                                 value={state.listen_address}
                                 onValue={this.set("listen_address")}/>
                    </FormRow>

                    <FormRow label={__("proxy.content.listen_port")} label_muted={__("proxy.content.listen_port_muted")}>
                        <InputNumber required
                                     min={1}
                                     max={65535}
                                     value={state.listen_port}
                                     onValue={this.set("listen_port")}/>
                    </FormRow>

                    <FormSeparator/>

                    <FormRow label={__("proxy.content.bricklet_table")}>
                        <Table hover borderless>
                            <thead class="thead-light">
                                <tr>
                                    <th scope="col">{__("proxy.content.port")}</th>
                                    <th scope="col">{__("proxy.content.UID")}</th>
                                    <th scope="col">{__("proxy.content.device_type")}</th>
                                </tr>
                            </thead>
                            <tbody>
                                {state.devices.length == 0 ? <tr><td colSpan={3}>{__("proxy.script.no_bricklets")}</td></tr>
                                    : state.devices.map(d => <tr>
                                    <td>{d.port}</td>
                                    <td>{d.uid}</td>
                                    <td>{d.name == "unknown device" ? __("proxy.script.unknown_device") : d.name}</td>
                                </tr>)}
                            </tbody>
                        </Table>
                    </FormRow>

                    <FormRow label={__("proxy.content.error_counters")}>
                        <Table hover borderless>
                            <thead class="thead-light">
                                <tr>
                                    <th scope="col">{__("proxy.content.port")}</th>
                                    <th scope="col">{__("proxy.content.spitfp_checksum")}</th>
                                    <th scope="col">{__("proxy.content.spitfp_frame")}</th>
                                    <th scope="col">{__("proxy.content.tfp_frame")}</th>
                                    <th scope="col">{__("proxy.content.tfp_unexpected")}</th>
                                </tr>
                            </thead>
                            <tbody>
                                {Object.keys(state.error_counters).map(port => <tr>
                                    <td>{port}</td>
                                    <td>{state.error_counters[port].SpiTfpChecksum}</td>
                                    <td>{state.error_counters[port].SpiTfpFrame}</td>
                                    <td>{state.error_counters[port].TfpFrame}</td>
                                    <td>{state.error_counters[port].TfpUnexpected}</td>
                                </tr>)}
                            </tbody>
                        </Table>
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

export function init() {
}
