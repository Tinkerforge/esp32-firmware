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
import { h, Fragment } from "preact";
import { __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Settings } from "react-feather";

export function NetworkNavbar() {
    return <NavbarItem name="network" module="network" title={__("network.navbar.network")} symbol={<Settings />} />;
}

type NetworkConfig = API.getType["network/config"];

export class Network extends ConfigComponent<'network/config'> {
    constructor() {
        super('network/config',
              () => __("network.script.save_failed"),
              () => __("network.script.reboot_content_changed"));
    }

    render(props: {}, state: Readonly<NetworkConfig>) {
        if (!util.render_allowed())
            return <SubPage name="network" />;

        return (
            <SubPage name="network">
                <ConfigForm id="network_config_form"
                            title={__("network.content.network")}
                            isModified={this.isModified()}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("network.content.hostname")}>
                        <InputText maxLength={32}
                                   pattern="[a-zA-Z0-9\-]*"
                                   required
                                   value={state.hostname}
                                   onValue={this.set("hostname")}
                                   invalidFeedback={__("network.content.hostname_invalid")}
                                   />
                    </FormRow>

                    <FormRow label={__("network.content.enable_mdns")}>
                        <Switch desc={__("network.content.enable_mdns_desc")}
                                checked={state.enable_mdns}
                                onClick={this.toggle('enable_mdns')}/>
                    </FormRow>

                    <FormRow label={__("network.content.web_server_port")}>
                        <InputNumber required
                                     min={1}
                                     max={65535}
                                     value={state.web_server_port}
                                     onValue={this.set("web_server_port")}/>
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

export function init() {
}
