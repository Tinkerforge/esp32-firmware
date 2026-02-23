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
import { __ } from "../../ts/translation";
import { h, Fragment, Component, RefObject } from "preact";
import { Alert } from "react-bootstrap";
import { ConfigComponent } from "../../ts/components/config_component";
import { FormRow         } from "../../ts/components/form_row";
import { IndicatorGroup  } from "../../ts/components/indicator_group";
import { InputText       } from "../../ts/components/input_text";
import { IPConfiguration } from "../../ts/components/ip_configuration";
import { Switch          } from "../../ts/components/switch";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { register_status_provider, ModuleStatus } from "../../ts/status_registry";
import { EthernetState } from "./ethernet_state.enum";

export function EthernetNavbar() {
    return (
        <NavbarItem name="ethernet" module="ethernet" title={__("ethernet.navbar.ethernet")} symbol={
            <svg width="24" height="24" fill="currentColor" version="1.1" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><g transform="matrix(-1 0 0 1 23 1.99)"><path d="m12 9.98 0.0144 4.83c1.65 0 2.2 0.548 2.2 2.2h6.8c1.33 0 1.33 2 3.64e-4 2h-6.8c0 1.65-0.548 2.2-2.2 2.2h-2c-1.65 0-2.2-0.548-2.2-2.2h-6.8c-1.33 0-1.33-2 0-2h6.8c0-1.65 0.548-2.2 2.2-2.2l-0.0082-4.82zm-2.49 7.28-5e-6 1.5c-1.8e-6 0.551 0.199 0.75 0.75 0.75h1.5c0.551 0 0.75-0.199 0.75-0.75l2e-6 -1.5c1e-6 -0.551-0.199-0.75-0.75-0.75h-1.5c-0.551 0-0.75 0.199-0.75 0.75z" stroke-width="1.99"/><g transform="translate(-.981 -.994)" fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="2"><rect x="2" y="2" width="20" height="8" rx="2" ry="2"/><line x1="6" x2="6.01" y1="6" y2="6"/></g></g></svg>
        } />);
}

type EthernetConfig = API.getType['ethernet/config'];

export class Ethernet extends ConfigComponent<'ethernet/config', {status_ref?: RefObject<EthernetStatus>}> {
    ipconfig_valid: boolean = true;

    constructor() {
        super('ethernet/config',
              () => __("ethernet.script.save_failed"));
    }

    override async isSaveAllowed(cfg: EthernetConfig) { return this.ipconfig_valid; }

    override async transformSave(cfg: EthernetConfig) {
        cfg.dns = cfg.dns == "" ? "0.0.0.0" : cfg.dns;
        cfg.dns2 = cfg.dns2 == "" ? "0.0.0.0" : cfg.dns2;
        return cfg;
    }

    render(props: {}, state: Readonly<EthernetConfig>) {
        if (!util.render_allowed())
            return <SubPage name="ethernet" />;

        const eth_state = API.get("ethernet/state");
        const saved_config = API.get("ethernet/config");

        // Mismatch: config says disabled, but the interface is still active
        // (will be turned off on next reboot).
        const disabled_but_active = !saved_config.enable_ethernet
            && eth_state.connection_state != EthernetState.NotConfigured;

        return (
            <SubPage name="ethernet" title={__("ethernet.content.ethernet")}>
                <SubPage.Status>
                    <FormRow label={__("ethernet.status.ethernet_connection")}>
                        <IndicatorGroup
                            style="width: 100%"
                            class="flex-wrap"
                            value={eth_state.connection_state}
                            items={[
                                ["primary", __("ethernet.status.not_configured")],
                                ["danger",  __("ethernet.status.not_connected")],
                                ["warning", __("ethernet.status.connecting")],
                                ["success", __("ethernet.status.connected")],
                            ]}/>
                    </FormRow>

                    <FormRow label={__("ethernet.content.status_ip")}>
                        <InputText
                            value={eth_state.ip != "0.0.0.0"
                                ? eth_state.ip + " (/" + util.countBits(util.parseIP(eth_state.subnet)) + ")"
                                : __("ethernet.content.status_ip_none")}
                        />
                    </FormRow>

                    <FormRow label={__("ethernet.content.status_link")}>
                        <InputText
                            value={eth_state.link_speed > 0
                                ? eth_state.link_speed + " Mbps, " + (eth_state.full_duplex
                                    ? __("ethernet.content.full_duplex")
                                    : __("ethernet.content.half_duplex"))
                                : __("ethernet.content.status_link_none")}
                        />
                    </FormRow>

                    <FormRow label={__("ethernet.content.mac")}>
                        <InputText
                            value={eth_state.mac.length == 0 ? __("ethernet.content.mac_none") : eth_state.mac}
                            style={eth_state.mac.length == 0 ? undefined : "font-family:monospace"}
                        />
                    </FormRow>
                </SubPage.Status>

                <SubPage.Config
                    id="ethernet_config_form"
                    isModified={this.isModified()}
                    isDirty={this.isDirty()}
                    onSave={this.save}
                    onReset={this.reset}
                    onDirtyChange={this.setDirty}>

                    {disabled_but_active &&
                        <Alert variant="warning">
                            {__("ethernet.content.disabled_but_active")}{" "}
                            <a href="#" onClick={async (e) => {
                                e.preventDefault();
                                if (await util.async_modal_ref.current.show({
                                        title: () => __("main.reboot_title"),
                                        body: () => __("ethernet.content.reboot_body"),
                                        no_text: () => __("main.abort"),
                                        yes_text: () => __("main.reboot"),
                                        no_variant: "secondary",
                                        yes_variant: "danger",
                                    })) {
                                    util.reboot();
                                }
                            }}>{__("ethernet.content.restart_now")}</a>
                        </Alert>}

                    <FormRow label={__("ethernet.content.enable")}>
                        <Switch desc={__("ethernet.content.enable_desc")}
                                checked={state.enable_ethernet}
                                onClick={this.toggle('enable_ethernet')}/>
                    </FormRow>

                    <IPConfiguration
                        showAnyAddress
                        showDhcp
                        showDns
                        onValue={(v) => this.setState(v)}
                        value={state}
                        setValid={(v) => this.ipconfig_valid = v}
                        forbidNetwork={[
                                {ip: util.parseIP("127.0.0.1"), subnet: util.parseIP("255.0.0.0"), name: "localhost"}
                            ].concat(
                                !API.hasModule("wifi") ? [] :
                                [{ip: util.parseIP(API.get_unchecked("wifi/ap_config").ip),
                                subnet: util.parseIP(API.get_unchecked("wifi/ap_config").subnet),
                                name: __("component.ip_configuration.wifi_ap")}]
                            ).concat(
                                !API.hasModule("wireguard") || API.get_unchecked("wireguard/config").internal_ip == "0.0.0.0" ? [] :
                                [{ip: util.parseIP(API.get_unchecked("wireguard/config").internal_ip),
                                subnet: util.parseIP(API.get_unchecked("wireguard/config").internal_subnet),
                                name: __("component.ip_configuration.wireguard")}]
                            )
                        }
                        />

                </SubPage.Config>
            </SubPage>
        );
    }
}

export class EthernetStatus extends Component
{
    render() {
        return <StatusSection name="ethernet" />;

        if (!util.render_allowed() || !API.get('ethernet/config').enable_ethernet)
            return <StatusSection name="ethernet" />

        let state = API.get('ethernet/state');

        return <StatusSection name="ethernet">
                <FormRow label={__("ethernet.status.ethernet_connection")} label_muted={state.ip != "0.0.0.0" ? state.ip : ""}>
                    <IndicatorGroup
                        style="width: 100%"
                        class="flex-wrap"
                        value={state.connection_state}
                        items={[
                            ["primary", __("ethernet.status.not_configured")],
                            ["danger", __("ethernet.status.not_connected")],
                            ["warning", __("ethernet.status.connecting")],
                            ["success", __("ethernet.status.connected")],
                        ]}/>
                </FormRow>
            </StatusSection>;
    }
}

export function pre_init() {
}

export function init() {
    register_status_provider("ethernet", {
        name: () => __("ethernet.navbar.ethernet"),
        href: "#ethernet",
        get_status: () => {
            const state = API.get("ethernet/state");
            const config = API.get("ethernet/config");

            if (!config?.enable_ethernet) {
                // Config says disabled, but interface is still active until reboot.
                if (state?.connection_state != undefined
                    && state.connection_state != EthernetState.NotConfigured) {
                    return {
                        status: ModuleStatus.Error,
                        text: () => __("ethernet.status.disabled_after_reboot")
                    };
                }
                return {status: ModuleStatus.Disabled};
            }

            switch (state?.connection_state) {
                case EthernetState.Connected:
                    return {
                        status: ModuleStatus.Ok,
                        text: () => state.ip + " (/" + util.countBits(util.parseIP(state.subnet)) + ")"
                    };
                case EthernetState.Connecting:
                    return {
                        status: ModuleStatus.Warning,
                        text: () => __("ethernet.status.connecting")
                    };
                default:
                    return {
                        status: ModuleStatus.Ok,
                        text: () => __("ethernet.status.not_connected")
                    };
            }
        }
    });
}
