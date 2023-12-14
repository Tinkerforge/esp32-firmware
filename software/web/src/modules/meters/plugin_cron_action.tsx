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
import { ComponentChildren } from "preact";
import { FormRow } from "../../ts/components/form_row";
import { METERS_SLOTS } from "../../build";
import { InputSelect } from "../../ts/components/input_select";
import * as API from "../../ts/api";

export type MeterCronAction = [
    CronActionID.MeterReset,
    {
        meter_slot: number
    }
]

function get_meter_reset_table_children(action: MeterCronAction) {
    const meter = API.get_unchecked(`meters/${action[1].meter_slot}/config`);
    if (!meter) {
        return __("meters.content.unknown_slot")(action[1].meter_slot);
    }
    return __("meters.cron.cron_action_text")(meter[1].display_name);
}

function get_meter_reset_edit_children(action: MeterCronAction, on_action: (action: CronAction) => void): ComponentChildren {
    let items: [string, string][] = [];
    for (let i = 0; i < METERS_SLOTS; i++) {
        const meter = API.get_unchecked(`meters/${i}/config`);
        if (meter[1]) {
            items.push([i.toString(), meter[1].display_name]);
        }
    }

    return [
        <FormRow label={__("meters.cron.meter_slot")}>
            <InputSelect
                items={items}
                onValue={(v) => {
                    on_action([CronActionID.MeterReset, {...action[1], meter_slot: parseInt(v)}]);
                }}
                value={action[1].meter_slot.toString()}/>
        </FormRow>
    ];
}

function new_meter_reset_config(): CronAction {
    return [
        CronActionID.MeterReset,
        {
            meter_slot: 0
        }
    ];
}

export function init() {
    return {
        action_components: {
            [CronActionID.MeterReset]: {
                name: __("meters.cron.meter_reset"),
                new_config: new_meter_reset_config,
                clone_config: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                get_edit_children: get_meter_reset_edit_children,
                get_table_children: get_meter_reset_table_children,
            },
        },
    };
}
