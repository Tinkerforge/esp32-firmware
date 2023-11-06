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
import { CronActionID } from "../cron/cron_defs";
import { Cron } from "../cron/main";
import { CronAction } from "../cron/types";
import { InputText } from "../../ts/components/input_text";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import * as API from "../../ts/api";

export type MqttCronAction = [
    CronActionID.MQTT,
    {
        topic: string,
        payload: string,
        retain: boolean,
        use_prefix: boolean
    }
];

function get_mqtt_table_children(action: CronAction) {
    const value = (action as MqttCronAction)[1];
    const mqtt_config = API.get("mqtt/config");
    const topic = value.use_prefix ? mqtt_config.global_topic_prefix + "/cron_action/" + value.topic : value.topic;

    return __("mqtt.cron.cron_action_text")(topic, value.payload, value.retain);
}

function get_mqtt_edit_children(cron: Cron, action: CronAction) {
    let value = (action as MqttCronAction)[1];
    const mqtt_config = API.get("mqtt/config");
    const [isInvalid, isInvalidSetter] = useState(false);

    return [<>
        <FormRow label={__("mqtt.cron.use_topic_prefix")}>
            <Switch
                checked={value.use_prefix}
                onClick={() => {
                    value.use_prefix = !value.use_prefix;
                    cron.setActionFromComponent(action);
                }}
                desc={__("mqtt.cron.use_topic_prefix_muted") + mqtt_config.global_topic_prefix}/>
        </FormRow>
        <FormRow label={__("mqtt.cron.topic")}>
             <InputText
                required
                value={value.topic}
                class={isInvalid ? "is-invalid" : undefined}
                maxLength={64}
                onValue={(v) => {
                    value.topic = v;
                    if (value.topic.startsWith(mqtt_config.global_topic_prefix)) {
                        isInvalidSetter(true);
                    } else {
                        isInvalidSetter(false);
                    }
                    cron.setActionFromComponent(action);
                }}
                invalidFeedback={__("mqtt.cron.use_topic_prefix_invalid")} />
            <InputText
                class="mt-2"
                value={mqtt_config.global_topic_prefix + "/cron_action/" + value.topic}
                hidden={!value.use_prefix} />
        </FormRow>
        <FormRow label={__("mqtt.cron.payload")}>
            <InputText
                required
                maxLength={64}
                value={value.payload}
                onValue={(v) => {
                    value.payload = v;
                    cron.setActionFromComponent(action);
                }} />
        </FormRow>
        <FormRow label={__("mqtt.cron.retain")}>
            <Switch
                checked={value.retain}
                onClick={() => {
                    value.retain = !value.retain;
                    cron.setActionFromComponent(action);
                }} />
        </FormRow>
    </>]
}

function new_mqtt_config(): CronAction {
    return [
        CronActionID.MQTT,
        {
            topic: "",
            payload: "",
            retain: false,
            use_prefix: false,
        },
    ];
}

export function init() {
    return {
        action_components: {
            [CronActionID.MQTT]: {
                name: __("mqtt.cron.mqtt"),
                new_config: new_mqtt_config,
                clone_config: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                get_edit_children: get_mqtt_edit_children,
                get_table_children: get_mqtt_table_children,
            },
        },
    };
}
