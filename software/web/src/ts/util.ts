
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

import { createRef, RefObject } from "preact";
import * as API from "./api";
import { __, removeUnicodeHacks } from "./translation";
import { AsyncModal } from "./components/async_modal";
import { api_cache } from "./api_defs";
import { batch, signal, Signal } from "@preact/signals-core";
import { deepSignal, DeepSignal } from "deepsignal";

export function reboot() {
    API.call("reboot", null, "").then(() => postReboot(__("util.reboot_title"), __("util.reboot_text")));
}

// react-bootstrap's Variant adds | string as the last union variant m(
type StrictVariant = 'primary' | 'secondary' | 'success' | 'danger' | 'warning' | 'info' | 'dark' | 'light';

let alerts: DeepSignal<Array<{id: string, variant: StrictVariant, title: string, text: string}>> = deepSignal([]);

export function get_alerts() {
    return alerts;
}

export function add_alert(id: string, variant: StrictVariant, title: string, text: string) {
    let idx = alerts.findIndex((alert) => alert.id == id);
    let alert = {id: id, variant: variant, title: title, text: text};

    if (idx >= 0) {
        alerts[idx] = alert;
    }
    else {
        alerts.push(alert);
    }
}

export function remove_alert(id: string) {
    let idx = alerts.findIndex((alert) => alert.id == id);

    if (idx >= 0) {
        alerts.splice(idx, 1);
    }
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

let number_formats: {[id: number]: Intl.NumberFormat} = {};

export function toLocaleFixed(i: number, fractionDigits?: number) {
    if (fractionDigits === undefined) {
        fractionDigits = 0;
    }

    if (!number_formats[fractionDigits]) {
        number_formats[fractionDigits] = new Intl.NumberFormat(undefined, {minimumFractionDigits: fractionDigits, maximumFractionDigits: fractionDigits});
    }

    return number_formats[fractionDigits].format(i);
}

let wsReconnectTimeout: number = null;
let wsReconnectCallback: () => void = null;
let ws: WebSocket = null;

const RECONNECT_TIME = 12000;

export function addApiEventListener<T extends keyof API.EventMap>(type: T, listener: (this: API.APIEventTarget, ev: API.EventMap[T]) => any, options?: boolean | AddEventListenerOptions) {
    eventTarget.addEventListener(type, listener, options);
    if (api_cache[type])
    {
        API.trigger(type, eventTarget);
    }
}

export function addApiEventListener_unchecked(type: string, callback: EventListenerOrEventListenerObject, options?: boolean | AddEventListenerOptions): void {
    eventTarget.addEventListener_unchecked(type, callback, options);
    let api_cache_any = api_cache as any;
    if (api_cache_any[type]) {
        API.trigger_unchecked(type, eventTarget);
    }
}

export let eventTarget: API.APIEventTarget = new API.APIEventTarget();

let active_sub_page: Signal<string> = signal("");

export function get_active_sub_page() {
    return active_sub_page.value;
}

export function set_active_sub_page(sub_page: string) {
    active_sub_page.value = sub_page;
}

let allow_render: Signal<boolean> = signal(false);

export function render_allowed() {
    return allow_render.value;
}

let caps_active: Signal<boolean> = signal(false);

export function capsLockActive() {
    return caps_active.value;
}

function checkCapsLock(e: MouseEvent | KeyboardEvent) {
    let active = e.getModifierState("CapsLock");
    if (caps_active.value && e instanceof KeyboardEvent && e.type == "keyup" && e.key == "CapsLock")
        active = false;
    caps_active.value = active;
}

export function initCapsLockCheck() {
    document.addEventListener("keyup", checkCapsLock);
    document.addEventListener("click", checkCapsLock);
}

export let remoteAccessMode = window.top !== window.self;
const path = location.origin;
export let connection_id = "";
let iframe_timeout: number = null;
let iframe_timeout_ms = 100;

window.addEventListener("message", (e) => {
    if (iframe_timeout != null) {
        window.clearTimeout(iframe_timeout);
        iframe_timeout = null;
    }
    if (typeof e.data === "string") {
        if (iFrameSocketCb) {
            iFrameSocketCb(e);
        }
    } else {
        connection_id = e.data.connection_id;
    }
});

if (remoteAccessMode) {
    window.parent.postMessage("webinterface_loaded");
    iframe_timeout = window.setTimeout(() => {
        remoteAccessMode = false;
    }, iframe_timeout_ms);
}

export function closeRemoteConnection() {
    window.parent.postMessage("close");
}

let iFrameSocketCb: (data: any) => void = null;

function iFrameSocketInit(first: boolean, keep_as_first: boolean, continuation: (ws: WebSocket | undefined, eventTarget: API.APIEventTarget) => void) {
    if (!first) {
        add_alert("event_connection_lost", "warning",  __("util.event_connection_lost_title"), __("util.event_connection_lost"))
    }
    if (wsReconnectTimeout != null) {
        clearTimeout(wsReconnectTimeout);
    }
    wsReconnectCallback = () => setupEventSource(keep_as_first ? first : false, keep_as_first, continuation, true)
    wsReconnectTimeout = window.setTimeout(wsReconnectCallback, RECONNECT_TIME);

    iFrameSocketCb = wsOnMessageCallback;
    continuation(undefined, eventTarget);
}

// Copy of keep_as_first parameter of setupEventSource
let k_a_f: boolean;
const wsOnMessageCallback = (e: MessageEvent) => {
    if(!k_a_f)
        remove_alert("event_connection_lost");

    if (wsReconnectTimeout != null) {
        window.clearTimeout(wsReconnectTimeout);
        wsReconnectTimeout = null;
    }
    wsReconnectTimeout = window.setTimeout(wsReconnectCallback, RECONNECT_TIME);

    let topics: any[] = [];

    let end_marker_found = (e.data as string).includes("\n\n");
    let messages = (e.data as string).trim();

    batch(() => {
        for (let item of messages.split("\n")) {
            let obj = JSON.parse(item);
            if (!("topic" in obj) || !("payload" in obj)) {
                console.log("Received malformed event", obj);
                return;
            }

            topics.push(obj["topic"]);
            API.update(obj["topic"], obj["payload"]);
        }

        if (allow_render.peek()) {
            for (let topic of topics) {
                API.trigger(topic, eventTarget);
            }
        }

        if (end_marker_found) {
            API.trigger_all(eventTarget);
            allow_render.value = true;
        }
    });
};

export function setupEventSource(first: boolean, keep_as_first: boolean, continuation: (ws: WebSocket, eventTarget: API.APIEventTarget) => void, isIframe?: boolean) {
    k_a_f = keep_as_first;
    if (remoteAccessMode) {
        if (iframe_timeout != null) {
            // We are currently checking whether the iframe parent is the remote access page.
            // Retry after the check is done.
            window.setTimeout(() => setupEventSource(first, keep_as_first, continuation, isIframe), iframe_timeout_ms);
        } else {
            pauseWebSockets();
            iFrameSocketInit(first, keep_as_first, continuation);
            window.parent.postMessage("initIFrame", "*");
            return;
        }
    }

    if (!first) {
        add_alert("event_connection_lost", "warning",  __("util.event_connection_lost_title"), __("util.event_connection_lost"))
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

    ws.onmessage = wsOnMessageCallback;

    continuation(ws, eventTarget);
}

export function pauseWebSockets() {
    if (ws !== null) {
        ws.close();
    }
    if (wsReconnectTimeout != null) {
        clearTimeout(wsReconnectTimeout);
    }
}

export function resumeWebSockets() {
    wsReconnectTimeout = window.setTimeout(wsReconnectCallback, RECONNECT_TIME);
    wsReconnectCallback();
}

export function postReboot(alert_title: string, alert_text: string) {
    if (ws !== null) {
        ws.close();
    }
    clearTimeout(wsReconnectTimeout);
    add_alert("reboot", "success", alert_title, alert_text);
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
    if (!remoteAccessMode || connection_id.length > 0) {
        download("/login_state", 10000)
            .catch(e => new Blob(["Logged in"]))
            .then(blob => blob.text())
            .then(text => text == "Logged in" ? if_continuation() : else_continuation());
    } else {
        setTimeout(() => ifLoggedInElse(if_continuation, else_continuation));
    }
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

export function iso8601ButLocal(date: Date) {
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
    return ip.split(".").map((x, i, _) => parseInt(x, 10) * (1 << (8 * (3 - i)))).reduce((a, b) => a + b) >>> 0;
}

export function unparseIP(ip: number) {
    return ((ip >>> 24) & 0xFF).toString() + "." +
           ((ip >>> 16) & 0xFF).toString() + "." +
           ((ip >>> 8 ) & 0xFF).toString() + "." +
           ((ip >>> 0 ) & 0xFF).toString();
}


export function downloadToFile(content: BlobPart, fileType: string, extension: string, contentType: string, timestamp?: Date) {
    if (timestamp === undefined) {
        timestamp = new Date();
    }

    const a = document.createElement('a');
    const file = new Blob([content], {type: contentType});
    let timestamp_str = iso8601ButLocal(timestamp).replace(/:/gi, "-").replace(/\./gi, "-");
    let name = API.get_unchecked('info/name')?.name ?? "unknown_uid";

    let filename = name + "-" + fileType + "-" + timestamp_str + "." + extension;
    filename = removeUnicodeHacks(filename);

    a.href= URL.createObjectURL(file);
    a.download = filename;
    a.click();

    URL.revokeObjectURL(a.href);
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

    if (remoteAccessMode) {
        url = path + (url.startsWith("/") ? "" : "/") + url;
    }

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
                xhr.response
                progress(1);
                if (xhr.status === 200)
                    resolve();
            }
            reject(error_message ?? xhr);
        });

        xhr.open("POST", url, true);
        xhr.setRequestHeader("X-Connection-Id", connection_id);
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
        if (remoteAccessMode) {
            url = path + (url.startsWith("/") ? "" : "/") + url;
        }
        response = await fetch(url, {signal: abort.signal, headers: {"X-Connection-Id": connection_id}});
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
        if (remoteAccessMode) {
            url = path + (url.startsWith("/") ? "" : "/") + url;
        }
        response = await fetch(url, {
            signal: abort.signal,
            method: "PUT",
            credentials: 'same-origin',
            headers: {
                "Content-Type": "application/json; charset=utf-8",
                "X-Connection-Id": connection_id,
            },
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

export function joinNonEmpty(sep: string, lst: string[]) {
    return lst.filter(x => x) // remove empty slots
              .filter(x => x.length > 0)
              .join(sep);
}

export function get_updated_union<T extends object>(union: [number, T], update: Partial<T>): [number, T] {
    return [union[0], {...union[1], ...update}];
}

export function toIsoString(date: Date) {
    return date.getFullYear() +
        '-' + leftPad(date.getMonth() + 1, 0, 2) +
        '-' + leftPad(date.getDate(), 0, 2) +
        'T' + leftPad(date.getHours(), 0, 2) +
        ':' + leftPad(date.getMinutes(), 0, 2) +
        ':' + leftPad(date.getSeconds(), 0, 2);
}

export function rgbToHex(r: number, g: number, b: number, a?: number) {
    return '#' + leftPad(r.toString(16), '0', 2)
               + leftPad(g.toString(16), '0', 2)
               + leftPad(b.toString(16), '0', 2)
               + (a !== undefined ? leftPad(a.toString(16), '0', 2) : "");
}

export function hexToRgb(hex: string): {r: number, g: number, b: number, a?: number} {
    if (hex.startsWith("#"))
        hex = hex.slice(1);

    if (hex.length == 3) {
        return {
            r: parseInt(hex[0], 16),
            g: parseInt(hex[1], 16),
            b: parseInt(hex[2], 16)
        };
    }

    if (hex.length == 4) {
        return {
            r: parseInt(hex[0], 16),
            g: parseInt(hex[1], 16),
            b: parseInt(hex[2], 16),
            a: parseInt(hex[3], 16)
        };
    }

    if (hex.length == 6) {
        return {
            r: parseInt(hex.slice(0, 2), 16),
            g: parseInt(hex.slice(2, 4), 16),
            b: parseInt(hex.slice(4, 6), 16)
        };
    }

    if (hex.length == 8) {
        return {
            r: parseInt(hex.slice(0, 2), 16),
            g: parseInt(hex.slice(2, 4), 16),
            b: parseInt(hex.slice(4, 6), 16),
            a: parseInt(hex.slice(6, 8), 16)
        };
    }
}

// From https://axonflux.com/handy-rgb-to-hsl-and-rgb-to-hsv-color-model-c
/**
 * Converts an RGB color value to HSV. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes r, g, and b are contained in the set [0, 255] and
 * returns h, s, and v in the set [0, 1].
 *
 * @param   Number  r       The red color value
 * @param   Number  g       The green color value
 * @param   Number  b       The blue color value
 * @return  Array           The HSV representation
 */
export function rgbToHsv(r: number, g: number, b: number): [number, number, number]{
    r = r/255, g = g/255, b = b/255;
    var max = Math.max(r, g, b), min = Math.min(r, g, b);
    var h, s, v = max;

    var d = max - min;
    s = max == 0 ? 0 : d / max;

    if(max == min){
        h = 0; // achromatic
    }else{
        switch(max){
            case r: h = (g - b) / d + (g < b ? 6 : 0); break;
            case g: h = (b - r) / d + 2; break;
            case b: h = (r - g) / d + 4; break;
        }
        h /= 6;
    }

    return [h, s, v];
}

/**
 * Converts an HSV color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes h, s, and v are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param   Number  h       The hue
 * @param   Number  s       The saturation
 * @param   Number  v       The value
 * @return  Array           The RGB representation
 */
export function hsvToRgb(h: number, s: number, v: number): [number, number, number]{
    var r, g, b;

    var i = Math.floor(h * 6);
    var f = h * 6 - i;
    var p = v * (1 - s);
    var q = v * (1 - f * s);
    var t = v * (1 - (1 - f) * s);

    switch(i % 6){
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
}

export function blobToBase64(blob: Blob): Promise<string> {
    return new Promise((resolve, _) => {
        const reader = new FileReader();
        reader.onloadend = () => resolve(reader.result as string);
        reader.readAsDataURL(blob);
    });
}
