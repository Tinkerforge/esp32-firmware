/* esp32-firmware
 * Copyright (C) 2020-2023 Erik Fleckstein <erik@tinkerforge.com>
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
import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { __, translate_unchecked } from "../../ts/translation";
import { h, render, Fragment, Component } from "preact";
import { Button         } from "react-bootstrap";
import { FormRow        } from "../../ts/components/form_row";
import { FormSeparator  } from "../../ts/components/form_separator";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { InputText      } from "../../ts/components/input_text";
import { OutputFloat    } from "../../ts/components/output_float";
import { PageHeader     } from "../../ts/components/page_header";
import { SubPage        } from "../../ts/components/sub_page";

export class Debug extends Component {
    render() {
        if (!util.render_allowed())
            return <></>

        let state_static = API.get('debug/state_static');
        let state_fast   = API.get('debug/state_fast');
        let state_slow   = API.get('debug/state_slow');
        let state_hwm    = API.get('debug/state_hwm');

        return (
            <SubPage>
                <PageHeader title={__("debug.content.debug")} />

                <FormRow label={__("debug.content.uptime")}>
                    <InputText value={util.format_timespan_ms(state_fast.uptime)}/>
                </FormRow>

                <FormRow label={__("debug.content.cpu_usage")} label_muted={__("debug.content.cpu_usage_muted")}>
                    <OutputFloat value={state_fast.cpu_usage} digits={0} scale={0} unit="%"/>
                </FormRow>

                <FormSeparator heading={__("debug.content.heap_integrity_header")} first={false} />

                <FormRow label={__("debug.content.heap_integrity_result")}>
                    <IndicatorGroup
                        value={state_slow.heap_integrity_ok ? 0 : 1} // intentionally inverted, OK is first
                        items={[
                            ["success", __("debug.content.heap_integrity_ok")],
                            ["danger", __("debug.content.heap_integrity_fail")],
                        ]}
                    />
                </FormRow>

                <FormRow label={__("debug.content.heap_integrity_runtime")} label_muted={__("debug.content.heap_integrity_runtime_muted")}>
                    <div class="row">
                        <div class="mb-1 col-12 col-sm-6">
                            <OutputFloat value={state_fast.heap_check_time_avg} digits={3} scale={3} unit="ms"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-6">
                            <OutputFloat value={state_fast.heap_check_time_max} digits={3} scale={3} unit="ms"/>
                        </div>
                    </div>
                </FormRow>

                <FormSeparator heading={__("debug.content.memory_header")} first={false} />

                <FormRow label="">
                    <div class="row">
                        <div class="col-12 col-sm-4">
                            <p class="mb-0 form-label text-center">{__("debug.content.dram")}</p>
                        </div>
                        <div class="col-12 col-sm-4">
                            <p class="mb-0 form-label text-center">{__("debug.content.iram")}</p>
                        </div>
                        <div class="col-12 col-sm-4">
                            <p class="mb-0 form-label text-center">{__("debug.content.psram")}</p>
                        </div>
                    </div>
                </FormRow>

                <FormRow label={__("debug.content.heap_used")}>
                    <div class="row">
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={state_static.heap_dram - state_fast.free_dram} digits={0} scale={0} unit="B"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={state_static.heap_iram - state_fast.free_iram} digits={0} scale={0} unit="B"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={state_static.heap_psram - state_fast.free_psram} digits={0} scale={0} unit="B"/>
                        </div>
                    </div>
                </FormRow>

                <FormRow label={__("debug.content.heap_free")}>
                    <div class="row">
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={state_fast.free_dram} digits={0} scale={0} unit="B"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={state_fast.free_iram} digits={0} scale={0} unit="B"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={state_fast.free_psram} digits={0} scale={0} unit="B"/>
                        </div>
                    </div>
                </FormRow>

                <FormRow label={__("debug.content.heap_block")}>
                    <div class="row">
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={state_slow.largest_free_dram_block} digits={0} scale={0} unit="B"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-4">
                        </div>
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={state_slow.largest_free_psram_block} digits={0} scale={0} unit="B"/>
                        </div>
                    </div>
                </FormRow>

                <FormRow label={__("debug.content.heap_size")}>
                    <div class="row">
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={state_static.heap_dram} digits={0} scale={0} unit="B"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={state_static.heap_iram} digits={0} scale={0} unit="B"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={state_static.heap_psram} digits={0} scale={0} unit="B"/>
                        </div>
                    </div>
                </FormRow>

                <FormRow label={__("debug.content.static")}>
                    <div class="row">
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={335872 - state_static.heap_dram} digits={0} scale={0} unit="B"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={131072 - state_static.heap_iram} digits={0} scale={0} unit="B"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={state_static.psram_size - state_static.heap_psram} digits={0} scale={0} unit="B"/>
                        </div>
                    </div>
                </FormRow>

                <FormRow label={__("debug.content.total_size")}>
                    <div class="row">
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={335872} digits={0} scale={0} unit="B"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={131072} digits={0} scale={0} unit="B"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-4">
                            <OutputFloat value={state_static.psram_size} digits={0} scale={0} unit="B"/>
                        </div>
                    </div>
                </FormRow>

                <FormSeparator heading={__("debug.content.stack_hwm_header")} first={false} />

                <FormRow label={__("debug.content.task_name")}>
                    <div class="row">
                        <div class="col-12 col-sm-4">
                            <p class="mb-0 mt-2 form-label text-center">{__("debug.content.free_stack")}</p>
                        </div>
                        <div class="col-12 col-sm-4">
                            <p class="mb-0 mt-2 form-label text-center">{__("debug.content.used_stack")}</p>
                        </div>
                        <div class="col-12 col-sm-4">
                            <p class="mb-0 mt-2 form-label text-center">{__("debug.content.stack_size")}</p>
                        </div>
                    </div>
                </FormRow>

                {state_hwm.map((task_hwm) => {
                    return <FormRow label={task_hwm.task_name}>
                        <div class="row">
                            <div class="mb-1 col-12 col-sm-4">
                                <OutputFloat value={task_hwm.hwm} digits={0} scale={0} unit="B"/>
                            </div>
                            {task_hwm.stack_size != 0 ? <>
                                <div class="mb-1 col-12 col-sm-4">
                                    <OutputFloat value={task_hwm.stack_size - task_hwm.hwm} digits={0} scale={0} unit="B"/>
                                </div>
                                <div class="mb-1 col-12 col-sm-4">
                                    <OutputFloat value={task_hwm.stack_size} digits={0} scale={0} unit="B"/>
                                </div>
                            </>:<></>}
                        </div>
                    </FormRow>
                })}

                <FormSeparator heading={__("debug.content.clocks_buses_header")} first={false} />

                <FormRow label={__("debug.content.cpu_apb")}>
                    <div class="row">
                        <div class="mb-1 col-12 col-sm-6">
                            <OutputFloat value={state_static.cpu_clk} digits={0} scale={6} unit="MHz"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-6">
                            <OutputFloat value={state_static.apb_clk} digits={0} scale={6} unit="MHz"/>
                        </div>
                    </div>
                </FormRow>

                <FormRow label="">
                    <div class="row">
                        <div class="col-12 col-sm-4">
                            <p class="mb-0 form-label text-center">{__("debug.content.spi_clock")}</p>
                        </div>
                        <div class="col-12 col-sm-4">
                            <p class="mb-0 form-label text-center">{__("debug.content.dummy_cycles")}</p>
                        </div>
                        <div class="col-12 col-sm-4">
                            <p class="mb-0 form-label text-center">{__("debug.content.spi_mode")}</p>
                        </div>
                    </div>
                </FormRow>

                {state_static.spi_buses.map((spi_bus, i) => {
                    return <FormRow label={translate_unchecked("debug.content.spi" + i)}>
                        <div class="row">
                            <div class="mb-1 col-12 col-sm-4">
                                <OutputFloat value={spi_bus.clk} digits={2} scale={6} unit="MHz"/>
                            </div>
                            <div class="mb-1 col-12 col-sm-4">
                                <OutputFloat value={spi_bus.dummy_cycles} digits={0} scale={0} unit=""/>
                            </div>
                            <div class="mb-1 col-12 col-sm-4">
                                <InputText value={spi_bus.spi_mode}/>
                            </div>
                        </div>
                    </FormRow>
                })}

                <FormSeparator heading={__("debug.content.memory_speed_header")} first={false} />

                <FormRow label="">
                    <div class="row">
                    <div class="mb-1 col-12 col-sm-6">
                            <p class="mb-0 form-label text-center">{__("debug.content.flash")}</p>
                        </div>
                        <div class="mb-1 col-12 col-sm-6">
                            <p class="mb-0 form-label text-center">{__("debug.content.psram")}</p>
                        </div>
                    </div>
                </FormRow>

                <FormRow label={__("debug.content.speed_benchmark")}>
                    <div class="row">
                        <div class="mb-1 col-12 col-sm-6">
                            <OutputFloat value={state_static.flash_benchmark} digits={1} scale={0} unit="MiB/s"/>
                        </div>
                        <div class="mb-1 col-12 col-sm-6">
                            <OutputFloat value={state_static.psram_benchmark} digits={1} scale={0} unit="MiB/s"/>
                        </div>
                    </div>
                </FormRow>

                <FormRow label={__("debug.content.memory_mode")} label_muted={__("debug.content.memory_mode_muted")}>
                    <div class="row">
                        <div class="mb-1 col-12 col-sm-6">
                            <InputText value={state_static.flash_mode}/>
                        </div>
                    </div>
                </FormRow>

                <FormSeparator heading={__("debug.content.websocket_header")} first={false} />

                <FormRow label={__("debug.content.websocket_connection")} label_muted={__("debug.content.websocket_connection_muted")}>
                    <div class="input-group pb-2">
                        <Button variant="primary" className="form-control rounded-right mr-2" onClick={util.pauseWebSockets}>{__("debug.content.websocket_pause")}</Button>
                        <Button variant="primary" className="form-control rounded-left" onClick={util.resumeWebSockets}>{__("debug.content.websocket_resume")}</Button>
                    </div>
                </FormRow>
            </SubPage>
        );
    }
}

export function init() {
}
render(<Debug />, $("#debug")[0]);

export function add_event_listeners(source: API.APIEventTarget) {
}

export function update_sidebar_state(module_init: any) {
    $("#sidebar-debug").prop("hidden", !module_init.debug);
}
