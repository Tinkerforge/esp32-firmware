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
    let ret = __("mqtt.content.topic") + ": \"" + props.topic + "\"\n";
    ret += __("mqtt.content.payload") + ": \"" + props.payload + "\"\n";
    ret += __("mqtt.content.retain") + ": " + props.retain;
    return ret
}

export function MqttCronTriggerConfig(cron_object: Cron, state: cron_trigger) {
    let props = state as any as MqttCronTrigger;
    if (props[1] === undefined) {
        props = MqttCronTriggerFactory() as any;
    }
    return [
        {
            name: __("mqtt.content.topic"),
            value: <InputText
                value={props[1].topic}
                onValue={(v) => {
                    props[1].topic = v;
                    cron_object.setTriggerFromComponent(props as any as cron_trigger);
                }}/>
        },
        {
            name: __("mqtt.content.payload"),
            value: <InputText
                value={props[1].payload}
                onValue={(v) => {
                    props[1].payload = v;
                    cron_object.setTriggerFromComponent(props as any as cron_trigger);
                }}/>
        },
        {
            name: __("mqtt.content.retain"),
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
