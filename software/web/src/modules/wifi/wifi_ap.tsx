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
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { IPConfiguration } from "../../ts/components/ip_configuration";
import { Button, Modal } from "react-bootstrap";
import { InputText } from "../../ts/components/input_text";
import { InputPassword } from "../../ts/components/input_password";
import { InputSelect } from "../../ts/components/input_select";
import { SubPage } from "../../ts/components/sub_page";

type APConfig = API.getType["wifi/ap_config"];

type WifiAPState = {show_modal: boolean};

export class WifiAP extends ConfigComponent<'wifi/ap_config', {}, WifiAPState> {
    ipconfig_valid: boolean = true;

    constructor() {
        super('wifi/ap_config',
              () => __("wifi.script.ap_save_failed"),
              () => __("wifi.script.ap_reboot_content_changed"));
    }

    override async isSaveAllowed(cfg: APConfig) { return this.ipconfig_valid; }

    dismissModal() {
        this.setState({show_modal: false,
                       ap_fallback_only: API.get("wifi/ap_config").ap_fallback_only,
                       enable_ap: API.get("wifi/ap_config").enable_ap});
    }

    override render(props: {}, state: Readonly<APConfig & WifiAPState>) {
        if (!util.render_allowed())
            return <SubPage name="wifi_ap" />;

        return (
            <SubPage name="wifi_ap">
                <ConfigForm id="wifi_ap_config_form"
                            title={__("wifi.content.ap_settings")}
                            isModified={this.isModified()}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("wifi.content.ap_enable")} label_muted={__("wifi.content.ap_enable_muted")}>
                        <InputSelect
                            value={
                                (state.enable_ap && state.ap_fallback_only) ? 1 :
                                (state.enable_ap ? 0 : 2)
                            }
                            onValue={(v) => {
                                this.setState({show_modal: v == "2",
                                               enable_ap: v != "2",
                                               ap_fallback_only: v == "1"});
                            }}
                            items={[
                                ["0", __("wifi.content.ap_enabled")],
                                ["1", __("wifi.content.ap_fallback_only")],
                                ["2", __("wifi.content.ap_disabled")]
                            ]}
                        />
                    </FormRow>

                    <FormRow label={__("wifi.content.ap_ssid")}>
                        <InputText required={state.enable_ap}
                                   maxLength={32}
                                   value={state.ssid}
                                   onValue={this.set("ssid")}/>
                    </FormRow>

                    <FormRow label={__("wifi.content.ap_hide_ssid")}>
                        <Switch desc={__("wifi.content.ap_hide_ssid_desc_pre") + (API.get("wifi/state")?.ap_bssid ?? " ") + __("wifi.content.ap_hide_ssid_desc_post")}
                                checked={state.hide_ssid}
                                onClick={this.toggle("hide_ssid")}
                        />
                    </FormRow>

                    <FormRow label={__("wifi.content.ap_passphrase")}>
                        <InputPassword minLength={8}
                                       maxLength={64}
                                       value={state.passphrase}
                                       onValue={this.set("passphrase")}
                                       hideClear
                                       />
                    </FormRow>

                    <FormRow label={__("wifi.content.ap_channel")} label_muted={__("wifi.content.ap_channel_muted")}>
                        <InputSelect
                            value={state.channel}
                            onValue={(v) => this.setState({channel: parseInt(v)})}
                            items={[
                                ["0", __("wifi.content.ap_channel_auto_select")],
                                ["1", "1"],
                                ["2", "2"],
                                ["3", "3"],
                                ["4", "4"],
                                ["5", "5"],
                                ["6", "6"],
                                ["7", "7"],
                                ["8", "8"],
                                ["9", "9"],
                                ["10", "10"],
                                ["11", "11"],
                                ["12", "12"],
                                ["13", "13"],
                            ]}
                        />
                    </FormRow>

                    <IPConfiguration
                        showAnyAddress
                        showDhcp={false}
                        showDns={false}
                        onValue={(v) => this.setState(v)}
                        value={state}
                        setValid={(v) => this.ipconfig_valid = v}
                        min_subnet_prefix={24}
                        max_subnet_prefix={30}
                        forbidNetwork={[
                                {ip: util.parseIP("127.0.0.1"), subnet: util.parseIP("255.0.0.0"), name: "localhost"}
                            ].concat(
                                !API.hasModule("ethernet") || API.get_unchecked("ethernet/config").ip == "0.0.0.0" ? [] :
                                [{ip: util.parseIP(API.get_unchecked("ethernet/config").ip),
                                subnet: util.parseIP(API.get_unchecked("ethernet/config").subnet),
                                name: __("component.ip_configuration.ethernet")}]
                            ).concat(
                                API.get("wifi/sta_config").ip == "0.0.0.0" ? [] :
                                [{ip: util.parseIP(API.get("wifi/sta_config").ip),
                                subnet: util.parseIP(API.get("wifi/sta_config").subnet),
                                name: __("component.ip_configuration.wifi_sta")}]
                            ).concat(
                                !API.hasModule("wireguard") || API.get_unchecked("wireguard/config").internal_ip == "0.0.0.0" ? [] :
                                [{ip: util.parseIP(API.get_unchecked("wireguard/config").internal_ip),
                                subnet: util.parseIP(API.get_unchecked("wireguard/config").internal_subnet),
                                name: __("component.ip_configuration.wireguard")}]
                            )
                        }
                        />

                </ConfigForm>

                <Modal show={state.show_modal} onHide={() => {this.dismissModal()}} centered>
                    {/* There seems to be an incompatibility between preact's and react-bootstrap's typings*/ }
                    <Modal.Header {...{closeButton: true} as any}>
                        <span class="modal-title form-label">{__("wifi.content.confirm_title")}</span>
                    </Modal.Header>
                    <Modal.Body>{__("wifi.content.confirm_content")}</Modal.Body>
                    <Modal.Footer>
                        <Button variant="secondary" onClick={() => {this.dismissModal()}}>
                            {__("wifi.content.confirm_abort")}
                        </Button>
                        <Button variant="danger" onClick={() => this.setState({show_modal: false})}>
                            {__("wifi.content.confirm_confirm")}
                        </Button>
                    </Modal.Footer>
                </Modal>
            </SubPage>
        );
    }
}
