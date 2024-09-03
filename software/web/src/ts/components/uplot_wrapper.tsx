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

// meters

import { h, Component, RefObject, createRef } from "preact";
import { effect } from "@preact/signals-core";
import * as util from "../util";
import * as plot from "../plot";
import uPlot from "uplot";

export const enum UplotPath {
    Line = 0,
    Bar = 1,
    Step = 2,
}

export interface UplotData {
    keys: string[];
    names: string[];
    values: number[][];
    paths?: UplotPath[];
    default_visibilty?: boolean[];
}

interface UplotWrapperProps {
    class: string;
    sub_page: string;
    color_cache_group: string;
    show: boolean;
    on_mount?: () => void;
    sync?: uPlot.SyncPubSub;
    legend_time_label: string;
    legend_time_with_seconds: boolean;
    legend_div_ref?: RefObject<HTMLDivElement>;
    aspect_ratio: number;
    x_height: number;
    x_padding_factor: number;
    x_include_date: boolean;
    y_min?: number;
    y_max?: number;
    y_diff_min?: number;
    y_unit: string;
    y_label: string;
    y_digits: number;
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
                        3600 * 48,
                        3600 * 72,
                        3600 * 168,
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
                    label: this.props.y_label,
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

    set_show(show: boolean) {
        this.div_ref.current.style.display = show ? 'block' : 'none';

        if (this.props.legend_div_ref && this.props.legend_div_ref.current) {
            this.props.legend_div_ref.current.style.display = show ? 'block' : 'none';
        }
    }

    get_series_opts(i: number, fill: boolean): uPlot.Series {
        let name = this.data.names[i];
        let color = plot.get_color(this.props.color_cache_group, name);
        let paths = undefined;

        if (this.data.paths) {
            if (this.data.paths[i] == UplotPath.Bar) {
                paths = uPlot.paths.bars({size: [0.4, 100], align: -1})
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
            value: (self: uPlot, rawValue: number) => util.hasValue(rawValue) ? util.toLocaleFixed(rawValue, this.props.y_digits) + " " + this.props.y_unit : null,
            stroke: color.stroke,
            fill: fill ? color.fill : undefined,
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

                this.uplot.addSeries(this.get_series_opts(this.uplot.series.length, this.data.keys.length <= 2));
            }

            this.update_internal_data();
        }
    }
}
