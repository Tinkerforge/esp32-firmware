/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

import { h, Fragment } from "preact";
import { __, translate_unchecked } from "../../ts/translation";
import { CronTriggerID } from "../cron/cron_defs";
import { CronTrigger } from "../cron/types";
import { InputText } from "../../ts/components/input_text";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { ListGroup, ListGroupItem } from "react-bootstrap";
import * as API from "../../ts/api";
import * as util from "../../ts/util";

export type NfcCronTrigger = [
    CronTriggerID.NFC,
    {
        tag_type: number;
        tag_id: string;
    },
];

function new_nfc_config(): CronTrigger {
    return [
        CronTriggerID.NFC,
        {
            tag_type: 0,
            tag_id: "",
        },
    ];
}

function get_nfc_table_children(trigger: NfcCronTrigger) {
    return __("nfc.cron.cron_trigger_text")(trigger[1].tag_id, translate_unchecked("nfc.cron.type_" + trigger[1].tag_type))
}

function get_nfc_edit_children(trigger: NfcCronTrigger, on_trigger: (trigger: CronTrigger) => void) {
    const tags = API.get("nfc/seen_tags");
    const known_tags = API.get("nfc/config").authorized_tags;
    const seen_tags = tags.filter(t => t.tag_id != "" && !known_tags.find(tag => t.tag_id == tag.tag_id)).map(t => <ListGroupItem action type="button" onClick={() => {
        if (t.tag_id != "") {
            on_trigger(util.get_updated_union(trigger, {tag_id: t.tag_id, tag_type: t.tag_type}));
        }
        }}>
            <h5 class="mb-1 pr-2">{t.tag_id}</h5>
            <div class="d-flex w-100 justify-content-between">
                <span class="text-left">{translate_unchecked(`nfc.cron.type_${t.tag_type}`)}</span>
                <span class="text-right">{__("nfc.cron.last_seen") + util.format_timespan_ms(t.last_seen) + __("nfc.cron.last_seen_suffix")}</span>
            </div>
        </ListGroupItem>);

    const users = API.get("users/config").users;
    const known_items = API.get("nfc/config").authorized_tags.map(t => <ListGroupItem action type="button" onClick={() => {
        if (t.tag_id != "") {
            on_trigger(util.get_updated_union(trigger, {tag_id: t.tag_id, tag_type: t.tag_type}));
        }
        }}>
            <h5 class="mb-1 pr-2">{t.tag_id}</h5>
            <div class="d-flex w-100 justify-content-between">
                <span class="text-left">{translate_unchecked(`nfc.cron.type_${t.tag_type}`)}</span>
                <span class="text-right">{__("nfc.cron.table_user_id") + ": " + users.find(u => u.id == t.user_id).display_name}</span>
            </div>
        </ListGroupItem>);

    const all_tags = known_items.concat(seen_tags);

    return [<>
        <FormRow label={__("nfc.cron.last_seen_and_known_tags")}>
            {all_tags.length > 0 ?
                <ListGroup>{all_tags}</ListGroup>
                : <span>{__("nfc.cron.add_tag_description")}</span>}
        </FormRow>
        <FormRow label={__("nfc.cron.table_tag_id")}>
            <InputText
                required
                value={trigger[1].tag_id}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {tag_id: v}));
                }}
                minLength={8}
                maxLength={29}
                pattern="^([0-9a-fA-F]{2}:?){3,9}[0-9a-fA-F]{2}$"
                invalidFeedback={__("nfc.cron.tag_id_invalid_feedback")} />
        </FormRow>
        <FormRow label={__("nfc.cron.table_tag_type")}>
            <InputSelect
                items={[
                    ["0",__("nfc.cron.type_0")],
                    ["1",__("nfc.cron.type_1")],
                    ["2",__("nfc.cron.type_2")],
                    ["3",__("nfc.cron.type_3")],
                    ["4",__("nfc.cron.type_4")],
                ]}
                value={trigger[1].tag_type.toString()}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {tag_type: parseInt(v)}));
                }} />
        </FormRow>
    </>]
}

export function init() {
    return {
        trigger_components: {
            [CronTriggerID.NFC]: {
                name: __("nfc.cron.cron_trigger_nfc"),
                new_config: new_nfc_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_edit_children: get_nfc_edit_children,
                get_table_children: get_nfc_table_children,
            },
        },
    };
}
