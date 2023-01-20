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

import { h, render, Fragment } from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent } from "src/ts/components/config_component";
import { FormRow } from "src/ts/components/form_row";
import { IndicatorGroup } from "src/ts/components/indicator_group";
import { Switch } from "src/ts/components/switch";
import { InputSelect } from "src/ts/components/input_select";
import { InputFloat } from "src/ts/components/input_float";
import { InputNumber } from "src/ts/components/input_number";
import { ConfigForm } from "src/ts/components/config_form";
import { FormSeparator } from "src/ts/components/form_separator";
import { Button, Collapse } from "react-bootstrap";
import { InputText } from "src/ts/components/input_text";
import { CollapsedSection } from "src/ts/components/collapsed_section";

interface EnergyManagerState {
    state: API.getType['energy_manager/state'];
    debug_running: boolean;
    debug_status: string;
}

export class EnergyManager extends ConfigComponent<'energy_manager/config', {}, EnergyManagerState> {
    debug_log = "";

    constructor() {
        super('energy_manager/config',
            __("energy_manager.script.save_failed"),
            __("energy_manager.script.reboot_content_changed"));

        util.eventTarget.addEventListener('energy_manager/state', () => {
            this.setState({state: API.get('energy_manager/state')});
        });

        util.eventTarget.addEventListener("energy_manager/debug_header", (e) => {
            this.debug_log += e.data + "\n";
        }, false);

        util.eventTarget.addEventListener("energy_manager/debug", (e) => {
            this.debug_log += e.data + "\n";
        }, false);
    }

    async get_debug_report_and_event_log() {
        try {
            this.setState({debug_status: __("energy_manager.script.loading_debug_report")});
            this.debug_log += await util.download("/debug_report").then(blob => blob.text());
            this.debug_log += "\n\n";
        } catch (error) {
            this.setState({debug_running: false, debug_status: __("energy_manager.script.loading_debug_report_failed")});
            throw __("energy_manager.script.loading_debug_report_failed") + ": " + error;
        }

        try {
            this.setState({debug_status: __("energy_manager.script.loading_event_log")});
            this.debug_log += await util.download("/event_log").then(blob => blob.text());
            this.debug_log += "\n";
        } catch (error) {
            this.setState({debug_running: false, debug_status: __("energy_manager.script.loading_event_log_failed")});
            throw __("energy_manager.script.loading_event_log_failed") + ": " + error;
        }
    }

    async debug_start() {
        this.debug_log = "";
        this.setState({debug_running: true});

        try {
            await this.get_debug_report_and_event_log();

            this.setState({debug_status: __("energy_manager.script.starting_debug")});
        } catch(error) {
            this.setState({debug_running: false, debug_status: error});
            return;
        }

        try{
            await util.download("/energy_manager/start_debug");
        } catch {
            this.setState({debug_running: false, debug_status: __("energy_manager.script.starting_debug_failed")});
            return;
        }

        this.setState({debug_status: __("energy_manager.script.debug_running")});
    }

    async debug_stop() {
        this.setState({debug_running: false});

        try {
            await util.download("/energy_manager/stop_debug");
        } catch {
            this.setState({debug_running: true, debug_status: __("energy_manager.script.debug_stop_failed")});
        }

        try {
            this.debug_log += "\n\n";
            this.setState({debug_status: __("energy_manager.script.debug_stopped")});

            await this.get_debug_report_and_event_log();
            this.setState({debug_status: __("energy_manager.script.debug_done")});
        } catch (error) {
            this.debug_log += "\n\nError while stopping charge protocol: ";
            this.debug_log += error;

            this.setState({debug_status: error});
        }

        //Download log in any case: Even an incomplete log can be useful for debugging.
        util.downloadToFile(this.debug_log, "energy_manager-debug-log", "txt", "text/plain");
    }

    render(props: {}, s: Readonly<API.getType['energy_manager/config'] & EnergyManagerState>) {
        if (!s || !s.state)
            return (<></>);

        if (s.debug_running) {
            window.onbeforeunload = (e: Event) => {
                e.preventDefault();
                // returnValue is not a boolean, but the string to be shown
                // in the "are you sure you want to close this tab" message
                // box. However this string is only shown in some browsers.
                e.returnValue = __("energy_manager.script.tab_close_warning") as any;
            }
        } else {
            window.onbeforeunload = null;
        }

        const TODO_DEFINE_MAXIMUM = 100000;

        return (
            <>
                <ConfigForm id="energy_manager_config_form" title={__("energy_manager.content.energy_manager")} isModified={this.isModified()} onSave={() => this.save()} onReset={this.reset} onDirtyChange={(d) => this.ignore_updates = d}>
                    <FormRow label={__("energy_manager.content.enable_excess_charging")}>
                        <Switch desc={__("energy_manager.content.enable_excess_charging_desc")}
                                checked={s.excess_charging_enable}
                                onClick={this.toggle('excess_charging_enable')}/>
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

                    <FormRow label={__("energy_manager.content.target_power_from_grid")} label_muted={__("energy_manager.content.target_power_from_grid_muted")}>
                        <InputFloat
                            unit="kW"
                            value={s.target_power_from_grid}
                            onValue={this.set('target_power_from_grid')}
                            digits={3}
                            min={0}
                            max={TODO_DEFINE_MAXIMUM}
                            />
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

                    <FormRow label={__("energy_manager.content.maximum_available_current")} label_muted={__("energy_manager.content.maximum_available_current_muted")}>
                        <InputFloat
                            unit="A"
                            value={s.maximum_available_current}
                            onValue={this.set('maximum_available_current')}
                            digits={3}
                            min={0}
                            max={TODO_DEFINE_MAXIMUM}
                            />
                    </FormRow>

                    <FormRow label={__("energy_manager.content.minimum_current")} label_muted={__("energy_manager.content.minimum_current_muted")}>
                        <InputFloat
                            unit="A"
                            value={s.minimum_current}
                            onValue={this.set('minimum_current')}
                            digits={3}
                            min={6000}
                            max={32000}
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
                                onClick={() => {this.toggle('hysteresis_wear_accepted')(); if (s.hysteresis_wear_accepted && s.hysteresis_time < 10) this.setState({hysteresis_time: 10});}}/>
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
                                    ["3", __("energy_manager.content.switch_excess_charging")],
                                    ["4", __("energy_manager.content.limit_max_current")],
                                    ["5", __("energy_manager.content.override_grid_draw")],
                                ]
                            }
                            value={s.input3_config}
                            onValue={(v) => this.setState({input3_config: parseInt(v)})}
                            disabled={s.contactor_installed}/>
                    </FormRow>

                    <Collapse in={s.input3_config >= 2}>
                        <div>
                            <FormRow label={__("energy_manager.content.input_when")}>
                                <InputSelect
                                    required={s.input3_config >= 2}
                                    items={[
                                            ["0", __("energy_manager.content.input_high")],
                                            ["1", __("energy_manager.content.input_low")],
                                        ]}
                                    value={s.input3_config_when}
                                    onValue={(v) => this.setState({input3_config_when: parseInt(v)})}/>
                            </FormRow>

                            <Collapse in={s.input3_config >= 4}>
                                <div>
                                    <FormRow label={s.input3_config == 4 ? __("energy_manager.content.limit_to_current") : __("energy_manager.content.limit_grid_draw")}>
                                    <InputNumber
                                        required={s.input3_config >= 4}
                                        unit={s.input3_config == 4 ? "A" : "kW"}
                                        value={s.input3_config_limit}
                                        onValue={this.set('input3_config_limit')}
                                        min={0}
                                        max={TODO_DEFINE_MAXIMUM}
                                        />
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
                                    ["3", __("energy_manager.content.switch_excess_charging")],
                                    ["4", __("energy_manager.content.limit_max_current")],
                                    ["5", __("energy_manager.content.override_grid_draw")],
                                ]
                            }
                            value={s.input4_config}
                            onValue={(v) => this.setState({input4_config: parseInt(v)})}/>
                    </FormRow>

                    <Collapse in={s.input4_config >= 2}>
                        <div>
                            <FormRow label={__("energy_manager.content.input_when")}>
                                <InputSelect
                                    required={s.input4_config >= 2}
                                    items={[
                                            ["0", __("energy_manager.content.input_high")],
                                            ["1", __("energy_manager.content.input_low")],
                                        ]}
                                    value={s.input4_config_when}
                                    onValue={(v) => this.setState({input4_config_when: parseInt(v)})}/>
                            </FormRow>

                            <Collapse in={s.input4_config >= 4}>
                                <div>
                                    <FormRow label={s.input4_config == 4 ? __("energy_manager.content.limit_to_current") : __("energy_manager.content.limit_grid_draw")}>
                                        <InputNumber
                                            required={s.input4_config >= 4}
                                            unit={s.input4_config == 4 ? "A" : "kW"}
                                            value={s.input4_config_limit}
                                            onValue={this.set('input4_config_limit')}
                                            min={0}
                                            max={TODO_DEFINE_MAXIMUM}
                                            />
                                    </FormRow>
                                </div>
                            </Collapse>
                        </div>
                    </Collapse>

                    <FormSeparator heading={__("energy_manager.content.debug")}/>

                    <FormRow label={__("energy_manager.content.debug_description")} label_muted={__("energy_manager.content.debug_description_muted")}>
                        <div class="input-group pb-2">
                            <Button variant="primary" className="form-control rounded-right mr-2" onClick={() => {this.debug_start()}} disabled={s.debug_running}>{__("energy_manager.content.debug_start")}</Button>
                            <Button variant="primary" className="form-control rounded-left" onClick={() => {this.debug_stop()}} disabled={!s.debug_running}>{__("energy_manager.content.debug_stop")}</Button>
                        </div>
                        <InputText value={s.debug_status}/>
                    </FormRow>

                    <CollapsedSection label={__("energy_manager.content.low_level_state")}>
                        <FormRow label={__("energy_manager.content.state_contactor")}>
                            <InputText value={s.state.contactor ? __('energy_manager.content.three_phases_active') : __('energy_manager.content.one_phase_active')}/>
                        </FormRow>

                        <FormRow label={__("energy_manager.content.state_led")} label_muted={__("energy_manager.content.state_led_names")}>
                            <div class="row mx-n1">
                                {s.state.led_rgb.map((x, i) => (
                                    <div key={i} class="mb-1 col-4 px-1">
                                        <InputText value={x}/>
                                    </div>
                                ))}
                            </div>
                        </FormRow>

                        <FormRow label={__("energy_manager.content.gpios")} label_muted={__("energy_manager.content.gpio_names_0")}>
                            <div class="row mx-n1">
                                {[...s.state.gpio_input_state, s.state.gpio_output_state].map((x, j) => (
                                    <IndicatorGroup vertical key={j} class="mb-1 col px-1"
                                        value={x ? 0 : 1} //intentionally inverted: the high button is the first
                                        items={[
                                            ["primary", __("energy_manager.content.gpio_high")],
                                            ["secondary",   __("energy_manager.content.gpio_low")]
                                        ]}/>
                                ))}
                            </div>
                        </FormRow>

                        <FormRow label={__("energy_manager.content.state_input_voltage")}>
                            <InputFloat value={s.state.input_voltage} digits={3} unit={'V'} />
                        </FormRow>

                        <FormRow label={__("energy_manager.content.state_contactor_check")}>
                            <InputNumber value={s.state.contactor_check_state} />
                        </FormRow>
                    </CollapsedSection>
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
