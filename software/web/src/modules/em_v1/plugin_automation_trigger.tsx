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
import { __ } from "../../ts/translation";
import { AutomationTriggerID } from "../automation/automation_trigger_id.enum";
import { AutomationTrigger, InitResult } from "../automation/types";
import { FormRow } from "../../ts/components/form_row";
import { InputSelect } from "../../ts/components/input_select";
import * as util from "../../ts/util";

export type EMInputThreeAutomationTrigger = [
    AutomationTriggerID.EMInputThree,
    {
        closed: boolean;
    },
];

export type EMInputFourAutomationTrigger = [
    AutomationTriggerID.EMInputFour,
    {
        closed: boolean;
    },
];

export type EMPhaseSwitchAutomationTrigger = [
    AutomationTriggerID.EMPhaseSwitch,
    {
        phases: number;
    },
];

export type EMContactorMonitoringAutomationTrigger = [
    AutomationTriggerID.EMContactorMonitoring,
    {
        contactor_okay: boolean;
    },
];

function get_em_input_three_table_children(trigger: EMInputThreeAutomationTrigger) {
    return __("energy_manager.automation.automation_input_text")(3, trigger[1].closed);
}

function get_em_input_three_edit_children(trigger: EMInputThreeAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    return [
        <FormRow label={__("energy_manager.automation.state")}>
            <InputSelect
                value={trigger[1].closed ? '1' : '0'}
                items = {[
                    ['0', __("energy_manager.automation.open")],
                    ['1', __("energy_manager.automation.closed")],
                ]}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {closed: v === '1'}));
                }}
            />
        </FormRow>,
    ];
}

function new_em_input_three_config(): AutomationTrigger {
    return [
        AutomationTriggerID.EMInputThree,
        {
            closed: false,
        },
    ];
}

function get_em_input_four_table_children(trigger: EMInputFourAutomationTrigger) {
    return __("energy_manager.automation.automation_input_text")(4, trigger[1].closed);
}

function get_em_input_four_edit_children(trigger: EMInputFourAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    return [
        <FormRow label={__("energy_manager.automation.state")}>
            <InputSelect
                value={trigger[1].closed ? '1' : '0'}
                items = {[
                    ['0', __("energy_manager.automation.open")],
                    ['1', __("energy_manager.automation.closed")],
                ]}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {closed: v === '1'}));
                }} />
        </FormRow>,
    ];
}

function new_em_input_four_config(): AutomationTrigger {
    return [
        AutomationTriggerID.EMInputFour,
        {
            closed: false,
        },
    ];
}

function get_em_phase_switch_table_children(trigger: EMPhaseSwitchAutomationTrigger) {
    return __("em_v1.automation.automation_phase_switch_text")(trigger[1].phases);
}

function get_em_phase_switch_edit_children(trigger: EMPhaseSwitchAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    return [
        <FormRow label={__("em_v1.automation.phase")}>
            <InputSelect
                value={trigger[1].phases.toString()}
                items = {[
                    ['1', __("em_v1.automation.single_phase")],
                    ['3', __("em_v1.automation.three_phase")],
                ]}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {phases: parseInt(v)}));
                }} />
        </FormRow>,
    ];
}

function new_em_phase_switch_config(): AutomationTrigger {
    return [
        AutomationTriggerID.EMPhaseSwitch,
        {
            phases: 1,
        },
    ];
}

function get_em_contactor_monitoring_table_children(trigger: EMContactorMonitoringAutomationTrigger) {
    return __("em_v1.automation.automation_contactor_monitoring_text")(trigger[1].contactor_okay);
}

function get_em_contactor_monitoring_edit_children(trigger: EMContactorMonitoringAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    return [
        <FormRow label={__("em_v1.automation.contactor_monitoring_state")}>
            <InputSelect
                value={trigger[1].contactor_okay ? '1' : '0'}
                items = {[
                    ['0', __("em_v1.automation.contactor_error")],
                    ['1', __("em_v1.automation.contactor_okay")],
                ]}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {contactor_okay: v === '1'}));
                }} />
        </FormRow>,
    ];
}

function new_em_contactor_monitoring_config(): AutomationTrigger {
    return [
        AutomationTriggerID.EMContactorMonitoring,
        {
            contactor_okay: false,
        },
    ];
}

export function init(): InitResult {
    return {
        trigger_components: {
            [AutomationTriggerID.EMInputThree]: {
                translation_name: () => __("energy_manager.automation.input")(3),
                new_config: new_em_input_three_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_table_children: get_em_input_three_table_children,
                get_edit_children: get_em_input_three_edit_children,
            },
            [AutomationTriggerID.EMInputFour]: {
                translation_name: () => __("energy_manager.automation.input")(4),
                new_config: new_em_input_four_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_table_children: get_em_input_four_table_children,
                get_edit_children: get_em_input_four_edit_children,
            },
            [AutomationTriggerID.EMPhaseSwitch]: {
                translation_name: () => __("em_v1.automation.phase_switch"),
                new_config: new_em_phase_switch_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_table_children: get_em_phase_switch_table_children,
                get_edit_children: get_em_phase_switch_edit_children,
            },
            [AutomationTriggerID.EMContactorMonitoring]: {
                translation_name: () => __("em_v1.automation.contactor_monitoring"),
                new_config: new_em_contactor_monitoring_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_table_children: get_em_contactor_monitoring_table_children,
                get_edit_children: get_em_contactor_monitoring_edit_children,
            },
        },
    };
}
