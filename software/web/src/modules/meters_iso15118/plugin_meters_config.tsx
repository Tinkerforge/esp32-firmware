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

//#include "generated/module_available.inc"

import { h, ComponentChildren } from "preact";
import { __, translate_unchecked } from "../../ts/translation";
import * as util from "../../ts/util";
import { MeterClassID } from "../meters/generated/meter_class_id.enum";
import { MeterLocation } from "../meters/generated/meter_location.enum";
import { translate_meter_location } from "../meters/meter_location";
import { MeterConfig } from "../meters/types";
import { InputText } from "../../ts/components/input_text";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import { EVDataSource } from "./generated/ev_data_source.enum";
import * as API from "../../ts/api";

export type ISO15118MetersConfig = [
    MeterClassID.ISO15118,
    {
        display_name: string;
        location: number;
        excluded: boolean;
    },
];

function translate_display_name(display_name: string) {
    return display_name == "Vehicle" ? __("meters_iso15118.content.vehicle_display_name") : display_name;
}

export function pre_init() {
    return {
        [MeterClassID.ISO15118]: {
            name: () => __("meters_iso15118.content.meter_class"),
            new_config: () => [MeterClassID.ISO15118, {display_name: "Vehicle", location: MeterLocation.Vehicle, excluded: false}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: ISO15118MetersConfig, on_config: (config: ISO15118MetersConfig) => void): ComponentChildren => {
                return [
                    <FormRow label={__("meters_iso15118.content.config_display_name")}>
                        <InputText value={translate_display_name(config[1].display_name)} />
                    </FormRow>,
                    <FormRow label={__("meters_iso15118.content.config_location")}>
                        <InputText value={translate_meter_location(config[1].location)} />
                    </FormRow>,
//#if MODULE_EM_ENERGY_ANALYSIS_AVAILABLE
                    <FormRow label={__("meters.content.config_excluded")} help={__("meters.content.config_excluded_help")}>
                        <Switch
                            desc={__("meters.content.config_excluded_desc")}
                            checked={config[1].excluded}
                            onClick={() => on_config(util.get_updated_union(config, {excluded: !config[1].excluded}))}/>
                    </FormRow>,
//#endif
                ];
            },
            get_extra_rows: (meter_slot: number) => {
                let source = API.get_unchecked(`meters/${meter_slot}/state`)?.source;
                let source_string: string;
                if (source == null) {
                    source_string = __("meters.script.reboot_required");
                } else if (source == EVDataSource.None) {
                    let charger_state = API.get("evse/state").charger_state;
                    source_string = charger_state == 0
                        ? __("meters_iso15118.script.source_no_ev")
                        : translate_unchecked(`meters_iso15118.script.source_${source}`);
                } else {
                    source_string = translate_unchecked(`meters_iso15118.script.source_${source}`);
                }

                return <FormRow label={__("meters_iso15118.content.source")} small>
                    <div class="row gx-2 gy-1">
                        <div class="col-sm-4">
                            <InputText class="form-control-sm" value={source_string}/>
                        </div>
                    </div>
                </FormRow>
            },
            translate_display_name: translate_display_name,
        },
    };
}

export function init() {
}
