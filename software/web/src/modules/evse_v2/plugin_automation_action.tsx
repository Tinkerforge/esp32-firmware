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
import { AutomationActionID } from "../automation/automation_action_id.enum";
import { AutomationAction, InitResult } from "../automation/types";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import * as util from "../../ts/util";

export type EvseGpOutputAutomationAction = [
    AutomationActionID.EVSEGPOutput,
    {
        closed: boolean;
    },
];

function get_evse_gp_output_table_children(action: EvseGpOutputAutomationAction) {
    return __("evse.automation.automation_gpout_action_text")(action[1].closed);
}

function get_evse_gp_output_edit_children(action: EvseGpOutputAutomationAction, on_action: (action: AutomationAction) => void) {
    return [
        <FormRow label={__("evse.automation.gpio_out")}>
            <InputSelect
                items={[
                    ["0", __("evse.automation.gpio_out_open")],
                    ["1", __("evse.automation.gpio_out_closed")],
                ]}
                value={action[1].closed ? "1" : "0"}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {closed: v === "1"}));
                }}
            />
        </FormRow>,
    ];
}

function new_evse_gp_output_config(): AutomationAction {
    return [
        AutomationActionID.EVSEGPOutput,
        {
            closed: true,
        },
    ];
}

export function init(): InitResult {
    return {
        action_components: {
            [AutomationActionID.EVSEGPOutput]: {
                name: __("evse.automation.action_gpio_out"),
                new_config: new_evse_gp_output_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_edit_children: get_evse_gp_output_edit_children,
                get_table_children: get_evse_gp_output_table_children,
            },
        },
    };
}
