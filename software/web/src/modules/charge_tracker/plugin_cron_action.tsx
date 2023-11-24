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
import { CronAction } from "../cron/types";
import { CronActionID } from "../cron/cron_defs";


export type ChargeLimitsCronAction = [
    CronActionID.ChargeTrackerReset,
    {}
]

function get_charge_tracker_reset_table_children(_: ChargeLimitsCronAction) {
    return __("charge_tracker.cron.cron_action_text");
}

function get_charge_tracker_reset_edit_chidren(_: ChargeLimitsCronAction, __: (action: CronAction) => void): h.JSX.Element[] {
    return [];
}

function new_charge_tracker_reset_config(): CronAction {
    return [
        CronActionID.ChargeTrackerReset,
        {}
    ];
}

export function init() {
    return {
        action_components:{
            [CronActionID.ChargeTrackerReset]: {
                name: __("charge_tracker.cron.charge_tracker_reset"),
                get_table_children: get_charge_tracker_reset_table_children,
                get_edit_children: get_charge_tracker_reset_edit_chidren,
                new_config: new_charge_tracker_reset_config,
                clone_config: (action: CronAction) => [action[0], {...action[1]}] as CronAction
            }
        }
    }
}
