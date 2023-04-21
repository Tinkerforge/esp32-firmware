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

import $ from "../../ts/jq";

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, Component, Fragment } from "preact";
import { Button } from "react-bootstrap";
import { FormRow } from "./form_row";
import { __, translate_unchecked} from "../translation";
import { ConfigMap } from "../api_defs";
import { InputText } from "./input_text";

interface DebugLoggerState {
    debug_running: boolean;
    debug_status: string;
}

interface DebugLoggerProps {
    debugHeader: keyof ConfigMap
    debug: keyof ConfigMap
    prefix: string
    translationPrefix: string
}

export class DebugLogger extends Component<DebugLoggerProps, DebugLoggerState>
{
    debug_log = "";

    constructor(props: any)
    {
        super(props);

        this.state = {
            debug_running: false,
            debug_status: ""
        }

        util.addApiEventListener(this.props.debugHeader, (e) => {
            this.debug_log += e.data + "\n";
        }, false);

        util.addApiEventListener(this.props.debug, (e) => {
            this.debug_log += e.data + "\n";
        }, false);
    }

    async get_debug_report_and_event_log() {
        try {
            this.setState({debug_status: translate_unchecked(this.props.translationPrefix + ".script.loading_debug_report")});
            this.debug_log += await util.download("/debug_report").then(blob => blob.text());
            this.debug_log += "\n\n";
        } catch (error) {
            this.setState({debug_running: false, debug_status: translate_unchecked(this.props.translationPrefix + ".script.loading_debug_report_failed")});
            throw translate_unchecked(this.props.translationPrefix + ".script.loading_debug_report_failed") + ": " + error;
        }

        try {
            this.setState({debug_status: translate_unchecked(this.props.translationPrefix + ".script.loading_event_log")});
            this.debug_log += await util.download("/event_log").then(blob => blob.text());
            this.debug_log += "\n";
        } catch (error) {
            this.setState({debug_running: false, debug_status: translate_unchecked(this.props.translationPrefix + ".script.loading_event_log_failed")});
            throw translate_unchecked(this.props.translationPrefix + ".script.loading_event_log_failed") + ": " + error;
        }
    }

    debugTimeout: number;

    async resetDebugWd(prefix: string) {
        try {
            await util.download("/" + prefix + "/continue_debug");
        }
        catch{
            this.setState({debug_running: false, debug_status: translate_unchecked(this.props.translationPrefix + ".script.starting_debug_failed")});
        }
    }

    async debug_start() {
        this.debug_log = "";
        this.setState({debug_running: true});

        try {
            await this.get_debug_report_and_event_log();

            this.setState({debug_status: translate_unchecked(this.props.translationPrefix + ".script.starting_debug")});
        } catch(error) {
            this.setState({debug_running: false, debug_status: error});
            return;
        }

        try{
            await util.download("/" + this.props.prefix + "/start_debug");
        } catch {
            this.setState({debug_running: false, debug_status: translate_unchecked(this.props.translationPrefix + ".script.starting_debug_failed")});
            return;
        }

        this.debugTimeout = setInterval(this.resetDebugWd, 15000, this.props.prefix);

        this.setState({debug_status: translate_unchecked(this.props.translationPrefix + ".script.debug_running")});
    }

    async debug_stop() {
        this.setState({debug_running: false});
        clearInterval(this.debugTimeout);

        try {
            await util.download("/" + this.props.prefix + "/stop_debug");
        } catch {
            this.setState({debug_running: true, debug_status: translate_unchecked(this.props.translationPrefix + ".script.debug_stop_failed")});
        }

        try {
            this.debug_log += "\n\n";
            this.setState({debug_status: translate_unchecked(this.props.translationPrefix + ".script.debug_stopped")});

            await this.get_debug_report_and_event_log();
            this.setState({debug_status: translate_unchecked(this.props.translationPrefix + ".script.debug_done")});
        } catch (error) {
            this.debug_log += "\n\nError while stopping charge protocol: ";
            this.debug_log += error;

            this.setState({debug_status: error});
        }

        //Download log in any case: Even an incomplete log can be useful for debugging.
        util.downloadToFile(this.debug_log, this.props.prefix + "-debug-log", "txt", "text/plain");
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
                e.returnValue = translate_unchecked(props.translationPrefix + ".script.tab_close_warning") as any;
            }
        } else {
            window.onbeforeunload = null;
        }

        return <>
                    <FormRow label={translate_unchecked(props.translationPrefix + ".content.debug_description")} label_muted={translate_unchecked(props.translationPrefix + ".content.debug_description_muted")}>
                        <div class="input-group pb-2">
                            <Button variant="primary" className="form-control rounded-right mr-2" onClick={() => {this.debug_start()}} disabled={debug_running}>{translate_unchecked(props.translationPrefix + ".content.debug_start")}</Button>
                            <Button variant="primary" className="form-control rounded-left" onClick={() => {this.debug_stop()}} disabled={!debug_running}>{translate_unchecked(props.translationPrefix + ".content.debug_stop")}</Button>
                        </div>
                        <InputText value={debug_status}/>
                    </FormRow>
            </>;
    }
}
