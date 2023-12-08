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
import { CronTrigger } from "../cron/types";
import { InputText } from "../../ts/components/input_text";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import * as API from "../../ts/api";
import * as util from "../../ts/util";

export type MqttCronTrigger = [
    CronTriggerID.MQTT,
    {
        topic: string;
        payload: string;
        retain: boolean;
        use_prefix: boolean;
    },
];

function get_mqtt_table_children(trigger: MqttCronTrigger) {
    const mqtt_config = API.get("mqtt/config");
    const topic = trigger[1].use_prefix ? mqtt_config.global_topic_prefix + "/cron_trigger/" + trigger[1].topic : trigger[1].topic;

    return __("mqtt.cron.cron_trigger_text")(topic, trigger[1].payload, trigger[1].retain);
}

function get_mqtt_edit_children(trigger: MqttCronTrigger, on_trigger: (trigger: CronTrigger) => void) {
    const mqtt_config = API.get("mqtt/config");
    const [isInvalid, isInvalidSetter] = useState(false);

    return [<>
        <FormRow label={__("mqtt.cron.use_topic_prefix")}>
            <Switch
                checked={trigger[1].use_prefix}
                onClick={() => {
                    on_trigger(util.get_updated_union(trigger, {use_prefix: !trigger[1].use_prefix}));
                }}
                desc={__("mqtt.cron.use_topic_prefix_muted") + mqtt_config.global_topic_prefix + "/cron_trigger/"} />
        </FormRow>
        <FormRow label={__("mqtt.cron.topic")}>
            <InputText
                required
                value={trigger[1].topic}
                class={isInvalid ? "is-invalid" : undefined}
                maxLength={64}
                onValue={(v) => {
                    if (v.startsWith(mqtt_config.global_topic_prefix)) {
                        isInvalidSetter(true);
                    } else {
                        isInvalidSetter(false);
                    }

                    on_trigger(util.get_updated_union(trigger, {topic: v}));
                }}
                invalidFeedback={__("mqtt.cron.use_topic_prefix_invalid")} />
        </FormRow>
        <FormRow label={__("mqtt.cron.full_topic")} hidden={!trigger[1].use_prefix}>
            <InputText
                class="mt-2"
                value={mqtt_config.global_topic_prefix + "/cron_trigger/" + trigger[1].topic} />
        </FormRow>
        <FormRow label={__("mqtt.cron.payload")}>
            <InputText
                placeholder={__("mqtt.cron.match_all")}
                maxLength={64}
                value={trigger[1].payload}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {payload: v}));
                }} />
        </FormRow>
        <FormRow label={__("mqtt.cron.accept_retain")}>
            <Switch
                checked={trigger[1].retain}
                onClick={() => {
                    on_trigger(util.get_updated_union(trigger, {retain: !trigger[1].retain}));
                }} />
        </FormRow>
    </>]
}

function new_mqtt_config(): CronTrigger {
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
                name: __("mqtt.cron.cron_trigger_mqtt"),
                new_config: new_mqtt_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_edit_children: get_mqtt_edit_children,
                get_table_children: get_mqtt_table_children,
            },
        },
    };
}
