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
import { Cron } from "../cron/main";
import { CronComponent, CronAction } from "../cron/types";
import { InputFloat } from "../../ts/components/input_float";

export type ChargeManagerCronAction = [
    CronActionID.SetManagerCurrent,
    {
        current: number;
    },
];

export function ChargeManagerCronComponent(action: CronAction): VNode {
    let value = (action as ChargeManagerCronAction)[1];
    return __("charge_manager.content.cron_action_text")(value.current);
}

export function ChargeManagerCronConfigComponent(cron: Cron, action: CronAction) {
    let value = (action as ChargeManagerCronAction)[1];
    return [{
        name: "Maximaler Strom",
        value: <InputFloat value={value.current}
                    onValue={(v) => {
                        value.current = v;
                        cron.setActionFromComponent(action);
                    }}
                    min={0}
                    unit="A"
                    digits={3}/>
    }]
}

function ChargeManagerCronActionFactory(): CronAction {
    return [
        CronActionID.SetManagerCurrent,
        {
            current: 0,
        },
    ];
}

export function init() {
    return {
        action_components: {
            [CronActionID.SetManagerCurrent]: {
                clone: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                config_builder: ChargeManagerCronActionFactory,
                config_component: ChargeManagerCronConfigComponent,
                table_row: ChargeManagerCronComponent,
                name: __("charge_manager.content.set_charge_manager"),
            },
        },
    };
}
