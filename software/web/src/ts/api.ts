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
