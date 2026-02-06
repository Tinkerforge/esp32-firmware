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
import { __, translate_unchecked } from "../../ts/translation";
import * as util from "../../ts/util";
import { MeterClassID } from "../meters/meter_class_id.enum";
import { MeterLocation } from "../meters/meter_location.enum";
import { MeterConfig } from "../meters/types";
import { InputText } from "../../ts/components/input_text";
import { FormRow } from "../../ts/components/form_row";
import { EVDataProtocol } from "./ev_data_protocol.enum";
import * as API from "../../ts/api";

export type EVMetersConfig = [
    MeterClassID.ISO15118,
    {
        display_name: string;
        location: number;
    },
];

export function pre_init() {
    return {
        [MeterClassID.ISO15118]: {
            name: () => __("meters_ev.content.meter_class"),
            new_config: () => [MeterClassID.ISO15118, {display_name: "", location: MeterLocation.EV}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: EVMetersConfig, on_config: (config: EVMetersConfig) => void): ComponentChildren => {
                return [
                    <FormRow label={__("meters_ev.content.config_display_name")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {display_name: v}));
                            }}
                        />
                    </FormRow>,
                ];
            },
            get_extra_rows: (meter_slot: number) => {
                let protocol = API.get_unchecked(`meters/${meter_slot}/state`)?.protocol;
                let protocol_string = protocol == null ? __("meters.script.reboot_required") : translate_unchecked(`meters_ev.script.protocol_${protocol}`)

                return <FormRow label={__("meters_ev.content.protocol")} small>
                    <div class="row"><div class="col-sm-4">
                        <InputText class="form-control-sm" value={protocol_string}/>
                    </div></div>
                </FormRow>
            },
        },
    };
}

export function init() {
}
