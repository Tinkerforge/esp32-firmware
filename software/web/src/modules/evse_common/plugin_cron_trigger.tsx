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
import { CronTriggerID } from "../cron/cron_defs";
import { Cron } from "../cron/main";
import { CronTrigger } from "../cron/types";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";

export type IECChangeCronTrigger = [
    CronTriggerID.IECChange,
    {
        charger_state: number;
    },
];

function get_iec_change_table_children(trigger: CronTrigger) {
    let value = (trigger as IECChangeCronTrigger)[1];
    const names = [
        [ __("evse.status.not_connected")],
        [__("evse.status.waiting_for_charge_release")],
        [__("evse.status.ready_to_charge")],
        [__("evse.status.charging")],
        [__("evse.status.error")]
    ]

    return __("evse.content.cron_state_change_trigger")(names[value.charger_state][0]);
}

function new_iec_change_config(): CronTrigger {
    return [
        CronTriggerID.IECChange,
        {
            charger_state: 0,
        },
    ];
}

function get_iec_change_edit_children(cron: Cron, trigger: CronTrigger) {
    let value = (trigger as IECChangeCronTrigger)[1];
    return [
        <FormRow label="">
            <InputSelect
                items={[
                    ["0", __("evse.status.not_connected")],
                    ["1", __("evse.status.waiting_for_charge_release")],
                    ["2", __("evse.status.ready_to_charge")],
                    ["3", __("evse.status.charging")],
                    ["4", __("evse.status.error")]
                ]}
                value={value.charger_state.toString()}
                onValue={(v) => {
                    value.charger_state = Number(v);
                    cron.setTriggerFromComponent(trigger);
                }} />
        </FormRow>
    ]
}

export function init() {
    return {
        trigger_components: {
            [CronTriggerID.IECChange]: {
                name: __("evse.content.state_change"),
                new_config: new_iec_change_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_edit_children: get_iec_change_edit_children,
                get_table_children: get_iec_change_table_children,
            },
        },
    };
}
