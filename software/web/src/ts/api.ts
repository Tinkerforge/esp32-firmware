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

import {ConfigMap, api_cache, Modules, ConfigModified} from './api_defs';

import * as util from './util';
import $ from "./jq";

export {type ConfigMap as getType, type Modules};

export type EventMap = {
    [key in keyof ConfigMap]: MessageEvent<Readonly<ConfigMap[key]>>;
}

export function update<T extends keyof ConfigMap>(topic: T, payload: ConfigMap[T]) {
    api_cache[topic] = payload;
}

export function get<T extends keyof ConfigMap>(topic: T): Readonly<ConfigMap[T]> {
    return api_cache[topic];
}

export function is_modified<T extends keyof ConfigMap>(topic: T): boolean {
    let modified = api_cache[(topic + "_modified") as T] as ConfigModified;
    if (modified == null)
        return false;
    return modified.modified > 1;
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

export function reset<T extends keyof ConfigMap>(topic: T, error_string: string, reboot_string: string) {
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

export function default_updater<T extends keyof ConfigMap>(topic: T, exclude?: Array<keyof ConfigMap[T]>, has_save_button=true) {
    let prefix = topic.replace('/', '_');
    let config = get(topic);
    let form = $(`#${prefix}_form`);
    let save_btn = $(`#${prefix}_save_button`);

    if (has_save_button && save_btn.length == 0) {
        console.error(`save btn not found #${prefix}_save_button`);
    }

    if (has_save_button && !save_btn.prop("disabled")) {
        return config;
    }

    if (form.length != 0) {
        form.removeClass('was-validated');
    }

    for (let key in config) {
        if (exclude && exclude.indexOf(key) >= 0)
            continue;

        let value = config[key];
        let id = `${prefix}_${key}`;
        let elem = $(`#${id}`);
        if (elem.length == 0) {
            console.error(`Can't update ${id}. Element not found`);
            continue;
        }

        if (elem.hasClass("btn-group")) {
            if (typeof value == "boolean")
                util.update_button_group(id, value ? 1 : 0);
            else if (typeof value == "number")
                util.update_button_group(id, value);
            else
                console.error(`Can't update button group ${id} from ${topic}[${key}] = ${value} (of type ${typeof value}): value type is not number or boolean`);
            continue;
        }

        if (typeof value == "boolean") {
            elem.prop("checked", value).trigger("change");
            continue;
        }

        if (typeof value == "string" || typeof value == "number") {
            if (elem.is("input") || elem.is("select"))
                elem.val(value).trigger("change");
            else if (elem.is("span"))
                elem.text(value).trigger("change");
            else
                console.error(`Can't update ${id} from ${topic}[${key}] = ${value} (of type ${typeof value}): ${id} is not an input, select or span`);
        }
    }

    return config;
}

// We could construct the default error_string and reboot_string placeholders.
// However then check_translation_completeness would not find them.
// Make error_string optional but print a run-time error to get the
// preferred argument order.
export function default_saver<T extends keyof ConfigMap>(topic: T, overrides?: Partial<ConfigMap[T]>, error_string?: string, reboot_string?: string) {
    let prefix = topic.replace('/', '_');
    if (!error_string)
        console.error(`default_saver for ${topic}: Missing error string.`);
    // We can't iterate over the keys of ConfigMap[T],
    // as the type information is only available at runtime.
    // Use the current config to have something to iterate over.
    let current_config = get(topic);
    let result: Partial<ConfigMap[T]> = {};
    for (let key in current_config) {
        if (overrides && key in overrides) {
            result[key] = overrides[key];
            continue;
        }

        let id = `${prefix}_${key}`;
        let elem = $(`#${id}`);
        let old_value = current_config[key];

        if (elem.length == 0) {
            console.error(`Can't save ${id}. Element not found`);
            continue;
        }

        if (typeof old_value == "boolean") {
            result[key] = elem.prop("checked");
            continue;
        }

        if (typeof old_value == "string") {
            result[key] = <any>elem.val();
            continue;
        }
        if (typeof old_value == "number") {
            result[key] = <any>parseInt(elem.val().toString(), 10);
            continue;
        }
    }

    return save(topic, <ConfigMap[T]>result, error_string, reboot_string)
            .then(() => $(`#${prefix}_save_button`).prop("disabled", true));
}

// Take the overrides as a function that returns the dictionary,
// so that we can control when the entries are evaluated.
// This makes calling this a bit uglier, because we have to pass the overrides
// like this: register_config_form(..., {overrides: () => ({a: 1, b:2}), ...})
export function register_config_form<T extends keyof ConfigMap>(topic: T, p: {
        // Replacements to be made before running the default HTML form validation
        overrides?: () => Partial<ConfigMap[T]>,
        // Custom validation to be checked before running the default HTML form validation.
        // Use this to reset validation changes (i.e. is-invalid classes, changed invalid-feedback texts)
        // done in post_validation.
        pre_validation?: () => boolean,
        // Custom validation to be checked after running the default HTML form validation.
        post_validation?: () => boolean,
        // Translated string to show in error message if calling API fails.
        error_string?: string,
        // Translated string to show in modal if calling API succeeds. No modal is shown if this is left undefined.
        reboot_string?: string
    }) {
    let prefix = topic.replace('/', '_');
    let form = $(`#${prefix}_form`);
    let save_btn = $(`#${prefix}_save_button`);

    if (form.length == 0) {
        console.error(`Cant register config form ${topic}. Form with id ${prefix}_form not found.`);
    }

    if (save_btn.length == 0) {
        console.error(`Cant register config form ${topic}. Form save button with id ${prefix}_save_button not found.`);
    }

    form.on('submit', function (this: HTMLFormElement, event: Event) {
        this.classList.remove('was-validated');

        event.preventDefault();
        event.stopPropagation();

        if (p.pre_validation && !p.pre_validation())
            return;

        if (this.checkValidity() === false) {
            this.classList.add('was-validated');
            return;
        }

        if (p.post_validation && !p.post_validation())
            return;

        default_saver(topic, p.overrides ? p.overrides() : undefined, p.error_string, p.reboot_string);
    });

    form.on('input', () => save_btn.prop("disabled", false));
}
