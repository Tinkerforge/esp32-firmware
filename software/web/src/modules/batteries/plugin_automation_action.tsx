/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

import { ComponentChildren } from "preact";
import { __ } from "../../ts/translation";
import { AutomationActionID } from "../automation/automation_action_id.enum";
import { AutomationAction, InitResult } from "../automation/types";

export type PermitGridChargeAutomationAction = [
    AutomationActionID.PermitGridCharge,
    {
        battery_slot: number,
    },
];

function new_permit_grid_charge_config(): AutomationAction {
    return [
        AutomationActionID.PermitGridCharge,
        {
            battery_slot: 0,
        },
    ];
}

function get_permit_grid_charge_table_children(action: PermitGridChargeAutomationAction) {
    return __("batteries.automation.permit_grid_charge");
}

function get_permit_grid_charge_edit_children(action: PermitGridChargeAutomationAction, on_action: (action: AutomationAction) => void): ComponentChildren {
    return [];
}

export type RevokeGridChargeOverrideAutomationAction = [
    AutomationActionID.RevokeGridChargeOverride,
    {
        battery_slot: number,
    },
];

function new_revoke_grid_charge_override_config(): AutomationAction {
    return [
        AutomationActionID.RevokeGridChargeOverride,
        {
            battery_slot: 0,
        },
    ];
}

function get_revoke_grid_charge_override_table_children(action: RevokeGridChargeOverrideAutomationAction) {
    return __("batteries.automation.revoke_grid_charge_override");
}

function get_revoke_grid_charge_override_edit_children(action: RevokeGridChargeOverrideAutomationAction, on_action: (action: AutomationAction) => void): ComponentChildren {
    return [];
}

export type ForbidDischargeAutomationAction = [
    AutomationActionID.ForbidDischarge,
    {
        battery_slot: number,
    },
];

function new_forbid_discharge_config(): AutomationAction {
    return [
        AutomationActionID.ForbidDischarge,
        {
            battery_slot: 0,
        },
    ];
}

function get_forbid_discharge_table_children(action: ForbidDischargeAutomationAction) {
    return __("batteries.automation.forbid_discharge");
}

function get_forbid_discharge_edit_children(action: ForbidDischargeAutomationAction, on_action: (action: AutomationAction) => void): ComponentChildren {
    return [];
}

export type RevokeDischargeOverrideAutomationAction = [
    AutomationActionID.RevokeDischargeOverride,
    {
        battery_slot: number,
    },
];

function new_revoke_discharge_override_config(): AutomationAction {
    return [
        AutomationActionID.RevokeDischargeOverride,
        {
            battery_slot: 0,
        },
    ];
}

function get_revoke_discharge_override_table_children(action: RevokeDischargeOverrideAutomationAction) {
    return __("batteries.automation.revoke_discharge_override");
}

function get_revoke_discharge_override_edit_children(action: RevokeDischargeOverrideAutomationAction, on_action: (action: AutomationAction) => void): ComponentChildren {
    return [];
}

export function init(): InitResult {
    return {
        action_components: {
            [AutomationActionID.PermitGridCharge]: {
                translation_name: () => __("batteries.automation.permit_grid_charge"),
                new_config: new_permit_grid_charge_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_table_children: get_permit_grid_charge_table_children,
                get_edit_children: get_permit_grid_charge_edit_children,
            },
            [AutomationActionID.RevokeGridChargeOverride]: {
                translation_name: () => __("batteries.automation.revoke_grid_charge_override"),
                new_config: new_revoke_grid_charge_override_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_table_children: get_revoke_grid_charge_override_table_children,
                get_edit_children: get_revoke_grid_charge_override_edit_children,
            },
            [AutomationActionID.ForbidDischarge]: {
                translation_name: () => __("batteries.automation.forbid_discharge"),
                new_config: new_forbid_discharge_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_table_children: get_forbid_discharge_table_children,
                get_edit_children: get_forbid_discharge_edit_children,
            },
            [AutomationActionID.RevokeDischargeOverride]: {
                translation_name: () => __("batteries.automation.revoke_discharge_override"),
                new_config: new_revoke_discharge_override_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_table_children: get_revoke_discharge_override_table_children,
                get_edit_children: get_revoke_discharge_override_edit_children,
            },
        },
    };
}
