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

import { CronTriggerID } from "../cron/cron_defs";
import { CronTrigger } from "../cron/types"
import { h } from 'preact'
import { Cron } from "../cron/main"
import { __ } from "../../ts/translation"
import { InputSelect } from "../../ts/components/input_select"
import { FormRow } from "../../ts/components/form_row"

export type EvseShutdownCronTrigger = [
    CronTriggerID.EVSEShutdownInput,
    {
        high: boolean;
    },
];

export type EvseGpInputCronTrigger = [
    CronTriggerID.EVSEGPInput,
    {
        high: boolean;
    },
];

export type EvseButtonCronTrigger = [
    CronTriggerID.EVSEButton,
    {
        button_pressed: boolean;
    },
];

function get_evse_button_table_children(trigger: CronTrigger) {
    const value = (trigger as EvseButtonCronTrigger)[1];
    return __("evse.content.cron_button_trigger_text")(value.button_pressed);
}

function get_evse_button_edit_children(cron: Cron, trigger: CronTrigger) {
    let value = (trigger as EvseButtonCronTrigger)[1];
    return [
        {
            name: __("evse.content.button_configuration"),
            value: <InputSelect
                items={[
                    ["0", __("evse.content.button_released")],
                    ["1", __("evse.content.button_pressed")]
                ]}
            value={value.button_pressed ? "1": "0"}
            onValue={(v) => {
                value.button_pressed = v == "1";
                cron.setTriggerFromComponent(trigger);
            }}/>
        }
    ]
}

function new_evse_button_config(): CronTrigger {
    return [
        CronTriggerID.EVSEButton,
        {
            button_pressed: true,
        },
    ];
}

function get_evse_shutdown_table_children(trigger: CronTrigger) {
    const value = (trigger as EvseShutdownCronTrigger)[1];
    return __("evse.content.cron_sd_trigger_text")(value.high);
}

function get_evse_shutdown_edit_children(cron: Cron, trigger: CronTrigger) {
    const value = (trigger as EvseShutdownCronTrigger)[1];
    return [
        {
            name: "",
            value: <InputSelect
                items={[
                    ["0", __("evse.content.cron_trigger_active_low")],
                    ["1", __("evse.content.cron_trigger_active_high")]
                ]}
                value={value.high ? "1" : "0"}
                onValue={(v) => {
                    value.high = v == "1";
                    cron.setTriggerFromComponent(trigger);
                }} />
        }
    ]
}

function new_evse_shutdown_input_config(): CronTrigger {
    return [
        CronTriggerID.EVSEShutdownInput,
        {
            high: true,
        },
    ];
}

function get_evse_gp_input_table_children(trigger: CronTrigger) {
    const value = (trigger as EvseGpInputCronTrigger)[1];
    return __("evse.content.cron_gpin_trigger_text")(value.high);
}

function get_evse_gp_input_edit_children(cron: Cron, trigger: CronTrigger) {
    const value = (trigger as EvseGpInputCronTrigger)[1];
    return [
        {
            name: "",
            value: <InputSelect
                items={[
                    ["0", __("evse.content.cron_trigger_active_low")],
                    ["1", __("evse.content.cron_trigger_active_high")]
                ]}
                value={value.high ? "1" : "0"}
                onValue={(v) => {
                    value.high = v == "1";
                    cron.setTriggerFromComponent(trigger);
                }} />
        }
    ]
}

function new_evse_gp_input_config(): CronTrigger {
    return [
        CronTriggerID.EVSEGPInput,
        {
            high: true,
        },
    ];
}

export function init() {
    return {
        trigger_components: {
            [CronTriggerID.EVSEButton]: {
                name: __("evse.content.cron_trigger_button"),
                new_config: new_evse_button_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_edit_children: get_evse_button_edit_children,
                get_table_rchildren: get_evse_button_table_children,
                require_feature: "button_configuration",
            },
            [CronTriggerID.EVSEShutdownInput]: {
                name: __("evse.content.cron_trigger_shutdown_input"),
                new_config: new_evse_shutdown_input_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_edit_children: get_evse_shutdown_edit_children,
                get_table_children: get_evse_shutdown_table_children,
                require_feature: "button_configuration",
            },
            [CronTriggerID.EVSEGPInput]: {
                name: __("evse.content.cron_trigger_gp_input"),
                new_config: new_evse_gp_input_config,
                clone_config: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                get_edit_children: get_evse_gp_input_edit_children,
                get_table_children: get_evse_gp_input_table_children,
                require_feature: "button_configuration",
            },
        },
    };
}
