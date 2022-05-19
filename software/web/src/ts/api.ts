import {ConfigMap, api_cache, Modules} from './api_defs';

import * as util from './util';
import $ from "./jq";

export {ConfigMap as getType, Modules};

type EventMap = {
    [key in keyof ConfigMap]: MessageEvent<Readonly<ConfigMap[key]>>;
}

export function update<T extends keyof ConfigMap>(topic: T, payload: ConfigMap[T]) {
    api_cache[topic] = payload;
}

export function get<T extends keyof ConfigMap>(topic: T): Readonly<ConfigMap[T]> {
    return api_cache[topic];
}

// Based on https://43081j.com/2020/11/typed-events-in-typescript
// and https://stackoverflow.com/questions/51343322/extending-eventtarget-in-typescript-angular-2
export class APIEventTarget implements EventTarget {
    private delegate = document.createDocumentFragment();

    public addEventListener<T extends keyof EventMap>(type: T, listener: (this: APIEventTarget, ev: EventMap[T]) => any, options?: boolean | AddEventListenerOptions) : void;

    public addEventListener(...args: any): void {
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

export function save<T extends keyof ConfigMap>(topic: T, payload: ConfigMap[T], error_string: string, reboot_string?: string) {
    return call(<any>(topic + "_update"), payload, error_string, reboot_string);
}

export function call<T extends keyof ConfigMap>(topic: T, payload: ConfigMap[T], error_string: string, reboot_string?: string) {
    return fetch('/' + topic, {
            method: 'PUT',
            credentials: 'same-origin',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(payload)
        })
        .then(response => {
            if (response.ok)
                return response;
            return response.text().catch(() => response.statusText).then(x => {throw new Error(x)});
        })
        .then(reboot_string ? util.getShowRebootModalFn(reboot_string) : undefined)
        .catch(error => {
            util.add_alert(topic.replace("/", "_") + '_failed', 'alert-danger', error_string, error);
            throw error;
        });
}

export function hasFeature(feature: string) {
    return get('info/features').includes(feature);
}

export function default_updater<T extends keyof ConfigMap>(topic: T, exclude?: Array<keyof ConfigMap[T]>, has_save_button=true) {
    let prefix = topic.replace('/', '_');
    let config = get(topic);
    let form = $(`#${prefix}`);
    let save_btn = $(`#${prefix}_save_button`);

    if (has_save_button && save_btn.length == 0) {
        console.error(`save btn not found #${prefix}_save_button`);
    }

    if (has_save_button && !save_btn.prop("disabled"))
        return;

    if (form.length != 0) {
        form.removeClass('was-validated');
    }

    for (let key in config) {
        if (exclude && exclude.includes(key))
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
            elem.prop("checked", value);
            continue;
        }

        if (typeof value == "string" || typeof value == "number") {
            if (elem.is("input") || elem.is("select"))
                elem.val(value);
            else if (elem.is("span"))
                elem.text(value);
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
// like this: register_config_form(..., () => ({a: 1, b:2}), ...
export function register_config_form<T extends keyof ConfigMap>(topic: T, overrides?: () => Partial<ConfigMap[T]>, validation_override?: () => void, error_string?: string, reboot_string?: string) {
    let prefix = topic.replace('/', '_');
    let form = $(`#${prefix}`);
    let save_btn = $(`#${prefix}_save_button`);

    if (form.length == 0) {
        console.error(`Cant register config form ${topic}. Form with id ${prefix} not found.`);
    }

    if (save_btn.length == 0) {
        console.error(`Cant register config form ${topic}. Form save button with id ${prefix}_save_button not found.`);
    }

    form.on('submit', function (this: HTMLFormElement, event: Event) {
        if (validation_override)
            validation_override();

        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            this.classList.add('was-validated');
            return;
        } else {
            this.classList.remove('was-validated');
        }

        default_saver(topic, overrides ? overrides() : undefined, error_string, reboot_string);
    });

    form.on('input', () => save_btn.prop("disabled", false));
}
