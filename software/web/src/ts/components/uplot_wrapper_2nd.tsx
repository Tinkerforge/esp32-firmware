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

export const enum UplotPath {
    Line = 0,
    Bar = 1,
    Step = 2,
}

export interface UplotData {
    keys: string[];
    names: string[];
    values: number[][];
    extras?: number[][];
    stacked?: boolean[];
    filled?: boolean[];
    paths?: UplotPath[];
    value_names?: {[id: number]: string}[];
    value_strokes?: {[id: number]: string}[];
    value_fills?: {[id: number]: string}[];
    extra_names?: {[id: number]: string}[];
    default_visibilty?: boolean[];
    lines_vertical?: {index: number, text: string, color: [number, number, number, number]}[];
    y_axes?: ('y' | 'y2')[];
}

interface UplotWrapperBProps {
    class: string;
    sub_page: string;
    color_cache_group: string;
    show: boolean;
    on_mount?: () => void;
    sync?: uPlot.SyncPubSub;
    legend_show?: boolean;
    legend_time_label: string;
    legend_time_with_minutes: boolean;
    legend_div_ref?: RefObject<HTMLDivElement>;
    aspect_ratio: number;
    x_format: Intl.DateTimeFormatOptions | null;
    x_padding_factor: number;
    x_include_date: boolean;
    y_min?: number;
    y_max?: number;
    y_unit: string;
    y_label: string;
    y_digits: number;
    y_three_split?: boolean;
    y_skip_upper?: boolean;
    y_sync_ref?: RefObject<UplotFlagsWrapper>;
    y2_enable?: boolean;
    y2_min?: number;
    y2_max?: number;
    y2_unit?: string;
    y2_label?: string;
    y2_digits?: number;
    y2_skip_upper?: boolean;
    padding?: uPlot.Padding;
    only_show_visible?: boolean;
    grid_show?: boolean;
    height_min?: number;
}

export class UplotWrapperB extends Component<UplotWrapperBProps, {}> {
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
    y_size_offset: number = 22;
    y_other_size: number = 0;
    y_label_size: number = 20;
    y2_min: number = 0;
    y2_max: number = 0;
    y2_size: number = 0;
    y2_size_offset: number = 22;
    y2_other_size: number = 0;
    y2_label_size: number = 20;

    override shouldComponentUpdate() {
        return false;
    }

    override componentDidMount() {
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
            padding = [null, null, null, null];
        }

        let legend_show = (this.props.legend_show === undefined) || this.props.legend_show;
        let x_height = (this.props.x_include_date ? 55 : 35) - (legend_show ? 5 : 0);

        let options: uPlot.Options = {
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
                    grid: {
                        show: (this.props.grid_show === undefined) || this.props.grid_show
                    },
                    font: '12px system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, "Noto Sans", sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol", "Noto Color Emoji"',
                    size: x_height,
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
                    values: (self: uPlot, splits: number[], axisIdx: number, foundSpace: number, foundIncr: number) => {
                        let values: string[] = new Array(splits.length);
                        if (this.props.x_format == null) {
                            return values;
                        }

                        let last_year: string = null;
                        let last_month_and_day: string = null;

                        for (let i = 0; i < splits.length; ++i) {
                            let date = new Date(splits[i] * 1000);
                            let value = date.toLocaleString([], this.props.x_format);

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
                    label: this.props.y_label,
                    labelSize: this.y_label_size,
                    labelGap: 0,
                    labelFont: 'bold 14px system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, "Noto Sans", sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol", "Noto Color Emoji"',
                    font: '12px system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, "Noto Sans", sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol", "Noto Color Emoji"',
                    size: (self: uPlot, values: string[], axisIdx: number, cycleNum: number): number => {
                        let size = 0;
                        let size_correction_factor = 1;

                        if (values) {
                            self.ctx.save();
                            self.ctx.font = self.axes[axisIdx].font;

                            if (self.ctx.font == '10px sans-serif') {
                                // FIXME: For some unknown reason sometimes changing the canvas font doesn't work and the
                                //        canvas font stays "10px sans-serif", work around this using a rought correction
                                //        factor. This is not the best because the result of this correction is not exact
                                size_correction_factor = 12 * devicePixelRatio / 10;
                            }

                            for (let i = 0; i < values.length; ++i) {
                                size = Math.max(size, self.ctx.measureText(values[i]).width);
                            }

                            self.ctx.restore();
                        }

                        size *= size_correction_factor;

                        this.y_size = Math.ceil(size / devicePixelRatio) + this.y_size_offset;
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
                show: legend_show,
                live: !util.is_native_median_app(),
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

                                if (y > ctx.canvas.height - x_height * devicePixelRatio) {
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
                                // Make blocks from vertical lines
                                let blocks: {
                                    start_index: number,
                                    end_index: number,
                                    color: [number, number, number, number]
                                }[] = [];
                                const col_eq = (a: [number, number, number, number], b: [number, number, number, number]) =>  a.every((v, i) => v === b[i]);
                                let lines_vertical_sorted = this.data?.lines_vertical?.sort((a, b) => a.index - b.index);
                                lines_vertical_sorted?.forEach(line  => {
                                    if (blocks.length == 0) {
                                        blocks.push({
                                            start_index: line.index,
                                            end_index: line.index+1,
                                            color: line.color
                                        });
                                    } else if ((col_eq(blocks[blocks.length-1].color, line.color)) && (blocks[blocks.length-1].end_index == line.index)) {
                                        blocks[blocks.length-1].end_index = line.index+1;
                                    } else {
                                        blocks.push({
                                            start_index: line.index,
                                            end_index: line.index+1,
                                            color: line.color
                                        });
                                    }
                                });

                                // Draw blocks
                                blocks.forEach(block  => {
                                    const { ctx, bbox } = self;

                                    let xd = self.data[0];
                                    let x  = self.valToPos(xd[block.start_index], 'x', true);
                                    let xn = self.valToPos(xd[block.end_index],   'x', true);

                                    ctx.save();

                                    ctx.beginPath();
                                    ctx.fillStyle = `rgba(${block.color[0]}, ${block.color[1]}, ${block.color[2]}, ${block.color[3]})`;
                                    ctx.fillRect(x, bbox.top, xn-x, bbox.height);

                                    ctx.restore();
                                });

                                // Draw text on top of vertical lines
                                lines_vertical_sorted?.forEach(line  => {
                                    if (line.text.length > 0) {
                                        const { ctx, bbox } = self;

                                        let xd = self.data[0];
                                        let x  = self.valToPos(xd[line.index],   'x', true);
                                        let xn = self.valToPos(xd[line.index+1], 'x', true);

                                        ctx.save();

                                        ctx.lineWidth = 1;
                                        let metrics   = ctx.measureText(line.text);
                                        let text_mid  = metrics.width/2 + (xn-x)/2;
                                        ctx.fillStyle = `rgba(32, 32, 32, 1)`;
                                        ctx.fillText(line.text, x + text_mid, 12 * devicePixelRatio + (metrics.actualBoundingBoxAscent + metrics.actualBoundingBoxDescent) / 2);
                                        ctx.restore();
                                    }
                                });
                            },
                        ],
                    },
                },
            ],
        };

        if (this.props.y2_enable === true) {
            options.axes.push({
                label: this.props.y2_label,
                labelSize: this.y2_label_size,
                labelGap: 0,
                labelFont: 'bold 14px system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, "Noto Sans", sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol", "Noto Color Emoji"',
                font: '12px system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, "Noto Sans", sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol", "Noto Color Emoji"',
                side: 1, // right
                scale: 'y2',
                size: (self: uPlot, values: string[], axisIdx: number, cycleNum: number): number => {
                    let size = 0;
                    let size_correction_factor = 1;

                    if (values) {
                        self.ctx.save();
                        self.ctx.font = self.axes[axisIdx].font;

                        if (self.ctx.font == '10px sans-serif') {
                            // FIXME: For some unknown reason sometimes changing the canvas font doesn't work and the
                            //        canvas font stays "10px sans-serif", work around this using a rought correction
                            //        factor. This is not the best because the result of this correction is not exact
                            size_correction_factor = 12 * devicePixelRatio / 10;
                        }

                        for (let i = 0; i < values.length; ++i) {
                            size = Math.max(size, self.ctx.measureText(values[i]).width);
                        }

                        self.ctx.restore();
                    }

                    size *= size_correction_factor;

                    this.y2_size = Math.ceil(size / devicePixelRatio) + this.y2_size_offset;
                    size = Math.max(this.y2_size + this.y2_label_size, this.y2_other_size) - this.y2_label_size;

                    if (this.props.y_sync_ref && this.props.y_sync_ref.current) {
                        this.props.y_sync_ref.current.set_y2_other_size(this.y2_size + this.y2_label_size);
                    }

                    return size;
                },
                values: (self: uPlot, splits: number[]) => {
                    let values: string[] = new Array(splits.length);

                    for (let digits = 0; digits <= 3; ++digits) {
                        let last_value: string = null;
                        let unique = true;

                        for (let i = 0; i < splits.length; ++i) {
                            if (this.props.y2_skip_upper && splits[i] >= this.y2_max) {
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
                grid: {
                    show: false, // FIXME: y and y2 grid are misaligned, hide y2 grid for now
                },
            });

            options.scales['y2'] = {
                range: (self: uPlot, initMin: number, initMax: number, scaleKey: string): uPlot.Range.MinMax => {
                    return uPlot.rangeNum(this.y2_min, this.y2_max, {min: {}, max: {}});
                }
            };
        }

        if (this.props.y_three_split) {
            options.axes[1].splits = (self: uPlot, axisIdx: number, scaleMin: number, scaleMax: number, foundIncr: number, foundSpace: number) => {
                return [scaleMin, (scaleMin + scaleMax) / 2, scaleMax];
            };
        }

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

        const height_min = this.props.height_min ? this.props.height_min : 0;
        return {
            width: div.clientWidth,
            height: Math.max(height_min, Math.floor((div.clientWidth + (window.innerWidth - document.documentElement.clientWidth)) / aspect_ratio)),
        }
    }

    set_y_other_size(size: number) {
        if (this.y_other_size == size) {
            return;
        }

        this.y_other_size = size;

        if (this.y_other_size != this.y_size + this.y_label_size) {
            this.resize();
        }
    }

    set_y2_other_size(size: number) {
        if (this.y2_other_size == size) {
            return;
        }

        this.y2_other_size = size;

        if (this.y2_other_size != this.y2_size + this.y2_label_size) {
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
        let color = plot.get_color(this.props.color_cache_group, this.data.keys[i]);
        let paths = undefined;

        if (this.data.paths) {
            if (this.data.paths[i] == UplotPath.Bar) {
                paths = uPlot.paths.bars({size: [0.4, 100], align: this.data.stacked !== undefined && this.data.stacked[i] ? 1 : -1})
            }
            else if (this.data.paths[i] == UplotPath.Step) {
                paths = uPlot.paths.stepped({align: 1});
            }
        }

        let y_axis = this.data.y_axes ? this.data.y_axes[i] : 'y';
        let y_digits = y_axis == 'y' ? this.props.y_digits : this.props.y2_digits;
        let y_unit = y_axis == 'y' ? this.props.y_unit : this.props.y2_unit;

        return {
            show: this.series_visibility[this.data.keys[i]],
            pxAlign: 0,
            spanGaps: false,
            label: this.data.names[i],
            value: (self: uPlot, rawValue: number, seriesIdx: number, idx: number | null) => {
                if (rawValue !== null) {
                    let prefix = '';

                    if (this.data.extras && this.data.extra_names && this.data.extra_names[seriesIdx]) {
                        prefix = this.data.extra_names[seriesIdx][this.data.extras[seriesIdx][idx]] + ' / ';
                    }

                    return prefix + util.toLocaleFixed(this.data.values[seriesIdx][idx], y_digits) + " " + y_unit;
                }

                return null;
            },
            stroke: color.stroke,
            fill: (this.data.stacked !== undefined && this.data.stacked[i]) || (this.data.filled !== undefined && this.data.filled[i]) ? color.fill : undefined,
            width: 2,
            paths: paths,
            points: {
                show: false,
            },
            scale: y_axis,
        };
    }

    update_internal_data() {
        let y_min: {[id: string]: number} = {'y': this.props.y_min, 'y2': this.props.y2_min};
        let y_max: {[id: string]: number} = {'y': this.props.y_max, 'y2': this.props.y2_max};
        let last_stacked_values: {[id: string]: number[]} = {'y': [], 'y2': []};

        this.uplot.delBand(null);

        for (let i = this.data.values.length - 1; i > 0; --i) {
            let y_axis = this.data.y_axes ? this.data.y_axes[i] : 'y';

            if (this.data.stacked === undefined || !this.data.stacked[i]) {
                for (let k = 0; k < this.data.values[i].length; ++k) {
                    let value = this.data.values[i][k];

                    if (value !== null) {
                        if (y_min[y_axis] === undefined || value < y_min[y_axis]) {
                            y_min[y_axis] = value;
                        }

                        if (y_max[y_axis] === undefined || value > y_max[y_axis]) {
                            y_max[y_axis] = value;
                        }
                    }
                }
            }
            else {
                let stacked_values: number[] = new Array(this.data.values[i].length);

                if ((this.props.only_show_visible !== true) || this.series_visibility[this.data.keys[i]]) {
                    for (let k = 0; k < this.data.values[i].length; ++k) {
                        if (last_stacked_values[y_axis][k] !== null
                            && last_stacked_values[y_axis][k] !== undefined
                            && this.data.values[i][k] !== null
                            && this.data.values[i][k] !== undefined) {
                            stacked_values[k] = last_stacked_values[y_axis][k] + this.data.values[i][k];
                        } else {
                            stacked_values[k] = this.data.values[i][k];
                        }

                        if (stacked_values[k] !== null) {
                            if (y_min[y_axis] === undefined || stacked_values[k] < y_min[y_axis]) {
                                y_min[y_axis] = stacked_values[k];
                            }

                            if (y_max[y_axis] === undefined || stacked_values[k] > y_max[y_axis]) {
                                y_max[y_axis] = stacked_values[k];
                            }
                        }
                    }
                }

                last_stacked_values[y_axis] = stacked_values;
            }
        }

        for (let y_axis of ['y', 'y2']) {
            if (y_min[y_axis] === undefined && y_max[y_axis] === undefined) {
                y_min[y_axis] = 0;
                y_max[y_axis] = 0;
            }
            else if (y_min[y_axis] === undefined) {
                y_min[y_axis] = y_max[y_axis];
            }
            else if (y_max[y_axis] === undefined) {
                y_max[y_axis] = y_min[y_axis];
            }
        }

        this.y_min = y_min['y'];
        this.y_max = y_max['y'];
        this.y2_min = y_min['y2'];
        this.y2_max = y_max['y2'];

        let uplot_values: number[][] = [];
        last_stacked_values = {'y': [], 'y2': []};

        for (let i = this.data.values.length - 1; i >= 0; --i) {
            let y_axis = this.data.y_axes ? this.data.y_axes[i] : 'y';

            if (this.data.stacked === undefined || !this.data.stacked[i] || !this.series_visibility[this.data.keys[i]]) {
                uplot_values.unshift(this.data.values[i]);
            }
            else {
                let stacked_values: number[] = new Array(this.data.values[i].length);

                for (let k = 0; k < this.data.values[i].length; ++k) {
                    if (last_stacked_values[y_axis][k] !== null
                        && last_stacked_values[y_axis][k] !== undefined
                        && this.data.values[i][k] !== null
                        && this.data.values[i][k] !== undefined) {
                        stacked_values[k] = last_stacked_values[y_axis][k] + this.data.values[i][k];
                    } else {
                        stacked_values[k] = this.data.values[i][k];
                    }
                }

                uplot_values.unshift(stacked_values);
                last_stacked_values[y_axis] = stacked_values;
            }
        }

        this.uplot.setData(uplot_values as any);

        let last_stacked_index: {[id: string]: number} = {'y': null, 'y2': null};

        for (let i = this.data.values.length - 1; i > 0; --i) {
            let y_axis = this.data.y_axes ? this.data.y_axes[i] : 'y';

            if (this.data.stacked !== undefined && this.data.stacked[i] && this.series_visibility[this.data.keys[i]]) {
                if (last_stacked_index[y_axis] === null) {
                    this.uplot.delSeries(i);
                    this.uplot.addSeries(this.get_series_opts(i), i);
                } else {
                    this.uplot.addBand({
                        series: [i, last_stacked_index[y_axis]],
                        fill: plot.get_color(this.props.color_cache_group, this.data.keys[i]).fill,
                    });
                }

                last_stacked_index[y_axis] = i;
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
