import { CronActionID } from "../cron/cron_defs";

export type NfcCronAction = [
    CronActionID.NFCInjectTag,
    {
        tag_type: number,
        tag_id: string,
        tag_action: number
    }
];

import { h } from "preact"
import { __, translate_unchecked } from "../../ts/translation";
import { CronComponent, CronAction, cron_action_components } from "../cron/api";
import { Cron } from "../cron/main";
import { InputText } from "../../ts/components/input_text";
import { InputSelect } from "../../ts/components/input_select";

const TRIGGER_CHARGE_ANY = 0;
const TRIGGER_CHARGE_START = 1;
const TRIGGER_CHARGE_STOP = 2;

function NFCTagInjectCronActionComponent(action: CronAction): CronComponent {
    const value = (action as NfcCronAction)[1];
    let ret = __("nfc.content.table_tag_id") + ": \"" + value.tag_id + "\",\n";
    ret += __("nfc.content.table_tag_type") + ": " + translate_unchecked("nfc.content.type_" + value.tag_type) + ",\n";
    ret += __("nfc.content.tag_action") + ": ";
    let tag_action = ""
    switch (value.tag_action) {
        case TRIGGER_CHARGE_ANY:
            tag_action = __("nfc.content.trigger_charge_any");
            break;

        case TRIGGER_CHARGE_START:
            tag_action = __("nfc.content.trigger_charge_start");
            break;

        case TRIGGER_CHARGE_STOP:
            tag_action = __("nfc.content.trigger_charge_stop");
    }
    return {
        text: ret + tag_action,
        fieldNames: [
            __("nfc.content.table_tag_id"),
            __("nfc.content.table_tag_type"),
            __("nfc.content.tag_action")
        ],
        fieldValues: [
            value.tag_id,
            translate_unchecked("nfc.content.type_" + value.tag_type),
            tag_action
        ]
    };
}

function NFCTagInjectCronActionConfig(cron: Cron, action: CronAction) {
    const value = (action as NfcCronAction)[1];
    return [
        {
            name: __("nfc.content.table_tag_id"),
            value: <InputText
            value={value.tag_id}
                onValue={(v) => {
                    value.tag_id = v;
                    cron.setActionFromComponent(action);
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
                    cron.setActionFromComponent(action);
                }}/>
        },
        {
            name: __("nfc.content.tag_action"),
            value: <InputSelect
                items={[
                    ["0", __("nfc.content.trigger_charge_any")],
                    ["1", __("nfc.content.trigger_charge_start")],
                    ["2", __("nfc.content.trigger_charge_stop")]
                ]}
                value={value.tag_action.toString()}
                onValue={(v) => {
                    value.tag_action = parseInt(v);
                    cron.setActionFromComponent(action)
                }}/>
        }
    ]
}

function NfCTagInjectCronActionFactory(): CronAction {
    return [
        CronActionID.NFCInjectTag,
        {
            tag_id: "",
            tag_type: 0,
            tag_action: 0
        }
    ]
}

export function init() {
    cron_action_components[CronActionID.NFCInjectTag] = {
        table_row: NFCTagInjectCronActionComponent,
        config_builder: NfCTagInjectCronActionFactory,
        config_component: NFCTagInjectCronActionConfig,
        name: __("nfc.content.nfc")
    };
}
