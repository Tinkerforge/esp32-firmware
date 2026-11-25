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

export type EEBusLPPAutomationTrigger = [
    AutomationTriggerID.EEBusLPP,
    {
        limited: boolean;
    },
];

function get_eebus_lpp_table_children(trigger: EEBusLPPAutomationTrigger) {
    return __("eebus.automation.automation_trigger_text")(trigger[1].limited);
}

function get_eebus_lpp_edit_children(trigger: EEBusLPPAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    return [
        <FormRow label={__("eebus.automation.state")}>
            <InputSelect
                items={[
                    ["1", __("eebus.automation.limited")],
                    ["0", __("eebus.automation.not_limited")],
                ]}
                value={trigger[1].limited ? "1" : "0"}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {limited: v === "1"}));
                }}
            />
        </FormRow>,
    ];
}

function new_eebus_lpp_config(): AutomationTrigger {
    return [
        AutomationTriggerID.EEBusLPP,
        {
            limited: true,
        },
    ];
}

export function pre_init() {
    return {
        [AutomationTriggerID.EEBusLPP]: {
            name: () => __("eebus.automation.automation_trigger"),
            new_config: new_eebus_lpp_config,
            clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
            get_edit_children: get_eebus_lpp_edit_children,
            get_table_children: get_eebus_lpp_table_children,
            get_disabled_reason: () => __("eebus.automation.trigger_disabled"),
        },
    };
}

export function init() {
}
