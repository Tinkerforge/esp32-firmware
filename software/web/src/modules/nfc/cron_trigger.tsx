export interface NfcCronTrigger {
    0: 5,
    1: {
        tag_type: number,
        tag_id: string
    }
}

import { h } from "preact"
import { __, translate_unchecked } from "src/ts/translation";
import { CronComponent, cron_trigger, cron_trigger_components } from "../cron/api";
import { Cron } from "../cron/main";
import { InputText } from "src/ts/components/input_text";
import { InputSelect } from "src/ts/components/input_select";

export function NFCCronTriggerComponent(cron: cron_trigger): CronComponent {
    const props = (cron as any as NfcCronTrigger)[1];

    let ret = __("nfc.content.table_tag_id") + ": \"" + props.tag_id + "\",\n";
    ret += __("nfc.content.table_tag_type") + ": " + translate_unchecked("nfc.content.type_" + props.tag_type);
    return {
        text: ret,
        fieldNames: [
            __("nfc.content.table_tag_id"),
            __("nfc.content.table_tag_type")
        ],
        fieldValues: [
            props.tag_id,
            translate_unchecked("nfc.content.type_" + props.tag_type)
        ]
    }
}

function NfcCronTriggerFactory(): cron_trigger{
    return [
        5 as any,
        {
            tag_type: 0,
            tag_id: ""
        }
    ]
}

export function NFCCronTriggerConfig(cron_object: Cron, props: cron_trigger) {
    let state = props as any as NfcCronTrigger;
    if (state[1] === undefined) {
        state = NfcCronTriggerFactory() as any;
    }
    return [
        {
            name: __("nfc.content.table_tag_id"),
            value: <InputText
                value={state[1].tag_id}
                onValue={(v) => {
                    state[1].tag_id = v;
                    cron_object.setTriggerFromComponent(state as any);
                }}/>
        },
        {
            name: __("nfc.content.table_tag_type"),
            value: <InputSelect
                items={[
                    ["0",__("nfc.content.type_0")],
                    ["1",__("nfc.content.type_1")],
                    ["2",__("nfc.content.type_2")],
                    ["3",__("nfc.content.type_3")],
                    ["4",__("nfc.content.type_4")],
                ]}
                value={state[1].tag_type.toString()}
                onValue={(v) => {
                    state[1].tag_type = parseInt(v);
                    cron_object.setTriggerFromComponent(state as any);
                }}/>
        }
    ]
}

cron_trigger_components[5] = {
    config_builder: NfcCronTriggerFactory,
    config_component: NFCCronTriggerConfig,
    table_row: NFCCronTriggerComponent,
    name: __("nfc.content.nfc")
};
