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
import { CronActionID } from "../cron/cron_defs";
import { CronAction } from "../cron/types";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import * as util from "../../ts/util";

export type EvseGpOutputCronAction = [
    CronActionID.EVSEGPOutput,
    {
        state: number;
    },
];

function get_evse_gp_output_table_children(action: EvseGpOutputCronAction) {
    return __("evse.cron.cron_gpout_action_text")(action[1].state);
}

function get_evse_gp_output_edit_children(action: EvseGpOutputCronAction, on_action: (action: CronAction) => void) {
    return [
        <FormRow label={__("evse.cron.gpio_out")}>
            <InputSelect
                items={[
                    ["0", __("evse.cron.gpio_out_low")],
                    ["1", __("evse.cron.gpio_out_high")]
                ]}
                value={action[1].state}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {state: parseInt(v)}));
                }} />
        </FormRow>
    ]
}

function new_evse_gp_output_config(): CronAction {
    return [
        CronActionID.EVSEGPOutput,
        {
            state: 0,
        },
    ];
}

export function init() {
    return {
        action_components: {
            [CronActionID.EVSEGPOutput]: {
                name: __("evse.cron.gpio_out"),
                new_config: new_evse_gp_output_config,
                clone_config: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                get_edit_children: get_evse_gp_output_edit_children,
                get_table_children: get_evse_gp_output_table_children,
                require_feature: "button_configuration",
            },
        },
    };
}
