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

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { h, Fragment, Component, ComponentChildren, VNode } from "preact";
import { __, translate_unchecked, removeUnicodeHacks } from "../../ts/translation";
import { InputText } from "../../ts/components/input_text";
import { InputIP } from "../../ts/components/input_ip";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { Progress } from "../../ts/components/progress";
import { OutputTextarea } from "../../ts/components/output_textarea";
import { Button, ListGroup, ListGroupItem, Alert } from "react-bootstrap";
import { Download, ArrowRight } from 'react-feather';
import { DiscoveryResultGroup, DiscoveryResultItem, DiscoveryResultItemProps } from "../../ts/components/discovery_result";

const KEEP_ALIVE_INTERVAL = 3000; // milliseconds
const LOG_INTERVAL = 250; // milliseconds

export interface PortScanResult {
    ip4: string;
    port: number;
    open: boolean;
}

interface PortScannerProps {
    ip4_first: string;
    ip4_last: string;
    port: number;
    on_result_selected: (result: PortScanResult) => void;
}

interface PortScannerState {
    ip4_first: string;
    ip4_last: string;
    port: number;
    running: boolean;
    cookie: number;
    aborted: boolean;
    progress: number;
    log: string;
    show_log: boolean;
    results: PortScanResult[];
}

export class PortScanner extends Component<PortScannerProps, PortScannerState> {
    keep_alive_timer: number = undefined;
    log_timer: number = undefined;
    pending_log: string = '';

    constructor(props: PortScannerProps) {
        super(props);

        this.state = {
            ip4_first: props.ip4_first,
            ip4_last: props.ip4_last,
            port: props.port,
            running: false,
            cookie: null,
            aborted: false,
            progress: 0,
            log: '',
            show_log: false,
            results: [],
        } as any;

        util.addApiEventListener('port_scanner/scan_log', () => {
            let scan_log = API.get('port_scanner/scan_log');

            if (!this.state.running || scan_log.cookie !== this.state.cookie) {
                return;
            }

            this.pending_log += scan_log.message;

            if (this.state.log.length == 0) {
                this.update_log();
            }
        });

        util.addApiEventListener('port_scanner/scan_progress', () => {
            let scan_progress = API.get('port_scanner/scan_progress');

            if (!this.state.running || scan_progress.cookie !== this.state.cookie) {
                return;
            }

            this.setState({progress: scan_progress.progress});
        });

        util.addApiEventListener('port_scanner/scan_result', () => {
            let scan_result = API.get('port_scanner/scan_result');

            if (!this.state.running || scan_result.cookie !== this.state.cookie) {
                return;
            }

            this.setState({results: this.state.results.concat({
                ip4: scan_result.ip4,
                port: scan_result.port,
                open: scan_result.open,
            })});
        });

        util.addApiEventListener('port_scanner/scan_done', () => {
            let scan_done = API.get('port_scanner/scan_done');

            if (!this.state.running || scan_done.cookie !== this.state.cookie) {
                return;
            }

            if (this.keep_alive_timer !== undefined) {
                clearInterval(this.keep_alive_timer);
                this.keep_alive_timer = undefined;
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

    async abort() {
        if (!this.state.running || this.state.aborted) {
            return;
        }

        let result;

        try {
            result = await (await util.put('/port_scanner/scan_abort', {cookie: this.state.cookie}, true)).text();
        }
        catch (e) {
            result = e.message.replace('400(Bad Request) ', '');
        }

        if (result.length > 0) {
            this.update_log(__("port_scanner.content.scan_abort_error") + ": " + result + "\n");
        }
        else {
            this.setState({aborted: true});
        }
    }

    override async componentWillUnmount() {
        await this.abort();
    }

    get_result_item(result: PortScanResult): VNode<DiscoveryResultItemProps> {
        return (
            <DiscoveryResultItem
                key={result.ip4}
                title={<h5>{result.ip4}</h5>}
                labelAdd={<ArrowRight />}
                error={result.open ? undefined : <span class="text-danger">{__("port_scanner.content.scan_result_port_closed")}</span>}
                onClick={() => this.props.on_result_selected(result)}>
                {__("port_scanner.content.scan_result_port")(result.port)}
            </DiscoveryResultItem>);
    }

    async keep_alive() {
        if (!this.state.running) {
            return;
        }

        let result;

        try {
            result = await (await util.put('/port_scanner/scan_keep_alive', {cookie: this.state.cookie}, true)).text();
        }
        catch (e) {
            result = e.message.replace('400(Bad Request) ', '');
        }

        if (result.length > 0) {
            this.update_log(__("port_scanner.content.scan_keep_alive_error") + ": " + result + "\n");
        }
    }

    render() {
        return <>
            <FormRow label={__("port_scanner.content.scan_title")} label_muted={__("port_scanner.content.scan_title_muted")}>
                <div class="row mb-3 gx-2">
                    <div class="col-sm-4">
                        <InputIP
                            required
                            disabled={this.state.running}
                            invalidFeedback={__("port_scanner.content.scan_ip4_invalid")}
                            value={this.state.ip4_first}
                            onValue={(v) => this.setState({ip4_first: v})} />
                    </div>
                    <div class="col-sm-4">
                        <InputIP
                            required
                            disabled={this.state.running}
                            invalidFeedback={__("port_scanner.content.scan_ip4_invalid")}
                            value={this.state.ip4_last}
                            onValue={(v) => this.setState({ip4_last: v})} />
                    </div>
                    <div class="col-sm-4">
                        <InputNumber
                            required
                            disabled={this.state.running}
                            min={1}
                            max={65535}
                            value={this.state.port}
                            onValue={(v) => this.setState({port: v})} />
                    </div>
                </div>
            {!this.state.running ?
                <Button key="scan_start"
                        variant="primary"
                        className="form-control"
                        onClick={async () => {
                            let cookie: number = Math.floor(Math.random() * 0xFFFFFFFF);

                            this.pending_log = '';

                            this.setState({
                                running: true,
                                cookie: cookie,
                                aborted: false,
                                show_log: true,
                                progress: 0,
                                log: '',
                                results: [],
                            }, async () => {
                                let result;

                                try {
                                    result = await (await util.put('/port_scanner/scan_start', {
                                        ip4_first: this.state.ip4_first,
                                        ip4_last: this.state.ip4_last,
                                        port: this.state.port,
                                        cookie: cookie,
                                    }, true)).text();
                                }
                                catch (e) {
                                    result = e.message.replace('400(Bad Request) ', '');
                                }

                                if (result.length > 0) {
                                    this.update_log(__("port_scanner.content.scan_start_error") + ": " + result + "\n");
                                    this.setState({running: false, cookie: null});

                                    return;
                                }

                                this.keep_alive_timer = window.setInterval(async () => {await this.keep_alive()}, KEEP_ALIVE_INTERVAL);
                                this.log_timer = window.setInterval(() => this.update_log(), LOG_INTERVAL);
                            });
                        }}
                        disabled={!util.hasValue(this.state.port)}>
                    {__("port_scanner.content.scan_start")}
                </Button> :
                <Button key="scan_abort"
                        variant="primary"
                        className="form-control"
                        onClick={async () => await this.abort()}
                        disabled={this.state.aborted}>
                    {__("port_scanner.content.scan_abort")}
                </Button>}
            </FormRow>

            {this.state.running ?
                <FormRow>
                    <Progress progress={this.state.progress / 100} />
                </FormRow>
                : undefined}

            {this.state.show_log ?
                <FormRow>
                    <OutputTextarea rows={10} resize='vertical' value={this.state.log} />
                </FormRow>
                : undefined}

            {!this.state.running && this.state.progress == 100 && this.state.results.length == 0 ?
                <FormRow label={__("port_scanner.content.scan_results")}>
                    <Alert variant="warning" className="mb-0">{__("port_scanner.content.scan_no_result")}</Alert>
                </FormRow>
                : undefined}

            {this.state.results.length > 0 ?
                <FormRow label={__("port_scanner.content.scan_results")}>
                    <DiscoveryResultGroup>
                        {this.state.results.map((result) => this.get_result_item(result))}
                    </DiscoveryResultGroup>
                </FormRow>
                : undefined}
        </>;
    }
}
