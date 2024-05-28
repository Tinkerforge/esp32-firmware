/* esp32-firmware
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
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

import { h, Fragment } from "preact";
import { useState } from "preact/hooks";
import { __ } from "../../ts/translation";
import { AutomationActionID } from "../automation/automation_action_id.enum";
import { AutomationAction, InitResult } from "../automation/types";
import { InputText } from "../../ts/components/input_text";
import { FormRow } from "../../ts/components/form_row";
import * as API from "../../ts/api";
import * as util from "../../ts/util";

export type PrintAutomationAction = [
    AutomationActionID.Print,
    {
        message: string;
    },
];

function get_print_table_children(action: PrintAutomationAction) {
    return __("automation.automation.print_action_text")(action[1].message);
}

function get_print_edit_children(action: PrintAutomationAction, on_action: (action: AutomationAction) => void) {
    return [
        <FormRow label={__("automation.automation.print_action_message")}>
             <InputText
                required
                value={action[1].message}
                maxLength={64}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {message: v}));
                }} />
        </FormRow>,
    ]
}

function new_print_config(): AutomationAction {
    return [
        AutomationActionID.Print,
        {
            message: ""
        },
    ];
}

export function init(): InitResult {
    return {
        action_components: {
            [AutomationActionID.Print]: {
                name: __("automation.automation.print_action"),
                new_config: new_print_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_edit_children: get_print_edit_children,
                get_table_children: get_print_table_children,
            },
        },
    };
}
