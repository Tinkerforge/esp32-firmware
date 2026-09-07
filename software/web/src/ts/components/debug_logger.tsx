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

interface FetchDebugReportParams {
        prefix?: string,
        progress?: (status: 'running' | 'done' | 'error', msg: string) => Promise<void>,
        content?: ('debug_report' | 'event_log' | 'trace_log' | 'coredump')[]
        override_fetch_event_log?: () => Promise<string>
}

export async function fetch_debug_report(p: Partial<FetchDebugReportParams>) {
    let text = "";

    const {
        prefix = "",
        progress = ((status, msg) => {}),
        content = ['debug_report', 'event_log', 'trace_log', 'coredump'],
        override_fetch_event_log = () => util.download("/event_log", true).then(blob => blob.text())
    } = p;

    if (content.indexOf('debug_report') >= 0) {
        await progress('running', __("component.debug_logger.loading_debug_report"));
        text += `___${prefix}DEBUG_REPORT_START___\n\n`;

        try {
            text += await util.download("/debug_report", true).then(blob => blob.text()) + "\n\n";
        } catch (e) {
            await progress('error', __("component.debug_logger.loading_debug_report_failed"));
            throw __("component.debug_logger.loading_debug_report_failed") + ": " + e;
        }

        text += `___${prefix}DEBUG_REPORT_END___\n\n`;
    }

    if (content.indexOf('event_log') >= 0) {
        await progress('running', __("component.debug_logger.loading_event_log"));
        text += `___${prefix}EVENT_LOG_START___\n\n`;

        try {
            text += await override_fetch_event_log() + "\n\n";
        } catch (e) {
            await progress('error', __("component.debug_logger.loading_event_log_failed"));
            throw __("component.debug_logger.loading_event_log_failed") + ": " + e;
        }

        text += `___${prefix}EVENT_LOG_END___\n\n`;
    }

    if (content.indexOf('trace_log') >= 0) {
        await progress('running', __("component.debug_logger.loading_trace_log"));
        text += `___${prefix}TRACE_LOG_START___\n\n`;

        try {
            text += (await util.download("/trace_log", true, 40000).then(blob => blob.text())).replace(/\s+$/, "") + "\n\n";
        }
        catch (e) {
            await progress('error', __("component.debug_logger.loading_trace_log_failed"));
            throw __("component.debug_logger.loading_trace_log_failed") + ": " + e;
        }

        text += `___${prefix}TRACE_LOG_END___\n\n`;
    }

    if (content.indexOf('coredump') >= 0) {
        await progress('running', __("component.debug_logger.loading_coredump"));
        text += `___${prefix}COREDUMP_START___\n\n`;

        try {
            text += (await util.download("/coredump/coredump.elf", true).then(util.blobToBase64)).replace(/(.{80})/g, "$1\n").trim() + "\n\n";
        }
        catch (e) {
            const msg = typeof(e) == "string" ? e : e?.message;
            if (!msg || !msg.startsWith("404")) {
                await progress('error', __("component.debug_logger.loading_coredump_failed"));
                throw __("component.debug_logger.loading_coredump_failed") + ": " + e;

            }
            text += "No core dump stored\n\n";
        }

        text += `___${prefix}COREDUMP_END___\n\n`;
    }

    await progress('done', "");

    return text;
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

    debugTimeout: number;

    async resetDebugWd() {
        try {
            await util.download("/debug_protocol/continue", true);
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
            this.debug_prefix = await fetch_debug_report({
                prefix: "PRE_",
                progress: async (status, msg) => this.setState({debug_status: msg}),
                content: ['debug_report', 'event_log', 'trace_log']
            });

            this.setState({debug_status: __("component.debug_logger.starting_debug")(this.props.name)});
        } catch(error) {
            this.setState({debug_running: false, debug_status: error});
            return;
        }

        try {
            await util.download("/debug_protocol/start", true);
        } catch {
            this.setState({debug_running: false, debug_status: __("component.debug_logger.starting_debug_failed")(this.props.name)});
            return;
        }

        this.debugTimeout = window.setInterval(this.resetDebugWd, 60000);

        this.setState({debug_status: __("component.debug_logger.debug_running")});
    }

    async debug_stop() {
        this.setState({debug_running: false});
        window.clearInterval(this.debugTimeout);

        try {
            await util.download("/debug_protocol/stop", true);
        } catch {
            this.setState({debug_running: true, debug_status: __("component.debug_logger.debug_stop_failed")(this.props.name)});
        }

        try {
            this.setState({debug_status: __("component.debug_logger.debug_stopped")(this.props.name)});
            this.debug_suffix = "\n" + await fetch_debug_report({
                prefix: "POST_",
                progress: async (status, msg) => this.setState({debug_status: msg})
            });
            this.setState({debug_status: __("component.debug_logger.debug_done")});
        } catch (error) {
            this.debug_suffix = "\nError while stopping charge protocol: " + error;
            this.setState({debug_status: error});
        }

        let full_log = [this.debug_prefix];

        full_log.push("___DEBUG_PROTOCOL_START___\n\n");

        if (this.debug_protocol_lines_dropped > 0) {
            full_log.push('' + this.debug_protocol_lines_dropped + ' lines have been dropped from the following table.\n\n');
        }
        full_log.push(this.debug_protocol_header);
        full_log = full_log.concat(this.debug_protocol_lines);

        full_log.push("___DEBUG_PROTOCOL_END___\n\n");

        full_log.push(this.debug_suffix);


        // Download log in any case: Even an incomplete log can be useful for debugging.
        util.downloadToTimestampedFile(full_log.join(''), this.props.filename, "txt", "text/plain");
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
                        <div class="row g-2">
                            <div class="col">
                                <Button variant="primary" className="w-100" onClick={() => {this.debug_start()}} disabled={debug_running}>{__("component.debug_logger.debug_start")}</Button>
                            </div>
                            <div class="col">
                                <Button variant="primary" className="w-100" onClick={() => {this.debug_stop()}} disabled={!debug_running}>{__("component.debug_logger.debug_stop")}</Button>
                            </div>
                        </div>
                        <InputText class="mt-2" value={debug_status}/>
                    </FormRow>
            </>;
    }
}
