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

interface EMDebugState {
    low_level_state: API.getType['energy_manager/low_level_state'];
    state: API.getType['energy_manager/state'];
    debug_running: boolean;
    debug_status: string;
}

export class EMDebug extends Component<{}, EMDebugState> {
    debug_log = "";

    constructor() {
        super();

        util.eventTarget.addEventListener('energy_manager/low_level_state', () => {
            this.setState({low_level_state: API.get('energy_manager/low_level_state')});
        });

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

    render(props: {}, s: Readonly<EMDebugState>) {
        // Must check presence of low_level_state here because an update to /state might trigger
        // a render when no bricklet is available and /state is the only exported state.
        if (!util.allow_render || !s.low_level_state) {
            return (<></>);
        }

        return (
            <>
                <PageHeader title={__("em_debug.content.em_debug")} colClasses="col-xl-10"/>

                <FormSeparator heading={__("em_debug.content.protocol")} />
                <DebugLogger prefix="energy_manager" debug="energy_manager/debug" debugHeader="energy_manager/debug_header" translationPrefix="em_debug"/>

                <FormSeparator heading={__("em_debug.content.internal_state")} />
                <FormRow label="power at meter">
                    <OutputFloat value={s.low_level_state.power_at_meter} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="power available">
                    <OutputFloat value={s.low_level_state.power_available} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="overall min power">
                    <OutputFloat value={s.low_level_state.overall_min_power} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="threshold 3to1">
                    <OutputFloat value={s.low_level_state.threshold_3to1} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="threshold 1to3">
                    <OutputFloat value={s.low_level_state.threshold_1to3} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="CM allocated current">
                    <OutputFloat value={s.low_level_state.charge_manager_allocated_current} digits={3} scale={3} unit={'A'} />
                </FormRow>
                <FormRow label="max current limited">
                    <OutputFloat value={s.low_level_state.max_current_limited} digits={3} scale={3} unit={'A'} />
                </FormRow>
                <FormRow label="uptime past hysteresis">
                    <IndicatorGroup
                        value={s.low_level_state.uptime_past_hysteresis ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="is 3phase">
                    <IndicatorGroup
                        value={s.low_level_state.is_3phase ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="wants 3phase">
                    <IndicatorGroup
                        value={s.low_level_state.wants_3phase ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="wants 3phase last">
                    <IndicatorGroup
                        value={s.low_level_state.wants_3phase_last ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="is on last">
                    <IndicatorGroup
                        value={s.low_level_state.is_on_last ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="wants on last">
                    <IndicatorGroup
                        value={s.low_level_state.wants_on_last ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="phase state change blocked">
                    <IndicatorGroup
                        value={s.low_level_state.phase_state_change_blocked ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="on state change blocked">
                    <IndicatorGroup
                        value={s.low_level_state.on_state_change_blocked ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="charging blocked">
                    <InputText value={"0x" + s.low_level_state.charging_blocked.toString(16)}/>
                </FormRow>
                <FormRow label="switching state">
                    <InputText value={s.low_level_state.switching_state}/>
                </FormRow>
                <FormRow label="consecutive bricklet errors">
                    <InputText value={s.low_level_state.consecutive_bricklet_errors}/>
                </FormRow>

                <FormSeparator heading={__("em_debug.content.low_level_state")} />
                <FormRow label={__("em_debug.content.contactor_control")}>
                    <IndicatorGroup
                        value={s.low_level_state.contactor ? 1 : 0}
                        items={[
                            ["secondary", __("em_debug.content.contactor_off")],
                            ["primary", __("em_debug.content.contactor_on")],
                        ]} />
                </FormRow>

                <FormRow label={__("em_debug.content.contactor_check")}>
                    <IndicatorGroup
                        value={s.low_level_state.contactor_check_state ? 0 : 1} // intentionally inverted, OK is first
                        items={[
                            ["success", __("em_debug.content.contactor_check_ok")],
                            ["danger", __("em_debug.content.contactor_check_fail")],
                        ]} />
                </FormRow>

                <FormRow label={__("em_debug.content.state_led")} label_muted={__("em_debug.content.state_led_names")}>
                    <div class="row mx-n1">
                        {s.low_level_state.led_rgb.map((x, i) => (
                            <div key={i} class="mb-1 col-4 px-1">
                                <InputText value={x} />
                            </div>
                        ))}
                    </div>
                </FormRow>

                <FormRow label={__("em_debug.content.gpios")} label_muted={__("em_debug.content.gpio_names_0")}>
                    <div class="row mx-n1">
                        {[s.state.input3_state, s.state.input4_state, s.state.relay_state].map((x, j) => (
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
                    <OutputFloat value={s.low_level_state.input_voltage} digits={3} scale={3} unit={'V'} />
                </FormRow>
            </>
        )
    }
}

render(<EMDebug />, $('#em_debug')[0]);

export function init() {
}

export function add_event_listeners(source: API.APIEventTarget) {
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-em_debug').prop('hidden', !module_init.energy_manager);
}
