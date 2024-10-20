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

import { __ } from "../../ts/translation";
import { AutomationTrigger, InitResult } from "../automation/types";
import { AutomationTriggerID } from "../automation/automation_trigger_id.enum";
import { ComponentChildren } from "preact";

export type ChargeLimitsAutomationTrigger = [
    AutomationTriggerID.ChargeLimits,
    {}
];

function get_charge_limits_table_children(_: ChargeLimitsAutomationTrigger) {
    return __("charge_limits.automation.automation_trigger_text");
}

function get_charge_limits_edit_children(_: ChargeLimitsAutomationTrigger, __: (trigger: AutomationTrigger) => void): ComponentChildren {
    return [];
}

function new_charge_limits_config(): AutomationTrigger {
    return [
        AutomationTriggerID.ChargeLimits,
        {}
    ];
}

export function init(): InitResult {
    return {
        trigger_components: {
            [AutomationTriggerID.ChargeLimits]: {
                translation_name: () => __("charge_limits.automation.charge_limits_expiration"),
                new_config: new_charge_limits_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_edit_children: get_charge_limits_edit_children,
                get_table_children: get_charge_limits_table_children,
            },
        },
    };
}
