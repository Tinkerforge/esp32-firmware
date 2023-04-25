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

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, render, Fragment } from "preact";
import { __ } from "../../ts/translation";
//import { ConfigPageHeader } from "../../ts/components/config_page_header";
//import { Switch } from "src/ts/components/switch";
import { ConfigComponent } from "src/ts/components/config_component";
import { ConfigForm } from "src/ts/components/config_form";
import { FormRow } from "src/ts/components/form_row";
import { InputSelect } from "src/ts/components/input_select";
import { Collapse } from "react-bootstrap";
import { FormSeparator } from "src/ts/components/form_separator";
import { IndicatorGroup } from "src/ts/components/indicator_group";
import { OutputFloat } from "src/ts/components/output_float";

interface EMMeterConfigState {
    meter_state: API.getType['energy_manager/meter_state']
}

export class EMMeterConfig extends ConfigComponent<'energy_manager/meter_config', {}, EMMeterConfigState> {
    constructor() {
        super('energy_manager/meter_config',
              __("em_meter_config.script.save_failed"),
              __("em_meter_config.script.reboot_content_changed"));

        util.addApiEventListener('energy_manager/meter_state', () => {
            this.setState({meter_state: API.get('energy_manager/meter_state')});
        });
    }

    render(props: {}, state: Readonly<API.getType['energy_manager/meter_config'] & EMMeterConfigState>) {
        if (!util.render_allowed() || !API.hasFeature("energy_manager"))
            return <></>

        let meter_state = state.meter_state;
        let local_meter_type;
        switch (meter_state.energy_meter_type) {
            case  0: local_meter_type =  0; break; // None
            case  2: local_meter_type =  1; break; // SDM630
            case  3: local_meter_type =  2; break; // SDM72DMV2
            case  5: local_meter_type =  3; break; // SDM630MCTV2
            default: local_meter_type = -1; // SDM72DM, SDM72CTM, unknown
        }

        return (
            <>
                <ConfigForm id="em_meter_config_config_form" title={__("em_meter_config.content.em_meter_config")} isModified={this.isModified()} onSave={() => this.save()} onReset={this.reset} onDirtyChange={(d) => this.ignore_updates = d}>
                    <FormRow label={__("em_meter_config.content.meter_source")} label_muted={__("em_meter_config.content.meter_source_muted")}>
                        <InputSelect
                            items={[
                                    ["0", __("em_meter_config.content.type_none")],
                                    ["1", __("em_meter_config.content.type_sdm")],
                                    //["2", __("em_meter_config.content.type_sunspec")],
                                    //["3", __("em_meter_config.content.type_modbus_tcp")],
                                    ["100", __("em_meter_config.content.type_custom")],
                                ]}
                            value={state.meter_source}
                            onValue={(v) => this.setState({meter_source: parseInt(v)})}/>
                    </FormRow>

                    <Collapse in={state.meter_source == 1}>
                        <div>
                            <FormSeparator heading={__("em_meter_config.content.conf_sdm")}/>

                            <FormRow label={__("em_meter_config.content.sdm_available")}>
                                <IndicatorGroup
                                    value={local_meter_type}
                                    items={[
                                        ["secondary", __("em_meter_config.content.sdm_none")],
                                        ["primary", __("em_meter_config.content.sdm_630")],
                                        ["primary", __("em_meter_config.content.sdm_72dmv2")],
                                        ["primary", __("em_meter_config.content.sdm_630mctv2")],
                                    ]}/>
                            </FormRow>

                            <FormRow label={__("em_meter_config.content.sdm_power")}>
                                <OutputFloat value={meter_state.energy_meter_power} digits={3} scale={3} unit={'kW'} />
                            </FormRow>

                            <FormRow label={__("em_meter_config.content.sdm_energy_import")} label_muted={__("em_meter_config.content.sdm_energy_import_muted")}>
                                <OutputFloat value={meter_state.energy_meter_energy_import} digits={3} scale={0} unit={'kWh'} />
                            </FormRow>

                            <FormRow label={__("em_meter_config.content.sdm_energy_export")} label_muted={__("em_meter_config.content.sdm_energy_export_muted")}>
                                <OutputFloat value={meter_state.energy_meter_energy_export} digits={3} scale={0} unit={'kWh'} />
                            </FormRow>
                        </div>
                    </Collapse>

                    <Collapse in={state.meter_source == 2}>
                        <div>
                            <FormSeparator heading={__("em_meter_config.content.conf_sunspec")}/>
                            <div>TBD</div>
                        </div>
                    </Collapse>

                    <Collapse in={state.meter_source == 3}>
                        <div>
                            <FormSeparator heading={__("em_meter_config.content.conf_modbus_tcp")}/>
                            <div>TBD</div>
                        </div>
                    </Collapse>
                </ConfigForm>
            </>
        )
    }
}

render(<EMMeterConfig/>, $('#em_meter_config')[0])

export function init() {}

export function add_event_listeners(source: API.APIEventTarget) {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-em_meter_config').prop('hidden', !module_init.energy_manager);
}
