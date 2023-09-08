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

import { METERS_SLOTS } from "../../build";
import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { __, translate_unchecked } from "../../ts/translation";
import { h, render, createRef, Fragment, Component, RefObject, ComponentChild } from "preact";
import { PageHeader } from "../../ts/components/page_header";
import { FormRow } from "../../ts/components/form_row";
import { InputSelect } from "../../ts/components/input_select";
import { CollapsedSection } from "../../ts/components/collapsed_section";
import { OutputFloat } from "../../ts/components/output_float";
import uPlot from 'uplot';
import { SubPage } from "../../ts/components/sub_page";
import { MeterValueID, METER_VALUE_IDS, METER_VALUE_INFOS } from "./meter_value_id";

interface CachedData {
    timestamps: number[];
    samples: number[/*meter_slot*/][];
};

interface UplotData {
    keys: string[];
    names: string[];
    values: number[][];
}

interface UplotWrapperProps {
    id: string;
    class: string;
    sidebar_id: string;
    show: boolean;
    legend_time_with_seconds: boolean;
    aspect_ratio: number;
    x_height: number;
    x_include_date: boolean;
    y_min?: number;
    y_max?: number;
    y_diff_min?: number;
}

// https://seaborn.pydata.org/tutorial/color_palettes.html#qualitative-color-palettes
// sns.color_palette("tab10")
const strokes = [
    'rgb(  0, 123, 255)', // use bootstrap blue instead of tab10 blue, to avoid subtle conflict between plot and button blue
    'rgb(255, 127,  14)',
    'rgb( 44, 160,  44)',
    'rgb(214,  39,  40)',
    'rgb(148, 103, 189)',
    'rgb(140,  86,  75)',
    'rgb(227, 119, 194)',
    'rgb(127, 127, 127)',
    'rgb(188, 189,  34)',
    'rgb( 23, 190, 207)',
];

const fills = [
    'rgb(  0, 123, 255, 0.1)', // use bootstrap blue instead of tab10 blue, to avoid subtle conflict between plot and button blue
    'rgb(255, 127,  14, 0.1)',
    'rgb( 44, 160,  44, 0.1)',
    'rgb(214,  39,  40, 0.1)',
    'rgb(148, 103, 189, 0.1)',
    'rgb(140,  86,  75, 0.1)',
    'rgb(227, 119, 194, 0.1)',
    'rgb(127, 127, 127, 0.1)',
    'rgb(188, 189,  34, 0.1)',
    'rgb( 23, 190, 207, 0.1)',
];

let color_cache: {[id: string]: {stroke: string, fill: string}} = {};
let color_cache_next: {[id: string]: number} = {};

function get_color(group: string, name: string)
{
    let key = group + '-' + name;

    if (!color_cache[key]) {
        if (!util.hasValue(color_cache_next[group])) {
            color_cache_next[group] = 0;
        }

        color_cache[key] = {
            stroke: strokes[color_cache_next[group] % strokes.length],
            fill: fills[color_cache_next[group] % fills.length],
        };

        color_cache_next[group]++;
    }

    return color_cache[key];
}

class UplotWrapper extends Component<UplotWrapperProps, {}> {
    uplot: uPlot;
    data: UplotData;
    pending_data: UplotData;
    series_visibility: {[id: string]: boolean} = {};
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
                aspect_ratio = this.props.aspect_ratio;
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
                    label: __("meters.script.time"),
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
                    label: __("meters.script.power") + " [Watt]",
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
                            this.series_visibility[this.data.keys[seriesIdx]] = opts.show;
                            this.update_internal_data();
                        },
                        drawAxes: [
                            (self: uPlot) => {
                                let ctx = self.ctx;
                                let s  = self.series[0];
                                let xd = self.data[0];
                                let [i0, i1] = s.idxs;
                                let x0 = self.valToPos(xd[i0], 'x', true) - self.axes[0].ticks.size * devicePixelRatio;
                                let x1 = self.valToPos(xd[i1], 'x', true);
                                let y = self.valToPos(0, 'y', true);

                                if (y > ctx.canvas.height - this.props.x_height) {
                                    return;
                                }

                                const lineWidth = 2 * devicePixelRatio;
                                const offset = (lineWidth % 2) / 2;

                                ctx.save();
                                ctx.translate(offset, offset);
                                ctx.beginPath();
                                ctx.lineWidth = lineWidth;
                                ctx.strokeStyle = 'rgb(0,0,0,0.2)';
                                ctx.moveTo(x0, y);
                                ctx.lineTo(x1, y);
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

    get_series_opts(i: number, fill: boolean): uPlot.Series {
        let name = this.data.names[i];
        let color = get_color('default', name);

        return {
            show: this.series_visibility[this.data.keys[i]],
            pxAlign: 0,
            spanGaps: false,
            label: name,
            value: (self: uPlot, rawValue: number) => util.hasValue(rawValue) ? util.toLocaleFixed(rawValue) + " W" : null,
            stroke: color.stroke,
            fill: fill ? color.fill : undefined,
            width: 2,
            points: {
                show: false,
            },
        };
    }

    update_internal_data() {
        let y_min: number = this.props.y_min;
        let y_max: number = this.props.y_max;

        for (let i = 1; i < this.data.values.length; ++i) {
            for (let k = 0; k < this.data.values[i].length; ++k) {
                let value = this.data.values[i][k];

                if (value !== null) {
                    if (y_min === undefined || value < y_min) {
                        y_min = value;
                    }

                    if (y_max === undefined || value > y_max) {
                        y_max = value;
                    }
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

        this.uplot.setData(this.data.values as any);
    }

    set_data(data: UplotData) {
        if (!this.uplot || !this.visible) {
            this.pending_data = data;
            return;
        }

        this.data = data;
        this.pending_data = undefined;

        while (this.uplot.series.length > 1) {
            this.uplot.delSeries(this.uplot.series.length - 1);
        }

        if (this.data) {
            while (this.uplot.series.length < this.data.keys.length) {
                if (this.series_visibility[this.data.keys[this.uplot.series.length]] === undefined) {
                    this.series_visibility[this.data.keys[this.uplot.series.length]] = true;
                }

                this.uplot.addSeries(this.get_series_opts(this.uplot.series.length, this.data.keys.length <= 2));
            }
        }

        this.update_internal_data();
    }
}

type ValuesByID = {[id: number]: number};

interface MetersProps {
    status_ref: RefObject<MetersStatus>;
}

interface MetersState {
    state: {[meter_slot: number]: Readonly<API.getType['meters/0/state']>};
    values_by_id: {[meter_slot: number]: ValuesByID};
    chart_selected: "history"|"live";
}

function calculate_live_data(offset: number, samples_per_second: number, samples: number[/*meter_slot*/][]): CachedData {
    let timestamp_slot_count: number = 0;

    if (samples_per_second == 0) { // implies atmost one sample
        timestamp_slot_count = 1;
    } else {
        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            if (samples[meter_slot] !== null) {
                timestamp_slot_count = Math.max(timestamp_slot_count, samples[meter_slot].length);
            }
        }
    }

    let data: CachedData = {timestamps: new Array(timestamp_slot_count), samples: new Array(METERS_SLOTS)};
    let now = Date.now();
    let start: number;
    let step: number;

    if (samples_per_second == 0) {
        start = now - offset;
        step = 0;
    } else {
        // (timestamp_slot_count - 1) because samples_per_second defines the gaps between
        // two samples. with N samples there are (N - 1) gaps, while the lastest/newest
        // sample is offset milliseconds old
        start = now - (timestamp_slot_count - 1) / samples_per_second * 1000 - offset;
        step = 1 / samples_per_second * 1000;
    }

    for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
        data.timestamps[timestamp_slot] = (start + timestamp_slot * step) / 1000;
    }

    for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
        if (samples[meter_slot] === null) {
            data.samples[meter_slot] = [];
        }
        else {
            data.samples[meter_slot] = samples[meter_slot];
        }
    }

    return data;
}

function calculate_history_data(offset: number, samples: number[/*meter_slot*/][]): CachedData {
    const HISTORY_MINUTE_INTERVAL = 4;

    let timestamp_slot_count: number = 0;

    for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
        if (samples[meter_slot] !== null) {
            timestamp_slot_count = Math.max(timestamp_slot_count, samples[meter_slot].length);
        }
    }

    let data: CachedData = {timestamps: new Array(timestamp_slot_count), samples: new Array(METERS_SLOTS)};
    let now = Date.now();
    let step = HISTORY_MINUTE_INTERVAL * 60 * 1000;

    // (timestamp_slot_count - 1) because step defines the gaps between two samples.
    // with N samples there are (N - 1) gaps, while the lastest/newest sample is
    // offset milliseconds old. there might be no data point on a full hour
    // interval. to get nice aligned ticks nudge the ticks by at most half of a
    // sampling interval
    let start = Math.round((now - (timestamp_slot_count - 1) * step - offset) / step) * step;

    for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
        data.timestamps[timestamp_slot] = (start + timestamp_slot * step) / 1000;
    }

    for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
        if (samples[meter_slot] === null) {
            data.samples[meter_slot] = [];
        }
        else {
            data.samples[meter_slot] = samples[meter_slot];
        }
    }

    return data;
}

function array_append<T>(a: Array<T>, b: Array<T>, tail: number): Array<T> {
    a.push(...b);

    return a.slice(-tail);
}

export class Meters extends Component<MetersProps, MetersState> {
    live_data: CachedData = {timestamps: [], samples: []};
    pending_live_data: CachedData;
    history_data: CachedData = {timestamps: [], samples: []};
    uplot_wrapper_live_ref = createRef();
    uplot_wrapper_history_ref = createRef();
    status_ref: RefObject<MetersStatus> = null;
    value_ids: {[meter_slot: number]: Readonly<number[]>} = {};
    values: {[meter_slot: number]: Readonly<number[]>} = {};

    constructor(props: MetersProps) {
        super(props);

        this.status_ref = props.status_ref;

        this.state = {
            state: {},
            values_by_id: {},
            chart_selected: "history",
        } as any;

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            this.live_data.samples.push([]);
            this.history_data.samples.push([]);
        }

        util.eventTarget.addEventListener('info/modules', () => {
            if (!API.hasFeature('meters')) {
                console.log("Meters: meters feature not available");
                return;
            }

            this.update_live_cache();
            this.update_history_cache();
        });

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            util.addApiEventListener_unchecked(`meters/${meter_slot}/state`, () => {
                this.setState({state: API.get_maybe(`meters/${meter_slot}/state`)});
            });

            util.addApiEventListener_unchecked(`meters/${meter_slot}/value_ids`, () => {
                this.value_ids[meter_slot] = API.get_maybe(`meters/${meter_slot}/value_ids`);
                let values_by_id: ValuesByID = {};

                if (this.values[meter_slot]) {
                    for (let id of METER_VALUE_IDS) {
                        let idx = this.value_ids[meter_slot].indexOf(id);

                        if (idx >= 0) {
                            values_by_id[id] = this.values[meter_slot][idx];
                        }
                    }
                }

                this.setState((prevState) => ({
                    values_by_id: {
                        ...prevState.values_by_id,
                        [meter_slot]: values_by_id
                    }
                }));
            });

            util.addApiEventListener_unchecked(`meters/${meter_slot}/values`, () => {
                this.values[meter_slot] = API.get_maybe(`meters/${meter_slot}/values`);
                let values_by_id: ValuesByID = {};

                if (this.value_ids[meter_slot]) {
                    for (let id of METER_VALUE_IDS) {
                        let idx = this.value_ids[meter_slot].indexOf(id);

                        if (idx >= 0) {
                            values_by_id[id] = this.values[meter_slot][idx];
                        }
                    }
                }

                this.setState((prevState) => ({
                    values_by_id: {
                        ...prevState.values_by_id,
                        [meter_slot]: values_by_id
                    }
                }));
            });
        }

        util.addApiEventListener("meters/live_samples", () => {
            if (this.live_data.timestamps.length == 0) {
                // received live_samples before live cache initialization
                this.update_live_cache();
                return
            }

            let live = API.get("meters/live_samples");
            let live_extra = calculate_live_data(0, live.samples_per_second, live.samples);

            this.pending_live_data.timestamps.push(...live_extra.timestamps);

            for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                this.pending_live_data.samples[meter_slot].push(...live_extra.samples[meter_slot]);
            }

            if (this.pending_live_data.samples.length >= 5) {
                this.live_data.timestamps = array_append(this.live_data.timestamps, this.pending_live_data.timestamps, 720);

                for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                    this.live_data.samples[meter_slot] = array_append(this.live_data.samples[meter_slot], this.pending_live_data.samples[meter_slot], 720);
                }

                this.pending_live_data.timestamps = [];
                this.pending_live_data.samples = [];

                for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                    this.pending_live_data.samples.push([])
                }

                if (this.state.chart_selected == "live") {
                    this.update_uplot();
                }
            }
        });

        util.addApiEventListener("meters/history_samples", () => {
            if (this.history_data.timestamps.length == 0) {
                // received history_samples before history cache initialization
                this.update_history_cache();
                return
            }

            let history = API.get("meters/history_samples");
            let history_samples: number[][] = new Array(METERS_SLOTS);

            for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                if (history.samples[meter_slot] !== null) {
                    history_samples[meter_slot] = array_append(this.history_data.samples[meter_slot], history.samples[meter_slot], 720);
                }
            }

            this.history_data = calculate_history_data(0, history_samples);

            if (this.state.chart_selected == "history") {
                this.update_uplot();
            }

            this.update_status_uplot();
        });
    }

    update_live_cache() {
        this.update_live_cache_async()
            .then((success: boolean) => {
                if (!success) {
                    window.setTimeout(() => {
                        this.update_live_cache();
                    }, 100);

                    return;
                }

                this.update_uplot();
            });
    }

    async update_live_cache_async() {
        let response: string = '';

        try {
            response = await (await util.download('meters/live')).text();
        } catch (e) {
            console.log('Meters: Could not get meters live data: ' + e);
            return false;
        }

        let payload = JSON.parse(response);

        this.live_data = calculate_live_data(payload.offset, payload.samples_per_second, payload.samples);
        this.pending_live_data = {timestamps: [], samples: []}

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            this.pending_live_data.samples.push([])
        }

        return true;
    }

    update_history_cache() {
        this.update_history_cache_async()
            .then((success: boolean) => {
                if (!success) {
                    window.setTimeout(() => {
                        this.update_history_cache();
                    }, 100);

                    return;
                }

                this.update_uplot();
            });
    }

    async update_history_cache_async() {
        let response: string = '';

        try {
            response = await (await util.download('meters/history')).text();
        } catch (e) {
            console.log('Meters: Could not get meters history data: ' + e);
            return false;
        }

        let payload = JSON.parse(response);

        this.history_data = calculate_history_data(payload.offset, payload.samples);

        return true;
    }

    update_uplot() {
        if (this.state.chart_selected == 'live') {
            if (this.uplot_wrapper_live_ref && this.uplot_wrapper_live_ref.current) {
                let live_data: UplotData = {
                    keys: [null],
                    names: [null],
                    values: [this.live_data.timestamps],
                };

                for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                    if (this.live_data.samples[meter_slot].length > 0) {
                        live_data.keys.push('meter_' + meter_slot);
                        live_data.names.push('Meter #' + meter_slot); // FIXME: use meter display name
                        live_data.values.push(this.live_data.samples[meter_slot]);
                    }
                }

                this.uplot_wrapper_live_ref.current.set_data(live_data);
            }
        }
        else {
            if (this.uplot_wrapper_history_ref && this.uplot_wrapper_history_ref.current) {
                let history_data: UplotData = {
                    keys: [null],
                    names: [null],
                    values: [this.history_data.timestamps],
                };

                for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                    if (this.history_data.samples[meter_slot].length > 0) {
                        history_data.keys.push('meter_' + meter_slot);
                        history_data.names.push('Meter #' + meter_slot); // FIXME: use meter display name
                        history_data.values.push(this.history_data.samples[meter_slot]);
                    }
                }

                this.uplot_wrapper_history_ref.current.set_data(history_data);
            }
        }

        this.update_status_uplot();
    }

    update_status_uplot() {
        if (this.status_ref.current && this.status_ref.current.uplot_wrapper_ref.current) {
            let status_data: UplotData = {
                keys: [null],
                names: [null],
                values: [this.history_data.timestamps],
            };

            if (this.history_data.samples[this.status_ref.current.meter_slot].length > 0) {
                status_data.keys.push('meter_' + this.status_ref.current.meter_slot);
                status_data.names.push('Meter #' + this.status_ref.current.meter_slot); // FIXME: use meter display name
                status_data.values.push(this.history_data.samples[this.status_ref.current.meter_slot]);
            }

            this.status_ref.current.uplot_wrapper_ref.current.set_data(status_data);
        }
    }

    render(props: {}, state: Readonly<MetersState>) {
        if (!util.render_allowed() || !API.hasFeature("meters")) {
            return (<></>);
        }

        return (
            <SubPage colClasses="col-xl-10">
                <PageHeader title={__("meters.content.meters")}>
                    <div>
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
                                ["history", __("meters.content.history")],
                                ["live", __("meters.content.live")],
                            ]}/>
                    </div>
                </PageHeader>
                <UplotWrapper ref={this.uplot_wrapper_live_ref}
                                id="meters_chart_live"
                                class="meters-chart"
                                sidebar_id="meters"
                                show={false}
                                legend_time_with_seconds={true}
                                aspect_ratio={3}
                                x_height={30}
                                x_include_date={false}
                                y_diff_min={100} />
                <UplotWrapper ref={this.uplot_wrapper_history_ref}
                                id="meters_chart_history"
                                class="meters-chart"
                                sidebar_id="meters"
                                show={true}
                                legend_time_with_seconds={false}
                                aspect_ratio={3}
                                x_height={50}
                                x_include_date={true}
                                y_min={0}
                                y_max={1500} />

                {[...Array(METERS_SLOTS).keys()].map((meter_slot) =>
                <CollapsedSection label={__("meters.content.detailed_values")}>
                    {METER_VALUE_IDS.filter((id) => util.hasValue(this.state.values_by_id[meter_slot][id])).map((id) => <FormRow label={translate_unchecked(`meters.content.detailed_${id}`)} label_muted="?">
                        <div class="row"><div class="col-sm-4"><OutputFloat value={this.state.values_by_id[meter_slot][id]} digits={METER_VALUE_INFOS[id].digits} scale={0} unit={METER_VALUE_INFOS[id].unit}/></div></div>
                    </FormRow>)}
                </CollapsedSection>)}
            </SubPage>
        )
    }
}

export class MetersStatus extends Component<{}, {}> {
    uplot_wrapper_ref = createRef();
    meter_slot: number = 0; // FIXME: make this configurable

    render(props: {}, state: {}) {
        // Don't check util.render_allowed() here.
        // We can receive graph data points with the first web socket packet and
        // want to push them into the uplot graph immediately.
        // This only works if the wrapper component is already created.
        // Hide the form rows to fix any visual bugs instead.
        let show = API.hasFeature('meters') && !API.hasFeature("energy_manager");

        // As we don't check util.render_allowed(),
        // we have to handle rendering before the web socket connection is established.
        let value_ids = API.get_maybe(`meters/${this.meter_slot}/value_ids`);
        let values = API.get_maybe(`meters/${this.meter_slot}/values`);
        let power = 0;

        if (value_ids && values.length > 0 && values && values.length > 0) {
            let idx = value_ids.indexOf(MeterValueID.PowerActiveLSumImExDiff);

            if (idx >= 0) {
                power = values[idx];
            }
        }

        return (
            <>
                <FormRow label={__("meters.status.power_history")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4" hidden={!show}>
                    <div class="card pl-1 pb-1">
                        <UplotWrapper ref={this.uplot_wrapper_ref}
                                      id="status_meters_chart"
                                      class="status-meters-chart"
                                      sidebar_id="status"
                                      show={true}
                                      legend_time_with_seconds={false}
                                      aspect_ratio={2.5}
                                      x_height={50}
                                      x_include_date={true}
                                      y_min={0}
                                      y_max={1500} />
                    </div>
                </FormRow>
                <FormRow label={__("meters.status.current_power")} label_muted={`Meter #${this.meter_slot}`} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4" hidden={!show}>
                    <OutputFloat value={power} digits={0} scale={0} unit="W" maxFractionalDigitsOnPage={0} maxUnitLengthOnPage={1}/>
                </FormRow>
            </>
        )
    }
}

let status_ref = createRef();

render(<MetersStatus ref={status_ref} />, $('#status-meters')[0]);

render(<Meters status_ref={status_ref} />, $('#meters')[0]);

export function init() {
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('info/features', () => $('#sidebar-meters').prop('hidden', !API.hasFeature('meters')));
}

export function update_sidebar_state(module_init: any) {}
