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

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { __, translate_unchecked } from "../../ts/translation";
import { METERS_SLOTS } from "../../build";
import { h, Fragment, Component, RefObject } from "preact";
import { Button, ButtonGroup, Collapse } from "react-bootstrap";
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
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { CheckCircle, Circle, List } from "react-feather";

export function PowerManagerNavbar() {
    return <NavbarItem name="power_manager" module="power_manager" title={__("power_manager.navbar.power_manager")} symbol={<List />} />;
}

type StringStringTuple = [string, string];

export class PowerManagerStatus extends Component {
    change_mode(mode: number) {
        API.save('power_manager/charge_mode', {"mode": mode}, __("power_manager.script.mode_change_failed"));
    }

    change_phase(phases: number) {
        API.save('power_manager/external_control', {"phases_wanted": phases}, __("power_manager.script.mode_change_failed"));
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
            {this.generate_config_error_label(config_error_flags &  1, __("power_manager.status.config_error_phase_switching"))}
            {this.generate_config_error_label(config_error_flags &  2, __("power_manager.status.config_error_no_max_current"))}
            {this.generate_config_error_label(config_error_flags &  4, __("power_manager.status.config_error_no_chargers"))}
            {this.generate_config_error_label(config_error_flags &  8, __("power_manager.status.config_error_excess_no_meter"))}
        </>
    }

    render() {
        if (!util.render_allowed())
            return <StatusSection name="power_manager" />

        let charge_mode = API.get('power_manager/charge_mode');
        let state       = API.get('power_manager/state');
        let ll_state    = API.get('power_manager/low_level_state');
        let config      = API.get('power_manager/config');

        return <StatusSection name="power_manager">
            <FormRow label={__("power_manager.status.mode")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                <ButtonGroup className="flex-wrap m-n1" style="width: calc(100% + 0.5rem);">
                    <Button
                        style="display: flex;align-items: center;justify-content: center;"
                        className="m-1 rounded-left rounded-right"
                        variant={charge_mode.mode == 1 ? "success" : "primary"}
                        disabled={charge_mode.mode == 1}
                        onClick={() => this.change_mode(1)}>
                        {charge_mode.mode == 1 ? <CheckCircle size="20"/> : <Circle size="20"/>} <span>&nbsp;&nbsp;</span><span>{__("power_manager.status.mode_off")}</span>
                    </Button>
                    <Button
                        style="display: flex;align-items: center;justify-content: center;"
                        className="m-1 rounded-left rounded-right"
                        variant={config.excess_charging_enable ? (charge_mode.mode == 2 ? "success" : "primary") : "secondary"}
                        disabled={!config.excess_charging_enable || charge_mode.mode == 2}
                        onClick={() => this.change_mode(2)}>
                        {!config.excess_charging_enable ? <Circle size="20"/> : (charge_mode.mode == 2 ? <CheckCircle size="20"/> : <Circle size="20"/>)} <span>&nbsp;&nbsp;</span><span>{__("power_manager.status.mode_pv")}</span>
                    </Button>
                    <Button
                        style="display: flex;align-items: center;justify-content: center;"
                        className="m-1 rounded-left rounded-right"
                        variant={config.excess_charging_enable ? (charge_mode.mode == 3 ? "success" : "primary") : "secondary"}
                        disabled={!config.excess_charging_enable || charge_mode.mode == 3}
                        onClick={() => this.change_mode(3)}>
                        {!config.excess_charging_enable ? <Circle size="20"/> : (charge_mode.mode == 3 ? <CheckCircle size="20"/> : <Circle size="20"/>)} <span>&nbsp;&nbsp;</span><span>{__("power_manager.status.mode_min_pv")}</span>
                    </Button>
                    <Button
                        style="display: flex;align-items: center;justify-content: center;"
                        className="m-1 rounded-left rounded-right"
                        variant={charge_mode.mode == 0 ? "success" : "primary"}
                        disabled={charge_mode.mode == 0}
                        onClick={() => this.change_mode(0)}>
                        {charge_mode.mode == 0 ? <CheckCircle size="20"/> : <Circle size="20"/>} <span>&nbsp;&nbsp;</span><span>{__("power_manager.status.mode_fast")}</span>
                    </Button>
                </ButtonGroup>
            </FormRow>


            {config.phase_switching_mode == 3 ?
                <>
                    <FormRow label={__("power_manager.status.phase_switching")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                        <ButtonGroup className="flex-wrap m-n1" style="width: calc(100% + 0.5rem);">
                            <Button
                                style="display: flex;align-items: center;justify-content: center;"
                                className="m-1 rounded-left rounded-right"
                                variant={!ll_state.is_3phase ? "success" : "primary"}
                                disabled={!ll_state.is_3phase || state.external_control != 0}
                                onClick={() => this.change_phase(1)}>
                                {!ll_state.is_3phase ? <CheckCircle size="20"/> : <Circle size="20"/>} <span>&nbsp;&nbsp;</span><span>{__("power_manager.status.single_phase")}</span>
                            </Button>
                            <Button
                                style="display: flex;align-items: center;justify-content: center;"
                                className="m-1 rounded-left rounded-right"
                                variant={ll_state.is_3phase ? "success" : "primary"}
                                disabled={ll_state.is_3phase || state.external_control != 0}
                                onClick={() => this.change_phase(3)}>
                                {ll_state.is_3phase ? <CheckCircle size="20"/> : <Circle size="20"/>} <span>&nbsp;&nbsp;</span><span>{__("power_manager.status.three_phase")}</span>
                            </Button>
                        </ButtonGroup>
                    </FormRow>

                    <FormRow label={__("power_manager.status.external_control_state")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                        <IndicatorGroup
                            value={state.external_control}
                            items={[
                                ["success", __("power_manager.status.external_control_state_available")],
                                ["danger",  __("power_manager.status.external_control_state_disabled")],
                                ["warning", __("power_manager.status.external_control_state_unavailable")],
                                ["primary", __("power_manager.status.external_control_state_switching")],
                            ]}
                        />
                    </FormRow>
                </>
            :
                <>
                    <FormRow label={__("power_manager.status.phase_switching")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                        <IndicatorGroup
                            value={!ll_state.is_3phase ? 0 : 1}
                            items={[
                                ["primary", __("power_manager.status.single_phase")],
                                ["primary", __("power_manager.status.three_phase")],
                            ]} />
                    </FormRow>
                </>
            }

            {this.generate_config_error_labels(state.config_error_flags)}
        </StatusSection>
    }
}

export class PowerManager extends ConfigComponent<'power_manager/config', {status_ref?: RefObject<PowerManagerStatus>}, {em_contactor_installed: boolean} & API.getType['power_manager/debug_config'] & {meter_configs: {[meter_slot: number]: MeterConfig}}> {
    // Need to use any here in case the automation module is not available.
    automation_config: any;

    constructor() {
        super('power_manager/config',
            __("power_manager.script.save_failed"),
            __("power_manager.script.reboot_content_changed"));

        this.setState({em_contactor_installed: false});

        util.addApiEventListener_unchecked('energy_manager/config', () => {
            this.setState({em_contactor_installed: API.get_unchecked('energy_manager/config')?.contactor_installed});
        });

        util.addApiEventListener('power_manager/debug_config', () => {
            this.setState({...API.get('power_manager/debug_config')});
        });

        util.addApiEventListener_unchecked('automation/config', () => {
            this.automation_config = API.get_unchecked('automation/config');
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

    override async sendSave(t: "power_manager/config", cfg: API.getType['power_manager/config']) {
        if (API.hasModule("energy_manager")) {
            await API.save_unchecked('energy_manager/config', {
                contactor_installed: this.state.em_contactor_installed,
            }, __("power_manager.script.save_failed"));
        }
        if (API.hasModule("debug")) {
            await API.save('power_manager/debug_config', {
                    hysteresis_time: this.state.hysteresis_time,
                }, __("power_manager.script.save_failed"));
        }
        if (API.hasModule("automation")) {
            await API.save_unchecked('automation/config', this.automation_config, __("power_manager.script.save_failed"));
        }
        await super.sendSave(t, cfg);
    }

    override async sendReset(t: "power_manager/config") {
        if (API.hasModule("energy_manager")) {
            await API.reset_unchecked('energy_manager/config', super.error_string, super.reboot_string);
        }
        if (API.hasModule("debug")) {
            await API.reset('power_manager/debug_config', super.error_string, super.reboot_string);
        }

        await super.sendReset(t);
    }

    override getIsModified(t: "power_manager/config"): boolean {
        if (API.hasModule("energy_manager") && API.is_modified_unchecked('energy_manager/config')) {
            return true;
        }

        if (API.hasModule("debug") && API.is_modified('power_manager/debug_config')) {
            return true;
        }

        return super.getIsModified(t);
    }

    get_meter_name(meter_slot: number) {
        let meter_name = __("power_manager.script.meter")(util.hasValue(meter_slot) ? meter_slot : '?');

        if (util.hasValue(meter_slot) && util.hasValue(this.state.meter_configs) && util.hasValue(this.state.meter_configs[meter_slot]) && util.hasValue(this.state.meter_configs[meter_slot][1])) {
            meter_name = this.state.meter_configs[meter_slot][1].display_name;
        }

        return meter_name;
    }

    disable_reset_switch() {
        if (!API.hasModule("automation") || !this.automation_config)
            return false;

            let disable = false
            let num_rules = 0;

            // We cant use the automation enums here because the automation Module is optional.
            // 12 is the ID for switching the charge mode and 1 the ID for the time trigger.
            this.automation_config.tasks.forEach((task: any) => {
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

    has_automation_reset_rule() {
        if (!API.hasModule("automation") || !this.automation_config)
            return false;

        let has_rule = false

        // We cant use the automation enums here because the automation Module is optional.
        // 12 is the ID for switching the charge mode.
        this.automation_config.tasks.forEach((task: any, idx: any) => {
            if (task.action[0] == 12) {
                has_rule = true;
            }
        });
        return has_rule;
    }

    render(props: {}, s: Readonly<API.getType['power_manager/config'] & API.getType['power_manager/debug_config'] & {meter_configs: {[meter_slot: number]: MeterConfig}}>) {
        if (!util.render_allowed())
            return <SubPage name="power_manager" />;

        let mode_list: StringStringTuple[] = [];

        mode_list.push(["1", __("power_manager.status.mode_off")]);
        mode_list.push([s.excess_charging_enable ? "2" : "2-disabled", __("power_manager.status.mode_pv")]);
        mode_list.push([s.excess_charging_enable ? "3" : "3-disabled", __("power_manager.status.mode_min_pv")]);
        mode_list.push(["0", __("power_manager.status.mode_fast")]);

        let meter_slots: StringStringTuple[] = [];
        for (let i = 0; i < METERS_SLOTS; i++) {
            if (s.meter_configs[i][0] != MeterClassID.None) {
                meter_slots.push([i.toString(), this.get_meter_name(i)]);
            }
        }

        const has_rule = this.has_automation_reset_rule();
        const disabled = this.disable_reset_switch();
        let reset = <></>;
        if (API.hasModule("automation")) {
            reset = <FormRow label={__("power_manager.content.auto_reset_charging_mode")}>
                <Switch desc={disabled ? __("power_manager.content.auto_reset_charging_mode_disabled") : __("power_manager.content.auto_reset_charging_mode_desc")}
                    checked={has_rule && !disabled}
                    disabled={disabled}
                    onClick={() => {
                        if (!has_rule) {
                            // Need to create config by hand in case automation module is not available.
                            const action = [12, {mode: 4}];
                            const trigger = [1, {hour: 0, minute: 0, mday: -1, wday: -1}];
                            const task = {trigger: trigger, action: action};
                            this.automation_config.tasks.push(task as any);
                        } else {
                            this.automation_config.tasks = this.automation_config.tasks.filter((task: any) => task.action[0] != 12 && task.trigger[0] != 1);
                        }
                    }}
                />
            </FormRow>
        }

        let show_enable = !API.hasFeature("energy_manager") || !s.enabled;

        let cm_config = API.get_unchecked("charge_manager/config");
        let cm_ok = cm_config?.enable_charge_manager && cm_config?.chargers.length >= 1;
        let cm_multiple_chargers = cm_config.chargers.length > 1;

        let is_em = API.hasModule("energy_manager");
        let device_translation_suffix = is_em ? "em" : "wb";

        let can_switch_phases = false;
        if (is_em) {
            can_switch_phases = this.state.em_contactor_installed;
        } else if (API.hasFeature("evse")) {
            can_switch_phases = API.get_unchecked('evse/hardware_configuration')?.evse_version >= 30;
        }

        let debug_mode = API.hasModule("debug");

        return (
            <SubPage name="power_manager">
                <ConfigForm id="power_manager_config_form" title={__("power_manager.content.page_header")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>

                    <Collapse in={show_enable}>
                        <div>
                            <FormSeparator heading={__("power_manager.content.header_general")} first={true} />
                            <FormRow label={__("power_manager.content.enable_pm")}>
                                <Switch desc={__("power_manager.content.enable_pm_desc")}
                                        checked={s.enabled}
                                        onClick={() => this.setState({
                                            enabled: !s.enabled,
                                            excess_charging_enable: s.enabled ? false : s.excess_charging_enable,
                                        })}
                                />
                            </FormRow>
                        </div>
                    </Collapse>

                    <Collapse in={s.enabled && !cm_ok}>
                        <div>
                            <FormRow label="">
                                <div style="color:red">
                                    {translate_unchecked("power_manager.content.cm_requirements_warning_" + device_translation_suffix)}
                                </div>
                            </FormRow>
                        </div>
                    </Collapse>
                    <Collapse in={s.enabled && !is_em && cm_multiple_chargers}>
                        <div>
                            <FormRow label="">
                                <div style="color:red">
                                    {__("power_manager.content.cm_multiple_chargers_warning")}
                                </div>
                            </FormRow>
                        </div>
                    </Collapse>

                    <FormSeparator heading={__("power_manager.content.header_phase_switching")} first={!show_enable} />
                    <FormRow label={__("power_manager.content.contactor_installed")} hidden={!API.hasFeature("energy_manager")}>
                        <Switch desc={__("power_manager.content.contactor_installed_desc")}
                                disabled={!this.state.enabled}
                                checked={this.state.em_contactor_installed}
                                onClick={() => this.setState({em_contactor_installed: !this.state.em_contactor_installed})}
                        />
                    </FormRow>

                    <FormRow label={__("power_manager.content.phase_switching_mode")}>
                        <InputSelect
                            required={s.enabled}
                            disabled={!s.enabled}
                            items={can_switch_phases ? [
                                ["0", __("power_manager.content.automatic")],
                                ["1", __("power_manager.content.always_single_phase")],
                                ["2", __("power_manager.content.always_three_phases")],
                                ["4", __("power_manager.content.pv1p_fast3p")],
                                ["3", __("power_manager.content.external_control")],
                            ] : [
                                ["1", __("power_manager.content.fixed_single_phase")],
                                ["2", __("power_manager.content.fixed_three_phases")],
                            ]}
                            value={s.phase_switching_mode}
                            onValue={(v) => {
                                this.setState({phase_switching_mode: parseInt(v)});
                                if (v == "2") {
                                    this.setState({guaranteed_power: Math.max(this.state.guaranteed_power, 230 * 6 * 3)});
                                } else if (this.state.guaranteed_power == (230 * 6 * 3)) {
                                    this.setState({guaranteed_power: Math.max(230 * 6, API.get("power_manager/config").guaranteed_power)});
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
                                    {__("power_manager.content.external_control_notification")}
                                </div>
                            </FormRow>
                        </div>
                    </Collapse>

                    <FormSeparator heading={__("power_manager.content.header_excess_charging")} />
                    <FormRow label={__("power_manager.content.enable_excess_charging")} label_muted={__("power_manager.content.enable_excess_charging_muted")}>
                        <Switch desc={__("power_manager.content.enable_excess_charging_desc")}
                            checked={s.excess_charging_enable}
                            disabled={!s.enabled || s.phase_switching_mode == 3}
                            onClick={this.toggle('excess_charging_enable')}/>
                    </FormRow>

                    <FormRow label={__("power_manager.content.default_mode")} label_muted={__("power_manager.content.default_mode_muted")}>
                        <InputSelect
                            required={s.enabled}
                            disabled={!s.enabled}
                            items={mode_list}
                            value={s.default_mode}
                            onValue={s.phase_switching_mode == 3 ? undefined : (v) => this.setState({default_mode: parseInt(v)})}
                        />
                    </FormRow>

                    {reset}

                    <Collapse in={s.excess_charging_enable}>
                        <div>
                            <FormRow label={__("power_manager.content.meter_slot_grid_power")} label_muted={__("power_manager.content.meter_slot_grid_power_muted")}>
                                <InputSelect
                                    required
                                    placeholder={meter_slots.length > 0 ? __("power_manager.content.meter_slot_grid_power_select") : __("power_manager.content.meter_slot_grid_power_none")}
                                    items={meter_slots}
                                    value={s.meter_slot_grid_power}
                                    onValue={(v) => this.setState({meter_slot_grid_power: parseInt(v)})}
                                    disabled={meter_slots.length == 0}
                                />
                            </FormRow>

                            <FormRow label={__("power_manager.content.guaranteed_power")} label_muted={__("power_manager.content.guaranteed_power_muted")}>
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
                                <FormRow label={__("power_manager.content.target_power_from_grid")} label_muted={__("power_manager.content.target_power_from_grid_muted")}>
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
                                <FormRow label={__("power_manager.content.control_behavior")} label_muted={__("power_manager.content.control_behavior_muted")}>
                                    <InputSelect
                                        items={[
                                            ["-200", __("power_manager.content.target_power_n200")],
                                            ["-100", __("power_manager.content.target_power_n100")],
                                            [ "-50", __("power_manager.content.target_power_n50" )],
                                            [   "0", __("power_manager.content.target_power_0"   )],
                                            [  "50", __("power_manager.content.target_power_p50" )],
                                            [ "100", __("power_manager.content.target_power_p100")],
                                            [ "200", __("power_manager.content.target_power_p200")],
                                        ]}
                                        value={s.target_power_from_grid}
                                        onValue={(v) => this.setState({target_power_from_grid: parseInt(v)})}
                                    />
                                </FormRow>
                            </>}

                            <FormRow label={__("power_manager.content.cloud_filter")} label_muted={__("power_manager.content.cloud_filter_muted")}>
                                <InputSelect
                                    items={[
                                        ["0", __("power_manager.content.cloud_filter_off")],
                                        ["1", __("power_manager.content.cloud_filter_weak")],
                                        ["2", __("power_manager.content.cloud_filter_medium")],
                                        ["3", __("power_manager.content.cloud_filter_strong")],
                                    ]}
                                    value={s.cloud_filter_mode}
                                    onValue={(v) => this.setState({cloud_filter_mode: parseInt(v)})}
                                />
                            </FormRow>
                        </div>
                    </Collapse>

                    <FormSeparator heading={__("power_manager.content.header_load_management")} />
                    <FormRow label="">
                        <div class="pt-3 pb-4">
                            {__("power_manager.content.load_management_explainer")}
                        </div>
                    </FormRow>

                    {debug_mode ? <>
                        <FormSeparator heading={__("power_manager.content.header_expert_settings")} />
                        <FormRow label={__("power_manager.content.hysteresis_time")} label_muted={__("power_manager.content.hysteresis_time_muted")}>
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

export function init() {
}
