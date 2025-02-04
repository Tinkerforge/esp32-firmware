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

import { h } from "preact";
import * as util from "../../ts/util";
import { __ } from "../../ts/translation";
import { AutomationActionID } from "../automation/automation_action_id.enum";
import { AutomationAction, InitResult } from "../automation/types";
import { InputFloat } from "../../ts/components/input_float";
import { FormRow } from "../../ts/components/form_row";
import * as API from "../../ts/api"

//#region SetManagerCurrent
export type SetManagerCurrentAutomationAction = [
    AutomationActionID.SetManagerCurrent,
    {
        current: number;
    },
];

function get_set_manager_current_table_children(action: SetManagerCurrentAutomationAction) {
    return __("charge_manager.automation.automation_action_text")(util.toLocaleFixed(action[1].current / 1000, 3));
}

function get_set_manager_current_edit_children(action: SetManagerCurrentAutomationAction, on_action: (action: AutomationAction) => void) {
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
//#endregion

//#region ChargeModeSwitch
export type PMChargeModeSwitchAutomationAction = [
    AutomationActionID.PMChargeModeSwitch,
    {
        mode: number;
    },
];

function get_pm_charge_mode_switch_table_children(action: PMChargeModeSwitchAutomationAction) {
    return __("power_manager.automation.charge_mode_switch_action_text")(action[1].mode, API.get("power_manager/config").default_mode);
}

function get_pm_charge_mode_switch_edit_children(action: PMChargeModeSwitchAutomationAction, on_action: (action: AutomationAction) => void) {
    const modes: [string, string][] = [
        ['0', __("power_manager.automation.fast")],
        ['1', __("power_manager.automation.disabled")],
        ['2', __("power_manager.automation.pv_excess")],
        ['3', __("power_manager.automation.guaranteed_power")],
    ]

    modes.push(['4', __("power_manager.automation.charge_mode_default") + " (" + modes[API.get("power_manager/config").default_mode][1] + ")"])

    return [
        <FormRow label={__("power_manager.automation.charge_mode")}>
            <InputSelect
                items={modes}
                value={action[1].mode.toString()}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {mode: parseInt(v)}));
                }} />
        </FormRow>,
    ];
}

function new_pm_charge_mode_switch_config(): AutomationAction {
    return [
        AutomationActionID.PMChargeModeSwitch,
        {
            mode: 4,
        },
    ];
}
//#endregion

export function init(): InitResult {
    return {
        action_components: {
            [AutomationActionID.SetManagerCurrent]: {
                translation_name: () => __("charge_manager.automation.set_charge_manager"),
                new_config: new_set_manager_current_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_edit_children: get_set_manager_current_edit_children,
                get_table_children: get_set_manager_current_table_children,
            },
            [AutomationActionID.PMChargeModeSwitch]: {
                translation_name: () => __("power_manager.automation.charge_mode_switch"),
                new_config: new_pm_charge_mode_switch_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_table_children: get_pm_charge_mode_switch_table_children,
                get_edit_children: get_pm_charge_mode_switch_edit_children,
            },
        },
    };
}
