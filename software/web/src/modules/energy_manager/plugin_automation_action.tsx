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

import * as API  from "../../ts/api";
import { h } from "preact";
import { __ } from "../../ts/translation";
import { AutomationActionID } from "../automation/automation_action_id.enum";
import { AutomationAction, InitResult } from "../automation/types";
import { FormRow } from "../../ts/components/form_row";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import * as util from "../../ts/util";

export type EMRelayAutomationAction = [
    AutomationActionID.EMRelaySwitch,
    {
        index: number;
        closed: boolean;
    },
];

function get_em_relay_table_children(action: EMRelayAutomationAction) {
    const em_version = API.get("energy_manager/state").em_version;
    const relay_count = em_version; // Amount of relays matches em_version.
    const relay_index = relay_count > 1 ? action[1].index : -1;

    return __("energy_manager.automation.relay_action_text")(relay_index, action[1].closed);
}

function get_em_relay_edit_children(action: EMRelayAutomationAction, on_action: (action: AutomationAction) => void) {
    const states: [string, string][] = [
        ['0', __("energy_manager.automation.relay_state_open")],
        ['1', __("energy_manager.automation.relay_state_closed")],
    ]

    const em_version = API.get("energy_manager/state").em_version;
    const relay_count = em_version; // Amount of relays matches em_version.

    let components = [];

    if (relay_count > 1) {
        components.push(
            <FormRow label={__("energy_manager.automation.relay_index")}>
                <InputNumber
                    required
                    min={1}
                    max={relay_count}
                    value={action[1].index + 1}
                    onValue={(v) => {on_action(util.get_updated_union(action, {index: v - 1}));}}
                />
            </FormRow>
        );
    }

    components.push(
        <FormRow label={__("energy_manager.automation.relay_state")}>
            <InputSelect
                items={states}
                value={action[1].closed ? '1' : '0'}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {closed: v == '1'}));
                }} />
        </FormRow>
    );

    return components;
}

function new_em_relay_config(): AutomationAction {
    const em_version = API.get("energy_manager/state").em_version;

    return [
        AutomationActionID.EMRelaySwitch,
        {
            index: 0,
            closed: true,
        },
    ];
}

export function init(): InitResult {
    return {
        action_components: {
            [AutomationActionID.EMRelaySwitch]: {
                translation_name: () => __("energy_manager.automation.switch_relay"),
                new_config: new_em_relay_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_table_children: get_em_relay_table_children,
                get_edit_children: get_em_relay_edit_children,
            },
        }
    }
}
