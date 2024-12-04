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
import { InputSelect     } from "../../ts/components/input_select";
import { Switch          } from "../../ts/components/switch";
import { SubPage         } from "../../ts/components/sub_page";
import { MeterClassID    } from "../meters/meter_class_id.enum";
import { MeterValueID    } from "../meters/meter_value_id";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { CheckCircle, Circle, Settings, Sun } from "react-feather";

const METER_SLOT_BATTERY_NO_BATTERY = 255;

export function PVExcessSettingsNavbar() {
    return <NavbarItem name="pv_excess_settings" module="power_manager" title={__("power_manager.navbar.pv_excess_settings")} symbol={<Sun />} />;
}

type StringStringTuple = [string, string];

export enum NoninternalMeterSelector {
    AnyValue,
    AllValues,
}

export function get_noninternal_meter_slots(required_ids : Readonly<MeterValueID[]>, meter_selector : NoninternalMeterSelector, missing_values_message: string) {
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
                let have_all_values = true;
                let have_any_value = false;

                for (const id of required_ids) {
                    if (value_ids.indexOf(id) >= 0) {
                        have_any_value = true;
                    } else {
                        have_all_values = false;
                    }
                }

                let meter_ok = false;

                if (meter_selector == NoninternalMeterSelector.AllValues && have_all_values) {
                    meter_ok = true;
                } else if (meter_selector == NoninternalMeterSelector.AnyValue && have_any_value) {
                    meter_ok = true;
                }

                if (meter_ok) {
                    meter_slots.push([i.toString(), cfg[1]?.display_name]);
                } else {
                    meter_slots.push([i.toString() + "-disabled", `${cfg[1]?.display_name} (${missing_values_message})`]);
                }
            }
        }
    }

    return meter_slots;
}

export class PowerManagerStatus extends Component {
    change_mode(mode: number) {
        API.save('power_manager/charge_mode', {"mode": mode}, () => __("power_manager.script.mode_change_failed"));
    }

    change_phase(phases: number) {
        API.save('power_manager/external_control', {"phases_wanted": phases}, () => __("power_manager.script.mode_change_failed"));
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
            {this.generate_config_error_label(config_error_flags & 16, __("power_manager.status.config_error_dlm_no_meter"))}
        </>
    }

    render() {
        if (!util.render_allowed())
            return <StatusSection name="power_manager" />

        let charge_mode = API.get('power_manager/charge_mode');
        let state       = API.get('power_manager/state');
        let ll_state    = API.get('power_manager/low_level_state');
        let config      = API.get('power_manager/config');
        const phases    = API.get_unchecked('evse/low_level_state')?.phases_current | API.get_unchecked('energy_manager/state')?.phases_switched;

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

            {API.get_unchecked("evse/hardware_configuration")?.evse_version >= 30 || API.get_unchecked("energy_manager/config")?.contactor_installed ?
                API.hasFeature("phase_switch") && (API.get_unchecked("evse/management_enabled") == null || !API.get_unchecked("evse/management_enabled").enabled) ?
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
                :
                    <FormRow label={__("power_manager.status.phase_switching")}>
                        <IndicatorGroup
                            value={phases == 3 ? 1 : 0}
                            items={[
                                ["primary", __("power_manager.status.single_phase")],
                                ["primary", __("power_manager.status.three_phase")],
                            ]} />
                    </FormRow>
            : undefined}

            {this.generate_config_error_labels(state.config_error_flags)}
        </StatusSection>
    }
}

export class PVExcessSettings extends ConfigComponent<'power_manager/config', {status_ref?: RefObject<PowerManagerStatus>}> {
    constructor() {
        super('power_manager/config',
              () => __("power_manager.script.save_failed"),
              () => __("power_manager.script.reboot_content_changed"));
    }

    render(props: {}, s: Readonly<API.getType['power_manager/config']>) {
        if (!util.render_allowed())
            return <SubPage name="pv_excess_settings" />;

        let mode_list: StringStringTuple[] = [];

        mode_list.push(["1", __("power_manager.status.mode_off")]);
        mode_list.push([s.excess_charging_enable ? "2" : "2-disabled", __("power_manager.status.mode_pv")]);
        mode_list.push([s.excess_charging_enable ? "3" : "3-disabled", __("power_manager.status.mode_min_pv")]);
        mode_list.push(["0", __("power_manager.status.mode_fast")]);

        let meter_slots = get_noninternal_meter_slots([MeterValueID.PowerActiveLSumImExDiff], NoninternalMeterSelector.AllValues, __("power_manager.content.meter_slot_grid_power_missing_value"));
        for (let i = 0; i < meter_slots.length; i++) {
            if (parseInt(meter_slots[i][0]) == s.meter_slot_battery_power) {
                meter_slots[i][0] += "-disabled";
                meter_slots[i][1] += " (" + __("power_manager.content.meter_slot_grid_power_in_use_by_battery") + ")";
                break;
            }
        }

        let meter_slots_for_battery = get_noninternal_meter_slots([MeterValueID.PowerActiveLSumImExDiff, MeterValueID.PowerDCImExDiff, MeterValueID.PowerDCChaDisDiff], NoninternalMeterSelector.AnyValue, __("power_manager.content.meter_slot_battery_power_missing_value"));
        for (let i = 0; i < meter_slots_for_battery.length; i++) {
            if (parseInt(meter_slots_for_battery[i][0]) == s.meter_slot_grid_power) {
                meter_slots_for_battery[i][0] += "-disabled";
                meter_slots_for_battery[i][1] += " (" + __("power_manager.content.meter_slot_battery_power_in_use_by_grid") + ")";
                break;
            }
        }
        meter_slots_for_battery.unshift(["255", __("power_manager.content.meter_slot_battery_power_none")]);

        let cm_config = API.get_unchecked("charge_manager/config");
        const em_phase_switcher_charger_config = API.get_unchecked("em_phase_switcher/charger_config");
        let cm_ok = cm_config?.enable_charge_manager && cm_config?.chargers.length >= 1;
        const cm_requirements_warning = em_phase_switcher_charger_config?.proxy_mode ? __("power_manager.content.em_proxy_warning") : __("power_manager.content.cm_requirements_warning");

        let is_em = API.hasModule("em_common");

        // On a charger, the power manager is enabled iff excess charging is enabled.
        let enabled = is_em ? s.enabled : s.excess_charging_enable;

        let debug_mode = API.hasModule("debug");

        const phase_switching_mode = API.hasModule("em_phase_switcher") ? <FormRow label={__("power_manager.content.phase_switching_mode")}>
                <InputSelect
                    items={[
                        ["0", __("power_manager.content.automatic")],
                        cm_ok && em_phase_switcher_charger_config?.idx < 254 // EM has controlled charger
                            ? ["3", __("power_manager.content.external_control")]
                            : ["3-disabled", __("power_manager.content.external_control_unavailable")],
                    ]}
                    value={s.phase_switching_mode}
                    onValue={(v) => {
                        this.setState({phase_switching_mode: parseInt(v)});
                        if (v == "3") {
                            this.setState({
                                excess_charging_enable: false,
                                default_mode: 0,
                            });
                        }
                    }}
                />
            </FormRow>
        : null;

        let control_behavior_items: [string,string][] = [
            ["-200", __("power_manager.content.target_power_n200")],
            ["-100", __("power_manager.content.target_power_n100")],
        ];

        if (s.meter_slot_battery_power == METER_SLOT_BATTERY_NO_BATTERY) {
            control_behavior_items.push(["-50", __("power_manager.content.target_power_n50_without_battery_meter")],
                                        [  "0", __("power_manager.content.target_power_0_without_battery_meter"  )],
                                        [ "50", __("power_manager.content.target_power_p50_without_battery_meter")]);
        }
        else {
            control_behavior_items.push(["-50", __("power_manager.content.target_power_n50_with_battery_meter")],
                                        [  "0", __("power_manager.content.target_power_0_with_battery_meter"  )],
                                        [ "50", __("power_manager.content.target_power_p50_with_battery_meter")]);
        }

        control_behavior_items.push(["100", __("power_manager.content.target_power_p100")],
                                    ["200", __("power_manager.content.target_power_p200")]);

        return (
            <SubPage name="pv_excess_settings">
                <ConfigForm id="pv_excess_config_form" title={__("power_manager.content.header_excess_charging")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>

                    <Collapse in={enabled && !cm_ok}>
                        <div>
                            <FormRow label="">
                                <div style="color:red">
                                    {cm_requirements_warning}
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

                    {phase_switching_mode}

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
                                    min={1380}
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
                                        items={control_behavior_items}
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

                            <FormSeparator heading={__("power_manager.content.header_battery_storage")} />

                            <FormRow label={__("power_manager.content.meter_slot_battery_power")} label_muted={__("power_manager.content.meter_slot_battery_power_muted")}>
                                <InputSelect
                                    required={s.excess_charging_enable}
                                    placeholder={meter_slots_for_battery.length > 0 ? __("power_manager.content.meter_slot_grid_power_select") : __("power_manager.content.meter_slot_grid_power_none")}
                                    items={meter_slots_for_battery}
                                    value={s.meter_slot_battery_power}
                                    onValue={(v) => this.setState({meter_slot_battery_power: parseInt(v)})}
                                />
                            </FormRow>

                            <FormRow label={__("power_manager.content.battery_mode")} label_muted="">
                                <InputSelect
                                    disabled={s.meter_slot_battery_power >= 255}
                                    required={s.meter_slot_battery_power < 255}
                                    items={[
                                        ["0", __("power_manager.content.battery_mode_prefer_chargers")],
                                        ["1", __("power_manager.content.battery_mode_prefer_battery")],
                                    ]}
                                    value={s.battery_mode}
                                    onValue={(v) => this.setState({battery_mode: parseInt(v)})}
                                />
                            </FormRow>

                            <FormRow label={__("power_manager.content.battery_inverted")} label_muted="">
                                <InputSelect
                                    disabled={s.meter_slot_battery_power >= 255}
                                    required={s.meter_slot_battery_power < 255}
                                    items={[
                                        ["n", __("power_manager.content.battery_inverted_n")],
                                        ["i", __("power_manager.content.battery_inverted_i")],
                                    ]}
                                    value={s.battery_inverted ? "i" : "n"}
                                    onValue={(v) => this.setState({battery_inverted: v == "i"})}
                                />
                            </FormRow>

                            <FormRow label={__("power_manager.content.battery_deadzone")} label_muted={__("power_manager.content.battery_deadzone_muted")} help={__("power_manager.content.battery_deadzone_help")}>
                                <InputFloat
                                    disabled={s.meter_slot_battery_power >= 255}
                                    required={s.meter_slot_battery_power < 255}
                                    unit="W"
                                    value={s.battery_deadzone}
                                    onValue={this.set('battery_deadzone')}
                                    digits={0}
                                    min={10}
                                    max={9999}
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
