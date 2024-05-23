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

import { h } from "preact";
import * as util from "../../ts/util";
import { __ } from "../../ts/translation";
import { AutomationActionID } from "../automation/automation_action_id.enum";
import { AutomationAction, InitResult } from "../automation/types";
import { InputFloat } from "../../ts/components/input_float";
import { FormRow } from "../../ts/components/form_row";
import * as API from "../../ts/api"

export type ChargeManagerAutomationAction = [
    AutomationActionID.SetManagerCurrent,
    {
        current: number;
    },
];

function get_set_manager_table_children(action: ChargeManagerAutomationAction) {
    return __("charge_manager.automation.automation_action_text")(util.toLocaleFixed(action[1].current / 1000, 3));
}

function get_set_manager_edit_children(action: ChargeManagerAutomationAction, on_action: (action: AutomationAction) => void) {
    return [
        <FormRow label={__("charge_manager.automation.max_current")}>
            <InputFloat
                value={action[1].current}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {current: v}));
                }}
                min={0}
                max={API.get("charge_manager/config").maximum_available_current}
                unit="A"
                digits={3} />
        </FormRow>,
    ];
}

function new_set_manager_current_config(): AutomationAction {
    return [
        AutomationActionID.SetManagerCurrent,
        {
            current: 0,
        },
    ];
}

export function init(): InitResult {
    return {
        action_components: {
            [AutomationActionID.SetManagerCurrent]: {
                name: __("charge_manager.automation.set_charge_manager"),
                new_config: new_set_manager_current_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_edit_children: get_set_manager_edit_children,
                get_table_children: get_set_manager_table_children,
            },
        },
    };
}
