/* esp32-firmware
 * Copyright (C) 2026 Mattias Schäffersmann <mattias@tinkerforge.com>
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
import { __ } from "../../ts/translation";
import * as util from "../../ts/util";
import { FormRow     } from "../../ts/components/form_row";
import { InputHost   } from "../../ts/components/input_host";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { InputText   } from "../../ts/components/input_text";
import { Switch      } from "../../ts/components/switch";
import { MeterClassID             } from "../meters/generated/meter_class_id.enum";
import { MeterLocation            } from "../meters/generated/meter_location.enum";
import { translate_meter_location } from "../meters/meter_location";
import { MeterConfig              } from "../meters/types";

export type SBMetersConfig = [
    MeterClassID.Sonnenbatterie,
    {
        display_name: string;
        location: number;
        excluded: boolean;
        host: string;
        port: number;
        virtual_meter: number;
    },
];

export function pre_init() {
    return {
        [MeterClassID.Sonnenbatterie]: {
            name: () => __("meters_sonnenbatterie.content.meter_class"),
            new_config: () => [MeterClassID.Sonnenbatterie, {display_name: "", location: MeterLocation.Unknown, excluded: false, host: "", port: 80, virtual_meter: null}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: SBMetersConfig, on_config: (config: SBMetersConfig) => void): ComponentChildren => {
                const virtual_meter_items: [string, string][] = [
                    [MeterLocation.Grid.toString(),    translate_meter_location(MeterLocation.Grid   )],
                    [MeterLocation.Battery.toString(), translate_meter_location(MeterLocation.Battery)],
                ];

                return [
                    <FormRow label={__("meters_sonnenbatterie.content.display_name")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => on_config(util.get_updated_union(config, {display_name: v}))}
                        />
                    </FormRow>,

                    <FormRow label={__("meters_sonnenbatterie.content.host")}>
                        <InputHost
                            required
                            maxLength={64}
                            value={config[1].host}
                            onValue={(v) => on_config(util.get_updated_union(config, {host: v}))}
                        />
                    </FormRow>,

                    <FormRow label={__("meters_sonnenbatterie.content.port")} label_muted={__("meters_sonnenbatterie.content.port_muted")}>
                        <InputNumber
                            required
                            min={1}
                            max={65535}
                            value={config[1].port}
                            onValue={(v) => on_config(util.get_updated_union(config, {port: v}))}
                        />
                    </FormRow>,

                    <FormRow label={__("meters_sonnenbatterie.content.virtual_meter")}>
                        <InputSelect
                            required
                            items={virtual_meter_items}
                            placeholder={__("select")}
                            value={util.hasValue(config[1].virtual_meter) ? config[1].virtual_meter.toString() : null}
                            onValue={(v) => {
                                const intval = parseInt(v);
                                on_config(util.get_updated_union(config, {virtual_meter: intval, location: intval}));
                            }}
                        />
                    </FormRow>,

                    <FormRow label={__("meters_sonnenbatterie.content.location")}>
                        <InputText value={config[1].location == MeterLocation.Unknown ? __("meters_sonnenbatterie.content.location_depends_virtual_meter") : translate_meter_location(config[1].location)} />
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
        },
    };
}

export function init() {
}
