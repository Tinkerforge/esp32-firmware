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

import $ from "jquery";
import { createRef, RefObject } from "preact";

import * as API from "./api";
import { __ } from "./translation";

import { AsyncModal } from "./components/async_modal";

export function reboot() {
    API.call("reboot", null, "").then(() => postReboot(__("util.reboot_title"), __("util.reboot_text")));
}

export function update_button_group(button_group_id: string, index_to_select: number, text_replacement?: string) {
    let buttons = $(`#${button_group_id} :button`);
    let color_suffixes = Array(buttons.length);
    for (let i = 0; i < buttons.length; ++i) {
        let classes = buttons[i].classList;
        for (let j = 0; j < classes.length; ++j) {
            if (classes[j].substring(0, 4) != "btn-")
                continue
            let splt = classes[j].split("-");
            color_suffixes[i] = splt[splt.length - 1];
        }
    }

    for (let i = 0; i < buttons.length; ++i) {
        buttons[i].classList.remove("btn-" + color_suffixes[i]);
        buttons[i].classList.add("btn-outline-" + color_suffixes[i]);
    }

    buttons[index_to_select].classList.remove("btn-outline-" + color_suffixes[index_to_select]);
    buttons[index_to_select].classList.add("btn-" + color_suffixes[index_to_select]);
    if (text_replacement != null)
        buttons[index_to_select].innerHTML = text_replacement;
}

export function add_alert(id: string, cls: string, title: string, text: string) {
    let to_add = `<div id="alert_${id}" class="alert ${cls} alert-dismissible fade show custom-alert" role="alert" style="line-height: 1.5rem;">
    <strong>${title}</strong> ${text}
    <button type="button" class="close" data-dismiss="alert" aria-label="Close">
    <span aria-hidden="true">&times;</span>
    </button>
</div>`;

    if(!document.getElementById(`alert_${id}`)) {
        $('#alert_placeholder').append(to_add);
    } else {
        $(`#alert_${id}`).replaceWith(to_add);
    }
}

export function remove_alert(id: string) {
    $(`#alert_${id}`).remove();
}

export function format_timespan(secs: number) {
    let days = 0;
    let hours = 0;
    let mins = 0;

    let dayString = "";
    let hourString = "";
    let minString = "";
    let secString = "";

    if (secs >= 60 * 60 * 24) {
        days = Math.floor(secs / (60 * 60 * 24));
        if (days > 1) {
            dayString = days + ` ${__("days")}, `;
        } else if (days > 0) {
            dayString = days + ` ${__("day")}, `;
        }
        secs %= 60 * 60 * 24;
    }

    if (secs >= 60 * 60) {
        hours = Math.floor(secs / (60 * 60));
        if (hours >= 10) {
            hourString = hours + ":";
        } else {
            hourString = "0" + hours + ":";
        }
        secs %= 60 * 60;
    } else {
        hourString = "00:";
    }

    if (secs >= 60) {
        mins = Math.floor(secs / 60);
        if (mins >= 10) {
            minString = mins + ":";
        } else {
            minString = "0" + mins + ":";
        }
        secs %= 60;
    } else {
        minString = "00:";
    }

    if (secs >= 10) {
        secString = secs.toString();
    } else {
        secString = "0" + secs;
    }

    return dayString + hourString + minString + secString;
}

export function toLocaleFixed(i: number, fractionDigits: number) {
    return i.toLocaleString(undefined, {
        minimumFractionDigits: fractionDigits,
        maximumFractionDigits: fractionDigits
    });
}

export function setNumericInput(id: string, i: number, fractionDigits: number) {
    // Firefox does not localize numbers with a fractional part correctly.
    // OTOH Webkit based browsers (correctly) expect setting the value to a non-localized number.
    // Unfortunately, setting the value to a localized number (i.e. with , instead of . for German)
    // does not raise an exception, instead only a warning on the console is shown.
    // So to make everyone happy, we use user agent detection.
    if (navigator.userAgent.indexOf("Gecko/") >= 0) {
        (<HTMLInputElement> document.getElementById(id)).value = toLocaleFixed(i, fractionDigits);
    } else {
        (<HTMLInputElement> document.getElementById(id)).value = i.toFixed(fractionDigits);
    }
}

export function toggle_password_fn(input_name: string) {
    return (ev: Event) => {
        let input = <HTMLInputElement>$(input_name)[0];
        let x = <HTMLInputElement>ev.target;

        if (x.checked)
            input.type = 'text';
        else
            input.type = 'password';
    }
}

export function clear_password_fn(input_name: string, to_be_cleared: string = __("util.to_be_cleared"), unchanged: string = __("util.unchanged")) {
    return (ev: Event) => {
        let x = <HTMLInputElement>ev.target;
        if (x.checked) {
            $(input_name).val('');
            $(input_name).attr('placeholder', to_be_cleared);
        } else {
            $(input_name).attr('placeholder', unchanged);
        }

        $(input_name).prop("disabled", x.checked);
    }
}

let wsReconnectTimeout: number = null;
let wsReconnectCallback: () => void = null;
let ws: WebSocket = null;

const RECONNECT_TIME = 12000;

export let eventTarget: API.APIEventTarget = new API.APIEventTarget();

export function setupEventSource(first: boolean, keep_as_first: boolean, continuation: (ws: WebSocket, eventTarget: API.APIEventTarget) => void) {
    if (!first) {
        add_alert("event_connection_lost", "alert-warning",  __("util.event_connection_lost_title"), __("util.event_connection_lost"))
    }
    console.log("Connecting to web socket");
    if (ws != null) {
        ws.close();
    }
    ws = new WebSocket((location.protocol == 'https:' ? 'wss://' : 'ws://') + location.host + '/ws');

    if (wsReconnectTimeout != null) {
        clearTimeout(wsReconnectTimeout);
    }
    wsReconnectCallback = () => setupEventSource(keep_as_first ? first : false, keep_as_first, continuation)
    wsReconnectTimeout = window.setTimeout(wsReconnectCallback, RECONNECT_TIME);

    ws.onmessage = (e: MessageEvent) => {
        if(!keep_as_first)
            remove_alert("event_connection_lost");

        if (wsReconnectTimeout != null) {
            window.clearTimeout(wsReconnectTimeout);
            wsReconnectTimeout = null;
        }
        wsReconnectTimeout = window.setTimeout(wsReconnectCallback, RECONNECT_TIME);

        let topics = [];
        for (let item of e.data.split("\n")) {
            if (item == "")
                continue;
            let obj = JSON.parse(item);
            if (!("topic" in obj) || !("payload" in obj)) {
                console.log("Received malformed event", obj);
                return;
            }

            topics.push(obj["topic"]);
            API.update(obj["topic"], obj["payload"]);
        }

        for (let topic of topics) {
            API.trigger(topic, eventTarget);
        }
    }

    continuation(ws, eventTarget);
}

export function pauseWebSockets() {
    ws.close();
    if (wsReconnectTimeout != null) {
        clearTimeout(wsReconnectTimeout);
    }
}

export function resumeWebSockets() {
    wsReconnectTimeout = window.setTimeout(wsReconnectCallback, RECONNECT_TIME);
}

export function postReboot(alert_title: string, alert_text: string) {
    ws.close();
    clearTimeout(wsReconnectTimeout);
    add_alert("reboot", "alert-success", alert_title, alert_text);
    // Wait 5 seconds before starting the reload/reconnect logic, to make sure the reboot has actually started yet.
    // Else it sometimes happens, that we reconnect _before_ the reboot starts.
    window.setTimeout(() => whenLoggedInElseReload(() =>
        setupEventSource(true, true, (ws, eventSource) => {
                // It is a bit of a hack to use version here, but
                // as opposed to keep-alive, version was already there in the first version.
                // so this will even work if downgrading to an version older than
                // 1.1.0
                console.log("setting up...");
                eventSource.addEventListener('info/version', function (e) {
                    console.log("reloading");
                    window.location.reload();
                }, false);})
    ), 5000);
}

let loginReconnectTimeout: number = null;

export function ifLoggedInElse(if_continuation: () => void, else_continuation: () => void) {
    download("/login_state", 3000)
        .catch(e => new Blob([e.message.startsWith("404") ? "Logged in" : ""]))
        .then(blob => blob.text())
        .then(text => text == "Logged in" ? if_continuation() : else_continuation());
}

export function ifLoggedInElseReload(continuation: () => void) {
    ifLoggedInElse(continuation, function() {
        window.location.reload();
    });
}

export function whenLoggedInElseReload(continuation: () => void) {
    if (loginReconnectTimeout != null) {
        clearTimeout(loginReconnectTimeout);
        loginReconnectTimeout = null;
    }
    if (wsReconnectTimeout != null) {
        clearTimeout(wsReconnectTimeout);
        wsReconnectTimeout = null;
    }
    loginReconnectTimeout = window.setTimeout(
        () => ifLoggedInElseReload(
            () => {clearTimeout(loginReconnectTimeout); continuation();}),
        RECONNECT_TIME);

    ifLoggedInElseReload(() => {clearTimeout(loginReconnectTimeout); continuation();});
}

// Password inputs use the empty string as the "unchanged" value.
// However the API expects a null value if the value should not be changed.
// If the input is disabled, the clear toggle was set.
// Return an empty string to remove the stored password in this case.
export function passwordUpdate(input_selector: string) {
    let input = $(input_selector);
    if (input.length == 0) {
        console.error(`Input with selector ${input_selector} not found.`);
    }
    if (input.attr("disabled"))
        return "";

    let value = input.val().toString();

    return value.length > 0 ? value : null;
}

function iso8601ButLocal(date: Date) {
    const offset = date.getTimezoneOffset() * 60 * 1000;
    const local =  date.getTime() - offset;
    const dateLocal = new Date(local);
    return dateLocal.toISOString().slice(0, -1);
}

export function downloadToFile(content: BlobPart, filename_prefix: string, extension: string, contentType: string) {
    const a = document.createElement('a');
    const file = new Blob([content], {type: contentType});
    let t = iso8601ButLocal(new Date()).replace(/:/gi, "-").replace(/\./gi, "-");
    let name = API.get('info/name').name;

    a.href= URL.createObjectURL(file);
    a.download = filename_prefix + "-" + name + "-" + t + "." + extension;
    a.click();

    URL.revokeObjectURL(a.href);
};

export function getShowRebootModalFn(changed_value_name: string) {
    return () => {
        $('#reboot_content_changed').html(changed_value_name);
        $('#reboot').modal('show');
    }
}

export function timestamp_min_to_date(timestamp_minutes: number, unsynced_string: string) {
    if (timestamp_minutes == 0) {
        return unsynced_string;
    }
    let date_fmt: any = { year: 'numeric', month: '2-digit', day: '2-digit'};
    let time_fmt: any = {hour: '2-digit', minute:'2-digit' };
    let fmt = Object.assign({}, date_fmt, time_fmt);

    let date = new Date(timestamp_minutes * 60000);
    let result = date.toLocaleString([], fmt);

    let date_result = date.toLocaleDateString([], date_fmt);
    let time_result = date.toLocaleTimeString([], time_fmt);

    // By default there is a comma between the date and time part of the string.
    // This comma (even if the whole date is marked as string for CSV) prevents office programs
    // to understand that this is a date.
    // Remove this (and only this) comma without assuming anything about the localized string.
    if (result == date_result + ", " + time_result) {
        return date_result + " " + time_result;
    }
    if (result == time_result + ", " + date_result) {
        return time_result + " " + date_result;
    }

    return result;
}

export function reset_static_ip_config_validation(ip_id: string, subnet_id: string, gateway_id: string) {
    $(`#${gateway_id}`).removeClass("is-invalid");
    $(`#${gateway_id} + .invalid-feedback`).html(__("util.gateway_invalid"));

    $(`#${subnet_id}`).removeClass("is-invalid");
    $(`#${subnet_id} + .invalid-feedback`).html(__("util.subnet_invalid"));
    return true;
}

export function validate_static_ip_config(ip_id: string, subnet_id: string, gateway_id: string, dhcp: boolean) {
    if (dhcp)
        return true;

    let ip = $(`#${ip_id}`).val().toString().split(".").map((x, i, _) => parseInt(x, 10) * (1 << (8 * (3 - i)))).reduce((a, b) => a+b);
    let subnet = $(`#${subnet_id}`).val().toString().split(".").map((x, i, _) => parseInt(x, 10) * (1 << (8 * (3 - i)))).reduce((a, b) => a+b);
    let gateway = $(`#${gateway_id}`).val().toString().split(".").map((x, i, _) => parseInt(x, 10) * (1 << (8 * (3 - i)))).reduce((a, b) => a+b);

    let result = true;
    if (gateway != 0 && (ip & subnet) != (gateway & subnet)) {
        $(`#${gateway_id}`).addClass("is-invalid");
        $(`#${gateway_id} + .invalid-feedback`).html(__("util.gateway_out_of_subnet"));
        result = false;
    }

    if ((ip & subnet) == (0x7F000001 & subnet)) {
        $(`#${subnet_id}`).addClass("is-invalid");
        $(`#${subnet_id} + .invalid-feedback`).html(__("util.subnet_captures_localhost"));
        result = false;
    }
    return result;
}

export function upload(data: Blob, url: string, progress: (i: number) => void = i => {}, contentType?: string, timeout_ms: number = 5000) {
    const xhr = new XMLHttpRequest();
    progress(0);

    let error_message: string = null;

    return new Promise<void>((resolve, reject) => {
        xhr.upload.addEventListener("abort", e => error_message = error_message ?? __("util.upload_abort"));
        // https://bugs.chromium.org/p/chromium/issues/detail?id=118096#c5
        // "The details of errors of XHRs and Fetch API are not exposed to JavaScript for security reasons."
        // Web development just sucks.
        xhr.upload.addEventListener("error", e => error_message = error_message ?? __("util.upload_error"));
        xhr.upload.addEventListener("timeout", e => error_message = error_message ?? __("util.upload_timeout"));

        xhr.upload.addEventListener("progress", (event) => {
            if (event.lengthComputable) {
                progress(event.loaded / event.total);
            }
        });

        xhr.addEventListener("abort", e => error_message = error_message ?? __("util.download_abort"));
        xhr.addEventListener("error", e => error_message = error_message ?? __("util.download_error"));
        xhr.addEventListener("timeout", e => error_message = error_message ?? __("util.download_timeout"));

        xhr.addEventListener("loadend", () => {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                progress(1);
                if (xhr.status === 200)
                    resolve();
            }
            reject(error_message ?? xhr);
        });

        xhr.open("POST", url, true);
        xhr.timeout = timeout_ms;
        if (contentType)
            xhr.setRequestHeader("Content-Type", contentType);
        xhr.send(data);
    });
}

export async function download(url: string, timeout_ms: number = 5000) {
    let abort = new AbortController();
    let timeout = setTimeout(() => abort.abort(), timeout_ms);

    let response = null;
    try {
        response = await fetch(url, {signal: abort.signal})
    } catch (e) {
        clearTimeout(timeout);
        throw new Error(e.name == "AbortError" ? __("util.download_timeout") : (__("util.download_error") + ": " + e.message));
    }

    if (!response.ok) {
        throw new Error(`${response.status}(${response.statusText}) ${await response.text()}`)
    }

    return await response.blob();
}

export async function put(url: string, payload: any, timeout_ms: number = 5000) {
    let abort = new AbortController();
    let timeout = setTimeout(() => abort.abort(), timeout_ms);

    let response = null;
    try {
        response = await fetch(url, {
            signal: abort.signal,
            method: "PUT",
            credentials: 'same-origin',
            headers: {"Content-Type": "application/json; charset=utf-8"},
            body: JSON.stringify(payload)})
    } catch (e) {
        clearTimeout(timeout);
        throw new Error(e.name == "AbortError" ? __("util.download_timeout") : (__("util.download_error") + ": " + e.message));
    }

    if (!response.ok) {
        throw new Error(`${response.status}(${response.statusText}) ${await response.text()}`)
    }

    return await response.blob();
}

export const async_modal_ref: RefObject<AsyncModal> = createRef();

export function range(stopOrStart: number, stop?: number) {
    if (stop === undefined) {
        stop = stopOrStart
        stopOrStart = 0
    }

    const len = (stop - stopOrStart)
    if (len <= 0)
        return [];

    return [...Array(len).keys()].map(i => i + stopOrStart);
}

export async function wait(t: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, t));
}

// From https://stackoverflow.com/questions/49580725/is-it-possible-to-restrict-typescript-object-to-contain-only-properties-defined
// First, define a type that, when passed a union of keys, creates an object which
// cannot have those properties. I couldn't find a way to use this type directly,
// but it can be used with the below type.
type Impossible<K extends keyof any> = {
    [P in K]: never;
};
// The secret sauce! Provide it the type that contains only the properties you want,
// and then a type that extends that type, based on what the caller provided
// using generics.
export type NoExtraProperties<T, U extends T = T> = U & Impossible<Exclude<keyof U, keyof T>>;

export function clamp(min: number | undefined, x: number, max: number | undefined) {
    let result = x;
    if (max !== undefined)
        result = Math.min(max, x);
    if (min !== undefined)
        result = Math.max(min, x);
    return result;
}
