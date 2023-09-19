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
import { CronComponent, CronTrigger } from "../cron/api";
import { Cron } from "../cron/main";
import { InputText } from "../../ts/components/input_text";
import { InputSelect } from "../../ts/components/input_select";
import { ListGroup, ListGroupItem } from "react-bootstrap";
import * as API from "../../ts/api";
import * as util from "../../ts/util";

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
    const value = (trigger as NfcCronTrigger)[1];
    const tags = API.get("nfc/seen_tags");
    const known_tags = API.get("nfc/config").authorized_tags;
    const seen_tags = tags.filter(t => t.tag_id != "" && !known_tags.find(tag => t.tag_id == tag.tag_id)).map(t => <ListGroupItem action type="button" onClick={() => {
        if (t.tag_id != "") {
            value.tag_id = t.tag_id;
            value.tag_type = t.tag_type;
            cron.setTriggerFromComponent(trigger);
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
            cron.setTriggerFromComponent(trigger);
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
    return {
        trigger_components: {
            [CronTriggerID.NFC]: {
                config_builder: NfcCronTriggerFactory,
                config_component: NFCCronTriggerConfig,
                table_row: NFCCronTriggerComponent,
                name: __("nfc.content.nfc")
            }
        }
    };
}
