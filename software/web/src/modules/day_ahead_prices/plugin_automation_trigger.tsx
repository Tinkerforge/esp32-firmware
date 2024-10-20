/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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
import { AutomationTrigger, InitResult } from "../automation/types";
import { FormRow } from "../../ts/components/form_row";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import * as util from "../../ts/util";

const enum DayAheadPriceNowTriggerType {
    AVERAGE = 0,
    ABSOLUTE
};

const enum DayAheadPriceNowTriggerComparison {
    GREATER = 0,
    LESS
};

export type DayAheadPriceNowAutomationTrigger = [
    AutomationTriggerID.DayAheadPriceNow,
    {
        type: DayAheadPriceNowTriggerType,
        comparison: DayAheadPriceNowTriggerComparison,
        value: number
    },
];

function get_day_ahead_prices_now_table_children(trigger: DayAheadPriceNowAutomationTrigger) {
    return __("day_ahead_prices.automation.trigger_text")(trigger[1].type, trigger[1].comparison, trigger[1].value);
}

function get_day_ahead_prices_now_edit_children(trigger: DayAheadPriceNowAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void): ComponentChildren {
    return [
        <FormRow label={__("day_ahead_prices.automation.comparative_value")} label_muted={__("day_ahead_prices.automation.comparative_value_muted")}>
            <InputSelect
                items={[
                    ['0', __("day_ahead_prices.automation.comparative_value_percent")],
                    ['1', __("day_ahead_prices.automation.comparative_value_absolute")]
                ]}
                value={trigger[1].type.toString()}
                onValue={(v) => {
                    on_trigger(util.get_updated_union(trigger, {type: parseInt(v)}));
                }}
            />
        </FormRow>,
        <FormRow label={__("day_ahead_prices.automation.comparison")}>
            <InputSelect
                items={[
                    ['0', __("day_ahead_prices.automation.comparison_greater_than")],
                    ['1', __("day_ahead_prices.automation.comparison_less_than")]
                ]}
                value={trigger[1].comparison.toString()}
                onValue={(v) => on_trigger(util.get_updated_union(trigger, {comparison: parseInt(v)}))}
            />
        </FormRow>,
        <FormRow label={__("day_ahead_prices.automation.value")}>
            <InputNumber
                required
                min={-10000}
                max={10000}
                value={trigger[1].value}
                onValue={(v) => on_trigger(util.get_updated_union(trigger, {value: v}))}
                unit={trigger[1].type == DayAheadPriceNowTriggerType.AVERAGE ? '%' : 'ct'}
            />
        </FormRow>,
    ]
}

function new_day_ahead_prices_now_config(): AutomationTrigger {
    return [
        AutomationTriggerID.DayAheadPriceNow,
        {
            type: 0,
            comparison: 0,
            value: 100
        },
    ];
}

export function init(): InitResult {
    return {
        trigger_components: {
            [AutomationTriggerID.DayAheadPriceNow]: {
                translation_name: () => __("day_ahead_prices.automation.current_electricity_price"),
                new_config: new_day_ahead_prices_now_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_table_children: get_day_ahead_prices_now_table_children,
                get_edit_children: get_day_ahead_prices_now_edit_children,
            },
        },
    };
}
