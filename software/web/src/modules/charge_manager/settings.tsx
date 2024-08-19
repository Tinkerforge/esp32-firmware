/* esp32-firmware
 * Copyright (C) 2021 Erik Fleckstein <erik@tinkerforge.com>
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
import { h, Fragment, RefObject } from "preact";
import { translate_unchecked, __ } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { Button, Collapse } from "react-bootstrap";
import { InputSelect } from "../../ts/components/input_select";
import { InputFloat } from "../../ts/components/input_float";
import { OutputFloat } from "src/ts/components/output_float";
import { Switch } from "../../ts/components/switch";
import { InputNumber } from "../../ts/components/input_number";
import { SubPage } from "../../ts/components/sub_page";

import { MeterValueID } from "../meters/meter_value_id";
import { get_noninternal_meter_slots } from "../power_manager/main";
import type { ChargeManagerStatus } from "./main"
import { FormSeparator } from "src/ts/components/form_separator";

type ChargeManagerConfig = API.getType["charge_manager/config"];

interface ChargeManagerState {
    showExpert: boolean
    dynamicLoadConfig: API.getType['power_manager/dynamic_load_config']
}

export class ChargeManagerSettings extends ConfigComponent<'charge_manager/config', {status_ref?: RefObject<ChargeManagerStatus>}, ChargeManagerState> {
    intervalID: number = null;

    constructor() {
        super('charge_manager/config',
              __("charge_manager.script.save_failed"),
              __("charge_manager.script.reboot_content_changed"), {
                  showExpert: false
              });

        util.addApiEventListener('power_manager/dynamic_load_config', (ev) => {
            this.setState({dynamicLoadConfig: ev.data});
        });
    }

    override async sendSave(t: "charge_manager/config", cfg: ChargeManagerConfig) {
        try {
            await API.save('power_manager/dynamic_load_config', {
                enabled: this.state.dynamicLoadConfig.enabled,
                meter_slot_grid_currents: this.state.dynamicLoadConfig.meter_slot_grid_currents,
                current_limit: this.state.dynamicLoadConfig.current_limit,
                largest_consumer_current: this.state.dynamicLoadConfig.largest_consumer_current,
                safety_margin_pct: this.state.dynamicLoadConfig.safety_margin_pct,
            }, __("power_manager.script.save_failed"));
        } catch (e) {
            console.log("charge_manager save failed", e);
        }

        let {enable_charge_manager, chargers, maximum_available_current, ...new_values} = cfg;
        let new_cfg: ChargeManagerConfig = {...API.get("charge_manager/config"), ...new_values};

        await super.sendSave(t, new_cfg);
    }

    override async sendReset(t: "charge_manager/config"){
        const modal = util.async_modal_ref.current;
        if (!await modal.show({
                title:__("reset.reset_modal"),
                body: __("charge_manager.content.charge_manager_settings_reset_modal_text"),
                no_text: __("reset.reset_modal_abort"),
                yes_text: __("reset.reset_modal_confirm"),
                no_variant: "secondary",
                yes_variant: "danger"
            }))
            return;

        let energyManagerMode = API.hasModule("energy_manager") && !(API.hasModule("evse_v2") || API.hasModule("evse"));
        if (!energyManagerMode)
            await API.save_unchecked('evse/management_enabled', {"enabled": false}, translate_unchecked("charge_manager.script.save_failed"));

        await super.sendReset(t);
    }

    override getIsModified(t: "charge_manager/config"): boolean {
        let evse_enabled = API.get_unchecked("evse/management_enabled");
        if (evse_enabled != null && evse_enabled.enabled)
            return true;
        return super.getIsModified(t);
    }

    render(props: {}, state: ChargeManagerConfig & ChargeManagerState) {
        if (!util.render_allowed())
            return <SubPage name="charge_manager_settings" />;

        let energyManagerMode = API.hasModule("energy_manager") && !(API.hasModule("evse_v2") || API.hasModule("evse"));
        let warpUltimateMode  = API.hasModule("energy_manager") &&  (API.hasModule("evse_v2") || API.hasModule("evse"));
        let is_warp3          = API.get_unchecked("evse/hardware_configuration")?.evse_version >= 30;
        let show_1p_current   = energyManagerMode || warpUltimateMode || is_warp3 || API.hasFeature("phase_switch");

        const meter_slots = get_noninternal_meter_slots([MeterValueID.CurrentL1ImExDiff, MeterValueID.CurrentL2ImExDiff, MeterValueID.CurrentL3ImExDiff], __("charge_manager.content.dlm_meter_slot_grid_currents_missing_values"));

        let verbose = <FormRow label={__("charge_manager.content.verbose")}>
                <Switch desc={__("charge_manager.content.verbose_desc")}
                        checked={state.verbose}
                        onClick={this.toggle("verbose")}/>
            </FormRow>;

        let watchdog = <FormRow label={__("charge_manager.content.enable_watchdog")} label_muted={__("charge_manager.content.enable_watchdog_muted")}>
                <Switch desc={__("charge_manager.content.enable_watchdog_desc")}
                        checked={state.enable_watchdog}
                        onClick={this.toggle("enable_watchdog")}/>
            </FormRow>;

        let default_available_current = <FormRow label={__("charge_manager.content.default_available_current")} label_muted={__("charge_manager.content.default_available_current_muted")}>
                <InputFloat
                    unit="A"
                    value={state.default_available_current}
                    onValue={this.set("default_available_current")}
                    digits={3}
                    min={0}
                    max={state.maximum_available_current}
                    disabled={state.dynamicLoadConfig.enabled}
                    />
                <div class="invalid-feedback">{__("charge_manager.content.default_available_current_invalid")}</div>
            </FormRow>;

        let requested_current_margin = <FormRow label={__("charge_manager.content.requested_current_margin")} label_muted={__("charge_manager.content.requested_current_margin_muted")}>
                <InputFloat
                    unit="A"
                    value={state.requested_current_margin}
                    onValue={this.set("requested_current_margin")}
                    digits={3}
                    min={1000}
                    max={10000}
                    />
            </FormRow>;

        let requested_current_threshold = <FormRow label={__("charge_manager.content.requested_current_threshold")} label_muted={__("charge_manager.content.requested_current_threshold_muted")}>
                <InputNumber
                    unit="s"
                    value={state.requested_current_threshold}
                    onValue={this.set("requested_current_threshold")}
                    min={20}
                    max={3600}
                    />
            </FormRow>;

        let minimum_current = <>
            <FormRow label={__("charge_manager.content.minimum_current_auto")}>
                <Switch desc={__("charge_manager.content.minimum_current_auto_desc")}
                    checked={state.minimum_current_auto}
                    onClick={this.toggle('minimum_current_auto')}
                />
            </FormRow>

            <Collapse in={state.minimum_current_auto}>
                <div>
                    <FormRow label={__("charge_manager.content.minimum_current_vehicle_type")}>
                        <InputSelect items={[
                                ["0", __("charge_manager.content.minimum_current_vehicle_type_other")],
                                ["1", __("charge_manager.content.minimum_current_vehicle_type_zoe")],
                            ]}
                            value={state.minimum_current_vehicle_type}
                            onValue={(v) => this.setState({minimum_current_vehicle_type: parseInt(v)})}
                            />
                    </FormRow>
                </div>
            </Collapse>

            <Collapse in={!state.minimum_current_auto}>
                <div>
                    <FormRow label={      show_1p_current ? __("charge_manager.content.minimum_current_3p")       : __("charge_manager.content.minimum_current")}
                             label_muted={show_1p_current ? __("charge_manager.content.minimum_current_3p_muted") : __("charge_manager.content.minimum_current_muted")}>
                        <InputFloat
                            unit="A"
                            value={state.minimum_current}
                            onValue={(v) => this.setState({minimum_current: v})}
                            digits={3}
                            min={6000}
                            max={state.maximum_available_current}
                        />
                    </FormRow>

                    {show_1p_current ?
                        <FormRow label={__("charge_manager.content.minimum_current_1p")} label_muted={__("charge_manager.content.minimum_current_1p_muted")}>
                            <InputFloat
                                unit="A"
                                value={state.minimum_current_1p}
                                onValue={(v) => this.setState({minimum_current_1p: v})}
                                digits={3}
                                min={6000}
                                max={state.maximum_available_current}
                            />
                        </FormRow>
                    :
                        null
                    }
                </div>
            </Collapse>
        </>

        return (
            <SubPage name="charge_manager_settings">
                <ConfigForm id="charge_manager_config_form" title={__("charge_manager.content.charge_manager_settings")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    {energyManagerMode ?
                        <>
                            {minimum_current}
                        </>
                        : <>
                            <Collapse in={!API.get("charge_manager/config").enable_charge_manager}>
                                <div>
                                    <FormRow label="">
                                        <div style="color:red">
                                            {__("charge_manager.content.managed_disabled")}
                                        </div>
                                    </FormRow>
                                </div>
                            </Collapse>

                            {minimum_current}

                            <FormRow label={__("charge_manager.content.configuration_mode")} label_muted={__("charge_manager.content.configuration_mode_muted")}>
                                <Button className="form-control" onClick={() => this.setState({showExpert: !state.showExpert})}>
                                    {state.showExpert ? __("component.collapsed_section.hide") : __("component.collapsed_section.show")}
                                </Button>
                            </FormRow>

                            <Collapse in={state.showExpert}>
                                <div>
                                    {verbose}
                                    {watchdog}
                                    {default_available_current}
                                    {requested_current_threshold}
                                    {requested_current_margin}
                                </div>
                            </Collapse>
                        </>
                    }
                    {API.hasModule("power_manager") ?
                        <>
                            <FormSeparator heading={__("charge_manager.content.header_load_management")} />

                            <FormRow label={__("charge_manager.content.dlm_enabled")}>
                                <Switch desc={__("charge_manager.content.dlm_enabled_desc")}
                                    checked={state.dynamicLoadConfig.enabled}
                                    onClick={() => this.setState({dynamicLoadConfig: {...state.dynamicLoadConfig, enabled: !state.dynamicLoadConfig.enabled}})}
                                />
                            </FormRow>

                            <FormRow label={__("charge_manager.content.dlm_meter_slot_grid_currents")}>
                                <InputSelect
                                    required={state.dynamicLoadConfig.enabled}
                                    placeholder={meter_slots.length > 0 ? __("charge_manager.content.dlm_meter_slot_grid_currents_select") : __("charge_manager.content.dlm_meter_slot_grid_currents_none")}
                                    items={meter_slots}
                                    value={state.dynamicLoadConfig.meter_slot_grid_currents}
                                    onValue={(v) => this.setState({dynamicLoadConfig: {...state.dynamicLoadConfig, meter_slot_grid_currents: parseInt(v)}})}
                                />
                            </FormRow>

                            <FormRow label={__("charge_manager.content.dlm_current_limit")} label_muted={__("charge_manager.content.dlm_current_limit_muted")}>
                                <InputFloat
                                    required={state.dynamicLoadConfig.enabled}
                                    unit="A"
                                    value={state.dynamicLoadConfig.current_limit / 1000}
                                    onValue={(v) => this.setState({dynamicLoadConfig: {...state.dynamicLoadConfig, current_limit: v * 1000}})}
                                    digits={0}
                                    min={25}
                                    max={524}
                                />
                            </FormRow>

                            <FormRow label={__("charge_manager.content.dlm_largest_consumer_current")} label_muted={__("charge_manager.content.dlm_largest_consumer_current_muted")}>
                                <InputFloat
                                    required={state.dynamicLoadConfig.enabled}
                                    unit="A"
                                    value={state.dynamicLoadConfig.largest_consumer_current / 1000}
                                    onValue={(v) => this.setState({dynamicLoadConfig: {...state.dynamicLoadConfig, largest_consumer_current: v * 1000}})}
                                    digits={0}
                                    min={16}
                                    max={state.dynamicLoadConfig.current_limit / 1000}
                                />
                            </FormRow>

                            <FormRow label={__("charge_manager.content.dlm_safety_margin_pct")} label_muted={__("charge_manager.content.dlm_safety_margin_pct_muted")}>
                                <InputFloat
                                    required={state.dynamicLoadConfig.enabled}
                                    unit="%"
                                    value={state.dynamicLoadConfig.safety_margin_pct}
                                    onValue={(v) => this.setState({dynamicLoadConfig: {...state.dynamicLoadConfig, safety_margin_pct: v}})}
                                    digits={0}
                                    min={0}
                                    max={50}
                                />
                            </FormRow>

                            {API.hasModule("debug") ?
                                <>
                                    <FormRow label="Target constant current" label_muted="for debugging">
                                        <OutputFloat
                                            unit="A"
                                            value={Math.min((state.dynamicLoadConfig.current_limit * 1.4) - state.dynamicLoadConfig.largest_consumer_current, state.dynamicLoadConfig.current_limit)
                                                * (100 - state.dynamicLoadConfig.safety_margin_pct) / 100}
                                            digits={3}
                                            scale={3}
                                        />
                                    </FormRow>

                                    <FormRow label="Expected peak current" label_muted="for debugging">
                                        <OutputFloat
                                            unit="A"
                                            value={Math.min((state.dynamicLoadConfig.current_limit * 1.4) - state.dynamicLoadConfig.largest_consumer_current, state.dynamicLoadConfig.current_limit)
                                                * (100 - state.dynamicLoadConfig.safety_margin_pct) / 100 + state.dynamicLoadConfig.largest_consumer_current}
                                            digits={3}
                                            scale={3}
                                        />
                                    </FormRow>
                                </>
                            :
                                null
                            }
                        </>
                    :
                        null
                    }
                    </ConfigForm>
            </SubPage>
        )
    }
}
