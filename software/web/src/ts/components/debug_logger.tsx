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

import * as util from "../../ts/util";
import { h, Component, Fragment } from "preact";
import { Button } from "react-bootstrap";
import { FormRow } from "./form_row";
import { __ } from "../translation";
import { InputText } from "./input_text";

interface DebugLoggerState {
    debug_running: boolean;
    debug_status: string;
}

interface DebugLoggerProps {
    description: string
    description_muted: string
    name: string
    filename: string
}

export class DebugLogger extends Component<DebugLoggerProps, DebugLoggerState>
{
    debug_prefix: string = '';
    debug_protocol_header: string = '';
    debug_protocol_lines_dropped: number = 0;
    debug_protocol_lines: Array<string> = [];
    debug_suffix: string = '';

    constructor(props: any) {
        super(props);

        this.state = {
            debug_running: false,
            debug_status: ""
        }

        util.addApiEventListener("debug_protocol/header", (e) => {
            this.debug_protocol_header = e.data + "\n";
        }, false);

        util.addApiEventListener("debug_protocol/line", (e) => {
            while (this.debug_protocol_lines.length > 20000) {
                this.debug_protocol_lines.shift();
                ++this.debug_protocol_lines_dropped;
            }

            this.debug_protocol_lines.push(e.data + "\n");
        }, false);
    }

    async get_debug_report_and_event_log() {
        let text = '';

        try {
            this.setState({debug_status: __("component.debug_logger.loading_debug_report")});
            text += await util.download("/debug_report").then(blob => blob.text()) + "\n\n";
        } catch (error) {
            this.setState({debug_running: false, debug_status: __("component.debug_logger.loading_debug_report_failed")});
            throw __("component.debug_logger.loading_debug_report_failed") + ": " + error;
        }

        try {
            this.setState({debug_status: __("component.debug_logger.loading_event_log")});
            text += await util.download("/event_log").then(blob => blob.text()) + "\n";
        } catch (error) {
            this.setState({debug_running: false, debug_status: __("component.debug_logger.loading_event_log_failed")});
            throw __("component.debug_logger.loading_event_log_failed") + ": " + error;
        }

        return text;
    }

    debugTimeout: number;

    async resetDebugWd() {
        try {
            await util.download("/debug_protocol/continue");
        }
        catch{
            this.setState({debug_running: false, debug_status: __("component.debug_logger.starting_debug_failed")(this.props.name)});
        }
    }

    async debug_start() {
        this.debug_prefix = '';
        this.debug_protocol_header = '';
        this.debug_protocol_lines_dropped = 0;
        this.debug_protocol_lines = [];
        this.debug_suffix = '';
        this.setState({debug_running: true});

        try {
            this.debug_prefix = await this.get_debug_report_and_event_log();

            this.setState({debug_status: __("component.debug_logger.starting_debug")(this.props.name)});
        } catch(error) {
            this.setState({debug_running: false, debug_status: error});
            return;
        }

        try {
            await util.download("/debug_protocol/start");
        } catch {
            this.setState({debug_running: false, debug_status: __("component.debug_logger.starting_debug_failed")(this.props.name)});
            return;
        }

        this.debugTimeout = setInterval(this.resetDebugWd, 60000);

        this.setState({debug_status: __("component.debug_logger.debug_running")});
    }

    async debug_stop() {
        this.setState({debug_running: false});
        clearInterval(this.debugTimeout);

        try {
            await util.download("/debug_protocol/stop");
        } catch {
            this.setState({debug_running: true, debug_status: __("component.debug_logger.debug_stop_failed")(this.props.name)});
        }

        try {
            this.setState({debug_status: __("component.debug_logger.debug_stopped")(this.props.name)});
            this.debug_suffix = "\n" + await this.get_debug_report_and_event_log();
            this.setState({debug_status: __("component.debug_logger.debug_done")});
        } catch (error) {
            this.debug_suffix = "\nError while stopping charge protocol: " + error;
            this.setState({debug_status: error});
        }

        let full_log = [this.debug_prefix];

        if (this.debug_protocol_lines_dropped > 0) {
            full_log.push('' + this.debug_protocol_lines_dropped + ' lines have been dropped from the following table.\n\n');
        }

        full_log.push(this.debug_protocol_header);
        full_log = full_log.concat(this.debug_protocol_lines);
        full_log.push(this.debug_suffix);

        //Download log in any case: Even an incomplete log can be useful for debugging.
        util.downloadToFile(full_log.join(''), this.props.filename, "txt", "text/plain");
    }

    render(props: DebugLoggerProps, s: DebugLoggerState)
    {
        if (!util.render_allowed())
            return <></>;

        let {
            debug_status,
            debug_running
        } = s;

        if (debug_running) {
            window.onbeforeunload = (e: Event) => {
                e.preventDefault();
                // returnValue is not a boolean, but the string to be shown
                // in the "are you sure you want to close this tab" message
                // box. However this string is only shown in some browsers.
                e.returnValue = __("component.debug_logger.tab_close_warning")(this.props.name) as any;
            }
        } else {
            window.onbeforeunload = null;
        }

        return <>
                    <FormRow label={this.props.description} label_muted={this.props.description_muted}>
                        <div class="input-group pb-2">
                            <Button variant="primary" className="form-control rounded-right mr-2" onClick={() => {this.debug_start()}} disabled={debug_running}>{__("component.debug_logger.debug_start")}</Button>
                            <Button variant="primary" className="form-control rounded-left" onClick={() => {this.debug_stop()}} disabled={!debug_running}>{__("component.debug_logger.debug_stop")}</Button>
                        </div>
                        <InputText value={debug_status}/>
                    </FormRow>
            </>;
    }
}
