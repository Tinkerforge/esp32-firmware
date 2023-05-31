/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

import * as API  from "../../ts/api";
import * as util from "../../ts/util";
import { __ }    from "../../ts/translation";

import { h, render, Fragment, Component } from "preact";
import { DebugLogger    } from "../../ts/components/debug_logger";
import { FormRow        } from "../../ts/components/form_row";
import { FormSeparator  } from "../../ts/components/form_separator";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { InputText      } from "../../ts/components/input_text";
import { OutputFloat    } from "../../ts/components/output_float";
import { PageHeader     } from "../../ts/components/page_header";

export class EMDebug extends Component {
    ms2time(ms: number) {
        let s = ms / 1000;
        let m = Math.trunc(s / 60);
        s = Math.trunc(s % 60);

        return m.toString() + "m " + s.toString() + "s";
    }

    render() {
        if (!util.render_allowed() || !API.hasFeature("energy_manager"))
            return <></>

        let ll_state = API.get('energy_manager/low_level_state');
        let state    = API.get('energy_manager/state');

        return (
            <>
                <PageHeader title={__("em_debug.content.em_debug")} colClasses="col-xl-10"/>

                <FormSeparator heading={__("em_debug.content.protocol")} />
                <DebugLogger prefix="energy_manager" translationPrefix="em_debug" debugHeader="energy_manager/debug_header" debug="energy_manager/debug" />

                <FormSeparator heading={__("em_debug.content.internal_state")} />
                <FormRow label="power at meter smooth">
                    <OutputFloat value={ll_state.power_at_meter} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="power at meter filtered">
                    <OutputFloat value={ll_state.power_at_meter_filtered} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="power available">
                    <OutputFloat value={ll_state.power_available} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="power available filtered">
                    <OutputFloat value={ll_state.power_available_filtered} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="overall min power">
                    <OutputFloat value={ll_state.overall_min_power} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="threshold 3to1">
                    <OutputFloat value={ll_state.threshold_3to1} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="threshold 1to3">
                    <OutputFloat value={ll_state.threshold_1to3} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="CM available current">
                    <OutputFloat value={ll_state.charge_manager_available_current} digits={3} scale={3} unit={'A'} />
                </FormRow>
                <FormRow label="CM allocated current">
                    <OutputFloat value={ll_state.charge_manager_allocated_current} digits={3} scale={3} unit={'A'} />
                </FormRow>
                <FormRow label="max current limited">
                    <OutputFloat value={ll_state.max_current_limited} digits={3} scale={3} unit={'A'} />
                </FormRow>
                <FormRow label="uptime past hysteresis">
                    <IndicatorGroup
                        value={ll_state.uptime_past_hysteresis ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="is 3phase">
                    <IndicatorGroup
                        value={ll_state.is_3phase ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="wants 3phase">
                    <IndicatorGroup
                        value={ll_state.wants_3phase ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="wants 3phase last">
                    <IndicatorGroup
                        value={ll_state.wants_3phase_last ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="is on last">
                    <IndicatorGroup
                        value={ll_state.is_on_last ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="wants on last">
                    <IndicatorGroup
                        value={ll_state.wants_on_last ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="phase state change blocked">
                    <IndicatorGroup
                        value={ll_state.phase_state_change_blocked ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="phase state change delay">
                    <InputText value={this.ms2time(ll_state.phase_state_change_delay)}/>
                </FormRow>
                <FormRow label="on state change blocked">
                    <IndicatorGroup
                        value={ll_state.on_state_change_blocked ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="on state change delay">
                    <InputText value={this.ms2time(ll_state.on_state_change_delay)}/>
                </FormRow>
                <FormRow label="charging blocked">
                    <InputText value={"0x" + ll_state.charging_blocked.toString(16)}/>
                </FormRow>
                <FormRow label="switching state">
                    <InputText value={ll_state.switching_state}/>
                </FormRow>
                <FormRow label="consecutive bricklet errors">
                    <InputText value={ll_state.consecutive_bricklet_errors}/>
                </FormRow>

                <FormSeparator heading={__("em_debug.content.hardware_state")} />
                <FormRow label={__("em_debug.content.contactor_control")}>
                    <IndicatorGroup
                        value={ll_state.contactor ? 1 : 0}
                        items={[
                            ["secondary", __("em_debug.content.contactor_off")],
                            ["primary", __("em_debug.content.contactor_on")],
                        ]} />
                </FormRow>

                <FormRow label={__("em_debug.content.contactor_check")}>
                    <IndicatorGroup
                        value={ll_state.contactor_check_state ? 0 : 1} // intentionally inverted, OK is first
                        items={[
                            ["success", __("em_debug.content.contactor_check_ok")],
                            ["danger", __("em_debug.content.contactor_check_fail")],
                        ]} />
                </FormRow>

                <FormRow label={__("em_debug.content.state_led")} label_muted={__("em_debug.content.state_led_names")}>
                    <div class="row mx-n1">
                        {ll_state.led_rgb.map((x, i) => (
                            <div key={i} class="mb-1 col-4 px-1">
                                <InputText value={x} />
                            </div>
                        ))}
                    </div>
                </FormRow>

                <FormRow label={__("em_debug.content.gpios")} label_muted={__("em_debug.content.gpio_names_0")}>
                    <div class="row mx-n1">
                        {[state.input3_state, state.input4_state, state.relay_state].map((x, j) => (
                            <IndicatorGroup vertical key={j} class="mb-1 col px-1"
                                value={x ? 0 : 1} //intentionally inverted: the high button is the first
                                items={[
                                    ["primary", __("em_debug.content.high")],
                                    ["secondary", __("em_debug.content.low")]
                                ]} />
                        ))}
                    </div>
                </FormRow>

                <FormRow label={__("em_debug.content.state_input_voltage")}>
                    <OutputFloat value={ll_state.input_voltage} digits={3} scale={3} unit={'V'} />
                </FormRow>
            </>
        )
    }
}

render(<EMDebug/>, $('#em_debug')[0]);

export function init() {}
export function add_event_listeners(source: API.APIEventTarget) {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-em_debug').prop('hidden', !module_init.energy_manager);
}
