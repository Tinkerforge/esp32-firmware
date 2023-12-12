/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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
import { __ } from "../../ts/translation";
import { h, render, Fragment } from "preact";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm      } from "../../ts/components/config_form";
import { FormRow         } from "../../ts/components/form_row";
import { InputText       } from "../../ts/components/input_text";
import { Switch          } from "../../ts/components/switch";
import { SubPage } from "../../ts/components/sub_page";

export class MqttMeter extends ConfigComponent<'mqtt_meter/config'> {
    constructor() {
        super('mqtt_meter/config',
            __("mqtt_meter.script.save_failed"),
            __("mqtt_meter.script.reboot_content_changed"));
    }

    render(props: {}, state: Readonly<API.getType['mqtt_meter/config']>) {
        if (!util.render_allowed())
            return <></>

        return <SubPage>
            <ConfigForm id="mqtt_meter_config_form" title={__("mqtt_meter.content.title")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                <FormRow label={__("mqtt_meter.content.enable_meter")}>
                    <Switch desc={__("mqtt_meter.content.enable_meter_desc")}
                        checked={state.enable}
                        onClick={this.toggle('enable')}
                    />
                </FormRow>

                <FormRow label={__("mqtt_meter.content.has_all_values")}>
                    <Switch desc={__("mqtt_meter.content.has_all_values_desc")}
                        checked={state.has_all_values}
                        onClick={this.toggle('has_all_values')}
                    />
                </FormRow>

                <FormRow label={__("mqtt_meter.content.meter_path")} label_muted={__("mqtt_meter.content.meter_path_muted")}>
                    <InputText required
                        maxLength={128}
                        pattern="^[^#+$][^#+]*"
                        value={state.source_meter_path}
                        onValue={this.set('source_meter_path')}
                        invalidFeedback={__("mqtt_meter.content.meter_path_invalid")}
                    />
                </FormRow>
            </ConfigForm>
        </SubPage>
    }
}

export function init() {
}
render(<MqttMeter />, $("#mqtt_meter")[0]);

export function add_event_listeners(source: API.APIEventTarget) {
}

export function update_sidebar_state(module_init: any) {
    $("#sidebar-mqtt_meter").prop("hidden", !module_init.mqtt_meter);
}
