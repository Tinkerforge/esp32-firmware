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

type AutomationConfig = API.getType["automation/config"];
export type Task = AutomationConfig["tasks"][0];
export type AutomationTrigger = Task["trigger"];
export type AutomationAction = Task["action"];

export interface AutomationTriggerComponent {
    name: string,
    new_config: () => AutomationTrigger,
    clone_config: (trigger: AutomationTrigger) => AutomationTrigger,
    get_edit_children: (trigger: AutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) => ComponentChildren,
    get_table_children: (trigger: AutomationTrigger) => ComponentChildren,
    get_disabled_reason?: (trigger: AutomationTrigger) => string,
}

export interface AutomationActionComponent {
    name: string,
    new_config: () => AutomationAction,
    clone_config: (action: AutomationAction) => AutomationAction,
    get_edit_children: (action: AutomationAction, on_action: (action: AutomationAction) => void) => ComponentChildren,
    get_table_children: (action: AutomationAction) => ComponentChildren,
    get_disabled_reason?: (action: AutomationAction) => string,
}

export type AutomationTriggerComponents = {[key: number]: AutomationTriggerComponent};
export type AutomationActionComponents = {[key: number]: AutomationActionComponent};

export type InitResult = {trigger_components?: AutomationTriggerComponents, action_components?: AutomationActionComponents};
