import { CronActionID } from "../cron/cron_defs";

export type MqttCronAction = [
    CronActionID.MQTT,
    {
        topic: string,
        payload: string,
        retain: boolean,
        use_prefix: boolean
    }
];

import { Cron } from "../cron/main";
import { CronComponent, CronAction, cron_action_components } from "../cron/api";
import { InputText } from "../../ts/components/input_text";
import { h } from "preact"
import { Switch } from "../../ts/components/switch";
import { __ } from "../../ts/translation";
import * as API from "../../ts/api"
import { useState } from "preact/hooks";

export function MqttCronActionComponent(action: CronAction): CronComponent {
    const value = (action as MqttCronAction)[1];
    const mqtt_config = API.get("mqtt/config");
    const topic = value.use_prefix ? mqtt_config.global_topic_prefix + "/cron_action/" + value.topic : value.topic;

    const fieldNames = [
        __("mqtt.content.topic"),
        __("mqtt.content.payload"),
        __("mqtt.content.accept_retain")
    ];
    const fieldValues = [
        topic,
        value.payload,
        value.retain ? __("mqtt.content.yes") : __("mqtt.content.no")
    ]
    let ret = "";
    fieldNames.map((name, idx) => {
        ret += name + ": \"" + fieldValues[idx] + "\"" + (idx != fieldNames.length - 1 ? ", " : "");
    });
    return {
        text: ret,
        fieldNames: fieldNames,
        fieldValues: fieldValues
    };
}

export function MqttCronActionConfig(cron: Cron, action: CronAction) {
    let value = (action as MqttCronAction)[1];
    const mqtt_config = API.get("mqtt/config");
    const [isInvalid, isInvalidSetter] = useState(false);

    return [
        {
            name: __("mqtt.content.use_topic_prefix"),
            value: <Switch
                checked={value.use_prefix}
                onClick={() => {
                    value.use_prefix = !value.use_prefix;
                    cron.setActionFromComponent(action);
                }}
                desc={__("mqtt.content.use_topic_prefix_muted") + mqtt_config.global_topic_prefix}/>
        },
        {
            name: __("mqtt.content.topic"),
            value: <InputText
                value={value.topic}
                class={isInvalid ? "is-invalid" : undefined}
                prefixChildren={
                    value.use_prefix ? <InputText value={mqtt_config.global_topic_prefix + "/cron_action/"}></InputText> : undefined
                }
                onValue={(v) => {
                    value.topic = v;
                    if (value.topic.startsWith(mqtt_config.global_topic_prefix)) {
                        isInvalidSetter(true);
                    } else {
                        isInvalidSetter(false);
                    }
                    cron.setActionFromComponent(action);
                }}
                invalidFeedback={__("mqtt.content.use_topic_prefix_invalid")}
                />
        },
        {
            name: __("mqtt.content.payload"),
            value: <InputText
                value={value.payload}
                onValue={(v) => {
                    value.payload = v;
                    cron.setActionFromComponent(action);
                }}/>
        },
        {
            name: __("mqtt.content.accept_retain"),
            value: <Switch
                checked={value.retain}
                onClick={() => {
                    value.retain = !value.retain;
                    cron.setActionFromComponent(action);
                }}/>
        }
    ]
}

function MqttCronActionFactory(): CronAction {
    return [
        CronActionID.MQTT,
        {
            topic: "",
            payload: "",
            retain: false,
            use_prefix: false
        }
    ]
}

export function init() {
    cron_action_components[CronActionID.MQTT] = {
        config_builder: MqttCronActionFactory,
        config_component: MqttCronActionConfig,
        table_row: MqttCronActionComponent,
        name: __("mqtt.content.mqtt")
    };
}
