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

import { h, render, Fragment, Component} from "preact";
import { __, translate_unchecked } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";


import { IndicatorGroup } from "../../ts/components/indicator_group";
import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { InputText } from "../../ts/components/input_text";
import { InputIndicator } from "../../ts/components/input_indicator";
import { Switch } from "../../ts/components/switch";
import { Button} from "react-bootstrap";
import { CollapsedSection } from "src/ts/components/collapsed_section";
import { EVSE_SLOT_EXTERNAL, EVSE_SLOT_GLOBAL, EVSE_SLOT_GP_INPUT, EVSE_SLOT_SHUTDOWN_INPUT } from "../evse_common/api";
import { InputFile } from "src/ts/components/input_file";
import { DebugLogger } from "../../ts/components/debug_logger"
import { extend } from "jquery";

interface EVSEState {
    state: API.getType['evse/state'];
    ll_state: API.getType['evse/low_level_state'];
    hardware_cfg: API.getType['evse/hardware_configuration'];
    slots: Readonly<API.getType['evse/slots']>;
    user_calibration: API.getType['evse/user_calibration'];
}

interface EVSESettingsState {
    slots: Readonly<API.getType['evse/slots']>;
    boost_mode: API.getType['evse/boost_mode'];
    auto_start_charging: API.getType['evse/auto_start_charging'];
}

let toDisplayCurrent = (x: number) => util.toLocaleFixed(x / 1000.0, 3) + " A"

export class EVSE extends Component<{}, EVSEState> {

    constructor() {
        super();

        util.eventTarget.addEventListener('evse/state', () => {
            this.setState({state: API.get('evse/state')});
        });

        util.eventTarget.addEventListener('evse/low_level_state', () => {
            this.setState({ll_state: API.get('evse/low_level_state')});
        });

        util.eventTarget.addEventListener('evse/hardware_configuration', () => {
            this.setState({hardware_cfg: API.get('evse/hardware_configuration')});
        });

        util.eventTarget.addEventListener('evse/slots', () => {
            this.setState({slots: API.get('evse/slots')});
        });

        util.eventTarget.addEventListener('evse/user_calibration', () => {
            this.setState({user_calibration: API.get('evse/user_calibration')});
        });
    }

    render(props: {}, s: Readonly<EVSEState>) {
        if (!util.allow_render)
            return (<></>);

        let {state,
            ll_state,
            hardware_cfg,
            slots,
            user_calibration} = s;

        let min = Math.min(...slots.filter(s => s.active).map(s => s.max_current));

        return (
            <>
                <PageHeader title={__("evse.content.status")} />
                    <FormRow label={__("evse.content.iec_state")}>
                        <IndicatorGroup
                            value={state.iec61851_state}
                            items={[
                                ["primary", __("evse.content.iec_state_a")],
                                ["warning", __("evse.content.iec_state_b")],
                                ["success", __("evse.content.iec_state_c")],
                                ["danger", __("evse.content.iec_state_d")],
                                ["danger", __("evse.content.iec_state_ef")]
                            ]}/>
                    </FormRow>

                    <FormRow label={__("evse.content.allowed_charging_current")} label_muted={__("evse.content.charging_current_muted")}>
                        <InputText value={toDisplayCurrent(state.allowed_charging_current)}/>
                    </FormRow>

                    <FormRow label={__("evse.content.error_state")} label_muted={__("evse.content.error_state_desc")}>
                        <IndicatorGroup
                            value={state.error_state == 0 ? 0 : state.error_state - 1}
                            items={[
                                ["success", __("evse.content.error_ok")],
                                ["danger", __("evse.content.error_switch")],
                                ["danger", __("evse.content.error_calibration")],
                                ["danger", __("evse.content.error_contactor")],
                                ["danger", __("evse.content.error_communication")]
                            ]}/>
                    </FormRow>

                    <FormRow label={__("evse.content.contactor_state")} label_muted={__("evse.content.contactor_names")}>
                        <div class="row form-group mx-n1">
                            <IndicatorGroup
                                class="mb-1 col px-1"
                                value={(state.contactor_state & 1) == 1 ? 1 : 0}
                                items={[
                                    ["secondary", __("evse.content.contactor_not_live")],
                                    ["primary", __("evse.content.contactor_live")]
                                ]}/>
                            <IndicatorGroup
                                class="mb-1 col px-1"
                                value={state.contactor_state > 1 ? 1 : 0}
                                items={[
                                    ["secondary", __("evse.content.contactor_not_live")],
                                    ["primary", __("evse.content.contactor_live")]
                                ]}/>
                            <IndicatorGroup
                                class="mb-1 col-auto px-1"
                                value={state.contactor_error != 0 ? 1 : 0}
                                items={[
                                    ["success", __("evse.content.contactor_ok")],
                                    ["danger", state.contactor_error != 0 ? __("evse.script.error_code") + " " + state.contactor_error : __("evse.content.contactor_error")]
                                ]}/>
                        </div>
                    </FormRow>

                    <div hidden={!hardware_cfg.has_lock_switch}>
                        <FormRow label={__("evse.content.lock_state")}>
                            <IndicatorGroup
                                value={state.lock_state}
                                items={[
                                    ["secondary", __("evse.content.lock_init")],
                                    ["primary", __("evse.content.lock_open")],
                                    ["primary", __("evse.content.lock_closing")],
                                    ["primary", __("evse.content.lock_close")],
                                    ["primary", __("evse.content.lock_opening")],
                                    ["danger", __("evse.content.lock_error")]
                                ]}/>
                        </FormRow>
                    </div>

                    <FormRow label={__("evse.content.time_since_state_change")}>
                        <InputText value={util.format_timespan(Math.floor(ll_state.time_since_state_change / 1000.0))}/>
                    </FormRow>

                    <FormRow label={__("evse.content.uptime")}>
                        <InputText value={util.format_timespan(Math.floor(ll_state.uptime / 1000.0))}/>
                    </FormRow>

                    <FormSeparator heading={__("evse.content.charging_current")}/>

                    {slots.map((slot, i) => {
                        let variant = "";
                        let value = "";

                        if (!slot.active) {
                            value = __("evse.script.slot_disabled");
                            variant = "secondary";
                        }
                        else if (slot.max_current == 0) {
                            value = __("evse.script.slot_blocks");
                            variant = "danger";
                        }
                        else if (slot.max_current == 32000 && i > 1) {
                            value = __("evse.script.slot_no_limit");
                            variant = "success";
                        }
                        else {
                            value = toDisplayCurrent(slot.max_current);
                            variant = slot.max_current == min ? "warning" : "primary";
                        }

                        if (i == EVSE_SLOT_GP_INPUT || i == EVSE_SLOT_SHUTDOWN_INPUT)
                            return <></>

                        if (i != EVSE_SLOT_GLOBAL && i != EVSE_SLOT_EXTERNAL)
                            return <FormRow key={i} label={translate_unchecked(`evse.content.slot_${i}`)}>
                                <InputIndicator value={value} variant={variant as any} />
                            </FormRow>

                        return <FormRow key={i} label={translate_unchecked(`evse.content.slot_${i}`)}>
                            <InputIndicator value={value} variant={variant as any}
                                onReset={
                                    i == EVSE_SLOT_GLOBAL ?
                                    () => API.save('evse/global_current', {"current": 32000}, __("evse.script.reset_slot_failed")) :
                                    () => {
                                        API.save('evse/external_defaults', {
                                                "current": 32000,
                                                "clear_on_disconnect": false
                                            },
                                            __("evse.script.reset_slot_failed"));


                                        API.save('evse/external_current',
                                            {"current": 32000},
                                            __("evse.script.reset_slot_failed"));

                                        API.save('evse/external_clear_on_disconnect',
                                            {"clear_on_disconnect": false},
                                            __("evse.script.reset_slot_failed"));
                                    }
                                }
                                resetText={__("evse.content.reset_slot")}
                                resetDisabled={!slot.active || slot.max_current == 32000}/>
                        </FormRow>
                    })}

                    <FormSeparator heading={__("evse.content.configuration")}/>

                    <FormRow label={__("evse.content.has_lock_switch")}>
                        <IndicatorGroup
                            value={hardware_cfg.has_lock_switch ? 1 : 0}
                            items={[
                                ["secondary", __("evse.content.lock_no")],
                                ["primary", __("evse.content.lock_yes")]
                            ]}/>
                    </FormRow>

                    <FormRow label={__("evse.content.jumper_config_max_current")} label_muted={__("evse.content.jumper_config")}>
                        <IndicatorGroup
                            value={hardware_cfg.jumper_configuration}
                            items={[
                                ["primary",  "6 A"],
                                ["primary", "10 A"],
                                ["primary", "13 A"],
                                ["primary", "16 A"],
                                ["primary", "20 A"],
                                ["primary", "25 A"],
                                ["primary", "32 A"],
                                ["danger", __("evse.content.jumper_config_software")],
                                ["danger", __("evse.content.jumper_config_unconfigured")]
                            ]}/>
                    </FormRow>

                    <FormRow label={__("evse.content.evse_version")}>
                        <InputText value={(hardware_cfg.evse_version / 10).toFixed(1)}/>
                    </FormRow>

                    <FormSeparator heading={__("evse.content.debug")}/>

                    <DebugLogger prefix="evse" debug="evse/debug" debugHeader="evse/debug_header" translationPrefix="evse"/>

                    <CollapsedSection label={__("evse.content.low_level_state")}>
                        <FormRow label={__("evse.content.led_state")}>
                            <IndicatorGroup
                                value={ll_state.led_state}
                                items={[
                                    ["secondary", __("evse.content.led_state_off")],
                                    ["primary",   __("evse.content.led_state_on")],
                                    ["primary",   __("evse.content.led_state_blinking")],
                                    ["danger",    __("evse.content.led_state_flickering")],
                                    ["primary",   __("evse.content.led_state_breathing")],
                                    ["primary",   __("evse.content.led_state_api")]
                                ]}/>
                        </FormRow>

                        <FormRow label={__("evse.content.cp_pwm_dc")}>
                            <InputText value={util.toLocaleFixed(ll_state.cp_pwm_duty_cycle / 10, 1) + " %"}/>
                        </FormRow>

                        <FormRow label={__("evse.content.adc_values")} label_muted={__("evse.content.adc_names")}>
                            <div class="row mx-n1">
                                {ll_state.adc_values.map((x, i) => (
                                    <div key={i} class="mb-1 col-6 px-1">
                                        <InputText value={x}/>
                                    </div>
                                ))}
                            </div>
                        </FormRow>

                        <FormRow label={__("evse.content.voltages")} label_muted={__("evse.content.voltage_names")}>
                            <div class="row mx-n1">
                                {ll_state.voltages.map((x, i) => (
                                    <div key={i} class="mb-1 col-6 px-1">
                                        <InputText value={util.toLocaleFixed(x / 1000.0, 3) + " V"}/>
                                    </div>
                                ))}
                            </div>
                        </FormRow>

                        <FormRow label={__("evse.content.resistances")} label_muted={__("evse.content.resistance_names")}>
                            <div class="row mx-n1">
                                {ll_state.resistances.map((x, i) => (
                                    <div key={i} class="mb-1 col-6 px-1">
                                        <InputText value={x + " Ω"}/>
                                    </div>
                                ))}
                            </div>
                        </FormRow>

                        <div>
                        <FormRow label={__("evse.content.gpios")} label_muted={__("evse.content.gpio_names")}>
                            <div class="row mx-n1">
                                {ll_state.gpio.map((x, j) => (
                                    <IndicatorGroup vertical key={j} class="mb-1 col px-1"
                                        value={x ? 0 : 1} //intentionally inverted: the high button is the first
                                        items={[
                                            ["primary", __("evse.content.gpio_high")],
                                            ["secondary",   __("evse.content.gpio_low")]
                                        ]}/>
                                ))}
                            </div>
                        </FormRow>
                        </div>

                        <FormRow label={__("evse.content.charging_time")}>
                            <InputText value={util.format_timespan(Math.floor(ll_state.charging_time / 1000))}/>
                        </FormRow>

                        <FormRow label={__("evse.content.reset_description")} label_muted={__("evse.content.reset_description_muted")}>
                        <div class="input-group pb-2">
                            <Button variant="primary" className="form-control rounded-right mr-2" onClick={() => API.call('evse/reset', {}, "")}>{__("evse.content.reset_evse")}</Button>
                            <Button variant="primary" className="form-control rounded-left" onClick={() => API.call('evse/reflash', {}, "")}>{__("evse.content.reflash_evse")}</Button>
                        </div>
                        </FormRow>

                        <FormSeparator heading={__("evse.content.user_calibration_description")}/>
                        <FormRow label={__("evse.content.user_calibration")}>
                            <IndicatorGroup
                                    value={user_calibration.user_calibration_active ? 1 : 0}
                                    items={[
                                        ["primary", __("evse.content.user_calibration_state_disabled")],
                                        ["warning",   __("evse.content.user_calibration_state_enabled")]
                                    ]}/>
                        </FormRow>

                        <FormRow label={__("evse.content.user_calibration_description")} label_muted={__("evse.content.user_calibration_description_muted")}>
                            <div class="input-group pb-2">
                                <Button variant="primary" className="form-control rounded-right mr-2" href="evse/user_calibration" download="calibration.json">{__("evse.content.user_calibration_download")}</Button>
                                <Button variant="primary" className="form-control rounded-left"
                                    onClick={async () =>  API.save("evse/user_calibration",  {
                                            "user_calibration_active": false,
                                            "voltage_diff": 0,
                                            "voltage_mul": 0,
                                            "voltage_div": 0,
                                            "resistance_2700": 0,
                                            "resistance_880": [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
                                        }, __("evse.script.user_calibration_upload_failed"))
                                    }>
                                    {__("evse.content.user_calibration_reset")}
                                </Button>
                            </div>
                            <InputFile
                                    browse={__("evse.content.user_calibration_browse")}
                                    select_file={__("evse.content.user_calibration_select_file")}
                                    upload={__("evse.content.user_calibration_upload")}
                                    url="/evse/user_calibration_update"

                                    timeout_ms={10 * 1000}
                                    onUploadSuccess={() => {}}
                                    onUploadError={error => {
                                        util.add_alert("firmware_update_failed","alert-danger", __("evse.script.user_calibration_upload_failed"), error.toString());
                                    }}/>
                        </FormRow>

                        <FormRow label={__("evse.content.voltage_calibration")} label_muted={__("evse.content.voltage_calibration_names")}>
                            <div class="row mx-n1">
                                <div class="mb-1 col-4 px-1">
                                    <InputText value={user_calibration.voltage_diff}/>
                                </div>
                                <div class="mb-1 col-4 px-1">
                                    <InputText value={user_calibration.voltage_div}/>
                                </div>
                                <div class="mb-1 col-4 px-1">
                                    <InputText value={user_calibration.voltage_mul}/>
                                </div>
                            </div>
                        </FormRow>

                        <FormRow label={__("evse.content.resistance_2700")}>
                            <InputText value={user_calibration.resistance_2700}/>
                        </FormRow>

                        <FormRow label={__("evse.content.resistance_880")}>
                            <InputText value={user_calibration.resistance_880.join(", ")}/>
                        </FormRow>
                    </CollapsedSection>
            </>
        )
    }
}

render(<EVSE />, $('#evse')[0]);

class EVSESettings extends Component<{}, EVSESettingsState>
{
    constructor()
    {
        super();

        util.eventTarget.addEventListener('evse/boost_mode', () => {
            this.setState({boost_mode: API.get('evse/boost_mode')});
        });

        util.eventTarget.addEventListener('evse/auto_start_charging', () => {
            this.setState({auto_start_charging: API.get('evse/auto_start_charging')});
        });

        util.eventTarget.addEventListener('evse/slots', () => {
            this.setState({slots: API.get('evse/slots')});
        });
    }
    render(props: {}, s: EVSESettingsState)
    {
        if (!util.allow_render)
            return (<></>);

        let {
            slots,
            boost_mode,
            auto_start_charging} = s;

        return <>
                <PageHeader title={__("evse.content.settings")}/>

                <FormRow label={__("evse.content.auto_start_description")} label_muted={__("evse.content.auto_start_description_muted")}>
                    <Switch desc={__("evse.content.auto_start_enable")}
                            checked={!auto_start_charging.auto_start_charging}
                            onClick={async () => {
                                let inverted = !auto_start_charging.auto_start_charging;
                                await API.save('evse/auto_start_charging', {"auto_start_charging": inverted}, __("evse.script.save_failed"));
                            }}/>
                </FormRow>

                <FormRow label={__("evse.content.external_description")} label_muted={__("evse.content.external_description_muted")}>
                    <Switch desc={__("evse.content.external_enable")}
                            checked={slots[EVSE_SLOT_EXTERNAL].active}
                            onClick={async () => {
                                let inverted = !slots[EVSE_SLOT_EXTERNAL].active;
                                await API.save('evse/external_enabled', {"enabled": inverted}, __("evse.script.save_failed"));
                            }}/>
                </FormRow>

                <FormRow label={__("evse.content.boost_mode_desc")} label_muted={__("evse.content.boost_mode_desc_muted")}>
                    <Switch desc={__("evse.content.boost_mode")}
                            checked={boost_mode.enabled}
                            onClick={async () => {
                                let inverted = !boost_mode.enabled;
                                await API.save('evse/boost_mode', {"enabled": inverted}, __("evse.script.save_failed"));
                            }}/>
                </FormRow>
            </>;
    }
}

render(<EVSESettings/>, $('#evse-settings')[0])

export function init(){}

export function add_event_listeners(){}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-evse').prop('hidden', !module_init.evse);
    $('#sidebar-evse-settings').prop('hidden', !module_init.evse);
    $('#status-evse').prop('hidden', !module_init.evse);
}
