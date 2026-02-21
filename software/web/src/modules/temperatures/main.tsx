/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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
import { h, Fragment } from "preact";
import { __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { FormRow } from "../../ts/components/form_row";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { InputFloat } from "../../ts/components/input_float";
import { InputText } from "../../ts/components/input_text";
import { Thermometer } from "react-feather";

const INT16_MAX = 32767;
const INT16_MIN = -32768;

export function TemperaturesNavbar() {
    return <NavbarItem name="temperatures" module="temperatures" title={__("temperatures.navbar.temperatures")} symbol={<Thermometer />} />;
}

export function is_temperatures_enabled() {
    return API.get("temperatures/config").enable;
}

// Get temperature in °C from the raw value (°C * 100)
function format_temperature(raw: number): string {
    if (raw === INT16_MAX || raw === INT16_MIN) {
        return __("temperatures.content.no_data");
    }
    return util.toLocaleFixed(raw / 100, 1) + " °C";
}

type TemperaturesConfig = API.getType["temperatures/config"];

interface TemperaturesState {
    state: API.getType["temperatures/state"];
    temperatures: API.getType["temperatures/temperatures"];
}

export class Temperatures extends ConfigComponent<"temperatures/config", {}, TemperaturesState> {
    constructor() {
        super('temperatures/config',
              () => __("temperatures.script.save_failed"));

        util.addApiEventListener("temperatures/state", () => {
            this.setState({state: API.get("temperatures/state")});
        });

        util.addApiEventListener("temperatures/temperatures", () => {
            this.setState({temperatures: API.get("temperatures/temperatures")});
        });
    }

    render(props: {}, state: TemperaturesState & TemperaturesConfig) {
        if (!util.render_allowed()) {
            return <SubPage name="temperatures" />;
        }

        const is_configured = state.lat !== 0 || state.lon !== 0;

        return (
            <SubPage name="temperatures" title={__("temperatures.content.temperatures")}>
                {state.enable &&
                    <SubPage.Status>
                        {!is_configured ? (
                            <FormRow>
                                <InputText value={__("temperatures.content.not_configured")} />
                            </FormRow>
                        ) : (
                            <>
                                <FormRow label={__("temperatures.content.today")} label_muted={state.temperatures?.today_date ? new Date(state.temperatures.today_date * 1000).toLocaleDateString() : ""}>
                                    <div class="row gx-2 gy-1">
                                        <div class="col-md-4">
                                            <div class="input-group">
                                                <span class="input-group-text" style="min-width: 60px;">{__("temperatures.content.min_temp")}</span>
                                                <InputText
                                                    value={state.temperatures ? format_temperature(state.temperatures.today_min) : __("temperatures.content.no_data")}
                                                />
                                            </div>
                                        </div>
                                        <div class="col-md-4">
                                            <div class="input-group">
                                                <span class="input-group-text" style="min-width: 60px;">{__("temperatures.content.avg_temp")}</span>
                                                <InputText
                                                    value={state.temperatures ? format_temperature(state.temperatures.today_avg) : __("temperatures.content.no_data")}
                                                />
                                            </div>
                                        </div>
                                        <div class="col-md-4">
                                            <div class="input-group">
                                                <span class="input-group-text" style="min-width: 60px;">{__("temperatures.content.max_temp")}</span>
                                                <InputText
                                                    value={state.temperatures ? format_temperature(state.temperatures.today_max) : __("temperatures.content.no_data")}
                                                />
                                            </div>
                                        </div>
                                    </div>
                                </FormRow>
                                <FormRow label={__("temperatures.content.tomorrow")} label_muted={state.temperatures?.tomorrow_date ? new Date(state.temperatures.tomorrow_date * 1000).toLocaleDateString() : ""}>
                                    <div class="row gx-2 gy-1">
                                        <div class="col-md-4">
                                            <div class="input-group">
                                                <span class="input-group-text" style="min-width: 60px;">{__("temperatures.content.min_temp")}</span>
                                                <InputText
                                                    value={state.temperatures ? format_temperature(state.temperatures.tomorrow_min) : __("temperatures.content.no_data")}
                                                />
                                            </div>
                                        </div>
                                        <div class="col-md-4">
                                            <div class="input-group">
                                                <span class="input-group-text" style="min-width: 60px;">{__("temperatures.content.avg_temp")}</span>
                                                <InputText
                                                    value={state.temperatures ? format_temperature(state.temperatures.tomorrow_avg) : __("temperatures.content.no_data")}
                                                />
                                            </div>
                                        </div>
                                        <div class="col-md-4">
                                            <div class="input-group">
                                                <span class="input-group-text" style="min-width: 60px;">{__("temperatures.content.max_temp")}</span>
                                                <InputText
                                                    value={state.temperatures ? format_temperature(state.temperatures.tomorrow_max) : __("temperatures.content.no_data")}
                                                />
                                            </div>
                                        </div>
                                    </div>
                                </FormRow>
                                <FormRow label={__("temperatures.content.last_update")}>
                                    <InputText
                                        value={state.state?.last_sync ? new Date(state.state.last_sync * 60 * 1000).toLocaleString() : __("temperatures.content.unknown")}
                                    />
                                </FormRow>
                                <FormRow label={__("temperatures.content.next_update")}>
                                    <InputText
                                        value={state.state?.next_check ? new Date(state.state.next_check * 60 * 1000).toLocaleString() : __("temperatures.content.unknown")}
                                    />
                                </FormRow>
                            </>
                        )}
                    </SubPage.Status>
                }
                <SubPage.Config id="temperatures_config_form"
                            isModified={false}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("temperatures.content.enable_temperatures")} label_muted={__("temperatures.content.temperatures_muted")(state.api_url)} help={__("temperatures.content.enable_temperatures_help")}>
                        <Switch desc={__("temperatures.content.temperatures_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}
                        />
                    </FormRow>
                    <FormRow label={__("temperatures.content.latitude")} label_muted={__("temperatures.content.latitude_muted")}>
                        <InputFloat
                            required
                            unit="°"
                            value={state.lat}
                            onValue={(v) => this.setState({lat: v})}
                            digits={4}
                            min={-900000}
                            max={900000}
                        />
                    </FormRow>
                    <FormRow label={__("temperatures.content.longitude")} label_muted={__("temperatures.content.longitude_muted")}>
                        <InputFloat
                            required
                            unit="°"
                            value={state.lon}
                            onValue={(v) => this.setState({lon: v})}
                            digits={4}
                            min={-1800000}
                            max={1800000}
                        />
                    </FormRow>
                </SubPage.Config>
            </SubPage>
        );
    }
}

export function pre_init() {
}

export function init() {
}
