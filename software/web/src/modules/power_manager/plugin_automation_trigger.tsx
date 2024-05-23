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
import { __ } from "../../ts/translation";
import { AutomationTriggerID } from "../automation/automation_trigger_id.enum";
import { AutomationTrigger, InitResult } from "../automation/types";
import { FormRow } from "../../ts/components/form_row";
import { InputSelect } from "../../ts/components/input_select";
import * as util from "../../ts/util";

export type PMPowerAvailableAutomationTrigger = [
    AutomationTriggerID.PMPowerAvailable,
    {
        power_available: boolean;
    },
];

export type PMGridPowerDrawAutomationTrigger = [
    AutomationTriggerID.PMGridPowerDraw,
    {
        drawing_power: boolean;
    },
];

function get_pm_power_available_table_children(trigger: PMPowerAvailableAutomationTrigger) {
    return __("power_manager.automation.automation_power_available_text")(trigger[1].power_available);
}

function get_pm_power_available_edit_children(trigger: PMPowerAvailableAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    return [
        <FormRow label={__("power_manager.automation.power")}>
            <InputSelect
                value={trigger[1].power_available ? '1' : '0'}
                items = {[
                    ['0', __("power_manager.automation.not_available")],
                    ['1', __("power_manager.automation.available")],
                ]}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {power_available: v === '1'}));
                }} />
        </FormRow>,
    ];
}

function new_pm_power_available_config(): AutomationTrigger {
    return [
        AutomationTriggerID.PMPowerAvailable,
        {
            power_available: false,
        },
    ];
}

function get_pm_grid_power_draw_table_children(trigger: PMGridPowerDrawAutomationTrigger) {
    return __("power_manager.automation.automation_grid_power_draw_text")(trigger[1].drawing_power);
}

function get_pm_grid_power_draw_edit_children(trigger: PMGridPowerDrawAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    return [
        <FormRow label={__("power_manager.automation.power")}>
            <InputSelect
                value={trigger[1].drawing_power ? '1' : '0'}
                items = {[
                    ['0', __("power_manager.automation.feeding")],
                    ['1', __("power_manager.automation.drawing")],
                ]}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {drawing_power: v === '1'}));
                }} />
        </FormRow>,
    ];
}

function new_pm_grid_power_draw_config(): AutomationTrigger {
    return [
        AutomationTriggerID.PMGridPowerDraw,
        {
            drawing_power: false,
        },
    ];
}

export function init(): InitResult {
    return {
        trigger_components: {
            [AutomationTriggerID.PMPowerAvailable]: {
                name: __("power_manager.automation.power_available"),
                new_config: new_pm_power_available_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_table_children: get_pm_power_available_table_children,
                get_edit_children: get_pm_power_available_edit_children,
            },
            [AutomationTriggerID.PMGridPowerDraw]: {
                name: __("power_manager.automation.grid_power_draw"),
                new_config: new_pm_grid_power_draw_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_table_children: get_pm_grid_power_draw_table_children,
                get_edit_children: get_pm_grid_power_draw_edit_children,
            },
        },
    };
}
