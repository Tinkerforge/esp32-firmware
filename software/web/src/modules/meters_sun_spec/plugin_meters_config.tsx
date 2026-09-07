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

//#include "generated/module_available.inc"

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { h, Fragment, Component, ComponentChildren, VNode } from "preact";
import { __, translate_unchecked, removeUnicodeHacks } from "../../ts/translation";
import { MeterClassID } from "../meters/generated/meter_class_id.enum";
import { MeterLocation } from "../meters/generated/meter_location.enum";
import { get_meter_location_items, translate_meter_location } from "../meters/meter_location";
import { MeterConfig } from "../meters/types";
import { DCPortType } from "./generated/dc_port_type.enum";
import { InputText } from "../../ts/components/input_text";
import { InputHost } from "../../ts/components/input_host";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import { Button } from "react-bootstrap";
import { SunSpecDeviceScanner, SunSpecDeviceScannerResult } from "../sun_spec/device_scanner";

export type SunSpecMetersConfig = [
    MeterClassID.SunSpec,
    {
        display_name: string;
        location: number;
        excluded: boolean;
        host: string;
        port: number;
        device_address: number;
        manufacturer_name: string;
        model_name: string;
        serial_number: string;
        model_id: number;
        model_instance: number;
        dc_port_type: number;
    },
];

const MODEL_SPECS: {[model_id: string]: {fixed_location: number, is_supported: boolean}} = {
    '101': {fixed_location: MeterLocation.Inverter, is_supported: true},
    '102': {fixed_location: MeterLocation.Inverter, is_supported: true},
    '103': {fixed_location: MeterLocation.Inverter, is_supported: true},
    '111': {fixed_location: MeterLocation.Inverter, is_supported: true},
    '112': {fixed_location: MeterLocation.Inverter, is_supported: true},
    '113': {fixed_location: MeterLocation.Inverter, is_supported: true},
    '122': {fixed_location: MeterLocation.Inverter, is_supported: false},
    '160': {fixed_location: MeterLocation.PV,       is_supported: true},
    '201': {fixed_location: MeterLocation.Unknown,  is_supported: true},
    '202': {fixed_location: MeterLocation.Unknown,  is_supported: true},
    '203': {fixed_location: MeterLocation.Unknown,  is_supported: true},
    '204': {fixed_location: MeterLocation.Unknown,  is_supported: true},
    '211': {fixed_location: MeterLocation.Unknown,  is_supported: true},
    '212': {fixed_location: MeterLocation.Unknown,  is_supported: true},
    '213': {fixed_location: MeterLocation.Unknown,  is_supported: true},
    '214': {fixed_location: MeterLocation.Unknown,  is_supported: true},
    '220': {fixed_location: MeterLocation.Unknown,  is_supported: false},
    '701': {fixed_location: MeterLocation.Inverter, is_supported: true},
    '713': {fixed_location: MeterLocation.Battery,  is_supported: true},
    '714': {fixed_location: MeterLocation.Unknown,  is_supported: true},
    '802': {fixed_location: MeterLocation.Battery,  is_supported: true},
}

function get_fixed_location(model_id: number, dc_port_type: number) {
    if (!util.hasValue(model_id)) {
        return MeterLocation.Unknown;
    }

    if (model_id == 714) {
        switch (dc_port_type) {
        case DCPortType.Photovoltaic:
            return MeterLocation.PV;

        case DCPortType.EnergyStorageSystem:
            return MeterLocation.Battery;

        case DCPortType.ElectricVehicle:
            return MeterLocation.Load;

        case DCPortType.GenericInjecting:
        case DCPortType.GenericAbsorbing:
        case DCPortType.GenericBidirectional:
        case DCPortType.DCDC:
            return MeterLocation.Unknown;

        case DCPortType.NotImplemented:
            return MeterLocation.Unknown;
        }

        return MeterLocation.Unknown;
    }

    let model_spec = MODEL_SPECS[model_id];

    if (model_spec === undefined) {
        return MeterLocation.Unknown;
    }

    return model_spec.fixed_location;
}

interface EditChildrenProps {
    config: SunSpecMetersConfig;
    on_config: (config: SunSpecMetersConfig) => void;
}

interface EditChildrenState {
    manual_override: boolean;
}

class EditChildren extends Component<EditChildrenProps, EditChildrenState> {
    constructor() {
        super();

        this.state = {
            manual_override: false,
        } as any;
    }

    render() {
        let model_id_items: [string, string][] = [];

        for (let model_id of Object.keys(MODEL_SPECS)) {
            if (MODEL_SPECS[model_id].is_supported) {
                model_id_items.push([model_id, translate_unchecked(`sun_spec.content.model_${model_id}`) + ` [${model_id}]`]);
            }
        }

        let edit_children = [
            <FormRow label={__("meters_sun_spec.content.config_host")}>
                <InputHost
                    required
                    maxLength={64}
                    value={this.props.config[1].host}
                    onValue={(v) => this.props.on_config(util.get_updated_union(this.props.config, {host: v}))} />
            </FormRow>,
            <FormRow label={__("meters_sun_spec.content.config_port")} label_muted={__("meters_sun_spec.content.config_port_muted")}>
                <InputNumber
                    required
                    min={1}
                    max={65535}
                    value={this.props.config[1].port}
                    onValue={(v) => {
                        this.props.on_config(util.get_updated_union(this.props.config, {port: v}));
                    }} />
            </FormRow>,
            <hr/>,
            <SunSpecDeviceScanner host={this.props.config[1].host} port={this.props.config[1].port}
                on_is_model_visible={(model_id: number) => MODEL_SPECS[model_id] !== undefined}
                on_is_model_supported={(model_id: number) => MODEL_SPECS[model_id].is_supported}
                on_result_selected={(result: SunSpecDeviceScannerResult) => {
                    this.setState({manual_override: false});

                    this.props.on_config(util.get_updated_union(this.props.config, {
                        display_name: result.display_name,
                        location: get_fixed_location(result.model_id, DCPortType.NotImplemented),
                        device_address: result.device_address,
                        manufacturer_name: result.manufacturer_name,
                        model_name: result.model_name,
                        serial_number: result.serial_number,
                        model_id: result.model_id,
                        model_instance: result.model_instance,
                        dc_port_type: DCPortType.NotImplemented,
                    }));
                }} />,
            <hr/>,
            <FormRow label={__("meters_sun_spec.content.config_display_name")}>
                <InputText
                    required
                    maxLength={65}
                    value={this.props.config[1].display_name}
                    onValue={(v) => {
                        this.props.on_config(util.get_updated_union(this.props.config, {display_name: v}));
                    }} />
            </FormRow>,
            <FormRow>
                <Button variant="primary"
                        className="form-control"
                        disabled={this.state.manual_override}
                        onClick={() => this.setState({manual_override: true})}
                        >
                    {__("meters_sun_spec.content.config_manual_override")}
                </Button>
            </FormRow>,
            <FormRow label={__("meters_sun_spec.content.config_device_address")}>
                <InputNumber
                    required
                    disabled={!this.state.manual_override}
                    min={0}
                    max={255}
                    value={this.props.config[1].device_address}
                    onValue={(v) => {
                        this.props.on_config(util.get_updated_union(this.props.config, {device_address: v}));
                    }} />
            </FormRow>,
            <FormRow label={__("meters_sun_spec.content.config_unique_id")} label_muted={__("meters_sun_spec.content.config_unique_id_muted")}>
                <div class="row">
                    <div class="col-sm-4">
                        <InputText
                            disabled={!this.state.manual_override}
                            maxLength={32}
                            value={this.props.config[1].manufacturer_name}
                            onValue={(v) => {
                                this.props.on_config(util.get_updated_union(this.props.config, {manufacturer_name: v}));
                            }} />
                    </div>
                    <div class="col-sm-4">
                        <InputText
                            disabled={!this.state.manual_override}
                            maxLength={32}
                            value={this.props.config[1].model_name}
                            onValue={(v) => {
                                this.props.on_config(util.get_updated_union(this.props.config, {model_name: v}));
                            }} />
                    </div>
                    <div class="col-sm-4">
                        <InputText
                            disabled={!this.state.manual_override}
                            maxLength={32}
                            value={this.props.config[1].serial_number}
                            onValue={(v) => {
                                this.props.on_config(util.get_updated_union(this.props.config, {serial_number: v}));
                            }} />
                    </div>
                </div>
            </FormRow>,
            <FormRow label={__("meters_sun_spec.content.config_model_id")}>
                <InputSelect
                    required
                    disabled={!this.state.manual_override}
                    items={model_id_items}
                    placeholder={__("select")}
                    value={util.hasValue(this.props.config[1].model_id) ? this.props.config[1].model_id.toString() : this.props.config[1].model_id}
                    onValue={(v) => {
                        this.props.on_config(util.get_updated_union(this.props.config, {model_id: parseInt(v), location: get_fixed_location(parseInt(v), DCPortType.NotImplemented), dc_port_type: DCPortType.NotImplemented}));
                    }} />
            </FormRow>,
            <FormRow label={__("meters_sun_spec.content.config_model_instance")}>
                <InputNumber
                    required
                    disabled={!this.state.manual_override}
                    min={0}
                    max={65535}
                    value={this.props.config[1].model_instance}
                    onValue={(v) => {
                        this.props.on_config(util.get_updated_union(this.props.config, {model_instance: v}));
                    }} />
            </FormRow>
        ];

        if (this.props.config[1].model_id == 714) {
            edit_children.push(
                <FormRow label={__("meters_sun_spec.content.config_dc_port_type")}>
                    <InputSelect
                        required
                        items={[
                            [DCPortType.Photovoltaic.toString(), __("meters_sun_spec.content.dc_port_type_photovoltaic")],
                            [DCPortType.EnergyStorageSystem.toString(), __("meters_sun_spec.content.dc_port_type_energy_storage_system")],
                            [DCPortType.ElectricVehicle.toString(), __("meters_sun_spec.content.dc_port_type_electric_vehicle")],
                            [DCPortType.GenericInjecting.toString(), __("meters_sun_spec.content.dc_port_type_generic_injecting")],
                            [DCPortType.GenericAbsorbing.toString(), __("meters_sun_spec.content.dc_port_type_generic_absorbing")],
                            [DCPortType.GenericBidirectional.toString(), __("meters_sun_spec.content.dc_port_type_generic_bidirectional")],
                            [DCPortType.DCDC.toString(), __("meters_sun_spec.content.dc_port_type_dc_dc")],
                        ]}
                        placeholder={__("select")}
                        value={this.props.config[1].dc_port_type.toString()}
                        onValue={(v) => {
                            this.props.on_config(util.get_updated_union(this.props.config, {dc_port_type: parseInt(v), location: get_fixed_location(this.props.config[1].model_id, parseInt(v))}));
                        }} />
                </FormRow>);
        }

        let fixed_location: number; // MeterLocation.Unknown: there is no fixed location, null: fixed location is not known yet

        if (this.props.config[1].model_id == 714 && this.props.config[1].dc_port_type == DCPortType.NotImplemented) {
            fixed_location = null;
        }
        else {
            fixed_location = get_fixed_location(this.props.config[1].model_id, this.props.config[1].dc_port_type);
        }

        edit_children.push(
            <FormRow label={__("meters_sun_spec.content.config_location")}>
                {this.props.config[1].model_id === null ?
                    <InputText value={__("meters_sun_spec.content.config_location_depends_model_id")} /> :
                    (fixed_location === null ?
                        <InputText value={__("meters_sun_spec.content.config_location_depends_dc_port_type")} /> :
                        (fixed_location == MeterLocation.Unknown ?
                            <InputSelect
                                required
                                items={get_meter_location_items()}
                                placeholder={__("select")}
                                value={this.props.config[1].location.toString()}
                                onValue={(v) => {
                                    this.props.on_config(util.get_updated_union(this.props.config, {location: parseInt(v)}));
                                }} /> :
                            <InputText value={translate_meter_location(fixed_location)} />))}
            </FormRow>,
//#if MODULE_EM_ENERGY_ANALYSIS_AVAILABLE
            <FormRow label={__("meters.content.config_excluded")} help={__("meters.content.config_excluded_help")}>
                <Switch
                    desc={__("meters.content.config_excluded_desc")}
                    checked={this.props.config[1].excluded}
                    onClick={() => this.props.on_config(util.get_updated_union(this.props.config, {excluded: !this.props.config[1].excluded}))}/>
            </FormRow>
//#endif
        );

        return edit_children;
    }
}

export function pre_init() {
    return {
        [MeterClassID.SunSpec]: {
            name: () => __("meters_sun_spec.content.meter_class"),
            new_config: () => [MeterClassID.SunSpec, {display_name: "", location: MeterLocation.Unknown, excluded: false, host: "", port: 502, device_address: null, manufacturer_name: null, model_name: null, serial_number: null, model_id: null, model_instance: null, dc_port_type: DCPortType.NotImplemented}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: SunSpecMetersConfig, on_config: (config: SunSpecMetersConfig) => void): ComponentChildren => {
                return <EditChildren config={config} on_config={on_config} />;
            },
        },
    };
}

export function init() {
}
