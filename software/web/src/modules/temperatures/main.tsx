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
import { h, Fragment, createRef } from "preact";
import { effect } from "@preact/signals-core";
import { __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { FormRow } from "../../ts/components/form_row";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { InputFloat } from "../../ts/components/input_float";
import { InputText } from "../../ts/components/input_text";
import { InputSelect } from "../../ts/components/input_select";
import { UplotLoader } from "../../ts/components/uplot_loader";
import { UplotData, UplotWrapperB, UplotPath } from "../../ts/components/uplot_wrapper_2nd";
import { TemperatureSource } from "./temperature_source.enum";

const INT16_MAX = 32767;
const INT16_MIN = -32768;

// Compute min/avg/max for a slice of the hourly array.
// Returns {min, avg, max} in raw °C*10 units, or sentinel values if the slice is empty.
function compute_slice_stats(hourly: number[], start: number, end: number): {min: number, avg: number, max: number} {
    if (start < 0) start = 0;
    if (end > hourly.length) end = hourly.length;
    if (start >= end) {
        return {min: INT16_MAX, avg: INT16_MAX, max: INT16_MIN};
    }

    let min_val = INT16_MAX;
    let max_val = INT16_MIN;
    let sum = 0;
    for (let i = start; i < end; i++) {
        const v = hourly[i];
        if (v < min_val) min_val = v;
        if (v > max_val) max_val = v;
        sum += v;
    }
    return {min: min_val, avg: Math.round(sum / (end - start)), max: max_val};
}

// Compute today/tomorrow stats from the flat hourly array + first_date.
// first_date is the UTC timestamp of local midnight (as aligned by timezone=auto).
// We determine the today/tomorrow boundary by finding local midnight of today
// in the browser's timezone, then computing the array index offset.
function compute_day_stats(first_date: number, hourly: number[]): {today: {min: number, avg: number, max: number}, tomorrow: {min: number, avg: number, max: number}} {
    const sentinel = {min: INT16_MAX, avg: INT16_MAX, max: INT16_MIN};
    if (!first_date || !hourly || hourly.length < 47) {
        return {today: sentinel, tomorrow: sentinel};
    }

    // Find local midnight of today in the browser's timezone
    const now = new Date();
    const today_midnight = new Date(now.getFullYear(), now.getMonth(), now.getDate()).getTime() / 1000;
    const tomorrow_midnight = new Date(now.getFullYear(), now.getMonth(), now.getDate() + 1).getTime() / 1000;
    const day_after_midnight = new Date(now.getFullYear(), now.getMonth(), now.getDate() + 2).getTime() / 1000;

    // Convert to array indices: each element is 1 hour apart starting at first_date
    const today_start_idx = Math.floor((today_midnight - first_date) / 3600);
    const tomorrow_start_idx = Math.floor((tomorrow_midnight - first_date) / 3600);
    const day_after_idx = Math.floor((day_after_midnight - first_date) / 3600);

    return {
        today: compute_slice_stats(hourly, today_start_idx, tomorrow_start_idx),
        tomorrow: compute_slice_stats(hourly, tomorrow_start_idx, day_after_idx),
    };
}

export function TemperaturesNavbar() {
    return <NavbarItem name="temperatures" module="temperatures" title={__("temperatures.navbar.temperatures")} symbol={
        <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" fill="var(--bs-body-bg)" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <path d="M12 9C11.1077 8.98562 10.2363 9.27003 9.52424 9.808C8.81222 10.346 8.30055 11.1066 8.07061 11.9688C7.84068 12.8311 7.90568 13.7455 8.25529 14.5665C8.6049 15.3876 9.21904 16.0682 10 16.5M12 3V5M6.6 18.4L5.2 19.8M4 13H2M6.6 7.6L5.2 6.2M20 14.5351V4C20 2.89543 19.1046 2 18 2C16.8954 2 16 2.89543 16 4V14.5351C14.8044 15.2267 14 16.5194 14 18C14 20.2091 15.7909 22 18 22C20.2091 22 22 20.2091 22 18C22 16.5194 21.1956 15.2267 20 14.5351Z"/>
        </svg>
    } />;
}

export function is_temperatures_enabled() {
    return API.get("temperatures/config").enable;
}

// Get temperature in °C from the raw value (°C * 10)
function format_temperature(raw: number): string {
    if (raw === INT16_MAX || raw === INT16_MIN) {
        return __("temperatures.content.no_data");
    }
    return util.toLocaleFixed(raw / 10, 1) + " °C";
}

// Interpolate the current temperature from the hourly array (like firmware get_current()).
// first_date_seconds is the UTC timestamp in seconds of the first array element.
// Returns the interpolated value in raw °C*10, or INT16_MAX if unavailable.
function get_current_temperature(first_date_seconds: number, temperatures: number[]): number {
    if (!first_date_seconds || !temperatures || temperatures.length < 47) {
        return INT16_MAX;
    }

    const now_utc = Math.floor(Date.now() / 1000);

    if (now_utc < first_date_seconds) {
        return temperatures[0];
    }

    const seconds_since_start = now_utc - first_date_seconds;
    const hour_index = Math.floor(seconds_since_start / 3600);

    if (hour_index >= temperatures.length - 1) {
        return temperatures[temperatures.length - 1];
    }

    // Interpolate between hour_index and hour_index + 1
    const t0 = temperatures[hour_index];
    const t1 = temperatures[hour_index + 1];
    const seconds_into_hour = seconds_since_start - hour_index * 3600;

    return Math.round(t0 + (t1 - t0) * seconds_into_hour / 3600);
}

type TemperaturesConfig = API.getType["temperatures/config"];

interface TemperaturesState {
    state: API.getType["temperatures/state"];
    temperatures: API.getType["temperatures/temperatures"];
    config_enable: boolean;
}

export class Temperatures extends ConfigComponent<"temperatures/config", {}, TemperaturesState> {
    uplot_loader_ref  = createRef();
    uplot_wrapper_ref = createRef();

    constructor() {
        super('temperatures/config',
              () => __("temperatures.script.save_failed"));

        util.addApiEventListener("temperatures/state", () => {
            this.setState({state: API.get("temperatures/state")});
        });

        util.addApiEventListener("temperatures/temperatures", () => {
            this.setState({temperatures: API.get("temperatures/temperatures")});
            this.update_uplot();
        });

        util.addApiEventListener("temperatures/config", () => {
            let config = API.get("temperatures/config");
            this.setState({config_enable: config.enable});
        });

        // Update vertical "now" line on time change
        effect(() => this.update_uplot());
    }

    override async sendSave(topic: "temperatures/config", config: TemperaturesConfig) {
        this.setState({config_enable: config.enable}); // avoid round trip time
        await super.sendSave(topic, config);
    }

    update_uplot() {
        // Touch the reactive signal so effect() tracks the dependency
        let date_now = util.get_date_now_1m_update_rate();

        if (this.uplot_wrapper_ref.current == null) {
            return;
        }

        const temps = this.state.temperatures;
        let data: UplotData;

        if (!temps || !temps.first_date || !temps.temperatures || temps.temperatures.length < 47) {
            data = {
                keys: [null],
                names: [null],
                values: [null],
                stacked: [null],
                paths: [null],
            }
        } else {
            // first_date is in minutes; convert to seconds
            const first_date_seconds = temps.first_date * 60;

            data = {
                keys: [null, 'temperature'],
                names: [null, __("temperatures.content.temperature")],
                values: [[], []],
                stacked: [null, false],
                paths: [null, UplotPath.Line],
                default_visibilty: [null, true],
                lines_vertical: [],
            }

            for (let i = 0; i < temps.temperatures.length; i++) {
                data.values[0].push(first_date_seconds + i * 3600);
                data.values[1].push(temps.temperatures[i] / 10);
            }

            // Duplicate last point to close the line
            data.values[0].push(first_date_seconds + temps.temperatures.length * 3600 - 1);
            data.values[1].push(temps.temperatures[temps.temperatures.length - 1] / 10);

            // Add vertical line at current time
            const diff_seconds = Math.floor(date_now / 1000) - first_date_seconds;
            const index = Math.floor(diff_seconds / 3600);
            if (index >= 0 && index < temps.temperatures.length) {
                data.lines_vertical.push({'index': index, 'text': __("temperatures.content.now"), 'color': [64, 64, 64, 0.2]});
            }
        }

        this.uplot_loader_ref.current.set_data(data && data.keys.length > 1);
        this.uplot_wrapper_ref.current.set_data(data);
    }

    render(props: {}, state: TemperaturesState & TemperaturesConfig) {
        if (!util.render_allowed()) {
            return <SubPage name="temperatures" />;
        }

        const is_configured = state.lat !== 0 || state.lon !== 0;
        const is_weather_service = state.source == TemperatureSource.WeatherService;

        const temps = state.temperatures;
        const day_stats = temps ? compute_day_stats(temps.first_date * 60, temps.temperatures) : null;

        return (
            <SubPage name="temperatures" title={__("temperatures.content.temperatures")}>
                {state.config_enable &&
                    <SubPage.Status>
                        {is_weather_service && !is_configured ? (
                            <FormRow>
                                <InputText value={__("temperatures.content.not_configured")} />
                            </FormRow>
                        ) : (
                            <>
                                <FormRow label={__("temperatures.content.temperature_forecast")}>
                                    <div class="card">
                                        <div style="position: relative;">
                                            <UplotLoader
                                                ref={this.uplot_loader_ref}
                                                show
                                                marker_class="h4"
                                                no_data={__("temperatures.content.no_data")}
                                                loading={__("temperatures.content.loading")}>
                                                <UplotWrapperB
                                                    ref={this.uplot_wrapper_ref}
                                                    class="temperatures-chart"
                                                    sub_page="temperatures"
                                                    color_cache_group="temperatures.default"
                                                    show
                                                    on_mount={() => this.update_uplot()}
                                                    legend_time_label={__("temperatures.content.time")}
                                                    legend_time_with_minutes
                                                    aspect_ratio={3}
                                                    x_format={{hour: '2-digit', minute: '2-digit'}}
                                                    x_padding_factor={0}
                                                    x_include_date
                                                    y_unit="°C"
                                                    y_label={__("temperatures.content.temperature_degc")}
                                                    y_digits={1}
                                                    only_show_visible
                                                    padding={[30, 15, null, 5]}
                                                />
                                            </UplotLoader>
                                        </div>
                                    </div>
                                </FormRow>
                                <FormRow label={__("temperatures.content.current_temperature")} label_muted={new Date().toLocaleTimeString([], {hour: '2-digit', minute: '2-digit'})}>
                                    <InputText
                                        value={temps ? format_temperature(get_current_temperature(temps.first_date * 60, temps.temperatures)) : __("temperatures.content.no_data")}
                                    />
                                </FormRow>
                                <FormRow label={__("temperatures.content.today")} label_muted={temps?.first_date ? new Date(temps.first_date * 60 * 1000).toLocaleDateString() : ""}>
                                    <div class="row gx-2 gy-1">
                                        <div class="col-md-4">
                                            <div class="input-group">
                                                <span class="input-group-text" style="min-width: 60px;">{__("temperatures.content.min_temp")}</span>
                                                <InputText
                                                    value={day_stats ? format_temperature(day_stats.today.min) : __("temperatures.content.no_data")}
                                                />
                                            </div>
                                        </div>
                                        <div class="col-md-4">
                                            <div class="input-group">
                                                <span class="input-group-text" style="min-width: 60px;">{__("temperatures.content.avg_temp")}</span>
                                                <InputText
                                                    value={day_stats ? format_temperature(day_stats.today.avg) : __("temperatures.content.no_data")}
                                                />
                                            </div>
                                        </div>
                                        <div class="col-md-4">
                                            <div class="input-group">
                                                <span class="input-group-text" style="min-width: 60px;">{__("temperatures.content.max_temp")}</span>
                                                <InputText
                                                    value={day_stats ? format_temperature(day_stats.today.max) : __("temperatures.content.no_data")}
                                                />
                                            </div>
                                        </div>
                                    </div>
                                </FormRow>
                                <FormRow label={__("temperatures.content.tomorrow")} label_muted={temps?.first_date ? new Date((temps.first_date * 60 + 86400) * 1000).toLocaleDateString() : ""}>
                                    <div class="row gx-2 gy-1">
                                        <div class="col-md-4">
                                            <div class="input-group">
                                                <span class="input-group-text" style="min-width: 60px;">{__("temperatures.content.min_temp")}</span>
                                                <InputText
                                                    value={day_stats ? format_temperature(day_stats.tomorrow.min) : __("temperatures.content.no_data")}
                                                />
                                            </div>
                                        </div>
                                        <div class="col-md-4">
                                            <div class="input-group">
                                                <span class="input-group-text" style="min-width: 60px;">{__("temperatures.content.avg_temp")}</span>
                                                <InputText
                                                    value={day_stats ? format_temperature(day_stats.tomorrow.avg) : __("temperatures.content.no_data")}
                                                />
                                            </div>
                                        </div>
                                        <div class="col-md-4">
                                            <div class="input-group">
                                                <span class="input-group-text" style="min-width: 60px;">{__("temperatures.content.max_temp")}</span>
                                                <InputText
                                                    value={day_stats ? format_temperature(day_stats.tomorrow.max) : __("temperatures.content.no_data")}
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
                                {is_weather_service &&
                                <FormRow label={__("temperatures.content.next_update")}>
                                    <InputText
                                        value={state.state?.next_check ? new Date(state.state.next_check * 60 * 1000).toLocaleString() : __("temperatures.content.unknown")}
                                    />
                                </FormRow>}
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
                    <FormRow label={__("temperatures.content.enable_temperatures")} help={__("temperatures.content.enable_temperatures_help")}>
                        <Switch desc={is_weather_service ? __("temperatures.content.temperatures_desc") : __("temperatures.content.temperatures_push_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}
                        />
                    </FormRow>
                    <FormRow label={__("temperatures.content.source")}
                             label_muted={is_weather_service
                                 ? __("temperatures.content.source_weather_service_desc")(state.api_url)
                                 : __("temperatures.content.source_push_desc")}>
                        <InputSelect
                            items={[
                                ["0", __("temperatures.content.source_weather_service")],
                                ["1", __("temperatures.content.source_push")],
                            ]}
                            value={state.source}
                            onValue={(v) => this.setState({source: parseInt(v)})}
                        />
                    </FormRow>
                    {is_weather_service &&
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
                    </FormRow>}
                    {is_weather_service &&
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
                    </FormRow>}
                </SubPage.Config>
            </SubPage>
        );
    }
}

export function pre_init() {
}

export function init() {
}
