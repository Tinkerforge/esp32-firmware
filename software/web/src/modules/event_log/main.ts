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

declare function __(s: string): string;

function load_event_log() {
    $.get("/event_log")
               .done((result) => {
                   $('#event_log_content').val(result);
                   util.remove_alert("event_log_load_failed");
                })
               .fail((xhr, status, error) => util.add_alert("event_log_load_failed", "alert-danger", __("event_log.script.load_event_log_error"), error + ": " + xhr.responseText))
}

let update_event_log_interval: number = null;

export function init() {
    $('#sidebar-event_log').on('shown.bs.tab', function (e) {
        load_event_log();
        if (update_event_log_interval == null) {
            update_event_log_interval = window.setInterval(load_event_log, 10000);
        }
    });

    $('#sidebar-event_log').on('hidden.bs.tab', function (e) {
        if (update_event_log_interval != null) {
            clearInterval(update_event_log_interval);
            update_event_log_interval = null;
        }
    });

    $('#download_debug_report').on("click", () => {
        let timeout = window.setTimeout(() => $('#debug_report_spinner').prop("hidden", false), 1000);
        let t = (new Date()).toISOString().replace(/:/gi, "-").replace(/\./gi, "-");

        let debug_log = t + "\nScroll down for event log!\n\n";
        $.get({url: "/debug_report", dataType: "text"})
                .fail((xhr, status, error) => {
                    util.add_alert("debug_report_load_failed", "alert-danger", __("event_log.script.load_debug_report_error"), error + ": " + xhr.responseText)
                    window.clearTimeout(timeout);
                    $('#debug_report_spinner').prop("hidden", true);
                })
                .done((result) => {
                    util.remove_alert("debug_report_load_failed");
                    debug_log += result + "\n\n";

                    $.get("/event_log")
                        .fail((xhr, status, error) => {
                            util.add_alert("debug_report_load_failed", "alert-danger", __("event_log.script.load_debug_report_error"), error + ": " + xhr.responseText)
                            window.clearTimeout(timeout);
                            $('#debug_report_spinner').prop("hidden", true);
                        })
                        .done((result) => {
                            util.remove_alert("debug_report_load_failed");
                            debug_log += result + "\n";
                            util.downloadToFile(debug_log, "debug-report", "txt", "text/plain");
                            window.clearTimeout(timeout);
                            $('#debug_report_spinner').prop("hidden", true);
                        });
                });
    });
}

export function add_event_listeners(source: API.APIEventTarget) {

}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-event_log').prop('hidden', !module_init.event_log);
}
