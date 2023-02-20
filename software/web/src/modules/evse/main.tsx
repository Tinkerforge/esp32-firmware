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
import { EVSE_SLOT_EXTERNAL, EVSE_SLOT_GLOBAL } from "../evse_common/api";
import { InputFile } from "src/ts/components/input_file";

interface EVSEState {
    state: API.getType['evse/state'];
    ll_state: API.getType['evse/low_level_state'];
    hardware_cfg: API.getType['evse/hardware_configuration'];
    slots: Readonly<API.getType['evse/slots']>;
    user_calibration: API.getType['evse/user_calibration'];
    debug_running: boolean;
    debug_status: string;
}

let toDisplayCurrent = (x: number) => util.toLocaleFixed(x / 1000.0, 3) + " A"

export class EVSE extends Component<{}, EVSEState> {
    debug_log = "";

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

        util.eventTarget.addEventListener("evse/debug_header", (e) => {
            this.debug_log += e.data + "\n";
        }, false);

        util.eventTarget.addEventListener("evse/debug", (e) => {
            this.debug_log += e.data + "\n";
        }, false);
    }

    async get_debug_report_and_event_log() {
        try {
            this.setState({debug_status: __("evse.script.loading_debug_report")});
            this.debug_log += await util.download("/debug_report").then(blob => blob.text());
            this.debug_log += "\n\n";
        } catch (error) {
            this.setState({debug_running: false, debug_status: __("evse.script.loading_debug_report_failed")});
            throw __("evse.script.loading_debug_report_failed") + ": " + error;
        }

        try {
            this.setState({debug_status: __("evse.script.loading_event_log")});
            this.debug_log += await util.download("/event_log").then(blob => blob.text());
            this.debug_log += "\n";
        } catch (error) {
            this.setState({debug_running: false, debug_status: __("evse.script.loading_event_log_failed")});
            throw __("evse.script.loading_event_log_failed") + ": " + error;
        }
    }

    async debug_start() {
        this.debug_log = "";
        this.setState({debug_running: true});

        try {
            await this.get_debug_report_and_event_log();

            this.setState({debug_status: __("evse.script.starting_debug")});
        } catch(error) {
            this.setState({debug_running: false, debug_status: error});
            return;
        }

        try{
            await util.download("/evse/start_debug");
        } catch {
            this.setState({debug_running: false, debug_status: __("evse.script.starting_debug_failed")});
            return;
        }

        this.setState({debug_status: __("evse.script.debug_running")});
    }

    async debug_stop() {
        this.setState({debug_running: false});

        try {
            await util.download("/evse/stop_debug");
        } catch {
            this.setState({debug_running: true, debug_status: __("evse.script.debug_stop_failed")});
        }

        try {
            this.debug_log += "\n\n";
            this.setState({debug_status: __("evse.script.debug_stopped")});

            await this.get_debug_report_and_event_log();
            this.setState({debug_status: __("evse.script.debug_done")});
        } catch (error) {
            this.debug_log += "\n\nError while stopping charge protocol: ";
            this.debug_log += error;

            this.setState({debug_status: error});
        }

        //Download log in any case: Even an incomplete log can be useful for debugging.
        util.downloadToFile(this.debug_log, "evse-debug-log", "txt", "text/plain");
    }

    render(props: {}, s: Readonly<EVSEState>) {
        if (!s || !s.slots)
            return (<></>);

        let {state,
            ll_state,
            hardware_cfg,
            slots,
            user_calibration,
            debug_running,
            debug_status} = s;

        let min = Math.min(...slots.filter(s => s.active).map(s => s.max_current));

        if (debug_running) {
            window.onbeforeunload = (e: Event) => {
                e.preventDefault();
                // returnValue is not a boolean, but the string to be shown
                // in the "are you sure you want to close this tab" message
                // box. However this string is only shown in some browsers.
                e.returnValue = __("evse.script.tab_close_warning") as any;
            }
        } else {
            window.onbeforeunload = null;
        }

        return (
            <>
                <PageHeader title={__("evse.content.evse")} />
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

                    <FormSeparator heading={__("evse.content.settings")}/>

                    <FormRow label={__("evse.content.external_description")} label_muted={__("evse.content.external_description_muted")}>
                        <Switch desc={__("evse.content.external_enable")}
                                checked={slots[EVSE_SLOT_EXTERNAL].active}
                                onClick={async () => {
                                    let inverted = !slots[EVSE_SLOT_EXTERNAL].active;
                                    await API.save('evse/external_enabled', {"enabled": inverted}, __("evse.script.save_failed"));
                                }}/>
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

                    <FormRow label={__("evse.content.debug_description")} label_muted={__("evse.content.debug_description_muted")}>
                        <div class="input-group pb-2">
                            <Button variant="primary" className="form-control rounded-right mr-2" onClick={() => {this.debug_start()}} disabled={debug_running}>{__("evse.content.debug_start")}</Button>
                            <Button variant="primary" className="form-control rounded-left" onClick={() => {this.debug_stop()}} disabled={!debug_running}>{__("evse.content.debug_stop")}</Button>
                        </div>
                        <InputText value={debug_status}/>
                    </FormRow>

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

function update_evse_status_start_charging_button() {
    let state = API.get('evse/state');
    let slots = API.get('evse/slots');
    let ll_state = API.get('evse/low_level_state');

    // It is not helpful to enable the button if auto-start is active, but we are blocked for some other reason.
    $('#status_start_charging').prop("disabled", state.iec61851_state != 1 || slots[4].max_current != 0 || ll_state.gpio[0]);
}

function update_evse_state() {
    let state = API.get('evse/state');

    util.update_button_group("btn_group_evse_state", state.charger_state);

    $('#status_stop_charging').prop("disabled", state.charger_state != 2 && state.charger_state != 3);
}

let status_charging_current_dirty = false;


function set_charging_current(current: number) {
    if (status_plus_minus_timeout != null) {
        window.clearTimeout(status_plus_minus_timeout);
        status_plus_minus_timeout = null;
    }

    status_charging_current_dirty = false;
    util.setNumericInput("status_charging_current", current / 1000, 3);

    API.save('evse/global_current', {"current": current}, __("evse.script.set_charging_current_failed"));
}


function update_evse_auto_start_charging() {
    let x = API.get('evse/auto_start_charging');

    $('#status_auto_start_charging').prop("checked", x.auto_start_charging);
}

function set_auto_start_charging(auto_start_charging: boolean) {
    API.save('evse/auto_start_charging', {"auto_start_charging": auto_start_charging}, __("evse.script.auto_start_charging_update"));
}

function start_charging() {
    API.call('evse/start_charging', {}, __("evse.script.start_charging_failed"));
}

function stop_charging() {
    API.call('evse/stop_charging', {}, __("evse.script.stop_charging_failed"));
}

function update_evse_slots() {
    let slots = API.get('evse/slots');

    let real_maximum = 32000;
    for(let i = 0; i < slots.length; ++i) {
        let s = slots[i];
        if (s.active)
            real_maximum = Math.min(real_maximum, s.max_current);
    }

    let theoretical_maximum = Math.min(slots[0].max_current, slots[1].max_current);
    let theoretical_maximum_str = util.toLocaleFixed(theoretical_maximum / 1000.0, 0) + " A";
    $('#status_charging_current').prop("max", theoretical_maximum / 1000);
    $("#status_charging_current_maximum").on("click", () => set_charging_current(theoretical_maximum));
    $('#status_charging_current_maximum').html(theoretical_maximum_str);

    if(!status_charging_current_dirty) {
        let shown_current = Math.min(slots[EVSE_SLOT_GLOBAL].max_current, theoretical_maximum);
        util.setNumericInput("status_charging_current", shown_current / 1000.0, 3);
    }

    if (real_maximum == 32000) {
        $('#evse_status_allowed_charging_current').val(util.toLocaleFixed(real_maximum / 1000.0, 3) + " A");
        return;
    }

    let status_string = util.toLocaleFixed(real_maximum / 1000.0, 3) + " A " + __("evse.script.by") + " ";

    let status_list = [];
    for(let i = 0; i < slots.length; ++i) {
        let s = slots[i];
        if (s.active && s.max_current == real_maximum && real_maximum > 0)
            $(`#slot_${i}`).css("border-left-color", "#ffc107");
        if (!s.active || s.max_current != real_maximum)
            continue;

        status_list.push(translate_unchecked(`evse.script.slot_${i}`));
    }

    status_string += status_list.join(", ");

    $('#evse_status_allowed_charging_current').val(status_string);
}

let status_plus_minus_timeout: number = null;

export function init() {
    $("#status_charging_current_minimum").on("click", () => set_charging_current(6000));
    $("#status_charging_current_maximum").on("click", () => set_charging_current(32000));

    $("#status_stop_charging").on("click", stop_charging);
    $("#status_start_charging").on("click", start_charging);

    $('#status_auto_start_charging').on("change", () => set_auto_start_charging($('#status_auto_start_charging').prop('checked')));

    let input = $('#status_charging_current');

    $('#evse_status_charging_current_form').on('submit', function (this: HTMLFormElement, event: Event) {
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }

        set_charging_current(Math.round((input.val() as number) * 1000));
    });

    $('#status_charging_current_minus').on("click", () => {
        let val: number = parseInt(input.val().toString());
        let target = (val % 1 === 0) ? (Math.floor(val) - 1) : Math.floor(val);

        if (target < $('#status_charging_current').prop("min"))
            return;

        if (status_plus_minus_timeout != null) {
            window.clearTimeout(status_plus_minus_timeout);
            status_plus_minus_timeout = null;
        }

        util.setNumericInput("status_charging_current", target, 3);

        status_plus_minus_timeout = window.setTimeout(() => {
            set_charging_current(target * 1000);
        }, 2000);
    });

    $('#status_charging_current_plus').on("click", () => {
        let val = parseFloat(input.val().toString());
        let target = Math.floor(val) + 1;

        if (target > $('#status_charging_current').prop("max"))
            return;

        if (status_plus_minus_timeout != null) {
            window.clearTimeout(status_plus_minus_timeout);
            status_plus_minus_timeout = null;
        }

        util.setNumericInput("status_charging_current", target, 3);

        status_plus_minus_timeout = window.setTimeout(() => {
            set_charging_current(target * 1000);
        }, 2000);
    });

    $('#status_charging_current').on("input", () => {
        status_charging_current_dirty = true;

        let val = parseFloat(input.val().toString());
        let target = val;

        if (target > parseInt($('#status_charging_current').prop("max"))) {
            return;
        }

        if (target < parseInt($('#status_charging_current').prop("min"))) {
            return;
        }

        if (status_plus_minus_timeout != null) {
            window.clearTimeout(status_plus_minus_timeout);
            status_plus_minus_timeout = null;
        }

        status_plus_minus_timeout = window.setTimeout(() => {
            // Use round here instead of float, as non-representable floats * 1000 create
            // confusing behaviour otherwise.
            // For example 8.123 (represented as 8.1229999...3 * 1000 is 8122.999...3, with floor results in 8122 instead of 8123.
            // This is only a problem here, as all other occurences only work on non-fractional numbers.
            set_charging_current(Math.round(target * 1000));
        }, 2000);
    });
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('evse/state', update_evse_state);
    source.addEventListener('evse/auto_start_charging', update_evse_auto_start_charging);
    source.addEventListener("evse/slots", update_evse_slots);
    source.addEventListener("evse/state", update_evse_status_start_charging_button);
    source.addEventListener("evse/slots", update_evse_status_start_charging_button);
    source.addEventListener("evse/low_level_state", update_evse_status_start_charging_button);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-evse').prop('hidden', !module_init.evse);
    $('#status-evse').prop('hidden', !module_init.evse);
}
