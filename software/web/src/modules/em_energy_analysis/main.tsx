/* esp32-firmware
 * Copyright (C) 2022 Olaf Lüke <olaf@tinkerforge.com>
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
import { METERS_SLOTS } from "../../build";
import { h, render, createRef, Fragment, Component, ComponentChild, RefObject, ComponentChildren } from "preact";
import { __ } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";
import { InputDate } from "../../ts/components/input_date";
import { InputMonth } from "../../ts/components/input_month";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { OutputFloat } from "../../ts/components/output_float";
import { SubPage } from "../../ts/components/sub_page";
import { FormSeparator } from "../../ts/components/form_separator";
import uPlot from "uplot";
import { uPlotTimelinePlugin } from "../../ts/uplot-plugins";
import { MeterValueID } from "../meters/meter_value_id";
import { MeterConfig } from "../meters/types";

const UPDATE_RETRY_DELAY = 500; // ms

interface CachedData {
    update_timestamp: number;
    use_timestamp: number;
}

interface UplotData extends CachedData {
    keys: string[];
    names: string[];
    values: number[][];
    extras?: number[][];
    stacked: boolean[];
    bars: boolean[];
    value_names?: {[id: number]: string}[];
    value_strokes?: {[id: number]: string}[];
    value_fills?: {[id: number]: string}[];
    extra_names?: {[id: number]: string}[];
    default_visibilty?: boolean[];
}

interface Wallbox5minData extends CachedData {
    empty: boolean;
    complete: boolean;
    flags: number[/*timestamp_slot*/]; // bit 0-2 = charger state, bit 7 = no data
    power: number[/*timestamp_slot*/];
}

interface WallboxDailyData extends CachedData {
    empty: boolean;
    complete: boolean;
    energy: number[/*timestamp_slot*/]; // kWh
}

interface EnergyManager5minData extends CachedData {
    empty: boolean;
    complete: boolean;
    flags: number[/*timestamp_slot*/]; // bit 0 = 1p/3p, bit 1-2 = input, bit 3 = output, bit 7 = no data
    power: number[/*meter_slot*/][/*timestamp_slot*/]; // W
    power_empty: boolean[/*meter_slot*/];
}

interface EnergyManagerDailyData extends CachedData {
    empty: boolean;
    complete: boolean;
    energy_import: number[/*meter_slot*/][/*timestamp_slot*/]; // kWh
    energy_export: number[/*meter_slot*/][/*timestamp_slot*/]; // kWh
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

const wb_state_names: {[id: number]: string} = {
    0: __("em_energy_analysis.content.state_not_connected"),
    1: __("em_energy_analysis.content.state_waiting_for_charge_release"),
    2: __("em_energy_analysis.content.state_ready_to_charge"),
    3: __("em_energy_analysis.content.state_charging"),
    4: __("em_energy_analysis.content.state_error"),
};

const wb_state_strokes: {[id: number]: string} = {
    0: 'rgb(  0, 123, 255)',
    1: 'rgb(255, 193,   7)',
    2: 'rgb( 13, 202, 240)',
    3: 'rgb( 40, 167,  69)',
    4: 'rgb(220,  53,  69)',
};

const wb_state_fills: {[id: number]: string} = {
    0: 'rgb(  0, 123, 255, 0.66)',
    1: 'rgb(255, 193,   7, 0.66)',
    2: 'rgb( 13, 202, 240, 0.66)',
    3: 'rgb( 40, 167,  69, 0.66)',
    4: 'rgb(220,  53,  69, 0.66)',
};

const em_phase_names: {[id: number]: string} = {
    0: __("em_energy_analysis.content.state_single_phase"),
    1: __("em_energy_analysis.content.state_three_phase"),
};

const em_phase_strokes: {[id: number]: string} = {
    0: 'rgb(108, 117, 125)',
    1: 'rgb( 40, 167,  69)',
};

const em_phase_fills: {[id: number]: string} = {
    0: 'rgb(108, 117, 125, 0.66)',
    1: 'rgb( 40, 167,  69, 0.66)',
};

const em_input_names: {[id: number]: string} = {
    0: __("em_energy_analysis.content.state_input_low"),
    1: __("em_energy_analysis.content.state_input_high"),
};

const em_input_strokes: {[id: number]: string} = {
    0: 'rgb(108, 117, 125)',
    1: 'rgb( 40, 167,  69)',
};

const em_input_fills: {[id: number]: string} = {
    0: 'rgb(108, 117, 125, 0.66)',
    1: 'rgb( 40, 167,  69, 0.66)',
};

const em_relay_names: {[id: number]: string} = {
    0: __("em_energy_analysis.content.state_relay_open"),
    1: __("em_energy_analysis.content.state_relay_closed"),
};

const em_relay_strokes: {[id: number]: string} = {
    0: 'rgb(108, 117, 125)',
    1: 'rgb( 40, 167,  69)',
};

const em_relay_fills: {[id: number]: string} = {
    0: 'rgb(108, 117, 125, 0.66)',
    1: 'rgb( 40, 167,  69, 0.66)',
};

interface UplotLoaderProps {
    show: boolean;
    marker_class: 'h3'|'h4';
    children: ComponentChildren;
}

class UplotLoader extends Component<UplotLoaderProps, {}> {
    no_data_ref = createRef();
    loading_ref = createRef();

    set_loading() {
        this.no_data_ref.current.style.visibility = 'hidden';
        this.loading_ref.current.style.visibility = 'inherit';
    }

    set_show(show: boolean) {
        this.no_data_ref.current.style.display = show ? 'flex' : 'none';
        this.loading_ref.current.style.display = show ? 'flex' : 'none';
    }

    set_data(data: UplotData, visible?: boolean) {
        this.loading_ref.current.style.visibility = 'hidden';

        if (visible === false || (visible === undefined && (!data || data.keys.length <= 1))) {
            this.no_data_ref.current.style.visibility = 'inherit';
        }
        else {
            this.no_data_ref.current.style.visibility = 'hidden';
        }
    }

    render(props?: UplotLoaderProps, state?: Readonly<{}>, context?: any): ComponentChild {
        return (
            <>
                <div ref={this.no_data_ref} style={`position: absolute; width: 100%; height: 100%; visibility: hidden; display: ${props.show ? 'flex' : 'none'};`}>
                    <span class={props.marker_class} style="margin: auto;">{__("em_energy_analysis.content.no_data")}</span>
                </div>
                <div ref={this.loading_ref} style={`position: absolute; width: 100%; height: 100%; visibility: ${props.show ? 'inherit' : 'hidden'}; display: ${props.show ? 'flex' : 'none'};`}>
                    <span class={props.marker_class} style="margin: auto;">{__("em_energy_analysis.content.loading")}</span>
                </div>
                {props.children}
            </>
        );
    }
}

interface UplotFlagsWrapperProps {
    id: string;
    class: string;
    sidebar_id: string;
    show: boolean;
    sync?: uPlot.SyncPubSub;
    legend_time_label: string;
    legend_time_with_minutes: boolean;
    legend_value_prefix: string;
    legend_div_ref: RefObject<HTMLDivElement>;
    x_format: Intl.DateTimeFormatOptions;
    x_padding_factor: number;
    y_sync_ref?: RefObject<UplotWrapper>;
}

class UplotFlagsWrapper extends Component<UplotFlagsWrapperProps, {}> {
    uplot: uPlot;
    data: UplotData;
    pending_data: UplotData;
    series_visibility: {[id: string]: boolean} = {};
    visible: boolean = false;
    div_ref = createRef();
    observer: ResizeObserver;
    bar_height: number = 20;
    bar_spacing: number = 5;
    y_size: number = 0;
    y_other_size: number = 0;

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

        let options = {
            ...this.get_size(),
            pxAlign: 0,
            cursor: {
                drag: {
                    x: false, // disable zoom
                },
                sync: {
                    key: this.props.sync?.key,
                },
            },
            series: [
                {
                    label: this.props.legend_time_label,
                    value: (self: uPlot, rawValue: number) => {
                        if (rawValue !== null) {
                            if (this.props.legend_time_with_minutes) {
                                return util.timestamp_min_to_date(rawValue / 60);
                            }
                            else {
                                return new Date(rawValue * 1000).toLocaleDateString([], {year: 'numeric', month: '2-digit', day: '2-digit'});
                            }
                        }

                        return null;
                    },
                },
            ],
            axes: [
                {
                    size: 1,// with size = 0 the width of the whole plot changes relative to the power plot
                    ticks: {
                        size: 0
                    },
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
                        3600 * 48,
                    ],
                    values: (self: uPlot, splits: number[]) => {
                        let values: string[] = new Array(splits.length);

                        for (let i = 0; i < splits.length; ++i) {
                            values[i] = (new Date(splits[i] * 1000)).toLocaleString([], this.props.x_format);
                        }

                        return values;
                    },
                    side: 0,
                },
                {
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

                        this.y_size = Math.ceil(size / devicePixelRatio) + 20;
                        size = Math.max(this.y_size, this.y_other_size);

                        if (this.props.y_sync_ref && this.props.y_sync_ref.current) {
                            this.props.y_sync_ref.current.set_y_other_size(this.y_size);
                        }

                        return size;
                    },
                },
            ],
            scales: {
                x: {
                    range: (self: uPlot, initMin: number, initMax: number, scaleKey: string): uPlot.Range.MinMax => {
                        let pad = (initMax - initMin) * this.props.x_padding_factor;
                        return [initMin - pad, initMax + pad];
                    },
                },
            },
            padding: [null, 5, 0, null] as uPlot.Padding,
            legend: {
                mount: (self: uPlot, legend: HTMLElement) => {
                    if (this.props.legend_div_ref.current) {
                        this.props.legend_div_ref.current.appendChild(legend);
                    }
                },
            },
            plugins: [
                uPlotTimelinePlugin({
                    fill: (seriesIdx: number, dataIdx: number, value: any) => this.data.value_fills && this.data.value_fills[seriesIdx] ? this.data.value_fills[seriesIdx][value] : 'rgb(0, 0, 0, 0.1)',
                    stroke: (seriesIdx: number, dataIdx: number, value: any) => this.data.value_strokes && this.data.value_strokes[seriesIdx] ? this.data.value_strokes[seriesIdx][value] : 'rgb(0, 0, 0)',
                    bar_height: this.bar_height,
                    bar_spacing: this.bar_spacing,
                }),
                {
                    hooks: {
                        setSeries: (self: uPlot, seriesIdx: number, opts: uPlot.Series) => {
                            this.series_visibility[this.data.keys[seriesIdx]] = opts.show;
                            this.resize();
                        },
                        addSeries: (self: uPlot, seriesIdx: number) => {
                            if (this.data && this.data.keys[seriesIdx].startsWith('wb_state_')) {
                                let series = document.querySelectorAll('.u-time-in-legend-alone .u-legend .u-series');
                                let element = series[seriesIdx] as HTMLElement;

                                if (element) {
                                    element.style.display = 'none';
                                }
                            }

                            this.resize();
                        },
                        delSeries: (self: uPlot, seriesIdx: number) => {
                            this.resize();
                        },
                    },
                },
            ],
        };

        let div = this.div_ref.current;
        this.uplot = new uPlot(options, [], div);

        try {
            this.observer = new ResizeObserver(() => {
                this.resize();
            });

            this.observer.observe(div);
        } catch (e) {
            setInterval(() => {
                this.resize();
            }, 500);

            window.addEventListener("resize", e => {
                this.resize();
            });
        }

        if (this.pending_data !== undefined) {
            this.set_data(this.pending_data);
        }
    }

    render(props?: UplotFlagsWrapperProps, state?: Readonly<{}>, context?: any): ComponentChild {
        return <div ref={this.div_ref} class={props.class} style={`display: ${props.show ? 'block' : 'none'}; visibility: hidden;`} />;
    }

    resize() {
        let size = this.get_size();

        if (size.width == 0 || size.height == 0) {
            return;
        }

        if (this.uplot) {
            this.uplot.setSize(size);
        }
        else {
            window.setTimeout(() => {
                this.resize();
            }, 100);
        }
    }

    get_size() {
        let div = this.div_ref.current;
        let count = 0;

        if (this.uplot) {
            for (let i = 1; i < this.uplot.series.length; ++i) {
                if (this.series_visibility[this.data.keys[i]]) {
                    ++count;
                }
            }
        }

        return {
            width: div.clientWidth,
            height: 1 + count * this.bar_height + Math.max(count - 1, 0) * this.bar_spacing + 17,
        }
    }

    set_y_other_size(size: number) {
        if (this.y_other_size == size) {
            return;
        }

        this.y_other_size = size;

        if (this.y_other_size != this.y_size) {
            this.resize();
        }
    }

    set_loading() {
        this.div_ref.current.style.visibility = 'hidden';

        if (this.props.legend_div_ref.current) {
            this.props.legend_div_ref.current.style.visibility = 'hidden';
        }
    }

    set_show(show: boolean) {
        this.div_ref.current.style.display = show ? 'block' : 'none';

        if (this.props.legend_div_ref.current) {
            this.props.legend_div_ref.current.style.display = show ? 'block' : 'none';
        }
    }

    get_series_opts(i: number): uPlot.Series {
        let name = this.data.names[i];

        return {
            show: this.series_visibility[this.data.keys[i]],
            label: (this.props.legend_value_prefix + ' ' + name).trim(),
            value: (self: uPlot, rawValue: number, seriesIdx: number, idx: number | null) => {
                if (rawValue !== null && this.data.value_names && this.data.value_names[seriesIdx]) {
                    return this.data.value_names[seriesIdx][this.data.values[seriesIdx][idx]];
                }

                return rawValue;
            },
            width: 0,
        };
    }

    set_data(data: UplotData, visible?: boolean) {
        if (!this.uplot || !this.visible) {
            this.pending_data = data;
            return;
        }

        this.data = data;
        this.pending_data = undefined;

        if (visible === false || (visible === undefined && (!this.data || this.data.keys.length <= 1))) {
            this.div_ref.current.style.visibility = 'hidden';

            if (this.props.legend_div_ref.current) {
                this.props.legend_div_ref.current.style.visibility = 'hidden';
            }
        }
        else {
            this.div_ref.current.style.visibility = 'inherit';

            if (this.props.legend_div_ref.current) {
                this.props.legend_div_ref.current.style.visibility = 'inherit';
            }

            while (this.uplot.series.length > 1) {
                this.uplot.delSeries(this.uplot.series.length - 1);
            }

            while (this.uplot.series.length < this.data.keys.length) {
                if (this.series_visibility[this.data.keys[this.uplot.series.length]] === undefined) {
                    let visibilty = true;

                    if (this.data.default_visibilty) {
                        visibilty = this.data.default_visibilty[this.uplot.series.length];
                    }

                    this.series_visibility[this.data.keys[this.uplot.series.length]] = visibilty;
                }

                this.uplot.addSeries(this.get_series_opts(this.uplot.series.length));
            }

            this.uplot.setData(this.data.values as any);
        }
    }
}

interface UplotWrapperProps {
    id: string;
    class: string;
    sidebar_id: string;
    color_cache_group: string;
    show: boolean;
    sync?: uPlot.SyncPubSub;
    legend_time_label: string;
    legend_time_with_minutes: boolean;
    legend_value_prefix: string;
    legend_div_ref?: RefObject<HTMLDivElement>;
    aspect_ratio: number;
    x_format: Intl.DateTimeFormatOptions;
    x_padding_factor: number;
    y_min?: number;
    y_max?: number;
    y_unit: string;
    y_label: string;
    y_digits: number;
    y_skip_upper?: boolean;
    y_sync_ref?: RefObject<UplotFlagsWrapper>;
    default_fill?: boolean;
    padding?: uPlot.Padding;
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
    y_size: number = 0;
    y_other_size: number = 0;
    y_label_size: number = 20;

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

        let padding: uPlot.Padding = this.props.padding;

        if (!padding) {
            padding = [null, null, null, null] as uPlot.Padding;
        }

        let options = {
            ...this.get_size(),
            pxAlign: 0,
            cursor: {
                drag: {
                    x: false, // disable zoom
                },
                sync: {
                    key: this.props.sync?.key,
                },
            },
            series: [
                {
                    label: this.props.legend_time_label,
                    value: (self: uPlot, rawValue: number) => {
                        if (rawValue !== null) {
                            if (this.props.legend_time_with_minutes) {
                                return util.timestamp_min_to_date(rawValue / 60);
                            }
                            else {
                                return new Date(rawValue * 1000).toLocaleDateString([], {year: 'numeric', month: '2-digit', day: '2-digit'});
                            }
                        }

                        return null;
                    },
                },
            ],
            axes: [
                {
                    size: 30,
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
                        3600 * 48,
                        3600 * 72,
                        3600 * 168,
                    ],
                    values: (self: uPlot, splits: number[]) => {
                        let values: string[] = new Array(splits.length);

                        for (let i = 0; i < splits.length; ++i) {
                            values[i] = (new Date(splits[i] * 1000)).toLocaleString([], this.props.x_format);
                        }

                        return values;
                    },
                },
                {
                    label: this.props.y_label,
                    labelSize: this.y_label_size,
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

                        this.y_size = Math.ceil(size / devicePixelRatio) + 20;
                        size = Math.max(this.y_size + this.y_label_size, this.y_other_size) - this.y_label_size;

                        if (this.props.y_sync_ref && this.props.y_sync_ref.current) {
                            this.props.y_sync_ref.current.set_y_other_size(this.y_size + this.y_label_size);
                        }

                        return size;
                    },
                    values: (self: uPlot, splits: number[]) => {
                        let values: string[] = new Array(splits.length);

                        for (let digits = 0; digits <= 3; ++digits) {
                            let last_value: string = null;
                            let unique = true;

                            for (let i = 0; i < splits.length; ++i) {
                                if (this.props.y_skip_upper && splits[i] >= this.y_max) {
                                    values[i] = '';
                                }
                                else {
                                    values[i] = util.toLocaleFixed(splits[i], digits);
                                }

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
                },
            ],
            scales: {
                x: {
                    range: (self: uPlot, initMin: number, initMax: number, scaleKey: string): uPlot.Range.MinMax => {
                        let pad = (initMax - initMin) * this.props.x_padding_factor;
                        return [initMin - pad, initMax + pad];
                    },
                },
                y: {
                    range: (self: uPlot, initMin: number, initMax: number, scaleKey: string): uPlot.Range.MinMax => {
                        return uPlot.rangeNum(this.y_min, this.y_max, {min: {}, max: {}});
                    }
                },
            },
            legend: {
                mount: (self: uPlot, legend: HTMLElement) => {
                    if (this.props.legend_div_ref && this.props.legend_div_ref.current) {
                        this.props.legend_div_ref.current.appendChild(legend);
                    }
                },
            },
            padding: padding,
            plugins: [
                {
                    hooks: {
                        setSeries: (self: uPlot, seriesIdx: number, opts: uPlot.Series) => {
                            this.series_visibility[this.data.keys[seriesIdx]] = opts.show;
                            this.update_internal_data();

                            if (this.props.y_sync_ref && this.props.y_sync_ref.current && this.data.keys[seriesIdx].startsWith('wb_power_')) {
                                let key = this.data.keys[seriesIdx].replace('_power_', '_state_');

                                for (let i = 1; i < this.props.y_sync_ref.current.data.keys.length; ++i) {
                                    if (this.props.y_sync_ref.current.data.keys[i] == key) {
                                        this.props.y_sync_ref.current.uplot.setSeries(i, {show: opts.show});
                                        break;
                                    }
                                }
                            }
                        },
                        drawAxes: [
                            (self: uPlot) => {
                                let ctx = self.ctx;
                                let s  = self.series[0];
                                let xd = self.data[0];
                                let [i0, i1] = s.idxs;
                                let xpad = (xd[i1] - xd[i0]) * this.props.x_padding_factor;
                                let x0 = self.valToPos(xd[i0] - xpad, 'x', true) - self.axes[0].ticks.size * devicePixelRatio;
                                let x1 = self.valToPos(xd[i1] + xpad, 'x', true);
                                let y = self.valToPos(0, 'y', true);

                                if (y > ctx.canvas.height - (self.axes[0].size as number)) {
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
                            },
                        ],
                    },
                },
            ],
        };

        let div = this.div_ref.current;
        this.uplot = new uPlot(options, [], div);

        try {
            this.observer = new ResizeObserver(() => {
                this.resize();
            });

            this.observer.observe(div);
        } catch (e) {
            setInterval(() => {
                this.resize();
            }, 500);

            window.addEventListener("resize", e => {
                this.resize();
            });
        }

        if (this.pending_data !== undefined) {
            this.set_data(this.pending_data);
        }
    }

    render(props?: UplotWrapperProps, state?: Readonly<{}>, context?: any): ComponentChild {
        return <div ref={this.div_ref} class={props.class} style={`display: ${props.show ? 'block' : 'none'}; visibility: hidden;`} />;
    }

    resize() {
        let size = this.get_size();

        if (size.width == 0 || size.height == 0) {
            return;
        }

        if (this.uplot) {
            this.uplot.setSize(size);
        }
        else {
            window.setTimeout(() => {
                this.resize();
            }, 100);
        }
    }

    get_size() {
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

    set_y_other_size(size: number) {
        if (this.y_other_size == size) {
            return;
        }

        this.y_other_size = size;

        if (this.y_other_size != this.y_size) {
            this.resize();
        }
    }

    set_loading() {
        this.div_ref.current.style.visibility = 'hidden';

        if (this.props.legend_div_ref && this.props.legend_div_ref.current) {
            this.props.legend_div_ref.current.style.visibility = 'hidden';
        }
    }

    set_show(show: boolean) {
        this.div_ref.current.style.display = show ? 'block' : 'none';

        if (this.props.legend_div_ref && this.props.legend_div_ref.current) {
            this.props.legend_div_ref.current.style.display = show ? 'block' : 'none';
        }
    }

    get_series_opts(i: number): uPlot.Series {
        let name = this.data.names[i];
        let color = get_color(this.props.color_cache_group, name);

        return {
            show: this.series_visibility[this.data.keys[i]],
            pxAlign: 0,
            spanGaps: false,
            label: this.props.legend_value_prefix + (name ? ' ' + name: ''),
            value: (self: uPlot, rawValue: number, seriesIdx: number, idx: number | null) => {
                if (rawValue !== null) {
                    let prefix = '';

                    if (this.data.extras && this.data.extra_names && this.data.extra_names[seriesIdx]) {
                        prefix = this.data.extra_names[seriesIdx][this.data.extras[seriesIdx][idx]] + ' / ';
                    }

                    return prefix + util.toLocaleFixed(this.data.values[seriesIdx][idx], this.props.y_digits) + " " + this.props.y_unit;
                }

                return null;
            },
            stroke: color.stroke,
            fill: this.data.stacked[i] || this.props.default_fill ? color.fill : undefined,
            width: 2,
            paths: this.data.bars[i] ? uPlot.paths.bars({size: [0.4, 100], align: this.data.stacked[i] ? 1 : -1}) : undefined,
            points: {
                show: false,
            },
        };
    }

    update_internal_data() {
        let y_min: number = this.props.y_min;
        let y_max: number = this.props.y_max;
        let last_stacked_values: number[] = [];

        this.uplot.delBand(null);

        for (let i = this.data.values.length - 1; i > 0; --i) {
            if (!this.data.stacked[i]) {
                for (let k = 0; k < this.data.values[i].length; ++k) {
                    let value = this.data.values[i][k];

                    if (value !== null) {
                        if (y_min === undefined || value < y_min) {
                            y_min = value;
                        }

                        if (y_min === undefined || value > y_max) {
                            y_max = value;
                        }
                    }
                }
            }
            else {
                let stacked_values: number[] = new Array(this.data.values[i].length);

                for (let k = 0; k < this.data.values[i].length; ++k) {
                    if (last_stacked_values[k] !== null
                        && last_stacked_values[k] !== undefined
                        && this.data.values[i][k] !== null
                        && this.data.values[i][k] !== undefined) {
                        stacked_values[k] = last_stacked_values[k] + this.data.values[i][k];
                    } else {
                        stacked_values[k] = this.data.values[i][k];
                    }

                    if (stacked_values[k] !== null) {
                        if (stacked_values[k] < y_min) {
                            y_min = stacked_values[k];
                        }

                        if (stacked_values[k] > y_max) {
                            y_max = stacked_values[k];
                        }
                    }
                }

                last_stacked_values = stacked_values;
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

        this.y_min = y_min;
        this.y_max = y_max;

        let uplot_values: number[][] = [];
        last_stacked_values = [];

        for (let i = this.data.values.length - 1; i >= 0; --i) {
            if (!this.data.stacked[i] || !this.series_visibility[this.data.keys[i]]) {
                uplot_values.unshift(this.data.values[i]);
            }
            else {
                let stacked_values: number[] = new Array(this.data.values[i].length);

                for (let k = 0; k < this.data.values[i].length; ++k) {
                    if (last_stacked_values[k] !== null
                        && last_stacked_values[k] !== undefined
                        && this.data.values[i][k] !== null
                        && this.data.values[i][k] !== undefined) {
                        stacked_values[k] = last_stacked_values[k] + this.data.values[i][k];
                    } else {
                        stacked_values[k] = this.data.values[i][k];
                    }
                }

                uplot_values.unshift(stacked_values);
                last_stacked_values = stacked_values;
            }
        }

        this.uplot.setData(uplot_values as any);

        let last_stacked_index: number = null;

        for (let i = this.data.values.length - 1; i > 0; --i) {
            if (this.data.stacked[i] && this.series_visibility[this.data.keys[i]]) {
                if (last_stacked_index === null) {
                    this.uplot.delSeries(i);
                    this.uplot.addSeries(this.get_series_opts(i), i);
                } else {
                    this.uplot.addBand({
                        series: [i, last_stacked_index],
                        fill: get_color(this.props.color_cache_group, this.data.names[i]).fill,
                    });
                }

                last_stacked_index = i;
            }
        }
    }

    set_data(data: UplotData, visible?: boolean) {
        if (!this.uplot || !this.visible) {
            this.pending_data = data;
            return;
        }

        this.data = data;
        this.pending_data = undefined;

        if (visible === false || (visible === undefined && (!this.data || this.data.keys.length <= 1))) {
            this.div_ref.current.style.visibility = 'hidden';

            if (this.props.legend_div_ref && this.props.legend_div_ref.current) {
                this.props.legend_div_ref.current.style.visibility = 'hidden';
            }
        }
        else {
            this.div_ref.current.style.visibility = 'inherit';

            if (this.props.legend_div_ref && this.props.legend_div_ref.current) {
                this.props.legend_div_ref.current.style.visibility = 'inherit';
            }

            while (this.uplot.series.length > 1) {
                this.uplot.delSeries(this.uplot.series.length - 1);
            }

            while (this.uplot.series.length < this.data.keys.length) {
                if (this.series_visibility[this.data.keys[this.uplot.series.length]] === undefined) {
                    let visibilty = true;

                    if (this.data.default_visibilty) {
                        visibilty = this.data.default_visibilty[this.uplot.series.length];
                    }

                    this.series_visibility[this.data.keys[this.uplot.series.length]] = visibilty;
                }

                this.uplot.addSeries(this.get_series_opts(this.uplot.series.length));
            }

            this.update_internal_data();
        }
    }
}

interface EMEnergyAnalysisStatusState {
    force_render: number,
    meter_slot: number,
    meter_configs: {[meter_slot: number]: MeterConfig},
}

function get_meter_name(meter_configs: {[meter_slot: number]: MeterConfig}, meter_slot: number) {
    let meter_name = __("em_energy_analysis.script.meter")(util.hasValue(meter_slot) ? meter_slot : '?');

    if (util.hasValue(meter_slot) && util.hasValue(meter_configs) && util.hasValue(meter_configs[meter_slot]) && util.hasValue(meter_configs[meter_slot][1])) {
        meter_name = meter_configs[meter_slot][1].display_name;
    }

    return meter_name;
}

export class EMEnergyAnalysisStatus extends Component<{}, EMEnergyAnalysisStatusState> {
    uplot_loader_ref = createRef();
    uplot_wrapper_ref = createRef();

    constructor() {
        super();

        this.state = {
            force_render: 0,
            meter_slot: 0,
        } as any;

        util.addApiEventListener('info/modules', () => {
            if (!API.hasFeature('energy_manager')) {
                console.log("Energy Analysis: energy_manager feature not available");
                return;
            }

            this.setState({force_render: Date.now()});
        });

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            util.addApiEventListener_unchecked(`meters/${meter_slot}/config`, () => {
                let config = API.get_unchecked(`meters/${meter_slot}/config`);

                this.setState((prevState) => ({
                    meter_configs: {
                        ...prevState.meter_configs,
                        [meter_slot]: config
                    }
                }));
            });
        }

        util.addApiEventListener("power_manager/config", () => {
            let config = API.get("power_manager/config");

            this.setState({meter_slot: config.meter_slot_grid_power});
        });
    }

    render(props: {}, state: EMEnergyAnalysisStatusState) {
        // Don't check util.render_allowed() here.
        // We can receive graph data points with the first web socket packet and
        // want to push them into the uplot graph immediately.
        // This only works if the wrapper component is already created.
        // Hide the form rows to fix any visual bugs instead.
        let show = util.render_allowed() && API.hasFeature("energy_manager");

        // As we don't check util.render_allowed(),
        // we have to handle rendering before the web socket connection is established.
        let value_ids = API.get_unchecked(`meters/${state.meter_slot}/value_ids`);
        let values = API.get_unchecked(`meters/${state.meter_slot}/values`);
        let power = 0;

        if (value_ids && value_ids.length > 0 && values && values.length > 0) {
            let idx = value_ids.indexOf(MeterValueID.PowerActiveLSumImExDiff);

            if (idx >= 0) {
                power = values[idx];
            }
        }

        return (
            <>
                <FormRow label={__("em_energy_analysis_status.status.power_history")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4" hidden={!show}>
                    <div class="card pl-1 pb-1">
                        <div style="position: relative;"> {/* this plain div is neccessary to make the size calculation stable in safari. without this div the height continues to grow */}
                            <UplotLoader ref={this.uplot_loader_ref}
                                         show={true}
                                         marker_class={'h4'} >
                                <UplotWrapper ref={this.uplot_wrapper_ref}
                                              id="em_energy_analysis_status_chart"
                                              class="em-energy-analysis-status-chart"
                                              sidebar_id="status"
                                              color_cache_group="status"
                                              show={true}
                                              legend_time_label={__("em_energy_analysis.script.time_5min")}
                                              legend_time_with_minutes={true}
                                              legend_value_prefix={""}
                                              aspect_ratio={2}
                                              x_format={{hour: '2-digit', minute: '2-digit'}}
                                              x_padding_factor={0}
                                              y_min={0}
                                              y_max={1500}
                                              y_unit={"W"}
                                              y_label={__("em_energy_analysis.script.power") + " [Watt]"}
                                              y_digits={0}
                                              default_fill={true}
                                              padding={[null, 15, null, 5] as uPlot.Padding} />
                            </UplotLoader>
                        </div>
                    </div>
                </FormRow>
                <FormRow label={__("em_energy_analysis_status.status.current_power")} label_muted={get_meter_name(state.meter_configs, state.meter_slot)} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4" hidden={!show}>
                    <OutputFloat value={power} digits={0} scale={0} unit="W" maxFractionalDigitsOnPage={0} maxUnitLengthOnPage={1}/>
                </FormRow>
            </>
        )
    }
}

interface EMEnergyAnalysisProps {
    status_ref: RefObject<EMEnergyAnalysisStatus>;
}

interface EMEnergyAnalysisState {
    force_render: number,
    meter_slot_status: number,
    meter_configs: {[meter_slot: number]: MeterConfig};
    data_type: '5min'|'daily';
    current_5min_date: Date;
    current_daily_date: Date;
    wallbox_5min_cache_energy_total: {[id: number]: {[id: string]: number[/*timestamp_slot*/]}};
    wallbox_daily_cache_energy_total: {[id: number]: {[id: string]: number}};
    energy_manager_5min_cache_energy_total: {[id: string]: {import: number[/*meter_slot*/][/*timestamp_slot*/], export: number[/*meter_slot*/][/*timestamp_slot*/]}};
    energy_manager_daily_cache_energy_total: {[id: string]: {import: number[/*meter_slot*/], export: number[/*meter_slot*/]}};
}

interface Charger {
    uid: number;
    name: string;
}

function array_last<T>(a: Array<T>): T {
    return a[a.length - 1];
}

export class EMEnergyAnalysis extends Component<EMEnergyAnalysisProps, EMEnergyAnalysisState> {
    uplot_sync = uPlot.sync('foobar');
    uplot_loader_5min_ref = createRef();
    uplot_wrapper_5min_flags_ref = createRef();
    uplot_wrapper_5min_power_ref = createRef();
    uplot_legend_div_5min_power_ref = createRef();
    uplot_legend_div_5min_flags_ref = createRef();
    uplot_loader_daily_ref = createRef();
    uplot_wrapper_daily_ref = createRef();
    uplot_update_timeout: number = null;
    uplot_5min_flags_cache: {[id: string]: UplotData} = {};
    uplot_5min_power_cache: {[id: string]: UplotData} = {};
    uplot_5min_status_cache: {[id: string]: UplotData} = {};
    uplot_daily_cache: {[id: string]: UplotData} = {};
    wallbox_5min_cache: {[id: number]: { [id: string]: Wallbox5minData}} = {};
    wallbox_daily_cache: {[id: string]: { [id: string]: WallboxDailyData}} = {};
    energy_manager_5min_cache: {[id: string]: EnergyManager5minData} = {};
    energy_manager_daily_cache: {[id: string]: EnergyManagerDailyData} = {};
    cache_limit = 100;
    chargers: Charger[] = [];

    constructor() {
        super();

        let current_5min_date: Date = new Date();

        current_5min_date.setHours(0);
        current_5min_date.setMinutes(0);
        current_5min_date.setSeconds(0);
        current_5min_date.setMilliseconds(0);

        let current_daily_date: Date = new Date();

        current_daily_date.setDate(1);
        current_daily_date.setHours(0);
        current_daily_date.setMinutes(0);
        current_daily_date.setSeconds(0);
        current_daily_date.setMilliseconds(0);

        this.state = {
            force_render: 0,
            meter_slot_status: 0,
            data_type: '5min',
            current_5min_date: current_5min_date,
            current_daily_date: current_daily_date,
            wallbox_5min_cache_energy_total: {},
            wallbox_daily_cache_energy_total: {},
            energy_manager_5min_cache_energy_total: {},
            energy_manager_daily_cache_energy_total: {}
        } as any;

        util.addApiEventListener('info/modules', () => {
            if (!API.hasFeature('energy_manager')) {
                console.log("Energy Analysis: energy_manager feature not available");
                return;
            }

            this.update_current_5min_cache();
            this.update_current_daily_cache();

            this.setState({force_render: Date.now()});
        });

        util.addApiEventListener('charge_manager/state', () => {
            if (!API.hasFeature('energy_manager'))
                return;

            let state = API.get('charge_manager/state');
            let chargers: Charger[] = [];

            for (let charger of state.chargers) {
                if (charger.uid > 0) {
                    chargers.push({uid: charger.uid, name: charger.name});
                }
            }

            if (this.chargers.length != chargers.length) {
                this.chargers = chargers;
                this.reload_wallbox_cache();
            }
            else {
                for (let i = 0; i < this.chargers.length; ++i) {
                    if (this.chargers[i].uid != chargers[i].uid || this.chargers[i].name != chargers[i].name) {
                        this.chargers = chargers;
                        this.reload_wallbox_cache();
                        break;
                    }
                }
            }
        });

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            util.addApiEventListener_unchecked(`meters/${meter_slot}/config`, () => {
                let config = API.get_unchecked(`meters/${meter_slot}/config`);

                this.setState((prevState) => ({
                    meter_configs: {
                        ...prevState.meter_configs,
                        [meter_slot]: config
                    }
                }));
            });
        }

        util.addApiEventListener('energy_manager/history_wallbox_5min_changed', () => {
            let changed = API.get('energy_manager/history_wallbox_5min_changed');
            let subcache = this.wallbox_5min_cache[changed.uid];
            let reload_subcache: boolean = false;

            if (!subcache) {
                // got changed event without having this UID cached before
                reload_subcache = true;
            } else {
                let key = `${changed.year}-${changed.month}-${changed.day}`;
                let data = subcache[key];

                if (!data) {
                    // got changed event without having this day cached before
                    reload_subcache = true;
                }
                else {
                    let timestamp_slot = Math.floor((changed.hour * 60 + changed.minute) / 5);

                    if (timestamp_slot > 0 && data.flags[timestamp_slot - 1] === null) {
                        // previous slot has no data. was a previous update event missed?
                        delete subcache[key];
                        reload_subcache = true;
                    }
                    else {
                        data.update_timestamp = Date.now();
                        data.empty = false;
                        data.complete = timestamp_slot == 287; // update for last 5min slot of the day
                        data.flags[timestamp_slot] = changed.flags;
                        data.power[timestamp_slot] = changed.power;

                        this.schedule_uplot_update();
                    }
                }
            }

            if (reload_subcache) {
                this.update_wallbox_5min_cache(changed.uid, new Date(changed.year, changed.month - 1, changed.day))
                    .then((success: boolean) => {
                        if (success) {
                            this.schedule_uplot_update();
                        }
                    });
            }
        });

        util.addApiEventListener('energy_manager/history_energy_manager_5min_changed', () => {
            let changed = API.get('energy_manager/history_energy_manager_5min_changed');
            let key = `${changed.year}-${changed.month}-${changed.day}`;
            let data = this.energy_manager_5min_cache[key];
            let reload_cache: boolean = false;

            if (!data) {
                // got changed event without having this day cached before
                reload_cache = true;
            } else {
                let timestamp_slot = Math.floor((changed.hour * 60 + changed.minute) / 5);

                if (timestamp_slot > 0 && data.flags[timestamp_slot - 1] === null) {
                    // previous slot has no data. was a previous update event missed?
                    delete this.energy_manager_5min_cache[key];
                    reload_cache = true;
                }
                else {
                    data.update_timestamp = Date.now();
                    data.empty = false;
                    data.complete = timestamp_slot == 287; // update for last 5min slot of the day
                    data.flags[timestamp_slot] = changed.flags;

                    for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                        data.power[meter_slot][timestamp_slot] = changed.power[meter_slot];

                        if (data.power[meter_slot][timestamp_slot] !== null) {
                            data.power_empty[meter_slot] = false;
                        }
                    }

                    this.schedule_uplot_update();
                }
            }

            if (reload_cache) {
                this.update_energy_manager_5min_cache(new Date(changed.year, changed.month - 1, changed.day))
                    .then((success: boolean) => {
                        if (success) {
                            this.schedule_uplot_update();
                        }
                    });
            }
        });

        util.addApiEventListener('energy_manager/history_wallbox_daily_changed', () => {
            let changed = API.get('energy_manager/history_wallbox_daily_changed');
            let subcache = this.wallbox_daily_cache[changed.uid];
            let reload_subcache: boolean = false;

            if (!subcache) {
                // got changed event without having this UID cached before
                reload_subcache = true;
            } else {
                let key = `${changed.year}-${changed.month}`;
                let data = subcache[key];

                if (!data) {
                    // got changed event without having this month cached before
                    reload_subcache = true;
                }
                else {
                    let timestamp_slot = changed.day - 1;

                    if (timestamp_slot > 0 && data.energy[timestamp_slot - 1] == null) {
                        // previous slot has no data. was a previous update event missed?
                        delete subcache[key];
                        reload_subcache = true;
                    }
                    else {
                        data.update_timestamp = Date.now();
                        data.empty = false;
                        // FIXME: how to set complete = true here?
                        data.energy[timestamp_slot] = changed.energy;
                    }

                    this.schedule_uplot_update();
                }
            }

            if (reload_subcache) {
                this.update_wallbox_daily_cache(changed.uid, new Date(changed.year, changed.month - 1))
                    .then((success: boolean) => {
                        if (success) {
                            this.schedule_uplot_update();
                        }
                    });
            }
        });

        util.addApiEventListener('energy_manager/history_energy_manager_daily_changed', () => {
            let changed = API.get('energy_manager/history_energy_manager_daily_changed');
            let key = `${changed.year}-${changed.month}`;
            let data = this.energy_manager_daily_cache[key];
            let reload_cache: boolean = false;

            if (!data) {
                // got changed event without having this day cached before
                reload_cache = true;
            } else {
                let timestamp_slot = changed.day - 1;

                data.update_timestamp = Date.now();
                data.empty = false;
                // FIXME: how to set complete = true here?

                for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                    data.energy_import[meter_slot][timestamp_slot] = changed.energy_import[meter_slot];
                    data.energy_export[meter_slot][timestamp_slot] = changed.energy_export[meter_slot];
                }

                this.schedule_uplot_update();
            }

            if (reload_cache) {
                this.update_energy_manager_daily_cache(new Date(changed.year, changed.month - 1))
                    .then((success: boolean) => {
                        if (success) {
                            this.schedule_uplot_update();
                        }
                    });
            }
        });

        util.addApiEventListener("power_manager/config", () => {
            let config = API.get("power_manager/config");

            this.setState({meter_slot_status: config.meter_slot_grid_power});
        });
    }

    date_to_5min_key(date: Date) {
        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;
        let day: number = date.getDate();

        return `${year}-${month}-${day}`;
    }

    date_to_daily_key(date: Date) {
        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;

        return`${year}-${month}`;
    }

    expire_cache(cache: {[id: string]: CachedData}) {
        let keys = Object.keys(cache);

        while (keys.length > this.cache_limit) {
            let oldest_key: string = keys[0];
            let oldest_timestamp: number = cache[oldest_key].use_timestamp;

            for (let key of keys) {
                if (oldest_timestamp > cache[key].use_timestamp) {
                    oldest_key = key;
                    oldest_timestamp = cache[oldest_key].use_timestamp;
                }
            }

            delete cache[oldest_key];

            keys = Object.keys(cache);
        }
    }

    update_uplot_5min_power_cache(date: Date) {
        let key = this.date_to_5min_key(date);
        let uplot_data = this.uplot_5min_power_cache[key];
        let needs_update = false;
        let now = Date.now();

        if (!uplot_data) {
            needs_update = true;
        }
        else {
            let energy_manager_data = this.energy_manager_5min_cache[key];

            if (energy_manager_data && uplot_data.update_timestamp < energy_manager_data.update_timestamp) {
                needs_update = true;
            }

            if (!needs_update) {
                for (let charger of this.chargers) {
                    if (this.wallbox_5min_cache[charger.uid]) {
                        let wallbox_data = this.wallbox_5min_cache[charger.uid][key];

                        if (wallbox_data && uplot_data.update_timestamp < wallbox_data.update_timestamp) {
                            needs_update = true;
                            break;
                        }
                    }
                }
            }
        }

        if (!needs_update) {
            // cache is valid
            uplot_data.use_timestamp = now;
            return;
        }

        uplot_data = {
            update_timestamp: now,
            use_timestamp: now,
            keys: [null],
            names: [null],
            values: [null],
            extras: [null],
            stacked: [false],
            bars: [false],
            extra_names: [null],
        };

        let timestamp_slot_count: number = 0;
        let energy_manager_data = this.energy_manager_5min_cache[key];

        if (energy_manager_data) {
            for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                if (!energy_manager_data.power_empty[meter_slot]) {
                    timestamp_slot_count = Math.max(timestamp_slot_count, energy_manager_data.power[meter_slot].length);

                    uplot_data.keys.push('em_power_' + meter_slot);
                    uplot_data.names.push(get_meter_name(this.state.meter_configs, meter_slot));
                    uplot_data.values.push(energy_manager_data.power[meter_slot]);
                    uplot_data.extras.push(null);
                    uplot_data.stacked.push(false);
                    uplot_data.bars.push(false);
                    uplot_data.extra_names.push(null);
                }
            }
        }

        for (let charger of this.chargers) {
            if (this.wallbox_5min_cache[charger.uid]) {
                let wallbox_data = this.wallbox_5min_cache[charger.uid][key];

                if (wallbox_data && !wallbox_data.empty) {
                    timestamp_slot_count = Math.max(timestamp_slot_count, wallbox_data.power.length);

                    let state = new Array(wallbox_data.flags.length);

                    for (let i = 0; i < wallbox_data.flags.length; ++i) {
                        if (wallbox_data.flags[i] === null) {
                            state[i] = null;
                        }
                        else {
                            state[i] = wallbox_data.flags[i] & 0b111;
                        }
                    }

                    uplot_data.keys.push('wb_power_' + charger.uid);
                    uplot_data.names.push(charger.name);
                    uplot_data.values.push(wallbox_data.power);
                    uplot_data.extras.push(state);
                    uplot_data.stacked.push(true);
                    uplot_data.bars.push(false);
                    uplot_data.extra_names.push(wb_state_names);
                }
            }
        }

        let timestamps: number[] = new Array(timestamp_slot_count);
        let base = date.getTime() / 1000;

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            timestamps[timestamp_slot] = base + timestamp_slot * 300;
        }

        uplot_data.values[0] = timestamps;

        this.uplot_5min_power_cache[key] = uplot_data;
        this.expire_cache(this.uplot_5min_power_cache);
    }

    update_uplot_5min_flags_cache(date: Date) {
        let key = this.date_to_5min_key(date);
        let uplot_data = this.uplot_5min_flags_cache[key];
        let needs_update = false;
        let now = Date.now();

        if (!uplot_data) {
            needs_update = true;
        }
        else {
            let energy_manager_data = this.energy_manager_5min_cache[key];

            if (energy_manager_data && uplot_data.update_timestamp < energy_manager_data.update_timestamp) {
                needs_update = true;
            }

            if (!needs_update) {
                for (let charger of this.chargers) {
                    if (this.wallbox_5min_cache[charger.uid]) {
                        let wallbox_data = this.wallbox_5min_cache[charger.uid][key];

                        if (wallbox_data && uplot_data.update_timestamp < wallbox_data.update_timestamp) {
                            needs_update = true;
                            break;
                        }
                    }
                }
            }
        }

        if (!needs_update) {
            // cache is valid
            uplot_data.use_timestamp = now;
            return;
        }

        uplot_data = {
            update_timestamp: now,
            use_timestamp: now,
            keys: [null],
            names: [null],
            values: [null],
            stacked: [false],
            bars: [false],
            value_names: [null],
            value_strokes: [null],
            value_fills: [null],
            default_visibilty: [null],
        };

        let timestamp_slot_count: number = 0;
        let energy_manager_data = this.energy_manager_5min_cache[key];

        if (energy_manager_data && !energy_manager_data.empty) {
            timestamp_slot_count = Math.max(timestamp_slot_count, energy_manager_data.flags.length);

            let phase = new Array(energy_manager_data.flags.length);
            let input3 = new Array(energy_manager_data.flags.length);
            let input4 = new Array(energy_manager_data.flags.length);
            let relay = new Array(energy_manager_data.flags.length);

            for (let i = 0; i < energy_manager_data.flags.length; ++i) {
                if (energy_manager_data.flags[i] === null) {
                    phase[i] = null;
                    input3[i] = null;
                    input4[i] = null;
                    relay[i] = null;
                }
                else {
                    if (i > 0 && energy_manager_data.flags[i - 1] !== null && (energy_manager_data.flags[i] & 0b0001) == (energy_manager_data.flags[i - 1] & 0b0001)) {
                        phase[i] = undefined;
                    }
                    else {
                        phase[i] = energy_manager_data.flags[i] & 0b0001;
                    }

                    if (i > 0 && energy_manager_data.flags[i - 1] !== null && (energy_manager_data.flags[i] & 0b0010) == (energy_manager_data.flags[i - 1] & 0b0010)) {
                        input3[i] = undefined;
                    }
                    else {
                        input3[i] = (energy_manager_data.flags[i] & 0b0010) >> 1;
                    }

                    if (i > 0 && energy_manager_data.flags[i - 1] !== null && (energy_manager_data.flags[i] & 0b0100) == (energy_manager_data.flags[i - 1] & 0b0100)) {
                        input4[i] = undefined;
                    }
                    else {
                        input4[i] = (energy_manager_data.flags[i] & 0b0100) >> 2;
                    }

                    if (i > 0 && energy_manager_data.flags[i - 1] !== null && (energy_manager_data.flags[i] & 0b1000) == (energy_manager_data.flags[i - 1] & 0b1000)) {
                        relay[i] = undefined;
                    }
                    else {
                        relay[i] = (energy_manager_data.flags[i] & 0b1000) >> 3;
                    }
                }
            }

            uplot_data.keys.push('em_phase');
            uplot_data.names.push(__("em_energy_analysis.content.state_phase"));
            uplot_data.values.push(phase);
            uplot_data.stacked.push(false);
            uplot_data.bars.push(false);
            uplot_data.value_names.push(em_phase_names);
            uplot_data.value_strokes.push(em_phase_strokes);
            uplot_data.value_fills.push(em_phase_fills);
            uplot_data.default_visibilty.push(true);

            uplot_data.keys.push('em_input3');
            uplot_data.names.push(__("em_energy_analysis.content.state_input3"));
            uplot_data.values.push(input3);
            uplot_data.stacked.push(false);
            uplot_data.bars.push(false);
            uplot_data.value_names.push(em_input_names);
            uplot_data.value_strokes.push(em_input_strokes);
            uplot_data.value_fills.push(em_input_fills);
            uplot_data.default_visibilty.push(false);

            uplot_data.keys.push('em_input4');
            uplot_data.names.push(__("em_energy_analysis.content.state_input4"));
            uplot_data.values.push(input4);
            uplot_data.stacked.push(false);
            uplot_data.bars.push(false);
            uplot_data.value_names.push(em_input_names);
            uplot_data.value_strokes.push(em_input_strokes);
            uplot_data.value_fills.push(em_input_fills);
            uplot_data.default_visibilty.push(false);

            uplot_data.keys.push('em_relay');
            uplot_data.names.push(__("em_energy_analysis.content.state_relay"));
            uplot_data.values.push(relay);
            uplot_data.stacked.push(false);
            uplot_data.bars.push(false);
            uplot_data.value_names.push(em_relay_names);
            uplot_data.value_strokes.push(em_relay_strokes);
            uplot_data.value_fills.push(em_relay_fills);
            uplot_data.default_visibilty.push(false);
        }

        for (let charger of this.chargers) {
            if (this.wallbox_5min_cache[charger.uid]) {
                let wallbox_data = this.wallbox_5min_cache[charger.uid][key];

                if (wallbox_data && !wallbox_data.empty) {
                    timestamp_slot_count = Math.max(timestamp_slot_count, wallbox_data.flags.length);

                    let state = new Array(wallbox_data.flags.length);

                    for (let i = 0; i < wallbox_data.flags.length; ++i) {
                        if (wallbox_data.flags[i] === null) {
                            state[i] = null;
                        }
                        else if (i > 0 && wallbox_data.flags[i - 1] !== null && (wallbox_data.flags[i] & 0b111) == (wallbox_data.flags[i - 1] & 0b111)) {
                            state[i] = undefined;
                        }
                        else {
                            state[i] = wallbox_data.flags[i] & 0b111;
                        }
                    }

                    uplot_data.keys.push('wb_state_' + charger.uid);
                    uplot_data.names.push(charger.name);
                    uplot_data.values.push(state);
                    uplot_data.stacked.push(true);
                    uplot_data.bars.push(false);
                    uplot_data.value_names.push(wb_state_names);
                    uplot_data.value_strokes.push(wb_state_strokes);
                    uplot_data.value_fills.push(wb_state_fills);
                    uplot_data.default_visibilty.push(true);
                }
            }
        }

        let timestamps: number[] = new Array(timestamp_slot_count);
        let base = date.getTime() / 1000;

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            timestamps[timestamp_slot] = base + timestamp_slot * 300;
        }

        uplot_data.values[0] = timestamps;

        this.uplot_5min_flags_cache[key] = uplot_data;
        this.expire_cache(this.uplot_5min_flags_cache);
    }

    update_uplot_5min_status_cache(date: Date) {
        let key = this.date_to_5min_key(date);
        let uplot_data = this.uplot_5min_status_cache[key];
        let needs_update = false;
        let now = Date.now();

        if (!uplot_data) {
            needs_update = true;
        }
        else {
            let energy_manager_data = this.energy_manager_5min_cache[key];

            if (energy_manager_data && uplot_data.update_timestamp < energy_manager_data.update_timestamp) {
                needs_update = true;
            }
        }

        if (!needs_update) {
            // cache is valid
            uplot_data.use_timestamp = now;
            return;
        }

        uplot_data = {
            update_timestamp: now,
            use_timestamp: now,
            keys: [null],
            names: [null],
            values: [null],
            stacked: [false],
            bars: [false],
        };

        let timestamp_slot_count: number = 0;
        let energy_manager_data = this.energy_manager_5min_cache[key];

        if (energy_manager_data && !energy_manager_data.power_empty[this.state.meter_slot_status]) {
            timestamp_slot_count = Math.max(timestamp_slot_count, energy_manager_data.power[this.state.meter_slot_status].length)

            uplot_data.keys.push('em_power_' + this.state.meter_slot_status);
            uplot_data.names.push(get_meter_name(this.state.meter_configs, this.state.meter_slot_status));
            uplot_data.values.push(energy_manager_data.power[this.state.meter_slot_status]);
            uplot_data.stacked.push(false);
            uplot_data.bars.push(false);
        }

        let timestamps: number[] = new Array(timestamp_slot_count);
        let base = date.getTime() / 1000;

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            timestamps[timestamp_slot] = base + timestamp_slot * 5 * 60;
        }

        uplot_data.values[0] = timestamps;

        this.uplot_5min_status_cache[key] = uplot_data;
        this.expire_cache(this.uplot_5min_status_cache);
    }

    update_uplot_daily_cache(date: Date) {
        let key = this.date_to_daily_key(date);
        let previous_key = this.date_to_daily_key(new Date(date.getFullYear(), date.getMonth() - 1));
        let uplot_data = this.uplot_daily_cache[key];
        let needs_update = false;
        let now = Date.now();

        if (!uplot_data) {
            needs_update = true;
        }
        else {
            let energy_manager_data = this.energy_manager_daily_cache[key];

            if (energy_manager_data && uplot_data.update_timestamp < energy_manager_data.update_timestamp) {
                needs_update = true;
            }

            if (!needs_update) {
                let energy_manager_previous_data = this.energy_manager_daily_cache[previous_key];

                if (energy_manager_previous_data && uplot_data.update_timestamp < energy_manager_previous_data.update_timestamp) {
                    needs_update = true;
                }

                if (!needs_update) {
                    for (let charger of this.chargers) {
                        if (this.wallbox_daily_cache[charger.uid]) {
                            let wallbox_data = this.wallbox_daily_cache[charger.uid][key];

                            if (wallbox_data && uplot_data.update_timestamp < wallbox_data.update_timestamp) {
                                needs_update = true;
                                break;
                            }

                            let wallbox_previous_data = this.wallbox_daily_cache[charger.uid][previous_key];

                            if (wallbox_previous_data && uplot_data.update_timestamp < wallbox_previous_data.update_timestamp) {
                                needs_update = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (!needs_update) {
            // cache is valid
            uplot_data.use_timestamp = now;
            return;
        }

        uplot_data = {
            update_timestamp: now,
            use_timestamp: now,
            keys: [null],
            names: [null],
            values: [null],
            stacked: [false],
            bars: [false],
        };

        let timestamp_slot_count: number = 0;
        let energy_manager_data = this.energy_manager_daily_cache[key];
        let energy_manager_previous_data = this.energy_manager_daily_cache[previous_key];
        let energy_import_5min_total = new Array(METERS_SLOTS);
        let energy_import_daily_total = new Array(METERS_SLOTS);
        let energy_export_5min_total = new Array(METERS_SLOTS);
        let energy_export_daily_total = new Array(METERS_SLOTS);

        if (energy_manager_data && !energy_manager_data.empty) {
            for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                // energy_import and energy_export have the same length
                timestamp_slot_count = Math.max(timestamp_slot_count, energy_manager_data.energy_import[meter_slot].length);

                let energy_import = new Array(energy_manager_data.energy_import[meter_slot].length);
                let last_energy_import: number = null;

                if (energy_manager_previous_data) {
                    last_energy_import = array_last(energy_manager_previous_data.energy_import[meter_slot]);
                }

                energy_import_5min_total[meter_slot] = new Array(energy_manager_data.energy_import[meter_slot].length);
                energy_import_daily_total[meter_slot] = null;

                for (let timestamp_slot = 0; timestamp_slot < energy_manager_data.energy_import[meter_slot].length; ++timestamp_slot) {
                    if (energy_manager_data.energy_import[meter_slot][timestamp_slot] !== null && last_energy_import !== null) {
                        energy_import[timestamp_slot] = energy_manager_data.energy_import[meter_slot][timestamp_slot] - last_energy_import;

                        if (energy_import_daily_total[meter_slot] === null) {
                            energy_import_daily_total[meter_slot] = energy_import[timestamp_slot];
                        }
                        else {
                            energy_import_daily_total[meter_slot] += energy_import[timestamp_slot];
                        }
                    }
                    else {
                        energy_import[timestamp_slot] = null;
                    }

                    energy_import_5min_total[meter_slot][timestamp_slot] = energy_import[timestamp_slot];
                    last_energy_import = energy_manager_data.energy_import[meter_slot][timestamp_slot];
                }

                // FIXME: only show meter #0 for now, because there is no good way to show the other
                //        meters. they would have to be shown side by side, but there is no space
                if (meter_slot == 0) {
                    uplot_data.keys.push('em_energy_import_' + meter_slot);
                    uplot_data.names.push(`${get_meter_name(this.state.meter_configs, meter_slot)} (${__("em_energy_analysis.content.import")})`);
                    uplot_data.values.push(energy_import);
                    uplot_data.stacked.push(false);
                    uplot_data.bars.push(true);
                }

                let energy_export = new Array(energy_manager_data.energy_export[meter_slot].length);
                let last_energy_export: number = null;

                if (energy_manager_previous_data) {
                    last_energy_export = array_last(energy_manager_previous_data.energy_export[meter_slot]);
                }

                energy_export_5min_total[meter_slot] = new Array(energy_manager_data.energy_export[meter_slot].length);
                energy_export_daily_total[meter_slot] = null;

                for (let timestamp_slot = 0; timestamp_slot < energy_manager_data.energy_export[meter_slot].length; ++timestamp_slot) {
                    if (energy_manager_data.energy_export[meter_slot][timestamp_slot] !== null && last_energy_export !== null) {
                        energy_export[timestamp_slot] = energy_manager_data.energy_export[meter_slot][timestamp_slot] - last_energy_export;

                        if (energy_export[timestamp_slot] > 0) {
                            energy_export[timestamp_slot] = -energy_export[timestamp_slot];
                        }

                        if (energy_export_daily_total[meter_slot] === null) {
                            energy_export_daily_total[meter_slot] = energy_export[timestamp_slot];
                        }
                        else {
                            energy_export_daily_total[meter_slot] += energy_export[timestamp_slot];
                        }
                    }
                    else {
                        energy_export[timestamp_slot] = null;
                    }

                    energy_export_5min_total[meter_slot][timestamp_slot] = energy_export[timestamp_slot];
                    last_energy_export = energy_manager_data.energy_export[meter_slot][timestamp_slot];
                }

                // FIXME: only show meter #0 for now, because there is no good way to show the other
                //        meters. they would have to be shown side by side, but there is no space
                if (meter_slot == 0) {
                    uplot_data.keys.push('em_energy_export_' + meter_slot);
                    uplot_data.names.push(`${get_meter_name(this.state.meter_configs, meter_slot)} (${__("em_energy_analysis.content.export")})`);
                    uplot_data.values.push(energy_export);
                    uplot_data.stacked.push(false);
                    uplot_data.bars.push(true);
                }
            }

            this.setState((prevState) => ({
                energy_manager_5min_cache_energy_total: {
                    ...prevState.energy_manager_5min_cache_energy_total,
                    [key]: {
                        import: energy_import_5min_total,
                        export: energy_export_5min_total
                    }
                },
                energy_manager_daily_cache_energy_total: {
                    ...prevState.energy_manager_daily_cache_energy_total,
                    [key]: {
                        import: energy_import_daily_total,
                        export: energy_export_daily_total
                    }
                }
            }));
        }

        for (let charger of this.chargers) {
            if (this.wallbox_daily_cache[charger.uid]) {
                let wallbox_data = this.wallbox_daily_cache[charger.uid][key];
                let wallbox_previous_data = this.wallbox_daily_cache[charger.uid][previous_key];

                if (wallbox_data && !wallbox_data.empty) {
                    timestamp_slot_count = Math.max(timestamp_slot_count, wallbox_data.energy.length);

                    let energy = new Array(wallbox_data.energy.length);
                    let last_energy: number = null;
                    let energy_5min_total = new Array(wallbox_data.energy.length);
                    let energy_daily_total: number = null;

                    if (wallbox_previous_data) {
                        last_energy = array_last(wallbox_previous_data.energy);
                    }

                    for (let timestamp_slot = 0; timestamp_slot < wallbox_data.energy.length; ++timestamp_slot) {
                        if (wallbox_data.energy[timestamp_slot] !== null && last_energy !== null) {
                            energy[timestamp_slot] = wallbox_data.energy[timestamp_slot] - last_energy;

                            if (energy_daily_total === null) {
                                energy_daily_total = energy[timestamp_slot];
                            }
                            else {
                                energy_daily_total += energy[timestamp_slot];
                            }
                        }
                        else {
                            energy[timestamp_slot] = null;
                        }

                        energy_5min_total[timestamp_slot] = energy[timestamp_slot];
                        last_energy = wallbox_data.energy[timestamp_slot];
                    }

                    uplot_data.keys.push('wb_energy_' + charger.uid);
                    uplot_data.names.push(charger.name);
                    uplot_data.values.push(energy);
                    uplot_data.stacked.push(true);
                    uplot_data.bars.push(true);

                    this.setState((prevState) => ({
                        wallbox_5min_cache_energy_total: {
                            ...prevState.wallbox_5min_cache_energy_total,
                            [charger.uid]: {
                                ...(prevState.wallbox_5min_cache_energy_total[charger.uid] || {}), [key]: energy_5min_total
                            }
                        },
                        wallbox_daily_cache_energy_total: {
                            ...prevState.wallbox_daily_cache_energy_total,
                            [charger.uid]: {
                                ...(prevState.wallbox_daily_cache_energy_total[charger.uid] || {}), [key]: energy_daily_total
                            }
                        }
                    }));
                }
            }
        }

        let timestamps: number[] = new Array(timestamp_slot_count);
        let base = date.getTime() / 1000;

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            timestamps[timestamp_slot] = base + timestamp_slot * 24 * 60 * 60;
        }

        uplot_data.values[0] = timestamps;

        this.uplot_daily_cache[key] = uplot_data;
        this.expire_cache(this.uplot_daily_cache);
    }

    async update_wallbox_5min_cache_all(date: Date) {
        let all: Promise<boolean>[] = [];

        for (let charger of this.chargers) {
            all.push(this.update_wallbox_5min_cache(charger.uid, date));
        }

        let result = await (Promise<boolean[]>).all(all);

        for (let success of result) {
            if (!success) {
                return false;
            }
        }

        return true;
    }

    timestamp_to_5min_slot(timestamp: number) {
        return Math.floor(timestamp / (5 * 60 * 1000));
    }

    set_5min_loading() {
        if (this.uplot_wrapper_5min_flags_ref.current) {
            this.uplot_wrapper_5min_flags_ref.current.set_loading();
        }

        if (this.uplot_wrapper_5min_power_ref.current) {
            this.uplot_wrapper_5min_power_ref.current.set_loading();
        }

        if (this.uplot_loader_5min_ref.current) {
            this.uplot_loader_5min_ref.current.set_loading();
        }
    }

    async update_wallbox_5min_cache(uid: number, date: Date) {
        let now = Date.now();

        if (date.getTime() > now) {
            return true;
        }

        let key = this.date_to_5min_key(date);

        if (this.wallbox_5min_cache[uid]
            && this.wallbox_5min_cache[uid][key]
            && (this.wallbox_5min_cache[uid][key].complete
                || this.timestamp_to_5min_slot(this.wallbox_5min_cache[uid][key].update_timestamp) >= this.timestamp_to_5min_slot(now))) {
            // cache is valid
            this.wallbox_5min_cache[uid][key].use_timestamp = now;
            return true;
        }

        this.set_5min_loading();

        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;
        let day: number = date.getDate();
        let response: string = '';

        try {
            response = await (await util.put('energy_manager/history_wallbox_5min', {uid: uid, year: year, month: month, day: day})).text();
        } catch (e) {
            console.log('Energy Analysis: Could not get wallbox 5min data: ' + e);
            return false;
        }

        // reload now timestamp, because of the await call before, the previous value is
        // old and might result in wrong cache ordering because an uplot cache update could
        // have occurred during the await call
        now = Date.now();

        let payload = JSON.parse(response);
        let timestamp_slot_count = payload.length / 2;
        let data: Wallbox5minData = {
            update_timestamp: now,
            use_timestamp: now,
            empty: true,
            complete: key < this.date_to_5min_key(new Date(now)),
            flags: new Array(timestamp_slot_count),
            power: new Array(timestamp_slot_count),
        };

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            data.flags[timestamp_slot] = payload[timestamp_slot * 2];
            data.power[timestamp_slot] = payload[timestamp_slot * 2 + 1];

            if (data.flags[timestamp_slot] !== null) {
                data.empty = false;
            }
        }

        if (!this.wallbox_5min_cache[uid]) {
            this.wallbox_5min_cache[uid] = {};
        }

        this.wallbox_5min_cache[uid][key] = data;
        this.expire_cache(this.wallbox_5min_cache[uid]);

        return true;
    }

    async update_energy_manager_5min_cache(date: Date) {
        let now = Date.now();

        if (date.getTime() > now) {
            return true;
        }

        let key = this.date_to_5min_key(date);

        if (this.energy_manager_5min_cache[key]
            && (this.energy_manager_5min_cache[key].complete
                || this.timestamp_to_5min_slot(this.energy_manager_5min_cache[key].update_timestamp) >= this.timestamp_to_5min_slot(now))) {
            // cache is valid
            this.energy_manager_5min_cache[key].use_timestamp = now;
            return true;
        }

        this.set_5min_loading();

        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;
        let day: number = date.getDate();
        let response: string = '';

        try {
            response = await (await util.put('energy_manager/history_energy_manager_5min', {year: year, month: month, day: day})).text();
        } catch (e) {
            console.log('Energy Analysis: Could not get energy manager 5min data: ' + e);
            return false;
        }

        // reload now timestamp, because of the await call before, the previous value is
        // old and might result in wrong cache ordering because an uplot cache update could
        // have occurred during the await call
        now = Date.now();

        let payload = JSON.parse(response);
        let timestamp_slot_count = payload.length / 8;
        let data: EnergyManager5minData = {
            update_timestamp: now,
            use_timestamp: now,
            empty: true,
            complete: key < this.date_to_5min_key(new Date(now)),
            flags: new Array(METERS_SLOTS),
            power: new Array(METERS_SLOTS),
            power_empty: new Array(METERS_SLOTS),
        };

        data.power_empty.fill(true);

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            data.flags[timestamp_slot] = payload[timestamp_slot * 8];

            if (data.flags[timestamp_slot] !== null) {
                data.empty = false;
            }
        }

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            data.power[meter_slot] = new Array(timestamp_slot_count);

            for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
                data.power[meter_slot][timestamp_slot] = payload[timestamp_slot * 8 + 1 + meter_slot];

                if (data.power[meter_slot][timestamp_slot] !== null) {
                    data.power_empty[meter_slot] = false;
                }
            }
        }

        this.energy_manager_5min_cache[key] = data;
        this.expire_cache(this.energy_manager_5min_cache);

        return true;
    }

    async update_wallbox_daily_cache_all(date: Date) {
        let all: Promise<boolean>[] = [];

        for (let charger of this.chargers) {
            all.push(this.update_wallbox_daily_cache(charger.uid, date));
        }

        let result = await (Promise<boolean[]>).all(all);

        for (let success of result) {
            if (!success) {
                return false;
            }
        }

        return true;
    }

    timestamp_to_daily_slot(timestamp: number) {
        return Math.floor(timestamp / (24 * 60 * 60 * 1000));
    }

    set_daily_loading() {
        if (this.uplot_wrapper_daily_ref.current) {
            this.uplot_wrapper_daily_ref.current.set_loading();
        }

        if (this.uplot_loader_daily_ref.current) {
            this.uplot_loader_daily_ref.current.set_loading();
        }
    }

    async update_wallbox_daily_cache(uid: number, date: Date, update_previous?: boolean) {
        if (update_previous !== false) {
            let previous_date = new Date(date.getFullYear(), date.getMonth() - 1, date.getDate());

            await this.update_wallbox_daily_cache(uid, previous_date, false);
        }

        let now = Date.now();

        if (date.getTime() > now) {
            return true;
        }

        let key = this.date_to_daily_key(date);

        if (this.wallbox_daily_cache[uid]
            && this.wallbox_daily_cache[uid][key]
            && (this.wallbox_daily_cache[uid][key].complete
                || this.timestamp_to_daily_slot(this.wallbox_daily_cache[uid][key].update_timestamp) >= this.timestamp_to_daily_slot(now))) {
            // cache is valid
            this.wallbox_daily_cache[uid][key].use_timestamp = now;
            return true;
        }

        this.set_daily_loading();

        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;
        let response: string = '';

        try {
            response = await (await util.put('energy_manager/history_wallbox_daily', {uid: uid, year: year, month: month})).text();
        } catch (e) {
            console.log('Energy Analysis: Could not get wallbox daily data: ' + e);
            return false;
        }

        // reload now timestamp, because of the await call before, the previous value is
        // old and might result in wrong cache ordering because an uplot cache update could
        // have orccured during the await call
        now = Date.now();

        let payload = JSON.parse(response);
        let timestamp_slot_count = payload.length;
        let data: WallboxDailyData = {
            update_timestamp: now,
            use_timestamp: now,
            empty: true,
            complete: key < this.date_to_daily_key(new Date(now)),
            energy: new Array(timestamp_slot_count),
        };

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            data.energy[timestamp_slot] = payload[timestamp_slot];

            if (data.energy[timestamp_slot] !== null) {
                data.empty = false;
            }
        }

        if (!this.wallbox_daily_cache[uid]) {
            this.wallbox_daily_cache[uid] = {};
        }

        this.wallbox_daily_cache[uid][key] = data;
        this.expire_cache(this.wallbox_daily_cache[uid]);

        return true;
    }

    async update_energy_manager_daily_cache(date: Date, update_previous?: boolean) {
        if (update_previous !== false) {
            let previous_date = new Date(date.getFullYear(), date.getMonth() - 1, date.getDate());

            await this.update_energy_manager_daily_cache(previous_date, false);
        }

        let now = Date.now();

        if (date.getTime() > now) {
            return true;
        }

        let key = this.date_to_daily_key(date);

        if (this.energy_manager_daily_cache[key]
            && (this.energy_manager_daily_cache[key].complete
                || this.timestamp_to_daily_slot(this.energy_manager_daily_cache[key].update_timestamp) >= this.timestamp_to_daily_slot(now))) {
            // cache is valid
            this.energy_manager_daily_cache[key].use_timestamp = now;
            return true;
        }

        this.set_daily_loading();

        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;
        let response: string = '';

        try {
            response = await (await util.put('energy_manager/history_energy_manager_daily', {year: year, month: month})).text();
        } catch (e) {
            console.log('Energy Analysis: Could not get energy manager daily data: ' + e);
            return false;
        }

        // reload now timestamp, because of the await call before, the previous value is
        // old and might result in wrong cache ordering because an uplot cache update could
        // have orccured during the await call
        now = Date.now();

        let payload = JSON.parse(response);
        let timestamp_slot_count = payload.length / 14;
        let data: EnergyManagerDailyData = {
            update_timestamp: now,
            use_timestamp: now,
            empty: true,
            complete: key < this.date_to_daily_key(new Date(now)),
            energy_import: new Array(METERS_SLOTS),
            energy_export: new Array(METERS_SLOTS),
        };

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            data.energy_import[meter_slot] = new Array(timestamp_slot_count);
            data.energy_export[meter_slot] = new Array(timestamp_slot_count);

            for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
                data.energy_import[meter_slot][timestamp_slot] = payload[timestamp_slot * 14 + meter_slot];

                if (data.energy_import[meter_slot][timestamp_slot] !== null) {
                    data.empty = false;
                }

                data.energy_export[meter_slot][timestamp_slot] = payload[timestamp_slot * 14 + 7 + meter_slot];

                if (data.energy_export[meter_slot][timestamp_slot] !== null) {
                    data.empty = false;
                }
            }
        }

        this.energy_manager_daily_cache[key] = data;
        this.expire_cache(this.energy_manager_daily_cache);

        return true;
    }

    set_current_5min_date(date: Date) {
        this.setState({current_5min_date: date}, () => {
            this.update_current_5min_cache();
        });
    }

    set_current_daily_date(date: Date) {
        this.setState({current_daily_date: date}, () => {
            this.update_current_daily_cache();
        });
    }

    reload_wallbox_cache() {
        this.wallbox_5min_cache = {};
        this.wallbox_daily_cache = {};

        this.update_wallbox_5min_cache_all(this.state.current_5min_date)
            .then((success: boolean) => {
                if (!success) {
                    return Promise.resolve(false);
                }

                return this.update_wallbox_daily_cache_all(this.state.current_daily_date);
            })
            .then((success: boolean) => {
                if (!success) {
                    window.setTimeout(() => {
                        this.reload_wallbox_cache();
                    }, UPDATE_RETRY_DELAY);

                    return;
                }

                this.update_uplot();
            });
    }

    update_current_5min_cache() {
        let current_daily_date: Date = new Date(this.state.current_5min_date);

        current_daily_date.setDate(1);
        current_daily_date.setHours(0);
        current_daily_date.setMinutes(0);
        current_daily_date.setSeconds(0);
        current_daily_date.setMilliseconds(0);

        this.update_energy_manager_5min_cache(this.state.current_5min_date)
            .then((success: boolean) => {
                if (!success) {
                    return Promise.resolve(false);
                }

                return this.update_wallbox_5min_cache_all(this.state.current_5min_date);
            })
            .then((success: boolean) => {
                if (!success) {
                    return Promise.resolve(false);
                }

                return this.update_energy_manager_daily_cache(current_daily_date);
            })
            .then((success: boolean) => {
                if (!success) {
                    return Promise.resolve(false);
                }

                return this.update_wallbox_daily_cache_all(current_daily_date);
            })
            .then((success: boolean) => {
                if (!success) {
                    window.setTimeout(() => {
                        this.update_current_5min_cache();
                    }, UPDATE_RETRY_DELAY);

                    return;
                }

                this.update_uplot();
            });
    }

    update_current_daily_cache() {
        this.update_energy_manager_daily_cache(this.state.current_daily_date)
            .then((success: boolean) => {
                if (!success) {
                    return Promise.resolve(false);
                }

                return this.update_wallbox_daily_cache_all(this.state.current_daily_date);
            })
            .then((success: boolean) => {
                if (!success) {
                    window.setTimeout(() => {
                        this.update_current_daily_cache();
                    }, UPDATE_RETRY_DELAY);

                    return;
                }

                this.update_uplot();
            });
    }

    schedule_uplot_update() {
        if (this.uplot_update_timeout) {
            window.clearTimeout(this.uplot_update_timeout);
        }

        this.uplot_update_timeout = window.setTimeout(() => {
            this.uplot_update_timeout = null;
            this.update_uplot();
        }, 100);
    }

    update_uplot() {
        if (this.state.data_type == '5min') {
            if (this.uplot_loader_5min_ref.current && this.uplot_wrapper_5min_power_ref.current && this.uplot_wrapper_5min_flags_ref.current) {
                this.update_uplot_5min_power_cache(this.state.current_5min_date);
                this.update_uplot_5min_flags_cache(this.state.current_5min_date);

                let current_daily_date: Date = new Date(this.state.current_5min_date);

                current_daily_date.setDate(1);
                current_daily_date.setHours(0);
                current_daily_date.setMinutes(0);
                current_daily_date.setSeconds(0);
                current_daily_date.setMilliseconds(0);

                this.update_uplot_daily_cache(current_daily_date);

                let key = this.date_to_5min_key(this.state.current_5min_date);
                let data_power = this.uplot_5min_power_cache[key];
                let data_flags = this.uplot_5min_flags_cache[key];

                let visible_flags = data_flags !== undefined && data_flags.keys.length > 1;
                let visible_power = data_power !== undefined && data_power.keys.length > 1;
                let visible = visible_flags || visible_power;

                if (visible_flags && !visible_power) {
                    data_power = {
                        update_timestamp: null,
                        use_timestamp: null,
                        keys: [null],
                        names: [null],
                        values: [data_flags.values[0]],
                        extras: [null],
                        stacked: [false],
                        bars: [false],
                        extra_names: [null],
                    };
                }

                if (!visible_flags && visible_power) {
                    data_flags = {
                        update_timestamp: null,
                        use_timestamp: null,
                        keys: [null],
                        names: [null],
                        values: [data_power.values[0]],
                        extras: [null],
                        stacked: [false],
                        bars: [false],
                        extra_names: [null],
                    };
                }

                this.uplot_loader_5min_ref.current.set_data(data_flags, visible);
                this.uplot_wrapper_5min_power_ref.current.set_data(data_power, visible);
                this.uplot_wrapper_5min_flags_ref.current.set_data(data_flags, visible);
            }
        }
        else {
            if (this.uplot_loader_daily_ref.current && this.uplot_wrapper_daily_ref.current) {
                this.update_uplot_daily_cache(this.state.current_daily_date);

                let key = this.date_to_daily_key(this.state.current_daily_date);
                let data = this.uplot_daily_cache[key];

                this.uplot_loader_daily_ref.current.set_data(data);
                this.uplot_wrapper_daily_ref.current.set_data(data);
            }
        }

        if (this.props.status_ref.current && this.props.status_ref.current.uplot_wrapper_ref.current) {
            let status_date: Date = new Date();

            status_date.setHours(0);
            status_date.setMinutes(0);
            status_date.setSeconds(0);
            status_date.setMilliseconds(0);

            this.update_uplot_5min_status_cache(status_date);

            let key = this.date_to_5min_key(status_date);
            let data = this.uplot_5min_status_cache[key];

            this.props.status_ref.current.uplot_loader_ref.current.set_data(data);
            this.props.status_ref.current.uplot_wrapper_ref.current.set_data(data);
        }
    }

    render(props: {}, state: Readonly<EMEnergyAnalysisState>) {
        if (!util.render_allowed()) {
            return (<></>);
        }

        let total_5min = () => {
            let key = this.date_to_daily_key(state.current_5min_date);
            let energy_total = state.energy_manager_5min_cache_energy_total[key];
            let timestamp_slot = state.current_5min_date.getDate() - 1;
            let rows = [];

            if (util.hasValue(energy_total)) {
                for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                    let has_import = util.hasValue(energy_total.import) && util.hasValue(energy_total.import[meter_slot]) && util.hasValue(energy_total.import[meter_slot][timestamp_slot]);
                    let has_export = util.hasValue(energy_total.export) && util.hasValue(energy_total.export[meter_slot]) && util.hasValue(energy_total.export[meter_slot][timestamp_slot]);

                    if (has_import || has_export) {
                        rows.push(
                            <FormRow label={`${get_meter_name(this.state.meter_configs, meter_slot)} (${__("em_energy_analysis.content.import")} / ${__("em_energy_analysis.content.export")})`}>
                                <div class="row">
                                    <div class="col-md-6">
                                        {has_import ?
                                            <OutputFloat value={energy_total.import[meter_slot][timestamp_slot]} digits={2} scale={0} unit="kWh"/>
                                            : undefined
                                        }
                                    </div>
                                    <div class="col-md-6">
                                        {has_export ?
                                            <OutputFloat value={energy_total.export[meter_slot][timestamp_slot]} digits={2} scale={0} unit="kWh"/>
                                            : undefined
                                        }
                                    </div>
                                </div>
                            </FormRow>
                        );
                    }
                }
            }

            for (let charger of this.chargers) {
                let energy_total = ((state.wallbox_5min_cache_energy_total[charger.uid] || {})[key] || [])[state.current_5min_date.getDate() - 1];

                if (util.hasValue(energy_total)) {
                    rows.push(
                        <FormRow label={charger.name}>
                            <div class="row">
                                <div class="col-md-6">
                                    <OutputFloat value={energy_total} digits={2} scale={0} unit="kWh"/>
                                </div>
                                <div class="col-md-6">
                                </div>
                            </div>
                        </FormRow>
                    );
                }
            }

            return rows;
        };

        let total_daily = () => {
            let key = this.date_to_daily_key(state.current_daily_date);
            let energy_total = state.energy_manager_daily_cache_energy_total[key];
            let rows = [];

            if (util.hasValue(energy_total)) {
                for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                    let has_import = util.hasValue(energy_total.import) && util.hasValue(energy_total.import[meter_slot]);
                    let has_export = util.hasValue(energy_total.export) && util.hasValue(energy_total.export[meter_slot]);

                    if (has_import || has_export) {
                        rows.push(
                            <FormRow label={`${get_meter_name(this.state.meter_configs, meter_slot)} (${__("em_energy_analysis.content.import")} / ${__("em_energy_analysis.content.export")})`}>
                                <div class="row">
                                    <div class="col-md-6">
                                        {has_import ?
                                            <OutputFloat value={energy_total.import[meter_slot]} digits={2} scale={0} unit="kWh"/>
                                            : undefined
                                        }
                                    </div>
                                    <div class="col-md-6">
                                        {has_export ?
                                            <OutputFloat value={energy_total.export[meter_slot]} digits={2} scale={0} unit="kWh"/>
                                            : undefined
                                        }
                                    </div>
                                </div>
                            </FormRow>
                        );
                    }
                }
            }

            for (let charger of this.chargers) {
                let energy_total = (state.wallbox_daily_cache_energy_total[charger.uid] || {})[key];

                if (util.hasValue(energy_total)) {
                    rows.push(
                        <FormRow label={charger.name}>
                            <div class="row">
                                <div class="col-md-6">
                                    <OutputFloat value={energy_total} digits={2} scale={0} unit="kWh"/>
                                </div>
                                <div class="col-md-6">
                                </div>
                            </div>
                        </FormRow>
                    );
                }
            }

            return rows;
        };

        let data_select =
            <InputSelect value={state.data_type} style="width: 6rem" onValue={(v) => {
                    let data_type: '5min'|'daily' = v as any;

                    this.setState({data_type: data_type}, () => {
                        if (data_type == '5min') {
                            this.uplot_loader_5min_ref.current.set_show(true);
                            this.uplot_wrapper_5min_flags_ref.current.set_show(true);
                            this.uplot_wrapper_5min_power_ref.current.set_show(true);
                            this.uplot_loader_daily_ref.current.set_show(false);
                            this.uplot_wrapper_daily_ref.current.set_show(false);
                        }
                        else {
                            this.uplot_loader_daily_ref.current.set_show(true);
                            this.uplot_wrapper_daily_ref.current.set_show(true);
                            this.uplot_loader_5min_ref.current.set_show(false);
                            this.uplot_wrapper_5min_flags_ref.current.set_show(false);
                            this.uplot_wrapper_5min_power_ref.current.set_show(false);
                        }

                        this.update_uplot();
                    });
                }}
                items={[
                    ["5min", __("em_energy_analysis.content.data_type_5min")],
                    ["daily", __("em_energy_analysis.content.data_type_daily")],
                ]}/>;

        return (
            <SubPage colClasses="col-xl-10">
                <PageHeader title={__("em_energy_analysis.content.em_energy_analysis")}>
                    <div>
                        {state.data_type == '5min'
                        ? <InputDate date={state.current_5min_date} onDate={this.set_current_5min_date.bind(this)} buttons="day" style="width: 11rem">{data_select}</InputDate>
                        : <InputMonth date={state.current_daily_date} onDate={this.set_current_daily_date.bind(this)} buttons="month" style="width: 11rem">{data_select}</InputMonth>}
                    </div>
                </PageHeader>

                <div style="position: relative;"> {/* this plain div is neccessary to make the size calculation stable in safari. without this div the height continues to grow */}
                    <UplotLoader ref={this.uplot_loader_5min_ref}
                                    show={true}
                                    marker_class={'h3'} >
                        <UplotFlagsWrapper ref={this.uplot_wrapper_5min_flags_ref}
                                            id="em_energy_analysis_5min_flags_chart"
                                            class="em-energy-analysis-flags-chart"
                                            sidebar_id="em_energy_analysis"
                                            show={true}
                                            sync={this.uplot_sync}
                                            legend_time_label={__("em_energy_analysis.script.time_5min")}
                                            legend_time_with_minutes={true}
                                            legend_value_prefix={""}
                                            legend_div_ref={this.uplot_legend_div_5min_flags_ref}
                                            x_format={{hour: '2-digit', minute: '2-digit'}}
                                            x_padding_factor={0}
                                            y_sync_ref={this.uplot_wrapper_5min_power_ref} />
                        <UplotWrapper ref={this.uplot_wrapper_5min_power_ref}
                                        id="em_energy_analysis_5min_power_chart"
                                        class="em-energy-analysis-chart pb-4"
                                        sidebar_id="em_energy_analysis"
                                        color_cache_group="analsyis"
                                        show={true}
                                        sync={this.uplot_sync}
                                        legend_time_label={__("em_energy_analysis.script.time_5min")}
                                        legend_time_with_minutes={true}
                                        legend_value_prefix={""}
                                        legend_div_ref={this.uplot_legend_div_5min_power_ref}
                                        aspect_ratio={3}
                                        x_format={{hour: '2-digit', minute: '2-digit'}}
                                        x_padding_factor={0}
                                        y_min={0}
                                        y_max={100}
                                        y_unit={"W"}
                                        y_label={__("em_energy_analysis.script.power") + " [Watt]"}
                                        y_digits={0}
                                        y_skip_upper={true}
                                        y_sync_ref={this.uplot_wrapper_5min_flags_ref}
                                        padding={[0, 5, null, null] as uPlot.Padding}/>
                        <div class="uplot u-hz u-time-in-legend-alone" ref={this.uplot_legend_div_5min_flags_ref} style="width: 100%; visibility: hidden;" />
                        <div class="uplot u-hz u-hide-first-series-in-legend" ref={this.uplot_legend_div_5min_power_ref} style="width: 100%; visibility: hidden;" />
                    </UplotLoader>
                    <UplotLoader ref={this.uplot_loader_daily_ref}
                                    show={false}
                                    marker_class={'h3'} >
                        <UplotWrapper ref={this.uplot_wrapper_daily_ref}
                                        id="em_energy_analysis_daily_chart"
                                        class="em-energy-analysis-chart pb-4"
                                        sidebar_id="em_energy_analysis"
                                        color_cache_group="analsyis"
                                        show={false}
                                        legend_time_label={__("em_energy_analysis.script.time_daily")}
                                        legend_time_with_minutes={false}
                                        legend_value_prefix={""}
                                        aspect_ratio={3}
                                        x_format={{month: '2-digit', day: '2-digit'}}
                                        x_padding_factor={0.015}
                                        y_min={0}
                                        y_max={10}
                                        y_unit={"kWh"}
                                        y_label={__("em_energy_analysis.script.energy") + " [kWh]"}
                                        y_digits={2}
                                        default_fill={true}
                                        padding={[null, 5, null, null] as uPlot.Padding} />
                    </UplotLoader>
                </div>

                <FormSeparator heading={__("em_energy_analysis.script.total_energy")}/>

                {state.data_type == '5min' ?
                    <>
                        {
                            total_5min()
                        }
                    </> :
                    <>
                        {
                            total_daily()
                        }
                    </>
                }
            </SubPage>
        )
    }
}

let status_ref = createRef();

render(<EMEnergyAnalysisStatus ref={status_ref} />, $('#status-em_energy_analysis_status')[0]);

render(<EMEnergyAnalysis status_ref={status_ref} />, $('#em_energy_analysis')[0]);

export function init() {
}

export function add_event_listeners(source: API.APIEventTarget) {
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-em_energy_analysis').prop('hidden', !module_init.energy_manager);
}
