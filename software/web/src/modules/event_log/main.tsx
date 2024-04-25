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

import * as util from "../../ts/util";
import { h, Component } from "preact";
import { __ } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";
import { FormRow } from "../../ts/components/form_row";
import { Button, Spinner } from "react-bootstrap";
import { SubPage } from "../../ts/components/sub_page";
import { OutputTextarea } from "../../ts/components/output_textarea";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Download, FileText } from "react-feather";
import { blobToBase64 } from "../../ts/util";

export function EventLogNavbar() {
    return <NavbarItem name="event_log" module="event_log" title={__("event_log.navbar.event_log")} symbol={<FileText />} />;
}

interface EventLogState {
    log: string;
    show_spinner: boolean;
}

const TIMESTAMP_LEN = 24;
const TIMESTAMP_REGEX = /^(\d{4})-(\d{2})-(\d{2}) (\d{2}):(\d{2}):(\d{2}),(\d{3}) $/;
const RELATIVE_TIME_REGEX = /^\s+(\d+),(\d{3}) $/;
const LOG_MAX_LEN = 10 * 1024 * 1024;
const LOG_CHUNK_LEN_DROPPED_WHEN_FULL = 1024 * 1024;

export class EventLog extends Component<{}, EventLogState> {
    last_boot_id = -1;

    constructor() {
        super();

        util.addApiEventListener("event_log/boot_id", (ev) => {
            this.load_event_log(this.last_boot_id != ev.data.boot_id);
            this.last_boot_id = ev.data.boot_id;
        });

        util.addApiEventListener("event_log/message", (ev) => {
            // If we have not seen the complete log yet, don't add updates.
            if (!this.state.log)
                return;

            this.set_log(this.state.log + ev.data + "\n");
        });
    }

    set_log(log: string) {
        if (log.length > LOG_MAX_LEN)
            log = log.slice(log.indexOf("\n", LOG_CHUNK_LEN_DROPPED_WHEN_FULL) + 1)

        this.setState({log: log});
    }

    get_line_date(line: string) {
        let match = line.substring(0, TIMESTAMP_LEN).match(TIMESTAMP_REGEX);
        if (match == null) {
            match = line.substring(0, TIMESTAMP_LEN).match(RELATIVE_TIME_REGEX);
            if (match == null)
                return null;
            let nums = match.slice(1).map(x => parseInt(x)) as [number, number];
            return new Date(nums[0] * 1000 + nums[1]);
        }
        let nums = match.slice(1).map(x => parseInt(x)) as [number,number,number,number,number,number,number];
        // Month is index from 0 to 11.
        nums[1]--;
        return new Date(...nums);
    }

    load_event_log(reboot: boolean) {
        util.download("/event_log")
            .then(blob => blob.text())
            .then(text => {
                util.remove_alert("event_log_load_failed");

                if (!text || text.length == 0)
                    return;

                if (!this.state.log) {
                    this.set_log(text);
                    return;
                }

                const new_lines = text.split("\n");
                let first_new_line = null;
                let first_new_date = null;

                if (new_lines.length == 0) {
                    this.set_log(text);
                    return;
                } else {
                    first_new_line = new_lines[0];
                    first_new_date = this.get_line_date(first_new_line);
                }

                if (first_new_date == null) {
                    this.set_log(text);
                    return;
                }

                // string.trimEnd is in es2019
                // log starts with (relevant!) whitespace
                const log = this.state.log.endsWith("\n") ? this.state.log.slice(0, -1) : this.state.log;

                if (reboot) {
                    this.set_log(log + "\n" + "-".repeat(TIMESTAMP_LEN - 2) + "  [Reboot]\n" + text);
                    return;
                }

                const old_lines = log.split("\n");

                let i = old_lines.length - 1;
                for (; i >= 0; --i) {
                    const line = old_lines[i];
                    if (line == ("-".repeat(TIMESTAMP_LEN - 2) + "  [Reboot]"))
                        break;

                    let date = this.get_line_date(line);
                    if (date == null)
                        continue;

                    if (date < first_new_date) {
                        ++i;
                        break;
                    }

                    // == compares references on objects and Date has no equals method.
                    // Isn't javascript fun?
                    if (date.getTime() == first_new_date.getTime() && line == first_new_line)
                        break;
                }

                if (i < 0) {
                    this.set_log(text);
                    return;
                }

                let new_log = old_lines.slice(0, i).join("\n");
                if (new_log.length > 0 && !new_log.endsWith("\n"))
                    new_log += "\n"
                if (i == old_lines.length)
                    new_log += "-".repeat(TIMESTAMP_LEN - 2) + "  [WebSocket reconnect]\n";
                new_log += text;

                this.set_log(new_log);
            })
            .catch(e => util.add_alert("event_log_load_failed", "danger", __("event_log.script.load_event_log_error"), e.message))
    }

    async download_debug_report() {
        let timeout = window.setTimeout(() => this.setState({show_spinner: true}), 1000);

        try {
            let timestamp = new Date();
            let debug_log = util.iso8601ButLocal(timestamp) + "\nScroll down for event log!\n\n";

            debug_log += await util.download("/debug_report").then(blob => blob.text());
            debug_log += "\n\n";
            debug_log += this.state.log;
            try {
                let blob = await util.download("/coredump/coredump.elf");
                let base64 = await blobToBase64(blob);
                base64 = base64.replace(/(.{80})/g, "$1\n");
                debug_log += "\n\n___CORE_DUMP_START___\n\n";
                debug_log += base64;
            }
            catch (e) {
                if (typeof(e) == "string" && e.includes("404"))
                    debug_log += "\n\nNo core dump recorded.";
            }

            util.downloadToFile(debug_log, __("event_log.content.debug_report_file"), "txt", "text/plain", timestamp);
        } catch (e) {
            util.add_alert("debug_report_load_failed", "danger", __("event_log.script.load_debug_report_error"), e.message)
        } finally {
            window.clearTimeout(timeout);
            this.setState({show_spinner: false})
        }
    }


    render(props: {}, state: Readonly<EventLogState>) {
        if (!util.render_allowed())
            return <SubPage name="event_log" />;

        return (
            <SubPage name="event_log" colClasses="col-xl-10">
                <PageHeader title={__("event_log.content.event_log")} />

                <OutputTextarea moreClass="form-group"
                        value={state.log}
                        placeholder={__("event_log.content.event_log_placeholder")}
                        style="resize: both; width: 100%; white-space: pre; line-height: 1.4; text-shadow: none; font-size: 0.75rem;"
                        />

                <div class="form-group">
                    <Button variant="primary" className="form-control" onClick={() => this.download_debug_report()} style="height: unset;">
                        <span class="text-nowrap">{__("event_log.content.debug_report")}</span>{" "}
                        <span class="text-nowrap">
                            <span class="ml-1 mr-2">{__("event_log.content.debug_report_no_passwords")}</span>
                            <Download/>
                            <Spinner animation="border" size="sm" as="span" className="ml-2" hidden={!state.show_spinner}/>
                        </span>
                    </Button>
                </div>
            </SubPage>
        );
    }
}

export function init() {
}
