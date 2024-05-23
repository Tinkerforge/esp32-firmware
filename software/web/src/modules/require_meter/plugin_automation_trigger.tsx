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

import { h, ComponentChildren } from "preact";
import { __ } from "../../ts/translation";
import { AutomationTriggerID } from "../automation/automation_trigger_id.enum";
import { AutomationTrigger, InitResult } from "../automation/types";

export type RequireMeterAutomationTrigger = [
    AutomationTriggerID.RequireMeter,
    {}
]

function get_require_meter_table_children(_: AutomationTrigger) {
    return __("require_meter.automation.automation_trigger_text");
}

function get_require_meter_edit_children(_: RequireMeterAutomationTrigger, __: (trigger: AutomationTrigger) => void): ComponentChildren {
    return [];
}

function new_require_meter_config(): AutomationTrigger {
    return [
        AutomationTriggerID.RequireMeter,
        {}
    ];
}

export function init(): InitResult {
    return {
        trigger_components: {
            [AutomationTriggerID.RequireMeter]: {
                name: __("require_meter.automation.require_meter"),
                new_config: new_require_meter_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_table_children: get_require_meter_table_children,
                get_edit_children: get_require_meter_edit_children,
            },
        },
    };
}
