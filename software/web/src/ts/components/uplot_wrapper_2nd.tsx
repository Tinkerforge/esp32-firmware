/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

// em_energy_analysis

import { h, Component, RefObject, createRef } from "preact";
import { effect } from "@preact/signals-core";
import * as util from "../util";
import * as plot from "../plot";
import uPlot from "uplot";
import type { UplotFlagsWrapper } from './uplot_wrapper_3rd';

export interface CachedData {
    update_timestamp: number;
    use_timestamp: number;
}

export const enum UplotPath {
    Line = 0,
    Bar = 1,
    Step = 2,
}

export interface UplotData extends CachedData {
    keys: string[];
    names: string[];
    values: number[][];
    extras?: number[][];
    stacked: boolean[];
    paths?: UplotPath[];
    value_names?: {[id: number]: string}[];
    value_strokes?: {[id: number]: string}[];
    value_fills?: {[id: number]: string}[];
    extra_names?: {[id: number]: string}[];
    default_visibilty?: boolean[];
    lines_vertical?: {index: number, text: string, color: [number, number, number, number]}[];
}

interface UplotWrapperProps {
    class: string;
    sub_page: string;
    color_cache_group: string;
    show: boolean;
    on_mount?: () => void;
    sync?: uPlot.SyncPubSub;
    legend_time_label: string;
    legend_time_with_minutes: boolean;
    legend_div_ref?: RefObject<HTMLDivElement>;
    aspect_ratio: number;
    x_height: number;
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
    only_show_visible?: boolean;
}

export class UplotWrapper extends Component<UplotWrapperProps, {}> {
    uplot: uPlot;
    data: UplotData;
    pending_data: UplotData;
    pending_visible: boolean;
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

        effect(() => {
            this.visible = util.get_active_sub_page() == this.props.sub_page;

            if (this.visible && this.pending_data !== undefined) {
                this.set_data(this.pending_data, this.pending_visible);
            }
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
                        draw: [
                            (self: uPlot) => {
                                this.data?.lines_vertical?.forEach(line  => {
                                    const { ctx, bbox } = self;

                                    let xd = self.data[0];
                                    let x = self.valToPos(xd[line.index], 'x', true);
                                    let xn = self.valToPos(xd[line.index+1], 'x', true) - 1;

                                    ctx.save();

                                    ctx.beginPath();
                                    ctx.strokeStyle = `rgba(${line.color[0]}, ${line.color[1]}, ${line.color[2]}, ${line.color[3]})`;
                                    ctx.lineWidth = xn-x;
                                    ctx.moveTo(x+ctx.lineWidth/2, bbox.top);
                                    ctx.lineTo(x+ctx.lineWidth/2, bbox.top + bbox.height);
                                    ctx.stroke();

                                    if (line.text.length > 0) {
                                        ctx.lineWidth = 1;
                                        let metrics   = ctx.measureText(line.text);
                                        let text_mid  = metrics.width/2 + (xn-x)/2;
                                        ctx.fillStyle = `rgba(32, 32, 32, 1)`;
                                        ctx.fillText(line.text, x + text_mid, 1 + (metrics.actualBoundingBoxAscent + metrics.actualBoundingBoxDescent)/2);
                                    }

                                    ctx.restore();
                                });
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
            this.set_data(this.pending_data, this.pending_visible);
        }

        if (this.props.on_mount) {
            this.props.on_mount();
        }
    }

    render() {
        return <div ref={this.div_ref} class={this.props.class} style={`display: ${this.props.show ? 'block' : 'none'}; visibility: hidden;`} />;
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
        let color = plot.get_color(this.props.color_cache_group, name);
        let paths = undefined;

        if (this.data.paths) {
            if (this.data.paths[i] == UplotPath.Bar) {
                paths = uPlot.paths.bars({size: [0.4, 100], align: this.data.stacked[i] ? 1 : -1})
            }
            else if (this.data.paths[i] == UplotPath.Step) {
                paths = uPlot.paths.stepped({align: 1});
            }
        }

        return {
            show: this.series_visibility[this.data.keys[i]],
            pxAlign: 0,
            spanGaps: false,
            label: name,
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
            paths: paths,
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

                        if (y_max === undefined || value > y_max) {
                            y_max = value;
                        }
                    }
                }
            }
            else {
                let stacked_values: number[] = new Array(this.data.values[i].length);

                if ((this.props.only_show_visible !== true) || this.series_visibility[this.data.keys[i]]) {
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
                        fill: plot.get_color(this.props.color_cache_group, this.data.names[i]).fill,
                    });
                }

                last_stacked_index = i;
            }
        }
    }

    set_data(data: UplotData, visible?: boolean) {
        if (!this.uplot || !this.visible) {
            this.pending_data = data;
            this.pending_visible = visible;
            return;
        }

        this.data = data;
        this.pending_data = undefined;
        this.pending_visible = undefined;

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
