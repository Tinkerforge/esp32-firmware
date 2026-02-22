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

import { h, ComponentChildren } from "preact";
import { __ } from "../../ts/translation";
import { AutomationTriggerID } from "../automation/automation_trigger_id.enum";
import { AutomationTrigger, PreInitResult } from "../automation/types";
import { FormRow } from "../../ts/components/form_row";
import { InputFloat } from "../../ts/components/input_float";
import { InputSelect } from "../../ts/components/input_select";
import * as util from "../../ts/util";

const enum TemperatureNowTriggerType {
    CURRENT = 0,
    TODAY_MIN,
    TODAY_AVG,
    TODAY_MAX,
    TOMORROW_MIN,
    TOMORROW_AVG,
    TOMORROW_MAX,
};

const enum TemperatureNowTriggerComparison {
    GREATER = 0,
    LESS,
};

export type TemperatureNowAutomationTrigger = [
    AutomationTriggerID.TemperatureNow,
    {
        type: TemperatureNowTriggerType,
        comparison: TemperatureNowTriggerComparison,
        value: number,
    },
];

function get_temperature_now_table_children(trigger: TemperatureNowAutomationTrigger) {
    return __("temperatures.automation.trigger_text")(trigger[1].type, trigger[1].comparison, trigger[1].value);
}

function get_temperature_now_edit_children(trigger: TemperatureNowAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    return [
        <FormRow label={__("temperatures.automation.temperature_type")}>
            <InputSelect
                items={[
                    ['0', __("temperatures.automation.type_current")],
                    ['1', __("temperatures.automation.type_today_min")],
                    ['2', __("temperatures.automation.type_today_avg")],
                    ['3', __("temperatures.automation.type_today_max")],
                    ['4', __("temperatures.automation.type_tomorrow_min")],
                    ['5', __("temperatures.automation.type_tomorrow_avg")],
                    ['6', __("temperatures.automation.type_tomorrow_max")],
                ]}
                value={trigger[1].type.toString()}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {type: parseInt(v)}));
                }}
            />
        </FormRow>,
        <FormRow label={__("temperatures.automation.comparison")}>
            <InputSelect
                items={[
                    ['0', __("temperatures.automation.comparison_greater_than")],
                    ['1', __("temperatures.automation.comparison_less_than")],
                ]}
                value={trigger[1].comparison.toString()}
                onValue={(v) => on_trigger(util.get_updated_union(trigger, {comparison: parseInt(v)}))}
            />
        </FormRow>,
        <FormRow label={__("temperatures.automation.value")}>
            <InputFloat
                required
                digits={1}
                min={-500}
                max={500}
                value={trigger[1].value}
                onValue={(v) => on_trigger(util.get_updated_union(trigger, {value: v}))}
                unit="°C"
            />
        </FormRow>,
    ];
}

function new_temperature_now_config(): AutomationTrigger {
    return [
        AutomationTriggerID.TemperatureNow,
        {
            type: 0,
            comparison: 0,
            value: 0,
        },
    ];
}

export function pre_init(): PreInitResult {
    return {
        trigger_components: {
            [AutomationTriggerID.TemperatureNow]: {
                translation_name: () => __("temperatures.automation.current_temperature"),
                new_config: new_temperature_now_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_table_children: get_temperature_now_table_children,
                get_edit_children: get_temperature_now_edit_children,
            },
        },
    };
}

export function init() {
}
