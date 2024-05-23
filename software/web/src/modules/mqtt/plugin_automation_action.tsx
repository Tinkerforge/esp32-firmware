/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

import { h, Fragment } from "preact";
import { useState } from "preact/hooks";
import { __ } from "../../ts/translation";
import { AutomationActionID } from "../automation/automation_action_id.enum";
import { AutomationAction, InitResult } from "../automation/types";
import { InputText } from "../../ts/components/input_text";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import * as API from "../../ts/api";
import * as util from "../../ts/util";

export type MqttAutomationAction = [
    AutomationActionID.MQTT,
    {
        topic: string;
        payload: string;
        retain: boolean;
        use_prefix: boolean;
    },
];

function get_mqtt_table_children(action: MqttAutomationAction) {
    const mqtt_config = API.get("mqtt/config");
    const topic = action[1].use_prefix ? mqtt_config.global_topic_prefix + "/automation_action/" + action[1].topic : action[1].topic;

    return __("mqtt.automation.automation_action_text")(topic, action[1].payload, action[1].retain);
}

function get_mqtt_edit_children(action: MqttAutomationAction, on_action: (action: AutomationAction) => void) {
    const mqtt_config = API.get("mqtt/config");
    const [isInvalid, isInvalidSetter] = useState(false);

    return [
        <FormRow label={__("mqtt.automation.use_topic_prefix")}>
            <Switch
                checked={action[1].use_prefix}
                onClick={() => {
                    on_action(util.get_updated_union(action, {use_prefix: !action[1].use_prefix}));
                }}
                desc={__("mqtt.automation.use_topic_prefix_muted") + mqtt_config.global_topic_prefix + "/automation_action/"}/>
        </FormRow>,
        <FormRow label={__("mqtt.automation.send_topic")}>
             <InputText
                required
                value={action[1].topic}
                class={isInvalid ? "is-invalid" : undefined}
                maxLength={32}
                onValue={(v) => {
                    if (v.startsWith(mqtt_config.global_topic_prefix)) {
                        isInvalidSetter(true);
                    } else {
                        isInvalidSetter(false);
                    }

                    on_action(util.get_updated_union(action, {topic: v}));
                }}
                invalidFeedback={__("mqtt.automation.use_topic_prefix_invalid")} />
        </FormRow>,
        <FormRow label={__("mqtt.automation.send_payload")}>
            <InputText
                required={!action[1].retain}
                placeholder={!action[1].retain ? "" : __("mqtt.automation.delete_reatianed_message")}
                maxLength={32}
                value={action[1].payload}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {payload: v}));
                }} />
        </FormRow>,
        <FormRow label={__("mqtt.automation.retain")}>
            <Switch
                checked={action[1].retain}
                onClick={() => {
                    on_action(util.get_updated_union(action, {retain: !action[1].retain}));
                }} />
        </FormRow>,
    ];
}

function new_mqtt_config(): AutomationAction {
    return [
        AutomationActionID.MQTT,
        {
            topic: "",
            payload: "",
            retain: false,
            use_prefix: false,
        },
    ];
}

export function init(): InitResult {
    return {
        action_components: {
            [AutomationActionID.MQTT]: {
                name: __("mqtt.automation.mqtt"),
                new_config: new_mqtt_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_edit_children: get_mqtt_edit_children,
                get_table_children: get_mqtt_table_children,
            },
        },
    };
}
