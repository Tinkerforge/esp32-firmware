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
import { CronTriggerID } from "../cron/cron_defs";
import { CronComponent, CronTrigger } from "../cron/types";
import { Cron } from "../cron/main";
import { InputText } from "../../ts/components/input_text";
import { Switch } from "../../ts/components/switch";
import * as API from "../../ts/api";

export type MqttCronTrigger = [
    CronTriggerID.MQTT,
    {
        topic: string,
        payload: string,
        retain: boolean,
        use_prefix: boolean
    }
];

export function MqttCronTriggerComponent(trigger: CronTrigger): VNode {
    const value = (trigger as MqttCronTrigger)[1];
    const mqtt_config = API.get("mqtt/config");

    const topic = value.use_prefix ? mqtt_config.global_topic_prefix + "/cron_trigger/" + value.topic : value.topic;

    return __("mqtt.content.cron_trigger_text")(topic, value.payload, value.retain);
}

export function MqttCronTriggerConfig(cron: Cron, trigger: CronTrigger) {
    const value = (trigger as MqttCronTrigger)[1];
    const mqtt_config = API.get("mqtt/config");
    const [isInvalid, isInvalidSetter] = useState(false);

    return [
        {
            name: __("mqtt.content.use_topic_prefix"),
            value: <Switch
                checked={value.use_prefix}
                onClick={() => {
                    value.use_prefix = !value.use_prefix;
                    cron.setTriggerFromComponent(trigger);
                }}
                desc={__("mqtt.content.use_topic_prefix_muted") + mqtt_config.global_topic_prefix + "/cron_trigger/"}/>
        },
        {
            name: __("mqtt.content.topic"),
            value: <>
             <InputText
                required
                maxLength={64}
                value={value.topic}
                class={isInvalid ? "is-invalid" : undefined}
                onValue={(v) => {
                    value.topic = v;
                    if (value.topic.startsWith(mqtt_config.global_topic_prefix)) {
                        isInvalidSetter(true);
                    } else {
                        isInvalidSetter(false);
                    }
                    cron.setTriggerFromComponent(trigger);
                }}
                invalidFeedback={__("mqtt.content.use_topic_prefix_invalid")}/>
                <InputText
                    class="mt-2"
                    value={mqtt_config.global_topic_prefix + "/cron_trigger/" + value.topic}
                    hidden={!value.use_prefix} />
            </>
        },
        {
            name: __("mqtt.content.payload"),
            value: <InputText
                required
                maxLength={64}
                value={value.payload}
                onValue={(v) => {
                    value.payload = v;
                    cron.setTriggerFromComponent(trigger);
                }}/>
        },
        {
            name: __("mqtt.content.retain"),
            value: <Switch
                checked={value.retain}
                onClick={() => {
                    value.retain = !value.retain;
                    cron.setTriggerFromComponent(trigger);
                }}/>
        }
    ]
}

function MqttCronTriggerFactory(): CronTrigger {
    return [
        CronTriggerID.MQTT,
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
        trigger_components: {
            [CronTriggerID.MQTT]: {
                clone: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                table_row: MqttCronTriggerComponent,
                config_builder: MqttCronTriggerFactory,
                config_component: MqttCronTriggerConfig,
                name: __("mqtt.content.mqtt"),
            },
        },
    };
}
