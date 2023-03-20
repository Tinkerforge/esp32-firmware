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

import { h, render, createRef, Fragment, Component, ComponentChild, RefObject } from "preact";
import { __ } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";
import { InputDate } from "../../ts/components/input_date";
import { FormRow } from "../../ts/components/form_row";
import uPlot from 'uplot';

interface CachedData {
    update_timestamp: number;
    use_timestamp: number;
}

interface UplotData extends CachedData {
    keys: string[];
    names: string[];
    values: number[][];
    stacked: boolean[];
}

interface Wallbox5minData extends CachedData {
    empty: boolean;
    flags: number[]; // bit 0-2 = charger state, bit 7 = no data
    power: number[];
};

interface WallboxDailyData extends CachedData {
    empty: boolean;
    energy: number[]; // kWh
};

interface EnergyManager5minData extends CachedData {
    empty: boolean;
    flags: number[]; // bit 0 = 1p/3p, bit 1-2 = input, bit 3 = output, bit 7 = no data
    power_grid: number[]; // W
    power_grid_empty: boolean;
    power_general: number[][]; // W
};

interface EnergyManagerDailyData extends CachedData {
    empty: boolean;
    energy_grid_in: number[]; // kWh
    energy_grid_out: number[]; // kWh
    energy_general_in: number[][]; // kWh
    energy_general_out: number[][]; // kWh
};

interface UplotWrapperProps {
    id: string;
    class: string;
    sidebar_id: string;
    y_min?: number;
    y_max?: number;
    y_step?: number;
    marker_width_reduction: number;
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

class UplotWrapper extends Component<UplotWrapperProps, {}> {
    uplot: uPlot;
    series_count: number = 1;
    data: UplotData;
    pending_data: UplotData;
    series_visibility: {[id: string]: boolean} = {};
    visible: boolean = false;
    div_ref = createRef();
    no_data_ref = createRef();
    loading_ref = createRef();
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
                    label: __("em_energy_analysis.script.time"),
                    value: __("em_energy_analysis.script.time_legend_format"),
                },
            ],
            axes: [
                {
                    size: 35,
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
                        // tick incr  default      year  month  day   hour  min   sec   mode
                        [60,          "{HH}:{mm}", null, null,  null, null, null, null, 1],
                    ],
                },
                {
                    size: 55,
                }
            ],
            scales: {
                y: {
                    range: {
                        min: {
                            mode: 1 as uPlot.Range.SoftMode,
                        },
                        max: {
                            mode: 1 as uPlot.Range.SoftMode,
                        },
                    },
                },
            },
            plugins: [
                {
                    hooks: {
                        setSeries: (self: uPlot, seriesIdx: number, opts: uPlot.Series) => {
                            this.series_visibility[this.data.keys[seriesIdx]] = opts.show;
                            this.update_internal_data();
                        },
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
        return (
            <div>
                <div ref={this.no_data_ref} style={`position: absolute; width: calc(100% - ${props.marker_width_reduction}px); height: 100%; visibility: hidden; display: flex;`}>
                    <span class="h3" style="margin: auto;">{__("em_energy_analysis.content.no_data")}</span>
                </div>
                <div ref={this.loading_ref} style={`position: absolute; width: calc(100% - ${props.marker_width_reduction}px); height: 100%; display: flex;`}>
                    <span class="h3" style="margin: auto;">{__("em_energy_analysis.content.loading")}</span>
                </div>
                <div ref={this.div_ref} id={props.id} class={props.class} style="visibility: hidden;" />
            </div>
        );
    }

    set_loading() {
        this.div_ref.current.style.visibility = 'hidden';
        this.no_data_ref.current.style.visibility = 'hidden';
        this.loading_ref.current.style.visibility = 'visible';
    }

    get_series_opts(i: number): uPlot.Series {
        let name = this.data.names[i];

        return {
            show: this.series_visibility[this.data.keys[i]],
            pxAlign: 0,
            spanGaps: false,
            label: __("em_energy_analysis.script.power") + (name ? ' ' + name: ''),
            value: (self: uPlot, rawValue: number, seriesIdx: number, idx: number | null) => rawValue !== null ? this.data.values[seriesIdx][idx] + " W" : null,
            stroke: strokes[(i - 1) % strokes.length],
            fill: fills[(i - 1) % fills.length],
            width: 2,
        };
    }

    update_internal_data() {
        let y_min: number = this.props.y_min;
        let y_max: number = this.props.y_max;
        let last_stacked_values: number[] = [];

        this.uplot.delBand(null);

        for (let i = 1; i < this.data.values.length; ++i) {
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

        let y_step = this.props.y_step;

        if (y_step !== undefined) {
            y_min = Math.floor(y_min / y_step) * y_step;
            y_max = Math.ceil(y_max / y_step) * y_step;
        }

        this.uplot.setScale('y', {min: y_min, max: y_max});

        let uplot_values: number[][] = [];
        last_stacked_values = [];

        for (let i = 0; i < this.data.keys.length; ++i) {
            if (!this.data.stacked[i] || !this.series_visibility[this.data.keys[i]]) {
                uplot_values.push(this.data.values[i]);
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

                uplot_values.push(stacked_values);
                last_stacked_values = stacked_values;
            }
        }

        this.uplot.setData(uplot_values as any);

        let last_stacked_index: number = null;

        for (let i = 1; i < this.data.keys.length; ++i) {
            if (this.data.stacked[i] && this.series_visibility[this.data.keys[i]]) {
                if (last_stacked_index === null) {
                    this.uplot.delSeries(i);
                    this.uplot.addSeries(this.get_series_opts(i), i);
                } else {
                    this.uplot.addBand({
                        series: [i, last_stacked_index],
                        fill: fills[(i - 1) % fills.length],
                    });
                }

                last_stacked_index = i;
            }
        }
    }

    set_data(data: UplotData) {
        if (!this.uplot || !this.visible) {
            this.pending_data = data;
            return;
        }

        this.data = data;
        this.pending_data = undefined;
        this.loading_ref.current.style.visibility = 'hidden';

        if (!this.data || this.data.keys.length <= 1) {
            this.div_ref.current.style.visibility = 'hidden';
            this.no_data_ref.current.style.visibility = 'visible';
        }
        else {
            this.div_ref.current.style.visibility = 'visible';
            this.no_data_ref.current.style.visibility = 'hidden';

            while (this.series_count > 1) {
                --this.series_count;

                this.uplot.delSeries(this.series_count);
            }

            while (this.series_count < this.data.keys.length) {
                if (this.series_visibility[this.data.keys[this.series_count]] === undefined) {
                    this.series_visibility[this.data.keys[this.series_count]] = true;
                }

                this.uplot.addSeries(this.get_series_opts(this.series_count));

                ++this.series_count;
            }

            this.update_internal_data();
        }
    }
}

export class EMEnergyAnalysisStatusChart extends Component<{}, {force_render: number}> {
    uplot_wrapper_ref = createRef();

    constructor() {
        super();

        this.state = {
            force_render: 0,
        } as any;

        util.eventTarget.addEventListener('info/modules', () => {
            this.setState({force_render: Date.now()});
        });
    }

    render(props: {}, state: {}) {
        if (!util.allow_render) {
            return (<></>);
        }

        return (
            <>
                <UplotWrapper ref={this.uplot_wrapper_ref}
                              id="em_energy_analysis_status_chart"
                              class="em-energy-analysis-status-chart"
                              sidebar_id="status"
                              y_min={0}
                              y_max={1500}
                              marker_width_reduction={8} />
            </>
        )
    }
}

interface EMEnergyAnalysisProps {
    status_ref: RefObject<EMEnergyAnalysisStatusChart>;
}

interface EMEnergyAnalysisState {
    force_render: number,
    current_5min_date: Date;
}

interface Charger {
    uid: number;
    name: string;
}

export class EMEnergyAnalysis extends Component<EMEnergyAnalysisProps, EMEnergyAnalysisState> {
    uplot_wrapper_ref = createRef();
    status_ref: RefObject<EMEnergyAnalysisStatusChart> = null;
    uplot_update_timeout: number = null;
    uplot_5min_cache: {[id: string]: UplotData} = {};
    uplot_5min_status_cache: {[id: string]: UplotData} = {};
    uplot_daily_cache: {[id: string]: UplotData} = {};
    wallbox_5min_cache: {[id: number]: { [id: string]: Wallbox5minData}} = {};
    wallbox_daily_cache: {[id: string]: { [id: string]: WallboxDailyData}} = {};
    energy_manager_5min_cache: {[id: string]: EnergyManager5minData} = {};
    energy_manager_daily_cache: {[id: string]: EnergyManagerDailyData} = {};
    cache_limit = 100;
    chargers: Charger[] = [];

    constructor(props: EMEnergyAnalysisProps) {
        super(props);

        this.status_ref = props.status_ref;

        let current_5min_date: Date = new Date();

        current_5min_date.setHours(0);
        current_5min_date.setMinutes(0);
        current_5min_date.setSeconds(0);
        current_5min_date.setMilliseconds(0);

        this.state = {
            force_render: 0,
            current_5min_date: current_5min_date,
        } as any;

        util.eventTarget.addEventListener('info/modules', () => {
            this.update_current_5min_cache();

            this.setState({force_render: Date.now()});
        });

        util.eventTarget.addEventListener('charge_manager/state', () => {
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

        util.eventTarget.addEventListener('energy_manager/history_wallbox_5min_changed', () => {
            let changed = API.get('energy_manager/history_wallbox_5min_changed');
            let subcache = this.wallbox_5min_cache[changed.uid];

            if (!subcache) {
                // got changed event without having this UID cached before
                this.update_wallbox_5min_cache(changed.uid, new Date(changed.year, changed.month - 1, changed.day))
                    .then((success: boolean) => {
                        if (success) {
                            this.schedule_uplot_update();
                        }
                    });
            } else {
                let key = `${changed.year}-${changed.month}-${changed.day}`;
                let data = subcache[key];

                if (!data) {
                    // got changed event without having this day cached before
                    this.update_wallbox_5min_cache(changed.uid, new Date(changed.year, changed.month - 1, changed.day))
                        .then((success: boolean) => {
                            if (success) {
                                this.schedule_uplot_update();
                            }
                        });
                }
                else {
                    let slot = Math.floor((changed.hour * 60 + changed.minute) / 5);

                    data.update_timestamp = Date.now();
                    data.empty = false;
                    data.flags[slot] = changed.flags;
                    data.power[slot] = changed.power;
                }

                this.schedule_uplot_update();
            }
        });

        util.eventTarget.addEventListener('energy_manager/history_energy_manager_5min_changed', () => {
            let changed = API.get('energy_manager/history_energy_manager_5min_changed');
            let key = `${changed.year}-${changed.month}-${changed.day}`;
            let data = this.energy_manager_5min_cache[key];

            if (!data) {
                // got changed event without having this day cached before
                this.update_energy_manager_5min_cache(new Date(changed.year, changed.month - 1, changed.day))
                    .then((success: boolean) => {
                        if (success) {
                            this.schedule_uplot_update();
                        }
                    });
            } else {
                let slot = Math.floor((changed.hour * 60 + changed.minute) / 5);

                data.update_timestamp = Date.now();
                data.empty = false;
                data.flags[slot] = changed.flags;
                data.power_grid[slot] = changed.power_grid;
                data.power_general[slot] = changed.power_general;

                if (data.power_grid[slot] !== null) {
                    data.power_grid_empty = false;
                }

                this.schedule_uplot_update();
            }
        });
    }

    date_to_5min_key(date: Date) {
        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;
        let day: number = date.getDate();

        return`${year}-${month}-${day}`;
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

    update_uplot_5min_cache(date: Date) {
        let key = this.date_to_5min_key(date);
        let uplot_data = this.uplot_5min_cache[key];
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

        let slot_count = 288;
        let timestamps: number[] = new Array(slot_count);
        let base = date.getTime() / 1000;

        for (let slot = 0; slot < slot_count; ++slot) {
            timestamps[slot] = base + slot * 300;
        }

        uplot_data = {update_timestamp: now, use_timestamp: now, keys: [null], names: [null], values: [timestamps], stacked: [false]};

        let energy_manager_data = this.energy_manager_5min_cache[key];

        if (energy_manager_data && !energy_manager_data.empty) {
            uplot_data.keys.push('em');
            uplot_data.names.push(__("em_energy_analysis.script.grid_connection"));
            uplot_data.values.push(energy_manager_data.power_grid);
            uplot_data.stacked.push(false);
        }

        for (let charger of this.chargers) {
            if (this.wallbox_5min_cache[charger.uid]) {
                let wallbox_data = this.wallbox_5min_cache[charger.uid][key];

                if (wallbox_data && !wallbox_data.empty) {
                    uplot_data.keys.push('wb' + charger.uid);
                    uplot_data.names.push(charger.name);
                    uplot_data.values.push(wallbox_data.power);
                    uplot_data.stacked.push(true);
                }
            }
        }

        this.uplot_5min_cache[key] = uplot_data;
        this.expire_cache(this.uplot_5min_cache);
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

        let slot_count = 288;
        let timestamps: number[] = new Array(slot_count);
        let base = date.getTime() / 1000;

        for (let slot = 0; slot < slot_count; ++slot) {
            timestamps[slot] = base + slot * 300;
        }

        uplot_data = {update_timestamp: now, use_timestamp: now, keys: [null], names: [null], values: [timestamps], stacked: [false]};

        let energy_manager_data = this.energy_manager_5min_cache[key];

        if (energy_manager_data && !energy_manager_data.power_grid_empty) {
            uplot_data.keys.push('em');
            uplot_data.names.push(null);
            uplot_data.values.push(energy_manager_data.power_grid);
            uplot_data.stacked.push(false);
        }

        this.uplot_5min_status_cache[key] = uplot_data;
        this.expire_cache(this.uplot_5min_status_cache);
    }

    async update_wallbox_5min_cache_all(date: Date) {
        let all: Promise<boolean>[] = [];

        for (let charger of this.chargers) {
            all.push(this.update_wallbox_5min_cache(charger.uid, date));
        }

        let result = await Promise<boolean[]>.all(all);

        for (let success of result) {
            if (!success) {
                return false;
            }
        }

        return true;
    }

    async update_wallbox_5min_cache(uid: number, date: Date) {
        if (date.getTime() > Date.now()) {
            return true;
        }

        let key = this.date_to_5min_key(date);

        if (this.wallbox_5min_cache[uid] && this.wallbox_5min_cache[uid][key]) {
            // cache is valid
            this.wallbox_5min_cache[uid][key].use_timestamp = Date.now();
            return true;
        }

        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;
        let day: number = date.getDate();
        let response: string = '';

        try {
            response = await (await util.put('energy_manager/history_wallbox_5min', {uid: uid, year: year, month: month, day: day})).text();
        } catch (e) {
            console.log('Could not get wallbox 5min data: ' + e);
            return false;
        }

        let payload = JSON.parse(response);
        let slot_count = 288;
        let now = Date.now();
        let data: Wallbox5minData = {
            update_timestamp: now,
            use_timestamp: now,
            empty: true,
            flags: new Array(slot_count),
            power: new Array(slot_count),
        };

        for (let slot = 0; slot < slot_count; ++slot) {
            data.flags[slot] = payload[slot * 2];
            data.power[slot] = payload[slot * 2 + 1];

            if ((data.flags[slot] & 0x80 /* no data */) == 0) {
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
        if (date.getTime() > Date.now()) {
            return true;
        }

        let key = this.date_to_5min_key(date);

        if (this.energy_manager_5min_cache[key]) {
            // cache is valid
            this.energy_manager_5min_cache[key].use_timestamp = Date.now();
            return true;
        }

        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;
        let day: number = date.getDate();
        let response: string = '';

        try {
            response = await (await util.put('energy_manager/history_energy_manager_5min', {year: year, month: month, day: day})).text();
        } catch (e) {
            console.log('Could not get energy manager 5min data: ' + e);
            return false;
        }

        let payload = JSON.parse(response);
        let slot_count = 288;
        let now = Date.now();
        let data: EnergyManager5minData = {
            update_timestamp: now,
            use_timestamp: now,
            empty: true,
            flags: new Array(slot_count),
            power_grid: new Array(slot_count),
            power_grid_empty: true,
            power_general: new Array(slot_count)
        };

        for (let slot = 0; slot < 288; ++slot) {
            data.flags[slot] = payload[slot * 8];
            data.power_grid[slot] = payload[slot * 8 + 1];
            data.power_general[slot] = [
                payload[slot * 8 + 2],
                payload[slot * 8 + 3],
                payload[slot * 8 + 4],
                payload[slot * 8 + 5],
                payload[slot * 8 + 6],
                payload[slot * 8 + 7],
            ];

            if ((data.flags[slot] & 0x80 /* no data */) == 0) {
                data.empty = false;
            }

            if (data.power_grid[slot] !== null) {
                data.power_grid_empty = false;
            }
        }

        this.energy_manager_5min_cache[key] = data;
        this.expire_cache(this.energy_manager_5min_cache);

        return true;
    }

    /*async update_wallbox_daily_cache() {
        for (let charger of this.chargers) {
            let year: number = 2023;
            let month: number = 2;
            let id = `${charger.uid}-${year}-${month}`;
            let response: string = '';

            try {
                response = await (await util.put('energy_manager/history_wallbox_daily', {uid: charger.uid, year: year, month: month})).text();
            } catch (e) {
                console.log('Could not get wallbox daily data: ' + e);
            }

            if (response) {
                let payload = JSON.parse(response);
                let length = payload.length;
                let data: WallboxDailyData = {
                    energy: new Array(length),
                };

                for (let i = 0; i < length; ++i) {
                    data.energy[i] = payload[i];
                }

                console.log('energy ' + id + ' ' +  data.energy);

                this.wallbox_daily_cache[id] = data;
            }
        }
    }

    async update_energy_manager_daily_cache() {
        let response: string = '';
        let year: number = 2023;
        let month: number = 2;
        let id = `${year}-${month}`;

        try {
            response = await (await util.put('energy_manager/analysis_energy_manager_daily', {year: year, month: month})).text();
        } catch (e) {
            console.log('Could not get energy manager daily data: ' + e);
        }

        if (response) {
            let payload = JSON.parse(response);
            let length = Math.floor(payload.length / 14);
            let data: EnergyManagerDailyData = {
                energy_grid_in: new Array(length),
                energy_grid_out: new Array(length),
                energy_meter_in_0: new Array(length),
                energy_meter_in_1: new Array(length),
                energy_meter_in_2: new Array(length),
                energy_meter_in_3: new Array(length),
                energy_meter_in_4: new Array(length),
                energy_meter_in_5: new Array(length),
                energy_meter_out_0: new Array(length),
                energy_meter_out_1: new Array(length),
                energy_meter_out_2: new Array(length),
                energy_meter_out_3: new Array(length),
                energy_meter_out_4: new Array(length),
                energy_meter_out_5: new Array(length),
            };

            for (let i = 0; i < length; ++i) {
                data.energy_grid_in[i] = payload[i * 14];
                data.energy_grid_out[i] = payload[i * 14 + 1];
                data.energy_meter_in_0[i] = payload[i * 14 + 2];
                data.energy_meter_in_1[i] = payload[i * 14 + 3];
                data.energy_meter_in_2[i] = payload[i * 14 + 4];
                data.energy_meter_in_3[i] = payload[i * 14 + 5];
                data.energy_meter_in_4[i] = payload[i * 14 + 6];
                data.energy_meter_in_5[i] = payload[i * 14 + 7];
                data.energy_meter_out_0[i] = payload[i * 14 + 8];
                data.energy_meter_out_1[i] = payload[i * 14 + 9];
                data.energy_meter_out_2[i] = payload[i * 14 + 10];
                data.energy_meter_out_3[i] = payload[i * 14 + 11];
                data.energy_meter_out_4[i] = payload[i * 14 + 12];
                data.energy_meter_out_5[i] = payload[i * 14 + 13];
            }

            console.log('energy_grid_in ' + id + ' ' + data.energy_grid_in);
            console.log('energy_grid_out ' + id + ' ' + data.energy_grid_out);
            console.log('energy_meter_in_0 ' + id + ' ' + data.energy_meter_in_0);
            console.log('energy_meter_in_1 ' + id + ' ' + data.energy_meter_in_1);
            console.log('energy_meter_in_2 ' + id + ' ' + data.energy_meter_in_2);
            console.log('energy_meter_in_3 ' + id + ' ' + data.energy_meter_in_3);
            console.log('energy_meter_in_4 ' + id + ' ' + data.energy_meter_in_4);
            console.log('energy_meter_in_5 ' + id + ' ' + data.energy_meter_in_5);
            console.log('energy_meter_out_0 ' + id + ' ' + data.energy_meter_out_0);
            console.log('energy_meter_out_1 ' + id + ' ' + data.energy_meter_out_1);
            console.log('energy_meter_out_2 ' + id + ' ' + data.energy_meter_out_2);
            console.log('energy_meter_out_3 ' + id + ' ' + data.energy_meter_out_3);
            console.log('energy_meter_out_4 ' + id + ' ' + data.energy_meter_out_4);
            console.log('energy_meter_out_5 ' + id + ' ' + data.energy_meter_out_5);

            this.energy_manager_daily_cache[id] = data;
        }
    }*/

    set_current_5min_date(date: Date) {
        this.setState({current_5min_date: date}, () => {
            this.update_current_5min_cache();
        });
    }

    reload_wallbox_cache() {
        if (this.uplot_wrapper_ref.current) {
            this.uplot_wrapper_ref.current.set_loading();
        }

        this.wallbox_5min_cache = {};

        this.update_wallbox_5min_cache_all(this.state.current_5min_date)
            .then((success: boolean) => {
                if (!success) {
                    window.setTimeout(() => {
                        this.reload_wallbox_cache();
                    }, 100);

                    return;
                }

                this.update_uplot();
            });

        // FIXME: reload daily cache as well
    }

    update_current_5min_cache() {
        if (this.uplot_wrapper_ref.current) {
            this.uplot_wrapper_ref.current.set_loading();
        }

        this.update_wallbox_5min_cache_all(this.state.current_5min_date)
            .then((success: boolean) => {
                if (!success) {
                    return Promise.resolve(false);
                }

                return this.update_energy_manager_5min_cache(this.state.current_5min_date);
            })
            .then((success: boolean) => {
                if (!success) {
                    window.setTimeout(() => {
                        this.update_current_5min_cache();
                    }, 100);

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
        if (this.uplot_wrapper_ref.current) {
            this.update_uplot_5min_cache(this.state.current_5min_date);

            let key = this.date_to_5min_key(this.state.current_5min_date);
            let data = this.uplot_5min_cache[key];

            this.uplot_wrapper_ref.current.set_data(data);
        }

        if (this.status_ref.current && this.status_ref.current.uplot_wrapper_ref.current) {
            let status_date: Date = new Date();

            status_date.setHours(0);
            status_date.setMinutes(0);
            status_date.setSeconds(0);
            status_date.setMilliseconds(0);

            this.update_uplot_5min_status_cache(status_date);

            let key = this.date_to_5min_key(status_date);
            let data = this.uplot_5min_status_cache[key];

            this.status_ref.current.uplot_wrapper_ref.current.set_data(data);
        }
    }

    render(props: {}, state: Readonly<EMEnergyAnalysisState>) {
        if (!util.allow_render) {
            return (<></>);
        }

        return (
            <>
                <PageHeader title={__("em_energy_analysis.content.em_energy_analysis")} colClasses="col-xl-10"/>
                <div class="row">
                    <div class="col-xl-10 mb-3">
                        <UplotWrapper ref={this.uplot_wrapper_ref}
                                      id="em_energy_analysis_chart"
                                      class="em-energy-analysis-chart"
                                      sidebar_id="em-energy-analysis"
                                      y_min={0}
                                      y_max={100}
                                      y_step={10}
                                      marker_width_reduction={30} />
                    </div>
                </div>
                <FormRow label={__("em_energy_analysis.content.date")} labelColClasses="col-lg-3 col-xl-3" contentColClasses="col-lg-9 col-xl-7">
                    <InputDate date={state.current_5min_date} onDate={this.set_current_5min_date.bind(this)} buttons="day"/>
                </FormRow>
            </>
        )
    }
}

let status_ref = createRef();

render(<EMEnergyAnalysisStatusChart ref={status_ref} />, $('#status_em_energy_analysis_status_chart_container')[0]);

render(<EMEnergyAnalysis status_ref={status_ref} />, $('#em-energy-analysis')[0]);

function update_meter_values() {
    let values = API.get('meter/values');

    $('#status_em_energy_analysis_status_grid_connection_power').val(util.toLocaleFixed(values.power, 0) + " W");
}

export function init() {

}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('meter/values', update_meter_values);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-em-energy-analysis').prop('hidden', !module_init.energy_manager);
}
