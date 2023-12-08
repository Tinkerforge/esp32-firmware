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
import { METERS_SLOTS } from "../../build";

import { h, render, Fragment, Component } from "preact";
import { Button, ButtonGroup, Collapse } from "react-bootstrap";
import { CheckCircle, Circle } from "react-feather";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm      } from "../../ts/components/config_form";
import { FormRow         } from "../../ts/components/form_row";
import { FormSeparator   } from "../../ts/components/form_separator";
import { IndicatorGroup  } from "../../ts/components/indicator_group";
import { InputFloat      } from "../../ts/components/input_float";
import { InputNumber     } from "../../ts/components/input_number";
import { InputSelect     } from "../../ts/components/input_select";
import { Switch          } from "../../ts/components/switch";
import { SubPage         } from "../../ts/components/sub_page";
import { MeterConfig     } from "../meters/types";
import { MeterClassID    } from "../meters/meters_defs";

type StringStringTuple = [string, string];

export class EnergyManagerStatus extends Component {
    change_mode(mode: number) {
        API.save('energy_manager/charge_mode', {"mode": mode}, __("energy_manager.script.mode_change_failed"));
    }

    generate_config_error_label(generate: number, label: string) {
        if (generate == 0)
            return <></>

        return <FormRow label="" labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
            <IndicatorGroup
                value={0}
                items={[
                    ["danger", label],
                ]}
            />
        </FormRow>
    }

    generate_config_error_labels(config_error_flags: number) {
        if (config_error_flags == 0)
            return <></>

        return <>
            {this.generate_config_error_label(config_error_flags &  1, __("energy_manager.status.config_error_phase_switching"))}
            {this.generate_config_error_label(config_error_flags &  2, __("energy_manager.status.config_error_no_max_current"))}
            {this.generate_config_error_label(config_error_flags &  4, __("energy_manager.status.config_error_no_chargers"))}
            {this.generate_config_error_label(config_error_flags &  8, __("energy_manager.status.config_error_excess_no_meter"))}
        </>
    }

    render() {
        if (!util.render_allowed())
            return <></>

        if (!API.hasFeature("energy_manager")) {
            return <>
                <FormRow label={__("energy_manager.status.status")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                    <IndicatorGroup
                        value={0}
                        items={[
                            ["danger", __("energy_manager.status.no_bricklet")],
                        ]}
                    />
                </FormRow>
            </>
        }

        let status           = API.get('energy_manager/state');
        let config           = API.get('energy_manager/config');
        let charge_mode      = API.get('energy_manager/charge_mode');
        let external_control = API.get('energy_manager/external_control');

        let error_flags_ok        = status.error_flags == 0;
        let error_flags_config    = status.error_flags & 0x80000000;
        let error_flags_internal  = status.error_flags & 0x7F000000;
        let error_flags_contactor = status.error_flags & 0x00010000;
        let error_flags_network   = status.error_flags & 0x00000002;

        return <>
            <FormRow label={__("energy_manager.status.mode")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                <ButtonGroup className="flex-wrap m-n1" style="width: calc(100% + 0.5rem);">
                    <Button
                        style="display: flex;align-items: center;justify-content: center;"
                        className="m-1 rounded-left rounded-right"
                        variant={charge_mode.mode == 1 ? "success" : "primary"}
                        disabled={charge_mode.mode == 1}
                        onClick={() => this.change_mode(1)}>
                        {charge_mode.mode == 1 ? <CheckCircle size="20"/> : <Circle size="20"/>} <span>&nbsp;&nbsp;</span><span>{__("energy_manager.status.mode_off")}</span>
                    </Button>
                    <Button
                        style="display: flex;align-items: center;justify-content: center;"
                        className="m-1 rounded-left rounded-right"
                        variant={config.excess_charging_enable ? (charge_mode.mode == 2 ? "success" : "primary") : "secondary"}
                        disabled={!config.excess_charging_enable || charge_mode.mode == 2}
                        onClick={() => this.change_mode(2)}>
                        {!config.excess_charging_enable ? <Circle size="20"/> : (charge_mode.mode == 2 ? <CheckCircle size="20"/> : <Circle size="20"/>)} <span>&nbsp;&nbsp;</span><span>{__("energy_manager.status.mode_pv")}</span>
                    </Button>
                    <Button
                        style="display: flex;align-items: center;justify-content: center;"
                        className="m-1 rounded-left rounded-right"
                        variant={config.excess_charging_enable ? (charge_mode.mode == 3 ? "success" : "primary") : "secondary"}
                        disabled={!config.excess_charging_enable || charge_mode.mode == 3}
                        onClick={() => this.change_mode(3)}>
                        {!config.excess_charging_enable ? <Circle size="20"/> : (charge_mode.mode == 3 ? <CheckCircle size="20"/> : <Circle size="20"/>)} <span>&nbsp;&nbsp;</span><span>{__("energy_manager.status.mode_min_pv")}</span>
                    </Button>
                    <Button
                        style="display: flex;align-items: center;justify-content: center;"
                        className="m-1 rounded-left rounded-right"
                        variant={charge_mode.mode == 0 ? "success" : "primary"}
                        disabled={charge_mode.mode == 0}
                        onClick={() => this.change_mode(0)}>
                        {charge_mode.mode == 0 ? <CheckCircle size="20"/> : <Circle size="20"/>} <span>&nbsp;&nbsp;</span><span>{__("energy_manager.status.mode_fast")}</span>
                    </Button>
                </ButtonGroup>
            </FormRow>

            <FormRow label={__("energy_manager.status.phase_switching")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                <IndicatorGroup
                    value={status.phases_switched == 1 ? 0 : status.phases_switched == 3 ? 1 : 42}
                    items={[
                        ["primary", __("energy_manager.status.single_phase")],
                        ["primary", __("energy_manager.status.three_phase")],
                    ]} />
            </FormRow>

            {config.phase_switching_mode == 3 ?
                <>
                    <FormRow label={__("energy_manager.status.external_control_state")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                        <IndicatorGroup
                            value={status.external_control}
                            items={[
                                ["success", __("energy_manager.status.external_control_state_available")],
                                ["danger",  __("energy_manager.status.external_control_state_disabled")],
                                ["warning", __("energy_manager.status.external_control_state_unavailable")],
                                ["primary", __("energy_manager.status.external_control_state_switching")],
                            ]}
                        />
                    </FormRow>

                    <FormRow label={__("energy_manager.status.external_control_request")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                        <IndicatorGroup
                            value={external_control.phases_wanted > 1 ? external_control.phases_wanted - 1 : external_control.phases_wanted}
                            items={[
                                ["warning", __("energy_manager.status.external_control_request_none")],
                                ["primary", __("energy_manager.status.single_phase")],
                                ["primary", __("energy_manager.status.three_phase")],
                            ]}
                        />
                    </FormRow>
                </>
            :
                null
            }

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

            {this.generate_config_error_labels(status.config_error_flags)}
        </>
    }
}

render(<EnergyManagerStatus />, $("#status-energy_manager")[0]);

export class EnergyManager extends ConfigComponent<'energy_manager/config', {}, API.getType['energy_manager/debug_config'] & {meter_configs: {[meter_slot: number]: MeterConfig}}> {
    old_input4_rule_then = -1;

    // Need to use any here in case the cron module is not available.
    cron_config: any;

    constructor() {
        super('energy_manager/config',
            __("energy_manager.script.save_failed"),
            __("energy_manager.script.reboot_content_changed"));

        util.addApiEventListener('energy_manager/debug_config', () => {
            this.setState({...API.get('energy_manager/debug_config')});
        });

        util.addApiEventListener_unchecked('cron/config', () => {
            this.cron_config = API.get_unchecked('cron/config');
        });

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            util.addApiEventListener_unchecked(`meters/${meter_slot}/config`, () => {
                let meter_config = API.get_unchecked(`meters/${meter_slot}/config`);

                this.setState((prevState) => ({
                    meter_configs: {
                        ...prevState.meter_configs,
                        [meter_slot]: meter_config
                    }
                }));
            });
        }
    }

    override async sendSave(t: "energy_manager/config", cfg: API.getType['energy_manager/config']) {
        if (API.hasModule("debug")) {
            await API.save('energy_manager/debug_config', {
                    hysteresis_time: this.state.hysteresis_time,
                }, __("energy_manager.script.save_failed"));
        }
        console.log(API.hasModule("cron"))
        if (API.hasModule("cron")) {
            await API.save_unchecked('cron/config', this.cron_config, __("energy_manager.script.save_failed"));
        }
        await super.sendSave(t, cfg);
    }

    override async sendReset(t: "energy_manager/config") {
        if (API.hasModule("debug")) {
            await API.reset('energy_manager/debug_config', super.error_string, super.reboot_string);
        }

        await super.sendReset(t);
    }

    override getIsModified(t: "energy_manager/config"): boolean {
        if (API.hasModule("debug") && API.is_modified('energy_manager/debug_config')) {
            return true;
        }

        return super.getIsModified(t);
    }

    get_meter_name(meter_slot: number) {
        let meter_name = __("energy_manager.script.meter")(util.hasValue(meter_slot) ? meter_slot : '?');

        if (util.hasValue(meter_slot) && util.hasValue(this.state.meter_configs) && util.hasValue(this.state.meter_configs[meter_slot]) && util.hasValue(this.state.meter_configs[meter_slot][1])) {
            meter_name = this.state.meter_configs[meter_slot][1].display_name;
        }

        return meter_name;
    }

    disable_reset_switch() {
        if (!API.hasModule("cron") || !this.cron_config)
            return false;

            let disable = false
            let num_rules = 0;

            // We cant use the cron enums here because the cron Module is optional.
            // 12 is the ID for switching the charge mode and 1 the ID for the time trigger.
            this.cron_config.tasks.forEach((task: any) => {
                if (task.action[0] == 12) {
                    num_rules++;
                    if (task.trigger[0] != 1) {
                        disable = true;
                    } else {
                        let trigger = task.trigger[1];
                        if (trigger.hour != 0 || trigger.minute != 0 || trigger.mday != -1 || trigger.wday != -1) {
                            disable = true;
                        } else if (task.action[1].mode != 4) {
                            disable = true;
                        }
                    }
                }

                if (num_rules > 1) {
                    disable = true;
                }
            });
            return disable;
    }

    has_cron_reset_rule() {
        if (!API.hasModule("cron") || !this.cron_config)
            return false;

        let has_rule = false

        // We cant use the cron enums here because the cron Module is optional.
        // 12 is the ID for switching the charge mode.
        this.cron_config.tasks.forEach((task: any, idx: any) => {
            if (task.action[0] == 12) {
                has_rule = true;
            }
        });
        return has_rule;
    }

    render(props: {}, s: Readonly<API.getType['energy_manager/config'] & API.getType['energy_manager/debug_config'] & {meter_configs: {[meter_slot: number]: MeterConfig}}>) {
        if (!util.render_allowed() || !API.hasFeature("energy_manager"))
            return <></>

        let mode_list: StringStringTuple[] = [];

        mode_list.push(["1", __("energy_manager.status.mode_off")]);
        mode_list.push([s.excess_charging_enable ? "2" : "2-disabled", __("energy_manager.status.mode_pv")]);
        mode_list.push([s.excess_charging_enable ? "3" : "3-disabled", __("energy_manager.status.mode_min_pv")]);
        mode_list.push(["0", __("energy_manager.status.mode_fast")]);

        let mode_list_for_inputs: StringStringTuple[] = [];
        for (let tuple of mode_list) {
            mode_list_for_inputs.push([tuple[0], __("energy_manager.content.input_switch_to_prefix") + tuple[1] + __("energy_manager.content.input_switch_to_suffix")]);
        }
        mode_list_for_inputs.push(["255", __("energy_manager.content.input_mode_nothing")]);

        let meter_slots: StringStringTuple[] = [];
        for (let i = 0; i < METERS_SLOTS; i++) {
            if (s.meter_configs[i][0] != MeterClassID.None) {
                meter_slots.push([i.toString(), this.get_meter_name(i)]);
            }
        }

        const has_rule = this.has_cron_reset_rule();
        const disabled = this.disable_reset_switch();
        let reset = <></>;
        if (API.hasModule("cron")) {
            reset = <FormRow label={__("energy_manager.content.auto_reset_charging_mode")}>
                <Switch desc={disabled ? __("energy_manager.content.auto_reset_charging_mode_disabled") : __("energy_manager.content.auto_reset_charging_mode_desc")}
                    checked={has_rule && !disabled}
                    disabled={disabled}
                    onClick={() => {
                        if (!has_rule) {
                            // Need to create config by hand in case cron module is not available.
                            const action = [12, {mode: 4}];
                            const trigger = [1, {hour: 0, minute: 0, mday: -1, wday: -1}];
                            const task = {trigger: trigger, action: action};
                            this.cron_config.tasks.push(task as any);
                        } else {
                            this.cron_config.tasks = this.cron_config.tasks.filter((task: any) => task.action[0] != 12 && task.trigger[0] != 1);
                        }
                    }}
                />
            </FormRow>
        }

        let debug_mode = API.hasModule("debug");

        return (
            <SubPage>
                <ConfigForm id="energy_manager_config_form" title={__("energy_manager.content.page_header")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>

                    <FormSeparator heading={__("energy_manager.content.header_phase_switching")} first={true} />
                    <FormRow label={__("energy_manager.content.contactor_installed")}>
                        <Switch desc={__("energy_manager.content.contactor_installed_desc")}
                                checked={s.contactor_installed}
                                onClick={() => this.setState({contactor_installed: !this.state.contactor_installed})} // input4_rule_then setting inverted because it checks the not-yet-toggled state of contactor_installed.
                        />
                    </FormRow>

                    <FormRow label={__("energy_manager.content.phase_switching_mode")}>
                        <InputSelect
                            required
                            items={s.contactor_installed ? [
                                ["0", __("energy_manager.content.automatic")],
                                ["1", __("energy_manager.content.always_single_phase")],
                                ["2", __("energy_manager.content.always_three_phases")],
                                ["4", __("energy_manager.content.pv1p_fast3p")],
                                ["3", __("energy_manager.content.external_control")],
                            ] : [
                                ["1", __("energy_manager.content.fixed_single_phase")],
                                ["2", __("energy_manager.content.fixed_three_phases")],
                            ]}
                            value={s.phase_switching_mode}
                            onValue={(v) => {
                                this.setState({phase_switching_mode: parseInt(v)});
                                if (v == "2") {
                                    this.setState({guaranteed_power: Math.max(this.state.guaranteed_power, 230 * 6 * 3)});
                                } else if (this.state.guaranteed_power == (230 * 6 * 3)) {
                                    this.setState({guaranteed_power: Math.max(230 * 6, API.get("energy_manager/config").guaranteed_power)});
                                }
                                if (v == "3") {
                                    this.setState({
                                        excess_charging_enable: false,
                                        default_mode: 0,
                                    });
                                }
                            }}
                        />
                    </FormRow>

                    <Collapse in={s.phase_switching_mode == 3}>
                        <div>
                            <FormRow label="">
                                <div style="color:red">
                                    {__("energy_manager.content.external_control_notification")}
                                </div>
                            </FormRow>
                        </div>
                    </Collapse>

                    <FormSeparator heading={__("energy_manager.content.header_excess_charging")} />
                    <FormRow label={__("energy_manager.content.enable_excess_charging")} label_muted={__("energy_manager.content.enable_excess_charging_muted")}>
                        <Switch desc={__("energy_manager.content.enable_excess_charging_desc")}
                            checked={s.excess_charging_enable}
                            disabled={s.phase_switching_mode == 3}
                            onClick={this.toggle('excess_charging_enable')}/>
                    </FormRow>

                    <FormRow label={__("energy_manager.content.default_mode")} label_muted={__("energy_manager.content.default_mode_muted")}>
                        <InputSelect
                            required
                            items={mode_list}
                            value={s.default_mode}
                            onValue={s.phase_switching_mode == 3 ? undefined : (v) => this.setState({default_mode: parseInt(v)})}
                        />
                    </FormRow>

                    {reset}

                    <Collapse in={s.excess_charging_enable}>
                        <div>
                            <FormRow label={__("energy_manager.content.meter_slot_grid_power")} label_muted={__("energy_manager.content.meter_slot_grid_power_muted")}>
                                <InputSelect
                                    required
                                    placeholder={meter_slots.length > 0 ? __("energy_manager.content.meter_slot_grid_power_select") : __("energy_manager.content.meter_slot_grid_power_none")}
                                    items={meter_slots}
                                    value={s.meter_slot_grid_power}
                                    onValue={(v) => this.setState({meter_slot_grid_power: parseInt(v)})}
                                    disabled={meter_slots.length == 0}
                                />
                            </FormRow>

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

                            {debug_mode ?
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

                            <FormRow label={__("energy_manager.content.cloud_filter")} label_muted={__("energy_manager.content.cloud_filter_muted")}>
                                <InputSelect
                                    items={[
                                        ["0", __("energy_manager.content.cloud_filter_off")],
                                        ["1", __("energy_manager.content.cloud_filter_weak")],
                                        ["2", __("energy_manager.content.cloud_filter_medium")],
                                        ["3", __("energy_manager.content.cloud_filter_strong")],
                                    ]}
                                    value={s.cloud_filter_mode}
                                    onValue={(v) => this.setState({cloud_filter_mode: parseInt(v)})}
                                />
                            </FormRow>
                        </div>
                    </Collapse>

                    <FormSeparator heading={__("energy_manager.content.header_load_management")} />
                    <FormRow label="">
                        <div class="pt-3 pb-4">
                            {__("energy_manager.content.load_management_explainer")}
                        </div>
                    </FormRow>

                    {debug_mode ? <>
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
            </SubPage>
        );
    }
}

render(<EnergyManager />, $("#energy_manager")[0]);

export function init() {}

export function add_event_listeners(source: API.APIEventTarget) {}

export function update_sidebar_state(module_init: any) {
    $("#sidebar-energy_manager").prop("hidden", !module_init.energy_manager);
}
