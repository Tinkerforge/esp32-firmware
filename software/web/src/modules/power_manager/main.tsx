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
import { __ } from "../../ts/translation";
import { METERS_SLOTS } from "../../build";
import { h, Fragment, Component, RefObject } from "preact";
import { Button, ButtonGroup, Collapse, Spinner } from "react-bootstrap";
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
import { MeterClassID    } from "../meters/meter_class_id.enum";
import { MeterValueID    } from "../meters/meter_value_id";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { CheckCircle, Circle, Settings, Sun } from "react-feather";

export function PowerManagerSettingsNavbar() {
    return <NavbarItem name="power_manager_settings" module="power_manager" title={__("power_manager.navbar.power_manager_settings")} symbol={<Settings />} />;
}

export function PVExcessSettingsNavbar() {
    return <NavbarItem name="pv_excess_settings" module="power_manager" title={__("power_manager.navbar.pv_excess_settings")} symbol={<Sun />} />;
}

type StringStringTuple = [string, string];

export function get_noninternal_meter_slots(required_ids : Readonly<MeterValueID[]>, missing_values_message: string) {
    let meter_slots: StringStringTuple[] = [];

    for (let i = 0; i < METERS_SLOTS; i++) {
        const cfg = API.get_unchecked(`meters/${i}/config`) as API.getType["meters/0/config"]

        if (cfg == null) {
            console.log(`power_manager: meters/${i}/config missing`);
            continue;
        } else if (cfg[0] == MeterClassID.None) {
            continue;
        } else if (cfg[0] as any == MeterClassID.RS485Bricklet || cfg[0] as any == MeterClassID.EVSEV2) {
            // Disallow selecting the charger-internal meter for PM
            meter_slots.push([i.toString() + "-disabled", `${cfg[1]?.display_name} (${__("power_manager.script.meter_slots_internal")})`]);
        } else {
            const value_ids = API.get_unchecked(`meters/${i}/value_ids`) as Readonly<number[]>;

            if (value_ids?.length <= 0) {
                meter_slots.push([i.toString(), `${cfg[1]?.display_name} (${__("power_manager.script.meter_slots_no_values")})`]);
            } else {
                let missing_required_value = false;

                for (const id of required_ids) {
                    if (value_ids.indexOf(id) < 0) {
                        missing_required_value = true;
                        break;
                    }
                }

                if (missing_required_value) {
                    meter_slots.push([i.toString() + "-disabled", `${cfg[1]?.display_name} (${missing_values_message})`]);
                } else {
                    meter_slots.push([i.toString(), cfg[1]?.display_name]);
                }
            }
        }
    }

    return meter_slots;
}

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

        return <FormRow label="">
            <IndicatorGroup
                value={0}
                items={[
                    ["danger", label],
                ]}
            />
        </FormRow>
    }

    generate_config_error_labels(config_error_flags: number) {
        if (!API.get('power_manager/config').enabled || config_error_flags == 0)
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
            {API.get('power_manager/config').enabled ?
                <FormRow label={__("power_manager.status.mode")}>
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
                : null}

            {API.hasFeature("phase_switch") && (API.get_unchecked("evse/management_enabled") == null || !API.get_unchecked("evse/management_enabled").enabled) ?
                <FormRow label={__("power_manager.status.phase_switching")}>
                    <ButtonGroup className="flex-wrap m-n1" style="width: calc(100% + 0.5rem);">
                        <Button
                            style="display: flex;align-items: center;justify-content: center;"
                            className="m-1 rounded-left rounded-right"
                            variant={!ll_state.is_3phase ? "success" : "primary"}
                            disabled={!ll_state.is_3phase || state.external_control != 0}
                            onClick={() => this.change_phase(1)}>
                            {state.external_control != 3 ? (!ll_state.is_3phase ? <CheckCircle size="20"/> : <Circle size="20"/>) : <Spinner size="sm" animation="grow" />} <span>&nbsp;&nbsp;</span><span>{__("power_manager.status.single_phase")}</span>
                        </Button>
                        <Button
                            style="display: flex;align-items: center;justify-content: center;"
                            className="m-1 rounded-left rounded-right"
                            variant={ll_state.is_3phase ? "success" : "primary"}
                            disabled={ll_state.is_3phase || state.external_control != 0}
                            onClick={() => this.change_phase(3)}>
                            {state.external_control != 3 ? (ll_state.is_3phase ? <CheckCircle size="20"/> :  <Circle size="20"/>) : <Spinner size="sm" animation="grow" />} <span>&nbsp;&nbsp;</span><span>{__("power_manager.status.three_phase")}</span>
                        </Button>
                    </ButtonGroup>
                </FormRow>
            : (API.get('power_manager/config').enabled || (API.hasFeature("phase_switch") && API.get_unchecked("evse/management_enabled")?.enabled) ?
                <FormRow label={__("power_manager.status.phase_switching")}>
                    <IndicatorGroup
                        value={!ll_state.is_3phase ? 0 : 1}
                        items={[
                            ["primary", __("power_manager.status.single_phase")],
                            ["primary", __("power_manager.status.three_phase")],
                        ]} />
                </FormRow>
                : undefined)
            }

            {this.generate_config_error_labels(state.config_error_flags)}
        </StatusSection>
    }
}

export class PowerManagerSettings extends ConfigComponent<'power_manager/config', {status_ref?: RefObject<PowerManagerStatus>}, {em_contactor_installed: boolean}> {
    constructor() {
        super('power_manager/config',
            __("power_manager.script.save_failed"),
            __("power_manager.script.reboot_content_changed"));

        this.setState({em_contactor_installed: false});

        util.addApiEventListener_unchecked('energy_manager/config', () => {
            this.setState({em_contactor_installed: API.get_unchecked('energy_manager/config')?.contactor_installed});
        });
    }

    override async sendSave(t: "power_manager/config", cfg: API.getType['power_manager/config']) {
        if (API.hasModule("em_v1")) {
            await API.save_unchecked('energy_manager/config', {
                contactor_installed: this.state.em_contactor_installed,
            }, __("power_manager.script.save_failed"));
        }

        // Only update enabled. Rest is controlled by the PV excess settings subpage
        let new_cfg: API.getType['power_manager/config'] = {...API.get("power_manager/config"), enabled:this.state.enabled};

        await super.sendSave(t, new_cfg);
    }

    override async sendReset(t: "power_manager/config") {
        if (API.hasModule("em_v1")) { // TODO EM V2 has this config too but without contactor_installed field.
            await API.save_unchecked('energy_manager/config', {
                ...API.get_unchecked('energy_manager/config'),
                contactor_installed: this.state.em_contactor_installed,
            }, this.error_string);
        }

        let new_cfg: API.getType['power_manager/config'] = {...API.get("power_manager/config"), enabled: API.hasModule("em_common")};

        await super.sendSave(t, new_cfg);
    }

    override getIsModified(t: "power_manager/config"): boolean {
        return API.get_unchecked("energy_manager/config")?.contactor_installed || API.get("power_manager/config").enabled;
    }

    render(props: {}, s: Readonly<API.getType['power_manager/config']>) {
        if (!util.render_allowed())
            return <SubPage name="power_manager_settings" />;

        // Currently assume that the power manager settings page is only compiled in if this is an energy manager.
        // The only setting to set is whether a contactor is installed,
        // which is always (WARP3) or never (WARP1/2) the case with chargers.
        // Enabling/Disabling the power manager is not necessary on chargers because it gets enabled iff PV excess charging is enabled.
        // Still show the UI to re-enable the power manager on an energy manager, because there disabling the
        // PV excess charging should not disable the power manager to allow manual phase switching of connected chargers.
        let show_enable = !s.enabled;

        return (
            <SubPage name="power_manager_settings">
                <ConfigForm id="power_manager_config_form" title={__("power_manager.content.page_header")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>

                    <Collapse in={show_enable}>
                        <div>
                            <FormRow label={__("power_manager.content.enable_pm")}>
                                <Switch desc={__("power_manager.content.enable_pm_desc")}
                                        checked={s.enabled}
                                        onClick={() => this.setState({
                                            enabled: !s.enabled
                                        })}
                                />
                            </FormRow>
                        </div>
                    </Collapse>

                    <Collapse in={API.hasModule("em_v1")}>
                        <FormRow label={__("power_manager.content.contactor_installed")}>
                            <Switch desc={__("power_manager.content.contactor_installed_desc")}
                                    checked={this.state.em_contactor_installed}
                                    onClick={() => this.setState({em_contactor_installed: !this.state.em_contactor_installed})}
                            />
                        </FormRow>
                    </Collapse>
                </ConfigForm>
            </SubPage>
        );
    }
}

export class PVExcessSettings extends ConfigComponent<'power_manager/config', {status_ref?: RefObject<PowerManagerStatus>}> {
    constructor() {
        super('power_manager/config',
            __("power_manager.script.save_failed"),
            __("power_manager.script.reboot_content_changed"));
    }

    render(props: {}, s: Readonly<API.getType['power_manager/config']>) {
        if (!util.render_allowed())
            return <SubPage name="pv_excess_settings" />;

        let mode_list: StringStringTuple[] = [];

        mode_list.push(["1", __("power_manager.status.mode_off")]);
        mode_list.push([s.excess_charging_enable ? "2" : "2-disabled", __("power_manager.status.mode_pv")]);
        mode_list.push([s.excess_charging_enable ? "3" : "3-disabled", __("power_manager.status.mode_min_pv")]);
        mode_list.push(["0", __("power_manager.status.mode_fast")]);

        const meter_slots = get_noninternal_meter_slots([MeterValueID.PowerActiveLSumImExDiff], __("power_manager.content.meter_slot_grid_power_missing_value"));

        let cm_config = API.get_unchecked("charge_manager/config");
        let cm_ok = cm_config?.enable_charge_manager && cm_config?.chargers.length >= 1;

        let is_em = API.hasModule("em_common");
        let device_translation_suffix = is_em ? "em" : "wb";

        // On a charger, the power manager is enabled iff excess charging is enabled.
        let enabled = is_em ? s.enabled : s.excess_charging_enable;

        let can_switch_phases = false;
        if (is_em) {
            can_switch_phases = API.get_unchecked('energy_manager/config')?.contactor_installed;
        } else if (API.hasFeature("evse")) {
            can_switch_phases = API.get_unchecked('evse/hardware_configuration')?.evse_version >= 30;
        }

        let debug_mode = API.hasModule("debug");

        let phase_switching_modes: [string,string][] = [];
        if (can_switch_phases) {
            phase_switching_modes.push(["0", __("power_manager.content.automatic")],
                                       ["1", __("power_manager.content.always_single_phase")],
                                       ["2", __("power_manager.content.always_three_phases")],
                                       ["4", __("power_manager.content.pv1p_fast3p")])
            if (is_em)
                phase_switching_modes.push(["3", __("power_manager.content.external_control")]);
        } else {
            phase_switching_modes.push(["1", __("power_manager.content.fixed_single_phase")],
                                       ["2", __("power_manager.content.fixed_three_phases")]);
        }

        let charge_manager_config = API.get("charge_manager/config");
        let guaranteed_power_lower_limit_1p = 230 * 1 * charge_manager_config.minimum_current_1p / 1000;
        let guaranteed_power_lower_limit_3p = 230 * 3 * charge_manager_config.minimum_current    / 1000;

        return (
            <SubPage name="pv_excess_settings">
                <ConfigForm id="pv_excess_config_form" title={__("power_manager.content.header_excess_charging")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>

                    <Collapse in={enabled && !cm_ok}>
                        <div>
                            <FormRow label="">
                                <div style="color:red">
                                    {__("power_manager.content.cm_requirements_warning")}
                                </div>
                            </FormRow>
                        </div>
                    </Collapse>

                    <FormRow label={__("power_manager.content.enable_excess_charging")} label_muted={__("power_manager.content.enable_excess_charging_muted")}>
                        <Switch desc={__("power_manager.content.enable_excess_charging_desc")}
                            checked={s.excess_charging_enable}
                            disabled={s.phase_switching_mode == 3}
                            onClick={() => {
                                        let enable_pv_excess = !this.state.excess_charging_enable;

                                        // Force PM on if PV excess is enabled. Never force off because dynamic load management might be on and the PM will disable itself if unused.
                                        if (enable_pv_excess) {
                                            this.setState({enabled: true});
                                        }

                                        this.setState({excess_charging_enable: enable_pv_excess});
                                    }}
                            />
                    </FormRow>

                    <FormRow label={__("power_manager.content.phase_switching_mode")}>
                        <InputSelect
                            items={phase_switching_modes}
                            value={s.phase_switching_mode}
                            onValue={(v) => {
                                this.setState({phase_switching_mode: parseInt(v)});
                                if (v == "2") {
                                    this.setState({guaranteed_power: Math.max(guaranteed_power_lower_limit_3p, this.state.guaranteed_power)});
                                } else if (this.state.guaranteed_power == (guaranteed_power_lower_limit_3p)) {
                                    this.setState({guaranteed_power: Math.max(guaranteed_power_lower_limit_1p, API.get("power_manager/config").guaranteed_power)});
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

                    <FormRow label={__("power_manager.content.default_mode")} label_muted={__("power_manager.content.default_mode_muted")}>
                        <InputSelect
                            items={mode_list}
                            value={s.default_mode}
                            onValue={s.phase_switching_mode == 3 ? undefined : (v) => this.setState({default_mode: parseInt(v)})}
                        />
                    </FormRow>

                    <Collapse in={s.excess_charging_enable || !is_em}>
                        <div>
                            <FormRow label={__("power_manager.content.meter_slot_grid_power")} label_muted={__("power_manager.content.meter_slot_grid_power_muted")}>
                                <InputSelect
                                    required={s.excess_charging_enable}
                                    placeholder={meter_slots.length > 0 ? __("power_manager.content.meter_slot_grid_power_select") : __("power_manager.content.meter_slot_grid_power_none")}
                                    items={meter_slots}
                                    value={s.meter_slot_grid_power}
                                    onValue={(v) => this.setState({meter_slot_grid_power: parseInt(v)})}
                                />
                            </FormRow>

                            <FormRow label={__("power_manager.content.guaranteed_power")} label_muted={__("power_manager.content.guaranteed_power_muted")}>
                                <InputFloat
                                    unit="kW"
                                    value={s.guaranteed_power}
                                    onValue={this.set('guaranteed_power')}
                                    digits={3}
                                    min={s.phase_switching_mode == 2 ? guaranteed_power_lower_limit_3p : guaranteed_power_lower_limit_1p}
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
                                        min={-63 * 230 * 3}
                                        max={500 * 230 * 3}
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
                </ConfigForm>
            </SubPage>
        );
    }
}

export function init() {
}
