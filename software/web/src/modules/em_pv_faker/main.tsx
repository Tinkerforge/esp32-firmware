/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

import * as API  from "../../ts/api";
import * as util from "../../ts/util";
import { __ }    from "../../ts/translation";

import { h, render, Fragment, Component } from "preact";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm      } from "../../ts/components/config_form";
import { FormRow         } from "../../ts/components/form_row";
import { InputFloat      } from "../../ts/components/input_float";
import { InputNumber     } from "../../ts/components/input_number";
import { InputText       } from "../../ts/components/input_text";
import { OutputFloat     } from "../../ts/components/output_float";
import { Switch          } from "../../ts/components/switch";

export class EmPvFakerStatus extends Component {
    render() {
        if (!util.render_allowed())
            return <></>

        let state  = API.get('em_pv_faker/state');
        let config = API.get('em_pv_faker/config');

        return <>
            {config.auto_fake ?
                <FormRow label={__("em_pv_faker.status.illuminance")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                    <OutputFloat value={state.illuminance} digits={0} scale={0} unit={'lx'} />
                </FormRow>
            :
                null
            }
            <FormRow label={__("em_pv_faker.status.fake_power")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                <OutputFloat value={state.fake_power} digits={3} scale={3} unit={'kW'} />
            </FormRow>
        </>
    }
}

render(<EmPvFakerStatus/>, $('#status-em_pv_faker')[0])


export class EmPvFaker extends ConfigComponent<'em_pv_faker/config', {}, API.getType['em_pv_faker/runtime_config']> {
    constructor() {
        super('em_pv_faker/config',
            __("energy_manager.script.save_failed"));

        util.addApiEventListener('em_pv_faker/runtime_config', () => {
            this.setState({...API.get('em_pv_faker/runtime_config')});
        });
    }

    override async sendSave(t: "em_pv_faker/config", cfg: API.getType['em_pv_faker/config']) {
        // Must save config before runtime config. Otherwise, the runtime config will be ignored.
        await super.sendSave(t, cfg);

        if (!this.state.auto_fake) {
            await API.save('em_pv_faker/runtime_config', {
                    manual_power: this.state.manual_power,
                }, __("em_pv_faker.script.save_failed"));
        }
    }

    render(props: {}, s: Readonly<API.getType['em_pv_faker/config'] & API.getType['em_pv_faker/runtime_config']>) {
        if (!util.render_allowed())
            return <></>

        return <>
            <ConfigForm id="em_pv_faker_config_form" title={__("em_pv_faker.content.page_header")} isModified={this.isModified()} onSave={this.save} onReset={this.reset} onDirtyChange={(d) => this.ignore_updates = d}>

                <FormRow label={__("em_pv_faker.content.auto_fake")}>
                    <Switch desc=""
                        checked={s.auto_fake}
                        onClick={this.toggle('auto_fake')}
                    />
                </FormRow>

                {s.auto_fake ? <>
                    <FormRow label={__("em_pv_faker.content.illuminance_topic")}>
                        <InputText required
                            maxLength={128}
                            pattern="^[^#+$][^#+]*"
                            value={s.topic}
                            onValue={this.set('topic')}
                            invalidFeedback={__("em_pv_faker.content.illuminance_topic_invalid")}
                        />
                    </FormRow>

                    <FormRow label={__("em_pv_faker.content.peak_power")}>
                        <InputFloat
                            unit="kW"
                            value={s.peak_power}
                            onValue={this.set('peak_power')}
                            digits={3}
                            min={1000}
                            max={100000}
                        />
                    </FormRow>

                    <FormRow label={__("em_pv_faker.content.zero_at_lux")}>
                        <InputFloat
                            unit="klx"
                            value={s.zero_at_lux}
                            onValue={this.set("zero_at_lux")}
                            digits={3}
                            min={1}
                            max={200000}
                        />
                    </FormRow>

                    <FormRow label={__("em_pv_faker.content.peak_at_lux")}>
                        <InputFloat
                            unit="klx"
                            value={s.peak_at_lux}
                            onValue={this.set("peak_at_lux")}
                            digits={3}
                            min={2}
                            max={200000}
                        />
                    </FormRow>

                    <FormRow label={__("em_pv_faker.content.filter_time_constant")}>
                        <InputNumber
                            unit={"s"}
                            value={s.filter_time_constant}
                            onValue={this.set('filter_time_constant')}
                            min={0}
                            max={600}
                        />
                    </FormRow>
                </> : <>
                    <FormRow label={__("em_pv_faker.content.manual_power")}>
                        <InputFloat
                            unit="kW"
                            value={s.manual_power}
                            onValue={this.set('manual_power')}
                            digits={3}
                            min={-50000}
                            max={ 50000}
                            showMinMax
                        />
                    </FormRow>
                </>}

            </ConfigForm>
        </>
    }
}

render(<EmPvFaker/>, $('#em_pv_faker')[0])

export function init() {}
export function add_event_listeners(source: API.APIEventTarget) {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-em_pv_faker').prop('hidden', !module_init.em_pv_faker);
}
