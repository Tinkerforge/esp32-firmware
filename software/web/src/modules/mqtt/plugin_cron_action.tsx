import { CronActionID } from "../cron/cron_defs";

export type MqttCronAction = [
    CronActionID.MQTT,
    {
        topic: string,
        payload: string,
        retain: boolean
    }
];

import { Cron } from "../cron/main";
import { CronComponent, CronAction, cron_action_components } from "../cron/api";
import { InputText } from "../../ts/components/input_text";
import { h } from "preact"
import { Switch } from "../../ts/components/switch";
import { __ } from "../../ts/translation";

export function MqttCronActionComponent(action: CronAction): CronComponent {
    const value = (action as MqttCronAction)[1];
    const fieldNames = [
        __("mqtt.content.topic"),
        __("mqtt.content.payload"),
        __("mqtt.content.accept_retain")
    ];
    const fieldValues = [
        value.topic,
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
    return [
        {
            name: __("mqtt.content.topic"),
            value: <InputText
                value={value.topic}
                onValue={(v) => {
                    value.topic = v;
                    cron.setActionFromComponent(action);
                }}/>
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
            retain: false
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
