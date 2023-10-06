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
import { Button } from "react-bootstrap";
import { InputSelect } from "../../ts/components/input_select";
import { CollapsedSection } from "../../ts/components/collapsed_section";
import { OutputFloat } from "../../ts/components/output_float";
import { Zap, ZapOff } from "react-feather";
import uPlot from "uplot";
import { FormSeparator } from "../../ts/components/form_separator";
import { SubPage } from "../../ts/components/sub_page";

interface DetailedViewEntry {
    i: number,
    name: string,
    desc: string,
    three_phase: boolean,
    unit: string
    digits: 0|1|2|3
}

function entry(name: string, three_phase: boolean, unit: string, digits: 0|1|2|3) : DetailedViewEntry {
    return {i: 0, name: translate_unchecked(`meter.content.detailed_${name}`), desc: translate_unchecked(`meter.content.detailed_${name}_desc`), three_phase: three_phase, unit: unit, digits: digits}
}

const entries: DetailedViewEntry[] = [
    entry("line_to_neutral_volts",             true,  "V",     1),
    entry("current",                           true,  "A",     3),
    entry("power",                             true,  "W",     0),
    entry("volt_amps",                         true,  "VA",    0),
    entry("volt_amps_reactive",                true,  "var",   0),
    entry("power_factor",                      true,  "",      3),
    entry("phase_angle",                       true,  "°",     1),
    entry("average_line_to_neutral_volts",     false, "V",     1),
    entry("average_line_current",              false, "A",     3),
    entry("sum_of_line_currents",              false, "A",     3),
    entry("total_system_power",                false, "W",     0),
    entry("total_system_volt_amps",            false, "VA",    0),
    entry("total_system_var",                  false, "var",   0),
    entry("total_system_power_factor",         false, "",      3),
    entry("total_system_phase_angle",          false, "°",     1),
    entry("frequency_of_supply_voltages",      false, "Hz",    3),
    entry("total_import_kwh",                  false, "kWh",   3),
    entry("total_export_kwh",                  false, "kWh",   3),
    entry("total_import_kvarh",                false, "kvarh", 3),
    entry("total_export_kvarh",                false, "kvarh", 3),
    entry("total_vah",                         false, "kVAh",  3),
    entry("ah",                                false, "Ah",    3),
    entry("total_system_power_demand",         false, "W",     0),
    entry("maximum_total_system_power_demand", false, "W",     0),
    entry("total_system_va_demand",            false, "VA",    0),
    entry("maximum_total_system_va_demand",    false, "VA",    0),
    entry("neutral_current_demand",            false, "A",     3),
    entry("maximum_neutral_current_demand",    false, "A",     3),
    entry("line1_to_line2_volts",              false, "V",     1),
    entry("line2_to_line3_volts",              false, "V",     1),
    entry("line3_to_line1_volts",              false, "V",     1),
    entry("average_line_to_line_volts",        false, "V",     1),
    entry("neutral_current",                   false, "A",     3),
    entry("ln_volts_thd",                      true,  "%",     1),
    entry("current_thd",                       true,  "%",     1),
    entry("average_line_to_neutral_volts_thd", false, "%",     1),
    entry("average_line_current_thd",          false, "%",     1),
    entry("current_demand",                    true,  "A",     3),
    entry("maximum_current_demand",            true,  "A",     3),
    entry("line1_to_line2_volts_thd",          false, "%",     1),
    entry("line2_to_line3_volts_thd",          false, "%",     1),
    entry("line3_to_line1_volts_thd",          false, "%",     1),
    entry("average_line_to_line_volts_thd",    false, "%",     1),
    entry("total_kwh_sum",                     false, "kWh",   3),
    entry("total_kvarh_sum",                   false, "kvarh", 3),
    entry("import_kwh",                        true,  "kWh",   3),
    entry("export_kwh",                        true,  "kWh",   3),
    entry("total_kwh",                         true,  "kWh",   3),
    entry("import_kvarh",                      true,  "kvarh", 3),
    entry("export_kvarh",                      true,  "kvarh", 3),
    entry("total_kvarh",                       true,  "kvarh", 3)
];

interface UplotData {
    timestamps: number[];
    samples: number[];
}

interface UplotWrapperProps {
    id: string;
    class: string;
    sidebar_id: string;
    show: boolean;
    legend_time_with_seconds: boolean;
    x_height: number;
    x_include_date: boolean;
    y_min?: number;
    y_max?: number;
    y_diff_min?: number;
}

class UplotWrapper extends Component<UplotWrapperProps, {}> {
    uplot: uPlot;
    data: UplotData;
    pending_data: UplotData;
    visible: boolean = false;
    div_ref = createRef();
    observer: ResizeObserver;
    y_min: number = 0;
    y_max: number = 0;

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

            if (isNaN(aspect_ratio)) {
                aspect_ratio = 2;
            }

            return {
                width: div.clientWidth,
                height: Math.floor((div.clientWidth + (window.innerWidth - document.documentElement.clientWidth)) / aspect_ratio),
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
                    value: (self: uPlot, rawValue: number) => {
                        if (rawValue !== null) {
                            if (this.props.legend_time_with_seconds) {
                                return util.timestamp_sec_to_date(rawValue)
                            }
                            else {
                                return util.timestamp_min_to_date((rawValue / 60), '???');
                            }
                        }

                        return null;
                    },
                },
                {
                    show: true,
                    pxAlign: 0,
                    spanGaps: false,
                    label: __("meter.script.power"),
                    value: (self: uPlot, rawValue: number) => util.hasValue(rawValue) ? util.toLocaleFixed(rawValue) + " W" : null,
                    stroke: "rgb(0, 123, 255)",
                    fill: "rgb(0, 123, 255, 0.1)",
                    width: 2,
                    points: {
                        show: false,
                    },
                },
            ],
            axes: [
                {
                    size: this.props.x_height,
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
                    values: (self: uPlot, splits: number[], axisIdx: number, foundSpace: number, foundIncr: number) => {
                        let values: string[] = new Array(splits.length);
                        let last_year: string = null;
                        let last_month_and_day: string = null;

                        for (let i = 0; i < splits.length; ++i) {
                            let date = new Date(splits[i] * 1000);
                            let value = date.toLocaleString([], {hour: '2-digit', minute: '2-digit'});

                            if (this.props.x_include_date && foundIncr >= 3600) {
                                let year = date.toLocaleString([], {year: 'numeric'});
                                let month_and_day = date.toLocaleString([], {month: '2-digit', day: '2-digit'});

                                if (year != last_year) {
                                    value += '\n' + date.toLocaleString([], {year: 'numeric', month: '2-digit', day: '2-digit'});
                                    last_year = year;
                                    last_month_and_day = month_and_day;
                                }

                                if (month_and_day != last_month_and_day) {
                                    value += '\n' + date.toLocaleString([], {month: '2-digit', day: '2-digit'});
                                    last_month_and_day = month_and_day;
                                }
                            }

                            values[i] = value;
                        }

                        return values;
                    },
                },
                {
                    label: __("meter.script.power") + " [Watt]",
                    labelSize: 20,
                    labelGap: 2,
                    labelFont: 'bold 14px system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, "Noto Sans", sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol", "Noto Color Emoji"',
                    size: (self: uPlot, values: string[], axisIdx: number, cycleNum: number): number => {
                        let size = 0;

                        if (values) {
                            self.ctx.save();
                            self.ctx.font = self.axes[axisIdx].font;

                            for (let i = 0; i < values.length; ++i) {
                                size = Math.max(size, self.ctx.measureText(values[i]).width);
                            }

                            self.ctx.restore();
                        }

                        return Math.ceil(size / devicePixelRatio) + 20;
                    },
                    values: (self: uPlot, splits: number[]) => {
                        let values: string[] = new Array(splits.length);

                        for (let digits = 0; digits <= 3; ++digits) {
                            let last_value: string = null;
                            let unique = true;

                            for (let i = 0; i < splits.length; ++i) {
                                values[i] = util.toLocaleFixed(splits[i], digits);

                                if (last_value == values[i]) {
                                    unique = false;
                                }

                                last_value = values[i];
                            }

                            if (unique) {
                                break;
                            }
                        }

                        return values;
                    },
                }
            ],
            scales: {
                y: {
                    range: (self: uPlot, initMin: number, initMax: number, scaleKey: string): uPlot.Range.MinMax => {
                        return uPlot.rangeNum(this.y_min, this.y_max, {min: {}, max: {}});
                    }
                },
            },
            padding: [null, 20, null, 5] as uPlot.Padding,
            plugins: [
                {
                    hooks: {
                        setSeries: (self: uPlot, seriesIdx: number, opts: uPlot.Series) => {
                            this.update_internal_data();
                        },
                        drawAxes: [
                            (self: uPlot) => {
                                let ctx = self.ctx;

                                ctx.save();

                                let s  = self.series[0];
                                let xd = self.data[0];
                                let [i0, i1] = s.idxs;
                                let x0 = self.valToPos(xd[i0], 'x', true) - self.axes[0].ticks.size;
                                let y0 = self.valToPos(0, 'y', true);
                                let x1 = self.valToPos(xd[i1], 'x', true);
                                let y1 = self.valToPos(0, 'y', true);

                                const lineWidth = 2;
                                const offset = (lineWidth % 2) / 2;

                                ctx.translate(offset, offset);

                                ctx.beginPath();
                                ctx.lineWidth = lineWidth;
                                ctx.strokeStyle = 'rgb(0,0,0,0.2)';
                                ctx.moveTo(x0, y0);
                                ctx.lineTo(x1, y1);
                                ctx.stroke();

                                ctx.translate(-offset, -offset);

                                ctx.restore();
                            }
                        ],
                    },
                },
            ],
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

        if (this.pending_data !== undefined) {
            this.set_data(this.pending_data);
        }
    }

    render(props?: UplotWrapperProps, state?: Readonly<{}>, context?: any): ComponentChild {
        // the plain div is neccessary to make the size calculation stable in safari. without this div the height continues to grow
        return <div><div ref={this.div_ref} id={props.id} class={props.class} style={`display: ${props.show ? 'block' : 'none'};`} /></div>;
    }

    set_show(show: boolean) {
        this.div_ref.current.style.display = show ? 'block' : 'none';
    }

    update_internal_data() {
        let y_min: number = this.props.y_min;
        let y_max: number = this.props.y_max;

        for (let i = 0; i < this.data.samples.length; ++i) {
            let value = this.data.samples[i];

            if (value !== null) {
                if (y_min === undefined || value < y_min) {
                    y_min = value;
                }

                if (y_max === undefined || value > y_max) {
                    y_max = value;
                }
            }
        }

        if (y_min === undefined && y_max === undefined) {
            y_min = 0;
            y_max = 0;
        }
        else if (y_min === undefined) {
            y_min = y_max;
        }
        else if (y_max === undefined) {
            y_max = y_min;
        }

        let y_diff_min = this.props.y_diff_min;

        if (y_diff_min !== undefined) {
            let y_diff = y_max - y_min;

            if (y_diff < y_diff_min) {
                let y_center = y_min + y_diff / 2;

                let new_y_min = Math.floor(y_center - y_diff_min / 2);
                let new_y_max = Math.ceil(y_center + y_diff_min / 2);

                if (new_y_min < 0 && y_min >= 0) {
                    // avoid negative range, if actual minimum is positive
                    y_min = 0;
                    y_max = y_diff_min;
                } else {
                    y_min = new_y_min;
                    y_max = new_y_max;
                }
            }
        }

        this.y_min = y_min;
        this.y_max = y_max;

        this.uplot.setData([this.data.timestamps, this.data.samples]);
    }

    set_data(data: UplotData) {
        if (!this.uplot || !this.visible) {
            this.pending_data = data;
            return;
        }

        this.data = data;
        this.pending_data = undefined;

        this.update_internal_data();
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
    uplot_wrapper_live_ref = createRef();
    uplot_wrapper_history_ref = createRef();

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
                this.update_uplot();
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
                    this.update_uplot();
                }
            }
        });

        util.addApiEventListener("meter/history", () => {
            let history = API.get("meter/history");

            this.history_data = calculate_history_data(history.offset, history.samples);

            if (this.state.chart_selected == "history") {
                this.update_uplot();
            }
        });

        util.addApiEventListener("meter/history_samples", () => {
            let history = API.get("meter/history_samples");

            this.history_data = calculate_history_data(0, array_append(this.history_data.samples, history.samples, 720));

            if (this.state.chart_selected == "history") {
                this.update_uplot();
            }
        });

        this.state = {
            chart_selected: "history",
        } as any;
    }

    update_uplot() {
        if (this.state.chart_selected == 'live') {
            if (this.uplot_wrapper_live_ref && this.uplot_wrapper_live_ref.current) {
                this.uplot_wrapper_live_ref.current.set_data(this.live_data);
            }
        }
        else {
            if (this.uplot_wrapper_history_ref && this.uplot_wrapper_history_ref.current) {
                this.uplot_wrapper_history_ref.current.set_data(this.history_data);
            }
        }
    }

    render(props: {}, state: Readonly<MeterState>) {
        if (!util.render_allowed() || !API.hasFeature("meter")) {
            return (<></>);
        }

        return (
            <SubPage colClasses="col-xl-10">
                <PageHeader title={__("meter.content.energy_meter")}/>
                    <div class="row">
                        <div class="col-lg-6">
                            <FormSeparator heading={__("meter.status.charge_history")} first={true} colClasses={"justify-content-between align-items-center col"} extraClasses={"pr-0 pr-lg-3"} >
                                <div class="mb-2">
                                    <InputSelect value={this.state.chart_selected} onValue={(v) => {
                                        let chart_selected: "live"|"history" = v as any;

                                        this.setState({chart_selected: chart_selected}, () => {
                                            if (chart_selected == 'live') {
                                                this.uplot_wrapper_live_ref.current.set_show(true);
                                                this.uplot_wrapper_history_ref.current.set_show(false);
                                            }
                                            else {
                                                this.uplot_wrapper_history_ref.current.set_show(true);
                                                this.uplot_wrapper_live_ref.current.set_show(false);
                                            }

                                            this.update_uplot();
                                        });
                                    }}
                                        items={[
                                            ["history", __("meter.content.history")],
                                            ["live", __("meter.content.live")],
                                        ]}/>
                                </div>
                            </FormSeparator>
                            <UplotWrapper ref={this.uplot_wrapper_live_ref}
                                          id="meter_chart_live"
                                          class="meter-chart"
                                          sidebar_id="meter"
                                          show={false}
                                          legend_time_with_seconds={true}
                                          x_height={30}
                                          x_include_date={false}
                                          y_diff_min={100} />
                            <UplotWrapper ref={this.uplot_wrapper_history_ref}
                                          id="meter_chart_history"
                                          class="meter-chart"
                                          sidebar_id="meter"
                                          show={true}
                                          legend_time_with_seconds={false}
                                          x_height={50}
                                          x_include_date={true}
                                          y_min={0}
                                          y_max={1500} />
                        </div>
                        <div class="col-lg-6">
                            <FormSeparator heading={__("meter.content.statistics")} first={true} colClasses={"justify-content-between align-items-center col"} >
                                <div class="mb-2" style="visibility: hidden;">
                                    <InputSelect items={[["a", "a"]]} />
                                </div>
                            </FormSeparator>
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
                                                    ["dark", <ZapOff/>]
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
                                                    ["dark", <ZapOff/>]
                                                ]}/>
                                        ))}
                                    </div>
                                </FormRow>
                            </>: undefined}
                        </div>
                    </div>
                    {API.hasFeature("meter_all_values") ?
                    <CollapsedSection label={__("meter.content.detailed_values")}>
                        {
                        entries.filter(e => this.state.all_values[e.i] != null).map(e => <FormRow label={e.name} label_muted={e.desc}>
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
            </SubPage>
        )
    }
}

render(<Meter />, $('#meter')[0]);

export class MeterStatus extends Component<{}, {}> {
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
        // Don't check util.render_allowed() here.
        // We can receive graph data points with the first web socket packet and
        // want to push them into the uplot graph immediately.
        // This only works if the wrapper component is already created.
        // Hide the form rows to fix any visual bugs instead.
        let show = API.hasFeature('meter') && !API.hasFeature("energy_manager");

        // As we don't check util.render_allowed(),
        // we have to handle rendering before the web socket connection is established.
        let power = API.get_unchecked('meter/values')?.power ?? 0;

        return (
            <>
                <FormRow label={__("meter.status.charge_history")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4" hidden={!show}>
                    <div class="card pl-1 pb-1">
                        <UplotWrapper ref={this.uplot_wrapper_ref}
                                      id="status_meter_chart"
                                      class="status-meter-chart"
                                      sidebar_id="status"
                                      show={true}
                                      legend_time_with_seconds={false}
                                      x_height={50}
                                      x_include_date={true}
                                      y_min={0}
                                      y_max={1500} />
                    </div>
                </FormRow>
                <FormRow label={__("meter.status.current_power")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4" hidden={!show}>
                    <OutputFloat value={power} digits={0} scale={0} unit="W" maxFractionalDigitsOnPage={0} maxUnitLengthOnPage={1}/>
                </FormRow>
            </>
        )
    }
}

render(<MeterStatus />, $("#status-meter")[0]);

export function init() {
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('info/features', () => $('#sidebar-meter').prop('hidden', !API.hasFeature('meter')));
}

export function update_sidebar_state(module_init: any) {}
