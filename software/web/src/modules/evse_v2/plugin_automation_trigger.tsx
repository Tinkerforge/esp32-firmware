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
import { AutomationTriggerID } from "../automation/automation_trigger_id.enum";
import { AutomationTrigger, InitResult } from "../automation/types";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import * as util from "../../ts/util";

export type EvseShutdownAutomationTrigger = [
    AutomationTriggerID.EVSEShutdownInput,
    {
        closed: boolean;
    },
];

export type EvseGpInputAutomationTrigger = [
    AutomationTriggerID.EVSEGPInput,
    {
        closed: boolean;
    },
];

export type EvseButtonAutomationTrigger = [
    AutomationTriggerID.EVSEButton,
    null,
];

function get_evse_button_table_children(trigger: EvseButtonAutomationTrigger) {
    return __("evse.automation.automation_button_trigger_text");
}

function get_evse_button_edit_children(_: EvseButtonAutomationTrigger, __: (trigger: AutomationTrigger) => void): ComponentChildren {
    return []
}

function new_evse_button_config(): AutomationTrigger {
    return [
        AutomationTriggerID.EVSEButton,
        null,
    ];
}

function get_evse_shutdown_table_children(trigger: EvseGpInputAutomationTrigger) {
    return __("evse.automation.automation_sd_trigger_text")(trigger[1].closed);
}

function get_evse_shutdown_edit_children(trigger: EvseGpInputAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    return [
        <FormRow label="">
            <InputSelect
                items={[
                    ["0", __("evse.automation.automation_trigger_input_opened")],
                    ["1", __("evse.automation.automation_trigger_input_closed")],
                ]}
                value={trigger[1].closed ? "1" : "0"}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {closed: v === "1"}));
                }}
            />
        </FormRow>,
    ];
}

function new_evse_shutdown_input_config(): AutomationTrigger {
    return [
        AutomationTriggerID.EVSEShutdownInput,
        {
            closed: true,
        },
    ];
}

function get_evse_gp_input_table_children(trigger: EvseGpInputAutomationTrigger) {
    return __("evse.automation.automation_gpin_trigger_text")(trigger[1].closed);
}

function get_evse_gp_input_edit_children(trigger: EvseGpInputAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    return [
        <FormRow label="">
            <InputSelect
                items={[
                    ["0", __("evse.automation.automation_trigger_input_opened")],
                    ["1", __("evse.automation.automation_trigger_input_closed")],
                ]}
                value={trigger[1].closed ? "1" : "0"}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {closed: v === "1"}));
                }}
            />
        </FormRow>,
    ];
}

function new_evse_gp_input_config(): AutomationTrigger {
    return [
        AutomationTriggerID.EVSEGPInput,
        {
            closed: true,
        },
    ];
}

export function init(): InitResult {
    return {
        trigger_components: {
            [AutomationTriggerID.EVSEButton]: {
                translation_name: () => __("evse.automation.automation_trigger_button"),
                new_config: new_evse_button_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], null] as AutomationTrigger,
                get_edit_children: get_evse_button_edit_children,
                get_table_children: get_evse_button_table_children,
            },
            [AutomationTriggerID.EVSEShutdownInput]: {
                translation_name: () => __("evse.automation.automation_trigger_shutdown_input"),
                new_config: new_evse_shutdown_input_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_edit_children: get_evse_shutdown_edit_children,
                get_table_children: get_evse_shutdown_table_children,
            },
            [AutomationTriggerID.EVSEGPInput]: {
                translation_name: () => __("evse.automation.automation_trigger_gp_input"),
                new_config: new_evse_gp_input_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_edit_children: get_evse_gp_input_edit_children,
                get_table_children: get_evse_gp_input_table_children,
            },
        },
    };
}
