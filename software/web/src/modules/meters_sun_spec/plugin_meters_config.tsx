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

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { h, Fragment, Component, ComponentChildren } from "preact";
import { __, translate_unchecked, removeUnicodeHacks } from "../../ts/translation";
import { MeterClassID } from "../meters/meter_class_id.enum";
import { MeterLocation } from "../meters/meter_location.enum";
import { get_meter_location_items } from "../meters/meter_location";
import { MeterConfig } from "../meters/types";
import { DCPortType } from "./dc_port_type.enum";
import { InputText } from "../../ts/components/input_text";
import { InputHost } from "../../ts/components/input_host";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { SwitchableInputSelect } from "../../ts/components/switchable_input_select";
import { FormRow } from "../../ts/components/form_row";
import { Progress } from "../../ts/components/progress";
import { OutputTextarea } from "../../ts/components/output_textarea";
import { Button, ListGroup, ListGroupItem, Alert } from "react-bootstrap";
import { Download } from 'react-feather';
import { SUN_SPEC_MODEL_INFOS, SUN_SPEC_MODEL_IS_METER_LIKE, SUN_SPEC_MODEL_METER_LOCATION, SUN_SPEC_MODEL_IS_SUPPORTED } from "./sun_spec_model_specs";

const SCAN_CONTINUE_INTERVAL = 3000; // milliseconds

export type SunSpecMetersConfig = [
    MeterClassID.SunSpec,
    {
        display_name: string;
        location: number;
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

interface DeviceScannerResult {
    unique_id: string;
    manufacturer_name: string;
    model_name: string;
    display_name: string;
    serial_number: string;
    device_address: number;
    model_id: number;
    model_instance: number;
}

interface DeviceScannerProps {
    host: string;
    port: number;
    onResultSelected: (result: DeviceScannerResult) => void;
}

interface DeviceScannerState {
    scan_device_address_first: number;
    scan_device_address_last: number;
    scan_running: boolean;
    scan_cookie: number;
    scan_error: boolean;
    scan_progress: number;
    scan_log: string;
    scan_show_log: boolean;
    scan_results: DeviceScannerResult[];
}

class DeviceScanner extends Component<DeviceScannerProps, DeviceScannerState> {
    scan_continue_timer: number = undefined;

    constructor() {
        super();

        this.state = {
            scan_device_address_first: 1,
            scan_device_address_last: 247,
            scan_running: false,
            scan_cookie: null,
            scan_error: false,
            scan_progress: 0,
            scan_log: '',
            scan_show_log: false,
            scan_results: [],
        } as any;

        util.addApiEventListener('meters_sun_spec/scan_log', () => {
            let scan_log = API.get('meters_sun_spec/scan_log');

            if (!this.state.scan_running || scan_log.cookie !== this.state.scan_cookie) {
                return;
            }

            this.setState({scan_log: this.state.scan_log + scan_log.message})
        });

        util.addApiEventListener('meters_sun_spec/scan_error', () => {
            let scan_error = API.get('meters_sun_spec/scan_error');

            if (!this.state.scan_running || scan_error.cookie !== this.state.scan_cookie) {
                return;
            }

            this.setState({scan_error: true});
        });

        util.addApiEventListener('meters_sun_spec/scan_progress', () => {
            let scan_progress = API.get('meters_sun_spec/scan_progress');

            if (!this.state.scan_running || scan_progress.cookie !== this.state.scan_cookie) {
                return;
            }

            this.setState({scan_progress: scan_progress.progress});
        });

        util.addApiEventListener('meters_sun_spec/scan_result', () => {
            let scan_result = API.get('meters_sun_spec/scan_result');

            if (!this.state.scan_running || scan_result.cookie !== this.state.scan_cookie) {
                return;
            }

            if (!SUN_SPEC_MODEL_IS_METER_LIKE[scan_result.model_id]) {
                return;
            }

            // this combination must be unique according to sunspec specification
            let unique_id = scan_result.manufacturer_name + scan_result.model_name + scan_result.serial_number;

            if (this.state.scan_results.filter((other) => other.unique_id == unique_id && other.model_id == scan_result.model_id && other.model_instance == scan_result.model_instance).length == 0) {
                let manufacturer_name = scan_result.manufacturer_name.trim();

                if (manufacturer_name == 'KOSTAL Solar Electric GmbH') {
                    manufacturer_name = 'KOSTAL';
                }

                this.setState({scan_results: this.state.scan_results.concat({
                    unique_id: unique_id,
                    manufacturer_name: scan_result.manufacturer_name,
                    model_name: scan_result.model_name,
                    display_name: removeUnicodeHacks((scan_result.model_name.startsWith(manufacturer_name) ? scan_result.model_name.trim() : manufacturer_name + ' ' + scan_result.model_name.trim()) + ': ' + translate_unchecked(`meters_sun_spec.content.model_${scan_result.model_id}`)).substring(0, 65),
                    serial_number: scan_result.serial_number,
                    device_address: scan_result.device_address,
                    model_id: scan_result.model_id,
                    model_instance: scan_result.model_instance,
                })});
            }
        });

        util.addApiEventListener('meters_sun_spec/scan_done', () => {
            let scan_done = API.get('meters_sun_spec/scan_done');

            if (!this.state.scan_running || scan_done.cookie !== this.state.scan_cookie) {
                return;
            }

            if (this.scan_continue_timer !== undefined) {
                clearTimeout(this.scan_continue_timer);
                this.scan_continue_timer = undefined;
            }

            this.setState({scan_running: false, scan_cookie: null, scan_progress: 100});
        });
    }

    async abort_scan() {
        if (this.scan_continue_timer !== undefined) {
            clearTimeout(this.scan_continue_timer);
            this.scan_continue_timer = undefined;
        }

        if (!this.state.scan_running) {
            return;
        }

        let result;

        try {
            result = await (await util.put('/meters_sun_spec/scan_abort', {cookie: this.state.scan_cookie})).text();
        }
        catch (e) {
            this.setState({
                scan_running: false,
                scan_cookie: null,
                scan_log: this.state.scan_log + "Error while aborting the scan: " + e.message.replace('400(Bad Request) ', '') + "\n",
            });

            return;
        }

        let scan_log = this.state.scan_log;

        if (result.length > 0) {
             scan_log += "Error while aborting the scan: " + result + "\n";
        }
        else {
             scan_log += "Scan aborted\n";
        }

        this.setState({
            scan_running: false,
            scan_cookie: null,
            scan_log: scan_log,
        });
    }

    override async componentWillUnmount() {
        await this.abort_scan();
    }

    get_scan_result_item(scan_result: DeviceScannerResult) {
        let preferred_model_id: number = null;

        if ([101, 102, 103, 201, 202, 203, 204].indexOf(scan_result.model_id) >= 0 &&
            this.state.scan_results.findIndex((other) => other.model_id == scan_result.model_id + 10) >= 0) {
            preferred_model_id = scan_result.model_id + 10;
        }

        let selectable = SUN_SPEC_MODEL_IS_SUPPORTED[scan_result.model_id] && preferred_model_id === null;

        return <ListGroupItem
                key={scan_result.model_id}
                action
                type="button"
                onClick={selectable ? () => {this.props.onResultSelected(scan_result)} : undefined}
                style={selectable ? "" : "cursor: default; background-color: #eeeeee !important;"}>
            <div class="d-flex w-100 justify-content-between">
                <span class="h5 text-left">{scan_result.display_name}</span>
                {selectable ? undefined :
                    <span class="text-right" style="color:red">{preferred_model_id !== null ? __("meters_sun_spec.content.model_other_preferred")(preferred_model_id) : __("meters_sun_spec.content.model_no_supported")}</span>
                }
            </div>
            <div class="d-flex w-100 justify-content-between">
                <span class="text-left">{__("meters_sun_spec.content.config_device_address")}: {scan_result.device_address}</span>
                <span class="text-center">{__("meters_sun_spec.content.config_serial_number")}: {scan_result.serial_number}</span>
                <span class="text-right">{__("meters_sun_spec.content.config_model_id")}: {translate_unchecked(`meters_sun_spec.content.model_${scan_result.model_id}`)} [{scan_result.model_id}] / {scan_result.model_instance}</span>
            </div>
        </ListGroupItem>;
    }

    async scan_continue() {
        this.scan_continue_timer = undefined;

        if (!this.state.scan_running) {
            return;
        }

        let result;

        try {
            result = await (await util.put('/meters_sun_spec/scan_continue', {cookie: this.state.scan_cookie})).text();
        }
        catch (e) {
            this.setState({
                scan_running: false,
                scan_cookie: null,
                scan_log: this.state.scan_log + "Error while keeping the scan running: " + e.message.replace('400(Bad Request) ', '') + "\n",
            });

            return;
        }

        if (result.length > 0) {
            this.setState({
                scan_running: false,
                scan_cookie: null,
                scan_log: this.state.scan_log + "Error while keeping the scan running: " + result + "\n",
            });

            return;
        }

        this.scan_continue_timer = setTimeout(async () => {await this.scan_continue()}, SCAN_CONTINUE_INTERVAL);
    }

    render() {
        return <>
            <FormRow label={__("meters_sun_spec.content.scan_title")} label_muted={__("meters_sun_spec.content.scan_title_muted")}>
                <div class="row mb-3">
                    <div class="col-sm-6">
                        <InputNumber
                            required
                            min={0}
                            max={255}
                            value={this.state.scan_device_address_first}
                            onValue={(v) => {
                                this.setState({scan_device_address_first: v});
                            }} />
                    </div>
                    <div class="col-sm-6">
                        <InputNumber
                            required
                            min={0}
                            max={255}
                            value={this.state.scan_device_address_last}
                            onValue={(v) => {
                                this.setState({scan_device_address_last: v});
                            }} />
                    </div>
                </div>
            {!this.state.scan_running ?
                <Button key="scan"
                        variant="primary"
                        className="form-control"
                        onClick={async () => {
                            let scan_cookie: number = Math.floor(Math.random() * 0xFFFFFFFF);

                            this.setState({
                                scan_running: true,
                                scan_cookie: scan_cookie,
                                scan_error: false,
                                scan_show_log: true,
                                scan_progress: 0,
                                scan_log: '',
                                scan_results: [],
                            }, async () => {
                                let result;

                                try {
                                    result = await (await util.put('/meters_sun_spec/scan', {
                                        host: this.props.host,
                                        port: this.props.port,
                                        device_address_first: this.state.scan_device_address_first,
                                        device_address_last: this.state.scan_device_address_last,
                                        cookie: scan_cookie,
                                    })).text();
                                }
                                catch (e) {
                                    this.setState({
                                        scan_running: false,
                                        scan_cookie: null,
                                        scan_log: e.message.replace('400(Bad Request) ', ''),
                                    });

                                    return;
                                }

                                if (result.length > 0) {
                                    this.setState({
                                        scan_running: false,
                                        scan_cookie: null,
                                        scan_log: result,
                                    });

                                    return;
                                }

                                this.scan_continue_timer = setTimeout(async () => {await this.scan_continue()}, SCAN_CONTINUE_INTERVAL);
                            });
                        }}
                        disabled={this.props.host.trim().length == 0 || !util.hasValue(this.props.port) || this.state.scan_running}>
                    {__("meters_sun_spec.content.scan")}
                </Button> :
                <Button key="scan_abort"
                        variant="primary"
                        className="form-control"
                        onClick={async () => await this.abort_scan()}
                        >
                    {__("meters_sun_spec.content.scan_abort")}
                </Button>}
            </FormRow>

            {this.state.scan_running ?
                <FormRow label="">
                    <Progress progress={this.state.scan_progress / 100} />
                </FormRow>
                : undefined}

            {this.state.scan_show_log ?
                <><FormRow label="">
                    <OutputTextarea rows={10} resize='vertical' value={this.state.scan_log} />
                </FormRow>

                {this.state.scan_error ?
                    <FormRow label="">
                        <Alert variant="warning" className="mb-0">{__("meters_sun_spec.content.scan_error")()}</Alert>
                    </FormRow>
                    : undefined}

                <FormRow label="">
                    <Button variant="primary"
                            disabled={this.state.scan_running || this.state.scan_log.length == 0}
                            className="form-control"
                            onClick={() => util.downloadToTimestampedFile(this.state.scan_log, __("meters_sun_spec.content.scan_log_file"), "txt", "text/plain")}>
                        <span class="mr-2">{__("meters_sun_spec.content.scan_log")}</span>
                        <Download/>
                    </Button>
                </FormRow></>
                : undefined}

            {this.state.scan_results.length > 0 ?
                <FormRow label={__("meters_sun_spec.content.scan_results")}>
                    <ListGroup>
                        {this.state.scan_results
                            .filter((scan_result) => SUN_SPEC_MODEL_IS_SUPPORTED[scan_result.model_id])
                            .map((scan_result) => this.get_scan_result_item(scan_result))}
                        {this.state.scan_results
                            .filter((scan_result) => !SUN_SPEC_MODEL_IS_SUPPORTED[scan_result.model_id])
                            .map((scan_result) => this.get_scan_result_item(scan_result))}
                    </ListGroup>
                </FormRow>
                : undefined}
        </>;
    }
}

function get_default_location(model_id: number, dc_port_type: number) {
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
        case DCPortType.GenericInjecting:
        case DCPortType.GenericAbsorbing:
        case DCPortType.GenericBidirectional:
        case DCPortType.DCDC:
            return MeterLocation.Other;

        case DCPortType.NotImplemented:
            return MeterLocation.Unknown;
        }

        return MeterLocation.Unknown;
    }

    let location = SUN_SPEC_MODEL_METER_LOCATION[model_id];

    if (location === undefined) {
        location = MeterLocation.Unknown;
    }

    return location;
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
        let model_ids: [string, string][] = [];

        for (let model_info of SUN_SPEC_MODEL_INFOS) {
            if (model_info.is_supported) {
                model_ids.push([model_info.model_id.toString(), translate_unchecked(`meters_sun_spec.content.model_${model_info.model_id}`) + ` [${model_info.model_id}]`]);
            }
        }

        let edit_children = [
            <FormRow label={__("meters_sun_spec.content.config_host")}>
                <InputHost
                    required
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
            <DeviceScanner host={this.props.config[1].host} port={this.props.config[1].port} onResultSelected={(result: DeviceScannerResult) => {
                this.setState({manual_override: false});

                this.props.on_config(util.get_updated_union(this.props.config, {
                    display_name: result.display_name,
                    location: get_default_location(result.model_id, DCPortType.NotImplemented),
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
            <FormRow label="">
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
                    items={model_ids}
                    placeholder={__("select")}
                    value={util.hasValue(this.props.config[1].model_id) ? this.props.config[1].model_id.toString() : this.props.config[1].model_id}
                    onValue={(v) => {
                        this.props.on_config(util.get_updated_union(this.props.config, {model_id: parseInt(v), location: get_default_location(parseInt(v), DCPortType.NotImplemented), dc_port_type: DCPortType.NotImplemented}));
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
                            this.props.on_config(util.get_updated_union(this.props.config, {dc_port_type: parseInt(v), location: get_default_location(this.props.config[1].model_id, parseInt(v))}));
                        }} />
                </FormRow>);
        }

        let default_location = get_default_location(this.props.config[1].model_id, this.props.config[1].dc_port_type);

        if (default_location == MeterLocation.Unknown) {
            edit_children.push(
                <FormRow label={__("meters_sun_spec.content.config_location")}>
                    <InputSelect
                        required
                        disabled={this.props.config[1].model_id === null || (this.props.config[1].model_id == 714 && this.props.config[1].dc_port_type == DCPortType.NotImplemented)}
                        items={get_meter_location_items()}
                        placeholder={__("select")}
                        value={this.props.config[1].location.toString()}
                        onValue={(v) => {
                            this.props.on_config(util.get_updated_union(this.props.config, {location: parseInt(v)}));
                        }} />
                </FormRow>);
        }
        else {
            let enable_location_override = this.props.config[1].model_id !== null && default_location != this.props.config[1].location;

            edit_children.push(
                <FormRow label={__("meters_sun_spec.content.config_location")}>
                    <SwitchableInputSelect
                        required
                        items={get_meter_location_items()}
                        placeholder={__("select")}
                        value={this.props.config[1].location.toString()}
                        onValue={(v) => {
                            this.props.on_config(util.get_updated_union(this.props.config, {location: parseInt(v)}));
                        }}
                        checked={enable_location_override}
                        onSwitch={() => {
                            this.props.on_config(util.get_updated_union(this.props.config, {location: (enable_location_override ? default_location : MeterLocation.Unknown)}));
                        }}
                        switch_label_active={__("meters_sun_spec.content.location_different")}
                        switch_label_inactive={__("meters_sun_spec.content.location_matching")}
                        />
                </FormRow>);
        }

        return edit_children;
    }
}

export function init() {
    return {
        [MeterClassID.SunSpec]: {
            name: () => __("meters_sun_spec.content.meter_class"),
            new_config: () => [MeterClassID.SunSpec, {display_name: "", location: MeterLocation.Unknown, host: "", port: 502, device_address: null, manufacturer_name: null, model_name: null, serial_number: null, model_id: null, model_instance: null, dc_port_type: DCPortType.NotImplemented}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: SunSpecMetersConfig, on_config: (config: SunSpecMetersConfig) => void): ComponentChildren => {
                return <EditChildren config={config} on_config={on_config} />;
            },
        },
    };
}
