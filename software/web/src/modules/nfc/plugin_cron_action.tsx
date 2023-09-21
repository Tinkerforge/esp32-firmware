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
import { CronComponent, CronAction } from "../cron/types";
import { Cron } from "../cron/main";
import { InputText } from "../../ts/components/input_text";
import { InputSelect } from "../../ts/components/input_select";
import { ListGroup, ListGroupItem } from "react-bootstrap";
import * as API from "../../ts/api";
import * as util from "../../ts/util";

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
    const tags = API.get("nfc/seen_tags");
    const known_tags = API.get("nfc/config").authorized_tags;
    const seen_tags = tags.filter(t => t.tag_id != "" && !known_tags.find(tag => t.tag_id == tag.tag_id)).map(t => <ListGroupItem action type="button" onClick={() => {
        if (t.tag_id != "") {
            value.tag_id = t.tag_id;
            value.tag_type = t.tag_type;
            cron.setActionFromComponent(action);
        }
        }}>
            <h5 class="mb-1 pr-2">{t.tag_id}</h5>
            <div class="d-flex w-100 justify-content-between">
                <span>{translate_unchecked(`nfc.content.type_${t.tag_type}`)}</span>
                <span>{__("nfc.content.last_seen") + util.format_timespan(Math.floor(t.last_seen / 1000)) + __("nfc.content.last_seen_suffix")}</span>
            </div>
        </ListGroupItem>);

    const users = API.get("users/config").users;
    const known_items = API.get("nfc/config").authorized_tags.map(t => <ListGroupItem action type="button" onClick={() => {
        if (t.tag_id != "") {
            value.tag_id = t.tag_id;
            value.tag_type = t.tag_type;
            cron.setActionFromComponent(action);
        }
        }}>
            <h5 class="mb-1 pr-2">{t.tag_id}</h5>
            <div class="d-flex w-100 justify-content-between">
                <span>{translate_unchecked(`nfc.content.type_${t.tag_type}`)}</span>
                <span>{__("nfc.content.table_user_id") + ": " + users.find(u => u.id == t.user_id).display_name}</span>
            </div>
        </ListGroupItem>);

    const all_tags = known_items.concat(seen_tags);

    return [
        {
            name: __("nfc.content.last_seen_and_known_tags"),
            value: all_tags.length == 0
                ? <span>{__("nfc.content.add_tag_description")}</span>
                : <ListGroup>{
                    all_tags
                }</ListGroup>
        },
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
    return {
        action_components: {
            [CronActionID.NFCInjectTag]: {
                clone: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                table_row: NFCTagInjectCronActionComponent,
                config_builder: NfCTagInjectCronActionFactory,
                config_component: NFCTagInjectCronActionConfig,
                name: __("nfc.content.nfc")
            }
        }
    };
}
