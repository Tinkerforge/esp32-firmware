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

import $ from "../../ts/jq";

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { __ } from "../../ts/translation";

import { h, render, Fragment, Component } from "preact";
import { Button, ButtonGroup, Collapse  } from "react-bootstrap";
import { CheckCircle, Circle            } from "react-feather";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm      } from "../../ts/components/config_form";
import { FormRow         } from "../../ts/components/form_row";
import { FormSeparator   } from "../../ts/components/form_separator";
import { IndicatorGroup  } from "../../ts/components/indicator_group";
import { InputFloat      } from "../../ts/components/input_float";
import { InputNumber     } from "../../ts/components/input_number";
import { InputSelect     } from "../../ts/components/input_select";
import { InputTime       } from "../../ts/components/input_time";
import { Switch          } from "../../ts/components/switch";

type StringStringTuple = [string, string];

interface EnergyManagerAllData {
    status: API.getType['energy_manager/state']
    config: API.getType['energy_manager/config']
    charge_mode: API.getType['energy_manager/charge_mode']
}

export class EnergyManagerStatus extends Component<{}, EnergyManagerAllData> {
    constructor() {
        super();

        util.addApiEventListener('energy_manager/state', () => {
            this.setState({status: API.get('energy_manager/state')});
        });

        util.addApiEventListener('energy_manager/config', () => {
            this.setState({config: API.get('energy_manager/config')});
        });

        util.addApiEventListener('energy_manager/charge_mode', () => {
            this.setState({charge_mode: API.get('energy_manager/charge_mode')});
        });
    }

    change_mode(mode: number) {
        API.save('energy_manager/charge_mode', {"mode": mode}, __("energy_manager.script.mode_change_failed"));
    }

    render(props: {}, d: Readonly<EnergyManagerAllData>) {
        if (!util.allow_render) {
            return <></>;
        }

        if (!API.get("info/modules").energy_manager) {
            return <>
                <FormRow label={__("energy_manager.status.status")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                    <IndicatorGroup
                        value={0}
                        items={[
                            ["danger", __("energy_manager.status.no_bricklet")],
                        ]}
                    />
                </FormRow>
            </>;
        }

        let error_flags_ok        = d.status.error_flags == 0;
        let error_flags_config    = d.status.error_flags & 0x80000000;
        let error_flags_internal  = d.status.error_flags & 0x7F000000;
        let error_flags_contactor = d.status.error_flags & 0x00010000;
        let error_flags_network   = d.status.error_flags & 0x00000002;

        return <>
            <FormRow label={__("energy_manager.status.mode")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                <ButtonGroup className="flex-wrap m-n1" style="width: calc(100% + 0.5rem);">
                    <Button
                        style="display: flex;align-items: center;justify-content: center;"
                        className="m-1 rounded-left rounded-right"
                        variant={d.charge_mode.mode == 1 ? "success" : "primary"}
                        disabled={d.charge_mode.mode == 1}
                        onClick={() => this.change_mode(1)}>
                        {d.charge_mode.mode == 1 ? <CheckCircle size="20"/> : <Circle size="20"/>} <span>&nbsp;&nbsp;</span><span>{__("energy_manager.status.mode_off")}</span>
                    </Button>
                    <Button
                        style="display: flex;align-items: center;justify-content: center;"
                        className="m-1 rounded-left rounded-right"
                        variant={d.config.excess_charging_enable ? (d.charge_mode.mode == 2 ? "success" : "primary") : "secondary"}
                        disabled={!d.config.excess_charging_enable || d.charge_mode.mode == 2}
                        onClick={() => this.change_mode(2)}>
                        {!d.config.excess_charging_enable ? <Circle size="20"/> : (d.charge_mode.mode == 2 ? <CheckCircle size="20"/> : <Circle size="20"/>)} <span>&nbsp;&nbsp;</span><span>{__("energy_manager.status.mode_pv")}</span>
                    </Button>
                    <Button
                        style="display: flex;align-items: center;justify-content: center;"
                        className="m-1 rounded-left rounded-right"
                        variant={d.config.excess_charging_enable ? (d.charge_mode.mode == 3 ? "success" : "primary") : "secondary"}
                        disabled={!d.config.excess_charging_enable || d.charge_mode.mode == 3}
                        onClick={() => this.change_mode(3)}>
                        {!d.config.excess_charging_enable ? <Circle size="20"/> : (d.charge_mode.mode == 3 ? <CheckCircle size="20"/> : <Circle size="20"/>)} <span>&nbsp;&nbsp;</span><span>{__("energy_manager.status.mode_min_pv")}</span>
                    </Button>
                    <Button
                        style="display: flex;align-items: center;justify-content: center;"
                        className="m-1 rounded-left rounded-right"
                        variant={d.charge_mode.mode == 0 ? "success" : "primary"}
                        disabled={d.charge_mode.mode == 0}
                        onClick={() => this.change_mode(0)}>
                        {d.charge_mode.mode == 0 ? <CheckCircle size="20"/> : <Circle size="20"/>} <span>&nbsp;&nbsp;</span><span>{__("energy_manager.status.mode_fast")}</span>
                    </Button>
                </ButtonGroup>
            </FormRow>
            <FormRow label={__("energy_manager.status.phase_switching")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                <IndicatorGroup
                    value={d.status.phases_switched == 1 ? 0 : d.status.phases_switched == 3 ? 1 : 42}
                    items={[
                        ["primary", __("energy_manager.status.one_phase")],
                        ["primary", __("energy_manager.status.three_phase")],
                    ]} />
            </FormRow>
            <FormRow label={__("energy_manager.status.status")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                <ButtonGroup className="flex-wrap w-100">
                    <Button disabled
                        variant={(error_flags_ok ? "" : "outline-") + "success"}>
                        {__("energy_manager.status.error_ok")}
                    </Button>
                    <Button disabled
                        variant={(error_flags_network ? "" : "outline-") + "warning"}>
                        {__("energy_manager.status.error_network")}
                    </Button>
                    <Button disabled
                        variant={(error_flags_contactor ? "" : "outline-") + "danger"}>
                        {__("energy_manager.status.error_contactor")}
                    </Button>
                    <Button disabled
                        variant={(error_flags_internal ? "" : "outline-") + "danger"}>
                        {__("energy_manager.status.error_internal")}
                    </Button>
                    <Button disabled
                        variant={(error_flags_config ? "" : "outline-") + "danger"}>
                        {__("energy_manager.status.error_config")}
                    </Button>
                </ButtonGroup>
            </FormRow>
        </>
    }
}

render(<EnergyManagerStatus/>, $('#status-energy_manager')[0])

interface DebugMode {
    debug_mode: boolean
}

export class EnergyManager extends ConfigComponent<'energy_manager/config', {}, DebugMode & API.getType['energy_manager/debug_config']> {
    old_input4_rule_then = -1;

    constructor() {
        super('energy_manager/config',
            __("energy_manager.script.save_failed"),
            __("energy_manager.script.reboot_content_changed"));

        util.addApiEventListener('info/modules', () => {
            this.setState({debug_mode: !!((API.get('info/modules') as any).debug)})
        });


        util.addApiEventListener('energy_manager/debug_config', () => {
            this.setState({...API.get('energy_manager/debug_config')});
        });
    }

    override async sendSave(t: "energy_manager/config", cfg: API.getType['energy_manager/config']) {
        if (this.state.debug_mode) {
            await API.save('energy_manager/debug_config', {
                    hysteresis_time: this.state.hysteresis_time,
                }, __("energy_manager.script.save_failed"));
        }
        await super.sendSave(t, cfg);
    }

    render(props: {}, s: Readonly<API.getType['energy_manager/config'] & DebugMode & API.getType['energy_manager/debug_config']>) {
        if (!util.allow_render || !API.get("info/modules").energy_manager)
            return <></>

        let mode_list: StringStringTuple[] = [];

        mode_list.push(["1", __("energy_manager.status.mode_off")]);
        mode_list.push([s.excess_charging_enable ? "2" : "disabled", __("energy_manager.status.mode_pv")]);
        mode_list.push([s.excess_charging_enable ? "3" : "disabled", __("energy_manager.status.mode_min_pv")]);
        mode_list.push(["0", __("energy_manager.status.mode_fast")]);

        let mode_list_for_inputs: StringStringTuple[] = [];
        for (let tuple of mode_list) {
            mode_list_for_inputs.push([tuple[0], __("energy_manager.content.input_switch_to_prefix") + tuple[1] + __("energy_manager.content.input_switch_to_suffix")]);
        }
        mode_list_for_inputs.push(["255", __("energy_manager.content.input_mode_nothing")]);

        // Remember previous input4_rule_then setting so that it can be restored after toggling the contactor installed setting multiple times.
        if (this.old_input4_rule_then < 0)
            this.old_input4_rule_then = this.state.input4_rule_then == 1 ? 0 : this.state.input4_rule_then;

        return (
            <>
                <ConfigForm id="energy_manager_config_form" title={__("energy_manager.content.page_header")} isModified={this.isModified()} onSave={() => this.save()} onReset={this.reset} onDirtyChange={(d) => this.ignore_updates = d}>

                    <FormRow label={__("energy_manager.content.default_mode")} label_muted={__("energy_manager.content.default_mode_muted")}>
                        <InputSelect
                            required
                            items={mode_list}
                            value={s.default_mode}
                            onValue={(v) => this.setState({default_mode: parseInt(v)})}/>
                    </FormRow>

                    <FormRow label={__("energy_manager.content.auto_reset_charging_mode")}>
                        <Switch desc={__("energy_manager.content.auto_reset_charging_mode_desc")}
                                checked={s.auto_reset_mode}
                                onClick={this.toggle('auto_reset_mode')}/>
                    </FormRow>

                    <Collapse in={s.auto_reset_mode}>
                        <div>
                            <FormRow label={__("energy_manager.content.auto_reset_time")}>
                                <InputTime
                                    value={[Math.floor(s.auto_reset_time / 60), s.auto_reset_time % 60]}
                                    onValue={(h, m) => this.setState({auto_reset_time: h * 60 + m})} />
                            </FormRow>
                        </div>
                    </Collapse>

                    <FormSeparator heading={__("energy_manager.content.header_excess_charging")} />
                    <FormRow label={__("energy_manager.content.enable_excess_charging")} label_muted={__("energy_manager.content.enable_excess_charging_muted")}>
                        <Switch desc={__("energy_manager.content.enable_excess_charging_desc")}
                                checked={s.excess_charging_enable}
                                onClick={this.toggle('excess_charging_enable')}/>
                    </FormRow>

                    <Collapse in={s.excess_charging_enable}>
                        <div>
                            <FormRow label={__("energy_manager.content.guaranteed_power")} label_muted={__("energy_manager.content.guaranteed_power_muted")}>
                                <InputFloat
                                    unit="kW"
                                    value={s.guaranteed_power}
                                    onValue={this.set('guaranteed_power')}
                                    digits={3}
                                    min={230 * 6 * (s.phase_switching_mode == 2 ? 3 : 1)}
                                    max={22000}
                                    showMinMax
                                />
                            </FormRow>

                            {s.debug_mode ?
                                <FormRow label={__("energy_manager.content.target_power_from_grid")} label_muted={__("energy_manager.content.target_power_from_grid_muted")}>
                                    <InputFloat
                                        unit="kW"
                                        value={s.target_power_from_grid}
                                        onValue={this.set('target_power_from_grid')}
                                        digits={3}
                                        min={-43470}
                                        max={345000}
                                    />
                                </FormRow>
                            : <>
                                <FormRow label={__("energy_manager.content.control_behavior")} label_muted={__("energy_manager.content.control_behavior_muted")}>
                                    <InputSelect
                                        items={[
                                            ["-200", __("energy_manager.content.target_power_n200")],
                                            ["-100", __("energy_manager.content.target_power_n100")],
                                            [ "-50", __("energy_manager.content.target_power_n50" )],
                                            [   "0", __("energy_manager.content.target_power_0"   )],
                                            [  "50", __("energy_manager.content.target_power_p50" )],
                                            [ "100", __("energy_manager.content.target_power_p100")],
                                            [ "200", __("energy_manager.content.target_power_p200")],
                                        ]}
                                        value={s.target_power_from_grid}
                                        onValue={(v) => this.setState({target_power_from_grid: parseInt(v)})}
                                    />
                                </FormRow>
                            </>}
                        </div>
                    </Collapse>

                    <FormRow label={__("energy_manager.content.contactor_installed")}>
                        <Switch desc={__("energy_manager.content.contactor_installed_desc")}
                                checked={s.contactor_installed}
                                onClick={() => this.setState({contactor_installed: !this.state.contactor_installed, input4_rule_then: this.state.contactor_installed ? this.old_input4_rule_then : 1})} // input4_rule_then setting inverted because it checks the not-yet-toggled state of contactor_installed.
                        />
                    </FormRow>

                    <FormRow label={__("energy_manager.content.phase_switching_mode")}>
                        <InputSelect
                            required
                            items={s.contactor_installed ? [
                                ["0", __("energy_manager.content.automatic")],
                                ["1", __("energy_manager.content.always_one_phase")],
                                ["2", __("energy_manager.content.always_three_phases")],
                            ] : [
                                ["1", __("energy_manager.content.fixed_one_phase")],
                                ["2", __("energy_manager.content.fixed_three_phases")],
                                ]
                            }
                            value={s.phase_switching_mode}
                            onValue={(v) => {
                                this.setState({phase_switching_mode: parseInt(v)});
                                if (v == "2") {
                                    this.setState({guaranteed_power: Math.max(this.state.guaranteed_power, 230 * 6 * 3)});
                                } else if (this.state.guaranteed_power == (230 * 6 * 3)) {
                                    this.setState({guaranteed_power: Math.max(230 * 6, API.get("energy_manager/config").guaranteed_power)});
                                }
                                }}/>
                    </FormRow>

                    <FormSeparator heading={__("energy_manager.content.header_load_management")} />
                    <FormRow label="">
                        <div class="pt-3 pb-4">
                            {__("energy_manager.content.load_management_explainer")}
                        </div>
                    </FormRow>

                    <FormSeparator heading={__("energy_manager.content.relay")}/>

                    <FormRow label={__("energy_manager.content.relay_config")}>
                        <InputSelect
                            items={[
                                    ["0", __("energy_manager.content.relay_manual")],
                                    ["1", __("energy_manager.content.relay_rules")],
                                ]}
                            value={s.relay_config}
                            onValue={(v) => this.setState({relay_config: parseInt(v)})}/>
                    </FormRow>

                    <Collapse in={s.relay_config == 1}>
                        <div>
                            <FormRow label={__("energy_manager.content.relay_rule_when")}>
                                <InputSelect
                                    required={s.relay_config == 1}
                                    items={[
                                            ["0", __("energy_manager.content.input3")],
                                            ["1", __("energy_manager.content.input4")],
                                            ["2", __("energy_manager.content.phase_switching")],
                                            ["3", __("energy_manager.content.contactor_check")],
                                            ["4", __("energy_manager.content.power_available")],
                                            ["5", __("energy_manager.content.grid_draw")],
                                        ]}
                                    value={s.relay_rule_when}
                                    onValue={(v) => this.setState({relay_rule_when: parseInt(v)})}/>
                            </FormRow>

                            <FormRow label={__("energy_manager.content.relay_rule_is")}>
                                <InputSelect
                                    required={s.relay_config == 1}
                                    items={({0: [
                                                ["0", __("energy_manager.content.high")],
                                                ["1", __("energy_manager.content.low")],
                                            ],
                                            1: [
                                                ["0", __("energy_manager.content.high")],
                                                ["1", __("energy_manager.content.low")],
                                            ],
                                            2: [
                                                ["2", __("energy_manager.content.one_phase")],
                                                ["3", __("energy_manager.content.three_phase")],
                                            ],
                                            3: [
                                                ["4", __("energy_manager.content.contactor_fail")],
                                                ["5", __("energy_manager.content.contactor_ok")]
                                            ],
                                            4: [
                                                ["6", __("energy_manager.content.power_sufficient")],
                                                ["7", __("energy_manager.content.power_insufficient")]
                                            ],
                                            5: [
                                                ["8", __("energy_manager.content.grid_gt0")],
                                                ["9", __("energy_manager.content.grid_le0")]
                                            ],
                                        }[s.relay_rule_when] as [string, string][])
                                    }
                                    value={s.relay_rule_is}
                                    onValue={(v) => this.setState({relay_rule_is: parseInt(v)})}/>
                            </FormRow>

                            <FormRow label={__("energy_manager.content.relay_config_then")}>
                                <label class="form-control">{__("energy_manager.content.relay_config_close")}</label>
                            </FormRow>
                        </div>
                    </Collapse>

                    <FormSeparator heading={__("energy_manager.content.input3")}/>

                    <FormRow label={__("energy_manager.content.input3_rule_then")}>
                        <InputSelect
                            required
                            items={[
                                ["0", __("energy_manager.content.input_unused")],
                                ["2", __("energy_manager.content.block_charging")],
                                ["3", __("energy_manager.content.limit_max_current")],
                                ["4", __("energy_manager.content.input_switch_mode")],
                            ]}
                            value={s.input3_rule_then}
                            onValue={(v) => this.setState({input3_rule_then: parseInt(v)})} />
                    </FormRow>

                    <Collapse in={s.input3_rule_then >= 2}>
                        <div>
                            <Collapse in={s.input3_rule_then == 2 || s.input3_rule_then == 3}>
                                <div>
                                    <FormRow label={__("energy_manager.content.input_when")}>
                                        <InputSelect
                                            required={s.input3_rule_then == 2 || s.input3_rule_then == 3}
                                            items={[
                                                ["0", __("energy_manager.content.input_high")],
                                                ["1", __("energy_manager.content.input_low")],
                                            ]}
                                            value={s.input3_rule_is}
                                            onValue={(v) => this.setState({ input3_rule_is: parseInt(v) })} />
                                    </FormRow>
                                </div>
                            </Collapse>

                            <Collapse in={s.input3_rule_then == 3}>
                                <div>
                                    <FormRow label={__("energy_manager.content.limit_to_current")}>
                                    <InputFloat
                                        //required={s.input3_rule_then == 3}
                                        digits={3}
                                        unit={"A"}
                                        value={s.input3_rule_then_limit}
                                        onValue={this.set('input3_rule_then_limit')}
                                        min={0}
                                        max={125000}
                                        />
                                    </FormRow>
                                </div>
                            </Collapse>
                            <Collapse in={s.input3_rule_then == 4}>
                                <div>
                                    <FormRow label={__("energy_manager.content.input_when_closing")}>
                                        <InputSelect
                                            required={s.input3_rule_then == 4}
                                            items={mode_list_for_inputs}
                                            value={s.input3_rule_then_on_high}
                                            onValue={(v) => this.setState({ input3_rule_then_on_high: parseInt(v) })} />
                                    </FormRow>
                                    <FormRow label={__("energy_manager.content.input_when_opening")}>
                                        <InputSelect
                                            required={s.input3_rule_then == 4}
                                            items={mode_list_for_inputs}
                                            value={s.input3_rule_then_on_low}
                                            onValue={(v) => this.setState({ input3_rule_then_on_low: parseInt(v) })} />
                                    </FormRow>
                                </div>
                            </Collapse>
                        </div>
                    </Collapse>

                    <FormSeparator heading={__("energy_manager.content.input4")}/>

                    <FormRow label={__("energy_manager.content.input4_rule_then")}>
                        <InputSelect
                            required
                            items={s.contactor_installed ? [
                                    ["1", __("energy_manager.content.contactor_check")]
                                ] : [
                                    ["0", __("energy_manager.content.input_unused")],
                                    ["2", __("energy_manager.content.block_charging")],
                                    ["3", __("energy_manager.content.limit_max_current")],
                                    ["4", __("energy_manager.content.input_switch_mode")],
                                ]
                            }
                            value={s.input4_rule_then}
                            onValue={(v) => this.setState({input4_rule_then: parseInt(v)})}
                            disabled={s.contactor_installed} />
                    </FormRow>

                    <Collapse in={s.input4_rule_then >= 2}>
                        <div>
                            <Collapse in={s.input4_rule_then == 2 || s.input4_rule_then == 3}>
                                <div>
                                    <FormRow label={__("energy_manager.content.input_when")}>
                                        <InputSelect
                                            required={s.input4_rule_then == 2 || s.input4_rule_then == 3}
                                            items={[
                                                ["0", __("energy_manager.content.input_high")],
                                                ["1", __("energy_manager.content.input_low")],
                                            ]}
                                            value={s.input4_rule_is}
                                            onValue={(v) => this.setState({ input4_rule_is: parseInt(v) })} />
                                    </FormRow>
                                </div>
                            </Collapse>

                            <Collapse in={s.input4_rule_then == 3}>
                                <div>
                                    <FormRow label={__("energy_manager.content.limit_to_current")}>
                                        <InputNumber
                                            required={s.input4_rule_then == 3}
                                            unit={"A"}
                                            value={s.input4_rule_then_limit}
                                            onValue={this.set('input4_rule_then_limit')}
                                            min={0}
                                            max={125}
                                            />
                                    </FormRow>
                                </div>
                            </Collapse>
                            <Collapse in={s.input4_rule_then == 4}>
                                <div>
                                    <FormRow label={__("energy_manager.content.input_when_closing")}>
                                        <InputSelect
                                            required={s.input4_rule_then == 4}
                                            items={mode_list_for_inputs}
                                            value={s.input4_rule_then_on_high}
                                            onValue={(v) => this.setState({ input4_rule_then_on_high: parseInt(v) })} />
                                    </FormRow>
                                    <FormRow label={__("energy_manager.content.input_when_opening")}>
                                        <InputSelect
                                            required={s.input4_rule_then == 4}
                                            items={mode_list_for_inputs}
                                            value={s.input4_rule_then_on_low}
                                            onValue={(v) => this.setState({ input4_rule_then_on_low: parseInt(v) })} />
                                    </FormRow>
                                </div>
                            </Collapse>
                        </div>
                    </Collapse>

                    {s.debug_mode ? <>
                        <FormSeparator heading={__("energy_manager.content.header_expert_settings")} />
                        <FormRow label={__("energy_manager.content.hysteresis_time")} label_muted={__("energy_manager.content.hysteresis_time_muted")}>
                            <InputNumber
                                unit="min"
                                value={s.hysteresis_time}
                                onValue={this.set('hysteresis_time')}
                                min={0}
                                max={60}
                            />
                        </FormRow>
                    </> : null }
                </ConfigForm>
            </>
        )
    }
}

render(<EnergyManager/>, $('#energy_manager')[0])

export function init() {}

export function add_event_listeners(source: API.APIEventTarget) {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-energy_manager').prop('hidden', !module_init.energy_manager);
}
