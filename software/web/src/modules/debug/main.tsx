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

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { __, translate_unchecked } from "../../ts/translation";
import { h, Context, Fragment, Component, ComponentChild } from "preact";
import { Button         } from "react-bootstrap";
import { FormRow as VanillaFormRow, FormRowProps, register_id_context_component_type } from "../../ts/components/form_row";
import { FormSeparator  } from "../../ts/components/form_separator";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { InputText      } from "../../ts/components/input_text";
import { NavbarItem     } from "../../ts/components/navbar_item";
import { OutputFloat    } from "../../ts/components/output_float";
import { PageHeader     } from "../../ts/components/page_header";
import { SubPage        } from "../../ts/components/sub_page";
import { Terminal } from "react-feather";

interface OutputPlainIntProps {
    idContext?: Context<string>;
    value: number;
}

export function OutputPlainInt(props: OutputPlainIntProps) {
    return <OutputFloat {...props}  digits={0} scale={0} unit="" maxFractionalDigitsOnPage={0} maxUnitLengthOnPage={0} />
}

register_id_context_component_type(OutputPlainInt);

interface OutputBytesProps {
    idContext?: Context<string>;
    value: number;
}

export function OutputBytes(props: OutputBytesProps) {
    return <OutputFloat {...props}  digits={0} scale={0} unit="B" maxFractionalDigitsOnPage={0} maxUnitLengthOnPage={1} />
}

register_id_context_component_type(OutputBytes);

export function DebugNavbar() {
    return <NavbarItem name="debug" module="debug" title={__("debug.navbar.debug")} symbol={<Terminal />} />;
}

export function FormRow(props: FormRowProps) { return <VanillaFormRow {...props} labelColClasses="col-lg-3" contentColClasses="col-lg-9" />; }

function Row(props: {className?: string, label?: string, label_muted?: string, l?: ComponentChild, c?: ComponentChild, r?: ComponentChild}) {
    return <FormRow class={props.className} label={props.label} label_muted={props.label_muted}>
        <div class="row gx-2 gy-1">
            <div class="col-12 col-sm-4">{props.l}</div>
            <div class="col-12 col-sm-4">{props.c}</div>
            <div class="col-12 col-sm-4">{props.r}</div>
        </div>
    </FormRow>
}

export class Debug extends Component {
    render() {
        if (!util.render_allowed())
            return <SubPage name="debug" />;

        let state_static = API.get('debug/state_static');
        let state_fast   = API.get('debug/state_fast');
        let state_slow   = API.get('debug/state_slow');
        let state_slots  = API.get('debug/state_slots');
        let state_hwm    = API.get('debug/state_hwm');

        const config_type_names = [
            __("debug.content.conf_uint_buf"),
            __("debug.content.conf_int_buf"),
            __("debug.content.conf_uint32_buf"),
            __("debug.content.conf_int32_buf"),
            __("debug.content.conf_uint53_buf"),
            __("debug.content.conf_int52_buf"),
            __("debug.content.conf_float_buf"),
            __("debug.content.conf_string_buf"),
            __("debug.content.conf_array_buf"),
            __("debug.content.conf_tuple_buf"),
            __("debug.content.conf_object_buf"),
            __("debug.content.conf_union_buf"),
        ];

        return (
            <SubPage name="debug">
                <PageHeader title={__("debug.content.debug")} />

                <FormRow label={__("debug.content.uptime")}>
                    <InputText value={util.format_timespan_ms(state_fast.uptime)}/>
                </FormRow>

                <FormRow label={__("debug.content.cpu_usage")} label_muted={__("debug.content.cpu_usage_muted")}>
                    <OutputFloat value={state_fast.cpu_usage} digits={0} scale={0} unit="%" maxFractionalDigitsOnPage={0} maxUnitLengthOnPage={1} />
                </FormRow>

                <FormRow label={__("debug.content.main_loop_max")}>
                    <div class="row gx-2 gy-1">
                        <div class="col-12 col-sm-6">
                            <OutputFloat value={state_slow.loop_max_us} digits={3} scale={3} unit="ms" maxUnitLengthOnPage={2} />
                        </div>
                        <div class="col-12 col-sm-6">
                            <InputText value={state_slow.loop_max_fn_file + ":" + state_slow.loop_max_fn_line}/>
                        </div>
                    </div>

                </FormRow>

                <FormSeparator heading={__("debug.content.heap_integrity_header")} />

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
                    <div class="row gx-2 gy-1">
                        <div class="col-12 col-sm-6">
                            <OutputFloat value={state_fast.heap_check_time_avg} digits={3} scale={3} unit="ms" maxUnitLengthOnPage={2} />
                        </div>
                        <div class="col-12 col-sm-6">
                            <OutputFloat value={state_fast.heap_check_time_max} digits={3} scale={3} unit="ms"  maxUnitLengthOnPage={2} />
                        </div>
                    </div>
                </FormRow>

                <FormSeparator heading={__("debug.content.memory_header")} />

                <Row className="d-none d-sm-flex"
                     l={<p class="mb-0 form-label text-center">{__("debug.content.dram")}</p>}
                     c={<p class="mb-0 form-label text-center">{__("debug.content.iram")}</p>}
                     r={<p class="mb-0 form-label text-center">{__("debug.content.psram")}</p>}/>

                <Row label={__("debug.content.heap_used")}
                     l={<OutputBytes value={state_static.heap_dram - state_fast.free_dram} />}
                     c={<OutputBytes value={state_static.heap_iram - state_fast.free_iram} />}
                     r={<OutputBytes value={state_static.heap_psram - state_fast.free_psram} />}/>

                <Row label={__("debug.content.heap_free")}
                     l={<OutputBytes value={state_fast.free_dram} />}
                     c={<OutputBytes value={state_fast.free_iram} />}
                     r={<OutputBytes value={state_fast.free_psram} />}/>

                <Row label={__("debug.content.heap_block")}
                     l={<OutputBytes value={state_slow.largest_free_dram_block} />}
                     c={<OutputBytes value={state_slow.largest_free_iram_block} />}
                     r={<OutputBytes value={state_slow.largest_free_psram_block} />}/>

                <Row label={__("debug.content.heap_min_free")}
                     l={<OutputBytes value={state_slow.min_free_dram} />}
                     c={<OutputBytes value={state_slow.min_free_iram} />}
                     r={<OutputBytes value={state_slow.min_free_psram} />}/>

                <Row label={__("debug.content.heap_size")}
                     l={<OutputBytes value={state_static.heap_dram} />}
                     c={<OutputBytes value={state_static.heap_iram} />}
                     r={<OutputBytes value={state_static.heap_psram} />}/>

                <Row label={__("debug.content.static")}
                     l={<OutputBytes value={335872 - state_static.heap_dram} />}
                     c={<OutputBytes value={131072 - state_static.heap_iram} />}
                     r={<OutputBytes value={state_static.psram_size - state_static.heap_psram} />}/>

                <Row label={__("debug.content.total_size")}
                     l={<OutputBytes value={335872} />}
                     c={<OutputBytes value={131072} />}
                     r={<OutputBytes value={state_static.psram_size} />}/>

                <FormSeparator heading={__("debug.content.config_buffers")} />

                <Row className="d-none d-sm-flex"
                     l={<p class="mb-0 form-label text-center">{__("debug.content.dram")}</p>}
                     c={<p class="mb-0 form-label text-center">{__("debug.content.iram")}</p>}
                     r={<p class="mb-0 form-label text-center">{__("debug.content.psram")}</p>}/>

                <Row label={__("debug.content.conf_uint_buf")}
                     c={<OutputBytes value={state_slow.conf_uint_buf_size} />}/>

                <Row label={__("debug.content.conf_int_buf")}
                     c={<OutputBytes value={state_slow.conf_int_buf_size} />}/>

                <Row label={__("debug.content.conf_uint32_buf")}
                     c={<OutputBytes value={state_slow.conf_uint32_buf_size} />}/>

                <Row label={__("debug.content.conf_int32_buf")}
                     c={<OutputBytes value={state_slow.conf_int32_buf_size} />}/>

                <Row label={__("debug.content.conf_uint53_buf")}
                     c={<OutputBytes value={state_slow.conf_uint53_buf_size} />}/>

                <Row label={__("debug.content.conf_int52_buf")}
                     c={<OutputBytes value={state_slow.conf_int52_buf_size} />}/>

                <Row label={__("debug.content.conf_float_buf")}
                     c={<OutputBytes value={state_slow.conf_float_buf_size} />}/>

                <Row label={__("debug.content.conf_string_buf")}
                     l={<OutputBytes value={state_slow.conf_string_buf_size} />}/>

                <Row label={__("debug.content.conf_array_buf")}
                     l={<OutputBytes value={state_slow.conf_array_buf_size} />}/>

                <Row label={__("debug.content.conf_tuple_buf")}
                     l={<OutputBytes value={state_slow.conf_tuple_buf_size} />}/>

                <Row label={__("debug.content.conf_object_buf")}
                     c={<OutputBytes value={state_slow.conf_object_buf_size} />}/>

                <Row label={__("debug.content.conf_union_buf")}
                     l={<OutputBytes value={state_slow.conf_union_buf_size} />}/>

                <FormSeparator heading={__("debug.content.config_slots")} />

                <FormRow class="d-none d-sm-flex">
                    <div class="row gx-2 gy-1">
                        <div class="col-12 col-sm-2">
                            <p class="mb-0 form-label text-center">{__("debug.content.slots_used")}</p>
                        </div>
                        <div class="col-12 col-sm-2">
                            <p class="mb-0 form-label text-center">{__("debug.content.slots_hwm")}</p>
                        </div>
                        <div class="col-12 col-sm-2">
                            <p class="mb-0 form-label text-center">{__("debug.content.slots_allocated")}</p>
                        </div>
                        <div class="col-12 col-sm-2">
                            <p class="mb-0 form-label text-center">{__("debug.content.slots_first_free")}</p>
                        </div>
                        <div class="col-12 col-sm-2">
                            <p class="mb-0 form-label text-center">{__("debug.content.slots_last_used")}</p>
                        </div>
                        <div class="col-12 col-sm-2">
                            <p class="mb-0 form-label text-center">{__("debug.content.slots_holes")}</p>
                        </div>
                    </div>
                </FormRow>

                {state_slots.map((slot, idx) => {
                    return <FormRow label={config_type_names[idx]}>
                        <div class="row gx-2 gy-1">
                            <div class="col-12 col-sm-2">
                                <OutputPlainInt value={slot[0]} />
                            </div>
                            <div class="col-12 col-sm-2">
                                <OutputPlainInt value={slot[1] + 1} />
                            </div>
                            <div class="col-12 col-sm-2">
                                <OutputPlainInt value={slot[2]} />
                            </div>
                            <div class="col-12 col-sm-2">
                                <OutputPlainInt value={slot[3] + 1} />
                            </div>
                            <div class="col-12 col-sm-2">
                                <OutputPlainInt value={slot[4] + 1} />
                            </div>
                            <div class="col-12 col-sm-2">
                                <OutputPlainInt value={slot[4] + 1 - slot[0]} />
                            </div>
                        </div>
                    </FormRow>
                })}

                <FormSeparator heading={__("debug.content.stack_hwm_header")} />

                <FormRow class="d-none d-sm-flex">
                    <div class="row gx-2 gy-1">
                        <div class="col-12 col-sm-3"><p class="mb-0 mt-2 form-label text-center">{__("debug.content.free_stack")}</p></div>
                        <div class="col-12 col-sm-3"><p class="mb-0 mt-2 form-label text-center">{__("debug.content.used_stack")}</p></div>
                        <div class="col-12 col-sm-3"><p class="mb-0 mt-2 form-label text-center">{__("debug.content.stack_size")}</p></div>
                        <div class="col-12 col-sm-3"><p class="mb-0 mt-2 form-label text-center">{__("debug.content.priority"  )}</p></div>
                    </div>
                </FormRow>

                {state_hwm.map((task_hwm) => {
                    return <FormRow label={task_hwm.task_name}>
                        <div class="row gx-2 gy-1">
                            <div class="col-12 col-sm-3"><OutputBytes value={task_hwm.hwm} /></div>
                            <div class="col-12 col-sm-3">{task_hwm.stack_size == 0 ? undefined : <OutputBytes value={task_hwm.stack_size - task_hwm.hwm} />}</div>
                            <div class="col-12 col-sm-3">{task_hwm.stack_size == 0 ? undefined : <OutputBytes value={task_hwm.stack_size} />}</div>
                            <div class="col-12 col-sm-3"><OutputPlainInt value={task_hwm.prio} /></div>
                        </div>
                    </FormRow>
                })}

                <FormSeparator heading="API" />

                <Row className="d-none d-sm-flex"
                     l={<p class="mb-0 mt-2 form-label text-center">State</p>}
                     c={<p class="mb-0 mt-2 form-label text-center">Command</p>}
                     r={<p class="mb-0 mt-2 form-label text-center">Response</p>}/>

                <Row label={__("debug.content.api_registrations")}
                     l={<OutputPlainInt value={state_slow.api_states   } />}
                     c={<OutputPlainInt value={state_slow.api_commands } />}
                     r={<OutputPlainInt value={state_slow.api_responses} />} />

                <FormSeparator heading={__("debug.content.sockets_header")} />

                <Row className="d-none d-sm-flex"
                     l={<p class="mb-0 mt-2 form-label text-center">{__("debug.content.lwip_sockets_used")}</p>}
                     c={<p class="mb-0 mt-2 form-label text-center">{__("debug.content.lwip_sockets_hwm")}</p>}
                     r={<p class="mb-0 mt-2 form-label text-center">{__("debug.content.lwip_sockets_max")}</p>}/>

                <Row label={__("debug.content.lwip_socket_counts")} label_muted={__("debug.content.lwip_socket_counts_muted")}
                     l={<OutputPlainInt value={state_slow.ipsock_cur  } />}
                     c={<OutputPlainInt value={state_slow.ipsock_hwm  } />}
                     r={<OutputPlainInt value={state_static.ipsock_max} />} />

                <FormSeparator heading={__("debug.content.clocks_buses_header")} />

                <FormRow label={__("debug.content.cpu_apb")}>
                    <div class="row gx-2 gy-1">
                        <div class="col-12 col-sm-6">
                            <OutputFloat value={state_static.cpu_clk} digits={0} scale={6} unit="MHz" />
                        </div>
                        <div class="col-12 col-sm-6">
                            <OutputFloat value={state_static.apb_clk} digits={0} scale={6} unit="MHz" />
                        </div>
                    </div>
                </FormRow>

                <Row className="d-none d-sm-flex"
                     l={<p class="mb-0 form-label text-center">{__("debug.content.spi_clock")}</p>}
                     c={<p class="mb-0 form-label text-center">{__("debug.content.dummy_cycles")}</p>}
                     r={<p class="mb-0 form-label text-center">{__("debug.content.spi_mode")}</p>}/>

                {state_static.spi_buses.map((spi_bus, i) => {
                    return <Row label={translate_unchecked("debug.content.spi" + i)}
                                l={<OutputFloat value={spi_bus.clk} digits={2} scale={6} unit="MHz" />}
                                c={<OutputPlainInt value={spi_bus.dummy_cycles}/>}
                                r={<InputText value={spi_bus.spi_mode}/>}/>
                })}

                <FormSeparator heading={__("debug.content.memory_speed_header")} />

                <Row className="d-none d-sm-flex"
                     l={<p class="mb-0 form-label text-center">{__("debug.content.dram")}</p>}
                     c={<p class="mb-0 form-label text-center">{__("debug.content.iram")}</p>}
                     r={<p class="mb-0 form-label text-center">{__("debug.content.psram")}</p>}/>

                <Row label={__("debug.content.ram_benchmark")}
                     l={<OutputFloat value={state_static.dram_benchmark} digits={1} scale={0} unit="MiB/s" maxUnitLengthOnPage={3} />}
                     c={<OutputFloat value={state_static.iram_benchmark} digits={1} scale={0} unit="MiB/s" maxUnitLengthOnPage={3} />}
                     r={<OutputFloat value={state_static.psram_benchmark} digits={1} scale={0} unit="MiB/s" maxUnitLengthOnPage={3} />} />

                <FormRow class="d-none d-sm-flex">
                    <div class="row gx-2 gy-1">
                        <div class="col-12 col-sm-6">
                            <p class="mb-0 form-label text-center">{__("debug.content.flash_rodata")}</p>
                        </div>
                        <div class="col-12 col-sm-6">
                            <p class="mb-0 form-label text-center">{__("debug.content.flash_text")}</p>
                        </div>
                    </div>
                </FormRow>

                <FormRow label={__("debug.content.flash_benchmark")}>
                    <div class="row gx-2 gy-1">
                        <div class="col-12 col-sm-6">
                            <OutputFloat value={state_static.rodata_benchmark} digits={1} scale={0} unit="MiB/s" maxUnitLengthOnPage={3} />
                        </div>
                        <div class="col-12 col-sm-6">
                            <OutputFloat value={state_static.text_benchmark} digits={1} scale={0} unit="MiB/s" maxUnitLengthOnPage={3} />
                        </div>
                    </div>
                </FormRow>

                <FormSeparator heading={__("debug.content.websocket_header")} />

                <FormRow label={__("debug.content.websocket_connection")} label_muted={__("debug.content.websocket_connection_muted")}>
                    <div class="row g-2">
                        <div class="col">
                            <Button variant="primary" className="w-100" onClick={util.pauseWebSockets}>{__("debug.content.websocket_pause")}</Button>
                        </div>
                        <div class="col">
                            <Button variant="primary" className="w-100" onClick={util.resumeWebSockets}>{__("debug.content.websocket_resume")}</Button>
                        </div>
                    </div>
                </FormRow>
            </SubPage>
        );
    }
}

export function pre_init() {
}

export function init() {
}
