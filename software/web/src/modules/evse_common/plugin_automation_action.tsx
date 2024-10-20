/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

import { h } from "preact";
import { __ } from "../../ts/translation";
import { AutomationActionID } from "../automation/automation_action_id.enum";
import { AutomationAction, InitResult } from "../automation/types";
import { InputSelect } from "../../ts/components/input_select";
import { InputFloat } from "../../ts/components/input_float";
import { InputNumber } from "../../ts/components/input_number";
import { FormRow } from "../../ts/components/form_row";
import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { Collapse } from "react-bootstrap";

export type EvseAutomationAction = [
    AutomationActionID.SetCurrent,
    {
        current: number;
    },
];

export type EvseLedAutomationAction = [
    AutomationActionID.LED,
    {
        indication: number;
        duration: number;
        color_h: number;
        color_s: number;
        color_v: number;
    },
];

function get_set_current_table_children(action: EvseAutomationAction) {
    return __("evse.automation.automation_action_text")(util.toLocaleFixed(action[1].current / 1000, 3));
}

function get_set_current_edit_children(action: EvseAutomationAction, on_action: (action: AutomationAction) => void) {
    const items: [string, string][] = [
        ["0", __("evse.automation.automation_action_block")],
        ["32000", __("evse.automation.automation_action_allow")],
        ["6000", __("evse.automation.automation_action_limit_current")]
    ];

    return [
        <FormRow label="">
            <InputSelect
                items={items}
                value={action[1].current === 0 ? "0" : action[1].current === 32000 ? "32000" : "6000"}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {current: parseInt(v)}));
                }} />
        </FormRow>,
        <Collapse in={action[1].current !== 0 && action[1].current !== 32000}>
                <div>
                <FormRow label={__("evse.automation.allowed_charging_current")}>
                    <InputFloat
                        digits={3}
                        min={6000}
                        max={32000}
                        unit="A"
                        value={action[1].current}
                        onValue={(v) => {
                            on_action(util.get_updated_union(action, {current: v}));
                        }}
                    />
                </FormRow>
            </div>
        </Collapse>,
    ];
}

function new_set_current_config(): AutomationAction {
    return [
        AutomationActionID.SetCurrent,
        {
            current: 0,
        },
    ];
}

function hsvToHex(x: {color_h: number, color_s: number, color_v: number}) {
    let hsv: [number, number, number] = [x.color_h / 359, x.color_s / 255, x.color_v / 255]
    let rgb = util.hsvToRgb(...hsv);
    return util.rgbToHex(...rgb);
}

function hexToHsv(hex: string) {
    let rgb = util.hexToRgb(hex);
    let hsv = util.rgbToHsv(rgb.r, rgb.g, rgb.b);
    return {color_h: Math.round(hsv[0] * 359), color_s: Math.round(hsv[1] * 255), color_v: Math.round(hsv[2] * 255)};
}

function get_led_table_children(action: EvseLedAutomationAction) {
    let indication_text = "";
    switch (action[1].indication) {
        case 0:
            indication_text = __("evse.automation.led_indication_off");
            break;

        case 255:
            indication_text = __("evse.automation.led_indication_on");
            break;

        case 1001:
            indication_text = __("evse.automation.led_indication_blinking");
            break;

        case 1002:
            indication_text = __("evse.automation.led_indication_flickering");
            break;

        case 1003:
            indication_text = __("evse.automation.led_indication_breathing");
            break;
    }
    if (action[1].indication > 2000 && action[1].indication < 2011) {
        indication_text = __("evse.automation.led_indication_error")(action[1].indication - 2000);
    }

    return __("evse.automation.automation_led_action_text")(action[1].indication, indication_text, action[1].duration, API.hasFeature("rgb_led") ? hsvToHex(action[1]) : "");
}

function get_led_edit_children(action: EvseLedAutomationAction, on_action: (action: AutomationAction) => void) {
    const items: [string, string][] = [
        ["0", __("evse.automation.led_indication_off")],
        ["255", __("evse.automation.led_indication_on")],
        ["1001", __("evse.automation.led_indication_blinking")],
        ["1002", __("evse.automation.led_indication_flickering")],
        ["1003", __("evse.automation.led_indication_breathing")],
    ];

    for (let i = 1; i <= 10; i++) {
        items.push([String(2000 + i), __("evse.automation.led_indication_error")(i)]);
    }

    let result = [
        <FormRow label={__("evse.automation.indication")}>
            <InputSelect
                items={items}
                value={action[1].indication.toString()}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {indication: parseInt(v)}));
                }} />
        </FormRow>,
        <FormRow label={ __("evse.automation.led_duration")}>
            <InputNumber
                min={1}
                max={60}
                value={action[1].duration / 1000}
                unit="s"
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {duration: v * 1000}));
                }} />
        </FormRow>
    ];

    if (API.hasFeature("rgb_led")) {
        result.push(
            <FormRow label={__("evse.automation.color")}>
                <input class="form-control" type="color" value={hsvToHex(action[1])} onInput={(event) => {
                    // Get current color value from the HTML element and create new config
                    //let hsv_scaled = {color_h: hsv[0] * 359, color_s: hsv[1] * 255, color_v: hsv[2] * 255};
                    let hsv = hexToHsv((event.target as HTMLInputElement).value.toString())
                    on_action(util.get_updated_union(action, hsv));
                }} />
            </FormRow>
        )
    }

    return result;
}

function new_led_config(): AutomationAction {
    return [
        AutomationActionID.LED,
        {
            duration: 1000,
            indication: 0,
            color_h: 0,
            color_s: 0,
            color_v: 0,
        },
    ];
}

export function init(): InitResult {
    return {
        action_components: {
            [AutomationActionID.SetCurrent]: {
                translation_name: () => __("evse.automation.action_allowed_charging_current"),
                new_config: new_set_current_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_edit_children: get_set_current_edit_children,
                get_table_children: get_set_current_table_children,
            },
            [AutomationActionID.LED]: {
                translation_name: () => __("evse.automation.led_indication"),
                new_config: new_led_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_edit_children: get_led_edit_children,
                get_table_children: get_led_table_children,
            },
        },
    };
}
