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
import { CronTriggerID } from "../cron/cron_defs";
import { CronTrigger } from "../cron/types";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import * as util from "../../ts/util";

export type IECChangeCronTrigger = [
    CronTriggerID.IECChange,
    {
        new_charger_state: number;
        old_charger_state: number;
    },
];

export type EVSEEexternalCurrentWdCronTrigger = [
    CronTriggerID.EVSEExternalCurrentWd,
    {},
];

function new_iec_change_config(): CronTrigger {
    return [
        CronTriggerID.IECChange,
        {
            new_charger_state: -1,
            old_charger_state: -1,
        },
    ];
}

function get_iec_change_table_children(trigger: IECChangeCronTrigger) {
    const names = [
        [__("evse.cron.any")],
        [__("evse.status.not_connected")],
        [__("evse.status.waiting_for_charge_release")],
        [__("evse.status.ready_to_charge")],
        [__("evse.status.charging")],
        [__("evse.status.error")],
    ];

    return __("evse.cron.cron_state_change_trigger")(names[trigger[1].old_charger_state + 1][0], names[trigger[1].new_charger_state + 1][0]);
}

function get_iec_change_edit_children(trigger: IECChangeCronTrigger, on_trigger: (trigger: CronTrigger) => void) {
    return [
        <FormRow label={__("evse.cron.from")}>
            <InputSelect
                items={[
                    ["-1", __("evse.cron.any")],
                    ["0", __("evse.status.not_connected")],
                    ["1", __("evse.status.waiting_for_charge_release")],
                    ["2", __("evse.status.ready_to_charge")],
                    ["3", __("evse.status.charging")],
                    ["4", __("evse.status.error")],
                ]}
                value={trigger[1].old_charger_state.toString()}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {old_charger_state: parseInt(v)}));
                }}
            />
        </FormRow>,
        <FormRow label={__("evse.cron.to")}>
            <InputSelect
                items={[
                    ["-1", __("evse.cron.any")],
                    ["0", __("evse.status.not_connected")],
                    ["1", __("evse.status.waiting_for_charge_release")],
                    ["2", __("evse.status.ready_to_charge")],
                    ["3", __("evse.status.charging")],
                    ["4", __("evse.status.error")],
                ]}
                value={trigger[1].new_charger_state.toString()}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {new_charger_state: parseInt(v)}));
                }}
            />
        </FormRow>,
    ];
}

function new_external_current_wd_config(): CronTrigger {
    return [
        CronTriggerID.EVSEExternalCurrentWd,
        {},
    ];
}

function get_external_current_wd_table_children(_: EVSEEexternalCurrentWdCronTrigger) {
    return __("evse.cron.external_current_wd_trigger");
}

function get_external_current_wd_edit_children(_: EVSEEexternalCurrentWdCronTrigger, __: (trigger: CronTrigger) => void): ComponentChildren {
    return []
}

export function init() {
    return {
        trigger_components: {
            [CronTriggerID.IECChange]: {
                name: __("evse.cron.state_change"),
                new_config: new_iec_change_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_edit_children: get_iec_change_edit_children,
                get_table_children: get_iec_change_table_children,
            },
            [CronTriggerID.EVSEExternalCurrentWd]: {
                name: __("evse.cron.external_current_wd"),
                new_config: new_external_current_wd_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_edit_children: get_external_current_wd_edit_children,
                get_table_children: get_external_current_wd_table_children,
            },
        },
    };
}
