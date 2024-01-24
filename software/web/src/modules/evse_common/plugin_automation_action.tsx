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

import { h, Fragment } from "preact";
import { __ } from "../../ts/translation";
import { AutomationActionID } from "../automation/automation_defs";
import { AutomationAction } from "../automation/types";
import { InputSelect } from "../../ts/components/input_select";
import { InputFloat } from "../../ts/components/input_float";
import { InputNumber } from "../../ts/components/input_number";
import { FormRow } from "../../ts/components/form_row";
import * as util from "../../ts/util";
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
    },
];

function get_set_current_table_children(action: EvseAutomationAction) {
    return __("evse.automation.automation_action_text")((action[1].current / 1000).toLocaleString());
}

function get_set_current_edit_children(action: EvseAutomationAction, on_action: (action: AutomationAction) => void) {
    const items: [string, string][] = [
        ["0", __("evse.automation.automation_action_block")],
        ["32000", __("evse.automation.automation_action_allow")],
        ["6000", __("evse.automation.automation_action_limit_current")]
    ];

    return [
        <>
            <FormRow label="">
                <InputSelect
                    items={items}
                    value={action[1].current === 0 ? "0" : action[1].current === 32000 ? "32000" : "6000"}
                    onValue={(v) => {
                        on_action(util.get_updated_union(action, {current: parseInt(v)}));
                    }} />
            </FormRow>
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
            </Collapse>
        </>,
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

function get_led_table_children(action: EvseLedAutomationAction) {
    let indication = "";
    switch (action[1].indication) {
        case 0:
            indication = __("evse.automation.led_indication_off");
            break;

        case 255:
            indication = __("evse.automation.led_indication_on");
            break;

        case 1001:
            indication = __("evse.automation.led_indication_blinking");
            break;

        case 1002:
            indication = __("evse.automation.led_indication_flickering");
            break;

        case 1003:
            indication = __("evse.automation.led_indication_breathing");
            break;
    }
    if (action[1].indication > 2000 && action[1].indication < 2011) {
        indication = __("evse.automation.led_indication_error")(action[1].indication - 2000);
    }

    return __("evse.automation.automation_led_action_text")(indication, action[1].duration);
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

    return [<>
        <FormRow label={__("evse.automation.indication")}>
            <InputSelect
                items={items}
                value={action[1].indication.toString()}
                onValue={(v) => {
                    on_action(util.get_updated_union(action, {indication: parseInt(v)}));
                }} />
        </FormRow>
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
    </>];
}

function new_led_config(): AutomationAction {
    return [
        AutomationActionID.LED,
        {
            duration: 1000,
            indication: 0,
        },
    ];
}

export function init() {
    return {
        action_components: {
            [AutomationActionID.SetCurrent]: {
                name: __("evse.automation.action_allowed_charging_current"),
                new_config: new_set_current_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_edit_children: get_set_current_edit_children,
                get_table_children: get_set_current_table_children,
            },
            [AutomationActionID.LED]: {
                name: __("evse.automation.led_indication"),
                new_config: new_led_config,
                clone_config: (action: AutomationAction) => [action[0], {...action[1]}] as AutomationAction,
                get_edit_children: get_led_edit_children,
                get_table_children: get_led_table_children,
            },
        },
    };
}
