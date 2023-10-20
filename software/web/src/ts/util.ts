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
import { api_cache } from "./api_defs";
import { batch, signal, Signal } from "@preact/signals-core";
import { useState } from "preact/hooks";

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

export function format_timespan_ms(ms: number, opts?: {replace_zero_with?: string, replace_u32max_with?: string}) {
    if (opts !== undefined && opts.replace_zero_with !== undefined && ms == 0)
        return opts.replace_zero_with;

    if (opts !== undefined && opts.replace_u32max_with !== undefined && ms == 0xFFFFFFFF)
        return opts.replace_u32max_with;

    return format_timespan(ms / 1000);
}

export function format_timespan(secs: number) {
    secs = Math.floor(secs);

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

export function toLocaleFixed(i: number, fractionDigits?: number) {
    if (fractionDigits === undefined) {
        fractionDigits = 0;
    }

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
        (document.getElementById(id) as HTMLInputElement).value = toLocaleFixed(i, fractionDigits);
    } else {
        (document.getElementById(id) as HTMLInputElement).value = i.toFixed(fractionDigits);
    }
}

export function toggle_password_fn(input_name: string) {
    return (ev: Event) => {
        let input = $(input_name)[0] as HTMLInputElement;
        let x = ev.target as HTMLInputElement;

        if (x.checked)
            input.type = 'text';
        else
            input.type = 'password';
    }
}

export function clear_password_fn(input_name: string, to_be_cleared: string = __("util.to_be_cleared"), unchanged: string = __("util.unchanged")) {
    return (ev: Event) => {
        let x = ev.target as HTMLInputElement;
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

export function addApiEventListener<T extends keyof API.EventMap>(type: T, listener: (this: API.APIEventTarget, ev: API.EventMap[T]) => any, options?: boolean | AddEventListenerOptions) {
    eventTarget.addEventListener(type, listener);
    if (api_cache[type])
    {
        API.trigger(type, eventTarget);
    }
}

export function addApiEventListener_unchecked(type: string, callback: EventListenerOrEventListenerObject, options?: boolean | AddEventListenerOptions): void {
    eventTarget.addEventListener_unchecked(type, callback, options);
    let api_cache_any = api_cache as any;
    if (api_cache_any[type])
    {
        API.trigger_unchecked(type, eventTarget);
    }
}

export let eventTarget: API.APIEventTarget = new API.APIEventTarget();

let allow_render: Signal<boolean> = signal(false);

export function render_allowed() {
    return allow_render.value;
}

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

        let topics: any[] = [];
        batch(() => {
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

            allow_render.value = true;
        });
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
    wsReconnectCallback();
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
                eventSource.addEventListener_unchecked('info/version', function (e) {
                    window.location.reload();
                }, false);})
    ), 5000);
}

let loginReconnectTimeout: number = null;

export function ifLoggedInElse(if_continuation: () => void, else_continuation: () => void) {
    download("/login_state", 10000)
        .catch(e => new Blob(["Logged in"]))
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

const INDEX_BY_CODE_POINT = new Map([
	[338, 12],
	[339, 28],
	[352, 10],
	[353, 26],
	[376, 31],
	[381, 14],
	[382, 30],
	[402, 3],
	[710, 8],
	[732, 24],
	[8211, 22],
	[8212, 23],
	[8216, 17],
	[8217, 18],
	[8218, 2],
	[8220, 19],
	[8221, 20],
	[8222, 4],
	[8224, 6],
	[8225, 7],
	[8226, 21],
	[8230, 5],
	[8240, 9],
	[8249, 11],
	[8250, 27],
	[8364, 0],
	[8482, 25]
]);

// Based on https://www.npmjs.com/package/windows-1252
export const win1252Encode = (input: string) => {
    // Undo the insertion of unicode soft hyphen characters.
    // Those can't be encoded in cp1252
    input = input.replace(/\u00AD/g, "");
	const length = input.length;
	const result = new Uint8Array(length);
	for (let index = 0; index < length; index++) {
		const codePoint = input.charCodeAt(index);
		// “If `code point` is an ASCII code point, return a byte whose
		// value is `code point`.”
		if ((0x00 <= codePoint && codePoint <= 0x7F) || (0xA0 <= codePoint && codePoint <= 0xFF)) {
			result[index] = codePoint;
			continue;
		}
		// “Let `pointer` be the index pointer for `code point` in index
		// single-byte.”
		if (INDEX_BY_CODE_POINT.has(codePoint)) {
			const pointer = INDEX_BY_CODE_POINT.get(codePoint);
			// “Return a byte whose value is `pointer + 0x80`.”
			result[index] = pointer + 0x80;
		} else {
			// “If `pointer` is `null`, replace with ¶
			result[index] = 0xB6;
		}
	}
	return result;
}

export function parseIP(ip: string) {
    // >>> 0 to force unsigned 32 bit integers
    return ip.split(".").map((x, i, _) => parseInt(x, 10) * (1 << (8 * (3 - i)))).reduce((a, b) => a+b) >>> 0;
}

export function unparseIP(ip: number) {
    return ((ip >>> 24) & 0xFF).toString() + "." +
           ((ip >>> 16) & 0xFF).toString() + "." +
           ((ip >>> 8 ) & 0xFF).toString() + "." +
           ((ip >>> 0 ) & 0xFF).toString();
}


export function downloadToFile(content: BlobPart, filename_prefix: string, extension: string, contentType: string) {
    const a = document.createElement('a');
    const file = new Blob([content], {type: contentType});
    let t = iso8601ButLocal(new Date()).replace(/:/gi, "-").replace(/\./gi, "-");
    let name = API.get_unchecked('info/name')?.name ?? "unknown_uid";

    a.href= URL.createObjectURL(file);
    a.download = filename_prefix + "-" + name + "-" + t + "." + extension;
    a.click();

    URL.revokeObjectURL(a.href);
}

export function getShowRebootModalFn(changed_value_name: string) {
    return () => {
        $('#reboot_content_changed').html(changed_value_name);
        $('#reboot').modal('show');
    }
}

function timestamp_to_date(timestamp: number, time_fmt: any) {
    let date_fmt: any = {year: 'numeric', month: '2-digit', day: '2-digit'};
    let fmt = Object.assign({}, date_fmt, time_fmt);

    let date = new Date(timestamp);
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

export function timestamp_min_to_date(timestamp_minutes: number, unsynced_string?: string) {
    if (unsynced_string !== undefined && timestamp_minutes == 0) {
        return unsynced_string;
    }

    return timestamp_to_date(timestamp_minutes * 60000, {hour: '2-digit', minute: '2-digit'});
}

export function timestamp_sec_to_date(timestamp_seconds: number, unsynced_string?: string) {
    if (unsynced_string !== undefined && timestamp_seconds == 0) {
        return unsynced_string;
    }

    return timestamp_to_date(timestamp_seconds * 1000, {hour: '2-digit', minute: '2-digit', second: '2-digit'});
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

export function isInteger(x: number) {
    return !isNaN(x) && (x === (x | 0));
}

export function range(stopOrStart: number, stop?: number) {
    if (stop === undefined) {
        stop = stopOrStart;
        stopOrStart = 0;
    }

    if (!isInteger(stopOrStart))
        throw "util.range: stopOrStart was not an integer";

    if (!isInteger(stop))
        throw "util.range: stop was not an integer";

    const len = stop - stopOrStart;
    if (len <= 0)
        return [];

    return [...Array(len).keys()].map(i => i + stopOrStart);
}

export async function wait(t: number) {
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
        result = Math.min(max, result);
    if (min !== undefined)
        result = Math.max(min, result);
    return result;
}

export function leftPad(s: string | number, c: string | number, len: number) {
    s = s.toString();
    c = c.toString();
    while (s.length < len) {
        s = c + s;
    }
    return s.slice(-len);
}

export function hasValue(a: any): boolean
{
    return a !== null && a !== undefined;
}

export function compareArrays(a: Array<any>, b: Array<any>): boolean
{
    return a.length === b.length && a.every((element, index) => element === b[index]);
}

// https://stackoverflow.com/a/1535650
export let nextId = (function() {
    var id = 0;
    return function() {return "ID-" + (++id).toString();};
})();

// Preact's useId does not work with multiple roots:
// https://github.com/preactjs/preact/issues/3781
// Once the port to preact is complete,
// we can switch back to Preact's useId.
export function useId() {
    return useState(nextId())[0];
}
