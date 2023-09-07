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
import { h, render, createRef, Fragment, Component, ComponentChild } from "preact";
import { PageHeader } from "../../ts/components/page_header";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { FormRow } from "../../ts/components/form_row";
import { Button } from "react-bootstrap";
import { InputSelect } from "../../ts/components/input_select";
import { CollapsedSection } from "../../ts/components/collapsed_section";
import { OutputFloat } from "../../ts/components/output_float";
import { Zap, ZapOff } from "react-feather";
import uPlot from 'uplot';
import { InputText } from "../../ts/components/input_text";
import { FormSeparator } from "../../ts/components/form_separator";
import { SubPage } from "../../ts/components/sub_page";
import { MeterValueID, METER_VALUE_IDS, METER_VALUE_INFOS } from "./meter_value_id";

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
    aspect_ratio: number;
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
                {
                    show: true,
                    pxAlign: 0,
                    spanGaps: false,
                    label: __("meters.script.power"),
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
                            this.update_internal_data();
                        },
                        drawAxes: [
                            (self: uPlot) => {
                                let ctx = self.ctx;

                                ctx.save();

                                let s  = self.series[0];
                                let xd = self.data[0];
                                let [i0, i1] = s.idxs;
                                let x0 = self.valToPos(xd[i0], 'x', true) - self.axes[0].ticks.size * devicePixelRatio;
                                let y0 = self.valToPos(0, 'y', true);
                                let x1 = self.valToPos(xd[i1], 'x', true);
                                let y1 = self.valToPos(0, 'y', true);

                                const lineWidth = 2 * devicePixelRatio;
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

type ValuesByID = {[id: number]: number};

interface MetersState {
    state: {[meter_slot: number]: Readonly<API.getType['meters/0/state']>};
    values_by_id: {[meter_slot: number]: ValuesByID};
    chart_selected: "history"|"live";
}

function calculate_live_data(offset: number, samples_per_second: number, samples: number[]): UplotData {
    let data: UplotData = {timestamps: new Array(samples.length), samples: samples};
    let now = Date.now();
    let start: number;
    let step: number;

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

export class Meters extends Component<{}, MetersState> {
    live_data: {[meter_slot: number]: UplotData} = {};
    pending_live_data: {[meter_slot: number]: UplotData} = {};
    history_data: {[meter_slot: number]: UplotData} = {};
    uplot_wrapper_live_ref = createRef();
    uplot_wrapper_history_ref = createRef();
    value_ids: {[meter_slot: number]: Readonly<number[]>} = {};
    values: {[meter_slot: number]: Readonly<number[]>} = {};

    constructor() {
        super();

        this.state = {
            state: {},
            values_by_id: {},
            chart_selected: "history",
        } as any;

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

            util.addApiEventListener_unchecked(`meters/${meter_slot}/live`, () => {
                let live = API.get_maybe(`meters/${meter_slot}/live`);

                this.live_data[meter_slot] = calculate_live_data(live.offset, live.samples_per_second, live.samples);
                this.pending_live_data[meter_slot] = {timestamps: [], samples: []};

                if (this.state.chart_selected == "live") {
                    this.update_uplot();
                }
            });

            util.addApiEventListener_unchecked(`meters/${meter_slot}/live_samples`, () => {
                let live = API.get_maybe(`meters/${meter_slot}/live_samples`);
                let live_extra = calculate_live_data(0, live.samples_per_second, live.samples);

                if (!this.pending_live_data[meter_slot]) {
                    console.log('Meters: Received live_samples before live message');
                    this.pending_live_data[meter_slot] = {timestamps: [], samples: []};
                }

                this.pending_live_data[meter_slot].timestamps.push(...live_extra.timestamps);
                this.pending_live_data[meter_slot].samples.push(...live_extra.samples);

                if (this.pending_live_data[meter_slot].samples.length >= 5) {
                    this.live_data[meter_slot].timestamps = array_append(this.live_data[meter_slot].timestamps, this.pending_live_data[meter_slot].timestamps, 720);
                    this.live_data[meter_slot].samples = array_append(this.live_data[meter_slot].samples, this.pending_live_data[meter_slot].samples, 720);

                    this.pending_live_data[meter_slot].timestamps = [];
                    this.pending_live_data[meter_slot].samples = [];

                    if (this.state.chart_selected == "live") {
                        this.update_uplot();
                    }
                }
            });

            util.addApiEventListener_unchecked(`meters/${meter_slot}/history`, () => {
                let history = API.get_maybe(`meters/${meter_slot}/history`);

                this.history_data[meter_slot] = calculate_history_data(history.offset, history.samples);

                if (this.state.chart_selected == "history") {
                    this.update_uplot();
                }
            });

            util.addApiEventListener_unchecked(`meters/${meter_slot}/history_samples`, () => {
                let history = API.get_maybe(`meters/${meter_slot}/history_samples`);

                this.history_data[meter_slot] = calculate_history_data(0, array_append(this.history_data[meter_slot].samples, history.samples, 720));

                if (this.state.chart_selected == "history") {
                    this.update_uplot();
                }
            });
        }
    }

    update_uplot() {
        if (this.state.chart_selected == 'live') {
            if (this.uplot_wrapper_live_ref && this.uplot_wrapper_live_ref.current) {
                this.uplot_wrapper_live_ref.current.set_data(this.live_data[0/*FIXME*/]);
            }
        }
        else {
            if (this.uplot_wrapper_history_ref && this.uplot_wrapper_history_ref.current) {
                this.uplot_wrapper_history_ref.current.set_data(this.history_data[0/*FIXME*/]);
            }
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

render(<Meters />, $('#meters')[0]);

export class MetersStatus extends Component<{}, {}> {
    history_data: UplotData;
    uplot_wrapper_ref = createRef();
    meter_slot: number = 0; // FIXME: make this configurable

    constructor() {
        super();

        util.addApiEventListener_unchecked(`meters/${this.meter_slot}/history`, () => {
            let history = API.get_maybe(`meters/${this.meter_slot}/history`);

            this.history_data = calculate_history_data(history.offset, history.samples);

            this.update_uplot();
        });

        util.addApiEventListener_unchecked(`meters/${this.meter_slot}/history_samples`, () => {
            let history = API.get_maybe(`meters/${this.meter_slot}/history_samples`);

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
                <FormRow label={__("meters.status.power_history")} label_muted={`Meter #${this.meter_slot}`} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4" hidden={!show}>
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

render(<MetersStatus />, $('#status-meters')[0]);

export function init() {
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('info/features', () => $('#sidebar-meters').prop('hidden', !API.hasFeature('meters')));
}

export function update_sidebar_state(module_init: any) {}
