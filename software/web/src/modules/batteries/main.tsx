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
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { InputSelect } from "../../ts/components/input_select";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { SubPage } from "../../ts/components/sub_page";
import { Switch } from "../../ts/components/switch";
import { BatteryClassID } from "./battery_class_id.enum";
import { BatteryConfig, BatteryConfigPlugin } from "./types";
import { plugins_init } from "./plugins";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Table } from "../../ts/components/table";
import { Battery } from "react-feather";

export function BatteriesNavbar() {
    return <NavbarItem name="batteries" module="batteries" title={__("batteries.navbar.batteries")} symbol={<Battery />} />;
}

let config_plugins: {[battery_class: number]: BatteryConfigPlugin} = {};

interface BatteriesState {
    configs: {[battery_slot: number]: BatteryConfig};
    add_battery_slot: number;
    add_battery_config: BatteryConfig;
    edit_battery_slot: number;
    edit_battery_config: BatteryConfig;
}

function get_battery_name(battery_configs: {[battery_slot: number]: BatteryConfig}, battery_slot: number) {
    let battery_name = __("batteries.script.battery")(util.hasValue(battery_slot) ? battery_slot : '?');

    if (util.hasValue(battery_slot) && util.hasValue(battery_configs) && util.hasValue(battery_configs[battery_slot]) && util.hasValue(battery_configs[battery_slot][1])) {
        battery_name = battery_configs[battery_slot][1].display_name;
    }

    return battery_name;
}

export class Batteries extends ConfigComponent<'battery_control/config', {}, BatteriesState> {
    constructor() {
        super('battery_control/config',
              () => __("batteries.script.save_failed"),
              () => __("batteries.script.reboot_content_changed"), {
                  configs: {},
                  add_battery_slot: null,
                  add_battery_config: [BatteryClassID.None, null],
                  edit_battery_slot: null,
                  edit_battery_config: [BatteryClassID.None, null],
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

    override async sendSave(topic: 'battery_control/config', new_config: API.getType['battery_control/config']) {
        for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
            await API.save_unchecked(
                `batteries/${battery_slot}/config`,
                this.state.configs[battery_slot],
                () => __("batteries.script.save_failed"));
        }

        await super.sendSave(topic, new_config);
    }

    override async sendReset(topic: 'battery_control/config') {
        for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
            await API.reset_unchecked(`batteries/${battery_slot}/config`, this.error_string);
        }

        await super.sendReset(topic);
    }

    override getIsModified(topic: 'battery_control/config'): boolean {
        for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
            if (API.is_modified_unchecked(`batteries/${battery_slot}/config`))
                return true;
        }

        return super.getIsModified(topic);
    }

    import_config(json: string, current_config: BatteryConfig) {
        let new_config;

        try {
            new_config = JSON.parse(json);
        }
        catch (error) {
            console.log("Batteries: JSON of imported config is malformed:", error.message);
            throw new Error(__("batteries.content.battery_import_error_json_malformed"));
        }

        if (!util.isNonNullObject(new_config)) {
            console.log("Batteries: JSON of imported config has no toplevel object");
            throw new Error(__("batteries.content.battery_import_error_config_malformed"));
        }

        if (config_plugins[new_config[0]] === undefined) {
            console.log("Batteries: Imported config has unknown class:", new_config[0]);
            throw new Error(__("batteries.content.battery_import_error_class_unknown"));
        }

        if (config_plugins[new_config[0]].import_config === undefined) {
            console.log("Batteries: Imported config class doesn't support import:", new_config[0]);
            throw new Error(__("batteries.content.battery_import_error_import_unsupported"));
        }

        new_config = config_plugins[new_config[0]].import_config(new_config, current_config);

        if (!util.hasValue(new_config)) {
            throw new Error(__("batteries.content.battery_import_error_config_malformed"));
        }

        return new_config;
    }

    render() {
        if (!util.render_allowed())
            return <SubPage name="batteries" />;

        const bc_state = API.get("battery_control/state");
        let active_battery_slots = Object.keys(this.state.configs).filter((battery_slot_str) => this.state.configs[parseInt(battery_slot_str)][0] != BatteryClassID.None);

        return (
            <SubPage name="batteries">
                <ConfigForm id="batteries_config_form" title={__("batteries.content.batteries")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("batteries.content.grid_charge_permitted")}>
                        <IndicatorGroup
                            style="width: 100%"
                            class="flex-wrap"
                            value={bc_state.grid_charge_permitted ? 1 : 0}
                            items={[
                                ["success", __("batteries.content.permitted_no")],
                                ["warning", __("batteries.content.permitted_yes")],
                            ]}/>
                    </FormRow>

                    <FormRow label={__("batteries.content.discharge_forbidden")}>
                        <IndicatorGroup
                            style="width: 100%"
                            class="flex-wrap"
                            value={bc_state.discharge_forbidden ? 1 : 0}
                            items={[
                                ["success", __("batteries.content.forbidden_no")],
                                ["warning", __("batteries.content.forbidden_yes")],
                            ]}/>
                    </FormRow>

                    <FormRow label={__("batteries.content.charge_forbidden")}>
                        <IndicatorGroup
                            style="width: 100%"
                            class="flex-wrap"
                            value={bc_state.charge_forbidden ? 1 : 0}
                            items={[
                                ["success", __("batteries.content.forbidden_no")],
                                ["warning", __("batteries.content.forbidden_yes")],
                            ]}/>
                    </FormRow>

                    <FormRow label={__("batteries.content.forbid_discharge_during_fast_charge")}>
                        <Switch desc={__("batteries.content.forbid_discharge_during_fast_charge_desc")}
                            checked={this.state.forbid_discharge_during_fast_charge}
                            onClick={this.toggle("forbid_discharge_during_fast_charge")}
                        />
                    </FormRow>

                    <FormRow label={__("batteries.content.managed_batteries")}>
                        <Table
                            columnNames={[__("batteries.content.table_display_name"), __("batteries.content.table_class")]}
                            rows={active_battery_slots.map((battery_slot_str) => {
                                let battery_slot = parseInt(battery_slot_str);
                                let config = this.state.configs[battery_slot];

                                return {
                                    columnValues: [
                                        get_battery_name(this.state.configs, battery_slot),
                                        config_plugins[config[0]].name()
                                    ],
                                    editTitle: __("batteries.content.edit_battery_title"),
                                    onEditShow: async () => {
                                        let config_plugin = config_plugins[config[0]];
                                        if (!config_plugin) {
                                            console.log("No config plugin available for battery class", config[0]);
                                            this.setState({edit_battery_slot: battery_slot, edit_battery_config: [BatteryClassID.None, null]});
                                            return;
                                        }
                                        this.setState({edit_battery_slot: battery_slot, edit_battery_config: config_plugin.clone_config(config)});
                                    },
                                    onEditGetChildren: () => {
                                        let slots: [string, string][] = [];
                                        let classes: [string, string][] = [];

                                        for (let free_battery_slot = 0; free_battery_slot < BATTERIES_SLOTS; ++free_battery_slot) {
                                            if (this.state.configs[free_battery_slot][0] == BatteryClassID.None || free_battery_slot == battery_slot) {
                                                slots.push([free_battery_slot.toString(), free_battery_slot.toString()]);
                                            }
                                        }

                                        for (let battery_class in config_plugins) {
                                            classes.push([battery_class.toString(), config_plugins[battery_class].name()])
                                        }

                                        let rows: ComponentChild[] = [<>
                                            <FormRow label={__("batteries.content.edit_battery_slot")}>
                                                <InputSelect
                                                    items={slots}
                                                    onValue={(v) => this.setState({edit_battery_slot: parseInt(v)})}
                                                    value={this.state.edit_battery_slot.toString()} />
                                            </FormRow>
                                            <FormRow label={__("batteries.content.edit_battery_class")}>
                                                <InputSelect
                                                    placeholder={__("select")}
                                                    items={classes}
                                                    onValue={(v) => {
                                                        let battery_class = parseInt(v);

                                                        if (battery_class != this.state.edit_battery_config[0]) {
                                                            if (battery_class == BatteryClassID.None) {
                                                                this.setState({edit_battery_config: [BatteryClassID.None, null]});
                                                            }
                                                            else {
                                                                this.setState({edit_battery_config: config_plugins[battery_class].new_config()});
                                                            }
                                                        }
                                                    }}
                                                    value={this.state.edit_battery_config[0].toString()} />
                                            </FormRow>
                                        </>]

                                        if (this.state.edit_battery_config[0] != BatteryClassID.None) {
                                            rows = rows.concat(<Fragment key={`edit_children_${this.state.edit_battery_config[0]}`}>{config_plugins[this.state.edit_battery_config[0]].get_edit_children(this.state.edit_battery_config, (battery_config) => this.setState({edit_battery_config: battery_config}))}</Fragment>);
                                        }

                                        return rows;
                                    },
                                    onEditSubmit: async () => {
                                        this.setState({configs: {...this.state.configs, [battery_slot]: [BatteryClassID.None, null], [this.state.edit_battery_slot]: this.state.edit_battery_config}});
                                        this.setDirty(true);
                                    },
                                    onEditHide: async () => {
                                        if (this.state.edit_battery_config[0] != BatteryClassID.None && config_plugins[this.state.edit_battery_config[0]].hide) {
                                            config_plugins[this.state.edit_battery_config[0]].hide();
                                        }
                                    },
                                    onRemoveClick: async () => {
                                        this.setState({configs: {...this.state.configs, [battery_slot]: [BatteryClassID.None, null]}});
                                        this.setDirty(true);
                                    },
                                    onEditImport: async (json: string) => {
                                        this.setState({edit_battery_config: this.import_config(json, this.state.edit_battery_config)});
                                    },
                                    onEditExport: this.state.edit_battery_config[0] != BatteryClassID.None && config_plugins[this.state.edit_battery_config[0]].export_config ? async () => {
                                        return JSON.stringify(config_plugins[this.state.edit_battery_config[0]].export_config(this.state.edit_battery_config), null, 4);
                                    } : undefined,
                                    editExportBasename: __("batteries.content.battery_export_basename") + (this.state.edit_battery_config[0] != BatteryClassID.None && config_plugins[this.state.edit_battery_config[0]].export_basename_suffix ? config_plugins[this.state.edit_battery_config[0]].export_basename_suffix() : ""),
                                }
                            })}
                            addEnabled={active_battery_slots.length < BATTERIES_SLOTS}
                            addTitle={__("batteries.content.add_battery_title")}
                            addMessage={__("batteries.content.add_battery_message")(active_battery_slots.length, BATTERIES_SLOTS)}
                            onAddShow={async () => {
                                let add_battery_slot = null;

                                for (let battery_slot = 0; battery_slot < BATTERIES_SLOTS; ++battery_slot) {
                                    if (this.state.configs[battery_slot][0] == BatteryClassID.None) {
                                        add_battery_slot = battery_slot;
                                        break;
                                    }
                                }

                                this.setState({add_battery_slot: add_battery_slot, add_battery_config: [BatteryClassID.None, null]});
                            }}
                            onAddGetChildren={() => {
                                let slots: [string, string][] = [];
                                let classes: [string, string][] = [];

                                for (let free_battery_slot = 0; free_battery_slot < BATTERIES_SLOTS; ++free_battery_slot) {
                                    if (this.state.configs[free_battery_slot][0] == BatteryClassID.None) {
                                        slots.push([free_battery_slot.toString(), free_battery_slot.toString()]);
                                    }
                                }

                                for (let battery_class in config_plugins) {
                                    classes.push([battery_class.toString(), config_plugins[battery_class].name()])
                                }

                                let rows: ComponentChild[] = [
                                    <FormRow label={__("batteries.content.add_battery_slot")}>
                                        <InputSelect
                                            placeholder={__("select")}
                                            items={slots}
                                            onValue={(v) => this.setState({add_battery_slot: parseInt(v)})}
                                            value={this.state.add_battery_slot !== null ? this.state.add_battery_slot.toString() : null}
                                            required />
                                    </FormRow>,
                                    <FormRow label={__("batteries.content.add_battery_class")}>
                                        <InputSelect
                                            placeholder={__("select")}
                                            items={classes}
                                            onValue={(v) => {
                                                let battery_class = parseInt(v);

                                                if (battery_class != this.state.add_battery_config[0]) {
                                                    if (battery_class == BatteryClassID.None) {
                                                        this.setState({add_battery_config: [BatteryClassID.None, null]});
                                                    }
                                                    else {
                                                        this.setState({add_battery_config: config_plugins[battery_class].new_config()});
                                                    }
                                                }
                                            }}
                                            value={this.state.add_battery_config[0].toString()}
                                            required />
                                    </FormRow>
                                ];

                                if (this.state.add_battery_config[0] != BatteryClassID.None) {
                                    rows = rows.concat(<Fragment key={`edit_children_${this.state.edit_battery_config[0]}`}>{config_plugins[this.state.add_battery_config[0]].get_edit_children(this.state.add_battery_config, (battery_config) => this.setState({add_battery_config: battery_config}))}</Fragment>);
                                }

                                return rows;
                            }}
                            onAddSubmit={async () => {
                                this.setState({configs: {...this.state.configs, [this.state.add_battery_slot]: this.state.add_battery_config}});
                                this.setDirty(true);
                            }}
                            onAddHide={async () => {
                                if (this.state.add_battery_config[0] != BatteryClassID.None && config_plugins[this.state.add_battery_config[0]].hide) {
                                    await config_plugins[this.state.add_battery_config[0]].hide();
                                }
                            }}
                            onAddImport={async (json: string) => {
                                this.setState({add_battery_config: this.import_config(json, this.state.add_battery_config)});
                            }}
                            onAddExport={this.state.add_battery_config[0] != BatteryClassID.None && config_plugins[this.state.add_battery_config[0]].export_config ? async () => {
                                return JSON.stringify(config_plugins[this.state.add_battery_config[0]].export_config(this.state.add_battery_config), null, 4);
                            } : undefined}
                            addExportBasename={__("batteries.content.battery_export_basename") + (this.state.add_battery_config[0] != BatteryClassID.None && config_plugins[this.state.add_battery_config[0]].export_basename_suffix ? config_plugins[this.state.add_battery_config[0]].export_basename_suffix() : "")}
                            />
                    </FormRow>
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
