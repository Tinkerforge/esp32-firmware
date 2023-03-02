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

import { h, render, Fragment, Component, ComponentChild } from "preact";
import { __ } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";

interface EMEnergyAnalysisState {

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
        //if (!util.allow_render) {
        //    return (<></>);
        //}
        // TODO Add this back in. It's commented out because otherwise the stuff below won't render because this module doesn't have any event listeners to trigger rendering later.

        return (
            <>
                <PageHeader title={__("em_energy_analysis.content.em_energy_analysis")} colClasses="col-xl-10"/>
                <div class="row">
                    <div class="col-xl-10">
                        <p>{__("em_energy_analysis.content.comming_soon_1")}</p>
                        <p>{__("em_energy_analysis.content.comming_soon_2")}<a href="https://www.warp-charger.com/energy-manager.html">{__("em_energy_analysis.content.comming_soon_3")}</a>{__("em_energy_analysis.content.comming_soon_4")}</p>
                        <p> </p>
                        <div id="em_energy_analysis_chart" class="em-energy-analysis-chart">
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
