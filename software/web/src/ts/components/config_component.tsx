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

import { Component } from "preact";
import { ConfigMap } from "../api_defs";
import { __ } from "../translation";
import * as API from "../api";
import * as util from "../util";

// https://github.com/piotrwitek/utility-types#pickbyvaluet-valuetype
type PickByValue<T, ValueType> = Pick<
    T,
    { [Key in keyof T]-?: T[Key] extends ValueType ? Key : never }[keyof T]
>;

export interface ConfigComponentState {
    internal_isDirty: boolean;
}

export abstract class ConfigComponent<Config extends keyof ConfigMap,
                                      P = {},
                                      // Make sure the additional state passed here does not overlap with the config's keys.
                                      // Only check this if API.getType[Config] is not an array (such as meters/x/config) because Arrays have keys such as toString
                                      // which are allowed to be overlapping.
                                      S extends (API.getType[Config] extends any[] ? {} :
                                                                                     (object & Partial<Record<keyof API.getType[Config], never>>)) = {}
                                     > extends Component<P, API.getType[Config] & S & ConfigComponentState> {
    t: Config;
    error_string?: () => string;
    reboot_string?: () => string;

    constructor(t: Config, error_string?: () => string, reboot_string?: () => string, initial_state?: Partial<API.getType[Config] & S>, props?: P, context?: any) {
        super(props, context);

        this.t = t;
        this.error_string = error_string;
        this.reboot_string = reboot_string;
        this.state = {
            ...(initial_state ? initial_state : {}),
            internal_isDirty: false,
        } as any;

        util.addApiEventListener(t, () => {
            if (!this.state.internal_isDirty) {
                this.setState(API.get(t) as Partial<API.getType[Config] & S & ConfigComponentState>);
            }
        });

        util.addApiEventListener((t + "_modified") as Config, () => {
            // Make sure that clicking the reset button
            // (which changes _modified because it removes the config saved in the ESPs flash)
            // re-renders the component to disable the reset button.
            this.forceUpdate();
        });
    }

    toggle(x: keyof PickByValue<API.getType[Config] & S & ConfigComponentState, boolean>, callback?: () => void) {
        return () => this.setState({ [x]: !this.state[x] } as unknown as Partial<API.getType[Config] & S & ConfigComponentState>, callback);
    }

    save = async () => {
        let cfg = API.extract(this.t, this.state);
        if (!await this.isSaveAllowed(cfg))
            throw new Error("saving not allowed");

        cfg = await this.transformSave(cfg);

        await this.sendSave(this.t, cfg);
    };

    reset = async () => {
        const modal = util.async_modal_ref.current;
        if (!await modal.show({
                title: () => __("reset.reset_modal"),
                body: () =>  this.reboot_string != undefined ? __("reset.reset_modal_body_prefix") + this.reboot_string() + __("reset.reset_modal_body_postfix") : __("reset.reset_modal_body"),
                no_text: () => __("reset.reset_modal_abort"),
                yes_text: () => __("reset.reset_modal_confirm"),
                no_variant: "secondary",
                yes_variant: "danger"
            }))
            return;
        await this.sendReset(this.t);
    };

    isModified = () => {
        return this.getIsModified(this.t);
    };

    isDirty = () => {
        return this.state.internal_isDirty;
    };

    setDirty = (dirty: boolean) => {
        if (this.state.internal_isDirty != dirty)
            this.setState({internal_isDirty: dirty} as any);
    };

    set<T extends keyof (API.getType[Config] & S & ConfigComponentState)>(x: T, callback?: () => void) {
        return (s: (API.getType[Config] & S & ConfigComponentState)[T]) => this.setState({ [x]: s } as unknown as Partial<API.getType[Config] & S & ConfigComponentState>, callback);
    }

    // Override this to block saving on a condition
    async isSaveAllowed(cfg: API.getType[Config]) {
        return true;
    }

    // Override this to implement a transformation before the config is saved
    async transformSave(cfg: API.getType[Config]): Promise<API.getType[Config]> {
        return cfg;
    }

    // Override this to implement custom saving logic
    async sendSave(t: Config, cfg: API.getType[Config]) {
        await API.save(t, cfg, this.error_string, this.reboot_string);
    }

    // Also override this if you override sendSave
    getIsModified(t: Config): boolean {
        return API.is_modified(t);
    }

    // Override this to implement custom reset logic
    async sendReset(t: Config) {
        await API.reset(t, this.error_string, this.reboot_string);
    }
}
