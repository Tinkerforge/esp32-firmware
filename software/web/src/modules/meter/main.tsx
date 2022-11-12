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

import Chartist from "../../ts/chartist"
import ctAxisTitle from "../../ts/chartist-plugin-axistitle";


import { h, render, Fragment, Component, ComponentChild} from "preact";
import { PageHeader } from "../../ts/components/page_header";


import { IndicatorGroup } from "../../ts/components/indicator_group";
import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { Button} from "react-bootstrap";
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

interface GraphWrapperProps {
    id: string
    graphClass: string
}

class GraphWrapper extends Component<GraphWrapperProps, {}> {
    override shouldComponentUpdate() {
        return false;
    }

    render(props?: GraphWrapperProps, state?: Readonly<{}>, context?: any): ComponentChild {
        return <div id={props.id} class={"ct-chart " + props.graphClass}></div>
    }
}

interface MeterState {
    state: Readonly<API.getType['meter/state']>;
    values: Readonly<API.getType['meter/values']>;
    phases: Readonly<API.getType['meter/phases']>;
    all_values: Readonly<API.getType['meter/all_values']>;
    graph_selected: "history"|"live";
}

export class Meter extends Component<{}, MeterState> {
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
    }

    render(props: {}, state: Readonly<MeterState>) {
        if (!state || !state.all_values)
            return (<></>);

        return (
            <>
                <PageHeader title={__("meter.content.energy_meter")} colClasses="col-xl-10"/>
                    <div class="row">
                        <div class="col-lg-6">
                        <div
                            class="d-flex justify-content-between align-items-center pt-3 mb-3 border-bottom">
                            <span class="h3">{__("meter.status.charge_history")}</span>
                            <div>
                                <InputSelect value={this.state.graph_selected} onValue={(v) => {
                                    this.setState({graph_selected: v as any});
                                    meter_chart_change_time(v);
                                }}
                                    items={[
                                        ["history", __("meter.content.history")],
                                        ["live", __("meter.content.live")],
                                    ]}/>
                            </div>
                        </div>
                            <GraphWrapper id="meter_chart" graphClass="ct-golden-section"/>
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
                        </div>
                    </div>
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
                    </CollapsedSection>
            </>
        )
    }
}

render(<Meter />, $('#meter')[0]);

let meter_show_navbar = true;

function update_meter_values() {
    let values = API.get('meter/values');

    $('#status_meter_power').val(util.toLocaleFixed(values.power, 0) + " W");
}

let graph_update_interval: number = null;
let status_interval: number = null;

async function update_live_meter() {
    let result = null;
    try{
        result = JSON.parse(await util.download("/meter/live").then(blob => blob.text()));
    } catch {
        return;
    }

    let values = result["samples"];
    let sps = result["samples_per_second"];
    let labels = [];

    let now = Date.now();
    let start = now - 1000 * values.length / sps;
    let last_minute = -1;
    for(let i = 0; i < values.length + 1; ++i) {
        let d = new Date(start + i * (1000 * (1/sps)));
        if(d.getSeconds() == 0 && d.getMinutes() != last_minute) {
            labels[i] = d.toLocaleTimeString(navigator.language, {hour: '2-digit', minute: '2-digit', hour12: false});
            last_minute = d.getMinutes();
        }
        else {
            labels[i] = null;
        }
    }

    let data = {
        labels: labels,
        series: [
            values
        ]
    };
    meter_chart.update(data);
}

async function update_history_meter() {
    let values = null;
    try {
        values = JSON.parse(await util.download("/meter/history").then(blob => blob.text()));
    } catch {
        return;
    }

    const HISTORY_MINUTE_INTERVAL = 4;
    const VALUE_COUNT = 48 * (60 / HISTORY_MINUTE_INTERVAL);
    const LABEL_COUNT = window.innerWidth < 500 ? 5 : 9;
    const VALUES_PER_LABEL = VALUE_COUNT / (LABEL_COUNT - 1); // - 1 for the last label that has no values

    if (values.length != VALUE_COUNT) {
        console.log("Unexpected number of values to plot!");
        return;
    }

    let labels = [];

    let now = Date.now();
    let start = now - 1000 * 60 * 60 * 48;
    for(let i = 0; i < values.length + 1; ++i) {
        if (i % VALUES_PER_LABEL == 0) {
            let d = new Date(start + i * (1000 * 60 * HISTORY_MINUTE_INTERVAL));
            labels[i] = d.toLocaleTimeString(navigator.language, {hour: '2-digit', minute: '2-digit', hour12: false});
        }
        else {
            labels[i] = null;
        }
    }

    let data = {
        labels: labels,
        series: [
            values
        ]
    };
    meter_chart.update(data);
}

function meter_chart_change_time(value: string) {
    if (graph_update_interval != null) {
        clearInterval(graph_update_interval);
        graph_update_interval = null;
    }

    if (value == "live") {
        update_live_meter();
        graph_update_interval = window.setInterval(update_live_meter, 1000);
        return;
    } else if (value == "history") {
        update_history_meter();
        graph_update_interval = window.setInterval(update_history_meter, 10000);
    } else {
        console.log("Unknown plot type!");
    }
}

let meter_chart: Chartist.IChartistLineChart;
let status_meter_chart: Chartist.IChartistLineChart;

function init_chart() {
    let data = {};

    // Create a new line chart object where as first parameter we pass in a selector
    // that is resolving to our chart container element. The Second parameter
    // is the actual data object.
    meter_chart = new Chartist.Line('#meter_chart', data as any, {
        fullWidth: true,
        showPoint: false,
        axisX: {
            offset: 50,
            labelOffset: {x: 0, y: 5}
        },
        axisY: {
            scaleMinSpace: 40,
            onlyInteger: true,
            offset: 60,
            labelOffset: {x: 0, y: 6}
        },
        plugins: [
            ctAxisTitle({
                axisX: {
                axisTitle: __("meter.script.time"),
                axisClass: "ct-axis-title",
                offset: {
                    x: 0,
                    y: 40
                },
                textAnchor: "middle"
                },
                axisY: {
                axisTitle: __("meter.script.power"),
                axisClass: "ct-axis-title",
                offset: {
                    x: 0,
                    y: 12
                },
                flipTitle: true
                }
            })
        ]
    });

    meter_chart_change_time("history");
}

async function update_status_chart() {
    let values = null;
    try {
        values = JSON.parse(await util.download("/meter/history").then(blob => blob.text()));
    } catch {
        return;
    }

    const HISTORY_MINUTE_INTERVAL = 4;
    const VALUE_COUNT = 48 * (60 / HISTORY_MINUTE_INTERVAL);
    const LABEL_COUNT = 5;
    const VALUES_PER_LABEL = VALUE_COUNT / (LABEL_COUNT - 1); // - 1 for the last label that has no values

    if (values.length != VALUE_COUNT) {
        console.log("Unexpected number of values to plot!");
        return;
    }

    let labels = [];

    let now = Date.now();
    let start = now - 1000 * 60 * 60 * 48;
    for(let i = 0; i < values.length + 1; ++i) {
        if (i % VALUES_PER_LABEL == 0) {
            let d = new Date(start + i * (1000 * 60 * HISTORY_MINUTE_INTERVAL));
            labels[i] = d.toLocaleTimeString(navigator.language, {hour: '2-digit', minute: '2-digit', hour12: false});
        }
        else {
            labels[i] = null;
        }
    }

    let data = {
        labels: labels,
        series: [
            values
        ]
    };

    init_status_chart(0, Math.max(6 * 230, Math.max(...values))),
    status_meter_chart.update(data);
}

function init_status_chart(min_value=0, max_value=0) {
    let data = {};

    // Create a new line chart object where as first parameter we pass in a selector
    // that is resolving to our chart container element. The Second parameter
    // is the actual data object.
    status_meter_chart = new Chartist.Line('#status_meter_chart', data as any, {
        fullWidth: true,
        showPoint: false,
        low: min_value,
        high: max_value,
        axisX: {
            offset: 50,
            labelOffset: {x: 0, y: 5}
        },
        axisY: {
            scaleMinSpace: 40,
            onlyInteger: true,
            offset: 60,
            labelOffset: {x: 0, y: 6}
        },
        plugins: [
            ctAxisTitle({
                axisX: {
                axisTitle: __("meter.script.time"),
                axisClass: "ct-axis-title",
                offset: {
                    x: 0,
                    y: 40
                },
                textAnchor: "middle"
                },
                axisY: {
                axisTitle: __("meter.script.power"),
                axisClass: "ct-axis-title",
                offset: {
                    x: -10,
                    y: 15
                },
                flipTitle: true
                }
            })
        ]
    });


}

export function init() {
    // The energy meter tab layout is generated when it is shown first.
    // We have to create the chart then, to make sure it is scaled correctly.
    // Immediately deregister afterwards, as we don't want to recreate the chart
    // every time.
    $('#sidebar-meter, #energy-manager-meter-configuration-sdm630-details').on('shown.bs.tab', function (e) {
        init_chart();
    });

    $('#sidebar-meter, #energy-manager-meter-configuration-sdm630-details').on('hidden.bs.tab', function (e) {
        if (graph_update_interval != null) {
            clearInterval(graph_update_interval);
            graph_update_interval = null;
        }
    });

    $('#sidebar-status').on('shown.bs.tab', function (e) {
        init_status_chart();
        update_status_chart();
    });

    init_status_chart();
    update_status_chart();
    status_interval = window.setInterval(update_status_chart, 60*1000);
}

function update_module_visibility() {
    let have_meter = API.hasFeature('meter');
    let have_phases = API.hasFeature('meter_phases');
    let have_all_values = API.hasFeature('meter_all_values');

    // Don't use meter navbar link if the Energy Manager module is loaded.
    $('#sidebar-meter').prop('hidden', !meter_show_navbar || !have_meter);
    $('#status-meter').prop('hidden', !meter_show_navbar || !have_meter);
/*
    $('#meter_phases_active').prop('hidden', !have_phases);
    $('#meter_phases_connected').prop('hidden', !have_phases);
    $('#meter_detailed_values_container').prop('hidden', !have_all_values);

    if (have_all_values)
        build_evse_v2_detailed_values_view();
*/
    if(!have_meter) {
        if (graph_update_interval != null) {
            clearInterval(graph_update_interval);
            graph_update_interval = null;
        }

        if (status_interval != null) {
            clearInterval(status_interval);
            status_interval = null;
        }
    } else {
        if (status_interval == null) {
            status_interval = window.setInterval(update_status_chart, 60*1000);
        }
    }
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
