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

import {ConfigMap, api_cache, Modules, ConfigModified, ConfigModifiedKey} from './api_defs';
import * as util from "./util";
import { __ } from "./translation";
import { RevertDeepSignal } from 'deepsignal';

export { type ConfigMap as getType, type Modules };

export type EventMap = {
    [key in keyof ConfigMap]: MessageEvent<Readonly<ConfigMap[key]>>;
};

function update_cache_item(left: any, right: any) {
    if (Array.isArray(left)) {
        let right_length = Object.keys(right).length;
        if (left.length > right_length)
            left.splice(right_length, left.length - right_length)
    }

    for (let key in left) {
        if (!Array.isArray(left) && !(key in right)) {
            delete left[key];
            continue;
        }

        if (left[key] == null || right[key] == null || typeof(left[key]) !== typeof(right[key])) {
            left[key] = right[key];
            continue;
        }

        if (typeof(left[key]) === "object") {
            update_cache_item(left[key], right[key]);
            continue;
        }

        left[key] = right[key];
    }

    for (let key in right) {
        if (!(key in left)) {
            left[key] = right[key];
        }
    }
}

function is_primitive(x: any) {
    return x === null || x === undefined || typeof(x) !== "object";
}

export function update<T extends keyof ConfigMap>(topic: T, payload: ConfigMap[T]) {
    if (is_primitive(api_cache[topic]) || is_primitive(payload))
        (api_cache[topic] as any) = payload;
    else
        update_cache_item(api_cache[topic], payload);
}

export function get<T extends keyof ConfigMap>(topic: T) : Readonly<ConfigMap[T]> {
    // This should be unnecessary, but putting a tuple in a DeepSignal seems to drop
    // the tuple's type information. Typescript then thinks the tuple is an array.
    return api_cache[topic] as any;
}

export function is_modified<T extends keyof ConfigMap>(topic: T): boolean {
    let modified = api_cache[(topic + "_modified") as ConfigModifiedKey] as ConfigModified;
    if (modified == null)
        return false;
    return modified.modified > 1;
}

export function is_modified_unchecked(topic: string): boolean {
    let modified = api_cache[(topic + "_modified") as ConfigModifiedKey] as ConfigModified;
    if (modified === undefined || modified == null)
        return false;
    return modified.modified > 1;
}

export function is_dirty<T extends keyof ConfigMap>(topic: T): boolean {
    let modified = api_cache[(topic + "_modified") as ConfigModifiedKey] as ConfigModified;
    if (modified == null)
        return false;
    return (modified.modified & 1) == 1;
}

export function get_unchecked<T extends string>(topic: T): (T extends keyof ConfigMap ? Readonly<ConfigMap[T]> : any) {
    if (topic in api_cache)
        return api_cache[topic as keyof ConfigMap] as any;
    return null as any;
}

// Based on https://43081j.com/2020/11/typed-events-in-typescript
// and https://stackoverflow.com/questions/51343322/extending-eventtarget-in-typescript-angular-2
export class APIEventTarget implements EventTarget {
    private delegate = document.createDocumentFragment();

    public addEventListener<T extends keyof EventMap>(type: T, listener: (this: APIEventTarget, ev: EventMap[T]) => any, options?: boolean | AddEventListenerOptions) : void;

    public addEventListener(...args: any): void {
        this.delegate.addEventListener.apply(this.delegate, args);
    }

    // Hack to circumvent typed implementation above, to allow adding an event listener for a module that is not compiled in.
    public addEventListener_unchecked(type: string, callback: EventListenerOrEventListenerObject, options?: boolean | AddEventListenerOptions): void;
    public addEventListener_unchecked(...args: any) : void {
        this.delegate.addEventListener.apply(this.delegate, args);
    }

    public dispatchEvent(...args: any): boolean {
        return this.delegate.dispatchEvent.apply(this.delegate, args);
    }

    public removeEventListener(...args: any): void {
        return this.delegate.removeEventListener.apply(this.delegate, args);
    }
}

export function trigger_all(event_source: APIEventTarget) {
    for (let x of Object.keys(api_cache as RevertDeepSignal<typeof api_cache>)) {
        let key = x as keyof ConfigMap;
        if (get(key) == undefined)
            continue;

        trigger(key, event_source);
    }
}

export function trigger<T extends keyof ConfigMap>(topic: T, event_source: APIEventTarget) {
    event_source.dispatchEvent(new MessageEvent<Readonly<ConfigMap[T]>>(topic, {'data': get(topic)}));
}

export function trigger_unchecked<T extends keyof ConfigMap>(topic: string, event_source: APIEventTarget) {
    event_source.dispatchEvent(new MessageEvent<Readonly<ConfigMap[T]>>(topic, {'data': get(topic as any)}));
}

export function save<T extends keyof ConfigMap>(topic: T, payload: ConfigMap[T], error_string?: string, reboot_string?: string) {
    let extracted = extract(topic, payload);
    return call((topic + "_update") as any, extracted, error_string, reboot_string);
}

export function save_unchecked<T extends string>(topic: T, payload: (T extends keyof ConfigMap ? ConfigMap[T] : any), error_string?: string, reboot_string?: string) {
    if (topic in api_cache)
        return call((topic + "_update") as any, payload, error_string, reboot_string);
    return Promise.resolve();
}

export function reset<T extends keyof ConfigMap>(topic: T, error_string?: string, reboot_string?: string) {
    return call((topic + "_reset") as any, null, error_string, reboot_string);
}


export function reset_unchecked<T extends string>(topic: T, error_string?: string, reboot_string?: string) {
    if (topic in api_cache)
        return call((topic + "_reset") as any, null, error_string, reboot_string);
    return Promise.resolve();
}

export async function call<T extends keyof ConfigMap>(topic: T, payload: ConfigMap[T], error_string?: string, reboot_string?: string, timeout_ms: number = 5000) {
    return call_unchecked(topic, payload, error_string, reboot_string, timeout_ms);
}

export async function call_unchecked(topic: string, payload: any, error_string?: string, reboot_string?: string, timeout_ms: number = 5000) {
    try {
        let blob = await util.put('/' + topic, payload, timeout_ms);
        if (reboot_string) {
            const modal = util.async_modal_ref.current;

            if(!await modal.show({
                    title: __("main.reboot_title"),
                    body: __("main.reboot_content")(reboot_string),
                    no_text: __("main.abort"),
                    yes_text: __("main.reboot"),
                    no_variant: "secondary",
                    yes_variant: "danger"
                }))
                return;

            util.reboot();
        }
        return blob;
    } catch (e) {
        if (error_string) {
            let text = e instanceof Error ? e.message : e;
            util.add_alert(topic.replace("/", "_") + '_failed', 'danger', error_string, text);
        }
        throw e;
    }
}

export type feature =
    "evse" |
    "energy_manager" |
    "cp_disconnect" |
    "button_configuration" |
    "ethernet" |
    "meters" |
    "nfc" |
    "phase_switch" |
    "rtc"|
    "meter" |
    "meter_phases" |
    "meter_all_values" |
    "rgb_led" |
    "firmware_update";

export function hasFeature(feature: feature) {
    return hasFeature_unchecked(feature);
}

export function hasFeature_unchecked(feature: string) {
    let features = get('info/features');
    if (features === null)
        return false;

    return features.indexOf(feature) >= 0;
}

export function hasModule(module: string) {
    let modules = get('info/modules');
    if (modules === null)
        return false;
    return module in modules && modules[module as keyof typeof modules];
}

//based on https://stackoverflow.com/a/50895613
export function extract<T extends keyof ConfigMap, U extends ConfigMap[T]>(topic: T, value: Readonly<U>){
    let stencil = get(topic)
    let result = {} as ConfigMap[T];
    for (const property of Object.keys(stencil) as Array<keyof ConfigMap[T]>) {
        result[property] = value[property];
    }
    return result;
}
