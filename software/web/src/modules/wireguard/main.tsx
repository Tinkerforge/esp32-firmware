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
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { InputPassword } from "../../ts/components/input_password";
import { Switch } from "../../ts/components/switch";
import { IPConfiguration } from "../../ts/components/ip_configuration";
import { Slash } from "react-feather";
import { InputIP } from "../../ts/components/input_ip";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { InputSubnet } from "../../ts/components/input_subnet";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";

export function WireguardNavbar() {
    return (
        <NavbarItem name="wireguard" module="wireguard" title={__("wireguard.navbar.wireguard")} symbol={
            <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="0.1"><path d="m8.69 7.44c2.48-1.52 5.64-.589 6.83 1.69.225.432.253 1.1.111 1.55-.491 1.56-1.65 2.44-3.24 2.81.469-.402.843-.858.962-1.49.125-.603-.0109-1.23-.375-1.73-.58-.796-1.62-1.11-2.54-.775-.98.372-1.52 1.27-1.42 2.37.0899 1.02.865 1.68 2.32 1.93-.217.115-.384.199-.547.29-.664.364-1.24.867-1.69 1.47-.147.199-.248.215-.473.0776-2.92-1.78-3.1-6.26.081-8.21zm-2.18 11c-.468.119-.922.295-1.4.452.234-1.58 2.08-3.04 3.65-2.87-.453.624-.718 1.37-.763 2.14-.52.0958-1.01.16-1.49.281zm9.97-15.4c.463.017.927.00998 1.39.0209.116.00764.231.0236.344.0479-.104.159-.22.309-.349.448-.166.154-.353.305-.591.0706-.0574-.0564-.193-.0435-.293-.0448-.461-.00605-.922-.0208-1.38-.00341-.399.0128-.797.0534-1.19.122-.0737.0132-.184.258-.15.349.08.213.197.449.369.585.639.504 1.32.957 1.96 1.46.623.486 1.2 1.02 1.56 1.75.461.956.474 1.96.275 2.97-.332 1.68-1.18 3.07-2.56 4.09-.555.408-1.24.639-1.88.932-.559.258-1.13.479-1.7.734-1.01.46-1.58 1.56-1.41 2.7.153 1.05 1.07 1.92 2.12 2.1 1.26.216 2.56-.604 2.87-1.89.347-1.44-.436-2.73-1.9-3.12-.0646-.0172-.129-.0334-.264-.0682.392-.175.731-.3 1.04-.472.546-.3 1.08-.618 1.61-.954.155-.0989.238-.099.37.015 1.01.872 1.61 1.96 1.78 3.29.279 2.2-.763 4.22-2.73 5.26-3.04 1.6-6.76-.222-7.43-3.59-.575-2.89 1.46-5.51 3.92-6.01 1.05-.218 2.02-.657 2.77-1.47.484-.524.718-.974.798-1.18.149-.38.225-.785.225-1.19-.0161-.353-.0991-.7-.245-1.02-.256-.584-1.24-1.51-1.48-1.71l-2.31-1.81c-.0815-.067-.173-.0622-.372-.0487-.236.016-.839.0494-1.1-.0188.211-.159.785-.391 1.03-.578-.749-.506-1.6-.323-2.39-.474.181-.338 1.08-.857 1.59-.915-.0303-.285-.0768-.568-.139-.848-.0312-.115-.159-.226-.271-.292-.271-.159-.559-.29-.87-.448.279-.18.603-.281.935-.289.315-.012.63.0187.936.0912.556.127 1 .0441 1.44-.334-.348-.14-.697-.268-1.03-.42-.333-.152-.657-.321-.972-.508.876.122 1.72.45 2.62.33.0076-.0407.0153-.0815.0229-.122-.67-.156-1.34-.312-2.08-.485 1.24-.114 2.4-.132 3.49.4.308.15.63.274.925.445.144.0835.241.248.359.376.0938.102.169.238.284.299.437.233.919.242 1.41.23.0036-.0559.0071-.108.0108-.164.494.154 1.05.723 1.05 1.14-.8 0-1.6-.00306-2.4.00445-.0854.000811-.17.0632-.255.097.0808.0471.16.132.243.135z" fill="currentColor"></path><path d="m15.4 1.84a.122.122 0 00-.0156.195.184.184 0 00.253.0678c.077-.0388.152-.0801.245-.129-.0749-.0639-.135-.117-.197-.168-.109-.0896-.199-.0333-.286.0337z" fill="#fff"></path></svg>
        } />);
}

type WireguardConfig = API.getType["wireguard/config"];

export class Wireguard extends ConfigComponent<'wireguard/config', {status_ref?: RefObject<WireguardStatus>}> {
    ipconfig_valid: boolean = true;
    constructor() {
        super('wireguard/config',
              __("wireguard.script.save_failed"),
              __("wireguard.script.reboot_content_changed"));
    }

    render(props: {}, state: Readonly<WireguardConfig>) {
        if (!util.render_allowed())
            return <SubPage name="wireguard" />;

        return (
            <SubPage name="wireguard">
                <ConfigForm id="wireguard_config_form" title={__("wireguard.content.wireguard")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("wireguard.content.enable_wireguard")}>
                        <Switch desc={__("wireguard.content.enable_wireguard_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}/>
                    </FormRow>

                    <FormRow label={__("wireguard.content.default_if")}>
                        <Switch desc={__("wireguard.content.default_if_desc")}
                                checked={state.make_default_interface}
                                onClick={this.toggle('make_default_interface')}/>
                    </FormRow>

                    <IPConfiguration
                        showAnyAddress={false}
                        onValue={(v) => this.setState({internal_ip: v.ip, internal_subnet: v.subnet, internal_gateway: v.gateway})}
                        value={{ip: state.internal_ip, subnet: state.internal_subnet, gateway: state.internal_gateway}}
                        setValid={(v) => this.ipconfig_valid = v}

                        ip_label={__("wireguard.content.internal_ip")}
                        subnet_label={__("wireguard.content.internal_subnet")}
                        gateway_label={__("wireguard.content.internal_gateway")}
                        forbidNetwork={[
                            {ip: util.parseIP("127.0.0.1"), subnet: util.parseIP("255.0.0.0"), name: "localhost"}
                        ].concat(
                            !API.hasModule("ethernet") || API.get_unchecked("ethernet/config").ip == "0.0.0.0" ? [] :
                            [{ip: util.parseIP(API.get_unchecked("ethernet/config").ip),
                            subnet: util.parseIP(API.get_unchecked("ethernet/config").subnet),
                            name: __("component.ip_configuration.ethernet")}]
                        ).concat(
                            !API.hasModule("wifi") || API.get_unchecked("wifi/sta_config").ip == "0.0.0.0" ? [] :
                            [{ip: util.parseIP(API.get_unchecked("wifi/sta_config").ip),
                            subnet: util.parseIP(API.get_unchecked("wifi/sta_config").subnet),
                            name: __("component.ip_configuration.wifi_sta")}]
                        ).concat(
                            !API.hasModule("wifi") ? [] :
                            [{ip: util.parseIP(API.get_unchecked("wifi/ap_config").ip),
                            subnet: util.parseIP(API.get_unchecked("wifi/ap_config").subnet),
                            name: __("component.ip_configuration.wifi_ap")}]
                        )
                    }
                        />

                    <FormRow label={__("wireguard.content.remote_host")}>
                        <InputText required
                                   maxLength={64}
                                   value={state.remote_host}
                                   onValue={this.set("remote_host")}/>
                    </FormRow>

                    <FormRow label={__("wireguard.content.port")}>
                        <InputNumber required
                                     min={1}
                                     max={65535}
                                     value={state.remote_port}
                                     onValue={this.set("remote_port")}/>
                    </FormRow>
                    <FormRow label={__("wireguard.content.local_port")}>
                        <InputNumber required
                                     min={1}
                                     max={65535}
                                     value={state.local_port}
                                     onValue={this.set("local_port")}/>
                    </FormRow>

                    <FormRow label={__("wireguard.content.private_key")}>
                        <InputPassword maxLength={44}
                                       value={state.private_key}
                                       onValue={this.set("private_key")}
                                       />
                    </FormRow>

                    <FormRow label={__("wireguard.content.remote_public_key")}>
                        <InputPassword maxLength={44}
                                       value={state.remote_public_key}
                                       onValue={this.set("remote_public_key")}
                                       />
                    </FormRow>

                    <FormRow label={__("wireguard.content.preshared_key")} label_muted={__("wireguard.content.preshared_key_muted")}>
                        <InputPassword maxLength={44}
                                       value={state.preshared_key}
                                       onValue={this.set("preshared_key")}
                                       clearSymbol={<Slash/>}
                                       clearPlaceholder={__("wireguard.script.preshared_key_unused")}
                                       allowAPIClear
                                       />
                    </FormRow>

                    <FormRow label={__("wireguard.content.allowed_ip")}>
                        <InputIP invalidFeedback={__("wireguard.content.allowed_ip_invalid")}
                            required
                            value={state.allowed_ip}
                            onValue={this.set("allowed_ip")}/>
                    </FormRow>

                    <FormRow label={__("wireguard.content.allowed_subnet")}>
                        <InputSubnet required
                            minPrefixLength={0}
                            maxPrefixLength={32}
                            value={state.allowed_subnet}
                            onValue={this.set("allowed_subnet")}
                            placeholder={__("component.ip_configuration.subnet_placeholder")}
                        />
                    </FormRow>
                </ConfigForm>
            </SubPage>
        )
    }
}

export class WireguardStatus extends Component {
    render() {
        if (!util.render_allowed() || !API.get("wireguard/config").enable)
            return <StatusSection name="wireguard" />;

        return <StatusSection name="wireguard">
                <FormRow label={__("wireguard.status.connection")}>
                    <IndicatorGroup
                        style="width: 100%"
                        class="flex-wrap"
                        value={API.get("wireguard/state").state}
                        items={[
                            ["primary", __("wireguard.status.not_configured")],
                            ["warning", __("wireguard.status.waiting_for_timesync")],
                            ["warning", __("wireguard.status.not_connected")],
                            ["success", __("wireguard.status.connected")]
                        ]}/>
                </FormRow>
            </StatusSection>;
    }
}

export function init() {
}
