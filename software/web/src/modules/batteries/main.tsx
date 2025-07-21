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
import { __ } from "../../ts/translation";
import { h, Fragment, Component, ComponentChild } from "preact";
import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { InputSelect } from "../../ts/components/input_select";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { SubPage } from "../../ts/components/sub_page";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { InputFloat } from "../../ts/components/input_float";
import { Switch } from "../../ts/components/switch";
import { BatteryClassID } from "./battery_class_id.enum";
import { BatteryConfig, BatteryConfigPlugin } from "./types";
import { RuleConfig } from "../battery_control/types";
import { RuleCondition } from "../battery_control/rule_condition.enum";
import { plugins_init } from "./plugins";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Table } from "../../ts/components/table";
import { Battery } from "react-feather";

const MAX_RULES_PER_TYPE = 32;

export function BatteriesNavbar() {
    return <NavbarItem name="batteries" module="batteries" title={__("batteries.navbar.batteries")} symbol={<Battery />} />;
}

let config_plugins: {[battery_class: number]: BatteryConfigPlugin} = {};

function get_battery_name(battery_configs: {[battery_slot: number]: BatteryConfig}, battery_slot: number) {
    let battery_name = __("batteries.script.battery")(util.hasValue(battery_slot) ? battery_slot : '?');

    if (util.hasValue(battery_slot) && util.hasValue(battery_configs) && util.hasValue(battery_configs[battery_slot]) && util.hasValue(battery_configs[battery_slot][1])) {
        battery_name = battery_configs[battery_slot][1].display_name;
    }

    return battery_name;
}

interface RulesEditorState {
    add_rule_config: RuleConfig;
    edit_rule_config: RuleConfig;
}

interface RulesEditorProps {
    rules: RuleConfig[];
    on_rules: (rules: RuleConfig[]) => void;
}

function get_column_cond(cond: number, th_str: string) {
    switch (cond) {
        case RuleCondition.Ignore: return '';
        case RuleCondition.Below:  return '< ' + th_str;
        case RuleCondition.Above:  return '> ' + th_str;
    }

    return '???';
}

class RulesEditor extends Component<RulesEditorProps, RulesEditorState> {
    constructor(props: RulesEditorProps) {
        super(props);

        this.state = {
            add_rule_config: null,
            edit_rule_config: null,
        } as RulesEditorState;
    }

    render() {
        return <Table
            columnNames={[__("batteries.content.table_rule_desc"), __("batteries.content.table_rule_soc"), __("batteries.content.table_rule_price"), __("batteries.content.table_rule_forecast")]}
            rows={this.props.rules.map((rule_config, i) => {
                return {
                    columnValues: [
                        rule_config.desc,
                        get_column_cond(rule_config.soc_cond, `${rule_config.soc_th} %`),
                        get_column_cond(rule_config.price_cond, `${util.toLocaleFixed(rule_config.price_th / 10, 1)} ct`),
                        get_column_cond(rule_config.forecast_cond, `${rule_config.forecast_th} kWh`),
                    ],
                    editTitle: __("batteries.content.edit_rule_title"),
                    onEditShow: async () => {
                        this.setState({edit_rule_config: {...rule_config}});
                    },
                    onEditGetChildren: () => {
                        let cond_items: [string, string][] = [
                            [RuleCondition.Ignore.toString(), __("batteries.content.condition_ignore")],
                            [RuleCondition.Below.toString(),  __("batteries.content.condition_below")],
                            [RuleCondition.Above.toString(),  __("batteries.content.condition_above")],
                        ];

                        return [
                            <FormRow label={__("batteries.content.edit_rule_desc")}>
                                <InputText
                                    maxLength={32}
                                    value={this.state.edit_rule_config.desc}
                                    onValue={(v) => {
                                        this.setState({edit_rule_config: {...this.state.edit_rule_config, desc: v}});
                                    }} />
                            </FormRow>,
                            <FormRow label={__("batteries.content.edit_rule_soc")}>
                                <InputNumber
                                    required={this.state.edit_rule_config.soc_cond != RuleCondition.Ignore}
                                    disabled={this.state.edit_rule_config.soc_cond == RuleCondition.Ignore}
                                    min={0}
                                    max={100}
                                    unit="%"
                                    value={this.state.edit_rule_config.soc_th}
                                    onValue={(v) => {
                                        this.setState({edit_rule_config: {...this.state.edit_rule_config, soc_th: v}});
                                    }}>
                                    <InputSelect
                                        style="border-top-right-radius: 0; border-bottom-right-radius: 0;"
                                        placeholder={__("select")}
                                        items={cond_items}
                                        onValue={(v) => {
                                            let soc_cond = parseInt(v);
                                            let soc_th = this.state.edit_rule_config.soc_th;

                                            if (soc_cond == RuleCondition.Ignore) {
                                                soc_th = rule_config.soc_th;
                                            }

                                            this.setState({edit_rule_config: {...this.state.edit_rule_config, soc_cond: soc_cond, soc_th: soc_th}});
                                        }}
                                        value={this.state.edit_rule_config.soc_cond.toString()} />
                                </InputNumber>
                            </FormRow>,
                            <FormRow label={__("batteries.content.edit_rule_price")}>
                                <InputFloat
                                    required={this.state.edit_rule_config.price_cond != RuleCondition.Ignore}
                                    disabled={this.state.edit_rule_config.price_cond == RuleCondition.Ignore}
                                    min={-32768}
                                    max={32767}
                                    digits={1}
                                    unit="ct"
                                    value={this.state.edit_rule_config.price_th}
                                    onValue={(v) => {
                                        this.setState({edit_rule_config: {...this.state.edit_rule_config, price_th: v}});
                                    }}>
                                    <InputSelect
                                        style="border-top-right-radius: 0; border-bottom-right-radius: 0;"
                                        placeholder={__("select")}
                                        items={cond_items}
                                        onValue={(v) => {
                                            let price_cond = parseInt(v);
                                            let price_th = this.state.edit_rule_config.price_th;

                                            if (price_cond == RuleCondition.Ignore) {
                                                price_th = rule_config.price_th;
                                            }

                                            this.setState({edit_rule_config: {...this.state.edit_rule_config, price_cond: price_cond, price_th: price_th}});
                                        }}
                                        value={this.state.edit_rule_config.price_cond.toString()} />
                                </InputFloat>
                            </FormRow>,
                            <FormRow label={__("batteries.content.edit_rule_forecast")}>
                                <InputNumber
                                    required={this.state.edit_rule_config.forecast_cond != RuleCondition.Ignore}
                                    disabled={this.state.edit_rule_config.forecast_cond == RuleCondition.Ignore}
                                    min={0}
                                    max={65535}
                                    unit="kWh"
                                    value={this.state.edit_rule_config.forecast_th}
                                    onValue={(v) => {
                                        this.setState({edit_rule_config: {...this.state.edit_rule_config, forecast_th: v}});
                                    }}>
                                    <InputSelect
                                        style="border-top-right-radius: 0; border-bottom-right-radius: 0;"
                                        placeholder={__("select")}
                                        items={cond_items}
                                        onValue={(v) => {
                                            let forecast_cond = parseInt(v);
                                            let forecast_th = this.state.edit_rule_config.forecast_th;

                                            if (forecast_cond == RuleCondition.Ignore) {
                                                forecast_th = rule_config.forecast_th;
                                            }

                                            this.setState({edit_rule_config: {...this.state.edit_rule_config, forecast_cond: forecast_cond, forecast_th: forecast_th}});
                                        }}
                                        value={this.state.edit_rule_config.forecast_cond.toString()} />
                                </InputNumber>
                            </FormRow>,
                        ];
                    },
                    onEditSubmit: async () => {
                        this.props.on_rules(this.props.rules.map((r, k) => k === i ? this.state.edit_rule_config : r));
                    },
                    onEditHide: async () => {
                    },
                    onRemoveClick: async () => {
                        this.props.on_rules(this.props.rules.filter((r, k) => k !== i));
                        return true;
                    },
                }
            })}
            addEnabled={this.props.rules.length < MAX_RULES_PER_TYPE}
            addTitle={__("batteries.content.add_rule_title")}
            addMessage={__("batteries.content.add_rule_message")(this.props.rules.length, MAX_RULES_PER_TYPE)}
            onAddShow={async () => {
                let rule_config: RuleConfig = {
                    desc: "",
                    soc_cond: RuleCondition.Ignore,
                    soc_th: null,
                    price_cond:RuleCondition.Ignore,
                    price_th: null,
                    forecast_cond: RuleCondition.Ignore,
                    forecast_th: null,
                };

                this.setState({add_rule_config: rule_config});
            }}
            onAddGetChildren={() => {
                let cond_items: [string, string][] = [
                    [RuleCondition.Ignore.toString(), __("batteries.content.condition_ignore")],
                    [RuleCondition.Below.toString(),  __("batteries.content.condition_below")],
                    [RuleCondition.Above.toString(),  __("batteries.content.condition_above")],
                ];

                return [
                    <FormRow label={__("batteries.content.add_rule_desc")}>
                        <InputText
                            maxLength={32}
                            value={this.state.add_rule_config.desc}
                            onValue={(v) => {
                                this.setState({add_rule_config: {...this.state.add_rule_config, desc: v}});
                            }} />
                    </FormRow>,
                    <FormRow label={__("batteries.content.add_rule_soc")}>
                       <InputNumber
                            required={this.state.add_rule_config.soc_cond != RuleCondition.Ignore}
                            disabled={this.state.add_rule_config.soc_cond == RuleCondition.Ignore}
                            min={0}
                            max={100}
                            unit="%"
                            value={this.state.add_rule_config.soc_th}
                            onValue={(v) => {
                                this.setState({add_rule_config: {...this.state.add_rule_config, soc_th: v}});
                            }}>
                            <InputSelect
                                style="border-top-right-radius: 0; border-bottom-right-radius: 0;"
                                placeholder={__("select")}
                                items={cond_items}
                                onValue={(v) => {
                                    let soc_cond = parseInt(v);
                                    let soc_th = this.state.add_rule_config.soc_th;

                                    if (soc_cond == RuleCondition.Ignore) {
                                        soc_th = null;
                                    }

                                    this.setState({add_rule_config: {...this.state.add_rule_config, soc_cond: soc_cond, soc_th: soc_th}});
                                }}
                                value={this.state.add_rule_config.soc_cond.toString()} />
                        </InputNumber>
                    </FormRow>,
                    <FormRow label={__("batteries.content.add_rule_price")}>
                       <InputFloat
                            required={this.state.add_rule_config.price_cond != RuleCondition.Ignore}
                            disabled={this.state.add_rule_config.price_cond == RuleCondition.Ignore}
                            min={-32768}
                            max={32767}
                            digits={1}
                            unit="ct"
                            value={this.state.add_rule_config.price_th}
                            onValue={(v) => {
                                this.setState({add_rule_config: {...this.state.add_rule_config, price_th: v}});
                            }}>
                            <InputSelect
                                style="border-top-right-radius: 0; border-bottom-right-radius: 0;"
                                placeholder={__("select")}
                                items={cond_items}
                                onValue={(v) => {
                                    let price_cond = parseInt(v);
                                    let price_th = this.state.add_rule_config.price_th;

                                    if (price_cond == RuleCondition.Ignore) {
                                        price_th = null;
                                    }

                                    this.setState({add_rule_config: {...this.state.add_rule_config, price_cond: price_cond, price_th: price_th}});
                                }}
                                value={this.state.add_rule_config.price_cond.toString()} />
                        </InputFloat>
                    </FormRow>,
                    <FormRow label={__("batteries.content.add_rule_forecast")}>
                       <InputNumber
                            required={this.state.add_rule_config.forecast_cond != RuleCondition.Ignore}
                            disabled={this.state.add_rule_config.forecast_cond == RuleCondition.Ignore}
                            min={0}
                            max={65535}
                            unit="kWh"
                            value={this.state.add_rule_config.forecast_th}
                            onValue={(v) => {
                                this.setState({add_rule_config: {...this.state.add_rule_config, forecast_th: v}});
                            }}>
                            <InputSelect
                                style="border-top-right-radius: 0; border-bottom-right-radius: 0;"
                                placeholder={__("select")}
                                items={cond_items}
                                onValue={(v) => {
                                    let forecast_cond = parseInt(v);
                                    let forecast_th = this.state.add_rule_config.forecast_th;

                                    if (forecast_cond == RuleCondition.Ignore) {
                                        forecast_th = null;
                                    }

                                    this.setState({add_rule_config: {...this.state.add_rule_config, forecast_cond: forecast_cond, forecast_th: forecast_th}});
                                }}
                                value={this.state.add_rule_config.forecast_cond.toString()} />
                        </InputNumber>
                    </FormRow>,
                ];
            }}
            onAddSubmit={async () => {
                let rules = [...this.props.rules];

                rules.push(this.state.add_rule_config);

                this.props.on_rules(rules);
            }}
            onAddHide={async () => {
            }}
            />;
    }
}

interface BatteriesState {
    configs: {[battery_slot: number]: BatteryConfig};
    add_battery_slot: number;
    add_battery_config: BatteryConfig;
    edit_battery_slot: number;
    edit_battery_config: BatteryConfig;
    rules_permit_grid_charge: RuleConfig[];
    rules_forbid_discharge: RuleConfig[];
    rules_forbid_charge: RuleConfig[];
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
                  rules_permit_grid_charge: null,
                  rules_forbid_discharge: null,
                  rules_forbid_charge: null,
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

        util.addApiEventListener('battery_control/rules_permit_grid_charge', () => {
            if (!this.isDirty()) {
                this.setState({rules_permit_grid_charge: API.get('battery_control/rules_permit_grid_charge') as RuleConfig[] /* FIXME */});
            }
        });

        util.addApiEventListener('battery_control/rules_forbid_discharge', () => {
            if (!this.isDirty()) {
                this.setState({rules_forbid_discharge: API.get('battery_control/rules_forbid_discharge') as RuleConfig[] /* FIXME */});
            }
        });

        util.addApiEventListener('battery_control/rules_forbid_charge', () => {
            if (!this.isDirty()) {
                this.setState({rules_forbid_charge: API.get('battery_control/rules_forbid_charge') as RuleConfig[] /* FIXME */});
            }
        });
    }

    override async sendSave(topic: 'battery_control/config', new_config: API.getType['battery_control/config']) {
        for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
            await API.save_unchecked(
                `batteries/${battery_slot}/config`,
                this.state.configs[battery_slot],
                () => __("batteries.script.save_failed"));
        }

        // FIXME: API.save() cannot handle top-level array
        await API.save_unchecked('battery_control/rules_permit_grid_charge',
                       this.state.rules_permit_grid_charge,
                       () => __("batteries.script.save_failed"));

        // FIXME: API.save() cannot handle top-level array
        await API.save_unchecked('battery_control/rules_forbid_discharge',
                       this.state.rules_forbid_discharge,
                       () => __("batteries.script.save_failed"));

        // FIXME: API.save() cannot handle top-level array
        await API.save_unchecked('battery_control/rules_forbid_charge',
                       this.state.rules_forbid_charge,
                       () => __("batteries.script.save_failed"));

        await super.sendSave(topic, new_config);
    }

    override async sendReset(topic: 'battery_control/config') {
        for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
            await API.reset_unchecked(`batteries/${battery_slot}/config`, this.error_string);
        }

        await API.reset('battery_control/rules_permit_grid_charge', this.error_string);
        await API.reset('battery_control/rules_forbid_discharge', this.error_string);
        await API.reset('battery_control/rules_forbid_charge', this.error_string);

        await super.sendReset(topic);
    }

    override getIsModified(topic: 'battery_control/config'): boolean {
        for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
            if (API.is_modified_unchecked(`batteries/${battery_slot}/config`)) {
                return true;
            }
        }

        if (API.is_modified('battery_control/rules_permit_grid_charge')) {
            return true;
        }

        if (API.is_modified('battery_control/rules_forbid_discharge')) {
            return true;
        }

        if (API.is_modified('battery_control/rules_forbid_charge')) {
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

                    <FormSeparator heading={__("batteries.content.managed_batteries")} />
                    <div class="form-group">
                        <Table
                            columnNames={[__("batteries.content.table_battery_display_name"), __("batteries.content.table_battery_class")]}
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

                                        for (let free_battery_slot = 0; free_battery_slot < options.BATTERIES_MAX_SLOTS; ++free_battery_slot) {
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
                                            rows = rows.concat(<Fragment key={`edit_battery_children_${this.state.edit_battery_config[0]}`}>{config_plugins[this.state.edit_battery_config[0]].get_edit_children(this.state.edit_battery_config, (battery_config) => this.setState({edit_battery_config: battery_config}))}</Fragment>);
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
                                        return true;
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
                            addEnabled={active_battery_slots.length < options.BATTERIES_MAX_SLOTS}
                            addTitle={__("batteries.content.add_battery_title")}
                            addMessage={__("batteries.content.add_battery_message")(active_battery_slots.length, options.BATTERIES_MAX_SLOTS)}
                            onAddShow={async () => {
                                let add_battery_slot = null;

                                for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
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

                                for (let free_battery_slot = 0; free_battery_slot < options.BATTERIES_MAX_SLOTS; ++free_battery_slot) {
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
                                    rows = rows.concat(<Fragment key={`add_battery_children_${this.state.edit_battery_config[0]}`}>{config_plugins[this.state.add_battery_config[0]].get_edit_children(this.state.add_battery_config, (battery_config) => this.setState({add_battery_config: battery_config}))}</Fragment>);
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
                    </div>

                    <FormSeparator heading={__("batteries.content.rules_permit_grid_charge")} />
                    <div class="form-group">
                        <RulesEditor rules={this.state.rules_permit_grid_charge} on_rules={(rules: RuleConfig[]) => {
                            this.setState({rules_permit_grid_charge: rules});
                            this.setDirty(true);
                        }} />
                    </div>

                    <FormSeparator heading={__("batteries.content.rules_forbid_discharge")} />
                    <div class="form-group">
                        <FormRow label={__("batteries.content.forbid_discharge_during_fast_charge")}>
                            <Switch desc={__("batteries.content.forbid_discharge_during_fast_charge_desc")}
                                checked={this.state.forbid_discharge_during_fast_charge}
                                onClick={this.toggle("forbid_discharge_during_fast_charge")}
                            />
                        </FormRow>
                        <RulesEditor rules={this.state.rules_forbid_discharge} on_rules={(rules: RuleConfig[]) => {
                            this.setState({rules_forbid_discharge: rules});
                            this.setDirty(true);
                        }} />
                    </div>

                    <FormSeparator heading={__("batteries.content.rules_forbid_charge")} />
                    <div class="form-group">
                        <RulesEditor rules={this.state.rules_forbid_charge} on_rules={(rules: RuleConfig[]) => {
                            this.setState({rules_forbid_charge: rules});
                            this.setDirty(true);
                        }} />
                    </div>
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
