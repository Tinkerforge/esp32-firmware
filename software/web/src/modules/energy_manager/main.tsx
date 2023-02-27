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

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, render, Fragment, Component} from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent } from "src/ts/components/config_component";
import { FormRow } from "src/ts/components/form_row";
import { Switch } from "src/ts/components/switch";
import { InputSelect } from "src/ts/components/input_select";
import { InputFloat } from "src/ts/components/input_float";
import { InputNumber } from "src/ts/components/input_number";
import { InputTime } from "src/ts/components/input_time";
import { ConfigForm } from "src/ts/components/config_form";
import { FormSeparator } from "src/ts/components/form_separator";
import { Button, ButtonGroup, Collapse } from "react-bootstrap";

type StringStringTuple = [string, string];

interface DebugMode {
    debug_mode: boolean
}

interface EnergyManagerAllData {
    status: API.getType['energy_manager/status_state']
    config: API.getType['energy_manager/config']
    runtime_config: API.getType['energy_manager/runtime_config']
}

export class EnergyManagerStatus extends Component<{}, EnergyManagerAllData> {
    constructor() {
        super();

        util.eventTarget.addEventListener('energy_manager/status_state', () => {
            this.setState({status: API.get('energy_manager/status_state')});
        });

        util.eventTarget.addEventListener('energy_manager/config', () => {
            this.setState({config: API.get('energy_manager/config')});
        });

        util.eventTarget.addEventListener('energy_manager/runtime_config', () => {
            this.setState({runtime_config: API.get('energy_manager/runtime_config')});
        });
    }

    change_mode(mode: number) {
        API.save('energy_manager/runtime_config', {"mode": mode}, __("energy_manager.script.mode_change_failed"));
    }

    render(props: {}, d: Readonly<EnergyManagerAllData>) {
        if (!util.allow_render)
            return <></>;

        let error_flags_ok        = d.status.error_flags == 0;
        let error_flags_internal  = d.status.error_flags & 0xFF000000;
        let error_flags_contactor = d.status.error_flags & 0x00010000;
        let error_flags_network   = d.status.error_flags & 0x00000002;

        return <>
            <FormRow label={__("energy_manager.status.mode")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                <div class="input-group">
                    {d.config.excess_charging_enable ? <>
                        <Button
                            className="form-control mr-2 rounded-right"
                            variant={d.runtime_config.mode == 2 ? "primary" : "secondary"}
                            onClick={() => this.change_mode(2)}>
                            {__("energy_manager.status.mode_pv")}
                        </Button>
                        <Button
                            className="form-control mr-2 rounded-left rounded-right"
                            variant={d.runtime_config.mode == 3 ? "primary" : "secondary"}
                            onClick={() => this.change_mode(3)}>
                            {__("energy_manager.status.mode_min_pv")}
                        </Button>
                    </>: <></>}
                    <Button
                        className="form-control mr-2 rounded-left rounded-right"
                        variant={d.runtime_config.mode == 0 ? "primary" : "secondary"}
                        onClick={() => this.change_mode(0)}>
                        {__("energy_manager.status.mode_fast")}
                    </Button>
                    <Button
                        className="form-control rounded-left"
                        variant={d.runtime_config.mode == 1 ? "primary" : "secondary"}
                        onClick={() => this.change_mode(1)}>
                        {__("energy_manager.status.mode_off")}
                    </Button>
                </div>
            </FormRow>
            <FormRow label={__("energy_manager.status.status")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                <ButtonGroup className="flex-wrap w-100">
                    <Button disabled
                        key="13"
                        variant={(error_flags_ok ? "" : "outline-") + "success"}>
                        {__("energy_manager.status.error_ok")}
                    </Button>
                    <Button disabled
                        key="42"
                        variant={(error_flags_network ? "" : "outline-") + "warning"}>
                        {__("energy_manager.status.error_network")}
                    </Button>
                    <Button disabled
                        key="99"
                        variant={(error_flags_contactor ? "" : "outline-") + "danger"}>
                        {__("energy_manager.status.error_contactor")}
                    </Button>
                    <Button disabled
                        key="7"
                        variant={(error_flags_internal ? "" : "outline-") + "danger"}>
                        {__("energy_manager.status.error_internal")}
                    </Button>
                </ButtonGroup>
            </FormRow>
        </>
    }
}

render(<EnergyManagerStatus/>, $('#status-energy_manager')[0])

export class EnergyManager extends ConfigComponent<'energy_manager/config', {}, DebugMode> {
    constructor() {
        super('energy_manager/config',
            __("energy_manager.script.save_failed"),
            __("energy_manager.script.reboot_content_changed"));

        util.eventTarget.addEventListener('info/modules', () => {
            this.setState({debug_mode: !!((API.get('info/modules') as any).debug)})
        });
    }

    render(props: {}, s: Readonly<API.getType['energy_manager/config'] & DebugMode>) {
        if (!util.allow_render)
            return <></>

        let mode_list: StringStringTuple[] = [];
        if (s.excess_charging_enable) {
            mode_list.push(["2", __("energy_manager.status.mode_pv")]);
            mode_list.push(["3", __("energy_manager.status.mode_min_pv")]);
        }
        mode_list.push(["0", __("energy_manager.status.mode_fast")]);
        mode_list.push(["1", __("energy_manager.status.mode_off")]);

        let mode_list_for_inputs: StringStringTuple[] = [];
        for (let tuple of mode_list) {
            mode_list_for_inputs.push([tuple[0], __("energy_manager.content.input_switch_to") + " " + tuple[1]]);
        }
        mode_list_for_inputs.push(["255", __("energy_manager.content.input_mode_nothing")]);

        return (
            <>
                <ConfigForm id="energy_manager_config_form" title={__("energy_manager.content.page_header")} isModified={this.isModified()} onSave={() => this.save()} onReset={this.reset} onDirtyChange={(d) => this.ignore_updates = d}>

                    <FormRow label={__("energy_manager.content.default_mode")}>
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

                    <FormSeparator heading={__("energy_manager.content.header_load_management")} />
                    <FormRow label="">
                        <div class="pt-3 pb-4">
                            {__("energy_manager.content.load_management_explainer")}
                        </div>
                    </FormRow>

                    <FormSeparator heading={__("energy_manager.content.header_excess_charging")} />
                    <FormRow label={__("energy_manager.content.enable_excess_charging")}>
                        <Switch desc={__("energy_manager.content.enable_excess_charging_desc")}
                                checked={s.excess_charging_enable}
                                onClick={this.toggle('excess_charging_enable')}/>
                    </FormRow>

                    <FormRow label={__("energy_manager.content.guaranteed_power")} label_muted={__("energy_manager.content.guaranteed_power_muted")}>
                        <InputFloat
                            unit="kW"
                            value={s.guaranteed_power}
                            onValue={this.set('guaranteed_power')}
                            digits={3}
                            min={0}
                            max={22000}
                            />
                    </FormRow>

                    <FormRow label={__("energy_manager.content.contactor_installed")}>
                        <Switch desc={__("energy_manager.content.contactor_installed_desc")}
                                checked={s.contactor_installed}
                                onClick={() => this.setState({contactor_installed: !this.state.contactor_installed, input3_config: this.state.contactor_installed ? this.state.input3_config : 1})}/>
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
                            onValue={(v) => this.setState({phase_switching_mode: parseInt(v)})}/>
                    </FormRow>

                    <FormSeparator heading={__("energy_manager.content.relay")}/>

                    <FormRow label={__("energy_manager.content.relay_config")}>
                        <InputSelect
                            items={[
                                    ["0", __("energy_manager.content.relay_unused")],
                                    ["1", __("energy_manager.content.relay_rules")],
                                    ["2", __("energy_manager.content.extern")],
                                ]}
                            value={s.relay_config}
                            onValue={(v) => this.setState({relay_config: parseInt(v)})}/>
                    </FormRow>

                    <Collapse in={s.relay_config == 1}>
                        <div>
                            <FormRow label={__("energy_manager.content.relay_config_when")}>
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
                                    value={s.relay_config_when}
                                    onValue={(v) => this.setState({relay_config_when: parseInt(v)})}/>
                            </FormRow>

                            <FormRow label={__("energy_manager.content.relay_config_is")}>
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
                                                ["9", __("energy_manager.content.grid_ge0")],
                                                ["10", __("energy_manager.content.grid_le0")],
                                                ["11", __("energy_manager.content.grid_lt0")]
                                            ],
                                        }[s.relay_config_when] as [string, string][])
                                    }
                                    value={s.relay_config_is}
                                    onValue={(v) => this.setState({relay_config_is: parseInt(v)})}/>
                            </FormRow>

                            <FormRow label={__("energy_manager.content.relay_config_then")}>
                                <label class="form-control">{__("energy_manager.content.relay_config_close")}</label>
                            </FormRow>
                        </div>
                    </Collapse>

                    <FormSeparator heading={__("energy_manager.content.input3")}/>

                    <FormRow label={__("energy_manager.content.input3_config")}>
                        <InputSelect
                            required
                            items={s.contactor_installed ?
                                [["1", __("energy_manager.content.contactor_check")]] :
                                [
                                    ["0", __("energy_manager.content.input_unused")],
                                    ["2", __("energy_manager.content.block_charging")],
                                    ["3", __("energy_manager.content.limit_max_current")],
                                    ["4", __("energy_manager.content.input_switch_mode")],
                                ]
                            }
                            value={s.input3_config}
                            onValue={(v) => this.setState({input3_config: parseInt(v)})}
                            disabled={s.contactor_installed}/>
                    </FormRow>

                    <Collapse in={s.input3_config >= 2}>
                        <div>
                            <Collapse in={s.input3_config == 2 || s.input3_config == 3}>
                                <div>
                                    <FormRow label={__("energy_manager.content.input_when")}>
                                        <InputSelect
                                            required={s.input3_config == 2 || s.input3_config == 3}
                                            items={[
                                                ["0", __("energy_manager.content.input_high")],
                                                ["1", __("energy_manager.content.input_low")],
                                            ]}
                                            value={s.input3_config_when}
                                            onValue={(v) => this.setState({ input3_config_when: parseInt(v) })} />
                                    </FormRow>
                                </div>
                            </Collapse>

                            <Collapse in={s.input3_config == 3}>
                                <div>
                                    <FormRow label={__("energy_manager.content.limit_to_current")}>
                                    <InputNumber
                                        required={s.input3_config == 3}
                                        unit={"A"}
                                        value={s.input3_config_limit}
                                        onValue={this.set('input3_config_limit')}
                                        min={0}
                                        max={125}
                                        />
                                    </FormRow>
                                </div>
                            </Collapse>
                            <Collapse in={s.input3_config == 4}>
                                <div>
                                    <FormRow label={__("energy_manager.content.input_when_closing")}>
                                        <InputSelect
                                            required={s.input3_config == 4}
                                            items={mode_list_for_inputs}
                                            value={s.input3_config_rising_mode}
                                            onValue={(v) => this.setState({ input3_config_rising_mode: parseInt(v) })} />
                                    </FormRow>
                                    <FormRow label={__("energy_manager.content.input_when_opening")}>
                                        <InputSelect
                                            required={s.input3_config == 4}
                                            items={mode_list_for_inputs}
                                            value={s.input3_config_falling_mode}
                                            onValue={(v) => this.setState({ input3_config_falling_mode: parseInt(v) })} />
                                    </FormRow>
                                </div>
                            </Collapse>
                        </div>
                    </Collapse>

                    <FormSeparator heading={__("energy_manager.content.input4")}/>

                    <FormRow label={__("energy_manager.content.input4_config")}>
                        <InputSelect
                            required
                            items={[
                                    ["0", __("energy_manager.content.input_unused")],
                                    ["2", __("energy_manager.content.block_charging")],
                                    ["3", __("energy_manager.content.limit_max_current")],
                                    ["4", __("energy_manager.content.input_switch_mode")],
                                ]
                            }
                            value={s.input4_config}
                            onValue={(v) => this.setState({input4_config: parseInt(v)})}/>
                    </FormRow>

                    <Collapse in={s.input4_config >= 2}>
                        <div>
                            <Collapse in={s.input4_config == 2 || s.input4_config == 3}>
                                <div>
                                    <FormRow label={__("energy_manager.content.input_when")}>
                                        <InputSelect
                                            required={s.input4_config == 2 || s.input4_config == 3}
                                            items={[
                                                ["0", __("energy_manager.content.input_high")],
                                                ["1", __("energy_manager.content.input_low")],
                                            ]}
                                            value={s.input4_config_when}
                                            onValue={(v) => this.setState({ input4_config_when: parseInt(v) })} />
                                    </FormRow>
                                </div>
                            </Collapse>

                            <Collapse in={s.input4_config == 3}>
                                <div>
                                    <FormRow label={__("energy_manager.content.limit_to_current")}>
                                        <InputNumber
                                            required={s.input4_config == 3}
                                            unit={"A"}
                                            value={s.input4_config_limit}
                                            onValue={this.set('input4_config_limit')}
                                            min={0}
                                            max={125}
                                            />
                                    </FormRow>
                                </div>
                            </Collapse>
                            <Collapse in={s.input4_config == 4}>
                                <div>
                                    <FormRow label={__("energy_manager.content.input_when_closing")}>
                                        <InputSelect
                                            required={s.input4_config == 4}
                                            items={mode_list_for_inputs}
                                            value={s.input4_config_rising_mode}
                                            onValue={(v) => this.setState({ input4_config_rising_mode: parseInt(v) })} />
                                    </FormRow>
                                    <FormRow label={__("energy_manager.content.input_when_opening")}>
                                        <InputSelect
                                            required={s.input4_config == 4}
                                            items={mode_list_for_inputs}
                                            value={s.input4_config_falling_mode}
                                            onValue={(v) => this.setState({ input4_config_falling_mode: parseInt(v) })} />
                                    </FormRow>
                                </div>
                            </Collapse>
                        </div>
                    </Collapse>

                    {s.debug_mode ? <>
                        <FormSeparator heading={__("energy_manager.content.header_expert_settings")} />
                        <FormRow label={__("energy_manager.content.target_power_from_grid")} label_muted={__("energy_manager.content.target_power_from_grid_muted")}>
                            <InputFloat
                                unit="kW"
                                value={s.target_power_from_grid}
                                onValue={this.set('target_power_from_grid')}
                                digits={3}
                                min={0}
                                max={345000}
                            />
                        </FormRow>

                        <FormRow label={__("energy_manager.content.hysteresis_time")} label_muted={__("energy_manager.content.hysteresis_time_muted")}>
                            <InputNumber
                                unit="min"
                                value={s.hysteresis_time}
                                onValue={this.set('hysteresis_time')}
                                min={s.hysteresis_wear_accepted ? 0 : 10}
                                max={60}
                            />
                        </FormRow>

                        <FormRow label={__("energy_manager.content.hysteresis_wear_accepted")}>
                            <Switch desc={__("energy_manager.content.hysteresis_wear_accepted_desc")}
                                checked={s.hysteresis_wear_accepted}
                                onClick={() => { this.toggle('hysteresis_wear_accepted')(); if (s.hysteresis_wear_accepted && s.hysteresis_time < 10) this.setState({ hysteresis_time: 10 }); }} />
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
