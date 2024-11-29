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
import { h, Fragment, Component, RefObject } from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputSelect } from "../../ts/components/input_select";
import { InputNumber } from "../../ts/components/input_number";
import { InputPassword } from "../../ts/components/input_password";
import { Switch } from "../../ts/components/switch";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { SubPage } from "../../ts/components/sub_page";
import { Collapse } from "react-bootstrap";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { Rss } from "react-feather";

export function MqttNavbar() {
    return <NavbarItem name="mqtt" module="mqtt" title={__("mqtt.navbar.mqtt")} symbol={<Rss />} />;
}

type MqttConfig = API.getType["mqtt/config"];

interface MqttState {
    auto_discovery_config: API.getType['mqtt/auto_discovery_config'];
}

export class Mqtt extends ConfigComponent<'mqtt/config', {status_ref?: RefObject<MqttStatus>}, MqttState> {
    constructor() {
        super('mqtt/config',
              () => __("mqtt.script.save_failed"),
              () => __("mqtt.script.reboot_content_changed"));

        util.addApiEventListener('mqtt/auto_discovery_config', () => {
            this.setState({auto_discovery_config: API.get('mqtt/auto_discovery_config')});
        });
    }

    override async sendSave(t: "mqtt/config", cfg: API.getType["mqtt/config"]) {
        if (API.hasModule('mqtt_auto_discovery'))
            await API.save('mqtt/auto_discovery_config', this.state.auto_discovery_config, () => __("mqtt.script.save_failed"));
        await super.sendSave(t, cfg);
    }

    override async sendReset(t: "mqtt/config") {
        if (API.hasModule('mqtt_auto_discovery'))
            await API.reset('mqtt/auto_discovery_config', this.error_string, this.reboot_string);

        await super.sendReset(t);
    }

    override getIsModified(t: "mqtt/config"): boolean {
        if (API.hasModule('mqtt_auto_discovery') && API.is_modified('mqtt/auto_discovery_config'))
            return true;
        return super.getIsModified(t);
    }

    render(props: {}, state: Readonly<MqttConfig & MqttState>) {
        if (!util.render_allowed())
            return <SubPage name="mqtt" />;

        let default_ports = [1883, 8883, 80, 443];

        let cert_state = API.get_unchecked('certs/state');
        let certs = cert_state == null ? [] : cert_state.certs.map((c: any) => [c.id.toString(), c.name]) as [string, string][];

        return (
            <SubPage name="mqtt">
                <ConfigForm id="mqtt_config_form" title={__("mqtt.content.mqtt")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("mqtt.content.enable_mqtt")}>
                        <Switch desc={__("mqtt.content.enable_mqtt_desc")}
                                checked={state.enable_mqtt}
                                onClick={this.toggle('enable_mqtt')}/>
                    </FormRow>

                    <FormRow label={__("mqtt.content.protocol")}>
                        <InputSelect
                            items={[
                                ["0", __("mqtt.content.protocol_mqtt")],
                                ["1", __("mqtt.content.protocol_mqtts")],
                                ["2", __("mqtt.content.protocol_ws")],
                                ["3", __("mqtt.content.protocol_wss")],
                            ]}
                            value={state.protocol}
                            onValue={(v) => {
                                let new_proto = parseInt(v);
                                let new_port = this.state.broker_port;
                                if (this.state.broker_port == default_ports[this.state.protocol])
                                    new_port = default_ports[new_proto];

                                this.setState({protocol: new_proto, broker_port: new_port})
                            }}/>
                    </FormRow>

                    <Collapse in={state.protocol == 1 || state.protocol == 3}>
                        <div>
                            <FormRow label={__("mqtt.content.cert")}>
                                <InputSelect items={[
                                        ["-1", __("mqtt.content.use_cert_bundle")],
                                    ].concat(certs) as [string, string][]
                                    }
                                    value={state.cert_id}
                                    onValue={(v) => this.setState({cert_id: parseInt(v)})}
                                    disabled={cert_state == null}
                                    required={state.protocol == 1 || state.protocol == 3}
                                />
                            </FormRow>

                            <FormRow label={__("mqtt.content.client_cert")}>
                                <InputSelect items={[
                                        ["-1", __("mqtt.content.no_cert")],
                                    ].concat(certs) as [string, string][]
                                    }
                                    value={state.client_cert_id}
                                    onValue={(v) => this.setState({client_cert_id: parseInt(v)})}
                                    disabled={cert_state == null}
                                    required={state.client_key_id != -1}
                                />
                            </FormRow>

                            <FormRow label={__("mqtt.content.client_key")}>
                                <InputSelect items={[
                                        ["-1", __("mqtt.content.no_cert")],
                                    ].concat(certs) as [string, string][]
                                    }
                                    value={state.client_key_id}
                                    onValue={(v) => this.setState({client_key_id: parseInt(v)})}
                                    disabled={cert_state == null}
                                    required={state.client_cert_id != -1}
                                />
                            </FormRow>
                        </div>
                    </Collapse>

                    <FormRow label={__("mqtt.content.broker_host")}>
                        <InputText required={state.enable_mqtt}
                                   maxLength={128}
                                   value={state.broker_host}
                                   onValue={this.set("broker_host")}/>
                    </FormRow>

                    <Collapse in={state.protocol == 2 || state.protocol == 3}>
                        <div>
                            <FormRow label={__("mqtt.content.path")} label_muted={__("mqtt.content.path_muted")}>
                                <InputText
                                    pattern="/.*"
                                    invalidFeedback={__("mqtt.content.path_invalid")}
                                    maxLength={64}
                                    value={state.path}
                                    onValue={this.set("path")}
                                />
                            </FormRow>
                        </div>
                    </Collapse>

                    <FormRow label={__("mqtt.content.port")} label_muted={__("mqtt.content.port_muted")(default_ports[state.protocol])}>
                        <InputNumber required
                                     min={1}
                                     max={65535}
                                     value={state.broker_port}
                                     onValue={this.set("broker_port")}/>
                    </FormRow>

                    <FormRow label={__("mqtt.content.username")} label_muted={__("mqtt.content.username_muted")}>
                        <InputText maxLength={64}
                                   value={state.broker_username}
                                   onValue={this.set("broker_username")}/>
                    </FormRow>

                    <FormRow label={__("mqtt.content.password")} label_muted={__("mqtt.content.password_muted")}>
                        <InputPassword maxLength={64}
                                       value={state.broker_password}
                                       onValue={this.set("broker_password")}
                                       />
                    </FormRow>

                    <FormRow label={__("mqtt.content.topic_prefix")} label_muted={__("mqtt.content.topic_prefix_muted")}>
                        <InputText maxLength={64}
                                   pattern="^[^#+$][^#+]*"
                                   value={state.global_topic_prefix}
                                   onValue={this.set("global_topic_prefix")}
                                   invalidFeedback={__("mqtt.content.topic_prefix_invalid")}
                                   />
                    </FormRow>

                    <FormRow label={__("mqtt.content.client_name")}>
                        <InputText required
                                   maxLength={64}
                                   value={state.client_name}
                                   onValue={this.set("client_name")}/>
                    </FormRow>

                    <FormRow label={__("mqtt.content.interval")} label_muted={__("mqtt.content.interval_muted")}>
                        <InputNumber required
                                     step={1}
                                     min={1}
                                     max={24*60*60}
                                     value={state.interval}
                                     unit="s"
                                     onValue={this.set("interval")}/>
                    </FormRow>

                    {API.hasModule('mqtt_auto_discovery') ? <>
                        <FormRow label={__("mqtt.content.auto_discovery_mode")} label_muted={__("mqtt.content.auto_discovery_mode_muted")}>
                            <InputSelect
                                items={[
                                    ["0", __("mqtt.content.auto_discovery_mode_disabled")],
                                    ["1", __("mqtt.content.auto_discovery_mode_generic")],
                                    ["2", __("mqtt.content.auto_discovery_mode_homeassistant")],
                                ]}
                                value={state.auto_discovery_config.auto_discovery_mode}
                                onValue={(v) => {this.setState({auto_discovery_config: {...this.state.auto_discovery_config, auto_discovery_mode: parseInt(v)}})}}/>
                        </FormRow>

                        <FormRow label={__("mqtt.content.auto_discovery_prefix")}>
                            <InputText required
                                    maxLength={64}
                                    pattern="^[^#+$][^#+]*"
                                    value={state.auto_discovery_config.auto_discovery_prefix}
                                    onValue={(v) => this.setState({auto_discovery_config: {...this.state.auto_discovery_config, auto_discovery_prefix: v}})}
                                    invalidFeedback={__("mqtt.content.auto_discovery_prefix_invalid")}
                                    />
                        </FormRow>
                    </> : null}
                </ConfigForm>
            </SubPage>
        );
    }
}

interface MqttStatusState {
    state: API.getType["mqtt/state"];
    config: API.getType["mqtt/config"];
}

export class MqttStatus extends Component<{}, MqttStatusState> {
    constructor() {
        super();

        util.addApiEventListener('mqtt/state', () => {
            this.setState({state: API.get('mqtt/state')})
        });

        util.addApiEventListener('mqtt/config', () => {
            this.setState({config: API.get('mqtt/config')})
        });
    }

    render(props: {}, state: MqttStatusState) {
        if (!util.render_allowed() || !state.config.enable_mqtt)
            return <StatusSection name="mqtt" />;

        return <StatusSection name="mqtt">
                <FormRow label={__("mqtt.status.connection")}>
                    <IndicatorGroup
                        style="width: 100%"
                        class="flex-wrap"
                        value={state.state.connection_state}
                        items={[
                            ["primary", __("mqtt.status.not_configured")],
                            ["danger", __("mqtt.status.not_connected")],
                            ["success", __("mqtt.status.connected")],
                            ["danger", __("mqtt.status.error") + (state.state.connection_state != 3 ? "" : state.state.last_error)]
                        ]}/>
                </FormRow>
            </StatusSection>;
    }
}

export function init() {
}
