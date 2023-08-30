export interface MqttCronAction {
    0: 2,
    1: {
        topic: string,
        payload: string,
        retain: boolean
    }
}

import { Cron } from "../cron/main";
import { cron_action, cron_action_components } from "../cron/api";
import { InputText } from "src/ts/components/input_text";
import { h } from "preact"
import { Switch } from "src/ts/components/switch";
import { __ } from "src/ts/translation";

export function MqttCronActionComponent(cron: cron_action) {
    const props = (cron as any as MqttCronAction)[1];
    let ret = __("mqtt.content.topic") + ": \"" + props.topic + "\"\n";
    ret += __("mqtt.content.payload") + ": \"" + props.payload + "\"\n";
    ret += __("mqtt.content.accept_retain") + ": " + props.retain;
    return ret;
}

export function MqttCronActionConfig(cron_object: Cron, state: cron_action) {
    let props = state as any as MqttCronAction;
    if (props[1] === undefined) {
        props = MqttCronActionFactory() as any;
    }
    return [
        {
            name: __("mqtt.content.topic"),
            value: <InputText
                value={props[1].topic}
                onValue={(v) => {
                    props[1].topic = v;
                    cron_object.setActionFromComponent(props as any);
                }}/>
        },
        {
            name: __("mqtt.content.payload"),
            value: <InputText
                value={props[1].payload}
                onValue={(v) => {
                    props[1].payload = v;
                    cron_object.setActionFromComponent(props as any);
                }}/>
        },
        {
            name: __("mqtt.content.accept_retain"),
            value: <Switch
                checked={props[1].retain}
                onClick={() => {
                    props[1].retain = !props[1].retain;
                    cron_object.setActionFromComponent(props as any);
                }}/>
        }
    ]
}

function MqttCronActionFactory(): cron_action {
    return [
        2 as any,
        {
            topic: "",
            payload: "",
            retain: false
        }
    ]
}

cron_action_components[2] = {
    config_builder: MqttCronActionFactory,
    config_component: MqttCronActionConfig,
    table_row: MqttCronActionComponent,
    name: __("mqtt.content.mqtt")
};
