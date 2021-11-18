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

import $ from "jquery";

import * as util from "../util";

declare function __(s: string): string;

import Chartist = require("chartist");
import ctAxisTitle = require("chartist-plugin-axistitle");

// Duplicated in evse/main.ts
interface MeterState {
    power: number,
    energy_rel: number,
    energy_abs: number,
    phases_active: boolean[]
    phases_connected: boolean[]
}

function update_meter_state(state: MeterState) {
    $('#status_meter_power').val(util.toLocaleFixed(state.power, 0) + " W");
    $('#meter_power').val(util.toLocaleFixed(state.power, 0) + " W");

    $('#meter_energy_rel').val(util.toLocaleFixed(state.energy_rel, 3) + " kWh");

    $('#meter_energy_abs').val(util.toLocaleFixed(state.energy_abs, 3) + " kWh");


    if(state.phases_active === undefined)
        return;

    for(let i = 0; i < 3; ++i) {
        util.update_button_group(`btn_group_meter_phase_active_${i}`, state.phases_active[i] ? 0 : 1);
        util.update_button_group(`btn_group_meter_phase_connected_${i}`, state.phases_connected[i] ? 0 : 1);
    }
}


let graph_update_interval: number = null;
let status_interval: number = null;

function update_live_meter() {
    $.get("/meter/live").done(function (result) {
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
    });
}

function update_history_meter() {
    $.get("/meter/history").done(function (values: Number[]) {
        const HISTORY_MINUTE_INTERVAL = 4;
        const VALUE_COUNT = 48 * (60 / HISTORY_MINUTE_INTERVAL);
        const LABEL_COUNT = 9;
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
    });
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
    meter_chart = new Chartist.Line('#meter_chart', <any>data, {
        fullWidth: true,
        showPoint: false,
        axisX: {
            offset: 50,
            labelOffset: {x: 0, y: 5}
        },
        axisY: {
            scaleMinSpace: 40,
            onlyInteger: true,
            offset: 50,
            labelOffset: {x: 0, y: 6}
        },
        plugins: [
            ctAxisTitle({
                axisX: {
                axisTitle: __("sdm72dm.script.time"),
                axisClass: "ct-axis-title",
                offset: {
                    x: 0,
                    y: 40
                },
                textAnchor: "middle"
                },
                axisY: {
                axisTitle: __("sdm72dm.script.power"),
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

    let input = <HTMLInputElement>document.getElementById("meter_chart_time_select");
    meter_chart_change_time(input.value);
}

let allowed_current = 32000;

function update_status_chart() {
    $.get("/meter/history").done(function (values: number[]) {
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

        // We have to assume three phases even if only one or two are connected,
        // because the other phases could be disconnected only temporarily.
        // The only case where scaling to one phase would be useful is when
        // the wallbox is always connected with one phase only, however
        // we can't reliably detect this case.
        // Maybe in the future (as in "as soon as the first customer complains about this" ;) )
        // we could add a configuration for this.
        let configured_max = allowed_current / 1000 * 3 * 230;
        init_status_chart(0, Math.max(configured_max, Math.max(...values))),
        status_meter_chart.update(data);
    });
}

function init_status_chart(min_value=0, max_value=0) {
    let data = {};

    // Create a new line chart object where as first parameter we pass in a selector
    // that is resolving to our chart container element. The Second parameter
    // is the actual data object.
    status_meter_chart = new Chartist.Line('#status_meter_chart', <any>data, {
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
                axisTitle: __("sdm72dm.script.time"),
                axisClass: "ct-axis-title",
                offset: {
                    x: 0,
                    y: 40
                },
                textAnchor: "middle"
                },
                axisY: {
                axisTitle: __("sdm72dm.script.power"),
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

function energy_meter_reset_statistics() {
    $.ajax({
        url: '/meter/reset',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(null)
    });
}

interface DetailedViewEntry {
    i: number,
    name: string,
    desc: string,
    three_phase: boolean,
    unit: string
}

function entry(name: string, three_phase: boolean, unit: string) : DetailedViewEntry {
    return {i: 0, name: __(`sdm72dm.content.detailed_${name}`), desc: __(`sdm72dm.content.detailed_${name}_desc`), three_phase: three_phase, unit: unit}
}

function entry_to_string(e: DetailedViewEntry) : string {
    if (e.three_phase) {
        return `
        <div class="form-group row">
            <div class="col-lg-3">
                <label for="sdm72dm_dv_${e.i}" class="col-form-label">
                    <span class="form-label pr-2">${e.name}</span>
                    <span class="text-muted">${e.desc}</span>
                </label>
            </div>
            <div class="col-lg-9 row pr-0">
                <div class="mb-1 col-12 col-sm-4 pr-xs-only-0">
                    <input id="sdm72dm_dv_${e.i + 0}" class="form-control" type="text" placeholder="..." readonly>
                </div>
                <div class="mb-1 col-12 col-sm-4 pr-xs-only-0">
                    <input id="sdm72dm_dv_${e.i + 1}" class="form-control" type="text" placeholder="..." readonly>
                </div>
                <div class="mb-1 col-12 col-sm-4 pr-0">
                    <input id="sdm72dm_dv_${e.i + 2}" class="form-control" type="text" placeholder="..." readonly>
                </div>
            </div>
        </div>`;
    } else {
        return `
        <div class="form-group row">
            <div class="col-lg-3">
                <label for="sdm72dm_dv_${e.i}" class="col-form-label">
                    <span class="form-label pr-2">${e.name}</span>
                    <span class="text-muted">${e.desc}</span>
                </label>
            </div>
            <div class="col-lg-9">
                <input id="sdm72dm_dv_${e.i}" class="form-control" type="text" placeholder="..." readonly>
            </div>
        </div>
        `;
    }
}

let entries: DetailedViewEntry[];

function build_evse_v2_detailed_values_view() {
    let container = $('#sdm72dm_detailed_values');
    container.empty();
    entries = [
        entry("line_to_neutral_volts",             true, "V"),
        entry("current",                           true, "A"),
        entry("power",                             true, "W"),
        entry("volt_amps",                         true, "VA"),
        entry("volt_amps_reactive",                true, "var"),
        entry("power_factor",                      true, ""),
        entry("phase_angle",                       true, "°"),
        entry("average_line_to_neutral_volts",     false, "V"),
        entry("average_line_current",              false, "A"),
        entry("sum_of_line_currents",              false, "A"),
        entry("total_system_power",                false, "W"),
        entry("total_system_volt_amps",            false, "VA"),
        entry("total_system_var",                  false, "var"),
        entry("total_system_power_factor",         false, ""),
        entry("total_system_phase_angle",          false, "°"),
        entry("frequency_of_supply_voltages",      false, "Hz"),
        entry("total_import_kwh",                  false, "kWh"),
        entry("total_export_kwh",                  false, "kWh"),
        entry("total_import_kvarh",                false, "kvarh"),
        entry("total_export_kvarh",                false, "kvarh"),
        entry("total_vah",                         false, "kVAh"),
        entry("ah",                                false, "Ah"),
        entry("total_system_power_demand",         false, "W"),
        entry("maximum_total_system_power_demand", false, "W"),
        entry("total_system_va_demand",            false, "VA"),
        entry("maximum_total_system_va_demand",    false, "VA"),
        entry("neutral_current_demand",            false, "A"),
        entry("maximum_neutral_current_demand",    false, "A"),
        entry("line1_to_line2_volts",              false, "V"),
        entry("line2_to_line3_volts",              false, "V"),
        entry("line3_to_line1_volts",              false, "V"),
        entry("average_line_to_line_volts",        false, "V"),
        entry("neutral_current",                   false, "A"),
        entry("ln_volts_thd",                      true, "%"),
        entry("current_thd",                       true, "%"),
        entry("average_line_to_neutral_volts_thd", false, "%"),
        entry("average_line_current_thd",          false, "%"),
        entry("current_demand",                    true, "A"),
        entry("maximum_current_demand",            true, "A"),
        entry("line1_to_line2_volts_thd",          false, "%"),
        entry("line2_to_line3_volts_thd",          false, "%"),
        entry("line3_to_line1_volts_thd",          false, "%"),
        entry("average_line_to_line_volts_thd",    false, "%"),
        entry("total_kwh_sum",                     false, "kWh"),
        entry("total_kvarh_sum",                   false, "kvarh"),
        entry("import_kwh",                        true, "kWh"),
        entry("export_kwh",                        true, "kWh"),
        entry("total_kwh",                         true, "kWh"),
        entry("import_kvarh",                      true, "kvarh"),
        entry("export_kvarh",                      true, "kvarh"),
        entry("total_kvarh",                       true, "kvarh")
    ];

    let i = 0;
    for(let e of entries) {
        e.i = i;
        i += e.three_phase ? 3 : 1;

        container.append(entry_to_string(e));
    }
}


function update_evse_v2_detailed_values(v: number[]) {
    let entry_idx = 0;
    let subentry_idx = 0;
    for(let i = 0; i < v.length; ++i) {
        $(`#sdm72dm_dv_${i}`).val(util.toLocaleFixed(v[i], 1) + " " + entries[entry_idx].unit);
        ++subentry_idx;
        if (subentry_idx == (entries[entry_idx].three_phase ? 3 : 1)) {
            entry_idx += 1
            subentry_idx = 0;
        }
    }
}

export function init() {
    let input = <HTMLInputElement>document.getElementById("meter_chart_time_select");
    input.addEventListener(
        "change", () => meter_chart_change_time(input.value)
    );

    (<HTMLButtonElement>document.getElementById("energy_meter_reset_statistics_button")).addEventListener("click", energy_meter_reset_statistics);

    // The energy meter tab layout is generated when it is shown first.
    // We have to create the chart then, to make sure it is scaled correctly.
    // Immediately deregister afterwards, as we don't want to recreate the chart
    // every time.
    $('#sidebar-meter').on('shown.bs.tab', function (e) {
        init_chart();
    });

    $('#sidebar-meter').on('hidden.bs.tab', function (e) {
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

export function addEventListeners(source: EventSource) {
    source.addEventListener('meter/state', function (e: util.SSE) {
        update_meter_state(<MeterState>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('meter/detailed_values', function (e: util.SSE) {
        update_evse_v2_detailed_values(<number[]>(JSON.parse(e.data)));
    }, false);

    source.addEventListener("evse/max_charging_current", function (e: util.SSE) {
        let parsed = JSON.parse(e.data);
        allowed_current = Math.min(parsed.max_current_incoming_cable, parsed.max_current_outgoing_cable);
    }, false);
}

export function updateLockState(module_init: any) {
    let module_available = module_init.sdm72dm || module_init.evse_v2_meter;
    $('#sidebar-meter').prop('hidden', !module_available);
    $('#status-meter').prop('hidden', !module_available);

    if(!module_available) {
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

    if (module_init.evse_v2_meter)
        build_evse_v2_detailed_values_view();

    $('#meter_phases_active').prop('hidden', !module_init.evse_v2_meter);
    $('#meter_phases_connected').prop('hidden', !module_init.evse_v2_meter);
    $('#meter_detailed_values_container').prop('hidden', !module_init.evse_v2_meter);
}
