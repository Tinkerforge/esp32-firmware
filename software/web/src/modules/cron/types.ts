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

import * as API from "../../ts/api";
import { ComponentChild, VNode } from "preact";
import { Cron } from "./main";
import { TableModalRow } from "../../ts/components/table";

type CronConfig = API.getType["cron/config"];
export type Task = CronConfig["tasks"][0];
export type CronTrigger = Task["trigger"];
export type CronAction = Task["action"];

export interface CronTriggerComponent {
    clone: (trigger: CronTrigger) => CronTrigger,
    table_row: (trigger: CronTrigger) => VNode,
    config_component: (cron: Cron, trigger: CronTrigger) => TableModalRow[],
    config_builder: () => CronTrigger,
    name: string,
    require_feature?: string
}

export interface CronActionComponent {
    clone: (action: CronAction) => CronAction,
    table_row: (action: CronAction) => VNode,
    config_component: (cron: Cron, action: CronAction) => TableModalRow[],
    config_builder: () => CronAction,
    name: string,
    require_feature?: string
}

export type CronTriggerComponents = {[key: number]: CronTriggerComponent};
export type CronActionComponents = {[key: number]: CronActionComponent};
