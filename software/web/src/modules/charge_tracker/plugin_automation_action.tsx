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

import { h, ComponentChildren } from "preact";
import { __ } from "../../ts/translation";
import { AutomationAction, InitResult } from "../automation/types";
import { AutomationActionID } from "../automation/automation_action_id.enum";

export type ChargeTrackerResetAutomationAction = [
    AutomationActionID.ChargeTrackerReset,
    {}
]

function get_charge_tracker_reset_table_children(_: ChargeTrackerResetAutomationAction) {
    return __("charge_tracker.automation.automation_action_text");
}

function get_charge_tracker_reset_edit_chidren(_: ChargeTrackerResetAutomationAction, __: (action: AutomationAction) => void): ComponentChildren {
    return [];
}

function new_charge_tracker_reset_config(): AutomationAction {
    return [
        AutomationActionID.ChargeTrackerReset,
        {}
    ];
}

export function init(): InitResult {
    return {
        action_components: {
            // [AutomationActionID.ChargeTrackerReset]: {
            //     translation_name: () => __("charge_tracker.automation.charge_tracker_reset"),
            //     get_table_children: get_charge_tracker_reset_table_children,
            //     get_edit_children: get_charge_tracker_reset_edit_chidren,
            //     new_config: new_charge_tracker_reset_config,
            //     clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction
            // },
        },
    };
}
