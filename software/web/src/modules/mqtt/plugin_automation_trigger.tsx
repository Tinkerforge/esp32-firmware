/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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
import { AutomationTriggerID } from "../automation/automation_trigger_id.enum";
import { AutomationTrigger, InitResult } from "../automation/types";
import { InputText } from "../../ts/components/input_text";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import * as API from "../../ts/api";
import * as util from "../../ts/util";

export type MqttAutomationTrigger = [
    AutomationTriggerID.MQTT,
    {
        topic_filter: string;
        payload: string;
        retain: boolean;
        use_prefix: boolean;
    },
];

function get_mqtt_table_children(trigger: MqttAutomationTrigger) {
    const mqtt_config = API.get("mqtt/config");
    const topic = trigger[1].use_prefix ? mqtt_config.global_topic_prefix + "/automation_trigger/" + trigger[1].topic_filter : trigger[1].topic_filter;

    return __("mqtt.automation.automation_trigger_text")(topic, trigger[1].payload, trigger[1].retain);
}

function get_mqtt_edit_children(trigger: MqttAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    const mqtt_config = API.get("mqtt/config");
    const [isInvalid, isInvalidSetter] = useState(false);

    return [
        <FormRow label={__("mqtt.automation.use_topic_prefix")}>
            <Switch
                checked={trigger[1].use_prefix}
                onClick={() => {
                    on_trigger(util.get_updated_union(trigger, {use_prefix: !trigger[1].use_prefix}));
                }}
                desc={__("mqtt.automation.use_topic_prefix_muted") + mqtt_config.global_topic_prefix + "/automation_trigger/"} />
        </FormRow>,
        <FormRow label={__("mqtt.automation.topic")}>
            <InputText
                required
                value={trigger[1].topic_filter}
                class={isInvalid ? "is-invalid" : undefined}
                maxLength={32}
                onValue={(v) => {
                    if (v.startsWith(mqtt_config.global_topic_prefix)) {
                        isInvalidSetter(true);
                    } else {
                        isInvalidSetter(false);
                    }

                    on_trigger(util.get_updated_union(trigger, {topic_filter: v}));
                }}
                invalidFeedback={__("mqtt.automation.use_topic_prefix_invalid")} />
        </FormRow>,
        <FormRow label={__("mqtt.automation.payload")}>
            <InputText
                placeholder={__("mqtt.automation.match_any")}
                maxLength={32}
                value={trigger[1].payload}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {payload: v}));
                }} />
        </FormRow>,
        <FormRow label={__("mqtt.automation.accept_retain")}>
            <Switch
                checked={trigger[1].retain}
                onClick={() => {
                    on_trigger(util.get_updated_union(trigger, {retain: !trigger[1].retain}));
                }} />
        </FormRow>,
    ];
}

function new_mqtt_config(): AutomationTrigger {
    return [
        AutomationTriggerID.MQTT,
        {
            topic_filter: "",
            payload: "",
            retain: false,
            use_prefix: false,
        },
    ];
}

export function init(): InitResult {
    return {
        trigger_components: {
            [AutomationTriggerID.MQTT]: {
                translation_name: () => __("mqtt.automation.automation_trigger_mqtt"),
                new_config: new_mqtt_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_edit_children: get_mqtt_edit_children,
                get_table_children: get_mqtt_table_children,
            },
        },
    };
}
