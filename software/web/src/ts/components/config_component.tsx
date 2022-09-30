/* esp32-firmware
 * Copyright (C) 2022 Matthias Bolte <matthias@tinkerforge.com>
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

import { Component} from "preact";
import { ConfigMap } from "../api_defs";
import * as API from "../api";
import * as util from "../util";

// https://github.com/piotrwitek/utility-types#pickbyvaluet-valuetype
type PickByValue<T, ValueType> = Pick<
  T,
  { [Key in keyof T]-?: T[Key] extends ValueType ? Key : never }[keyof T]
>;

//based on https://stackoverflow.com/a/50895613
function extract<T extends keyof ConfigMap, U extends API.getType[T]>(topic: T, value: Readonly<U>){
    let stencil = API.get(topic)
    let result = {} as API.getType[T];
    for (const property of Object.keys(stencil) as Array<keyof API.getType[T]>) {
        result[property] = value[property];
    }
    return result;
}

export abstract class ConfigComponent<Config extends keyof ConfigMap, P = {}, S = {}> extends Component<P, API.getType[Config] & S> {
    t: Config;
    ignore_updates: boolean = false;
    error_string?: string;
    reboot_string?: string;

    constructor(t: Config, error_string?: string, reboot_string?: string) {
        super();
        this.t = t;
        this.error_string = error_string;
        this.reboot_string = reboot_string;

        util.eventTarget.addEventListener(t, () => {
            if (!this.ignore_updates)
                this.setState(API.get(t) as Partial<API.getType[Config] & S>);
        });
    }

    toggle(x: keyof PickByValue<API.getType[Config] & S, boolean>) {
        return () => this.setState({ [x]: !this.state[x] } as unknown as Partial<API.getType[Config] & S>);
    }

    save = async () => {
        let cfg = extract(this.t, this.state);
        if (!this.isSaveAllowed(cfg))
            throw new Error("saving not allowed");

        cfg = this.transformSave(cfg);

        await this.sendSave(this.t, cfg);
    }

    set<T extends keyof (API.getType[Config] & S)>(x: T) {
        return (s: (API.getType[Config] & S)[T]) => this.setState({ [x]: s } as unknown as Partial<API.getType[Config] & S>);
    }

    // Override this to block saving on a condition
    isSaveAllowed(cfg: API.getType[Config]) {
        return true;
    }

    // Override this to implement a transformation before the config is saved
    transformSave(cfg: API.getType[Config]): API.getType[Config] {
        return cfg;
    }

    // Override this to implement custom saving logic
    async sendSave(t: Config, cfg: API.getType[Config]) {
        await API.save(t, cfg, this.error_string, this.reboot_string);
    }
}
