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

//#include "module_available.inc"

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { h, Component } from "preact";
import { translate_unchecked, __ } from "../../ts/translation";
import { FormRow } from "../../ts/components/form_row";
import { InputFloat } from "../../ts/components/input_float";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { CheckCircle, Circle, Server, Sliders } from "react-feather";
//#if MODULE_ECO_AVAILABLE
import { EcoChart } from "modules/eco/main";
//#endif
import { ButtonGroup, Button, Collapse } from "react-bootstrap";

import { ConfigChargeMode } from "./config_charge_mode.enum";
import { AllocatorDecision } from "./allocator_decision.enum";
import { GlobalAllocatorDecision } from "./global_allocator_decision.enum";
import { InputText } from "ts/components/input_text";

export { ChargeManagerChargers } from "./chargers";
export { ChargeManagerSettings } from "./settings";

export function ChargeManagerSettingsNavbar() {
    return <NavbarItem name="charge_manager_settings" module="charge_manager" title={__("charge_manager.navbar.charge_manager_settings")} symbol={<Sliders />} />;
}

export function ChargeManagerChargersNavbar() {
    return <NavbarItem name="charge_manager_chargers" module="charge_manager" title={__("charge_manager.navbar.charge_manager_chargers")} symbol={<Server {...{style: "transform: rotate(180deg);"} as any} />} />;
}

function change_charge_mode(mode: number) {
    API.save('power_manager/charge_mode', {"mode": mode}, () => __("charge_manager.script.mode_change_failed"));
}

export function get_allowed_charge_modes(params: {with_default: boolean, pv_enabled_override?: boolean, eco_enabled_override?: boolean, add_pv_if_disabled?: boolean}) {
    const pv_enabled = params.pv_enabled_override !== undefined ? params.pv_enabled_override : API.get('power_manager/config').excess_charging_enable;
    const eco_enabled = params.eco_enabled_override !== undefined ? params.eco_enabled_override : API.get_unchecked('eco/config')?.enable ?? false;

    /*
        PV Eco  Buttons
        0   0   PV, Min+PV
        0   1   Eco, Min+Eco
        1   0   PV, Min+PV
        1   1   PV, Eco+PV
    */

    let allowed_modes: ConfigChargeMode[] = [ConfigChargeMode.Off];
    if (!pv_enabled && !eco_enabled && params.add_pv_if_disabled)
        allowed_modes.push(ConfigChargeMode.PV, ConfigChargeMode.MinPV);
    else if (!pv_enabled && eco_enabled)
        allowed_modes.push(ConfigChargeMode.Eco, ConfigChargeMode.EcoMin);
    else if (pv_enabled && !eco_enabled)
        allowed_modes.push(ConfigChargeMode.PV, ConfigChargeMode.MinPV);
    else if (pv_enabled && eco_enabled)
        allowed_modes.push(ConfigChargeMode.PV, ConfigChargeMode.EcoPV);

    allowed_modes.push(ConfigChargeMode.Fast);
    if (params.with_default)
        allowed_modes.push(ConfigChargeMode.Default);

    return allowed_modes;
}

function ChargeModeButton(props: {current_mode: ConfigChargeMode, disabled?: boolean, mode: ConfigChargeMode, name: string}) {
    return <Button
        style="display: flex;align-items: center;justify-content: center;"
        className="m-1 rounded-left rounded-right"
        variant={props.disabled ? "secondary" : (props.current_mode == props.mode ? "success" : "primary")}
        disabled={props.disabled || props.current_mode == props.mode}
        onClick={() => change_charge_mode(props.mode)}>
        {props.disabled || props.current_mode != props.mode ? <Circle size="20"/> : <CheckCircle size="20"/>} <span>&nbsp;&nbsp;</span><span>{props.name}</span>
    </Button>
}

export function ChargeModeButtons() {
    if (!util.render_allowed() || !API.get('charge_manager/config').enable_charge_manager)
        return <StatusSection name="charge_manager_charge_mode_buttons"></StatusSection>

    const mode = API.get('power_manager/charge_mode').mode;
    const pv_enabled = API.get('power_manager/config').excess_charging_enable;
    const eco_enabled = API.get_unchecked('eco/config')?.enable ?? false;

    const buttons = [
        <ChargeModeButton current_mode={mode} mode={ConfigChargeMode.Fast} name={__("charge_manager.status.mode_fast")}/>,
        <ChargeModeButton current_mode={mode} mode={ConfigChargeMode.Off} name={__("charge_manager.status.mode_off")}/>,
        <ChargeModeButton current_mode={mode} mode={ConfigChargeMode.PV} name={__("charge_manager.status.mode_pv")} disabled={!pv_enabled}/>,
        <ChargeModeButton current_mode={mode} mode={ConfigChargeMode.MinPV} name={__("charge_manager.status.mode_min_pv")} disabled={!pv_enabled}/>,
        null, //<ChargeModeButton current_mode={mode} mode={ConfigChargeMode.Default} name={}/>,
        <ChargeModeButton current_mode={mode} mode={ConfigChargeMode.Min} name={__("charge_manager.status.mode_min")}/>,
        <ChargeModeButton current_mode={mode} mode={ConfigChargeMode.Eco} name={__("charge_manager.status.mode_eco")} disabled={!eco_enabled}/>,
        <ChargeModeButton current_mode={mode} mode={ConfigChargeMode.EcoPV} name={__("charge_manager.status.mode_eco_pv")} disabled={!eco_enabled || !pv_enabled}/>,
        <ChargeModeButton current_mode={mode} mode={ConfigChargeMode.EcoMin} name={__("charge_manager.status.mode_eco_min")} disabled={!eco_enabled}/>,
        <ChargeModeButton current_mode={mode} mode={ConfigChargeMode.EcoMinPV} name={__("charge_manager.status.mode_eco_min_pv")} disabled={!eco_enabled || !pv_enabled}/>,
    ];
    let button_indices: ConfigChargeMode[] = get_allowed_charge_modes({with_default: false, add_pv_if_disabled: true});

    // Add another button if another charge mode was selected via the API.
    if (mode >= ConfigChargeMode._min && mode <= ConfigChargeMode._max && button_indices.indexOf(mode) < 0)
        button_indices.push(mode);

    let used_buttons = button_indices.map(i => buttons[i]);

    return <StatusSection name="charge_manager_charge_mode_buttons">
        <FormRow label={__("charge_manager.status.mode")}>
            <ButtonGroup className="flex-wrap m-n1" style="width: calc(100% + 0.5rem);">
                {used_buttons}
            </ButtonGroup>
        </FormRow>
    </StatusSection>
}

interface ChargeManagerStatusState {
    state: API.getType['charge_manager/state']
    available_current: API.getType['charge_manager/available_current']
    config: API.getType['charge_manager/config']
    uptime: number
}

function with_timespan(fn: (timespan: string) => string, x: {d1: number, d2: number}) {
    const ts = x.d1 * Math.pow(2, 32) + x.d2;
    const now = API.get('info/keep_alive').uptime / 1000;
    if (ts < now)
        return ""
    return fn(util.format_timespan(ts - now));
}

function alloc_decision_to_text(x: API.getType['charge_manager/state']['chargers'][0]): string {
    switch (x.d) {
        case AllocatorDecision.WaitingForRotation0: return __("charge_manager.script.waiting_for_rotation");
        case AllocatorDecision.ShuttingDownUnknown0: return __("charge_manager.script.shutting_down_unknown");
        case AllocatorDecision.ShuttingDownNotActive0: return __("charge_manager.script.shutting_down_not_active");
        case AllocatorDecision.ShuttingDownRotatedForB10: return __("charge_manager.script.shutting_down_rotated_for_b1");
        case AllocatorDecision.ShuttingDownRotatedForHigherPrio0: return __("charge_manager.script.shutting_down_rotated_for_higher_prio");
        case AllocatorDecision.ShuttingDownOffOrError0: return __("charge_manager.script.shutting_down_off_or_error");
        case AllocatorDecision.WelcomeChargeUntil2: return with_timespan(__("charge_manager.script.welcome_charge_for"), x);
        case AllocatorDecision.ShuttingDownPhaseOverload2: return __("charge_manager.script.shutting_down_phase_overload")(x.d1, x.d2);
        case AllocatorDecision.CantActivatePhaseMinimum3: return __("charge_manager.script.cant_activate_phase_minimum")(x.d1, x.d2, x.d3);
        case AllocatorDecision.Activating1: return __("charge_manager.script.activating")(x.d1);
        case AllocatorDecision.PhaseSwitching0: return __("charge_manager.script.phase_switching");
        case AllocatorDecision.PhaseSwitchingUnblockedAt2: return with_timespan(__("charge_manager.script.phase_switching_unblocked_at"), x);
        case AllocatorDecision.WakingUp0: return __("charge_manager.script.waking_up");
        case AllocatorDecision.None0: return "";
    }
}

function global_alloc_decision_to_text(x: API.getType['charge_manager/state']): string {
    switch (x.d) {
        case GlobalAllocatorDecision.None0: return "";
        case GlobalAllocatorDecision.NextRotationAt2: return with_timespan(__("charge_manager.script.next_rotation_at"), x);
        case GlobalAllocatorDecision.PVExcessOverloadedHysteresisElapsesAt3: return with_timespan((ts) => __("charge_manager.script.pv_excess_overloaded_hysteresis_not_elapsed")(x.d3, ts), x);
        case GlobalAllocatorDecision.HysteresisElapsesAt2: return with_timespan(__("charge_manager.script.hysteresis_elapses_at"), x);
    }
}

export class ChargeManagerStatus extends Component<{}, ChargeManagerStatusState> {
    constructor() {
        super();

        util.addApiEventListener('charge_manager/state', () => {
            this.setState({state: API.get('charge_manager/state')})
        });

        util.addApiEventListener('charge_manager/available_current', () => {
            this.setState({available_current: API.get('charge_manager/available_current')})
        });

        util.addApiEventListener('charge_manager/config', () => {
            this.setState({config: API.get('charge_manager/config')})
        });

        util.addApiEventListener('info/keep_alive', () => {
            this.setState({uptime: API.get('info/keep_alive').uptime})
        });
    }

    render(props: {}, state: Readonly<ChargeManagerStatusState>) {
        if (!util.render_allowed()
         || !state.config.enable_charge_manager
         || state.config.chargers.length == 0)
            return <StatusSection name="charge_manager" />;

//#if MODULE_ECO_AVAILABLE
        let cm_eco = API.get("power_manager/charge_mode").mode >= ConfigChargeMode.Eco && API.get("power_manager/charge_mode").mode <= ConfigChargeMode.EcoMinPV;
        let show_eco_chart = cm_eco && API.get("eco/config").enable && API.get("eco/charge_plan").enable;
//#endif

        let cards = state.state.chargers.map((c, i) => {
            let c_state = "";
            let c_info = "";
            let c_body_classes = "";

            let last_update = Math.floor((state.uptime - c.lu) / 1000);
            let c_status_text = `${util.toLocaleFixed(c.sc / 1000.0, 3)} A @ ${((c.sp & 4) != 0) ? "3p/1" : c.sp}p ${__("charge_manager.script.ampere_supported")}`;

            if (last_update >= 10)
                c_status_text += "; " + __("charge_manager.script.last_update_prefix") + " " + util.format_timespan(last_update) + (__("charge_manager.script.last_update_suffix"));

            if (c.s != 5) {
                if (state.state.state == 2) {
                    c_body_classes = "bg-warning bg-disabled";
                    c_state = __("charge_manager.script.charge_state_blocked_by_other_box");
                    c_info = __("charge_manager.script.charge_state_blocked_by_other_box_details");
                } else {
                    c_state = translate_unchecked(`charge_manager.script.charge_state_${c.s}`);
                    let cur = c.ac / 1000.0;
                    let p = c.ap;
                    let kw = util.toLocaleFixed(cur * p * 230 / 1000.0, 3);

                    c_info = `${util.toLocaleFixed(cur, 3)} A @ ${p}p = ${kw} kW ${__("charge_manager.script.ampere_allocated")}`
                }
            }
            else {
                if (c.e < 192)
                    c_state = __("charge_manager.script.charge_error_type_management");
                else
                    c_state = __("charge_manager.script.charge_error_type_client");

                c_body_classes = "bg-danger text-white bg-disabled";
                c_info = translate_unchecked(`charge_manager.script.charge_error_${c.e}`);
            }

            let charger_config = state.config.chargers[i];
            const name_link = (API.is_dirty("charge_manager/config") || !charger_config || util.remoteAccessMode) ? c.n :
                                <a target="_blank" rel="noopener noreferrer" href={(charger_config.host == '127.0.0.1' || charger_config.host == 'localhost') ? '/' : "http://" + charger_config.host}>{c.n}</a>

            return  <div class={"card " + (i + 1 == state.config.chargers.length ? "mb-0" : "")}>
                        <h5 class="card-header">
                            {name_link}
                        </h5>
                        <div class={"card-body " + c_body_classes}>
                            <h5 class="card-title">{c_state}</h5>
                            <span class="card-text">{c_info}</span>
                            <div class="card-text">
                                {alloc_decision_to_text(c)}
                            </div>
{/*#if MODULE_ECO_AVAILABLE*/}
                            <Collapse in={show_eco_chart}>
                                <div><div class="mt-3">
                                    <EcoChart charger_id={i} />
                                </div></div>
                            </Collapse>
{/*#endif*/}
                        </div>
                        <div class="card-footer">
                            <span>{c_status_text}</span>
                        </div>
                    </div>
        });

        let controls_only_self = false && (state.config.chargers.length == 1
                       && (state.config.chargers[0].host == '127.0.0.1' || state.config.chargers[0].host == 'localhost'));
        let row_count = Math.ceil(cards.length / 2);

        return <StatusSection name="charge_manager">
            <FormRow label="charge_manager_info"><InputText value={global_alloc_decision_to_text(state.state)}/></FormRow>

            {controls_only_self && API.get_unchecked("power_manager/config")?.enabled ? null :
                <FormRow label={__("charge_manager.status.charge_manager")}>
                    <IndicatorGroup
                        style="width: 100%"
                        class="flex-wrap"
                        value={state.state.state}
                        items={[
                            ["primary", __("charge_manager.status.not_configured")],
                            ["success", __("charge_manager.status.manager")],
                            ["danger", __("charge_manager.status.error")]
                        ]}/>
                </FormRow>
            }

            {// The power manager controls the available current.
                API.get_unchecked("power_manager/config")?.enabled ? null:
                <FormRow label={__("charge_manager.status.available_current")}>
                    <InputFloat min={0} max={state.config.maximum_available_current} digits={3} unit="A"
                        value={state.available_current.current}
                        onValue={(v) => API.save("charge_manager/available_current", {"current": v}, () => __("charge_manager.script.set_available_current_failed"))}
                        showMinMax/>
                </FormRow>
            }

            {controls_only_self ? null :
                <FormRow label={__("charge_manager.status.managed_boxes")}>
                    {util.range(row_count).map(i =>
                        <div class={"card-deck" + (i + 1 < row_count ? " mb-3" : "")}>
                            {cards[2 * i]}
                            {cards.length > (2 * i + 1) ? cards[2 * i + 1] : undefined}
                        </div>)
                    }
                </FormRow>
            }
        </StatusSection>;
    }
}

export function init() {
}
