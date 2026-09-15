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
import { h, Fragment, Component, ComponentChildren, VNode } from "preact";
import { __, translate_unchecked, removeUnicodeHacks } from "../../ts/translation";
import { InputText } from "../../ts/components/input_text";
import { InputHost } from "../../ts/components/input_host";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import { Progress } from "../../ts/components/progress";
import { OutputTextarea } from "../../ts/components/output_textarea";
import { Button, ListGroup, ListGroupItem, Alert } from "react-bootstrap";
import { Download, Plus } from 'react-feather';
import { DiscoveryResultGroup, DiscoveryResultItem, DiscoveryResultItemProps } from "../../ts/components/discovery_result";

const SCAN_CONTINUE_INTERVAL = 3000; // milliseconds
const SCAN_LOG_INTERVAL = 250; // milliseconds

export interface SunSpecScanResult {
    unique_id: string;
    manufacturer_name: string;
    model_name: string;
    display_name: string;
    serial_number: string;
    device_address: number;
    model_id: number;
    model_instance: number;
}

interface SunSpecScannerProps {
    host: string;
    port: number;
    on_is_model_visible: (model_id: number) => boolean;
    on_is_model_supported: (model_id: number) => boolean;
    on_result_selected: (result: SunSpecScanResult) => void;
}

interface SunSpecScannerState {
    device_address_first: number;
    device_address_last: number;
    running: boolean;
    cookie: number;
    aborted: boolean;
    error: boolean;
    progress: number;
    log: string;
    show_log: boolean;
    results: SunSpecScanResult[];
}

export class SunSpecScanner extends Component<SunSpecScannerProps, SunSpecScannerState> {
    continue_timer: number = undefined;
    log_timer: number = undefined;
    pending_log: string = '';

    constructor() {
        super();

        this.state = {
            device_address_first: 1,
            device_address_last: 247,
            running: false,
            cookie: null,
            aborted: false,
            error: false,
            progress: 0,
            log: '',
            show_log: false,
            results: [],
        } as any;

        util.addApiEventListener('sun_spec/scan_log', () => {
            let scan_log = API.get('sun_spec/scan_log');

            if (!this.state.running || scan_log.cookie !== this.state.cookie) {
                return;
            }

            this.pending_log += scan_log.message;

            if (this.state.log.length == 0) {
                this.update_log();
            }
        });

        util.addApiEventListener('sun_spec/scan_error', () => {
            let scan_error = API.get('sun_spec/scan_error');

            if (!this.state.running || scan_error.cookie !== this.state.cookie) {
                return;
            }

            this.setState({error: true});
        });

        util.addApiEventListener('sun_spec/scan_progress', () => {
            let scan_progress = API.get('sun_spec/scan_progress');

            if (!this.state.running || scan_progress.cookie !== this.state.cookie) {
                return;
            }

            this.setState({progress: scan_progress.progress});
        });

        util.addApiEventListener('sun_spec/scan_result', () => {
            let scan_result = API.get('sun_spec/scan_result');

            if (!this.state.running || scan_result.cookie !== this.state.cookie) {
                return;
            }

            if (!this.props.on_is_model_visible(scan_result.model_id)) {
                return;
            }

            // this combination must be unique according to sunspec specification
            let unique_id = scan_result.manufacturer_name + scan_result.model_name + scan_result.serial_number;

            if (this.state.results.filter((other) => other.unique_id == unique_id && other.model_id == scan_result.model_id && other.model_instance == scan_result.model_instance).length == 0) {
                let manufacturer_name = scan_result.manufacturer_name.trim();

                if (manufacturer_name == 'KOSTAL Solar Electric GmbH') {
                    manufacturer_name = 'KOSTAL';
                }

                this.setState({results: this.state.results.concat({
                    unique_id: unique_id,
                    manufacturer_name: scan_result.manufacturer_name,
                    model_name: scan_result.model_name,
                    display_name: removeUnicodeHacks((scan_result.model_name.startsWith(manufacturer_name) ? scan_result.model_name.trim() : manufacturer_name + ' ' + scan_result.model_name.trim()) + ': ' + translate_unchecked(`sun_spec.content.model_${scan_result.model_id}`)).substring(0, 65),
                    serial_number: scan_result.serial_number,
                    device_address: scan_result.device_address,
                    model_id: scan_result.model_id,
                    model_instance: scan_result.model_instance,
                })});
            }
        });

        util.addApiEventListener('sun_spec/scan_done', () => {
            let scan_done = API.get('sun_spec/scan_done');

            if (!this.state.running || scan_done.cookie !== this.state.cookie) {
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

            let progress = this.state.progress;

            if (!this.state.aborted) {
                progress = 100;
            }

            this.setState({running: false, cookie: null, progress: progress});
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

    async abort_scan() {
        if (!this.state.running || this.state.aborted) {
            return;
        }

        let result;

        try {
            result = await (await util.put('/sun_spec/scan_abort', {cookie: this.state.cookie}, true)).text();
        }
        catch (e) {
            result = e.message.replace('400(Bad Request) ', '');
        }

        if (result.length > 0) {
            this.update_log(__("sun_spec.content.scan_abort_error") + ": " + result + "\n");
        }
        else {
            this.setState({aborted: true});
        }
    }

    override async componentWillUnmount() {
        await this.abort_scan();
    }

    get_scan_result_item(result: SunSpecScanResult): VNode<DiscoveryResultItemProps> {
        let preferred_model_id: number = null;

        if ([101, 102, 103, 201, 202, 203, 204].indexOf(result.model_id) >= 0 &&
            this.state.results.findIndex((other) => other.model_id == result.model_id + 10) >= 0) {
            preferred_model_id = result.model_id + 10;
        }

        let selectable = this.props.on_is_model_supported(result.model_id) && preferred_model_id === null;

        return <DiscoveryResultItem
                key={result.model_id}
                title={<h5>{result.display_name}</h5>}
                labelAdd={<Plus />}
                error={selectable ? undefined : <span class="text-danger">{preferred_model_id !== null ? __("sun_spec.content.scan_result_model_other_preferred")(preferred_model_id) : __("sun_spec.content.scan_result_model_no_supported")}</span>}
                onClick={() => this.props.on_result_selected(result)}>
                <div>{__("sun_spec.content.scan_result_device_address")}: {result.device_address}</div>
                <div>{__("sun_spec.content.scan_result_serial_number")}: {result.serial_number}</div>
                <div>{__("sun_spec.content.scan_result_model_id")}: {translate_unchecked(`sun_spec.content.model_${result.model_id}`)} [{result.model_id}] / {result.model_instance}</div>
        </DiscoveryResultItem>;
    }

    async scan_continue() {
        if (!this.state.running) {
            return;
        }

        let result;

        try {
            result = await (await util.put('/sun_spec/scan_continue', {cookie: this.state.cookie}, true)).text();
        }
        catch (e) {
            result = e.message.replace('400(Bad Request) ', '');
        }

        if (result.length > 0) {
            this.update_log(__("sun_spec.content.scan_continue_error") + ": " + result + "\n");
        }
    }

    render() {
        return <>
            <FormRow label={__("sun_spec.content.scan_title")} label_muted={__("sun_spec.content.scan_title_muted")}>
                <div class="row mb-3">
                    <div class="col-sm-6">
                        <InputNumber
                            required
                            min={0}
                            max={255}
                            value={this.state.device_address_first}
                            onValue={(v) => {
                                this.setState({device_address_first: v});
                            }} />
                    </div>
                    <div class="col-sm-6">
                        <InputNumber
                            required
                            min={0}
                            max={255}
                            value={this.state.device_address_last}
                            onValue={(v) => {
                                this.setState({device_address_last: v});
                            }} />
                    </div>
                </div>
            {!this.state.running ?
                <Button key="scan"
                        variant="primary"
                        className="form-control"
                        onClick={async () => {
                            let cookie: number = Math.floor(Math.random() * 0xFFFFFFFF);

                            this.pending_log = '';

                            this.setState({
                                running: true,
                                cookie: cookie,
                                error: false,
                                aborted: false,
                                show_log: true,
                                progress: 0,
                                log: '',
                                results: [],
                            }, async () => {
                                let result;

                                try {
                                    result = await (await util.put('/sun_spec/scan', {
                                        host: this.props.host,
                                        port: this.props.port,
                                        device_address_first: this.state.device_address_first,
                                        device_address_last: this.state.device_address_last,
                                        cookie: cookie,
                                    }, true)).text();
                                }
                                catch (e) {
                                    result = e.message.replace('400(Bad Request) ', '');
                                }

                                if (result.length > 0) {
                                    this.update_log(__("sun_spec.content.scan_start_error") + ": " + result + "\n");
                                    this.setState({running: false, cookie: null});

                                    return;
                                }

                                this.continue_timer = window.setInterval(async () => {await this.scan_continue()}, SCAN_CONTINUE_INTERVAL);
                                this.log_timer = window.setInterval(() => this.update_log(), SCAN_LOG_INTERVAL);
                            });
                        }}
                        disabled={this.props.host.trim().length == 0 || !util.hasValue(this.props.port)}>
                    {__("sun_spec.content.scan")}
                </Button> :
                <Button key="scan_abort"
                        variant="primary"
                        className="form-control"
                        onClick={async () => await this.abort_scan()}
                        disabled={this.state.aborted}>
                    {__("sun_spec.content.scan_abort")}
                </Button>}
            </FormRow>

            {this.state.running ?
                <FormRow>
                    <Progress progress={this.state.progress / 100} />
                </FormRow>
                : undefined}

            {this.state.show_log ?
                <><FormRow>
                    <OutputTextarea rows={10} resize='vertical' value={this.state.log} />
                </FormRow>

                {this.state.error ?
                    <FormRow>
                        <Alert variant="warning" className="mb-0">{__("sun_spec.content.scan_error")()}</Alert>
                    </FormRow>
                    : undefined}

                <FormRow>
                    <Button variant="primary"
                            disabled={this.state.running || this.state.log.length == 0}
                            className="form-control"
                            onClick={() => util.downloadToTimestampedFile(this.state.log, __("sun_spec.content.scan_log_file"), "txt", "text/plain")}>
                        <span class="me-2">{__("sun_spec.content.scan_log")}</span>
                        <Download/>
                    </Button>
                </FormRow></>
                : undefined}

            {this.state.results.length > 0 ?
                <FormRow label={__("sun_spec.content.scan_results")}>
                    <DiscoveryResultGroup>
                        {this.state.results.map((result) => this.get_scan_result_item(result))}
                    </DiscoveryResultGroup>
                </FormRow>
                : undefined}
        </>;
    }
}
