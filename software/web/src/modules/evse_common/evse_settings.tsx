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
import { h, Fragment, Component } from "preact";
import { __ } from "../../ts/translation";
import { FormRow } from "../../ts/components/form_row";
import { SubPage } from "../../ts/components/sub_page";
import { ConfigComponent } from "../../ts/components/config_component";
import { InputSelect } from "../../ts/components/input_select";
import { ConfigForm } from "../../ts/components/config_form";
import { Switch } from "../../ts/components/switch";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Settings } from "react-feather";

export function EVSESettingsNavbar() {
    return <NavbarItem name="evse_settings" title={__("evse.navbar.evse_settings")} symbol={<Settings />} hidden={!API.hasModule("evse_v2") && !API.hasModule("evse")} />;
}

interface EVSESettingsState {
    button_cfg: API.getType['evse/button_configuration']
    slots: Readonly<API.getType['evse/slots']>;
    gpio_cfg: API.getType['evse/gpio_configuration'];
    ev_wakeup: API.getType['evse/ev_wakeup'];
    phase_auto_switch: API.getType['evse/phase_auto_switch'];
    phases_connected: API.getType['evse/phases_connected'];
    boost_mode: API.getType['evse/boost_mode'];
    auto_start_charging: API.getType['evse/auto_start_charging'];
    require_meter_enabled: API.getType['require_meter/config'];
    led_configuration: API.getType['evse/led_configuration'];
    evse_uptime: number
    is_evse_v2: boolean
    is_evse_v3: boolean
    have_meter: boolean
}

type ChargeLimitsConfig = API.getType["charge_limits/default_limits"];

export class EVSESettings extends ConfigComponent<"charge_limits/default_limits", {}, EVSESettingsState> {
    constructor() {
        super("charge_limits/default_limits",
              () => __("evse.script.save_failed"),
              () => __("evse.script.reboot_content_changed"));

        util.addApiEventListener('evse/gpio_configuration', () => {
            this.setState({gpio_cfg: API.get('evse/gpio_configuration')});
        });

        util.addApiEventListener('evse/hardware_configuration', () => {
            this.setState({have_meter: (API.get('evse/hardware_configuration').energy_meter_type ?? 0) > 0});
        });

        util.addApiEventListener('evse/button_configuration', () => {
            this.setState({button_cfg: API.get('evse/button_configuration')});
        });

        util.addApiEventListener('evse/ev_wakeup', () => {
            this.setState({ev_wakeup: API.get('evse/ev_wakeup')});
        });

        util.addApiEventListener('evse/phase_auto_switch', () => {
            this.setState({phase_auto_switch: API.get('evse/phase_auto_switch')});
        });

        util.addApiEventListener('evse/boost_mode', () => {
            this.setState({boost_mode: API.get('evse/boost_mode')});
        });

        util.addApiEventListener('evse/auto_start_charging', () => {
            this.setState({auto_start_charging: API.get('evse/auto_start_charging')});
        });

        util.addApiEventListener('evse/slots', () => {
            this.setState({slots: API.get('evse/slots')});
        });

        util.addApiEventListener("evse/low_level_state", () => {
            this.setState({evse_uptime: API.get("evse/low_level_state").uptime});
        });

        util.addApiEventListener("require_meter/config", () => {
            this.setState({require_meter_enabled: API.get("require_meter/config")});
        });

        util.addApiEventListener("evse/led_configuration", () => {
            this.setState({led_configuration: API.get("evse/led_configuration")});
        });

        util.addApiEventListener("evse/hardware_configuration", () => {
            this.setState({is_evse_v2: API.get("evse/hardware_configuration").evse_version >= 20,
                           is_evse_v3: API.get("evse/hardware_configuration").evse_version >= 30});
        });

        util.addApiEventListener("evse/phases_connected", () => {
            this.setState({phases_connected: API.get("evse/phases_connected")});
        });
    }

    override async sendSave(t: "charge_limits/default_limits", cfg: EVSESettingsState & ChargeLimitsConfig) {
        await API.save('evse/auto_start_charging', {"auto_start_charging": this.state.auto_start_charging.auto_start_charging}, () => __("evse.script.save_failed"));
        await API.save('evse/boost_mode', {"enabled": this.state.boost_mode.enabled}, () => __("evse.script.save_failed"));
        await API.save('require_meter/config', {"config": this.state.require_meter_enabled.config}, () => __("evse.script.save_failed"));
        await API.save('evse/led_configuration', this.state.led_configuration, () => __("evse.script.save_failed"));

        if (this.state.is_evse_v2) {
            await API.save('evse/button_configuration', {"button": this.state.button_cfg.button}, () => __("evse.script.save_failed"));
            await API.save('evse/gpio_configuration', this.state.gpio_cfg, () => __("evse.script.gpio_configuration_failed"));
            await API.save('evse/ev_wakeup', {"enabled": this.state.ev_wakeup.enabled}, () => __("evse.script.save_failed"));
            await API.save('evse/phase_auto_switch', {"enabled": this.state.phase_auto_switch.enabled}, () => __("evse.script.save_failed"));
            await API.save('evse/phases_connected', this.state.phases_connected, () => __("evse.script.save_failed"));
        }

        super.sendSave(t, cfg);
    }

    //TODO: Substitute hardcoded values after evse-reset-api is available.

    override async sendReset(t: "charge_limits/default_limits") {
        await API.save('evse/auto_start_charging', {"auto_start_charging": true}, () => __("evse.script.save_failed"));
        await API.save('evse/boost_mode', {"enabled": false}, () => __("evse.script.save_failed"));
        await API.reset('require_meter/config', () => __("evse.script.save_failed"));
        await API.reset('evse/led_configuration', () => __("evse.script.save_failed"));

        if (this.state.is_evse_v2) {
            await API.save('evse/button_configuration', {"button": 2}, () => __("evse.script.save_failed"));
            await API.save('evse/gpio_configuration', {"input": 0, "output": 1, "shutdown_input": 0}, () => __("evse.script.gpio_configuration_failed"));
            await API.save('evse/ev_wakeup', {"enabled": true}, () => __("evse.script.save_failed"));
            await API.save('evse/phase_auto_switch', {"enabled": true}, () => __("evse.script.save_failed"));
            await API.save('evse/phases_connected', {"phases": 3}, () => __("evse.script.save_failed"));
        }

        super.sendReset(t);
    }

    override getIsModified(t: "charge_limits/default_limits"): boolean {
        let result = false;

        result ||= API.is_modified('evse/auto_start_charging');
        result ||= API.is_modified('evse/boost_mode');
        result ||= API.is_modified('require_meter/config');
        result ||= API.is_modified('evse/led_configuration');

        if (this.state.is_evse_v2) {
            result ||= API.is_modified('evse/button_configuration');
            result ||= API.is_modified('evse/gpio_configuration');
            result ||= API.is_modified('evse/ev_wakeup');
            result ||= API.is_modified('evse/phase_auto_switch');
            result ||= API.is_modified('evse/phases_connected');
        }

        result ||= super.getIsModified(t);
        return result;
    }

    render(props: {}, s: EVSESettingsState & ChargeLimitsConfig) {
        if (!util.render_allowed() || !API.hasFeature("evse"))
            return <SubPage name="evse_settings" />;

        let {
            button_cfg,
            slots,
            gpio_cfg,
            ev_wakeup,
            phase_auto_switch,
            phases_connected,
            boost_mode,
            auto_start_charging,
            require_meter_enabled,
            led_configuration} = s;

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

        return <SubPage name="evse_settings">
                <ConfigForm id="evse_settings" title={__("evse.content.evse_settings")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("evse.content.auto_start_description")} label_muted={__("evse.content.auto_start_description_muted")}>
                        <Switch desc={__("evse.content.auto_start_enable")}
                                checked={!auto_start_charging.auto_start_charging}
                                onClick={async () => this.setState({auto_start_charging: {...auto_start_charging, auto_start_charging: !auto_start_charging.auto_start_charging}})}/>
                    </FormRow>

                    <FormRow label={__("evse.content.enable_led_api")}>
                        <Switch onClick={async () => this.setState({led_configuration: {enable_api: !led_configuration.enable_api}})}
                                checked={led_configuration.enable_api}
                                desc={__("evse.content.enable_led_api_desc")}/>
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

                    {!this.state.is_evse_v2 ? undefined :
                        <>
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
                                            ["3",__("evse.content.gpio_4200w_on_open")],
                                            ["4",__("evse.content.gpio_4200w_on_close")],
                                        ]}
                                    value={gpio_cfg.shutdown_input}
                                    onValue={async (v) => {
                                        let cfg = gpio_cfg;
                                        cfg.shutdown_input = parseInt(v);
                                        this.setState({gpio_cfg: cfg});
                                    }}
                            />
                        </FormRow>

                        {this.state.is_evse_v3 ? undefined :
                        <>
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
                        </>
                        }

                        <FormRow label={__("evse.content.ev_wakeup_desc")} label_muted={__("evse.content.ev_wakeup_desc_muted")}>
                            <Switch desc={__("evse.content.ev_wakeup")}
                                    checked={ev_wakeup.enabled}
                                    onClick={async () => this.setState({ev_wakeup: {enabled: !ev_wakeup.enabled}})}/>
                        </FormRow>

                        {!this.state.is_evse_v3 || !this.state.have_meter ? undefined :
                            <FormRow label={__("evse.content.phase_auto_switch_desc")} label_muted={__("evse.content.phase_auto_switch_desc_muted")}>
                                <Switch desc={__("evse.content.phase_auto_switch")}
                                        checked={phase_auto_switch.enabled}
                                        onClick={async () => this.setState({phase_auto_switch: {enabled: !phase_auto_switch.enabled}})}/>
                            </FormRow>
                        }

                        {!this.state.is_evse_v2 ? undefined :
                            <FormRow label={__("evse.content.phases_connected")} label_muted={__("evse.content.phases_connected_muted")}>
                                <InputSelect items={[
                                                ["1",__("evse.content.phases_connected_1")],
                                                ["3",__("evse.content.phases_connected_3")]
                                            ]}
                                        value={phases_connected.phases}
                                        onValue={(v) => {
                                            this.setState({phases_connected: {"phases":parseInt(v)}});
                                        }}
                                />
                            </FormRow>
                        }
                        </>
                    }
                </ConfigForm>
        </SubPage>;
    }
}
