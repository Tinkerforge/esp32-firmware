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

import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  ChartData,
  ChartOptions,
} from 'chart.js';
import { Line } from 'react-chartjs-2';

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);

interface LineProps {
  options: ChartOptions<"line">;
  data: ChartData<"line">;
}

import { h, render, Fragment, Component, ComponentChild } from "preact";
import { PageHeader } from "../../ts/components/page_header";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { Button } from "react-bootstrap";
import { InputSelect } from "src/ts/components/input_select";
import { CollapsedSection } from "src/ts/components/collapsed_section";
import { OutputFloat } from "src/ts/components/output_float";
import { Zap, ZapOff } from "react-feather";

interface DetailedViewEntry {
    i: number,
    name: string,
    desc: string,
    three_phase: boolean,
    unit: string,
    sdm630_only: boolean
    digits: 0|1|2|3
}

function entry(name: string, three_phase: boolean, unit: string, sdm630_only: boolean, digits: 0|1|2|3) : DetailedViewEntry {
    return {i: 0, name: translate_unchecked(`meter.content.detailed_${name}`), desc: translate_unchecked(`meter.content.detailed_${name}_desc`), three_phase: three_phase, unit: unit, sdm630_only: sdm630_only, digits: digits}
}

const entries: DetailedViewEntry[] = [
    entry("line_to_neutral_volts",             true,  "V",     false, 1),
    entry("current",                           true,  "A",     false, 3),
    entry("power",                             true,  "W",     false, 0),
    entry("volt_amps",                         true,  "VA",    false, 0),
    entry("volt_amps_reactive",                true,  "var",   false, 0),
    entry("power_factor",                      true,  "",      false, 3),
    entry("phase_angle",                       true,  "°",     true,  1),
    entry("average_line_to_neutral_volts",     false, "V",     true,  1),
    entry("average_line_current",              false, "A",     false, 3),
    entry("sum_of_line_currents",              false, "A",     false, 3),
    entry("total_system_power",                false, "W",     false, 0),
    entry("total_system_volt_amps",            false, "VA",    false, 0),
    entry("total_system_var",                  false, "var",   false, 0),
    entry("total_system_power_factor",         false, "",      false, 3),
    entry("total_system_phase_angle",          false, "°",     true,  1),
    entry("frequency_of_supply_voltages",      false, "Hz",    false, 3),
    entry("total_import_kwh",                  false, "kWh",   false, 3),
    entry("total_export_kwh",                  false, "kWh",   false, 3),
    entry("total_import_kvarh",                false, "kvarh", true,  3),
    entry("total_export_kvarh",                false, "kvarh", true,  3),
    entry("total_vah",                         false, "kVAh",  true,  3),
    entry("ah",                                false, "Ah",    true,  3),
    entry("total_system_power_demand",         false, "W",     true,  0),
    entry("maximum_total_system_power_demand", false, "W",     true,  0),
    entry("total_system_va_demand",            false, "VA",    true,  0),
    entry("maximum_total_system_va_demand",    false, "VA",    true,  0),
    entry("neutral_current_demand",            false, "A",     true,  3),
    entry("maximum_neutral_current_demand",    false, "A",     true,  3),
    entry("line1_to_line2_volts",              false, "V",     false, 1),
    entry("line2_to_line3_volts",              false, "V",     false, 1),
    entry("line3_to_line1_volts",              false, "V",     false, 1),
    entry("average_line_to_line_volts",        false, "V",     false, 1),
    entry("neutral_current",                   false, "A",     false, 3),
    entry("ln_volts_thd",                      true,  "%",     true,  1),
    entry("current_thd",                       true,  "%",     true,  1),
    entry("average_line_to_neutral_volts_thd", false, "%",     true,  1),
    entry("average_line_current_thd",          false, "%",     true,  1),
    entry("current_demand",                    true,  "A",     true,  3),
    entry("maximum_current_demand",            true,  "A",     true,  3),
    entry("line1_to_line2_volts_thd",          false, "%",     true,  1),
    entry("line2_to_line3_volts_thd",          false, "%",     true,  1),
    entry("line3_to_line1_volts_thd",          false, "%",     true,  1),
    entry("average_line_to_line_volts_thd",    false, "%",     true,  1),
    entry("total_kwh_sum",                     false, "kWh",   false, 3),
    entry("total_kvarh_sum",                   false, "kvarh", false, 3),
    entry("import_kwh",                        true,  "kWh",   true,  3),
    entry("export_kwh",                        true,  "kWh",   true,  3),
    entry("total_kwh",                         true,  "kWh",   true,  3),
    entry("import_kvarh",                      true,  "kvarh", true,  3),
    entry("export_kvarh",                      true,  "kvarh", true,  3),
    entry("total_kvarh",                       true,  "kvarh", true,  3)
];

interface LiveExtra {
    samples: number[];
    tooltip_titles: string[];
    grid_ticks: string[];
    grid_colors: string[];
    last_minute: number;
}

interface HistoryExtra {
    samples: number[];
    tooltip_titles: string[];
    grid_ticks: string[];
    grid_colors: string[];
}

interface MeterState {
    state: Readonly<API.getType['meter/state']>;
    values: Readonly<API.getType['meter/values']>;
    phases: Readonly<API.getType['meter/phases']>;
    all_values: Readonly<API.getType['meter/all_values']>;
    live_extra: LiveExtra;
    history_extra: HistoryExtra;
    chart_selected: "history"|"live";
}

interface StatusMeterChartState {
    history_extra: HistoryExtra;
}

function calculate_live_extra(samples_per_second: number, samples: number[], last_minute: number): LiveExtra {
    let extra: LiveExtra = {samples: samples, tooltip_titles: [], grid_ticks: [], grid_colors: [], last_minute: last_minute};
    let now = Date.now();
    let start = now - 1000 * samples.length / samples_per_second;

    for(let i = 0; i < samples.length; ++i) {
        let d = new Date(start + i * (1000 * (1 / samples_per_second)));
        extra.tooltip_titles[i] = d.toLocaleTimeString(navigator.language, {hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false});

        if(d.getSeconds() == 0 && d.getMinutes() != extra.last_minute) {
            extra.grid_ticks[i] = d.toLocaleTimeString(navigator.language, {hour: '2-digit', minute: '2-digit', hour12: false});
            extra.grid_colors[i] = "rgba(0,0,0,0.1)";
            extra.last_minute = d.getMinutes();
        }
        else {
            extra.grid_ticks[i] = "";
            extra.grid_colors[i] = "rgba(0,0,0,0)";
        }
    }

    return extra;
}

function calculate_history_extra(offset: number, samples: Readonly<number[]>): HistoryExtra {
    const HISTORY_MINUTE_INTERVAL = 4;
    const VALUE_COUNT = 48 * (60 / HISTORY_MINUTE_INTERVAL);
    const LABEL_COUNT = window.innerWidth < 500 ? 5 : 9;
    const VALUES_PER_LABEL = VALUE_COUNT / (LABEL_COUNT - 1); // -1 for the last label that has no value

    let extra: HistoryExtra = {samples: [], tooltip_titles: [], grid_ticks: [], grid_colors: []};
    let now = Date.now();
    let start = now - 1000 * 60 * 60 * 48 - offset;

    for(let i = 0; i < samples.length + 1; ++i) { // +1 for the last label that has no value
        extra.samples[i] = samples[i];

        let d = new Date(start + i * (1000 * 60 * HISTORY_MINUTE_INTERVAL));
        extra.tooltip_titles[i] = d.toLocaleTimeString(navigator.language, {hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false});

        if (i % VALUES_PER_LABEL == 0) {
            extra.grid_ticks[i] = d.toLocaleTimeString(navigator.language, {hour: '2-digit', minute: '2-digit', hour12: false});
            extra.grid_colors[i] = "rgba(0,0,0,0.1)";
        }
        else {
            extra.grid_ticks[i] = "";
            extra.grid_colors[i] = "rgba(0,0,0,0)";
        }
    }

    return extra;
}

function array_append<T>(a: Array<T>, b: Array<T>, tail: number): Array<T> {
    a.push(...b);

    return a.slice(-tail);
}

function build_chart_data(chart_extra: LiveExtra|HistoryExtra) {
    let data: ChartData<"line"> = {
        labels: chart_extra.grid_ticks,
        datasets: [
            {
                data: chart_extra.samples,
                backgroundColor: "#007bff",
                borderColor: "#007bff",
                normalized: true,
            }
        ]
    };

    return data;
}

function build_chart_options(chart_extra: LiveExtra|HistoryExtra) {
    let options: ChartOptions<"line"> = {
        normalized: true,
        animation: false,
        layout: {
            autoPadding: false,
            padding: {
                right: 25,
            }
        },
        elements: {
            point: {
                pointStyle: false,
            }
        },
        plugins: {
            legend: {
                display: false,
            },
            tooltip: {
                intersect: false,
                callbacks: {
                    title: function(context) {
                        return chart_extra.tooltip_titles[context[0].dataIndex];
                    },
                    label: function(context) {
                        return " " + context.formattedValue + " Watt";
                    }
                }
            }
        },
        scales: {
            x: {
                title: {
                    display: true,
                    text: __("meter.script.time"),
                    font: {
                        size: 14,
                    }
                },
                ticks: {
                    autoSkip: false,
                    maxRotation: 0,
                    includeBounds: false,
                    sampleSize: 0,
                },
                grid: {
                    color: chart_extra.grid_colors,
                }
            },
            y: {
                title: {
                    display: true,
                    text: __("meter.script.power"),
                    font: {
                        size: 14,
                    }
                },
                border: {
                    display: false,
                },
                ticks: {
                    autoSkipPadding: 10,
                }
            }
        }
    };

    return options;
}

export class Meter extends Component<{}, MeterState> {
    pending_live: {samples_per_second: number, samples: number[]};

    constructor() {
        super();

        let i = 0;
        for(let e of entries) {
            e.i = i;
            i += e.three_phase ? 3 : 1;
        }

        util.eventTarget.addEventListener("meter/state", () => {
            this.setState({state: API.get("meter/state")});
        });

        util.eventTarget.addEventListener("meter/values", () => {
            this.setState({values: API.get("meter/values")});
        });

        util.eventTarget.addEventListener("meter/phases", () => {
            this.setState({phases: API.get("meter/phases")});
        });

        util.eventTarget.addEventListener("meter/all_values", () => {
            this.setState({all_values: API.get("meter/all_values")});
        });

        util.eventTarget.addEventListener("meter/live", () => {
            let live = API.get("meter/live");
            let live_extra = calculate_live_extra(live.samples_per_second, live.samples, -1);

            this.setState({live_extra: live_extra});
        });

        this.pending_live = { samples_per_second: undefined, samples: []};

        util.eventTarget.addEventListener("meter/live_samples", () => {
            let live = API.get("meter/live_samples");

            this.pending_live.samples_per_second = live.samples_per_second;
            this.pending_live.samples.push(...live.samples);

            if (this.pending_live.samples.length >= 5) {
                let live_extra = calculate_live_extra(this.pending_live.samples_per_second, this.pending_live.samples, this.state.live_extra.last_minute);
                this.pending_live.samples_per_second = undefined;
                this.pending_live.samples = [];

                this.setState({
                    live_extra: {
                        samples: array_append(this.state.live_extra.samples, live_extra.samples, 720),
                        tooltip_titles: array_append(this.state.live_extra.tooltip_titles, live_extra.tooltip_titles, 720),
                        grid_ticks: array_append(this.state.live_extra.grid_ticks, live_extra.grid_ticks, 720),
                        grid_colors: array_append(this.state.live_extra.grid_colors, live_extra.grid_colors, 720),
                        last_minute: live_extra.last_minute,
                    }
                });
            }
        });

        util.eventTarget.addEventListener("meter/history", () => {
            let history = API.get("meter/history");
            let history_extra = calculate_history_extra(history.offset, history.samples);

            this.setState({history_extra: history_extra});
        });

        util.eventTarget.addEventListener("meter/history_samples", () => {
            let history = API.get("meter/history_samples");
            let history_extra = calculate_history_extra(0, array_append(this.state.history_extra.samples, history.samples, 720));

            this.setState({history_extra: history_extra});
        });

        this.state = {
            chart_selected: "history",
        } as any;
    }

    render(props: {}, state: Readonly<MeterState>) {
        if (!state || !state.all_values) {
            return (<></>);
        }

        let chart_extra = state[`${state.chart_selected}_extra`];

        if (!chart_extra) {
            return (<></>);
        }

        let data = build_chart_data(chart_extra);
        let options = build_chart_options(chart_extra);

        options.aspectRatio = 1.5;

        return (
            <>
                <PageHeader title={__("meter.content.energy_meter")} colClasses="col-xl-10"/>
                    <div class="row">
                        <div class="col-lg-6">
                        <div
                            class="d-flex justify-content-between align-items-center pt-3 mb-3 border-bottom">
                            <span class="h3">{__("meter.status.charge_history")}</span>
                            <div>
                                <InputSelect value={this.state.chart_selected} onValue={(v) => {
                                    this.setState({chart_selected: v as any});
                                }}
                                    items={[
                                        ["history", __("meter.content.history")],
                                        ["live", __("meter.content.live")],
                                    ]}/>
                            </div>
                        </div>
                            <Line data={data} options={options} />
                        </div>
                        <div class="col-lg-6 col-xl-4">
                            <FormSeparator heading={__("meter.content.statistics")} colClasses="col"/>
                            <FormRow label={__("meter.content.power")} labelColClasses="col-sm-6" contentColClasses="col-sm-6">
                                <OutputFloat value={this.state.values.power} digits={0} scale={0} unit="W"/>
                            </FormRow>
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
                        entries.filter(e => state.state.type == 2 ? true : !e.sdm630_only).map(e => <FormRow label={e.name} label_muted={e.desc} labelColClasses="col-lg-3 col-xl-3" contentColClasses="col-lg-9 col-xl-7">
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

export class StatusMeterChart extends Component<{}, StatusMeterChartState> {
    constructor() {
        super();

        util.eventTarget.addEventListener("meter/history", () => {
            let history = API.get("meter/history");
            let history_extra = calculate_history_extra(history.offset, history.samples);

            this.setState({history_extra: history_extra});
        });

        util.eventTarget.addEventListener("meter/history_samples", () => {
            let history = API.get("meter/history_samples");
            let history_extra = calculate_history_extra(0, array_append(this.state.history_extra.samples, history.samples, 720));

            this.setState({history_extra: history_extra});
        });
    }

    render(props: {}, state: Readonly<MeterState>) {
        if (!state || !state.history_extra) {
            return (<></>);
        }

        let data = build_chart_data(state.history_extra);
        let options = build_chart_options(state.history_extra);

        options.aspectRatio = 2.5;

        return (
            <>
                <Line data={data} options={options} />
            </>
        )
    }
}

render(<StatusMeterChart />, $('#status_meter_chart')[0]);

let meter_show_navbar = true;

function update_meter_values() {
    let values = API.get('meter/values');

    $('#status_meter_power').val(util.toLocaleFixed(values.power, 0) + " W");
}

export function init() {
}

function update_module_visibility() {
    let have_meter = API.hasFeature('meter');

    // Don't use meter navbar link if the Energy Manager module is loaded.
    $('#sidebar-meter').prop('hidden', !meter_show_navbar || !have_meter);
    $('#status-meter').prop('hidden', !meter_show_navbar || !have_meter);
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('meter/values', update_meter_values);
    /*source.addEventListener('meter/phases', update_meter_phases);
    source.addEventListener('meter/all_values', update_evse_v2_all_values);*/
    source.addEventListener('info/features', update_module_visibility);
}

export function update_sidebar_state(module_init: any) {
    // Don't use meter navbar link if the Energy Manager module is loaded.
    // The energy manager has its own meter configration module and a link
    // to the meter frontend directly in the configuration module instead of the navbar.
    meter_show_navbar = !module_init.energy_manager;
}
