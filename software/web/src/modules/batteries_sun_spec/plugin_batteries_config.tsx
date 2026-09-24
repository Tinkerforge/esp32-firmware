/* esp32-firmware
 * Copyright (C) 2026 Matthias Bolte <matthias@tinkerforge.com>
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

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import * as options from "../../options";
import { h, Fragment, Component, ComponentChild, ComponentChildren } from "preact";
import { Button, Dropdown, Alert } from "react-bootstrap";
import { __ } from "../../ts/translation";
import { BatteryClassID } from "../batteries/generated/battery_class_id.enum";
import { BatteryConfig } from "../batteries/types";
import { BatteryMode } from "../batteries/generated/battery_mode.enum";
import { InputText, InputTextPatterned } from "../../ts/components/input_text";
import { InputHost } from "../../ts/components/input_host";
import { CollapsedSection } from "../../ts/components/collapsed_section";
import { InputNumber } from "../../ts/components/input_number";
import { InputFloat } from "../../ts/components/input_float";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { OutputTextarea } from "../../ts/components/output_textarea";
import { Table, TableRow } from "../../ts/components/table";
import { SunSpecScanner, SunSpecScanResult } from "../sun_spec/scanner";

export type SunSpecBatteriesConfig = [
    BatteryClassID.SunSpec,
    {
        display_name: string;
        host: string;
        port: number;
        device_address: number;
        manufacturer_name: string;
        model_name: string;
        serial_number: string;
        model_instance: number;
        force_charge_rate: number;
        force_discharge_rate: number;
    },
];

export type SunSpecBatteriesState = {
    mode: number,
    resolving: boolean,
    testing: boolean,
}

const TEST_CONTINUE_INTERVAL = 3000; // milliseconds
const TEST_LOG_INTERVAL = 250; // milliseconds

interface TestRunnerProps {
    slot: number;
    host: string;
    port: number;
    device_address: number;
    manufacturer_name: string;
    model_name: string;
    serial_number: string;
    model_instance: number;
    force_charge_rate: number;
    force_discharge_rate: number;
    modes: [number, string][];
}

interface TestRunnerState {
    mode: [number, string];
    running: boolean;
    cookie: number;
    stopped: boolean;
    log: string;
    show_log: boolean;
}

class TestRunner extends Component<TestRunnerProps, TestRunnerState> {
    continue_timer: number = undefined;
    log_timer: number = undefined;
    pending_log: string = '';

    constructor() {
        super();

        this.state = {
            mode: null,
            running: false,
            cookie: null,
            stopped: false,
            log: '',
            show_log: false,
        } as any;

        util.addApiEventListener('batteries_sun_spec/test_log', () => {
            let test_log = API.get('batteries_sun_spec/test_log');

            if (!this.state.running || test_log.cookie !== this.state.cookie) {
                return;
            }

            this.pending_log += test_log.message;

            if (this.state.log.length == 0) {
                this.update_log();
            }
        });

        util.addApiEventListener('batteries_sun_spec/test_done', () => {
            let test_done = API.get('batteries_sun_spec/test_done');

            if (!this.state.running || test_done.cookie !== this.state.cookie) {
                return;
            }

            if (this.continue_timer !== undefined) {
                clearInterval(this.continue_timer);
                this.continue_timer = undefined;
            }

            if (this.log_timer !== undefined) {
                clearInterval(this.log_timer);
                this.log_timer = undefined;
            }

            this.update_log();
            this.setState({running: false, cookie: null});
        });
    }

    update_log(message?: string) {
        let log = this.state.log + this.pending_log;

        this.pending_log = '';

        if (message) {
            log += message;
        }

        this.setState({log: log});
    }

    async stop_test() {
        if (!this.state.running || this.state.stopped) {
            return;
        }

        let result;

        try {
            result = await (await util.put('/batteries_sun_spec/test_stop', {cookie: this.state.cookie}, true)).text();
        }
        catch (e) {
            result = e.message.replace('400(Bad Request) ', '');
        }

        if (result.length > 0) {
            this.update_log(__("batteries_sun_spec.content.test_stop_error") + ": " + result + "\n");
        }
        else {
            this.setState({stopped: true});
        }
    }

    override async componentWillUnmount() {
        await this.stop_test();
    }

    async test_continue() {
        if (!this.state.running) {
            return;
        }

        let result;

        try {
            result = await (await util.put('/batteries_sun_spec/test_continue', {cookie: this.state.cookie}, true)).text();
        }
        catch (e) {
            result = e.message.replace('400(Bad Request) ', '');
        }

        if (result.length > 0) {
            this.update_log(__("batteries_sun_spec.content.test_continue_error") + ": " + result + "\n");
        }
    }

    async start_mode(mode: [number, string]) {
        let cookie: number = Math.floor(Math.random() * 0xFFFFFFFF);

        this.pending_log = '';

        this.setState({
            mode: mode,
            running: true,
            cookie: cookie,
            stopped: false,
            show_log: true,
            log: '',
        }, async () => {
            let result;

            try {
                result = await (await util.put('/batteries_sun_spec/test', {
                    slot: this.props.slot,
                    host: this.props.host,
                    port: this.props.port,
                    device_address: this.props.device_address,
                    manufacturer_name: this.props.manufacturer_name,
                    model_name: this.props.model_name,
                    serial_number: this.props.serial_number,
                    model_instance: this.props.model_instance,
                    force_charge_rate: this.props.force_charge_rate,
                    force_discharge_rate: this.props.force_discharge_rate,
                    mode: mode[0],
                    cookie: cookie,
                }, true)).text();
            }
            catch (e) {
                result = e.message.replace('400(Bad Request) ', '');
            }

            if (result.length > 0) {
                this.update_log(__("batteries_sun_spec.content.test_start_error") + ": " + result + "\n");
                this.setState({running: false, cookie: null});

                return;
            }

            this.continue_timer = window.setInterval(async () => {await this.test_continue()}, TEST_CONTINUE_INTERVAL);
            this.log_timer = window.setInterval(() => this.update_log(), TEST_LOG_INTERVAL);
        });
    }

    render() {
        let start_button_disabled = this.props.host.trim().length == 0 || !util.hasValue(this.props.port);

        return <>
            <FormRow label={__("batteries_sun_spec.content.test")}>
            {!this.state.running ?
                <Dropdown key="test_start">
                    <Dropdown.Toggle
                        variant="primary"
                        className="form-control"
                        disabled={start_button_disabled}>
                        {__("batteries_sun_spec.content.test_start")}
                    </Dropdown.Toggle>
                    <Dropdown.Menu renderOnMount align="end">
                        <Dropdown.Header class="text-wrap">{__("batteries_sun_spec.content.test_mode")}</Dropdown.Header>
                        {this.props.modes.map(mode =>
                            <Dropdown.Item
                                as="button"
                                className="py-2"
                                onClick={async () => await this.start_mode(mode)}>
                                {mode[1]}
                            </Dropdown.Item>)}
                    </Dropdown.Menu>
                </Dropdown> :
                <Button key="test_stop"
                        variant="primary"
                        className="form-control"
                        onClick={async () => await this.stop_test()}
                        disabled={this.state.stopped}>
                    {__("batteries_sun_spec.content.test_stop")(this.state.mode[1])}
                </Button>}
            </FormRow>

            {this.state.show_log ?
                <FormRow>
                    <OutputTextarea rows={10} resize='vertical' value={this.state.log} />
                </FormRow>
                : undefined}
        </>;
    }
}

interface EditChildrenProps {
    battery_slot: number;
    config: SunSpecBatteriesConfig;
    on_config: (config: SunSpecBatteriesConfig) => void;
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
        let edit_children = [
            <FormRow label={__("batteries_sun_spec.content.host")}>
                <InputHost
                    required
                    maxLength={64}
                    value={this.props.config[1].host}
                    onValue={(v) => this.props.on_config(util.get_updated_union(this.props.config, {host: v}))} />
            </FormRow>,
            <FormRow label={__("batteries_sun_spec.content.port")} label_muted={__("batteries_sun_spec.content.port_muted")}>
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
            <SunSpecScanner
                host={this.props.config[1].host} port={this.props.config[1].port}
                exclude_model_from_display_name
                on_is_model_visible={(model_id: number) => model_id == 124}
                on_is_model_supported={(model_id: number) => model_id == 124}
                on_result_selected={(result: SunSpecScanResult) => {
                    this.setState({manual_override: false});

                    this.props.on_config(util.get_updated_union(this.props.config, {
                        display_name: result.display_name,
                        device_address: result.device_address,
                        manufacturer_name: result.manufacturer_name,
                        model_name: result.model_name,
                        serial_number: result.serial_number,
                        model_instance: result.model_instance,
                    }));
                }} />,
            <hr/>,
            <FormRow label={__("batteries_sun_spec.content.display_name")}>
                <InputText
                    required
                    maxLength={32}
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
                    {__("batteries_sun_spec.content.manual_override")}
                </Button>
            </FormRow>,
            <FormRow label={__("batteries_sun_spec.content.device_address")}>
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
            <FormRow label={__("batteries_sun_spec.content.unique_id")} label_muted={__("batteries_sun_spec.content.unique_id_muted")}>
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
            <FormRow label={__("batteries_sun_spec.content.model_id")}>
                <InputSelect
                    required
                    disabled
                    items={[
                        ["124", __("sun_spec.content.model_124") + " [124]"]
                    ]}
                    value="124" />
            </FormRow>,
            <FormRow label={__("batteries_sun_spec.content.model_instance")}>
                <InputNumber
                    required
                    disabled={!this.state.manual_override}
                    min={0}
                    max={65535}
                    value={this.props.config[1].model_instance}
                    onValue={(v) => {
                        this.props.on_config(util.get_updated_union(this.props.config, {model_instance: v}));
                    }} />
            </FormRow>,
            <FormRow label={__("batteries_sun_spec.content.force_charge_rate")}>
                <InputNumber
                    required
                    min={1}
                    max={100}
                    unit="%"
                    value={this.props.config[1].force_charge_rate}
                    onValue={(v) => {
                        this.props.on_config(util.get_updated_union(this.props.config, {force_charge_rate: v}));
                    }} />
            </FormRow>,
            <FormRow label={__("batteries_sun_spec.content.force_discharge_rate")}>
                <InputNumber
                    required
                    min={1}
                    max={100}
                    unit="%"
                    value={this.props.config[1].force_discharge_rate}
                    onValue={(v) => {
                        this.props.on_config(util.get_updated_union(this.props.config, {force_discharge_rate: v}));
                    }} />
            </FormRow>
        ];

        let battery_mode_names = [
            __("batteries.content.battery_mode_block"),
            __("batteries.content.battery_mode_normal"),
            __("batteries.content.battery_mode_block_discharge"),
            __("batteries.content.battery_mode_force_charge"),
            __("batteries.content.battery_mode_block_charge"),
            __("batteries.content.battery_mode_force_discharge"),
        ];

        let battery_mode_order = [
            BatteryMode.Normal,
            BatteryMode.Block,
            BatteryMode.BlockCharge,
            BatteryMode.BlockDischarge,
            BatteryMode.ForceCharge,
            BatteryMode.ForceDischarge,
        ];

        let battery_mode_items = battery_mode_order.map(mode => [mode, battery_mode_names[mode]]);

        edit_children.push(
            <TestRunner
                slot={this.props.battery_slot}
                host={this.props.config[1].host}
                port={this.props.config[1].port}
                device_address={this.props.config[1].device_address}
                manufacturer_name={this.props.config[1].manufacturer_name}
                model_name={this.props.config[1].model_name}
                serial_number={this.props.config[1].serial_number}
                model_instance={this.props.config[1].model_instance}
                force_charge_rate={this.props.config[1].force_charge_rate}
                force_discharge_rate={this.props.config[1].force_discharge_rate}
                modes={battery_mode_items as [number, string][]} />);

        return edit_children;
    }
}

export function pre_init() {
    return {
        [BatteryClassID.SunSpec]: {
            name: () => __("batteries_sun_spec.content.battery_class"),
            new_config: () => [BatteryClassID.SunSpec, {display_name: "", host: "", port: 502, device_address: null, manufacturer_name: null, model_name: null, serial_number: null, model_instance: null, force_charge_rate: 100, force_discharge_rate: 100}] as BatteryConfig,
            clone_config: (config: BatteryConfig) => [config[0], {...config[1]}] as BatteryConfig,
            get_state_info: (battery_slot: number, config: SunSpecBatteriesConfig, battery_state: SunSpecBatteriesState): {state_name: string, warning: ComponentChild} => {
                if (!util.hasValue(battery_state)) {
                    return undefined;
                }

                let state_name = __("batteries.content.battery_state_by_mode")(battery_state.mode);
                let warning = [];

                if (battery_state.resolving) {
                    state_name += ', ' + __("batteries_sun_spec.content.resolving");
                }

                if (battery_state.testing) {
                    state_name += ', ' + __("batteries_sun_spec.content.testing");
                    warning.push(__("batteries_sun_spec.content.testing_warning"));
                }

                if (warning.length == 0) {
                    warning = undefined;
                }

                return {state_name: state_name, warning: warning};
            },
            get_edit_children: (battery_slot: number, config: SunSpecBatteriesConfig, on_config: (config: SunSpecBatteriesConfig) => void): ComponentChildren => {
                return <EditChildren battery_slot={battery_slot} config={config} on_config={on_config} />;
            },
        },
    };
}

export function init() {
}
