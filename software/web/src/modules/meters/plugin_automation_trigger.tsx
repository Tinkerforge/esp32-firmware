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

import * as API from "../../ts/api";
import * as options from "../../options";
import { h } from "preact";
import { __ } from "../../ts/translation";
import { AutomationTriggerID } from "../automation/generated/automation_trigger_id.enum";
import { AutomationTrigger, PreInitResult } from "../automation/types";
import { FormRow } from "../../ts/components/form_row";
import { InputFloat } from "../../ts/components/input_float";
import { InputSelect } from "../../ts/components/input_select";
import { METER_VALUE_INFOS } from "./generated/meter_value_id";
import { translate_unchecked } from "../../ts/translation";
import * as util from "../../ts/util";

export type MeterValueAutomationTrigger = [
    AutomationTriggerID.MeterValue,
    {
        meter_slot: number,
        value_id: number,
        comparator: number,
        threshold: number,
        hysteresis: number,
    },
];

function get_meter_name(slot: number): string {
    const meter = API.get_unchecked(`meters/${slot}/config`);
    if (meter[1]) {
        return meter[1].display_name;
    }
    return __("meters.content.unknown_slot")(slot) as string;
}

function get_value_id_name(value_id: number): string {
    if (!(value_id in METER_VALUE_INFOS)) {
        return `ID ${value_id}`;
    }

    let name = translate_unchecked(`meters.content.value_${value_id}`);
    let name_muted = translate_unchecked(`meters.content.value_${value_id}_muted`);

    if (name_muted.length > 0) {
        name += "; " + name_muted;
    }

    return name;
}

function get_comparator_text(comparator: number): string {
    switch (comparator) {
        case 0: return ">";
        case 1: return "<";
        case 2: return ">=";
        case 3: return "<=";
        case 4: return "==";
        case 5: return "!=";
        default: return "?";
    }
}

function get_meter_value_table_children(trigger: MeterValueAutomationTrigger) {
    const meter_name = get_meter_name(trigger[1].meter_slot);
    const value_name = get_value_id_name(trigger[1].value_id);
    const comparator = get_comparator_text(trigger[1].comparator);
    const info       = METER_VALUE_INFOS[trigger[1].value_id];
    const unit       = info ? info.unit : "";
    const digits     = info ? info.digits : 3;
    const threshold  = trigger[1].threshold.toFixed(digits);
    const hysteresis = trigger[1].hysteresis > 0 ? trigger[1].hysteresis.toFixed(digits) : "";

    return __("meters.automation.automation_trigger_text")(meter_name, value_name, comparator, threshold, hysteresis, unit);
}

function get_value_id_items(meter_slot: number): [string, string][] {
    const value_ids = API.get_unchecked(`meters/${meter_slot}/value_ids`);
    if (!value_ids) {
        return [];
    }

    let items: [string, string][] = [];
    for (let i = 0; i < value_ids.length; i++) {
        const vid = value_ids[i];
        const info = METER_VALUE_INFOS[vid];
        if (info) {
            let name = get_value_id_name(vid);
            if (info.unit.length > 0) {
                name += " [" + info.unit + "]";
            }
            items.push([vid.toString(), name]);
        } else {
            items.push([vid.toString(), `ID ${vid}`]);
        }
    }
    return items;
}

function get_meter_value_edit_children(trigger: MeterValueAutomationTrigger, on_trigger: (trigger: AutomationTrigger) => void) {
    let meter_items: [string, string][] = [];
    for (let i = 0; i < options.METERS_MAX_SLOTS; i++) {
        const meter = API.get_unchecked(`meters/${i}/config`);
        if (meter[1]) {
            meter_items.push([i.toString(), meter[1].display_name]);
        }
    }

    const value_id_items = get_value_id_items(trigger[1].meter_slot);
    const info = METER_VALUE_INFOS[trigger[1].value_id];
    const unit = info ? info.unit : "";
    const digits = info ? info.digits : 3;

    return [
        <FormRow label={__("meters.automation.meter_slot")}>
            <InputSelect
                items={meter_items}
                value={trigger[1].meter_slot.toString()}
                onValue={(v) => {
                    const new_slot = parseInt(v);
                    const new_value_ids = get_value_id_items(new_slot);
                    const new_value_id = new_value_ids.length > 0 ? parseInt(new_value_ids[0][0]) : 0;
                    on_trigger(util.get_updated_union(trigger, {meter_slot: new_slot, value_id: new_value_id}));
                }}
            />
        </FormRow>,
        <FormRow label={__("meters.automation.value_id")}>
            <InputSelect
                items={value_id_items}
                value={trigger[1].value_id.toString()}
                onValue={(v) => on_trigger(util.get_updated_union(trigger, {value_id: parseInt(v)}))}
            />
        </FormRow>,
        <FormRow label={__("meters.automation.comparator")}>
            <InputSelect
                items={[
                    ['0', __("meters.automation.comparator_gt")],
                    ['1', __("meters.automation.comparator_lt")],
                    ['2', __("meters.automation.comparator_gte")],
                    ['3', __("meters.automation.comparator_lte")],
                    ['4', __("meters.automation.comparator_eq")],
                    ['5', __("meters.automation.comparator_neq")],
                ]}
                value={trigger[1].comparator.toString()}
                onValue={(v) => on_trigger(util.get_updated_union(trigger, {comparator: parseInt(v)}))}
            />
        </FormRow>,
        <FormRow label={__("meters.automation.threshold")}>
            <InputFloat
                required
                digits={digits}
                min={-2147483}
                max={2147483}
                value={Math.round(trigger[1].threshold * Math.pow(10, digits))}
                onValue={(v) => on_trigger(util.get_updated_union(trigger, {threshold: v / Math.pow(10, digits)}))}
                unit={unit}
            />
        </FormRow>,
        <FormRow label={__("meters.automation.hysteresis")} label_muted={__("meters.automation.hysteresis_muted")}>
            <InputFloat
                required
                digits={digits}
                min={0}
                max={2147483}
                value={Math.round(trigger[1].hysteresis * Math.pow(10, digits))}
                onValue={(v) => on_trigger(util.get_updated_union(trigger, {hysteresis: v / Math.pow(10, digits)}))}
                unit={unit}
            />
        </FormRow>,
    ];
}

function new_meter_value_config(): AutomationTrigger {
    let meter_slot = 0;
    let value_id = 0;

    // Find the first configured meter and its first value ID
    for (let i = 0; i < options.METERS_MAX_SLOTS; i++) {
        const meter = API.get_unchecked(`meters/${i}/config`);
        if (meter[1]) {
            meter_slot = i;
            const value_ids = API.get_unchecked(`meters/${i}/value_ids`);
            if (value_ids && value_ids.length > 0) {
                value_id = value_ids[0];
            }
            break;
        }
    }

    return [
        AutomationTriggerID.MeterValue,
        {
            meter_slot: meter_slot,
            value_id: value_id,
            comparator: 0,
            threshold: 0,
            hysteresis: 0,
        },
    ];
}

export function pre_init(): PreInitResult {
    return {
        trigger_components: {
            [AutomationTriggerID.MeterValue]: {
                translation_name: () => __("meters.automation.meter_value"),
                new_config: new_meter_value_config,
                clone_config: (trigger: AutomationTrigger) => [trigger[0], {...trigger[1]}] as AutomationTrigger,
                get_table_children: get_meter_value_table_children,
                get_edit_children: get_meter_value_edit_children,
            },
        },
    };
}

export function init() {
}
