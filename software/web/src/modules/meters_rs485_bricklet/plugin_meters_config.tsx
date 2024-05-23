/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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
import { __, translate_unchecked } from "../../ts/translation";
import * as util from "../../ts/util";
import { MeterClassID } from "../meters/meter_class_id.enum";
import { MeterConfig } from "../meters/types";
import { InputText } from "../../ts/components/input_text";
import { FormRow } from "../../ts/components/form_row";
import * as API from "../../ts/api";
import { InputSelect } from "../../ts/components/input_select";

export type RS485BrickletMetersConfig = [
    MeterClassID.RS485Bricklet,
    {
        display_name: string;
        type_override: number;
    },
];

export function init() {
    return {
        [MeterClassID.RS485Bricklet]: {
            name: __("meters_rs485_bricklet.content.meter_class"),
            new_config: () => [MeterClassID.RS485Bricklet, {display_name: "", type_override: 255}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: RS485BrickletMetersConfig, on_config: (config: RS485BrickletMetersConfig) => void): ComponentChildren => {
                return [
                    <FormRow label={__("meters_rs485_bricklet.content.config_display_name")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {display_name: v}));
                            }}
                        />
                    </FormRow>,
                    <FormRow label={__("meters_rs485_bricklet.content.type_override")}>
                        <InputSelect items={[
                                ["255", __("meters.script.meter_type_255")],
                                ["1",   __("meters.script.meter_type_1")],
                                ["2",   __("meters.script.meter_type_2")],
                                ["3",   __("meters.script.meter_type_3")],
                                ["0",   __("meters.script.meter_type_0")],
                            ]}
                            value={config[1].type_override}
                            onValue={(v) => on_config(util.get_updated_union(config, {type_override: parseInt(v)}))}
                            />
                    </FormRow>,
                ];
            },
            get_extra_rows: (meter_slot: number) => {
                let meter_type = API.get_unchecked(`meters/${meter_slot}/state`)?.type;
                let meter_type_string = meter_type == null ? __("meters.script.reboot_required") : translate_unchecked(`meters.script.meter_type_${meter_type}`)

                return <FormRow label={__("meters.content.meter_type")} small>
                    <div class="row mx-n1 mx-xl-n3"><div class="col-sm-4 px-1 px-xl-3">
                        <InputText class="form-control-sm"
                                value={meter_type_string}/>
                    </div></div>
                </FormRow>;
            }
        },
    };
}
