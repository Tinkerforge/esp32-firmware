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
import { Switch } from "../../ts/components/switch";
import { InputNumber } from "../../ts/components/input_number";
import { SubPage } from "../../ts/components/sub_page";

import type { ChargeManagerStatus } from "./main"
import { FormSeparator } from "src/ts/components/form_separator";

type ChargeManagerConfig = API.getType["charge_manager/config"];

interface ChargeManagerState {
    showExpert: boolean
}

export class ChargeManagerSettings extends ConfigComponent<'charge_manager/config', {status_ref?: RefObject<ChargeManagerStatus>}, ChargeManagerState> {
    intervalID: number = null;

    constructor() {
        super('charge_manager/config',
              __("charge_manager.script.save_failed"),
              __("charge_manager.script.reboot_content_changed"), {
                  showExpert: false
              });
    }

    override async sendSave(t: "charge_manager/config", cfg: ChargeManagerConfig) {
        let {enable_charge_manager, chargers, ...new_values} = cfg;

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
                    />
                <div class="invalid-feedback">{__("charge_manager.content.default_available_current_invalid")}</div>
            </FormRow>;

        let maximum_available_current = <FormRow label={__("charge_manager.content.maximum_available_current")} label_muted={__("charge_manager.content.maximum_available_current_muted")}>
                <InputFloat
                    unit="A"
                    value={state.maximum_available_current}
                    onValue={(v) => this.setState({
                        maximum_available_current: v,
                        default_available_current: Math.min(v, state.default_available_current)
                    })}
                    digits={3}
                    min={state.minimum_current}
                    max={1000000}
                    />
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
                    <FormRow label={      energyManagerMode ? __("charge_manager.content.minimum_current_3p")       : __("charge_manager.content.minimum_current")}
                             label_muted={energyManagerMode ? __("charge_manager.content.minimum_current_3p_muted") : __("charge_manager.content.minimum_current_muted")}>
                        <InputFloat
                            unit="A"
                            value={state.minimum_current}
                            onValue={(v) => this.setState({
                                minimum_current: v,
                                maximum_available_current: Math.max(v, state.maximum_available_current) // TODO Is this a good idea? Should warn instead?
                            })}
                            digits={3}
                            min={6000}
                            max={32000}
                        />
                    </FormRow>

                    {energyManagerMode || warpUltimateMode ?
                        <FormRow label={__("charge_manager.content.minimum_current_1p")} label_muted={__("charge_manager.content.minimum_current_1p_muted")}>
                            <InputFloat
                                unit="A"
                                value={state.minimum_current_1p}
                                onValue={(v) => this.setState({minimum_current_1p: v})}
                                digits={3}
                                min={6000}
                                max={32000}
                            />
                        </FormRow>
                    :
                        null
                    }
                </div>
            </Collapse>
        </>

        let available_current = <FormRow label={__("charge_manager.content.maximum_available_current")}>
                <InputFloat
                    unit="A"
                    value={state.maximum_available_current}
                    onValue={(v) => this.setState({maximum_available_current: v, default_available_current: v})}
                    digits={3}
                    min={0}
                    max={1000000}
                    />
            </FormRow>

        return (
            <SubPage name="charge_manager_settings">
                <ConfigForm id="charge_manager_config_form" title={__("charge_manager.content.charge_manager_settings")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    {energyManagerMode ?
                        <>
                            {maximum_available_current}
                            {minimum_current}
                        </>
                        : <>
                            <Collapse in={!API.get("charge_manager/config").enable_charge_manager}>
                                <div>
                                    <FormRow label="">
                                        <div style="color:red">
                                            Lastmanagement-Einstellungen werden ignoriert: Wallbox steuert nur sich selbst oder ist fremdgesteuert.
                                        </div>
                                    </FormRow>
                                </div>
                            </Collapse>

                            <FormRow label={__("charge_manager.content.configuration_mode")} label_muted={__("charge_manager.content.configuration_mode_muted")}>
                                <Button className="form-control" onClick={() => this.setState({showExpert: !state.showExpert})}>
                                    {state.showExpert ? __("component.collapsed_section.hide") : __("component.collapsed_section.show")}
                                </Button>
                            </FormRow>

                            <Collapse in={state.showExpert}>
                                <div>
                                    {verbose}
                                    {watchdog}
                                    {maximum_available_current}
                                    {default_available_current}
                                    {requested_current_threshold}
                                    {requested_current_margin}
                                </div>
                            </Collapse>

                            <Collapse in={!state.showExpert}>
                                <div>{available_current}</div>
                            </Collapse>

                            {minimum_current}
                            </>
                    }
                    <FormSeparator heading={__("charge_manager.content.header_load_management")} />
                    <FormRow label="">
                        <div>
                            {__("charge_manager.content.load_management_explainer")}
                        </div>
                    </FormRow>
                </ConfigForm>
            </SubPage>
        )
    }
}
