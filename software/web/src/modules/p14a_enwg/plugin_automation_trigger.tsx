/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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
import { AutomationTriggerID } from "../automation/generated/automation_trigger_id.enum";
import { AutomationTrigger } from "../automation/types";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import * as util from "../../ts/util";

export type P14aEnwgAutomationTrigger = [
    AutomationTriggerID.P14aEnwg,
    {
        active: boolean;
    },
];

function get_p14a_enwg_table_children(trigger: P14aEnwgAutomationTrigger) {
    return __("p14a_enwg.automation.automation_trigger_text")(trigger[1].active);
}

function get_p14a_enwg_edit_children(trigger: P14aEnwgAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    return [
        <FormRow label={__("p14a_enwg.automation.state")}>
            <InputSelect
                items={[
                    ["1", __("p14a_enwg.automation.triggered")],
                    ["0", __("p14a_enwg.automation.not_triggered")],
                ]}
                value={trigger[1].active ? "1" : "0"}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {active: v === "1"}));
                }}
            />
        </FormRow>,
    ];
}

function new_p14a_enwg_config(): AutomationTrigger {
    return [
        AutomationTriggerID.P14aEnwg,
        {
            active: true,
        },
    ];
}

export function pre_init() {
    return {
        [AutomationTriggerID.P14aEnwg]: {
            name: () => __("p14a_enwg.automation.automation_trigger"),
            new_config: new_p14a_enwg_config,
            clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
            get_edit_children: get_p14a_enwg_edit_children,
            get_table_children: get_p14a_enwg_table_children,
            get_disabled_reason: () => __("p14a_enwg.automation.trigger_disabled"),
        },
    };
}

export function init() {
}
