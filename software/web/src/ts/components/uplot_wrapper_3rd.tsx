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
import uPlot from "uplot";
import type { UplotWrapper } from "./uplot_wrapper_2nd";
import { UplotData } from "./uplot_wrapper_2nd";
import { uPlotTimelinePlugin } from "../uplot-plugins";

interface UplotFlagsWrapperProps {
    class: string;
    sub_page: string;
    show: boolean;
    on_mount?: () => void;
    sync?: uPlot.SyncPubSub;
    legend_time_label: string;
    legend_time_with_minutes: boolean;
    legend_div_ref?: RefObject<HTMLDivElement>;
    x_padding_factor: number;
    y_sync_ref?: RefObject<UplotWrapper>;
    y2_enable?: boolean;
}

export class UplotFlagsWrapper extends Component<UplotFlagsWrapperProps, {}> {
    uplot: uPlot;
    data: UplotData;
    pending_data: UplotData;
    pending_visible: boolean;
    series_visibility: {[id: string]: boolean} = {};
    visible: boolean = false;
    div_ref = createRef();
    observer: ResizeObserver;
    bar_height: number = 20;
    bar_spacing: number = 5;
    y_size: number = 0;
    y_size_offset: number = 22;
    y_other_size: number = 0;
    y2_size: number = 0;
    y2_size_offset: number = 22;
    y2_other_size: number = 0;

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
                    font: '12px system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, "Noto Sans", sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol", "Noto Color Emoji"',
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
                        3600 * 72,
                        3600 * 168,
                    ],
                    values: (self: uPlot, splits: number[]) => {
                        return new Array(splits.length);
                    },
                    side: 0, // top
                },
                {
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
            legend: {
                live: !util.is_native_median_app(),
                mount: (self: uPlot, legend: HTMLElement) => {
                    if (this.props.legend_div_ref && this.props.legend_div_ref.current) {
                        this.props.legend_div_ref.current.appendChild(legend);
                    }
                },
            },
            padding: [null, 5, 0, null],
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

        if (this.props.y2_enable === true) {
            options.axes.push({
                side: 1, // right
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

                    this.y2_size = Math.ceil(size / devicePixelRatio) + this.y2_size_offset;
                    size = Math.max(this.y2_size, this.y2_other_size);

                    if (this.props.y_sync_ref && this.props.y_sync_ref.current) {
                        this.props.y_sync_ref.current.set_y2_other_size(this.y2_size);
                    }

                    return size;
                },
                values: (self: uPlot, splits: number[]) => {
                    let values: string[] = new Array(splits.length);

                    values.fill('');

                    return values;
                },
                grid: {
                    show: false, // FIXME: y and y2 grid are misaligned, hide y2 grid for now
                },
                ticks: {
                    show: false,
                },
            });
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

    set_y2_other_size(size: number) {
        if (this.y2_other_size == size) {
            return;
        }

        this.y2_other_size = size;

        if (this.y2_other_size != this.y2_size) {
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

        return {
            show: this.series_visibility[this.data.keys[i]],
            label: name,
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

            this.uplot.setData(this.data.values as any);
        }
    }
}
