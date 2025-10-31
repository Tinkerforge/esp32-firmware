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
import { h, Component, ComponentChild, Fragment, toChildArray } from "preact";
import { useState } from "preact/hooks";
import { translate_unchecked, __ } from "../../ts/translation";
import { FormRow } from "../../ts/components/form_row";
import { InputFloat } from "../../ts/components/input_float";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { CheckCircle, ChevronRight, Circle, Server, Sliders } from "react-feather";
//#if MODULE_ECO_AVAILABLE
import { EcoChart } from "modules/eco/main";
//#endif
import { Button, Collapse } from "react-bootstrap";

import { ConfigChargeMode } from "modules/cm_networking/config_charge_mode.enum";
import { ChargeModeButtons } from "modules/cm_networking/charge_mode_buttons";
import { InputSelect } from "ts/components/input_select";
import { ZeroPhaseDecisionTag } from "./zero_phase_decision_tag.enum";
import { ZeroPhaseDecision } from "./zero_phase_decision.union";
import { OnePhaseDecision } from "./one_phase_decision.union";
import { OnePhaseDecisionTag } from "./one_phase_decision_tag.enum";
import { ThreePhaseDecision } from "./three_phase_decision.union";
import { ThreePhaseDecisionTag } from "./three_phase_decision_tag.enum";
import { CurrentDecision } from "./current_decision.union";
import { CurrentDecisionTag } from "./current_decision_tag.enum";
import { CASState } from "./cas_state.enum";
import { CASError } from "./cas_error.enum";
import { FormSeparator } from "ts/components/form_separator";

export { ChargeManagerChargers } from "./chargers";
export { ChargeManagerSettings } from "./settings";

export function ChargeManagerSettingsNavbar() {
    return <NavbarItem name="charge_manager_settings" module="charge_manager" title={__("charge_manager.navbar.charge_manager_settings")} symbol={<Sliders />} />;
}

export function ChargeManagerChargersNavbar() {
    return <NavbarItem name="charge_manager_chargers" module="charge_manager" title={__("charge_manager.navbar.charge_manager_chargers")} symbol={<Server {...{style: "transform: rotate(180deg);"} as any} />} />;
}

export function get_allowed_charge_modes(params: {with_default: boolean, pv_enabled_override?: boolean, add_pv_if_disabled?: boolean}) {
    const pv_enabled = params.pv_enabled_override !== undefined ? params.pv_enabled_override : API.get('power_manager/config').excess_charging_enable;
    const eco_enabled = API.get_unchecked('eco/config')?.enable ?? false;

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

interface ChargeManagerStatusState {
    state: API.getType['charge_manager/state']
    available_current: API.getType['charge_manager/available_current']
    config: API.getType['charge_manager/config']
    uptime: number
}

function with_timespan(fn: (timespan: string) => string, timestamp: number) {
    const now = API.get('info/keep_alive').uptime;
    if (timestamp < now) {
        return fn(__("charge_manager.script.until_alloc"));
    }
    return fn(__("charge_manager.script.for_timespan")(util.format_timespan_ms(timestamp - now)));
}

function zero_phase_desc_to_text(d0: ZeroPhaseDecision): string {
    switch (d0[0]) {
        case ZeroPhaseDecisionTag.None:
            return __("charge_manager.script.d_none")
        case ZeroPhaseDecisionTag.YesWaitingForRotation:
            return with_timespan(__("charge_manager.script.zpd_yes_waiting_for_rotation"), d0[1]);
        case ZeroPhaseDecisionTag.YesNotActive:
            return __("charge_manager.script.zpd_yes_not_active");
        case ZeroPhaseDecisionTag.YesRotatedForB1:
            return __("charge_manager.script.zpd_yes_rotated_for_b1");
        case ZeroPhaseDecisionTag.YesRotatedForHigherPrio:
            return __("charge_manager.script.zpd_yes_rotated_for_higher_prio");
        case ZeroPhaseDecisionTag.YesPhaseOverload:
            return __("charge_manager.script.zpd_yes_phase_overload")(d0[1][1], d0[1][0]);
        case ZeroPhaseDecisionTag.YesPVExcessOverload:
            return __("charge_manager.script.zpd_yes_pv_excess_overload")(d0[1]);
        case ZeroPhaseDecisionTag.NoCloudFilterBlocksUntil:
            return with_timespan(ts => __("charge_manager.script.zpd_no_cloud_filter_blocks_until")(ts, d0[1][1]), d0[1][0]);
        case ZeroPhaseDecisionTag.NoHysteresisBlocksUntil:
            return with_timespan(__("charge_manager.script.zpd_no_hysteresis_blocks_until"), d0[1]);
    }
}

function one_phase_desc_to_text(d1: OnePhaseDecision): ComponentChild {
    switch (d1[0]) {
        case OnePhaseDecisionTag.None:
            return __("charge_manager.script.d_none");
        case OnePhaseDecisionTag.YesWelcomeChargeUntil:
            return with_timespan(__("charge_manager.script.pd_yes_welcome_charge_until"), d1[1]);
        case OnePhaseDecisionTag.NoPhaseMinimum:
            return with_timespan(ts => __("charge_manager.script.pd_no_phase_minimum")(ts, d1[1][3], d1[1][1], d1[1][2]), d1[1][0]);
        case OnePhaseDecisionTag.NoPhaseImprovement:
            return  with_timespan(ts => __("charge_manager.script.pd_no_phase_improvement")(ts, d1[1][3], d1[1][1], d1[1][2]), d1[1][0]);
        case OnePhaseDecisionTag.NoForced3pUntil:
            return with_timespan(__("charge_manager.script.opd_no_force_3p_until"), d1[1]);
        case OnePhaseDecisionTag.NoFixed3p:
            return __("charge_manager.script.opd_no_fixed_3p");
        case OnePhaseDecisionTag.YesWakingUp:
            return __("charge_manager.script.pd_yes_waking_up");
        case OnePhaseDecisionTag.YesNormal:
            return __("charge_manager.script.pd_yes");
        case OnePhaseDecisionTag.NoHysteresisBlockedUntil:
            return with_timespan(__("charge_manager.script.pd_no_hysteresis_blocks_until"), d1[1]);
        case OnePhaseDecisionTag.YesSwitchedToFixed1p:
            return __("charge_manager.script.opd_yes_switched_to_fixed_1p");
    }
}

function three_phase_desc_to_text(d3: ThreePhaseDecision): ComponentChild {
    switch (d3[0]) {
        case ThreePhaseDecisionTag.None:
            return __("charge_manager.script.d_none");
        case ThreePhaseDecisionTag.YesWelcomeChargeUntil:
            return with_timespan(__("charge_manager.script.pd_yes_welcome_charge_until"), d3[1]);
        case ThreePhaseDecisionTag.NoPhaseMinimum:
            return with_timespan(ts => __("charge_manager.script.pd_no_phase_minimum")(ts, d3[1][3], d3[1][1], d3[1][2]), d3[1][0]);
        case ThreePhaseDecisionTag.NoPhaseImprovement:
            return with_timespan(ts => __("charge_manager.script.pd_no_phase_improvement")(ts, d3[1][3], d3[1][1], d3[1][2]), d3[1][0]);
        case ThreePhaseDecisionTag.NoForced1pUntil:
            return with_timespan(__("charge_manager.script.tpd_no_force_1p_until"), d3[1]);
        case ThreePhaseDecisionTag.NoFixed1p:
            return __("charge_manager.script.tpd_no_fixed_1p")
        case ThreePhaseDecisionTag.YesUnknownRotSwitchable:
            return __("charge_manager.script.tpd_yes_unknown_rot_switchable");
        case ThreePhaseDecisionTag.NoHysteresisBlockedUntil:
            return with_timespan(__("charge_manager.script.pd_no_hysteresis_blocks_until"), d3[1]);
        case ThreePhaseDecisionTag.YesWakingUp:
            return __("charge_manager.script.pd_yes_waking_up");
        case ThreePhaseDecisionTag.YesNormal:
            return __("charge_manager.script.pd_yes");
        case ThreePhaseDecisionTag.YesSwitchedToFixed3p:
            return __("charge_manager.script.tpd_yes_switched_to_fixed_3p");
    }
}

function current_desc_to_text(dc: CurrentDecision): ComponentChild {
    switch (dc[0]) {
        case CurrentDecisionTag.None:
            return __("charge_manager.script.d_none");
        case CurrentDecisionTag.Minimum:
            return __("charge_manager.script.cd_minimum");
        case CurrentDecisionTag.EnableNotCharging:
            return __("charge_manager.script.cd_enable_not_charging");
        case CurrentDecisionTag.PhaseLimit:
            return __("charge_manager.script.cd_phase_limit");
        case CurrentDecisionTag.Requested:
            return __("charge_manager.script.cd_requested");
        case CurrentDecisionTag.Fair:
            return __("charge_manager.script.cd_fair");
        case CurrentDecisionTag.GuaranteedPV:
            return __("charge_manager.script.cd_guaranteed_pv");
        case CurrentDecisionTag.FastRampUp:
            return __("charge_manager.script.cd_fast_ramp_up");
        case CurrentDecisionTag.SupportedByCharger:
            return __("charge_manager.script.cd_supported_by_charger");
        case CurrentDecisionTag.LeftOver:
            return __("charge_manager.script.cd_left_over");
    }
}

function get_timestamp_d0(d: ZeroPhaseDecision): number {
    switch (d[0]) {
        case ZeroPhaseDecisionTag.None:
        case ZeroPhaseDecisionTag.YesNotActive:
        case ZeroPhaseDecisionTag.YesRotatedForB1:
        case ZeroPhaseDecisionTag.YesRotatedForHigherPrio:
        case ZeroPhaseDecisionTag.YesPhaseOverload:
        case ZeroPhaseDecisionTag.YesPVExcessOverload:
            return Number.MAX_SAFE_INTEGER;

        case ZeroPhaseDecisionTag.YesWaitingForRotation:
        case ZeroPhaseDecisionTag.NoHysteresisBlocksUntil:
            return d[1];
        case ZeroPhaseDecisionTag.NoCloudFilterBlocksUntil:
            return d[1][1];
    }
}

function get_timestamp_d1(d: OnePhaseDecision): number {
    switch (d[0]) {
        case OnePhaseDecisionTag.None:
        case OnePhaseDecisionTag.YesSwitchedToFixed1p:
        case OnePhaseDecisionTag.YesNormal:
        case OnePhaseDecisionTag.YesWakingUp:
        case OnePhaseDecisionTag.NoFixed3p:
            return Number.MAX_SAFE_INTEGER;


        case OnePhaseDecisionTag.YesWelcomeChargeUntil:
        case OnePhaseDecisionTag.NoForced3pUntil:
        case OnePhaseDecisionTag.NoHysteresisBlockedUntil:
            return d[1];

        case OnePhaseDecisionTag.NoPhaseMinimum:
        case OnePhaseDecisionTag.NoPhaseImprovement:
            return d[1][1];
    }
}

function get_timestamp_d3(d: ThreePhaseDecision): number {
    switch (d[0]) {
        case ThreePhaseDecisionTag.None:
        case ThreePhaseDecisionTag.YesSwitchedToFixed3p:
        case ThreePhaseDecisionTag.YesNormal:
        case ThreePhaseDecisionTag.YesUnknownRotSwitchable:
        case ThreePhaseDecisionTag.YesWakingUp:
        case ThreePhaseDecisionTag.NoFixed1p:
            return Number.MAX_SAFE_INTEGER;

        case ThreePhaseDecisionTag.YesWelcomeChargeUntil:
        case ThreePhaseDecisionTag.NoForced1pUntil:
        case ThreePhaseDecisionTag.NoHysteresisBlockedUntil:
            return d[1];

        case ThreePhaseDecisionTag.NoPhaseMinimum:
        case ThreePhaseDecisionTag.NoPhaseImprovement:
            return d[1][1];
    }
}

function get_timestamp_dc(d: CurrentDecision): number {
    switch (d[0]) {
        case CurrentDecisionTag.None:
        case CurrentDecisionTag.Minimum:
        case CurrentDecisionTag.EnableNotCharging:
        case CurrentDecisionTag.PhaseLimit:
        case CurrentDecisionTag.Requested:
        case CurrentDecisionTag.Fair:
        case CurrentDecisionTag.GuaranteedPV:
        case CurrentDecisionTag.FastRampUp:
        case CurrentDecisionTag.SupportedByCharger:
        case CurrentDecisionTag.LeftOver:
            return Number.MAX_SAFE_INTEGER;
    }
}


function alloc_decision_to_text(x: API.getType['charge_manager/state']['chargers'][0]): [ComponentChild[], ComponentChild] {
    let decs: [ZeroPhaseDecision, OnePhaseDecision, ThreePhaseDecision] = [undefined, undefined, undefined];

    let details = [<div>{__("charge_manager.script.off")}: {zero_phase_desc_to_text(x.d0)}</div>,
                   <div>1p: {one_phase_desc_to_text(x.d1)}</div>,
                   <div>3p: {three_phase_desc_to_text(x.d3)}</div>,
                   <div>{__("charge_manager.script.current")}: {current_desc_to_text(x.dc)}</div>]

    switch (x.ap) {
        case 0:
            switch (x.d0[0]) {
                case ZeroPhaseDecisionTag.None:
                case ZeroPhaseDecisionTag.NoCloudFilterBlocksUntil:
                case ZeroPhaseDecisionTag.NoHysteresisBlocksUntil:
                case ZeroPhaseDecisionTag.YesNotActive:
                    // Hide
                    break;

                case ZeroPhaseDecisionTag.YesRotatedForB1:
                case ZeroPhaseDecisionTag.YesRotatedForHigherPrio:
                case ZeroPhaseDecisionTag.YesPhaseOverload:
                case ZeroPhaseDecisionTag.YesPVExcessOverload:
                case ZeroPhaseDecisionTag.YesWaitingForRotation:
                    decs[0] = x.d0;
                    break;
            }
            switch (x.d1[0]) {
                case OnePhaseDecisionTag.None:
                case OnePhaseDecisionTag.YesSwitchedToFixed1p:
                case OnePhaseDecisionTag.YesNormal:
                case OnePhaseDecisionTag.YesWelcomeChargeUntil:
                case OnePhaseDecisionTag.YesWakingUp:
                case OnePhaseDecisionTag.NoFixed3p:
                    // Hide
                    break;

                case OnePhaseDecisionTag.NoPhaseMinimum:
                case OnePhaseDecisionTag.NoPhaseImprovement:
                case OnePhaseDecisionTag.NoForced3pUntil:
                case OnePhaseDecisionTag.NoHysteresisBlockedUntil:
                    decs[1] = x.d1;
                    break;
            }
            switch (x.d3[0]) {
                case ThreePhaseDecisionTag.None:
                case ThreePhaseDecisionTag.YesSwitchedToFixed3p:
                case ThreePhaseDecisionTag.YesNormal:
                case ThreePhaseDecisionTag.YesWelcomeChargeUntil:
                case ThreePhaseDecisionTag.YesWakingUp:
                case ThreePhaseDecisionTag.NoFixed1p:
                    // Hide
                    break;

                case ThreePhaseDecisionTag.NoPhaseMinimum:
                case ThreePhaseDecisionTag.NoPhaseImprovement:
                case ThreePhaseDecisionTag.NoForced1pUntil:
                case ThreePhaseDecisionTag.NoHysteresisBlockedUntil:
                    decs[2] = x.d3;
                    break;
            }
            break;
        case 1:
            switch (x.d0[0]) {
                case ZeroPhaseDecisionTag.None:
                case ZeroPhaseDecisionTag.YesNotActive:
                case ZeroPhaseDecisionTag.YesRotatedForB1:
                case ZeroPhaseDecisionTag.YesRotatedForHigherPrio:
                case ZeroPhaseDecisionTag.YesPhaseOverload:
                case ZeroPhaseDecisionTag.YesPVExcessOverload:
                case ZeroPhaseDecisionTag.YesWaitingForRotation:
                    // Hide
                    break;


                case ZeroPhaseDecisionTag.NoCloudFilterBlocksUntil:
                case ZeroPhaseDecisionTag.NoHysteresisBlocksUntil:
                    decs[0] = x.d0;
                    break;
            }
            switch (x.d1[0]) {
                case OnePhaseDecisionTag.None:
                case OnePhaseDecisionTag.NoFixed3p:
                case OnePhaseDecisionTag.NoPhaseMinimum:
                case OnePhaseDecisionTag.NoPhaseImprovement:
                case OnePhaseDecisionTag.NoForced3pUntil:
                case OnePhaseDecisionTag.NoHysteresisBlockedUntil:
                case OnePhaseDecisionTag.YesNormal:
                    // Hide
                    break;

                case OnePhaseDecisionTag.YesSwitchedToFixed1p:
                case OnePhaseDecisionTag.YesWelcomeChargeUntil:
                case OnePhaseDecisionTag.YesWakingUp:
                    decs[1] = x.d1;
                    break;
            }
            switch (x.d3[0]) {
                case ThreePhaseDecisionTag.None:
                case ThreePhaseDecisionTag.YesSwitchedToFixed3p:
                case ThreePhaseDecisionTag.YesNormal:
                case ThreePhaseDecisionTag.YesWelcomeChargeUntil:
                case ThreePhaseDecisionTag.YesWakingUp:
                case ThreePhaseDecisionTag.NoFixed1p:
                    // Hide
                    break;

                case ThreePhaseDecisionTag.NoPhaseMinimum:
                case ThreePhaseDecisionTag.NoPhaseImprovement:
                case ThreePhaseDecisionTag.NoForced1pUntil:
                case ThreePhaseDecisionTag.NoHysteresisBlockedUntil:
                    decs[2] = x.d3;
                    break;
            }
            break;
        case 3:
            switch (x.d0[0]) {
                case ZeroPhaseDecisionTag.None:
                case ZeroPhaseDecisionTag.YesNotActive:
                case ZeroPhaseDecisionTag.YesRotatedForB1:
                case ZeroPhaseDecisionTag.YesRotatedForHigherPrio:
                case ZeroPhaseDecisionTag.YesPhaseOverload:
                case ZeroPhaseDecisionTag.YesPVExcessOverload:
                case ZeroPhaseDecisionTag.YesWaitingForRotation:
                    // Hide
                    break;


                case ZeroPhaseDecisionTag.NoCloudFilterBlocksUntil:
                case ZeroPhaseDecisionTag.NoHysteresisBlocksUntil:
                    decs[0] = x.d0;
                    break;
            }
            switch (x.d1[0]) {
                case OnePhaseDecisionTag.None:
                case OnePhaseDecisionTag.NoFixed3p:
                case OnePhaseDecisionTag.YesSwitchedToFixed1p:
                case OnePhaseDecisionTag.YesNormal:
                case OnePhaseDecisionTag.YesWelcomeChargeUntil:
                case OnePhaseDecisionTag.YesWakingUp:
                    // Hide
                    break;

                case OnePhaseDecisionTag.NoPhaseMinimum:
                case OnePhaseDecisionTag.NoPhaseImprovement:
                case OnePhaseDecisionTag.NoForced3pUntil:
                case OnePhaseDecisionTag.NoHysteresisBlockedUntil:
                    decs[1] = x.d1;
                    break;
            }
            switch (x.d3[0]) {
                case ThreePhaseDecisionTag.None:
                case ThreePhaseDecisionTag.NoPhaseMinimum:
                case ThreePhaseDecisionTag.NoPhaseImprovement:
                case ThreePhaseDecisionTag.NoForced1pUntil:
                case ThreePhaseDecisionTag.NoHysteresisBlockedUntil:
                case ThreePhaseDecisionTag.NoFixed1p:
                case ThreePhaseDecisionTag.YesNormal:
                    // Hide
                    break;

                case ThreePhaseDecisionTag.YesSwitchedToFixed3p:
                case ThreePhaseDecisionTag.YesWelcomeChargeUntil:
                case ThreePhaseDecisionTag.YesWakingUp:
                    decs[2] = x.d3;
                    break;
            }
            break;
    }

    let timestamps = [Number.MAX_SAFE_INTEGER, Number.MAX_SAFE_INTEGER, Number.MAX_SAFE_INTEGER]

    if (decs[0] !== undefined)
        timestamps[0] = get_timestamp_d0(decs[0])
    if (decs[1] !== undefined)
        timestamps[1] = get_timestamp_d1(decs[1])
    if (decs[2] !== undefined)
        timestamps[2] = get_timestamp_d3(decs[2])

    let dec_tups = decs.map((d, i) => {return {d: d, i:i, ts:timestamps[i]}}).filter(tup => tup.d !== undefined)

    let idx = 0;

    if (dec_tups.some(tup => tup.ts < Number.MAX_SAFE_INTEGER)) {
        idx = dec_tups.reduce((acc, cur) => cur.ts < acc.ts ? cur : acc).i;

    } else if (dec_tups.length > 0) {
        idx = dec_tups[0].i;

    } else {
        return [[], details];
    }

    let d = [details[idx]];
    if (x.dc[0] != CurrentDecisionTag.None)
        d = d.concat(details[3]);
    return [d, details];
}

function CMStatusCharger(props: {
        show_eco_chart: boolean,
        uptime: number,
        charge_manager_state: number,
        default_mode: ConfigChargeMode,
        charger_count: number,
        charger_index: number,
        charge_mode: ConfigChargeMode,
        charger_state: API.getType['charge_manager/state']['chargers'][0],
        charger_config: API.getType['charge_manager/config']['chargers'][0]
    }) {
    const [showDetails, setShowDetails] = useState(false);

    let c_state = "";
    let c_info = "";
    let c_body_classes = "";

    const c = props.charger_state

    let last_update = Math.floor((props.uptime - c.lu) / 1000);

    let cur = c.ac / 1000.0;
    let p = c.ap;
    let kw = util.toLocaleFixed(cur * p * 230 / 1000.0, 3);

    let c_status_text = `${util.toLocaleFixed(cur, 3)} A @ ${p}p = ${kw} kW ${__("charge_manager.script.ampere_allocated")}`;

    if (last_update >= 10)
        c_status_text += "; " + __("charge_manager.script.last_update_prefix") + " " + util.format_timespan(last_update) + (__("charge_manager.script.last_update_suffix"));

    if (c.s != CASState.Error) {
        if (props.charge_manager_state == 2) {
            c_body_classes = "bg-warning bg-disabled";
            c_state = __("charge_manager.script.charge_state_blocked_by_other_box");
            c_info = __("charge_manager.script.charge_state_blocked_by_other_box_details");
        } else {
            c_state = __("charge_manager.script.charge_state")(c.s);
        }
    }
    else {
        if (c.e < CASError.ClientErrorOK)
            c_state = __("charge_manager.script.charge_error_type_management");
        else
            c_state = __("charge_manager.script.charge_error_type_client");

        c_body_classes = "bg-danger text-white bg-disabled";
        c_info = __("charge_manager.script.charge_error")(c.e);
    }

    let charger_config = props.charger_config;
    const name_link = (API.is_dirty("charge_manager/config") || !charger_config || util.remoteAccessMode) ? c.n :
                        <a target="_blank" rel="noopener noreferrer" href={(charger_config.host == '127.0.0.1' || charger_config.host == 'localhost') ? '/' : "http://" + charger_config.host}>{c.n}</a>

    let [desc, details] = alloc_decision_to_text(c)
    if (c_info != "")
        desc = [c_info];

    let supported_charge_modes = API.get("charge_manager/supported_charge_modes")
    let modes: [string, string][] = supported_charge_modes.map(x => [x.toString(), __("cm_networking.status.mode_by_index")(x, props.default_mode)]);
    let mode = props.charge_mode
    let mode_found = supported_charge_modes.indexOf(mode) >= 0;

    return  <div class={"card " + (props.charger_index + 1 == props.charger_count ? "mb-0" : "")}>
                <div class="card-header">
                    <div class="row align-items-center mx-n1">
                        <div class="col px-1"><h5 class="m-0">{name_link}</h5></div>
                        <div class="col-auto px-1">
                            <InputSelect
                                items={modes}
                                value={mode.toString()}
                                placeholder={mode_found ? "" : __("cm_networking.status.mode_by_index")(mode, props.default_mode)}
                                onValue={x => API.call_with_path("charge_manager/charge_modes", props.charger_index, parseInt(x))}
                                />
                        </div>
                    </div>
                </div>
                <div class={"card-body " + c_body_classes}>
                    <h5 class={"card-title my-0"}><Button
                        className="mr-2"
                        size="sm"
                        onClick={() => setShowDetails(!showDetails)}>
                        <ChevronRight {...{class: showDetails ? "rotated-chevron" : "unrotated-chevron"} as any}/>
                    </Button>{c_state}</h5>
                    <Collapse in={!showDetails}>
                        <div>
                            <div class={"card-text"} style={desc.length > 0 ? "margin-top: 0.75rem;" : ""}>
                                {desc}
                            </div>
                        </div>
                    </Collapse>
                    <Collapse in={showDetails}>
                        <div>
                            <div class={"card-text"} style="margin-top: 0.75rem;">
                                {details}
                            </div>
                        </div>
                    </Collapse>
{/*#if MODULE_ECO_AVAILABLE*/}
                    <Collapse in={props.show_eco_chart}>
                        <div><div class="mt-3">
                            <EcoChart visible={props.show_eco_chart} charger_id={props.charger_index} />
                        </div></div>
                    </Collapse>
{/*#endif*/}
                </div>
                <div class="card-footer">
                    <span>{c_status_text}</span>
                </div>
            </div>
}

function render_allowed() {
    return util.render_allowed()
         && API.get("charge_manager/config").enable_charge_manager
         && API.get("charge_manager/config").chargers.length != 0;
}

export function ChargeManagerHeading() {
    if (!render_allowed())
        return <StatusSection name="charge_manager_heading" />;

    return <StatusSection name="charge_manager_heading">
        <FormSeparator heading={__("charge_manager.status.charge_manager")}/>
    </StatusSection>
}

export function ChargeManagerModeButtons() {
    if (!render_allowed())
        return <StatusSection name="charge_manager_mode_buttons" />;

    let current_mode = API.get("power_manager/charge_mode").mode
    let modes = API.get("charge_manager/charge_modes")

    return <StatusSection name="charge_manager_mode_buttons">
        <FormRow label={__("charge_manager.status.mode")} label_muted={__("charge_manager.status.mode_muted")}>
            <ChargeModeButtons
                mode={current_mode}
                supportedModes={API.get("charge_manager/supported_charge_modes")}
                setMode={mode => API.save('power_manager/charge_mode', {"mode": mode}, () => __("charge_manager.script.mode_change_failed"))}
                modeEnabled={modes.some(m => m != current_mode)}
            />
        </FormRow>
    </StatusSection>
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
        if (!render_allowed())
            return <StatusSection name="charge_manager" />;

        let eco_enabled = false;

//#if MODULE_ECO_AVAILABLE
        // Even if the charge plan is disabled, the eco module can decide to charge if the electricity price is under a threshold
        eco_enabled = API.get("eco/config").enable
//#endif
        const default_mode = API.get("power_manager/config").default_mode;
        const charge_modes = API.get("charge_manager/charge_modes");
        const uptime = API.get("info/keep_alive").uptime;

        let cards = state.state.chargers.map((c, i) =>
            <CMStatusCharger
                show_eco_chart={eco_enabled && charge_modes[i] >= ConfigChargeMode.Eco && charge_modes[i] <= ConfigChargeMode.EcoMinPV}
                uptime={uptime}
                charge_manager_state={state.state.state}
                default_mode={default_mode}
                charger_count={state.config.chargers.length}
                charger_index={i}
                charge_mode={charge_modes[i]}
                charger_state={c}
                charger_config={state.config.chargers[i]}
            />);

        let controls_only_self = false && (state.config.chargers.length == 1
                       && (state.config.chargers[0].host == '127.0.0.1' || state.config.chargers[0].host == 'localhost'));
        let row_count = Math.ceil(cards.length / 2);

        return <StatusSection name="charge_manager">

            {state.state.state != 2 ? null :
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
