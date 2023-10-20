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
import { VNode, h } from 'preact'
import { Cron } from "../cron/main"
import { __ } from "../../ts/translation"
import { InputSelect } from "../../ts/components/input_select"

export type EvseShutdownCronTrigger = [
    CronTriggerID.EVSEShutdownInput,
    {
        high: boolean;
    },
];

export type EvseGpioCronTrigger = [
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

function EvseButtonCronTriggerComponent(trigger: CronTrigger): VNode {
    const value = (trigger as EvseButtonCronTrigger)[1];
    return __("evse.content.cron_button_trigger_text")(value.button_pressed);
}

function EvseButtonCronTriggerConfig(cron: Cron, trigger: CronTrigger) {
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

function EvseButtonCronTriggerFactory(): CronTrigger {
    return [
        CronTriggerID.EVSEButton,
        {
            button_pressed: true,
        },
    ];
}

function EvseShutdownTriggerComponent(trigger: CronTrigger): VNode {
    const value = (trigger as EvseShutdownCronTrigger)[1];
    return __("evse.content.cron_sd_trigger_text")(value.high);
}

function EvseShutdownTriggerConfig(cron: Cron, trigger: CronTrigger) {
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

function EvseShutdownTriggerFactory(): CronTrigger {
    return [
        CronTriggerID.EVSEShutdownInput,
        {
            high: true,
        },
    ];
}

function EvseGpioInputCronTriggerComponent(trigger: CronTrigger): VNode {
    const value = (trigger as EvseGpioCronTrigger)[1];
    return __("evse.content.cron_gpin_trigger_text")(value.high);
}

function EvseGpioInputCrontTriggerConfigComponent(cron: Cron, trigger: CronTrigger) {
    const value = (trigger as EvseGpioCronTrigger)[1];
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

function EvseGpioInputCronTriggerConfigFactory(): CronTrigger {
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
                clone: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                config_builder: EvseButtonCronTriggerFactory,
                config_component: EvseButtonCronTriggerConfig,
                table_row: EvseButtonCronTriggerComponent,
                name: __("evse.content.cron_trigger_button"),
                require_feature: "button_configuration",
            },
            [CronTriggerID.EVSEShutdownInput]: {
                clone: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                config_component: EvseShutdownTriggerConfig,
                table_row: EvseShutdownTriggerComponent,
                config_builder: EvseShutdownTriggerFactory,
                name: __("evse.content.cron_trigger_gpio_shutdown"),
                require_feature: "button_configuration",
            },
            [CronTriggerID.EVSEGPInput]: {
                clone: (trigger: CronTrigger) => [trigger[0], {...trigger[1]}] as CronTrigger,
                config_builder: EvseGpioInputCronTriggerConfigFactory,
                config_component: EvseGpioInputCrontTriggerConfigComponent,
                table_row: EvseGpioInputCronTriggerComponent,
                name: __("evse.content.cron_trigger_gpio_in"),
                require_feature: "button_configuration",
            },
        },
    };
}
