/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
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

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { h, Component, Fragment, RefObject } from "preact";
import { Button } from "react-bootstrap";
import { CollapsedSection } from "../../ts/components/collapsed_section";
import { DebugLogger } from "../../ts/components/debug_logger";
import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { InputFile } from "../../ts/components/input_file";
import { InputIndicator } from "../../ts/components/input_indicator";
import { InputText } from "../../ts/components/input_text";
import { PageHeader } from "../../ts/components/page_header";
import { SubPage } from "../../ts/components/sub_page";
import { __, translate_unchecked } from "../../ts/translation";
import { EVSE_SLOT_EXTERNAL, EVSE_SLOT_AUTOMATION } from "./api";
import { EVSEStatus } from "./evse_status";
import { NavbarItem } from "../../ts/components/navbar_item";
import { BatteryCharging } from "react-feather";

export function EVSENavbar() {
    return <NavbarItem name="evse" title={__("evse.navbar.evse")} symbol={<BatteryCharging />} hidden={!API.hasModule("evse_v2") && !API.hasModule("evse")} />;
}

let toDisplayCurrent = (x: number) => util.toLocaleFixed(x / 1000.0, 3) + " A"

export class EVSE extends Component<{status_ref?: RefObject<EVSEStatus>}, {}> {
    override render(props: {}, s: {}) {
        if (!util.render_allowed() || !API.hasFeature("evse"))
            return <SubPage name="evse" />;

        let state = API.get('evse/state');
        let ll_state = API.get('evse/low_level_state');
        let hardware_cfg = API.get('evse/hardware_configuration');
        let slots = API.get('evse/slots');
        let user_calibration = API.get('evse/user_calibration');

        let is_evse_v1 = hardware_cfg.evse_version >= 10;
        let is_evse_v2 = hardware_cfg.evse_version >= 20;
        let is_evse_v3 = hardware_cfg.evse_version >= 30;

        let min = Math.min(...slots.filter(s => s.active).map(s => s.max_current));

        let pe_error = state.error_state == 4 && ((!is_evse_v3 && state.contactor_error == 4)
                                                    || (is_evse_v3 && (state.contactor_error & 1) == 1));

        // EVSE 3.0 can report both a PE and a contactor error!
        let contactor_error = state.error_state == 4 && ((!is_evse_v3 && state.contactor_error != 4)
                                                           || (is_evse_v3 && (state.contactor_error & 1) == 0));

        return <SubPage name="evse">
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
                                ["danger", __("evse.content.error_2")(is_evse_v2)],
                                ["danger", __("evse.content.error_contactor")(pe_error, contactor_error)],
                                ["danger", __("evse.content.error_communication")]
                            ]}/>
                    </FormRow>

                    <FormRow label={__("evse.content.contactor_state")} label_muted={__("evse.content.contactor_names")(is_evse_v3)}>
                        <div class="row mx-n1">
                            <IndicatorGroup
                                class="mb-1 col px-1"
                                value={(state.contactor_state & 1) == 1 ? 1 : 0}
                                items={[
                                    ["secondary", __("evse.content.contactor_not_live")(is_evse_v3)],
                                    ["primary", __("evse.content.contactor_live")(is_evse_v3)]
                                ]}/>
                            <IndicatorGroup
                                class="mb-1 col px-1"
                                value={(state.contactor_state & 2) == 2 ? 1 : 0}
                                items={[
                                    ["secondary", __("evse.content.contactor_not_live")(is_evse_v3)],
                                    ["primary", __("evse.content.contactor_live")(is_evse_v3)]
                                ]}/>
                            <IndicatorGroup
                                class="mb-1 col-auto px-1"
                                value={state.contactor_error > (is_evse_v3 ? 1 : 0) ? 1 : 0}
                                items={[
                                    ["success", __("evse.content.contactor_ok")],
                                    ["danger", __("evse.content.contactor_error")(state.contactor_error >> (is_evse_v3 ? 1 : 0))]
                                ]}/>
                        </div>
                    </FormRow>

                    {!is_evse_v2 ? undefined :
                        <FormRow label={__("evse.content.dc_fault_current_state")} label_muted={__("evse.content.dc_fault_current_state_desc")}>
                            <div class="row mx-n1">
                                <IndicatorGroup
                                    class="mb-1 col px-1 flex-wrap"
                                    value={state.dc_fault_current_state}
                                    items={[
                                        ["success", __("evse.content.dc_fault_current_ok")],
                                        ["danger", __("evse.content.dc_fault_current_6_ma")],
                                        ["danger", __("evse.content.dc_fault_current_system")],
                                        ["danger", __("evse.content.dc_fault_current_unknown")],
                                        ["danger", __("evse.content.dc_fault_current_calibration")(state.dc_fault_current_state, ll_state.dc_fault_pins)],
                                    ].concat((ll_state.dc_fault_sensor_type > 0 ? [
                                        ["danger", __("evse.content.dc_fault_current_20_ma")],
                                        ["danger", __("evse.content.dc_fault_current_6_ma_20_ma")],
                                    ] : [])) as any
                                    }/>
                                <Button
                                    disabled={state.dc_fault_current_state == 0}
                                    variant="danger"
                                    className="col-auto mx-1 mb-1"
                                    onClick={async () => {
                                        const modal = util.async_modal_ref.current;
                                        if(!await modal.show({
                                            title: () => __("evse.content.reset_dc_fault_title"),
                                            body: () => __("evse.content.reset_dc_fault_content"),
                                            no_text: () => __("evse.content.abort"),
                                            yes_text: () => __("evse.content.reset"),
                                            no_variant: "secondary",
                                            yes_variant: "danger",
                                        }))
                                        return false;

                                        await API.call('evse/reset_dc_fault_current_state',
                                                    {"password": 0xDC42FA23},
                                                    () => __("evse.script.reset_dc_fault_current_failed"));
                                    }}>
                                        {__("evse.content.dc_fault_current_reset")}
                                    </Button>
                            </div>
                        </FormRow>
                    }

                    {!hardware_cfg.has_lock_switch ? undefined :
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
                    }

                    <FormRow label={__("evse.content.time_since_state_change")}>
                        <InputText value={util.format_timespan_ms(ll_state.time_since_state_change)}/>
                    </FormRow>

                    <FormRow label={__("evse.content.uptime")}>
                        <InputText value={util.format_timespan_ms(ll_state.uptime)}/>
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

                        switch (i) {
                            case EVSE_SLOT_EXTERNAL:
                                return <FormRow key={i} label={__("evse.content.slot")(i)}>
                                    <InputIndicator value={value} variant={variant as any}
                                        onReset={
                                            () => {
                                                API.save('evse/external_defaults', {
                                                        "current": 32000,
                                                        "clear_on_disconnect": false
                                                    },
                                                    () => __("evse.script.reset_slot_failed"));


                                                API.save('evse/external_current',
                                                    {"current": 32000},
                                                    () => __("evse.script.reset_slot_failed"));

                                                API.save('evse/external_clear_on_disconnect',
                                                    {"clear_on_disconnect": false},
                                                    () => __("evse.script.reset_slot_failed"));
                                            }
                                        }
                                        resetVariant="danger"
                                        resetText={__("evse.content.reset_slot")}
                                        resetHidden={!slot.active || slot.max_current == 32000}/>
                                </FormRow>
                            case EVSE_SLOT_AUTOMATION:
                                return <FormRow key={i} label={__("evse.content.slot")(i)}>
                                    <InputIndicator value={value} variant={variant as any}
                                        onReset={
                                            () => API.save('evse/automation_current',
                                                    {"current": 32000},
                                                    () => __("evse.script.reset_slot_failed"))
                                        }
                                        resetVariant="danger"
                                        resetText={__("evse.content.reset_slot")}
                                        resetHidden={!slot.active || slot.max_current == 32000}/>
                                </FormRow>
                            default:
                                return <FormRow key={i} label={__("evse.content.slot")(i)}>
                                    <InputIndicator value={value} variant={variant as any} />
                                </FormRow>
                        }
                    })}

                    <FormSeparator heading={__("evse.content.configuration")}/>

                    {/*Hide this by default to not confuse users.
                       In the unexpected case that the EVSE reports that it has
                       a lock switch (because of a firmware error?), show this.*/}
                    {!hardware_cfg.has_lock_switch ? undefined :
                    <FormRow label={__("evse.content.has_lock_switch")}>
                        <IndicatorGroup
                            value={hardware_cfg.has_lock_switch ? 1 : 0}
                            items={[
                                ["secondary", __("evse.content.lock_no")],
                                ["primary", __("evse.content.lock_yes")]
                            ]}/>
                    </FormRow>
                    }

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

                    <FormRow label={__("evse.content.evse_fw_version")}>
                        <InputText value={API.get("evse/identity").fw_version}/>
                    </FormRow>

                    {!is_evse_v2 ? undefined :
                        <FormRow label={__("evse.content.energy_meter_type")}>
                            <InputText value={translate_unchecked(`evse.script.meter_type_${hardware_cfg.energy_meter_type}`)}/>
                        </FormRow>
                    }

                    <FormSeparator heading={__("evse.content.debug")}/>
                    <DebugLogger translationPrefix="evse" />

                    <CollapsedSection>
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

                        <FormRow label={__("evse.content.adc_values")} label_muted={__("evse.content.adc_names")(is_evse_v2)}>
                            <div class="row mx-n1">
                                {ll_state.adc_values.map((x, i) => (
                                    <div key={i} class="mb-1 col-6 px-1">
                                        <InputText value={x}/>
                                    </div>
                                ))}
                            </div>
                        </FormRow>

                        <FormRow label={__("evse.content.voltages")} label_muted={__("evse.content.voltage_names")(is_evse_v2)}>
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

                        {!is_evse_v2 ?
                            (<FormRow label={__("evse.content.gpios")} label_muted={__("evse.content.gpio_names")}>
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
                            </FormRow>)
                         : (util.range(ll_state.gpio.length / 4).map(i =>
                                <FormRow key={i} label={i == 0 ? __("evse.content.gpios") : ""} label_muted={translate_unchecked(`evse.content.evse_v${is_evse_v3 ? 3 : 2}_gpio_names_${i}`)}>
                                    <div class="row mx-n1">
                                        {ll_state.gpio.slice(i*4, i*4+4).map((x, j) => (
                                            <IndicatorGroup vertical key={j} class="mb-1 col-3 px-1"
                                                value={x ? 0 : 1} //intentionally inverted: the high button is the first
                                                items={[
                                                    ["primary", __("evse.content.gpio_high")],
                                                    ["secondary",   __("evse.content.gpio_low")]
                                                ]}/>
                                        ))}
                                    </div>
                                </FormRow>))
                        }

                        <FormRow label={__("evse.content.car_stopped_charging")}>
                            <IndicatorGroup
                                value={ll_state.charging_time}
                                items={[
                                    ["secondary", __("evse.content.car_stopped_false")],
                                    ["primary", __("evse.content.car_stopped_true")]
                                ]}/>
                        </FormRow>

                        {!is_evse_v3 ? undefined :
                        <>
                            <FormRow label={__("evse.content.temperature")}>
                                <InputText value={util.toLocaleFixed(ll_state.temperature / 100, 2) + " °C"} />
                            </FormRow>

                            <FormRow label={__("evse.content.phases_current")}>
                                <InputText value={ll_state.phases_current}/>
                            </FormRow>

                            <FormRow label={__("evse.content.phases_requested")}>
                                <InputText value={ll_state.phases_requested}/>
                            </FormRow>

                            <FormRow label={__("evse.content.phases_state")}>
                                <InputText value={ll_state.phases_state}/>
                            </FormRow>
                        </>
                        }

                        {!is_evse_v3 && !is_evse_v2 ? undefined :
                        <>
                            <FormRow label={__("evse.content.time_since_dc_fault_check")}>
                                <InputText value={util.format_timespan_ms(ll_state.time_since_dc_fault_check)}/>
                            </FormRow>

                            <FormRow label={__("evse.content.dc_fault_sensor_type")}>
                                <InputText value={ll_state.dc_fault_sensor_type.toString()}/>
                            </FormRow>

                            <FormRow label={__("evse.content.dc_fault_pins")}>
                                <InputText value={ll_state.dc_fault_pins.toString()}/>
                            </FormRow>
                        </>
                        }

                        <FormRow label={__("evse.content.reset_description")} label_muted={__("evse.content.reset_description_muted")}>
                            <div class="input-group pb-2">
                                <Button variant="primary" className="form-control rounded-right mr-2" onClick={() => API.call('evse/reset', {}, () => "")}>{__("evse.content.reset_evse")}</Button>
                                <Button variant="primary" className="form-control rounded-left" onClick={() => API.call('evse/reflash', {}, () => "")}>{__("evse.content.reflash_evse")}</Button>
                            </div>
                            {!is_evse_v2 ? undefined :
                                <Button variant="primary" className="form-control" onClick={() => API.call('evse/trigger_dc_fault_test', {}, () => "")}>{__("evse.content.trigger_dc_fault_test")}</Button>
                            }
                        </FormRow>

                        {(is_evse_v1 && !is_evse_v2) ?
                            <>
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
                                        onClick={async () =>  API.save("evse/user_calibration", {
                                                "user_calibration_active": false,
                                                "voltage_diff": 0,
                                                "voltage_mul": 0,
                                                "voltage_div": 0,
                                                "resistance_2700": 0,
                                                "resistance_880": [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
                                            }, () => __("evse.script.user_calibration_upload_failed"))
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
                                            util.add_alert("firmware_update_failed", "danger", () => __("evse.script.user_calibration_upload_failed"), () => error.toString());
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
                            </>
                            : undefined
                        }


                    </CollapsedSection>

        </SubPage>
    }
}
