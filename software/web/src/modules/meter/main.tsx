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

import $ from "../../ts/jq";

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { __, translate_unchecked } from "../../ts/translation";
import { h, render, createRef, Fragment, Component, ComponentChild } from "preact";
import { PageHeader } from "../../ts/components/page_header";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { Button } from "react-bootstrap";
import { InputSelect } from "src/ts/components/input_select";
import { CollapsedSection } from "src/ts/components/collapsed_section";
import { OutputFloat } from "src/ts/components/output_float";
import { Zap, ZapOff } from "react-feather";
import uPlot from 'uplot';

interface DetailedViewEntry {
    i: number,
    name: string,
    desc: string,
    three_phase: boolean,
    unit: string,
    meter_types: number[]
    digits: 0|1|2|3
}

function entry(name: string, three_phase: boolean, unit: string, meter_types: (2|3|4|5|200)[], digits: 0|1|2|3) : DetailedViewEntry {
    return {i: 0, name: translate_unchecked(`meter.content.detailed_${name}`), desc: translate_unchecked(`meter.content.detailed_${name}_desc`), three_phase: three_phase, unit: unit, meter_types: meter_types, digits: digits}
}

const entries: DetailedViewEntry[] = [
    entry("line_to_neutral_volts",             true,  "V",     [2, 3, 5, 200], 1),
    entry("current",                           true,  "A",     [2, 3, 5, 200], 3),
    entry("power",                             true,  "W",     [2, 3, 4, 5, 200], 0),
    entry("volt_amps",                         true,  "VA",    [2, 3, 5, 200], 0),
    entry("volt_amps_reactive",                true,  "var",   [2, 3, 5, 200], 0),
    entry("power_factor",                      true,  "",      [2, 3, 5, 200], 3),
    entry("phase_angle",                       true,  "°",     [2, 5, 200],  1),
    entry("average_line_to_neutral_volts",     false, "V",     [2, 5, 200],  1),
    entry("average_line_current",              false, "A",     [2, 3, 5, 200], 3),
    entry("sum_of_line_currents",              false, "A",     [2, 3, 5, 200], 3),
    entry("total_system_power",                false, "W",     [2, 3, 4, 5, 200], 0),
    entry("total_system_volt_amps",            false, "VA",    [2, 3, 5, 200], 0),
    entry("total_system_var",                  false, "var",   [2, 3, 5, 200], 0),
    entry("total_system_power_factor",         false, "",      [2, 3, 5, 200], 3),
    entry("total_system_phase_angle",          false, "°",     [2, 5, 200],  1),
    entry("frequency_of_supply_voltages",      false, "Hz",    [2, 3, 5, 200], 3),
    entry("total_import_kwh",                  false, "kWh",   [2, 3, 4, 5, 200], 3),
    entry("total_export_kwh",                  false, "kWh",   [2, 3, 4, 5, 200], 3),
    entry("total_import_kvarh",                false, "kvarh", [2, 5, 200],  3),
    entry("total_export_kvarh",                false, "kvarh", [2, 5, 200],  3),
    entry("total_vah",                         false, "kVAh",  [2, 5, 200],  3),
    entry("ah",                                false, "Ah",    [2, 5, 200],  3),
    entry("total_system_power_demand",         false, "W",     [2, 5, 200],  0),
    entry("maximum_total_system_power_demand", false, "W",     [2, 5, 200],  0),
    entry("total_system_va_demand",            false, "VA",    [2, 5, 200],  0),
    entry("maximum_total_system_va_demand",    false, "VA",    [2, 5, 200],  0),
    entry("neutral_current_demand",            false, "A",     [2, 5, 200],  3),
    entry("maximum_neutral_current_demand",    false, "A",     [2, 5, 200],  3),
    entry("line1_to_line2_volts",              false, "V",     [2, 3, 5, 200], 1),
    entry("line2_to_line3_volts",              false, "V",     [2, 3, 5, 200], 1),
    entry("line3_to_line1_volts",              false, "V",     [2, 3, 5, 200], 1),
    entry("average_line_to_line_volts",        false, "V",     [2, 3, 5, 200], 1),
    entry("neutral_current",                   false, "A",     [2, 3, 5, 200], 3),
    entry("ln_volts_thd",                      true,  "%",     [2, 5, 200],  1),
    entry("current_thd",                       true,  "%",     [2, 5, 200],  1),
    entry("average_line_to_neutral_volts_thd", false, "%",     [2, 5, 200],  1),
    entry("average_line_current_thd",          false, "%",     [2, 5, 200],  1),
    entry("current_demand",                    true,  "A",     [2, 5, 200],  3),
    entry("maximum_current_demand",            true,  "A",     [2, 5, 200],  3),
    entry("line1_to_line2_volts_thd",          false, "%",     [2, 5, 200],  1),
    entry("line2_to_line3_volts_thd",          false, "%",     [2, 5, 200],  1),
    entry("line3_to_line1_volts_thd",          false, "%",     [2, 5, 200],  1),
    entry("average_line_to_line_volts_thd",    false, "%",     [2, 5, 200],  1),
    entry("total_kwh_sum",                     false, "kWh",   [2, 3, 4, 5, 200], 3),
    entry("total_kvarh_sum",                   false, "kvarh", [2, 3, 5, 200], 3),
    entry("import_kwh",                        true,  "kWh",   [2, 5, 200],  3),
    entry("export_kwh",                        true,  "kWh",   [2, 5, 200],  3),
    entry("total_kwh",                         true,  "kWh",   [2, 5, 200],  3),
    entry("import_kvarh",                      true,  "kvarh", [2, 5, 200],  3),
    entry("export_kvarh",                      true,  "kvarh", [2, 5, 200],  3),
    entry("total_kvarh",                       true,  "kvarh", [2, 5, 200],  3)
];

interface UplotData {
    timestamps: number[];
    samples: number[];
}

interface UplotWrapperProps {
    id: string;
    class: string;
    sidebar_id: string;
    y_min: number;
    y_max: number;
}

class UplotWrapper extends Component<UplotWrapperProps, {}> {
    uplot: uPlot;
    pending_data: UplotData;
    visible: boolean = false;
    div_ref = createRef();
    observer: ResizeObserver;

    shouldComponentUpdate() {
        return false;
    }

    componentDidMount() {
        if (this.uplot) {
            return;
        }

        // FIXME: special hack for status page that is visible by default
        //        and doesn't receive an initial shown event because of that
        this.visible = this.props.sidebar_id === "status";

        // We have to use jquery here or else the events don't fire?
        // This can be removed once the sidebar is ported to preact.
        $(`#sidebar-${this.props.sidebar_id}`).on('shown.bs.tab', () => {
            this.visible = true;

            if (this.pending_data !== undefined) {
                this.set_data(this.pending_data);
            }
        });

        $(`#sidebar-${this.props.sidebar_id}`).on('hidden.bs.tab', () => {
            this.visible = false;
        });

        let get_size = () => {
            let div = this.div_ref.current;
            let aspect_ratio = parseFloat(getComputedStyle(div).aspectRatio);

            return {
                width: div.clientWidth,
                height: Math.floor(div.clientWidth / aspect_ratio),
            }
        }

        let options = {
            ...get_size(),
            pxAlign: 0,
            cursor: {
                drag: {
                    x: false, // disable zoom
                },
            },
            series: [
                {
                    label: __("meter.script.time"),
                    value: __("meter.script.time_legend_format"),
                },
                {
                    show: true,
                    pxAlign: 0,
                    spanGaps: false,
                    label: __("meter.script.power"),
                    value: (self: uPlot, rawValue: number) => rawValue !== null ? rawValue + " W" : null,
                    stroke: "#007bff",
                    width: 2,
                },
            ],
            axes: [
                {
                    incrs: [
                        60,
                        60 * 2,
                        3600,
                        3600 * 2,
                        3600 * 4,
                        3600 * 6,
                        3600 * 8,
                        3600 * 12,
                        3600 * 24,
                    ],
                    // [0]:   minimum num secs in found axis split (tick incr)
                    // [1]:   default tick format
                    // [2-7]: rollover tick formats
                    // [8]:   mode: 0: replace [1] -> [2-7], 1: concat [1] + [2-7]
                    values: [
                        // tick incr  default      year                                             month  day                                               hour  min   sec   mode
                        [3600,        "{HH}:{mm}", "\n" + __("meter.script.time_long_date_format"), null,  "\n" + __("meter.script.time_short_date_format"), null, null, null, 1],
                        [60,          "{HH}:{mm}", null,                                            null,  null,                                             null, null, null, 1],
                    ],
                },
                {
                    size: 55,
                }
            ],
            scales: {
                y: {
                    range: {
                        max: {
                            soft: this.props.y_max,
                            mode: (this.props.y_max !== undefined ? 1 : 3) as uPlot.Range.SoftMode,
                        },
                        min: {
                            soft: this.props.y_min,
                            mode: (this.props.y_min !== undefined ? 1 : 3) as uPlot.Range.SoftMode,
                        },
                    },
                },
            },
        };

        let div = this.div_ref.current;
        this.uplot = new uPlot(options, [], div);

        let resize = () => {
            let size = get_size();

            if (size.width == 0 || size.height == 0) {
                return;
            }

            this.uplot.setSize(size);
        };

        try {
            this.observer = new ResizeObserver(() => {
                resize();
            });

            this.observer.observe(div);
        } catch (e) {
            setInterval(() => {
                resize();
            }, 500);

            window.addEventListener("resize", e => {
                resize();
            });
        }
    }

    render(props?: UplotWrapperProps, state?: Readonly<{}>, context?: any): ComponentChild {
        return <div><div ref={this.div_ref} id={props.id} class={props.class} /></div>;
    }

    set_data(data: UplotData) {
        if (!this.uplot || !this.visible) {
            this.pending_data = data;
            return;
        }

        this.pending_data = undefined;

        this.uplot.setData([data.timestamps, data.samples]);
    }
}

interface MeterState {
    state: Readonly<API.getType['meter/state']>;
    values: Readonly<API.getType['meter/values']>;
    phases: Readonly<API.getType['meter/phases']>;
    all_values: Readonly<API.getType['meter/all_values']>;
    chart_selected: "history"|"live";
}

function calculate_live_data(offset: number, samples_per_second: number, samples: number[]): UplotData {
    let data: UplotData = {timestamps: new Array(samples.length), samples: samples};
    let now = Date.now();
    let start;
    let step;

    if (samples_per_second == 0) { // implies samples.length == 1
        start = now - offset;
        step = 0;
    } else {
        // (samples.length - 1) because samples_per_second defines the gaps between
        // two samples. with N samples there are (N - 1) gaps, while the lastest/newest
        // sample is offset milliseconds old
        start = now - (samples.length - 1) / samples_per_second * 1000 - offset;
        step = 1 / samples_per_second * 1000;
    }

    for(let i = 0; i < samples.length; ++i) {
        data.timestamps[i] = (start + i * step) / 1000;
    }

    return data;
}

function calculate_history_data(offset: number, samples: number[]): UplotData {
    const HISTORY_MINUTE_INTERVAL = 4;

    let data: UplotData = {timestamps: new Array(samples.length), samples: samples};
    let now = Date.now();
    let step = HISTORY_MINUTE_INTERVAL * 60 * 1000;
    // (samples.length - 1) because step defines the gaps between two samples.
    // with N samples there are (N - 1) gaps, while the lastest/newest sample is
    // offset milliseconds old. there might be no data point on a full hour
    // interval. to get nice aligned ticks nudge the ticks by at most half of a
    // sampling interval
    let start = Math.round((now - (samples.length - 1) * step - offset) / step) * step;

    for(let i = 0; i < samples.length; ++i) {
        data.timestamps[i] = (start + i * step) / 1000;
    }

    return data;
}

function array_append<T>(a: Array<T>, b: Array<T>, tail: number): Array<T> {
    a.push(...b);

    return a.slice(-tail);
}

export class Meter extends Component<{}, MeterState> {
    live_data: UplotData;
    pending_live_data: UplotData = {timestamps: [], samples: []};
    history_data: UplotData;
    uplot_wrapper_ref = createRef();

    constructor() {
        super();

        let i = 0;
        for(let e of entries) {
            e.i = i;
            i += e.three_phase ? 3 : 1;
        }

        util.addApiEventListener("meter/state", () => {
            this.setState({state: API.get("meter/state")});
        });

        util.addApiEventListener("meter/values", () => {
            this.setState({values: API.get("meter/values")});
        });

        util.addApiEventListener("meter/phases", () => {
            this.setState({phases: API.get("meter/phases")});
        });

        util.addApiEventListener("meter/all_values", () => {
            this.setState({all_values: API.get("meter/all_values")});
        });

        util.addApiEventListener("meter/live", () => {
            let live = API.get("meter/live");

            this.live_data = calculate_live_data(live.offset, live.samples_per_second, live.samples);
            this.pending_live_data = {timestamps: [], samples: []};

            if (this.state.chart_selected == "live") {
                this.update_uplot(this.live_data);
            }
        });

        util.addApiEventListener("meter/live_samples", () => {
            let live = API.get("meter/live_samples");
            let live_extra = calculate_live_data(0, live.samples_per_second, live.samples);

            this.pending_live_data.timestamps.push(...live_extra.timestamps);
            this.pending_live_data.samples.push(...live_extra.samples);

            if (this.pending_live_data.samples.length >= 5) {
                this.live_data.timestamps = array_append(this.live_data.timestamps, this.pending_live_data.timestamps, 720);
                this.live_data.samples = array_append(this.live_data.samples, this.pending_live_data.samples, 720);

                this.pending_live_data.timestamps = [];
                this.pending_live_data.samples = [];

                if (this.state.chart_selected == "live") {
                    this.update_uplot(this.live_data);
                }
            }
        });

        util.addApiEventListener("meter/history", () => {
            let history = API.get("meter/history");

            this.history_data = calculate_history_data(history.offset, history.samples);

            if (this.state.chart_selected == "history") {
                this.update_uplot(this.history_data);
            }
        });

        util.addApiEventListener("meter/history_samples", () => {
            let history = API.get("meter/history_samples");

            this.history_data = calculate_history_data(0, array_append(this.history_data.samples, history.samples, 720));

            if (this.state.chart_selected == "history") {
                this.update_uplot(this.history_data);
            }
        });

        this.state = {
            chart_selected: "history",
        } as any;
    }

    update_uplot(data: UplotData) {
        if (!this.uplot_wrapper_ref || !this.uplot_wrapper_ref.current) {
            return;
        }

        this.uplot_wrapper_ref.current.set_data(data);
    }

    render(props: {}, state: Readonly<MeterState>) {
        if (!util.allow_render) {
            return (<></>);
        }

        return (
            <>
                <PageHeader title={__("meter.content.energy_meter")} colClasses="col-xl-10"/>
                    <div class="row">
                        <div class="col-lg-6">
                            <div class="row mb-3 pt-3 pr-0 pr-lg-3">
                                <div class="d-flex justify-content-between align-items-center border-bottom col">
                                    <span class="h3">{__("meter.status.charge_history")}</span>
                                    <div class="mb-2">
                                        <InputSelect value={this.state.chart_selected} onValue={(v) => {
                                            let chart_selected: "live"|"history" = v as any;

                                            this.setState({chart_selected: chart_selected});
                                            this.update_uplot(this[`${chart_selected}_data`]);
                                        }}
                                            items={[
                                                ["history", __("meter.content.history")],
                                                ["live", __("meter.content.live")],
                                            ]}/>
                                    </div>
                                </div>
                            </div>
                            <UplotWrapper ref={this.uplot_wrapper_ref} id="meter_chart" class="meter-chart" sidebar_id="meter" y_min={undefined} y_max={undefined} />
                        </div>
                        <div class="col-lg-6 col-xl-4">
                            <div class="row mb-3 pt-3">
                                <div class="d-flex justify-content-between align-items-center border-bottom col">
                                    <span class="h3">{__("meter.content.statistics")}</span>
                                    <div class="mb-2" style="visibility: hidden;">
                                        <InputSelect items={[["a", "a"]]} />
                                    </div>
                                </div>
                            </div>
                            <FormRow label={__("meter.content.power")} labelColClasses="col-sm-6" contentColClasses="col-sm-6">
                                <OutputFloat value={this.state.values.power} digits={0} scale={0} unit="W"/>
                            </FormRow>
                            {!API.hasFeature("energy_manager") ? // TODO Remove hack when meter module is removed from Energy Manager.
                            <>
                            <FormRow label={__("meter.content.energy")} label_muted={__("meter.content.energy_since_reset")} labelColClasses="col-sm-6" contentColClasses="col-sm-6">
                                <OutputFloat value={this.state.values.energy_rel} digits={3} scale={0} unit="kWh"/>
                                <Button className="form-control mt-2" onClick={async () => {
                                     const modal = util.async_modal_ref.current;
                                     if (!await modal.show({
                                             title: __("meter.content.reset_modal"),
                                             body: __("meter.content.reset_modal_body"),
                                             no_text: __("meter.content.reset_modal_abort"),
                                             yes_text: __("meter.content.reset_modal_confirm"),
                                             no_variant: "secondary",
                                             yes_variant: "danger"
                                         }))
                                    return;

                                    API.call("meter/reset", {}, __("meter.content.reset_failed"));
                                }}>{__("meter.content.reset_statistics")}</Button>

                            </FormRow>
                            <FormRow label={__("meter.content.energy")} label_muted={__("meter.content.energy_lifetime")} labelColClasses="col-sm-6" contentColClasses="col-sm-6">
                                <OutputFloat value={this.state.values.energy_abs} digits={3} scale={0} unit="kWh"/>
                            </FormRow>
                            </> : undefined}

                            {API.hasFeature("meter_phases") ?
                            <>
                                <FormRow label={ __("meter.content.phases_connected")} label_muted={__("meter.content.phases_connected_desc")} labelColClasses="col-sm-6" contentColClasses="col-sm-6">
                                    <div class="row mx-n1">
                                        {this.state.phases.phases_connected.map((x, j) => (
                                            <IndicatorGroup vertical key={j} class="mb-1 px-1 flex-wrap col-4"
                                                value={x ? 0 : 1} //intentionally inverted: the high button is the first
                                                items={[
                                                    ["primary", <Zap/>],
                                                    ["secondary", <ZapOff/>]
                                                ]}/>
                                        ))}
                                    </div>
                                </FormRow>

                                <FormRow label={ __("meter.content.phases_active")} label_muted={__("meter.content.phases_active_desc")} labelColClasses="col-sm-6" contentColClasses="col-sm-6">
                                    <div class="row mx-n1">
                                        {this.state.phases.phases_active.map((x, j) => (
                                            <IndicatorGroup vertical key={j} class="mb-1 px-1 flex-wrap col-4"
                                                value={x ? 0 : 1} //intentionally inverted: the high button is the first
                                                items={[
                                                    ["primary", <Zap/>],
                                                    ["secondary", <ZapOff/>]
                                                ]}/>
                                        ))}
                                    </div>
                                </FormRow>
                            </>: undefined}
                        </div>
                    </div>
                    {API.hasFeature("meter_all_values") ?
                    <CollapsedSection colClasses="col-xl-10" label={__("meter.content.detailed_values")}>
                        {
                        entries.filter(e => e.meter_types.indexOf(state.state.type) > 0).map(e => <FormRow label={e.name} label_muted={e.desc} labelColClasses="col-lg-3 col-xl-3" contentColClasses="col-lg-9 col-xl-7">
                            {e.three_phase ? <div class="row">
                                <div class="mb-1 col-12 col-sm-4">
                                    <OutputFloat value={this.state.all_values[e.i + 0]} digits={e.digits} scale={0} unit={e.unit}/>
                                </div>
                                <div class="mb-1 col-12 col-sm-4">
                                    <OutputFloat value={this.state.all_values[e.i + 1]} digits={e.digits} scale={0} unit={e.unit}/>
                                </div>
                                <div class="mb-1 col-12 col-sm-4">
                                    <OutputFloat value={this.state.all_values[e.i + 2]} digits={e.digits} scale={0} unit={e.unit}/>
                                </div>
                            </div> : <div class="row"><div class="col-sm-4"><OutputFloat value={this.state.all_values[e.i]} digits={e.digits} scale={0} unit={e.unit}/></div></div>}
                        </FormRow>)
                        }
                    </CollapsedSection> : undefined}
            </>
        )
    }
}

render(<Meter />, $('#meter')[0]);

export class StatusMeterChart extends Component<{}, {}> {
    history_data: UplotData;
    uplot_wrapper_ref = createRef();

    constructor() {
        super();

        util.addApiEventListener("meter/history", () => {
            let history = API.get("meter/history");

            this.history_data = calculate_history_data(history.offset, history.samples);

            this.update_uplot();
        });

        util.addApiEventListener("meter/history_samples", () => {
            let history = API.get("meter/history_samples");

            this.history_data = calculate_history_data(0, array_append(this.history_data.samples, history.samples, 720));

            this.update_uplot();
        });
    }

    update_uplot() {
        if (!this.history_data || !this.uplot_wrapper_ref || !this.uplot_wrapper_ref.current) {
            return;
        }

        this.uplot_wrapper_ref.current.set_data(this.history_data);
    }

    render(props: {}, state: {}) {
        return (
            <>
                <UplotWrapper ref={this.uplot_wrapper_ref} id="status_meter_chart" class="status-meter-chart" sidebar_id="status" y_min={0} y_max={1500} />
            </>
        )
    }
}

render(<StatusMeterChart />, $('#status_meter_chart_container')[0]);

let meter_show_status = true;

function update_meter_values() {
    let values = API.get('meter/values');

    $('#status_meter_power').val(util.toLocaleFixed(values.power, 0) + " W");
}

export function init() {
}

function update_module_visibility() {
    let have_meter = API.hasFeature('meter');

    $('#sidebar-meter').prop('hidden', !have_meter);

    // Don't use meter status if the Energy Manager module is loaded.
    // The Energy Manager has its own status component
    $('#status-meter').prop('hidden', !meter_show_status || !have_meter);
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('meter/values', update_meter_values);
    source.addEventListener('info/features', update_module_visibility);
}

export function update_sidebar_state(module_init: any) {
    meter_show_status = !module_init.energy_manager;
}
