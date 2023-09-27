import { CronTriggerID } from "../cron/cron_defs";

export type MqttCronTrigger = [
    CronTriggerID.MQTT,
    {
        topic: string,
        payload: string,
        retain: boolean,
        use_prefix: boolean
    }
];

import { h } from "preact"
import { __ } from "../../ts/translation";
import { CronComponent, CronTrigger, cron_trigger_components } from "../cron/api";
import { Cron } from "../cron/main";
import { InputText } from "../../ts/components/input_text";
import { Switch } from "../../ts/components/switch";
import * as API from "../../ts/api"
import { useState } from "preact/hooks";

export function MqttCronTriggerComponent(trigger: CronTrigger): CronComponent {
    const value = (trigger as MqttCronTrigger)[1];
    const mqtt_config = API.get("mqtt/config");

    const topic = value.use_prefix ? mqtt_config.global_topic_prefix + "/cron_trigger/" + value.topic : value.topic;

    let ret = __("mqtt.content.topic") + ": \"" + topic + "\",\n";
    ret += __("mqtt.content.payload") + ": \"" + value.payload + "\",\n";
    ret += __("mqtt.content.retain") + ": " + (value.retain ? __("mqtt.content.yes") : __("mqtt.content.no"));
    return {
        text: ret,
        fieldNames: [
            __("mqtt.content.topic"),
            __("mqtt.content.payload"),
            __("mqtt.content.retain")
        ],
        fieldValues: [
            value.topic,
            value.payload,
            value.retain ? __("mqtt.content.yes") : __("mqtt.content.no")
        ]
    };
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
            value: <InputText
                value={value.topic}
                class={isInvalid ? "is-invalid" : undefined}
                prefixChildren={
                    value.use_prefix ? <InputText value={mqtt_config.global_topic_prefix + "/cron_trigger/"}></InputText> : undefined
                }
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
        },
        {
            name: __("mqtt.content.payload"),
            value: <InputText
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
            use_prefix: false
        }
    ]
}

export function init() {
    cron_trigger_components[CronTriggerID.MQTT] = {
        table_row: MqttCronTriggerComponent,
        config_builder: MqttCronTriggerFactory,
        config_component: MqttCronTriggerConfig,
        name: __("mqtt.content.mqtt")
    };
}
