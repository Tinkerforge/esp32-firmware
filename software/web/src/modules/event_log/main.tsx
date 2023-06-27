/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

import { h, render, Fragment, Component } from "preact";
import { __ } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";


import { FormRow } from "../../ts/components/form_row";
import { Button, Spinner } from "react-bootstrap";

import { Download } from 'react-feather';
import { SubPage } from "src/ts/components/sub_page";

interface EventLogState {
    log: string
    show_spinner: boolean
}

export class EventLog extends Component<{}, EventLogState> {
    update_event_log_interval: number = null;

    constructor() {
        super();

        // We have to use jquery here or else the events don't fire?
        // This can be removed once the sidebar is ported to preact.
        $('#sidebar-event_log').on('shown.bs.tab', () => {
            this.load_event_log();
            if (this.update_event_log_interval == null) {
                this.update_event_log_interval = window.setInterval(() => this.load_event_log(), 10000);
            }
        });

        $('#sidebar-event_log').on('hidden.bs.tab', () => {
            if (this.update_event_log_interval != null) {
                clearInterval(this.update_event_log_interval);
                this.update_event_log_interval = null;
            }
        });
    }

    load_event_log() {
        util.download("/event_log")
            .then(blob => blob.text())
            .then(text => {
                this.setState({log: text});
                util.remove_alert("event_log_load_failed");
            })
            .catch(e => util.add_alert("event_log_load_failed", "alert-danger", __("event_log.script.load_event_log_error"), e.message))
    }

    blobToBase64(blob: Blob): Promise<string> {
        return new Promise((resolve, _) => {
            const reader = new FileReader();
            reader.onloadend = () => resolve(reader.result as string);
            reader.readAsDataURL(blob);
        })
    }

    async download_debug_report() {
        let timeout = window.setTimeout(() => this.setState({show_spinner: true}), 1000);

        try {
            let t = (new Date()).toISOString().replace(/:/gi, "-").replace(/\./gi, "-");
            let debug_log = t + "\nScroll down for event log!\n\n";

            debug_log += await util.download("/debug_report").then(blob => blob.text());
            debug_log += "\n\n";
            debug_log += await util.download("/event_log").then(blob => blob.text());
            try {
                let blob = await util.download("/coredump/coredump.elf");
                let base64 = await this.blobToBase64(blob);
                base64 = base64.replace(/(.{80})/g, "$1\n");
                debug_log += "\n\n___CORE_DUMP_START___\n\n";
                debug_log += base64;
            }
            catch (e) {
                if (typeof(e) == "string" && e.includes("404"))
                    debug_log += "\n\nNo core dump recorded.";
            }

            util.downloadToFile(debug_log, "debug-report", "txt", "text/plain");
        } catch (e) {
            util.add_alert("debug_report_load_failed", "alert-danger", __("event_log.script.load_debug_report_error"), e.message)
        } finally {
            window.clearTimeout(timeout);
            this.setState({show_spinner: false})
        }
    }

    render(props: {}, state: Readonly<EventLogState>) {
        if (!util.render_allowed())
            return (<></>);

        return (
            <SubPage>
                <PageHeader title={__("event_log.content.event_log")} />

                <FormRow label={__("event_log.content.event_log_desc")} label_muted={__("event_log.content.event_log_desc_muted")}>
                    <textarea class="text-monospace mb-1 form-control"
                              readonly
                              id="event_log_content"
                              rows={20}
                              style="resize: both; width: 100%; white-space: pre; line-height: 1.2; text-shadow: none; font-size: 0.875rem;"
                              placeholder={__("event_log.content.event_log_placeholder")}>
                        {state.log}
                    </textarea>
                </FormRow>

                <FormRow label={__("event_log.content.debug_report_desc")} label_muted={__("event_log.content.debug_report_desc_muted")}>
                    <Button variant="primary" className="form-control" onClick={() => this.download_debug_report()}>
                        <span class="mr-2">{__("event_log.content.debug_report")}</span>
                        <Download/>
                        <Spinner animation="border" size="sm" as="span" className="ml-2" hidden={!state.show_spinner}/>
                    </Button>
                </FormRow>
            </SubPage>
        );
    }
}

render(<EventLog/>, $('#event_log')[0])

export function init() {
}

export function add_event_listeners(source: API.APIEventTarget) {

}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-event_log').prop('hidden', !module_init.event_log);
}
