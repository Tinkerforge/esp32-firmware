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

//#include "module_available.inc"

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import * as options from "../../options";
import { __ } from "../../ts/translation";
import { h, Fragment, Component, ComponentChild, createRef } from "preact";
import { effect } from "@preact/signals-core";
import { Alert, Collapse } from "react-bootstrap";
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
//#if MODULE_BATTERY_CONTROL_AVAILABLE
import { RuleConfig } from "../battery_control/types";
import { RuleCondition } from "../battery_control/rule_condition.enum";
import { ScheduleRuleCondition } from "../battery_control/schedule_rule_condition.enum";
//#endif
//#if MODULE_DAY_AHEAD_PRICES_AVAILABLE
import { get_price_from_index } from "../day_ahead_prices/main";
//#endif
import { plugins_init } from "./plugins";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Table } from "../../ts/components/table";
import { UplotLoader } from "../../ts/components/uplot_loader";
import { UplotData, UplotWrapperB, UplotPath } from "../../ts/components/uplot_wrapper_2nd";
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

//#if MODULE_BATTERY_CONTROL_AVAILABLE
interface RulesEditorState {
    add_rule_config: RuleConfig;
    edit_rule_config: RuleConfig;
    all_conditions_ignored: boolean;
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

function get_column_schedule_cond(cond: number) {
    switch (cond) {
        case ScheduleRuleCondition.Ignore:       return '';
        case ScheduleRuleCondition.Cheap:        return __("batteries.content.condition_schedule_cheap");
        case ScheduleRuleCondition.NotCheap:     return __("batteries.content.condition_schedule_not_cheap");
        case ScheduleRuleCondition.Expensive:    return __("batteries.content.condition_schedule_expensive");
        case ScheduleRuleCondition.NotExpensive: return __("batteries.content.condition_schedule_not_expensive");
        case ScheduleRuleCondition.Moderate:     return __("batteries.content.condition_schedule_moderate");
    }

    return '???';
}

class RulesEditor extends Component<RulesEditorProps, RulesEditorState> {
    constructor(props: RulesEditorProps) {
        super(props);

        this.state = {
            add_rule_config: null,
            edit_rule_config: null,
            all_conditions_ignored: false,
        } as RulesEditorState;
    }

    render() {
        return <Table
            columnNames={[__("batteries.content.table_rule_desc"), __("batteries.content.table_rule_soc"), __("batteries.content.table_rule_price"), __("batteries.content.table_rule_forecast"), __("batteries.content.table_rule_schedule")]}
            rows={this.props.rules.map((rule_config, i) => {
                return {
                    columnValues: [
                        rule_config.desc,
                        get_column_cond(rule_config.soc_cond, `${rule_config.soc_th} %`),
                        get_column_cond(rule_config.price_cond, `${util.toLocaleFixed(rule_config.price_th / 10, 1)} ct`),
                        get_column_cond(rule_config.forecast_cond, `${rule_config.forecast_th} kWh`),
                        get_column_schedule_cond(rule_config.schedule_cond),
                    ],
                    editTitle: __("batteries.content.edit_rule_title"),
                    onEditShow: async () => {
                        this.setState({edit_rule_config: {...rule_config}, all_conditions_ignored: false});
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
                                            } else {
                                                this.setState({all_conditions_ignored: false});
                                            }

                                            this.setState({edit_rule_config: {...this.state.edit_rule_config, soc_cond: soc_cond, soc_th: soc_th}});
                                        }}
                                        value={this.state.edit_rule_config.soc_cond.toString()} />
                                </InputNumber>
                            </FormRow>,
                            <FormRow label={__("batteries.content.edit_rule_price")} label_muted={__("batteries.content.edit_rule_price_muted")}>
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
                                            } else {
                                                this.setState({all_conditions_ignored: false});
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
                                            } else {
                                                this.setState({all_conditions_ignored: false});
                                            }

                                            this.setState({edit_rule_config: {...this.state.edit_rule_config, forecast_cond: forecast_cond, forecast_th: forecast_th}});
                                        }}
                                        value={this.state.edit_rule_config.forecast_cond.toString()} />
                                </InputNumber>
                            </FormRow>,
                            <FormRow label={__("batteries.content.edit_rule_schedule")}>
                                <InputSelect
                                    placeholder={__("select")}
                                    items={[
                                        [ScheduleRuleCondition.Ignore.toString(),       __("batteries.content.condition_ignore")],
                                        [ScheduleRuleCondition.Cheap.toString(),        __("batteries.content.condition_schedule_cheap")],
                                        [ScheduleRuleCondition.NotCheap.toString(),     __("batteries.content.condition_schedule_not_cheap")],
                                        [ScheduleRuleCondition.Expensive.toString(),    __("batteries.content.condition_schedule_expensive")],
                                        [ScheduleRuleCondition.NotExpensive.toString(), __("batteries.content.condition_schedule_not_expensive")],
                                        [ScheduleRuleCondition.Moderate.toString(),     __("batteries.content.condition_schedule_moderate")],
                                            ]}
                                    onValue={(v) => {
                                        const schedule_cond = parseInt(v);

                                        if (schedule_cond != ScheduleRuleCondition.Ignore) {
                                            this.setState({all_conditions_ignored: false});
                                        }

                                        this.setState({edit_rule_config: {...this.state.edit_rule_config, schedule_cond: schedule_cond}});
                                    }}
                                    value={this.state.edit_rule_config.schedule_cond.toString()} />
                            </FormRow>,
                            <FormRow label={__("batteries.content.edit_rule_fast_chg")}>
                                <InputSelect
                                    placeholder={__("select")}
                                    items={[
                                        [RuleCondition.Ignore.toString(), __("batteries.content.condition_ignore")],
                                        [RuleCondition.Below.toString(),  __("batteries.content.condition_fast_chg_inactive")],
                                        [RuleCondition.Above.toString(),  __("batteries.content.condition_fast_chg_active")],
                                    ]}
                                    onValue={(v) => {
                                        let fast_chg_cond = parseInt(v);

                                        if (fast_chg_cond != RuleCondition.Ignore) {
                                            this.setState({all_conditions_ignored: false});
                                        }

                                        this.setState({edit_rule_config: {...this.state.edit_rule_config, fast_chg_cond: fast_chg_cond}});
                                    }}
                                    value={this.state.edit_rule_config.fast_chg_cond.toString()} />
                            </FormRow>,
                            <Collapse in={this.state.all_conditions_ignored}>
                                <div>
                                    <Alert variant="danger">
                                        {__("batteries.content.invalid_feedback_all_ignored")}
                                    </Alert>
                                </div>
                            </Collapse>,
                        ];
                    },
                    onEditCheck: async () => {
                        const all_ignored = this.state.edit_rule_config.soc_cond      == RuleCondition.Ignore &&
                                            this.state.edit_rule_config.price_cond    == RuleCondition.Ignore &&
                                            this.state.edit_rule_config.forecast_cond == RuleCondition.Ignore &&
                                            this.state.edit_rule_config.schedule_cond == ScheduleRuleCondition.Ignore &&
                                            this.state.edit_rule_config.fast_chg_cond == RuleCondition.Ignore;

                        return new Promise<boolean>((resolve) => {
                            this.setState({all_conditions_ignored: all_ignored}, () => resolve(!all_ignored));
                        });
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
                    schedule_cond: ScheduleRuleCondition.Ignore,
                    fast_chg_cond: ScheduleRuleCondition.Ignore,
                };

                this.setState({add_rule_config: rule_config, all_conditions_ignored: false});
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
                                    } else {
                                        this.setState({all_conditions_ignored: false});
                                    }

                                    this.setState({add_rule_config: {...this.state.add_rule_config, soc_cond: soc_cond, soc_th: soc_th}});
                                }}
                                value={this.state.add_rule_config.soc_cond.toString()} />
                        </InputNumber>
                    </FormRow>,
                    <FormRow label={__("batteries.content.add_rule_price")} label_muted={__("batteries.content.add_rule_price_muted")}>
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
                                    } else {
                                        this.setState({all_conditions_ignored: false});
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
                                    } else {
                                        this.setState({all_conditions_ignored: false});
                                    }

                                    this.setState({add_rule_config: {...this.state.add_rule_config, forecast_cond: forecast_cond, forecast_th: forecast_th}});
                                }}
                                value={this.state.add_rule_config.forecast_cond.toString()} />
                        </InputNumber>
                    </FormRow>,
                    <FormRow label={__("batteries.content.add_rule_schedule")}>
                        <InputSelect
                            placeholder={__("select")}
                            items={[
                                [ScheduleRuleCondition.Ignore.toString(),       __("batteries.content.condition_ignore")],
                                [ScheduleRuleCondition.Cheap.toString(),        __("batteries.content.condition_schedule_cheap")],
                                [ScheduleRuleCondition.NotCheap.toString(),     __("batteries.content.condition_schedule_not_cheap")],
                                [ScheduleRuleCondition.Expensive.toString(),    __("batteries.content.condition_schedule_expensive")],
                                [ScheduleRuleCondition.NotExpensive.toString(), __("batteries.content.condition_schedule_not_expensive")],
                                [ScheduleRuleCondition.Moderate.toString(),     __("batteries.content.condition_schedule_moderate")],
                            ]}
                            onValue={(v) => {
                                const schedule_cond = parseInt(v);

                                if (schedule_cond != ScheduleRuleCondition.Ignore) {
                                    this.setState({all_conditions_ignored: false});
                                }

                                this.setState({add_rule_config: {...this.state.add_rule_config, schedule_cond: schedule_cond}});
                            }}
                            value={this.state.add_rule_config.schedule_cond.toString()} />
                    </FormRow>,
                    <FormRow label={__("batteries.content.add_rule_fast_chg")}>
                        <InputSelect
                            placeholder={__("select")}
                            items={[
                                [RuleCondition.Ignore.toString(), __("batteries.content.condition_ignore")],
                                [RuleCondition.Below.toString(),  __("batteries.content.condition_fast_chg_inactive")],
                                [RuleCondition.Above.toString(),  __("batteries.content.condition_fast_chg_active")],
                            ]}
                            onValue={(v) => {
                                const fast_chg_cond = parseInt(v);

                                if (fast_chg_cond != RuleCondition.Ignore) {
                                    this.setState({all_conditions_ignored: false});
                                }

                                this.setState({add_rule_config: {...this.state.add_rule_config, fast_chg_cond: fast_chg_cond}});
                            }}
                            value={this.state.add_rule_config.fast_chg_cond.toString()} />
                    </FormRow>,
                    <Collapse in={this.state.all_conditions_ignored}>
                        <div>
                            <Alert variant="danger">
                                {__("batteries.content.invalid_feedback_all_ignored")}
                            </Alert>
                        </div>
                    </Collapse>,
                ];
            }}
            onAddCheck={async () => {
                const all_ignored = this.state.add_rule_config.soc_cond      == RuleCondition.Ignore &&
                                    this.state.add_rule_config.price_cond    == RuleCondition.Ignore &&
                                    this.state.add_rule_config.forecast_cond == RuleCondition.Ignore &&
                                    this.state.add_rule_config.schedule_cond == ScheduleRuleCondition.Ignore &&
                                    this.state.add_rule_config.fast_chg_cond == RuleCondition.Ignore;

                return new Promise<boolean>((resolve) => {
                    this.setState({all_conditions_ignored: all_ignored}, () => resolve(!all_ignored));
                });
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
//#endif

interface BatteriesState {
    configs: {[battery_slot: number]: BatteryConfig};
    add_battery_slot: number;
    add_battery_config: BatteryConfig;
    edit_battery_slot: number;
    edit_battery_config: BatteryConfig;
//#if MODULE_BATTERY_CONTROL_AVAILABLE
    battery_control_config: API.getType['battery_control/config'];
    rules_permit_grid_charge: RuleConfig[];
    rules_forbid_discharge: RuleConfig[];
    rules_forbid_charge: RuleConfig[];
//#endif
}

export class Batteries extends ConfigComponent<'batteries/config', {}, BatteriesState> {
    uplot_loader_ref  = createRef();
    uplot_wrapper_ref = createRef();

    constructor() {
        super('batteries/config',
              () => __("batteries.script.save_failed"),
              () => __("batteries.script.reboot_content_changed"), {
                  configs: {},
                  add_battery_slot: null,
                  add_battery_config: [BatteryClassID.None, null],
                  edit_battery_slot: null,
                  edit_battery_config: [BatteryClassID.None, null],
//#if MODULE_BATTERY_CONTROL_AVAILABLE
                  battery_control_config: null,
                  rules_permit_grid_charge: null,
                  rules_forbid_discharge: null,
                  rules_forbid_charge: null,
//#endif
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

//#if MODULE_BATTERY_CONTROL_AVAILABLE
        util.addApiEventListener('battery_control/config', (ev) => {
            if (!this.isDirty()) {
                this.setState({battery_control_config: ev.data});
            }
        });

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

//#if MODULE_DAY_AHEAD_PRICES_AVAILABLE
        util.addApiEventListener("day_ahead_prices/prices", () => {
            // Update chart every time new price data comes in
            this.update_uplot();
        });

        // Update vertical "now" line on time change
        effect(() => this.update_uplot());
//#endif
//#endif
    }

    override async sendSave(topic: 'batteries/config', new_config: API.getType['batteries/config']) {
        for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
            await API.save_unchecked(
                `batteries/${battery_slot}/config`,
                this.state.configs[battery_slot],
                () => __("batteries.script.save_failed"));
        }

//#if MODULE_BATTERY_CONTROL_AVAILABLE
        await API.save_unchecked('battery_control/config',
                       this.state.battery_control_config,
                       () => __("batteries.script.save_failed"));

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
//#endif

        await super.sendSave(topic, new_config);
    }

    override async sendReset(topic: 'batteries/config') {
        for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
            await API.reset_unchecked(`batteries/${battery_slot}/config`, this.error_string);
        }

//#if MODULE_BATTERY_CONTROL_AVAILABLE
        await API.reset('battery_control/config');
        await API.reset('battery_control/rules_permit_grid_charge', this.error_string);
        await API.reset('battery_control/rules_forbid_discharge', this.error_string);
        await API.reset('battery_control/rules_forbid_charge', this.error_string);
//#endif

        await super.sendReset(topic);
    }

    override getIsModified(topic: 'batteries/config'): boolean {
        for (let battery_slot = 0; battery_slot < options.BATTERIES_MAX_SLOTS; ++battery_slot) {
            if (API.is_modified_unchecked(`batteries/${battery_slot}/config`)) {
                return true;
            }
        }

//#if MODULE_BATTERY_CONTROL_AVAILABLE
        if (API.is_modified('battery_control/config')) {
            return true;
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
//#endif

        return super.getIsModified(topic);
    }

//#if MODULE_BATTERY_CONTROL_AVAILABLE
//#if MODULE_DAY_AHEAD_PRICES_AVAILABLE
    date_with_day_offset(date: Date, day_offset: number) {
        let date_copy = new Date(date);
        date_copy.setDate(date_copy.getDate() + day_offset);
        return date_copy;
    }

    clamp(value: number, min: number, max: number) {
        if (value < min) {
            return min;
        }
        if (value > max) {
            return max;
        }
        return value;
    }

    draw_selected_bars(uplot_data: UplotData, data_first_date_min: number, date_start: Date, date_end: Date, quarters: number, cheap: boolean, color: [number, number, number, number]) {
        const start_index = this.clamp((date_start.valueOf() / (60 * 1000) - data_first_date_min) / 15, 0, uplot_data.values[1].length - 1); // Subtract duplicated value at the end.
        const end_index   = this.clamp((date_end.valueOf()   / (60 * 1000) - data_first_date_min) / 15, 0, uplot_data.values[1].length - 1);

        if (start_index >= end_index) {
            return;
        }

        const price_index_pairs = uplot_data.values[1].slice(start_index, end_index).map((price, index) => ({ price, index }));
        const sorted_pairs      = cheap ? price_index_pairs.sort((a, b) => a.price - b.price) : price_index_pairs.sort((a, b) => b.price - a.price);
        const selected_indices  = sorted_pairs.slice(0, quarters).map(item => item.index + start_index);

        selected_indices.forEach(index => {
            uplot_data.lines_vertical.push({'index': index, 'text': '', 'color': color});
        });
    }

    draw_cheap_expensive_bars(uplot_data: UplotData, data_first_date_min: number, date_start: Date, date_end: Date, cheap_quarters: number, expensive_quarters: number) {
        this.draw_selected_bars(uplot_data, data_first_date_min, date_start, date_end, cheap_quarters,     true,  [40, 167, 69, 0.5]);
        this.draw_selected_bars(uplot_data, data_first_date_min, date_start, date_end, expensive_quarters, false, [220, 53, 69, 0.5]);
    }

    update_uplot() {
        // Use signal here to make effect() record its use, even
        // if this function might exit early on its first call
        const date_now = util.get_date_now_1m_update_rate();

        if (this.uplot_wrapper_ref.current == null) {
            return;
        }

        const dap_prices = API.get("day_ahead_prices/prices");
        let data: UplotData;

        if (dap_prices.prices.length == 0) {
            data = {
                keys: [null],
                names: [null],
                values: [null],
                stacked: [null],
                paths: [null],
            }
        } else {
            data = {
                keys: [null, 'price'],
                names: [null, __("day_ahead_prices.content.electricity_price")],
                values: [[], [], [], []],
                stacked: [null, true],
                paths: [null, UplotPath.Step],
                default_visibilty: [null, true],
                lines_vertical: []
            }

            let today_20h = new Date(date_now);
            today_20h.setHours(20, 0, 0, 0);

            const yesterday_20h = this.date_with_day_offset(today_20h, -1);
            const tomorrow_20h  = this.date_with_day_offset(today_20h,  1);

            const yesterday_20h_s = yesterday_20h.valueOf() / 1000;
            const today_20h_s     = today_20h.valueOf()     / 1000;
            const tomorrow_20h_s  = tomorrow_20h.valueOf()  / 1000;

            const resolution_multiplier = dap_prices.resolution == 0 ? 15 : 60
            const quarter_multiplier = resolution_multiplier / 15;

            const data_end_time = (dap_prices.first_date + dap_prices.prices.length * resolution_multiplier) * 60;
            const graph_end_time = data_end_time > tomorrow_20h_s ? tomorrow_20h_s : today_20h_s;
            let graph_start_time = undefined;
            let sample_time;
            let sample_percent;

            for (let i = 0; i < dap_prices.prices.length; i++) {
                sample_time = (dap_prices.first_date + i * resolution_multiplier) * 60;

                if (sample_time < yesterday_20h_s) {
                    continue;
                }

                if (graph_start_time === undefined) {
                    graph_start_time = sample_time;
                }

                if (sample_time >= graph_end_time) {
                    break;
                }

                sample_percent = get_price_from_index(i) / 1000.0;

                for (let j = 0; j < quarter_multiplier; j++) {
                    data.values[0].push(sample_time);
                    data.values[1].push(sample_percent);
                    sample_time += 15 * 60;
                }
            }

            // Duplicate final values to avoid cutting off the last bar.
            data.values[0].push(sample_time);
            data.values[1].push(sample_percent);

            this.draw_cheap_expensive_bars(data, graph_start_time / 60, yesterday_20h, today_20h,    this.state.battery_control_config.cheap_tariff_quarters, this.state.battery_control_config.expensive_tariff_quarters);
            this.draw_cheap_expensive_bars(data, graph_start_time / 60, today_20h,     tomorrow_20h, this.state.battery_control_config.cheap_tariff_quarters, this.state.battery_control_config.expensive_tariff_quarters);

            // Add vertical line at current time
            const resolution_divisor = 15;
            const diff = Math.trunc(date_now / 60000) - Math.trunc(graph_start_time / 60);
            const index = Math.trunc(diff / resolution_divisor);
            data.lines_vertical.push({'index': index, 'text': __("day_ahead_prices.content.now"), 'color': [64, 64, 64, 0.2]});
        }

        // Show loader or data depending on the availability of data
        this.uplot_loader_ref.current.set_data(data && data.keys.length > 1);
        this.uplot_wrapper_ref.current.set_data(data);
    }
//#endif
//#endif

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

//#if MODULE_BATTERY_CONTROL_AVAILABLE
        const bc_state = API.get("battery_control/state");
//#endif

        let active_battery_slots = Object.keys(this.state.configs).filter((battery_slot_str) => this.state.configs[parseInt(battery_slot_str)][0] != BatteryClassID.None);

        return (
            <SubPage name="batteries">
                <ConfigForm id="batteries_config_form" title={__("batteries.content.batteries")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
{/*#if MODULE_BATTERY_CONTROL_AVAILABLE*/}
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
{/*#endif*/}

                    <FormSeparator heading={__("batteries.content.managed_batteries")} />
                    <div class="form-group">
                        <FormRow label={__("batteries.content.enable_battery_control")}>
                            <Switch
                                checked={this.state.enabled}
                                onClick={this.toggle("enabled")}
                            />
                        </FormRow>

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

{/*#if MODULE_BATTERY_CONTROL_AVAILABLE*/}
{/*#if MODULE_DAY_AHEAD_PRICES_AVAILABLE*/}
                    <FormSeparator heading={__("batteries.content.dynamic_tariff_schedule")} />
                    <FormRow label={__("batteries.content.schedule_cheap_hours")} label_muted={__("batteries.content.schedule_hours_muted")} help={__("batteries.content.schedule_cheap_hours_help")}>
                        <InputFloat
                            unit="h"
                            value={this.state.battery_control_config.cheap_tariff_quarters * 100 / 4}
                            onValue={(v) => this.setState({battery_control_config: {...this.state.battery_control_config, cheap_tariff_quarters: Math.round(v * 4 / 100)}}, this.update_uplot)}
                            digits={2}
                            min={0}
                            max={2000 - this.state.battery_control_config.expensive_tariff_quarters * 100 / 4}
                        />
                    </FormRow>

                    <FormRow label={__("batteries.content.schedule_expensive_hours")} label_muted={__("batteries.content.schedule_hours_muted")} help={__("batteries.content.schedule_expensive_hours_help")}>
                        <InputFloat
                            unit="h"
                            value={this.state.battery_control_config.expensive_tariff_quarters * 100 / 4}
                            onValue={(v) => this.setState({battery_control_config: {...this.state.battery_control_config, expensive_tariff_quarters: Math.round(v * 4 / 100)}}, this.update_uplot)}
                            digits={2}
                            min={0}
                            max={2000 - this.state.battery_control_config.cheap_tariff_quarters * 100 / 4}
                        />
                    </FormRow>

                    <FormRow label={__("batteries.content.schedule_graph")} label_muted={__("batteries.content.schedule_graph_muted")}>
                        <div class="card">
                            <div style="position: relative;"> {/* this plain div is necessary to make the size calculation stable in safari. without this div the height continues to grow */}
                                <UplotLoader
                                    ref={this.uplot_loader_ref}
                                    show={true}
                                    marker_class={'h4'}
                                    no_data={__("day_ahead_prices.content.no_data")}
                                    loading={__("day_ahead_prices.content.loading")}>
                                    <UplotWrapperB
                                        ref={this.uplot_wrapper_ref}
                                        class="batteries-chart"
                                        sub_page="batteries"
                                        color_cache_group="batteries.default"
                                        show={true}
                                        on_mount={() => this.update_uplot()}
                                        legend_time_label={__("day_ahead_prices.content.time")}
                                        legend_time_with_minutes={true}
                                        aspect_ratio={3}
                                        x_format={{hour: '2-digit', minute: '2-digit'}}
                                        x_padding_factor={0}
                                        x_include_date={true}
                                        y_unit="ct/kWh"
                                        y_label={__("day_ahead_prices.content.price_ct_per_kwh")}
                                        y_digits={3}
                                        only_show_visible={true}
                                        padding={[30, 15, null, 5]}
                                    />
                                </UplotLoader>
                            </div>
                        </div>
                    </FormRow>
{/*#endif*/}

                    <FormSeparator heading={__("batteries.content.rules_permit_grid_charge")} />
                    <div class="form-group">
                        <RulesEditor rules={this.state.rules_permit_grid_charge} on_rules={(rules: RuleConfig[]) => {
                            this.setState({rules_permit_grid_charge: rules});
                            this.setDirty(true);
                        }} />
                    </div>

                    <FormSeparator heading={__("batteries.content.rules_forbid_discharge")} />
                    <div class="form-group">
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
{/*#endif*/}
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
