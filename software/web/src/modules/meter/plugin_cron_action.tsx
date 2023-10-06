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
import { CronComponent, CronAction } from "../cron/types";
import { CronActionID } from "../cron/cron_defs";
import { TableModalRow } from "../../ts/components/table";

export type MeterCronAction = [
    CronActionID.MeterReset,
    {}
]

function MeterResetCronActionComponent(_: CronAction): VNode {
    return __("meter.content.cron_action_text") as any as VNode;
}

function MeterResetCronActionConfigComponent(_: Cron, __: CronAction): TableModalRow[] {
    return [];
}

function MeterResetCronActionConfigFactory(): CronAction {
    return [
        CronActionID.MeterReset,
        {}
    ];
}

export function init() {
    return {
        action_components: {
            [CronActionID.MeterReset]: {
                clone: (action: CronAction) => [action[0], {...action[1]}] as CronAction,
                config_builder: MeterResetCronActionConfigFactory,
                config_component: MeterResetCronActionConfigComponent,
                table_row: MeterResetCronActionComponent,
                name: __("meter.content.meter_reset"),
                require_feature: "meter",
            },
        },
    };
}
