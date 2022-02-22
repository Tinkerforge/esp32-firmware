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
export class ApiEventTarget implements EventTarget {
    private delegate = document.createDocumentFragment();

    public addEventListener<T extends keyof EventMap>(type: T, listener: (this: ApiEventTarget, ev: EventMap[T]) => any, options?: boolean | AddEventListenerOptions) : void;

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

export function trigger<T extends keyof ConfigMap>(topic: T, event_source: ApiEventTarget) {
    event_source.dispatchEvent(new MessageEvent<Readonly<ConfigMap[T]>>(topic, {'data': get(topic)}));
}

export function save<T extends keyof ConfigMap>(topic: T, payload: ConfigMap[T], error_string: string, reboot_string?: string) {
    console.log(reboot_string);
    $.ajax({
        url: '/' + topic + '_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: reboot_string ? util.getShowRebootModalFn(reboot_string) : undefined,
        error: (xhr, status, error) => util.add_alert(topic + '_failed', 'alert-danger', error_string, error + ': ' + xhr.responseText),
    });
}
