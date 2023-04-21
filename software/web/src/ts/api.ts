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

import * as util from './util';
import $ from "./jq";

export {type ConfigMap as getType, type Modules};

export type EventMap = {
    [key in keyof ConfigMap]: MessageEvent<Readonly<ConfigMap[key]>>;
}

function update_cache_item(left: any, right: any) {
    for (var key in left) {
        if (!right.hasOwnProperty(key)) {
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

    for (var key in right)
        if (!left.hasOwnProperty(key)) {
            left[key] = right[key];
        }
}

export function update<T extends keyof ConfigMap>(topic: T, payload: ConfigMap[T]) {
    if (api_cache[topic] === null || api_cache[topic] === undefined)
        (api_cache[topic] as any) = payload;
    else
        update_cache_item(api_cache[topic], payload);
}

export function get<T extends keyof ConfigMap>(topic: T) {
    return api_cache[topic];
}

export function is_modified<T extends keyof ConfigMap>(topic: T): boolean {
    let modified = api_cache[(topic + "_modified") as ConfigModifiedKey] as ConfigModified;
    if (modified == null)
        return false;
    return modified.modified > 1;
}

export function is_dirty<T extends keyof ConfigMap>(topic: T): boolean {
    let modified = api_cache[(topic + "_modified") as ConfigModifiedKey] as ConfigModified;
    if (modified == null)
        return false;
    return (modified.modified & 1) == 1;
}

export function get_maybe<T extends string>(topic: T): (T extends keyof ConfigMap ? Readonly<ConfigMap[T]> : any) {
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

export function trigger<T extends keyof ConfigMap>(topic: T, event_source: APIEventTarget) {
    event_source.dispatchEvent(new MessageEvent<Readonly<ConfigMap[T]>>(topic, {'data': get(topic)}));
}

export function trigger_unchecked<T extends keyof ConfigMap>(topic: string, event_source: APIEventTarget) {
    event_source.dispatchEvent(new MessageEvent<Readonly<ConfigMap[T]>>(topic, {'data': get(topic as any)}));
}

export function save<T extends keyof ConfigMap>(topic: T, payload: ConfigMap[T], error_string: string, reboot_string?: string) {
    return call(<any>(topic + "_update"), payload, error_string, reboot_string);
}

export function save_maybe<T extends string>(topic: T, payload: (T extends keyof ConfigMap ? ConfigMap[T] : any), error_string: string, reboot_string?: string) {
    if (topic in api_cache)
        return call(<any>(topic + "_update"), payload, error_string, reboot_string);
    return Promise.resolve();
}

export function reset<T extends keyof ConfigMap>(topic: T, error_string: string, reboot_string?: string) {
    return call(<any>(topic + "_reset"), null, error_string, reboot_string);
}

export async function call<T extends keyof ConfigMap>(topic: T, payload: ConfigMap[T], error_string: string, reboot_string?: string, timeout_ms: number = 5000) {
    try {
        let blob = await util.put('/' + topic, payload, timeout_ms);
        if (reboot_string)
            util.getShowRebootModalFn(reboot_string)();
        return blob;
    } catch (e) {
        util.add_alert(topic.replace("/", "_") + '_failed', 'alert-danger', error_string, e);
        throw e;
    }
}

export function hasFeature(feature: string) {
    return get('info/features').indexOf(feature) >= 0;
}

export function hasModule(module: string) {
    return get('info/modules')?.hasOwnProperty(module) && (get('info/modules') as any)[module];
}
