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

import { ConfigComponent } from "src/ts/components/config_component";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { InputText } from "../../ts/components/input_text";
import { InputIndicator } from "../../ts/components/input_indicator";
import { Switch } from "../../ts/components/switch";
import { Button} from "react-bootstrap";
import { InputSelect } from "src/ts/components/input_select";
import { CollapsedSection } from "src/ts/components/collapsed_section";
import { EVSE_SLOT_EXTERNAL, EVSE_SLOT_GLOBAL } from "../evse_common/api";
import { DebugLogger } from "../../ts/components/debug_logger";
import { ConfigForm } from "src/ts/components/config_form";
import { InputFloat } from "src/ts/components/input_float";

interface EVSEState {
    state: API.getType['evse/state'];
    ll_state: API.getType['evse/low_level_state'];
    hardware_cfg: API.getType['evse/hardware_configuration'];
    slots: Readonly<API.getType['evse/slots']>;
    control_pilot_disconnect: API.getType['evse/control_pilot_disconnect'];
}

interface EVSESSettingsState {
    button_cfg: API.getType['evse/button_configuration']
    slots: Readonly<API.getType['evse/slots']>;
    gpio_cfg: API.getType['evse/gpio_configuration'];
    ev_wakeup: API.getType['evse/ev_wakeup'];
    boost_mode: API.getType['evse/boost_mode'];
    auto_start_charging: API.getType['evse/auto_start_charging'];
    require_meter_enabled: API.getType['require_meter/config'];
    meter_abs: number
    evse_uptime: number
}

type ChargeLimitsConfig = API.getType["charge_limits/default_limits"];

let toDisplayCurrent = (x: number) => util.toLocaleFixed(x / 1000.0, 3) + " A"

export class EVSEV2 extends Component<{}, EVSEState> {

    constructor() {
        super();

        util.addApiEventListener('evse/state', () => {
            this.setState({state: API.get('evse/state')});
        });

        util.addApiEventListener('evse/low_level_state', () => {
            this.setState({ll_state: API.get('evse/low_level_state')});
        });

        util.addApiEventListener('evse/hardware_configuration', () => {
            this.setState({hardware_cfg: API.get('evse/hardware_configuration')});
        });

        util.addApiEventListener('evse/slots', () => {
            this.setState({slots: API.get('evse/slots')});
        });

        util.addApiEventListener('evse/control_pilot_disconnect', () => {
            this.setState({control_pilot_disconnect: API.get('evse/control_pilot_disconnect')});
        });
    }

    render(props: {}, s: Readonly<EVSEState>) {
        if (!util.render_allowed() || !API.hasFeature("evse"))
            return (<></>);

        let {state,
            ll_state,
            hardware_cfg,
            slots,} = s;

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
                                ["danger", __("evse.content.error_dc_fault_current")],
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

                    <FormRow label={__("evse.content.dc_fault_current_state")} label_muted={__("evse.content.dc_fault_current_state_desc")}>
                        <div class="row form-group mx-n1">
                            <IndicatorGroup
                                class="mb-1 col px-1 flex-wrap"
                                value={state.dc_fault_current_state}
                                items={[
                                    ["success", __("evse.content.dc_fault_current_ok")],
                                    ["danger", __("evse.content.dc_fault_current_6_ma")],
                                    ["danger", __("evse.content.dc_fault_current_system")],
                                    ["danger", __("evse.content.dc_fault_current_unknown")],
                                    ["danger", __("evse.content.dc_fault_current_calibration")]
                                ]}/>
                            <Button
                                disabled={state.dc_fault_current_state == 0}
                                variant="danger"
                                className="col-auto mx-1 mb-1"
                                onClick={async () => {
                                    const modal = util.async_modal_ref.current;
                                    if(!await modal.show({
                                        title: __("evse.content.reset_dc_fault_title"),
                                        body: __("evse.content.reset_dc_fault_content"),
                                        no_text: __("evse.content.abort"),
                                        yes_text: __("evse.content.reset"),
                                        no_variant: "secondary",
                                        yes_variant: "danger"
                                    }))
                                    return false;

                                    await API.call('evse/reset_dc_fault_current_state',
                                                   {"password": 0xDC42FA23},
                                                   __("evse.script.reset_dc_fault_current_failed"));
                                }}>
                                    {__("evse.content.dc_fault_current_reset")}
                                </Button>
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

                    {/*<FormRow label={__("evse.content.control_pilot_disconnect_desc")} label_muted={__("evse.content.control_pilot_disconnect_desc_muted")}>
                        <Switch desc={__("evse.content.control_pilot_disconnect")}
                                checked={control_pilot_disconnect.disconnect}
                                onClick={async () => {
                                    let inverted = !control_pilot_disconnect.disconnect;
                                    await API.save('evse/control_pilot_disconnect', {"disconnect": inverted}, __("evse.script.save_failed"));
                                }}/>
                            </FormRow>*/}

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

                    <FormRow label={__("evse.content.evse_fw_version")}>
                        <InputText value={API.get("evse/identity").fw_version}/>
                    </FormRow>

                    <FormRow label={__("evse.content.energy_meter_type")}>
                        <InputText value={translate_unchecked(`evse.script.meter_type_${hardware_cfg.energy_meter_type}`)}/>
                    </FormRow>

                    <FormSeparator heading={__("evse.content.debug")}/>
                    <DebugLogger prefix="evse" debugHeader="evse/debug_header" debug="evse/debug" translationPrefix="evse"/>

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
                        {
                            util.range(ll_state.gpio.length / 4).map(i =>
                            <FormRow key={i} label={i == 0 ? __("evse.content.gpios") : ""} label_muted={translate_unchecked(`evse.content.gpio_names_${i}`)}>
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
                            </FormRow>)
                        }
                        </div>

                        <FormRow label={__("evse.content.charging_time")}>
                            <InputText value={util.format_timespan(Math.floor(ll_state.charging_time / 1000))}/>
                        </FormRow>

                        <FormRow label={__("evse.content.reset_description")} label_muted={__("evse.content.reset_description_muted")}>
                        <div class="input-group pb-2">
                            <Button variant="primary" className="form-control rounded-right mr-2" onClick={() => API.call('evse/reset', {}, "")}>{__("evse.content.reset_evse")}</Button>
                            <Button variant="primary" className="form-control rounded-left" onClick={() => API.call('evse/reflash', {}, "")}>{__("evse.content.reflash_evse")}</Button>
                        </div>
                        <Button variant="primary" className="form-control" onClick={() => API.call('evse/trigger_dc_fault_test', {}, "")}>{__("evse.content.trigger_dc_fault_test")}</Button>
                        </FormRow>
                    </CollapsedSection>
            </>
        )
    }
}

render(<EVSEV2 />, $('#evse')[0]);

class EVSEV2Settings extends ConfigComponent<"charge_limits/default_limits", {}, EVSESSettingsState>
{
    constructor()
    {
        super("charge_limits/default_limits",
            __("evse.script.save_failed"),
            __("evse.script.reboot_content_changed"));

        util.addApiEventListener('evse/gpio_configuration', () => {
            this.setState({gpio_cfg: API.get('evse/gpio_configuration')});
        });

        util.addApiEventListener('evse/button_configuration', () => {
            this.setState({button_cfg: API.get('evse/button_configuration')});
        });

        util.addApiEventListener('evse/ev_wakeup', () => {
            this.setState({ev_wakeup: API.get('evse/ev_wakeup')});
        });

        util.addApiEventListener('evse/boost_mode', () => {
            this.setState({boost_mode: API.get('evse/boost_mode')});
        });

        util.addApiEventListener('evse/auto_start_charging', () => {
            this.setState({auto_start_charging: API.get('evse/auto_start_charging')});
        });
        util.addApiEventListener('evse/slots', () => {
            this.setState({slots: API.get('evse/slots')});
        })

        util.addApiEventListener("meter/values", () => {
            this.setState({meter_abs: API.get("meter/values").energy_abs});
        })

        util.addApiEventListener("evse/low_level_state", () => {
            this.setState({evse_uptime: API.get("evse/low_level_state").uptime});
        })

        util.addApiEventListener("require_meter/config", () => {
            this.setState({require_meter_enabled: API.get("require_meter/config")});
        })
    }

    override async sendSave(t: "charge_limits/default_limits", cfg: EVSESSettingsState & ChargeLimitsConfig): Promise<void> {
        await API.save('evse/auto_start_charging', {"auto_start_charging": this.state.auto_start_charging.auto_start_charging}, __("evse.script.save_failed"));
        await API.save('evse/external_enabled', {"enabled": this.state.slots[EVSE_SLOT_EXTERNAL].active}, __("evse.script.save_failed"));
        await API.save('evse/button_configuration', {"button": this.state.button_cfg.button}, __("evse.script.save_failed"));
        await API.save('evse/gpio_configuration', this.state.gpio_cfg, __("evse.script.gpio_configuration_failed"));
        await API.save('evse/ev_wakeup', {"enabled": this.state.ev_wakeup.enabled}, __("evse.script.save_failed"));
        await API.save('evse/boost_mode', {"enabled": this.state.boost_mode.enabled}, __("evse.script.save_failed"));
        await API.save('require_meter/config', {"config": this.state.require_meter_enabled.config}, __("evse.script.save_failed"));
        super.sendSave(t, cfg);
    }

    //TODO: Substitute hardcoded values after evse-reset-api is available.

    override async sendReset(t: "charge_limits/default_limits"): Promise<void> {
        await API.save('evse/auto_start_charging', {"auto_start_charging": true}, __("evse.script.save_failed"));
        await API.save('evse/external_enabled', {"enabled": false}, __("evse.script.save_failed"));
        await API.save('evse/button_configuration', {"button": 2}, __("evse.script.save_failed"));
        await API.save('evse/gpio_configuration', {"input": 0, "output": 1, "shutdown_input": 0}, __("evse.script.gpio_configuration_failed"));
        await API.save('evse/ev_wakeup', {"enabled": true}, __("evse.script.save_failed"));
        await API.save('evse/boost_mode', {"enabled": false}, __("evse.script.save_failed"));
        await API.reset('require_meter/config', __("evse.script.save_failed"));
        super.sendReset(t);
    }

    render(props: {}, s: EVSESSettingsState & ChargeLimitsConfig)
    {
        if (!util.render_allowed() || !API.hasFeature("evse"))
            return <></>;

        let {
            button_cfg,
            slots,
            gpio_cfg,
            ev_wakeup,
            boost_mode,
            auto_start_charging,
            require_meter_enabled} = s;

        const has_meter = API.hasFeature("meter");

        const energy_settings = <FormRow label={__("charge_limits.content.energy")} label_muted={__("charge_limits.content.energy_muted")}>
        <InputSelect items={[
            ["0", __("charge_limits.content.unlimited")],
            ["5000", util.toLocaleFixed(5, 0) + " kWh"],
            ["10000", util.toLocaleFixed(10, 0) + " kWh"],
            ["15000", util.toLocaleFixed(15, 0) + " kWh"],
            ["20000", util.toLocaleFixed(20, 0) + " kWh"],
            ["25000", util.toLocaleFixed(25, 0) + " kWh"],
            ["30000", util.toLocaleFixed(30, 0) + " kWh"],
            ["40000", util.toLocaleFixed(40, 0) + " kWh"],
            ["50000", util.toLocaleFixed(50, 0) + " kWh"],
            ["60000", util.toLocaleFixed(60, 0) + " kWh"],
            ["70000", util.toLocaleFixed(70, 0) + " kWh"],
            ["80000", util.toLocaleFixed(80, 0) + " kWh"],
            ["90000", util.toLocaleFixed(90, 0) + " kWh"],
            ["100000", util.toLocaleFixed(100, 0) + " kWh"]
        ]}
        value={s.energy_wh}
        onValue={(v) => this.setState({energy_wh: Number(v)})}/>
    </FormRow>;

        const require_meter = <FormRow label={__("evse.content.meter_monitoring")}>
                                        <Switch desc={__("evse.content.meter_monitoring_desc")}
                                            checked={require_meter_enabled.config == 2}
                                            onClick={async () => {
                                                this.setState({require_meter_enabled: {config: require_meter_enabled.config == 2 ? 1 : 2}});
                                            }}/>
                                    </FormRow>;

        return <>
                <ConfigForm id="evse_settings" title={__("evse.content.settings")} isModified={this.isModified()} onSave={this.save} onReset={this.reset} onDirtyChange={(d) => this.ignore_updates = d}>
                    <FormRow label={__("evse.content.auto_start_description")} label_muted={__("evse.content.auto_start_description_muted")}>
                        <Switch desc={__("evse.content.auto_start_enable")}
                                checked={!auto_start_charging.auto_start_charging}
                                onClick={async () => this.setState({auto_start_charging: {...auto_start_charging, auto_start_charging: !auto_start_charging.auto_start_charging}})}/>
                    </FormRow>

                    <FormRow label={__("evse.content.external_description")} label_muted={__("evse.content.external_description_muted")}>
                        <Switch desc={__("evse.content.external_enable")}
                                checked={slots[EVSE_SLOT_EXTERNAL].active}
                                onClick={async () => {
                                    let tmp = slots;
                                    slots[EVSE_SLOT_EXTERNAL].active = !slots[EVSE_SLOT_EXTERNAL].active;
                                    this.setState({slots: tmp});
                                }}/>
                    </FormRow>

                    <FormRow label={__("evse.content.button_configuration")} label_muted={__("evse.content.button_configuration_muted")}>
                        <InputSelect items={[
                                        ["0",__("evse.content.button_configuration_deactivated")],
                                        ["1",__("evse.content.button_configuration_start_charging")],
                                        ["2",__("evse.content.button_configuration_stop_charging")],
                                        ["3",__("evse.content.button_configuration_start_and_stop_charging")],
                                    ]}
                                value={button_cfg.button}
                                onValue={async (v) => {
                                    this.setState({button_cfg: {button: parseInt(v)}});
                                }}
                        />
                    </FormRow>

                    <FormRow label={__("evse.content.gpio_shutdown")} label_muted={__("evse.content.gpio_shutdown_muted")}>
                        <InputSelect items={[
                                        ["0",__("evse.content.gpio_shutdown_not_configured")],
                                        ["1",__("evse.content.gpio_shutdown_on_open")],
                                        ["2",__("evse.content.gpio_shutdown_on_close")],
                                    ]}
                                value={gpio_cfg.shutdown_input}
                                onValue={async (v) => {
                                    let cfg = gpio_cfg;
                                    cfg.shutdown_input = parseInt(v);
                                    this.setState({gpio_cfg: cfg});
                                }}
                        />
                    </FormRow>

                    <FormRow label={__("evse.content.gpio_in")} label_muted={__("evse.content.gpio_in_muted")}>
                        <InputSelect items={[
                                        ["0", __("evse.content.not_configured")],
                                        ["1",  __("evse.content.active_low_blocked")],
                                        ["2", __("evse.content.active_low_prefix")  + '6 A' + __("evse.content.active_low_suffix")],
                                        ["3", __("evse.content.active_low_prefix")  + '8 A' + __("evse.content.active_low_suffix")],
                                        ["4", __("evse.content.active_low_prefix")  + '10 A' + __("evse.content.active_low_suffix")],
                                        ["5", __("evse.content.active_low_prefix")  + '13 A' + __("evse.content.active_low_suffix")],
                                        ["6", __("evse.content.active_low_prefix")  + '16 A' + __("evse.content.active_low_suffix")],
                                        ["7", __("evse.content.active_low_prefix")  + '20 A' + __("evse.content.active_low_suffix")],
                                        ["8", __("evse.content.active_low_prefix")  + '25 A' + __("evse.content.active_low_suffix")],
                                        ["9",  __("evse.content.active_high_blocked")],
                                        ["10", __("evse.content.active_high_prefix")  + '6 A' + __("evse.content.active_high_suffix")],
                                        ["11", __("evse.content.active_high_prefix")  + '8 A' + __("evse.content.active_high_suffix")],
                                        ["12", __("evse.content.active_high_prefix")  + '10 A' + __("evse.content.active_high_suffix")],
                                        ["13", __("evse.content.active_high_prefix")  + '13 A' + __("evse.content.active_high_suffix")],
                                        ["14", __("evse.content.active_high_prefix")  + '16 A' + __("evse.content.active_high_suffix")],
                                        ["15", __("evse.content.active_high_prefix")  + '20 A' + __("evse.content.active_high_suffix")],
                                        ["16", __("evse.content.active_high_prefix")  + '25 A' + __("evse.content.active_high_suffix")],
                                        ["disabled", __("evse.content.todo")]
                                    ]}
                                value={gpio_cfg.input}
                                onValue={async (v) => {
                                    let cfg = gpio_cfg;
                                    cfg.input = parseInt(v);
                                    this.setState({gpio_cfg: cfg});
                                }}
                        />
                    </FormRow>

                    <FormRow label={__("evse.content.gpio_out")} label_muted={__("evse.content.gpio_out_muted")}>
                        <InputSelect items={[
                                        ["0",__("evse.content.gpio_out_low")],
                                        ["1",__("evse.content.gpio_out_high")],
                                        ["disabled",__("evse.content.todo")],
                                    ]}
                                value={gpio_cfg.output}
                                onValue={async (v) => {
                                    let cfg = gpio_cfg;
                                    cfg.output = parseInt(v);
                                    this.setState({gpio_cfg: cfg});
                                }}
                        />
                    </FormRow>

                    <FormRow label={__("evse.content.ev_wakeup_desc")} label_muted={__("evse.content.ev_wakeup_desc_muted")}>
                        <Switch desc={__("evse.content.ev_wakeup")}
                                checked={ev_wakeup.enabled}
                                onClick={async () => this.setState({ev_wakeup: {enabled: !ev_wakeup.enabled}})}/>
                    </FormRow>

                    <FormRow label={__("evse.content.boost_mode_desc")} label_muted={__("evse.content.boost_mode_desc_muted")}>
                        <Switch desc={__("evse.content.boost_mode")}
                                checked={boost_mode.enabled}
                                onClick={async () => this.setState({boost_mode: {enabled: !boost_mode.enabled}})}/>
                    </FormRow>

                    {require_meter_enabled.config != 0 ? require_meter : <></>}

                    <FormRow label={__("charge_limits.content.duration")} label_muted={__("charge_limits.content.duration_muted")}>
                        <InputSelect items={[
                            ["0", __("charge_limits.content.unlimited")],
                            ["1", __("charge_limits.content.min15")],
                            ["2", __("charge_limits.content.min30")],
                            ["3", __("charge_limits.content.min45")],
                            ["4", __("charge_limits.content.h1")],
                            ["5", __("charge_limits.content.h2")],
                            ["6", __("charge_limits.content.h3")],
                            ["7", __("charge_limits.content.h4")],
                            ["8", __("charge_limits.content.h6")],
                            ["9", __("charge_limits.content.h8")],
                            ["10", __("charge_limits.content.h12")]
                        ]}
                        value={s.duration}
                        onValue={(v) => this.setState({duration: Number(v)})}/>
                    </FormRow>
                    {has_meter ? energy_settings : <></>}
                </ConfigForm>
        </>;
    }
}

render(<EVSEV2Settings/>, $('#evse-settings')[0]);

export function init() {
}

export function add_event_listeners(source: API.APIEventTarget) {

}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-evse').prop('hidden', !module_init.evse_v2);
    $('#sidebar-evse-settings').prop('hidden', !module_init.evse_v2);
    $('#status-evse').prop('hidden', !module_init.evse_v2);
}
