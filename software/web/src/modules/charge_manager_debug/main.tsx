/* esp32-firmware
 * Copyright (C) 2024 Erik Fleckstein <erik@tinkerforge.com>
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
import { h, Fragment, Component } from "preact";
import { DebugLogger    } from "../../ts/components/debug_logger";
import { FormRow        } from "../../ts/components/form_row";
import { FormSeparator  } from "../../ts/components/form_separator";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { InputText      } from "../../ts/components/input_text";
import { OutputFloat    } from "../../ts/components/output_float";
import { PageHeader     } from "../../ts/components/page_header";
import { Button, Card } from "react-bootstrap";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Download, Terminal } from "react-feather";
import { InputNumber } from "src/ts/components/input_number";

export function ChargeManagerDebugNavbar() {
    return <NavbarItem name="charge_manager_debug" module="charge_manager" title={__("charge_manager_debug.navbar.charge_manager_debug")} symbol={<Terminal />} />;
}


const CMDOutFloat = (props: any) => <OutputFloat maxFractionalDigitsOnPage={3} maxUnitLengthOnPage={2} {...props}/>
const CMDCardOutFloat = (props: any) => <OutputFloat maxUnitLengthOnPage={3.5} {...props}/>
const CMDFormRow = (props: any) => <FormRow labelColClasses="col-lg-2" contentColClasses="col-lg-10" {...props}/>

function Charger (props: {i: number,
                          state: API.getType['charge_manager/state']['chargers'][0],
                          ll_state: API.getType['charge_manager/low_level_state']['chargers'][0]} ) {
    const uptime = API.get('info/keep_alive')?.uptime ?? 0;
    const ll_cfg = API.get("charge_manager/low_level_config");
    const cfg = API.get("charge_manager/config");

    return <>
        <Card>
            <Card.Header as="h5">{`${props.i} - ${props.state.n} (${props.state.u})`}</Card.Header>
            <Card.Body>
                <FormRow label="State">
                    <CMDCardOutFloat value={props.state.s} digits={0} scale={0} unit={''}/>
                </FormRow>

                <FormRow label="Error">
                    <CMDCardOutFloat value={props.state.e} digits={0} scale={0} unit={''}/>
                </FormRow>

                <FormRow label="Flags">
                    <CMDCardOutFloat value={props.ll_state.b} digits={0} scale={0} unit={''}/>
                </FormRow>

                <FormRow label="Allocated">
                    <CMDCardOutFloat value={props.state.ac} digits={3} scale={3} unit={'A@' + props.state.ap + "p"}/>
                </FormRow>

                <FormRow label="Supported">
                    <CMDCardOutFloat value={props.state.sc} digits={3} scale={3} unit={'A@' + ((props.state.sp & 4) != 0 ? "3/1" : props.state.sp) + "p"}/>
                </FormRow>

                <FormRow label="Requested">
                    <CMDCardOutFloat value={props.ll_state.rc} digits={3} scale={3} unit={'A'}/>
                </FormRow>

                <FormRow label="Alloc'd energy">
                    <CMDCardOutFloat value={props.ll_state.ae} digits={3} scale={3} unit={'kWh'}/>
                </FormRow>

                <FormRow label="Alloc'd energy rot">
                    <div class="row no-gutters justify-content-center">
                        <div class="col">
                            <CMDCardOutFloat value={props.ll_state.ar} digits={3} scale={3} unit={''} maxUnitLengthOnPage={0}/>
                        </div>
                        <div class="col-auto"><span class="form-control" style="border: none;"> / </span></div>
                        <div class="col">
                            <CMDCardOutFloat value={ll_cfg.alloc_energy_rot_thres} digits={0} scale={0} maxFractionalDigitsOnPage={0} unit={'kWh'}/>
                        </div>
                    </div>
                </FormRow>

                <FormRow label="Last switch">
                    <div class="row no-gutters justify-content-center">
                        <div class="col">
                            <InputText value={(props.ll_state.ls == 0 ? 0 : util.format_timespan_ms(uptime - props.ll_state.ls))}/>
                        </div>
                        <div class="col-auto"><span class="form-control" style="border: none;"> / </span></div>
                        <div class="col">
                            <InputText value={util.format_timespan(ll_cfg.min_active_time)}/>
                        </div>
                    </div>
                </FormRow>

                <FormRow label="Last plug in">
                    <div class="row no-gutters justify-content-center">
                        <div class="col">
                            <InputText value={(props.ll_state.lp == 0 ? 0 : util.format_timespan_ms(uptime - props.ll_state.lp))}/>
                        </div>
                        <div class="col-auto"><span class="form-control" style="border: none;"> / </span></div>
                        <div class="col">
                            <InputText value={util.format_timespan(ll_cfg.plug_in_time)}/>
                        </div>
                    </div>
                </FormRow>


                <FormRow label="Last wakeup">
                    <div class="row no-gutters justify-content-center">
                        <div class="col">
                            <InputText value={(props.ll_state.lw == 0 ? 0 : util.format_timespan_ms(uptime - props.ll_state.lw))}/>
                        </div>
                        <div class="col-auto"><span class="form-control" style="border: none;"> / </span></div>
                        <div class="col">
                            <InputText value={util.format_timespan(ll_cfg.wakeup_time)}/>
                        </div>
                    </div>
                </FormRow>


                <FormRow label="Ignore requested">
                    <div class="row no-gutters justify-content-center">
                        <div class="col">
                            <InputText value={((props.ll_state.ip == 0) || (props.ll_state.ip - uptime < 0) ? 0 : util.format_timespan_ms(props.ll_state.ip - uptime))}/>
                        </div>
                        <div class="col-auto"><span class="form-control" style="border: none;"> / </span></div>
                        <div class="col">
                            <InputText value={util.format_timespan(cfg.requested_current_threshold)}/>
                        </div>
                    </div>
                </FormRow>
           </Card.Body>

            <Card.Footer>
                Last update {util.format_timespan_ms(Math.max(0, uptime - props.state.lu))}
            </Card.Footer>

        </Card>
    </>
}

export class ChargeManagerDebug extends Component {

    render() {
        if (!util.render_allowed())
            return <SubPage name="charge_manager_debug" />;

        // For some reason info/keep_alive can be missing even if render_allowed is true?
        const uptime      = API.get('info/keep_alive')?.uptime ?? 0;
        const ll_cfg      = API.get('charge_manager/low_level_config');
        const state       = API.get('charge_manager/state');
        const ll_state    = API.get('charge_manager/low_level_state');
        const pm_ll_state = API.get_unchecked('power_manager/low_level_state');

        return (
            <SubPage name="charge_manager_debug" colClasses="col-xl-10">
                <PageHeader title={__("charge_manager_debug.content.charge_manager_debug")}/>

                <FormSeparator heading={__("charge_manager_debug.content.protocol")} first={true} />
                <CMDFormRow label="Trace log" labelColClasses="col-lg-2" contentColClasses="col-lg-10">
                    <Button variant="primary" className="form-control" onClick={async () => util.downloadToFile(await util.download("/trace_log"), "charge-manager-trace-log", "txt", "text/plain")}><span class="ml-1 mr-2">Download</span> <Download/></Button>
                </CMDFormRow>

                <FormSeparator heading="Limits" />

                <CMDFormRow label="Hysteresis">
                    <div class="row">
                        <div class="mb-1 col-12 col-lg-6">
                            <InputText value={(ll_state.last_hyst_reset == 0 ? 0 : util.format_timespan_ms(uptime - ll_state.last_hyst_reset)) + " / " + util.format_timespan(ll_cfg.global_hysteresis)}/>
                        </div>
                    </div>
                </CMDFormRow>

                <CMDFormRow label="">
                    <div class="row">
                        <div class="col-12 col-sm-3">
                            <p class="mb-0 form-label text-center">PV</p>
                        </div>
                        <div class="col-12 col-sm-3">
                            <p class="mb-0 form-label text-center">L1</p>
                        </div>
                        <div class="col-12 col-sm-3">
                            <p class="mb-0 form-label text-center">L2</p>
                        </div>
                        <div class="col-12 col-sm-3">
                            <p class="mb-0 form-label text-center">L3</p>
                        </div>
                    </div>
                </CMDFormRow>

                <CMDFormRow label="Raw">
                    <div class="row">
                        {util.range(4).map(i => <div class="mb-1 col-12 col-lg-3">
                            <CMDOutFloat value={state.l_raw[i]} digits={3} scale={3} unit="A"/>
                        </div>)}
                    </div>
                </CMDFormRow>

                <CMDFormRow label="Min">
                    <div class="row">
                        {util.range(4).map(i => <div class="mb-1 col-12 col-lg-3">
                            <CMDOutFloat value={state.l_min[i]} digits={3} scale={3} unit="A"/>
                        </div>)}
                    </div>
                </CMDFormRow>

                <CMDFormRow label="Spread">
                    <div class="row">
                        {util.range(4).map(i => <div class="mb-1 col-12 col-lg-3">
                            <CMDOutFloat value={state.l_spread[i]} digits={3} scale={3} unit="A"/>
                        </div>)}
                    </div>
                </CMDFormRow>

                <CMDFormRow label="Max PV">
                    <div class="row">
                        <div class="mb-1 col-12 col-lg-3">
                            <CMDOutFloat value={state.l_max_pv} digits={3} scale={3} unit="A"/>
                        </div>
                    </div>
                </CMDFormRow>

                <FormSeparator heading="Window" />

                <CMDFormRow label="Max">
                    <div class="row">
                        {util.range(4).map(i => <div class="mb-1 col-12 col-lg-3">
                            <CMDOutFloat value={ll_state.wnd_max[i]} digits={3} scale={3} unit="A"/>
                        </div>)}
                    </div>
                </CMDFormRow>

                <CMDFormRow label="Allocated">
                    <div class="row">
                        {util.range(4).map(i => <div class="mb-1 col-12 col-lg-3">
                            <CMDOutFloat value={state.alloc[i]} digits={3} scale={3} unit="A"/>
                        </div>)}
                    </div>
                </CMDFormRow>

                <CMDFormRow label="Min">
                    <div class="row">
                        {util.range(4).map(i => <div class="mb-1 col-12 col-lg-3">
                            <CMDOutFloat value={ll_state.wnd_min[i]} digits={3} scale={3} unit="A"/>
                        </div>)}
                    </div>
                </CMDFormRow>

                {pm_ll_state ?
                    <>
                        <FormSeparator heading="Power Manager" />

                        <CMDFormRow label="Meter">
                            <div class="row">
                                <div class="mb-1 col-12 col-lg-3">
                                    <CMDOutFloat value={pm_ll_state.power_at_meter} digits={3} scale={3} unit="kW"/>
                                </div>
                                {util.range(3).map(i => <div class="mb-1 col-12 col-lg-3">
                                    <CMDOutFloat value={pm_ll_state.i_meter[i]} digits={3} scale={3} unit="A"/>
                                </div>)}
                            </div>
                        </CMDFormRow>

                        <CMDFormRow label="I_pp_max">
                            <div class="row">
                                <div class="mb-1 col-12 col-lg-3">
                                </div>
                                {util.range(3).map(i => <div class="mb-1 col-12 col-lg-3">
                                    <CMDOutFloat value={pm_ll_state.i_pp_max[i]} digits={3} scale={3} unit="A"/>
                                </div>)}
                            </div>
                        </CMDFormRow>

                        <CMDFormRow label="I_pp_mavg">
                            <div class="row">
                                <div class="mb-1 col-12 col-lg-3">
                                </div>
                                {util.range(3).map(i => <div class="mb-1 col-12 col-lg-3">
                                    <CMDOutFloat value={pm_ll_state.i_pp_mavg[i]} digits={3} scale={3} unit="A"/>
                                </div>)}
                            </div>
                        </CMDFormRow>

                        <CMDFormRow label="P_avl / I_pp">
                            <div class="row">
                                <div class="mb-1 col-12 col-lg-3">
                                    <CMDOutFloat value={pm_ll_state.power_available} digits={3} scale={3} unit="kW"/>
                                </div>
                                {util.range(3).map(i => <div class="mb-1 col-12 col-lg-3">
                                    <CMDOutFloat value={pm_ll_state.i_pp[i]} digits={3} scale={3} unit="A"/>
                                </div>)}
                            </div>
                        </CMDFormRow>
                    </>
                : null}

                <FormSeparator heading="Chargers" />

                <div class="row">
                    {util.range(state.chargers.length).map(i => <div class="mb-5 col-12 col-sm-6 ">
                        <Charger i={i} state={state.chargers[i]} ll_state={ll_state.chargers[i]}/>
                    </div>)}
                </div>



{/*

                <FormRow label="power at meter smooth">
                    <OutputFloat value={pm_ll_state.power_at_meter} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="power available">
                    <OutputFloat value={pm_ll_state.power_available} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="overall min power">
                    <OutputFloat value={pm_ll_state.overall_min_power} digits={3} scale={3} unit={'kW'} />
                </FormRow>
                <FormRow label="CM available current">
                    <OutputFloat value={pm_ll_state.charge_manager_available_current} digits={3} scale={3} unit={'A'} />
                </FormRow>
                <FormRow label="CM allocated current">
                    <OutputFloat value={pm_ll_state.charge_manager_allocated_current} digits={3} scale={3} unit={'A'} />
                </FormRow>
                <FormRow label="max current limited">
                    <OutputFloat value={pm_ll_state.max_current_limited} digits={3} scale={3} unit={'A'} />
                </FormRow>
                <FormRow label="is 3phase">
                    <IndicatorGroup
                        value={pm_ll_state.is_3phase ? 1 : 0}
                        items={[
                            ["secondary", "false"],
                            ["primary",   "true" ],
                        ]} />
                </FormRow>
                <FormRow label="charging blocked">
                    <InputText value={"0x" + pm_ll_state.charging_blocked.toString(16)}/>
                </FormRow>
                <FormRow label="consecutive bricklet errors">
                    <InputText value={ll_state.consecutive_bricklet_errors}/>
                </FormRow>

                <FormSeparator heading={__("charge_manager_debug.content.hardware_state")} />
                <FormRow label={__("charge_manager_debug.content.contactor_control")}>
                    <IndicatorGroup
                        value={ll_state.contactor ? 1 : 0}
                        items={[
                            ["secondary", __("charge_manager_debug.content.contactor_off")],
                            ["primary", __("charge_manager_debug.content.contactor_on")],
                        ]} />
                </FormRow>

                <FormRow label={__("charge_manager_debug.content.contactor_check")}>
                    <IndicatorGroup
                        value={ll_state.contactor_check_state ? 0 : 1} // intentionally inverted, OK is first
                        items={[
                            ["success", __("charge_manager_debug.content.contactor_check_ok")],
                            ["danger", __("charge_manager_debug.content.contactor_check_fail")],
                        ]} />
                </FormRow>

                <FormRow label={__("charge_manager_debug.content.state_led")} label_muted={__("charge_manager_debug.content.state_led_names")}>
                    <div class="row mx-n1">
                        {ll_state.led_rgb.map((x, i) => (
                            <div key={i} class="mb-1 col-4 px-1">
                                <InputText value={x} />
                            </div>
                        ))}
                    </div>
                </FormRow>

                <FormRow label={__("charge_manager_debug.content.gpios")} label_muted={__("charge_manager_debug.content.gpio_names_0")}>
                    <div class="row mx-n1">
                        {[state.input3_state, state.input4_state, state.relay_state].map((x, j) => (
                            <IndicatorGroup vertical key={j} class="mb-1 col px-1"
                                value={x ? 0 : 1} //intentionally inverted: the high button is the first
                                items={[
                                    ["primary", __("charge_manager_debug.content.high")],
                                    ["secondary", __("charge_manager_debug.content.low")]
                                ]} />
                        ))}
                    </div>
                </FormRow>

                <FormRow label={__("charge_manager_debug.content.state_input_voltage")}>
                    <OutputFloat value={ll_state.input_voltage} digits={3} scale={3} unit={'V'} />
                </FormRow>*/}
            </SubPage>
        );
    }
}

export function init() {
}
