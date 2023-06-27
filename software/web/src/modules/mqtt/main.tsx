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

import { h, render, Fragment, Component } from "preact";
import { __ } from "../../ts/translation";

import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputSelect } from "src/ts/components/input_select";
import { InputNumber } from "../../ts/components/input_number";
import { InputPassword } from "../../ts/components/input_password";
import { Switch } from "../../ts/components/switch";
import { IndicatorGroup } from "src/ts/components/indicator_group";
import { SubPage } from "src/ts/components/sub_page";

type MqttConfig = API.getType['mqtt/config'];

interface MqttState {
    auto_discovery_config: API.getType['mqtt/auto_discovery_config'];
}

export class Mqtt extends ConfigComponent<'mqtt/config', {}, MqttState> {
    constructor() {
        super('mqtt/config',
              __("mqtt.script.save_failed"),
              __("mqtt.script.reboot_content_changed"));

        util.addApiEventListener('mqtt/auto_discovery_config', () => {
            this.setState({auto_discovery_config: API.get('mqtt/auto_discovery_config')});
        });
    }

    override async sendSave(t: "mqtt/config", cfg: API.getType["mqtt/config"]) {
        if (API.hasModule('mqtt_auto_discovery'))
            await API.save('mqtt/auto_discovery_config', this.state.auto_discovery_config, __("mqtt.script.save_failed"));
        await super.sendSave(t, cfg);
    }

    render(props: {}, state: Readonly<MqttConfig & MqttState>) {
        if (!util.render_allowed())
            return <></>

        return (
            <SubPage>
                <ConfigForm id="mqtt_config_form" title={__("mqtt.content.mqtt")} isModified={this.isModified()} onSave={() => this.save()} onReset={this.reset} onDirtyChange={(d) => this.ignore_updates = d}>
                    <FormRow label={__("mqtt.content.enable_mqtt")}>
                        <Switch desc={__("mqtt.content.enable_mqtt_desc")}
                                checked={state.enable_mqtt}
                                onClick={this.toggle('enable_mqtt')}/>
                    </FormRow>

                    <FormRow label={__("mqtt.content.broker_host")}>
                        <InputText required={state.enable_mqtt}
                                   maxLength={128}
                                   value={state.broker_host}
                                   onValue={this.set("broker_host")}/>
                    </FormRow>

                    <FormRow label={__("mqtt.content.port")} label_muted={__("mqtt.content.port_muted")}>
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
                                     max={4294967296}
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

render(<Mqtt/>, $('#mqtt')[0])


interface MqttStatusState {
    state: API.getType['mqtt/state']
    config: API.getType['mqtt/config'];
}

export class MqttStatus extends Component<{}, MqttStatusState>
{
    constructor()
    {
        super();

        util.addApiEventListener('mqtt/state', () => {
            this.setState({state: API.get('mqtt/state')})
        });

        util.addApiEventListener('mqtt/config', () => {
            this.setState({config: API.get('mqtt/config')})
        });
    }

    render(props: {}, state: MqttStatusState)
    {
        if (!util.render_allowed() || !state.config.enable_mqtt)
            return <></>;

        return <>
                <FormRow label={__("mqtt.status.connection")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
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
            </>;
    }
}

render(<MqttStatus/>, $('#status-mqtt')[0]);

export function add_event_listeners(source: API.APIEventTarget) {}

export function init() {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-mqtt').prop('hidden', !module_init.mqtt);
}
