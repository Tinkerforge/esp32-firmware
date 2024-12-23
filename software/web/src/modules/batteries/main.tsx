/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import * as options from "../../options";
import { __, translate_unchecked } from "../../ts/translation";
import { h, Fragment, ComponentChild } from "preact";
import { Button, ButtonGroup } from "react-bootstrap";
import { FormRow } from "../../ts/components/form_row";
import { InputSelect } from "../../ts/components/input_select";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { SubPage } from "../../ts/components/sub_page";
import { BatteryClassID } from "./battery_class_id.enum";
import { BatteryConfig, BatteryConfigPlugin } from "./types";
import { plugins_init } from "./plugins";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Battery } from "react-feather";

export function BatteriesNavbar() {
    return <NavbarItem name="batteries" module="batteries" title={__("batteries.navbar.batteries")} symbol={<Battery />} />;
}

let config_plugins: {[battery_class: number]: BatteryConfigPlugin} = {};

interface BatteriesState {
    configs: {[battery_slot: number]: BatteryConfig};
}

export class Batteries extends ConfigComponent<null, {}, BatteriesState> {
    constructor() {
        super(null,
              () => __("batteries.script.save_failed"),
              () => __("batteries.script.reboot_content_changed"), {
                  configs: {},
              });

        for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
            util.addApiEventListener_unchecked(`batteries/${battery_slot}/config`, () => {
                let config = API.get_unchecked(`batteries/${battery_slot}/config`);

                if (!this.isDirty()) {
                    this.setState((prevState) => ({
                        configs: {
                            ...prevState.configs,
                            [battery_slot]: config
                        }
                    }));
                }
            });
        }
    }

    override async sendSave(topic: null, new_config: null) {
        for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
            await API.save_unchecked(
                `batteries/${battery_slot}/config`,
                this.state.configs[battery_slot],
                () => __("batteries.script.save_failed"),
                battery_slot == options.BATTERIES_MAX_SLOTS - 1 ? this.reboot_string : undefined);
        }
    }

    override async sendReset(topic: null) {
        for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
            await API.reset_unchecked(`batteries/${battery_slot}/config`, this.error_string, this.reboot_string);
        }
    }

    override getIsModified(topic: null): boolean {
        for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
            if (API.is_modified_unchecked(`batteries/${battery_slot}/config`))
                return true;
        }

        return false;
    }

    render() {
        if (!util.render_allowed())
            return <SubPage name="batteries" />;

        let classes: [string, string][] = [[BatteryClassID.None.toString(), __("batteries.content.battery_class_none")]];
        let battery_slot = 0

        for (let battery_class in config_plugins) {
            classes.push([battery_class.toString(), config_plugins[battery_class].name()])
        }

        return (
            <SubPage name="batteries">
                <ConfigForm id="batteries_config_form" title={__("batteries.content.batteries")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("batteries.content.battery_class")}>
                        <InputSelect
                            items={classes}
                            onValue={(v) => {
                                let battery_class = parseInt(v);

                                if (battery_class != this.state.configs[battery_slot][0]) {
                                    if (battery_class == BatteryClassID.None) {
                                        this.setState({configs: {...this.state.configs, [battery_slot]: [BatteryClassID.None, null]}});
                                    }
                                    else {
                                        this.setState({configs: {...this.state.configs, [battery_slot]: config_plugins[battery_class].new_config()}});
                                    }
                                }
                            }}
                            value={this.state.configs[battery_slot][0].toString()} />
                    </FormRow>

                    {this.state.configs[battery_slot][0] != BatteryClassID.None ?
                        config_plugins[this.state.configs[battery_slot][0]].get_edit_children(this.state.configs[battery_slot], (battery_config) => this.setState({configs: {...this.state.configs, [battery_slot]: battery_config}}))
                        : undefined
                    }
                </ConfigForm>
            </SubPage>
        );
    }
}


export function init() {
    let result = plugins_init();

    for (let plugins of result) {
        for (let i in plugins) {
            if (config_plugins[i]) {
                console.log('Batteries: Overwriting class ID ' + i);
            }

            config_plugins[i] = plugins[i];
        }
    }
}
