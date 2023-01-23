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

import { h, render, Fragment, Component, ComponentChild } from "preact";
import { __ } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";

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

interface ChartExtra {
    samples: number[];
    tooltip_titles: string[];
    grid_ticks: string[];
    grid_colors: string[];
}

interface EMEnergyAnalysisState {
    chart_extra: ChartExtra;
}

function build_chart_data(chart_extra: ChartExtra) {
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

function build_chart_options(chart_extra: ChartExtra, chart_container_id: string) {
    let options: ChartOptions<"line"> = {
        normalized: true,
        animation: false,
        onResize: function(chart, size) {
            let element = document.getElementById(chart_container_id);
            chart.options.aspectRatio = parseFloat(getComputedStyle(element).aspectRatio);
        },
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
                        return " " + context.formattedValue + " Watt"; // FIXME
                    }
                }
            }
        },
        scales: {
            x: {
                title: {
                    display: true,
                    text: "Hour", // FIXME
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
                    text: "Value", // FIXME
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

export class EMEnergyAnalysis extends Component<{}, EMEnergyAnalysisState> {
    constructor() {
        super();

        this.state = {
            chart_extra: {
                samples: [1, 2, 3],
                tooltip_titles: ["a!", "b!", "c!"],
                grid_ticks: ["a", "b", "c"],
                grid_colors: ["rgba(0,0,0,0.1)", "rgba(0,0,0,0.1)", "rgba(0,0,0,0.1)"],
            }
        } as any;
    }

    render(props: {}, state: Readonly<EMEnergyAnalysisState>) {
        if (!state || !state.chart_extra) {
            return (<></>);
        }

        let data = build_chart_data(state.chart_extra);
        let options = build_chart_options(state.chart_extra, "em_energy_analysis_chart");

        return (
            <>
                <PageHeader title={__("em_energy_analysis.content.em_energy_analysis")} colClasses="col-xl-10"/>
                <div class="row">
                    <div class="col-xl-10">
                        <div id="em_energy_analysis_chart" class="em-energy-analysis-chart">
                            <Line data={data} options={options} />
                        </div>
                    </div>
                </div>
            </>
        )
    }
}

render(<EMEnergyAnalysis />, $('#em-energy-analysis')[0]);

export function init() {

}

export function add_event_listeners(source: API.APIEventTarget) {

}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-em-energy-analysis').prop('hidden', !module_init.energy_manager);
}
