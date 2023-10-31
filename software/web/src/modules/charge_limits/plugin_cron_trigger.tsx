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

import { __ } from "../../ts/translation";
import { Cron } from "../cron/main";
import { CronTrigger } from "../cron/types";
import { CronTriggerID } from "../cron/cron_defs";
import { VNode, ComponentChildren } from "preact";

export type ChargeLimitsCronTrigger = [
    CronTriggerID.ChargeLimits,
    {}
];

function get_charge_limits_table_children(_: CronTrigger) {
    return __("charge_limits.content.cron_trigger_text");
}

function get_charge_limits_edit_children(_: Cron, __: CronTrigger): ComponentChildren {
    return []
}

function new_charge_limits_config(): CronTrigger {
    return [
        CronTriggerID.ChargeLimits,
        {}
    ];
}

export function init() {
    return {
        trigger_components: {
            [CronTriggerID.ChargeLimits]: {
                name: __("charge_limits.content.charge_limits_expiration"),
                new_config: new_charge_limits_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_edit_children: get_charge_limits_edit_children,
                get_table_children: get_charge_limits_table_children,
            },
        },
    };
}
