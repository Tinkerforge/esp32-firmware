/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import * as util from "../../ts/util";

export type EMInputAutomationTrigger = [
    AutomationTriggerID.EMInput,
    {
        index: number;
        closed: boolean;
    },
];

function get_em_input_table_children(trigger: EMInputAutomationTrigger) {
    return __("energy_manager.automation.automation_input_text")(trigger[1].index + 1, trigger[1].closed);
}

function get_em_input_edit_children(trigger: EMInputAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    return [
        <FormRow label={__("em_v2.automation.input_index")}>
            <InputNumber
                required
                min={1}
                max={4}
                value={trigger[1].index + 1}
                onValue={(v) => {on_trigger(util.get_updated_union(trigger, {index: v - 1}));}}
            />
        </FormRow>,

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

function new_em_input_config(): AutomationTrigger {
    return [
        AutomationTriggerID.EMInput,
        {
            index: 0,
            closed: true,
        },
    ];
}

export function init(): InitResult {
    return {
        trigger_components: {
            [AutomationTriggerID.EMInput]: {
                translation_name: () => __("em_v2.automation.input_switches"),
                new_config: new_em_input_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_table_children: get_em_input_table_children,
                get_edit_children: get_em_input_edit_children,
            },
        },
    };
}
