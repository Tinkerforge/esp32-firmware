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
import { ComponentChildren } from "preact";

type CronConfig = API.getType["cron/config"];
export type Task = CronConfig["tasks"][0];
export type CronTrigger = Task["trigger"];
export type CronAction = Task["action"];

export interface CronTriggerComponent {
    name: string,
    new_config: () => CronTrigger,
    clone_config: (trigger: CronTrigger) => CronTrigger,
    get_edit_children: (trigger: CronTrigger, on_trigger: (trigger: CronTrigger) => void) => ComponentChildren,
    get_table_children: (trigger: CronTrigger) => ComponentChildren,
    require_feature?: string,
}

export interface CronActionComponent {
    name: string,
    new_config: () => CronAction,
    clone_config: (action: CronAction) => CronAction,
    get_edit_children: (action: CronAction, on_action: (action: CronAction) => void) => ComponentChildren,
    get_table_children: (action: CronAction) => ComponentChildren,
    require_feature?: string,
}

export type CronTriggerComponents = {[key: number]: CronTriggerComponent};
export type CronActionComponents = {[key: number]: CronActionComponent};
