import { CronTriggerID } from "../cron/cron_defs";

export type NfcCronTrigger = [
    CronTriggerID.NFC,
    {
        tag_type: number,
        tag_id: string
    }
];

import { h } from "preact"
import { __, translate_unchecked } from "../../ts/translation";
import { CronComponent, CronTrigger, cron_trigger_components } from "../cron/api";
import { Cron } from "../cron/main";
import { InputText } from "../../ts/components/input_text";
import { InputSelect } from "../../ts/components/input_select";

export function NFCCronTriggerComponent(trigger: CronTrigger): CronComponent {
    const value = (trigger as NfcCronTrigger)[1];
    let ret = __("nfc.content.table_tag_id") + ": \"" + value.tag_id + "\",\n";
    ret += __("nfc.content.table_tag_type") + ": " + translate_unchecked("nfc.content.type_" + value.tag_type);
    return {
        text: ret,
        fieldNames: [
            __("nfc.content.table_tag_id"),
            __("nfc.content.table_tag_type")
        ],
        fieldValues: [
            value.tag_id,
            translate_unchecked("nfc.content.type_" + value.tag_type)
        ]
    }
}

function NfcCronTriggerFactory(): CronTrigger {
    return [
        CronTriggerID.NFC,
        {
            tag_type: 0,
            tag_id: ""
        }
    ]
}

export function NFCCronTriggerConfig(cron: Cron, trigger: CronTrigger) {
    let value = (trigger as NfcCronTrigger)[1];
    return [
        {
            name: __("nfc.content.table_tag_id"),
            value: <InputText
                value={value.tag_id}
                onValue={(v) => {
                    value.tag_id = v;
                    cron.setTriggerFromComponent(trigger);
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
                value={value.tag_type.toString()}
                onValue={(v) => {
                    value.tag_type = parseInt(v);
                    cron.setTriggerFromComponent(trigger);
                }}/>
        }
    ]
}

export function init() {
    cron_trigger_components[CronTriggerID.NFC] = {
        config_builder: NfcCronTriggerFactory,
        config_component: NFCCronTriggerConfig,
        table_row: NFCCronTriggerComponent,
        name: __("nfc.content.nfc")
    };
}
