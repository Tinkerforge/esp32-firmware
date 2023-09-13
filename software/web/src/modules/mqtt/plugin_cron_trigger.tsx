import { CronTriggerID } from "../cron/cron_defs";

export type MqttCronTrigger = [
    CronTriggerID.MQTT,
    {
        topic: string,
        payload: string,
        retain: boolean
    }
];

import { h } from "preact"
import { __ } from "../../ts/translation";
import { CronComponent, CronTrigger, cron_trigger_components } from "../cron/api";
import { Cron } from "../cron/main";
import { InputText } from "../../ts/components/input_text";
import { Switch } from "../../ts/components/switch";

export function MqttCronTriggerComponent(trigger: CronTrigger): CronComponent {
    const value = (trigger as MqttCronTrigger)[1];
    let ret = __("mqtt.content.topic") + ": \"" + value.topic + "\",\n";
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
    let value = (trigger as MqttCronTrigger)[1];
    return [
        {
            name: __("mqtt.content.topic"),
            value: <InputText
                value={value.topic}
                onValue={(v) => {
                    value.topic = v;
                    cron.setTriggerFromComponent(trigger);
                }}/>
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
            retain: false
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
