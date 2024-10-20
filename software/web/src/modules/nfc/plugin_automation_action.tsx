/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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
import { AutomationActionID } from "../automation/automation_action_id.enum";
import { AutomationAction, InitResult } from "../automation/types";
import { InputText } from "../../ts/components/input_text";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { ListGroup, ListGroupItem } from "react-bootstrap";
import * as API from "../../ts/api";
import * as util from "../../ts/util";

export type NfcAutomationAction = [
    AutomationActionID.NFCInjectTag,
    {
        tag_type: number;
        tag_id: string;
        action: number;
    },
];

function get_nfc_inject_tag_table_children(action: NfcAutomationAction) {
    return __("nfc.automation.automation_action_text")(action[1].tag_id, translate_unchecked("nfc.automation.type_" + action[1].tag_type), action[1].action);
}

function get_nfc_inject_tag_edit_children(action: NfcAutomationAction, on_action: (action: AutomationAction) => void) {
    const tags = API.get("nfc/seen_tags");
    const known_tags = API.get("nfc/config").authorized_tags;
    const seen_tags = tags.filter(t => t.tag_id != "" && !known_tags.find(tag => t.tag_id == tag.tag_id)).map(t => <ListGroupItem action type="button" onClick={() => {
        if (t.tag_id != "") {
            on_action(util.get_updated_union(action, {tag_id: t.tag_id, tag_type: t.tag_type}));
        }
        }}>
            <h5 class="mb-1 pr-2">{t.tag_id}</h5>
            <div class="d-flex w-100 justify-content-between">
                <span class="text-left">{translate_unchecked(`nfc.automation.type_${t.tag_type}`)}</span>
                <span class="text-right">{__("nfc.automation.last_seen") + util.format_timespan_ms(t.last_seen) + __("nfc.automation.last_seen_suffix")}</span>
            </div>
        </ListGroupItem>);

    const users = API.get("users/config").users;
    const known_items = API.get("nfc/config").authorized_tags.map(t => <ListGroupItem action type="button" onClick={() => {
        if (t.tag_id != "") {
            on_action(util.get_updated_union(action, {tag_id: t.tag_id, tag_type: t.tag_type}));
        }
        }}>
            <h5 class="mb-1 pr-2">{t.tag_id}</h5>
            <div class="d-flex w-100 justify-content-between">
                <span class="text-left">{translate_unchecked(`nfc.automation.type_${t.tag_type}`)}</span>
                <span class="text-right">{__("nfc.automation.table_user_id") + ": " + users.find(u => u.id == t.user_id).display_name}</span>
            </div>
        </ListGroupItem>);

    const all_tags = known_items.concat(seen_tags);

    return [
        <FormRow label={__("nfc.automation.last_seen_and_known_tags")}>
            {all_tags.length > 0 ?
                <ListGroup>{all_tags}</ListGroup>
                : <span>{__("nfc.automation.add_tag_description")}</span>}
        </FormRow>,
        <FormRow label={__("nfc.automation.table_tag_id")}>
            <InputText
                required
                value={action[1].tag_id}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {tag_id: v}));
                }}
                minLength={8} maxLength={29}
                pattern="^([0-9a-fA-F]{2}:?){3,9}[0-9a-fA-F]{2}$"
                invalidFeedback={__("nfc.automation.tag_id_invalid_feedback")} />
        </FormRow>,
        <FormRow label={__("nfc.automation.table_tag_type")}>
            <InputSelect
                items={[
                    ["0",__("nfc.automation.type_0")],
                    ["1",__("nfc.automation.type_1")],
                    ["2",__("nfc.automation.type_2")],
                    ["3",__("nfc.automation.type_3")],
                    ["4",__("nfc.automation.type_4")],
                ]}
                value={action[1].tag_type.toString()}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {tag_type: parseInt(v)}));
                }} />
        </FormRow>,
        <FormRow label={__("nfc.automation.action")}>
            <InputSelect
                items={[
                    ["0", __("nfc.automation.trigger_charge_any")],
                    ["1", __("nfc.automation.trigger_charge_start")],
                    ["2", __("nfc.automation.trigger_charge_stop")]
                ]}
                value={action[1].action.toString()}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {action: parseInt(v)}));
                }} />
        </FormRow>,
    ]
}

function new_nfc_inject_tag_config(): AutomationAction {
    return [
        AutomationActionID.NFCInjectTag,
        {
            tag_id: "",
            tag_type: 0,
            action: 0,
        },
    ];
}

export function init(): InitResult {
    return {
        action_components: {
            [AutomationActionID.NFCInjectTag]: {
                translation_name: () => __("nfc.automation.nfc"),
                new_config: new_nfc_inject_tag_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_edit_children: get_nfc_inject_tag_edit_children,
                get_table_children: get_nfc_inject_tag_table_children,
            },
        },
    };
}
