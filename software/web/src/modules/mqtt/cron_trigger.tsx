export interface MqttCronTrigger {
    0: 3,
    1: {
        topic: string,
        payload: string,
        retain: boolean
    }
}

import { h } from "preact"
import { __ } from "src/ts/translation";
import { cron_trigger,cron_trigger_components } from "../cron/api";
import { Cron } from "../cron/main";
import { InputText } from "src/ts/components/input_text";
import { Switch } from "src/ts/components/switch";

export function MqttCronTriggerComponent(cron: cron_trigger) {
    const props = (cron as any as MqttCronTrigger)[1];
    let ret = "Topic: \"" + props.topic + "\"\n";
    ret += "Payload: \"" + props.payload + "\"\n";
    ret += "Retain: " + props.retain;
    return ret
}

export function MqttCronTriggerConfig(cron_object: Cron, state: cron_trigger) {
    let props = state as any as MqttCronTrigger;
    if (props[1] === undefined) {
        props = MqttCronTriggerFactory() as any;
    }
    return [
        {
            name: "Topic",
            value: <InputText
                value={props[1].topic}
                onValue={(v) => {
                    props[1].topic = v;
                    cron_object.setTriggerFromComponent(props as any as cron_trigger);
                }}/>
        },
        {
            name: "Payload",
            value: <InputText
                value={props[1].payload}
                onValue={(v) => {
                    props[1].payload = v;
                    cron_object.setTriggerFromComponent(props as any as cron_trigger);
                }}/>
        },
        {
            name: "Retain",
            value: <Switch
                checked={props[1].retain}
                onClick={() => {
                    props[1].retain = !props[1].retain;
                    cron_object.setTriggerFromComponent(props as any as cron_trigger);
                }}/>
        }
    ]
}

function MqttCronTriggerFactory(): cron_trigger {
    return [
        3 as any,
        {
            topic: "",
            payload: "",
            retain: false
        }
    ]
}

cron_trigger_components[3] = {
    table_row: MqttCronTriggerComponent,
    config_builder: MqttCronTriggerFactory,
    config_component: MqttCronTriggerConfig,
    name: __("mqtt.content.mqtt")
};
