/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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
import { createRef, h, Fragment, Component, RefObject } from "preact";
import { __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { InputFloat } from "../../ts/components/input_float";
import { Sunrise } from "react-feather";
import { Table } from "../../ts/components/table";
import { InputNumber } from "../../ts/components/input_number";
import { FormSeparator } from "../../ts/components/form_separator";
import { UplotLoader } from "../../ts/components/uplot_loader";
import { UplotData, UplotWrapper } from "../../ts/components/uplot_wrapper_2nd";
import { InputText } from "../../ts/components/input_text";
import { CollapsedSection } from "../../ts/components/collapsed_section";
import { StatusSection } from "../../ts/components/status_section";

const SOLAR_FORECAST_PLANES = 6;

function get_active_planes() {
    let active_planes = [];
    for (let i = 0; i < SOLAR_FORECAST_PLANES; i++) {
        const plane_config = API.get_unchecked(`solar_forecast/planes/${i}/config`);
        if (plane_config.active) {
            active_planes.push(i);
        }
    }
    return active_planes;
}

function does_forecast_exist() {
    for (let i = 0; i < SOLAR_FORECAST_PLANES; i++) {
        const plane_forecast = API.get_unchecked(`solar_forecast/planes/${i}/forecast`);
        if (plane_forecast.length > 0) {
            return true;
        }
    }
    return false;
}

function get_timestamp_today_00_00_in_seconds() {
    return Math.floor(new Date(util.get_date_now_1m_update_rate()).setHours(0, 0, 0, 0) / 1000);
}

function forecast_time_between(first_date: number, index: number, start: number, end: number) {
    let index_timestamp_seconds = (first_date + index*60)*60;
    return (index_timestamp_seconds >= start) && (index_timestamp_seconds <= end);
}

function get_kwh_now_to_midnight() {
    let start         = Math.floor(new Date(util.get_date_now_1m_update_rate()).setMinutes(0, 0, 0) / 1000);
    let end           = get_timestamp_today_00_00_in_seconds() + 60*60*24 - 1;
    let active_planes = get_active_planes();
    let wh            = 0.0;
    let count         = 0
    for (const plane of active_planes) {
        const plane_forecast = API.get_unchecked(`solar_forecast/planes/${plane}/forecast`);
        for (let index = 0; index < plane_forecast.forecast.length; index++) {
            if (forecast_time_between(plane_forecast.first_date, index, start, end)) {
                wh += plane_forecast.forecast[index] || 0.0;
                count++;
            }
        }
    }

    if (count == 0) {
        return NaN;
    }

    return wh/1000.0;
}

export function is_solar_forecast_enabled() {
    return API.get("solar_forecast/config").enable;
}

export function get_kwh_today() {
    let start         = get_timestamp_today_00_00_in_seconds();
    let end           = start + 60*60*24 - 1;
    let active_planes = get_active_planes();
    let wh            = 0.0;
    let count         = 0;
    for (const plane of active_planes) {
        const plane_forecast = API.get_unchecked(`solar_forecast/planes/${plane}/forecast`);
        for (let index = 0; index < plane_forecast.forecast.length; index++) {
            if (forecast_time_between(plane_forecast.first_date, index, start, end)) {
                wh += plane_forecast.forecast[index] || 0.0;
                count++;
            }
        }
    }

    if (count == 0) {
        return NaN;
    }

    return wh/1000.0;
}

export function get_kwh_tomorrow() {
    let start         = get_timestamp_today_00_00_in_seconds() + 60*60*24;
    let end           = start + 60*60*24 - 1;
    let active_planes = get_active_planes();
    let wh            = 0.0;
    let count         = 0;
    for (const plane of active_planes) {
        const plane_forecast = API.get_unchecked(`solar_forecast/planes/${plane}/forecast`);
        for (let index = 0; index < plane_forecast.forecast.length; index++) {
            if (forecast_time_between(plane_forecast.first_date, index, start, end)) {
                wh += plane_forecast.forecast[index] || 0.0;
                count++;
            }
        }
    }

    if (count == 0) {
        return NaN;
    }

    return wh/1000.0;
}


export function SolarForecastNavbar() {
    return <NavbarItem name="solar_forecast" title={__("solar_forecast.navbar.solar_forecast")} symbol={<Sunrise />} hidden={false}/>;
}

type PlaneConfig = (API.getType['planes/0/plane_config'])
type SolarForecastConfig = API.getType["solar_forecast/config"];

interface SolarForecastState {
    state: API.getType["solar_forecast/state"];
    plane_states: {[plane_index: number]: Readonly<API.getType['planes/0/plane_state']>};
    plane_configs: {[plane_index: number]: PlaneConfig};
    plane_forecasts: {[plane_index: number]: API.getType['planes/0/plane_forecast']};
    plane_config_tmp: PlaneConfig;
}

export class SolarForecast extends ConfigComponent<"solar_forecast/config", {status_ref?: RefObject<SolarForecastStatus>}, SolarForecastState> {
    uplot_loader_ref  = createRef();
    uplot_wrapper_ref = createRef();

    constructor() {
        super('solar_forecast/config',
              () => __("solar_forecast.script.save_failed"),
              undefined, {});

        util.addApiEventListener("solar_forecast/state", () => {
            this.setState({state: API.get("solar_forecast/state")});
        });

        for (let plane_index = 0; plane_index < SOLAR_FORECAST_PLANES; ++plane_index) {
            util.addApiEventListener_unchecked(`solar_forecast/planes/${plane_index}/state`, () => {
                let state = API.get_unchecked(`solar_forecast/planes/${plane_index}/state`);

                this.setState((prevState) => ({
                    plane_states: {
                        ...prevState.plane_states,
                        [plane_index]: state
                    }
                }));
            });

            util.addApiEventListener_unchecked(`solar_forecast/planes/${plane_index}/forecast`, () => {
                let forecast = API.get_unchecked(`solar_forecast/planes/${plane_index}/forecast`);

                this.setState((prevState) => ({
                    plane_forecasts: {
                        ...prevState.plane_forecasts,
                        [plane_index]: forecast
                    }
                }));

                this.update_uplot();
            });

            util.addApiEventListener_unchecked(`solar_forecast/planes/${plane_index}/config`, () => {
                let config = API.get_unchecked(`solar_forecast/planes/${plane_index}/config`);

                this.setState((prevState) => ({
                    plane_configs: {
                        ...prevState.plane_configs,
                        [plane_index]: config
                    }
                }));

                if (!this.isDirty()) {
                    this.setState((prevState) => ({
                        plane_configs: {
                            ...prevState.plane_configs,
                            [plane_index]: config
                        }
                    }));
                }
            });
        }
    }
    override async sendSave(topic: "solar_forecast/config", config: SolarForecastConfig) {
        for (let plane_index = 0; plane_index < SOLAR_FORECAST_PLANES; plane_index++) {
            await API.save_unchecked(
                `solar_forecast/planes/${plane_index}/config`,
                this.state.plane_configs[plane_index],
                () => __("solar_forecast.script.save_failed"),
                plane_index == SOLAR_FORECAST_PLANES - 1 ? this.reboot_string : undefined);
        }

        await super.sendSave(topic, config);
    }

    override async sendReset(topic: "solar_forecast/config") {
        for (let plane_index = 0; plane_index < SOLAR_FORECAST_PLANES; plane_index++) {
            await API.reset_unchecked(`solar_forecast/planes/${plane_index}/config`, this.error_string, this.reboot_string);
        }

        await super.sendReset(topic);
    }

    override getIsModified(topic: "solar_forecast/config"): boolean {
        for (let plane_index = 0; plane_index < SOLAR_FORECAST_PLANES; plane_index++) {
            if (API.is_modified_unchecked(`solar_forecast/planes/${plane_index}/config`))
                return true;
        }

        return super.getIsModified(topic);
    }

    // FormRows for onEditGetChildren and onAddGetChildren
    on_get_children() {
        return [<>
            <FormRow label={__("solar_forecast.content.plane_config_name")} label_muted={__("solar_forecast.content.plane_config_name_muted")}>
                <InputText
                    value={this.state.plane_config_tmp.name}
                    onValue={(v) => this.setState({plane_config_tmp: {...this.state.plane_config_tmp, name: v}})}
                    minLength={1}
                    maxLength={16}
                />
            </FormRow>
            <FormRow label={__("solar_forecast.content.plane_config_latitude")} label_muted={__("solar_forecast.content.plane_config_latitude_muted")}>
                <InputFloat
                    unit="°"
                    value={this.state.plane_config_tmp.latitude}
                    onValue={(v) => this.setState({plane_config_tmp: {...this.state.plane_config_tmp, latitude: v}})}
                    digits={4}
                    min={-900000}
                    max={900000}
                />
            </FormRow>
            <FormRow label={__("solar_forecast.content.plane_config_longitude")} label_muted={__("solar_forecast.content.plane_config_longitude_muted")}>
                <InputFloat
                    unit="°"
                    value={this.state.plane_config_tmp.longitude}
                    onValue={(v) => this.setState({plane_config_tmp: {...this.state.plane_config_tmp, longitude: v}})}
                    digits={4}
                    min={-1800000}
                    max={1800000}
                />
            </FormRow>
            <FormRow label={__("solar_forecast.content.plane_config_declination")} label_muted={__("solar_forecast.content.plane_config_declination_muted")}>
                <InputNumber
                    unit="°"
                    value={this.state.plane_config_tmp.declination}
                    onValue={(v) => this.setState({plane_config_tmp: {...this.state.plane_config_tmp, declination: v}})}
                    min={0}
                    max={90}
                />
            </FormRow>
            <FormRow label={__("solar_forecast.content.plane_config_azimuth")} label_muted={__("solar_forecast.content.plane_config_azimuth_muted")}>
                <InputNumber
                    unit="°"
                    value={this.state.plane_config_tmp.azimuth}
                    onValue={(v) => this.setState({plane_config_tmp: {...this.state.plane_config_tmp, azimuth: v}})}
                    min={-180}
                    max={180}
                />
            </FormRow>
            <FormRow label={__("solar_forecast.content.plane_config_kwp")} label_muted={__("solar_forecast.content.plane_config_kwp_muted")}>
                <InputFloat
                    unit="kW"
                    value={this.state.plane_config_tmp.wp}
                    onValue={(v) => this.setState({plane_config_tmp: {...this.state.plane_config_tmp, wp: v}})}
                    digits={3}
                    min={0}
                    max={100000}
                />
            </FormRow>
        </>]
    }

    update_uplot() {
        if (this.uplot_wrapper_ref.current == null) {
            return;
        }

        let data: UplotData;

        // Is data to display available?
        let data_available = false;
        let first_index = 0;
        let active_planes = [];
        for (let i = 0; i < SOLAR_FORECAST_PLANES; i++) {
            // Data is available if at least one plane is active and all active planes have forecast data
            if(this.state.plane_configs[i].active && (this.state.plane_forecasts[i].forecast.length > 0)) {
                if (!data_available) {
                    // We use first date from first plane that has data available
                    first_index = i;
                }
                active_planes.push(i);
                data_available = true;
            }
        }

        if (!data_available) {
            data = {
                keys: [null],
                names: [null],
                values: [null],
                filled: [null],
            }
        } else {
            data = {
                keys: [null],
                names: [null],
                values: [[]],
                filled: [null],
            }

            for (const index in active_planes) {
                data.keys.push('plane' + index);
                data.names.push(this.state.plane_configs[index].name);
                data.values.push([]);
                data.filled.push(true);
            }

            let resolution_multiplier = 60;
            for (let i = 0; i < this.state.plane_forecasts[first_index].forecast.length; i++) {
                data.values[0].push(this.state.plane_forecasts[first_index].first_date * 60 + i * 60 * resolution_multiplier);
                let j = 1;
                for (const index in active_planes) {
                    data.values[j].push(this.state.plane_forecasts[index].forecast[i]);
                    j++;
                }
            }

            data.values[0].push(this.state.plane_forecasts[first_index].first_date * 60 + this.state.plane_forecasts[first_index].forecast.length * 60 * resolution_multiplier - 1);
            let j = 1;
            for (const index in active_planes) {
                data.values[j].push(this.state.plane_forecasts[index].forecast[this.state.plane_forecasts[first_index].forecast.length - 1]);
                j++;
            }
        }

        // Show loader or data depending on the availability of data
        this.uplot_loader_ref.current.set_data(data && data.keys.length > 1);
        this.uplot_wrapper_ref.current.set_data(data);
    }

    render(props: {}, state: SolarForecastState & SolarForecastConfig) {
        if (!util.render_allowed()) {
            return <SubPage name="solar_forecast" />;
        }

        function get_plane_info(plane_index: number) {
            let plane_state = state.plane_states[plane_index];
            if (plane_state.last_check == 0) {
                // Darauf hinweisen das noch nicht gespeichert wurde falls isDirty()
                return <div class="form-group row"><span class="col-12">{__("solar_forecast.content.not_set_for_this_plane")}</span></div>;
            } else {
                return <div>
                        <div class="form-group row"><span class="col-4">{__("solar_forecast.content.address_of_pv_plane")}</span><span class="col-8"> {plane_state.place} ({__("solar_forecast.content.resolution")})</span></div>
                        <div class="form-group row"><span class="col-4">{__("solar_forecast.content.last_update_attempt")}</span><span class="col-8">{new Date(plane_state.last_check*60*1000).toLocaleString()}</span></div>
                        <div class="form-group row"><span class="col-4">{__("solar_forecast.content.last_successful_update")}</span><span class="col-8">{new Date(plane_state.last_sync*60*1000).toLocaleString()}</span></div>
                        <div class="form-group row"><span class="col-4">{__("solar_forecast.content.next_update")}</span><span class="col-8">{new Date(plane_state.next_check*60*1000).toLocaleString()}</span></div>
                       </div>;
            }
        }

        function get_active_unsaved_planes() {
            let active_planes = [];
            for (let i = 0; i < SOLAR_FORECAST_PLANES; i++) {
                if (state.plane_configs[i].active) {
                    active_planes.push(i);
                }
            }
            return active_planes;
        }

        function get_next_free_unsaved_plane_index() {
            for (let i = 0; i < SOLAR_FORECAST_PLANES; i++) {
                if (!state.plane_configs[i].active) {
                    return i;
                }
            }

            return -1;
        }

        function get_next_update_string() {
            let now = util.get_date_now_1m_update_rate()/(60*1000);
            if ((state.state.next_api_call == 0) || (get_active_planes().length == 0) || (state.state.next_api_call < now)) {
                return __("util.not_yet_known");
            } else {
                let diff    = state.state.next_api_call - now;
                let hours   = Math.floor(diff / 60);
                let minutes = Math.floor(diff % 60);
                let update_string = (hours == 0) ? `${minutes}m`:`${hours}h ${minutes}m`;
                if ((state.state.rate_limit != -1) && (state.state.rate_remaining != -1)) {
                    update_string += " " + __("solar_forecast.content.remaining_queries")(state.state.rate_remaining, state.state.rate_limit);
                }

                return update_string;
            }
        }

        return (
            <SubPage name="solar_forecast">
                <ConfigForm
                    id="plane_configs_config_form"
                    title={__("solar_forecast.content.solar_forecast")}
                    isModified={false}
                    isDirty={this.isDirty()}
                    onSave={this.save}
                    onReset={this.reset}
                    onDirtyChange={this.setDirty}>
                    <FormRow label={__("solar_forecast.content.enable_solar_forecast")} label_muted={__("solar_forecast.content.solar_forecast_muted")}>
                        <Switch desc={__("solar_forecast.content.solar_forecast_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}
                        />
                    </FormRow>
                    <FormRow label={__("solar_forecast.content.planes")}>
                        <div></div>
                    </FormRow>
                    <Table columnNames={[__("solar_forecast.content.table_name"), __("solar_forecast.content.table_latitude"), __("solar_forecast.content.table_longitude"), __("solar_forecast.content.table_declination"), __("solar_forecast.content.table_azimuth"), __("solar_forecast.content.table_kwp")]}
                        tableTill="lg"
                        rows={get_active_unsaved_planes().map((active_plane_index) => {
                            let plane_config = state.plane_configs[active_plane_index];
                            return {
                                extraValue: get_plane_info(active_plane_index),
                                columnValues: [
                                    plane_config.name,
                                    util.toLocaleFixed(plane_config.latitude / 10000, 4) + "°",
                                    util.toLocaleFixed(plane_config.longitude / 10000, 4) + "°",
                                    plane_config.declination + "°",
                                    plane_config.azimuth + "°",
                                    util.toLocaleFixed(plane_config.wp / 1000, 3) + " kWp",
                                ],
                                editTitle: __("solar_forecast.content.edit_plane_config_title"),
                                onEditShow: async () => this.setState({plane_config_tmp: {...plane_config}}),
                                onEditGetChildren: () => this.on_get_children(),
                                onEditSubmit: async () => {
                                    this.setState({plane_configs: {...state.plane_configs, [active_plane_index]: state.plane_config_tmp}});
                                    this.setDirty(true);
                                },
                                onRemoveClick: async () => {
                                    this.setState({plane_configs: {...state.plane_configs, [active_plane_index]: {active: false, name: "#" + active_plane_index, latitude: 0, longitude: 0, declination: 0, azimuth: 0, wp: 0}}});
                                    this.setDirty(true);
                                }}
                            })
                        }
                        addEnabled={get_active_unsaved_planes().length < SOLAR_FORECAST_PLANES}
                        addTitle={__("solar_forecast.content.add_plane_config_title")}
                        addMessage={get_active_unsaved_planes().length == SOLAR_FORECAST_PLANES ? __("solar_forecast.content.add_plane_config_done") : __("solar_forecast.content.add_plane_config_count")(get_active_unsaved_planes().length, SOLAR_FORECAST_PLANES)}
                        onAddShow={async () => {
                            this.setState({plane_config_tmp: {active: true, name: "#" + get_next_free_unsaved_plane_index(), latitude: 0, longitude: 0, declination: 0, azimuth: 0, wp: 0}})
                        }}
                        onAddGetChildren={() => this.on_get_children()}
                        onAddSubmit={async () => {
                            this.setState({plane_configs: {...state.plane_configs, [get_next_free_unsaved_plane_index()]: state.plane_config_tmp}});
                            this.setDirty(true);
                        }}
                    />
                </ConfigForm>
                <FormSeparator heading={__("solar_forecast.content.solar_forecast_chart_heading")}/>
                <FormRow label={__("solar_forecast.content.next_update_in")} help={__("solar_forecast.content.next_update_in_help")}>
                        <InputText value={get_next_update_string()}/>
                    </FormRow>
                <FormRow label={__("solar_forecast.content.solar_forecast_now_label")} label_muted={("0" + new Date(util.get_date_now_1m_update_rate()).getHours()).slice(-2) + ":00 " + __("solar_forecast.content.time_to") + " 23:59"}>
                    <InputText
                        value={util.get_value_with_unit(get_kwh_now_to_midnight(), "kWh", 2)}
                    />
                </FormRow>
                <FormRow label={__("solar_forecast.content.solar_forecast")} label_muted={__("solar_forecast.content.solar_forecast_today_label_muted")}>
                    <div class="row mx-n1">
                        <div class="col-md-6 px-1">
                            <div class="input-group">
                                <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("solar_forecast.content.solar_forecast_today_label")}</span></div>
                                <InputText
                                    value={util.get_value_with_unit(get_kwh_today(), "kWh", 2)}
                                />
                            </div>
                        </div>
                        <div class="col-md-6 px-1">
                            <div class="input-group">
                                <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("solar_forecast.content.solar_forecast_tomorrow_label")}</span></div>
                                <InputText
                                    value={util.get_value_with_unit(get_kwh_tomorrow(), "kWh", 2)}
                                />
                            </div>
                        </div>
                    </div>
                </FormRow>
                <div>
                    <div style="position: relative;"> {/* this plain div is neccessary to make the size calculation stable in safari. without this div the height continues to grow */}
                        <UplotLoader
                            ref={this.uplot_loader_ref}
                            show={true}
                            marker_class={'h4'}
                            no_data={__("solar_forecast.content.no_data")}
                            loading={__("solar_forecast.content.loading")}>
                            <UplotWrapper
                                ref={this.uplot_wrapper_ref}
                                class="solar-forecast-chart pb-3"
                                sub_page="solar_forecast"
                                color_cache_group="solar_forecast.default"
                                show={true}
                                on_mount={() => this.update_uplot()}
                                legend_time_label={__("solar_forecast.content.time")}
                                legend_time_with_minutes={true}
                                aspect_ratio={3}
                                x_height={50}
                                x_format={{hour: '2-digit', minute: '2-digit'}}
                                x_padding_factor={0}
                                x_include_date={true}
                                y_min={0}
                                y_unit={"W"}
                                y_label={__("solar_forecast.script.power") + " [W]"}
                                y_digits={2}
                                only_show_visible={true}
                                padding={[null, 20, null, null]}
                            />
                        </UplotLoader>
                    </div>
                </div>
            </SubPage>
        );
    }
}

export class SolarForecastStatus extends Component
{
    render() {
        const config = API.get('solar_forecast/config')
        if (!util.render_allowed() || !config.enable)
            return <StatusSection name="solar_forecast" />

        return <StatusSection name="solar_forecast">
            <FormRow label={__("solar_forecast.content.solar_forecast_now_label")} label_muted={("0" + new Date(util.get_date_now_1m_update_rate()).getHours()).slice(-2) + ":00 " + __("solar_forecast.content.time_to") + " 23:59"}>
                <InputText
                    value={util.get_value_with_unit(get_kwh_now_to_midnight(), "kWh", 2)}
                />
            </FormRow>
            <FormRow label={__("solar_forecast.content.solar_forecast")} label_muted={__("solar_forecast.content.solar_forecast_today_label_muted")}>
                <div class="row mx-n1">
                    <div class="col-md-6 px-1">
                        <div class="input-group">
                            <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("solar_forecast.content.solar_forecast_today_label")}</span></div>
                            <InputText
                                value={util.get_value_with_unit(get_kwh_today(), "kWh", 2)}
                            />
                        </div>
                    </div>
                    <div class="col-md-6 px-1">
                        <div class="input-group">
                            <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("solar_forecast.content.solar_forecast_tomorrow_label")}</span></div>
                            <InputText
                                value={util.get_value_with_unit(get_kwh_tomorrow(), "kWh", 2)}
                            />
                        </div>
                    </div>
                </div>
            </FormRow>
        </StatusSection>;
    }
}

export function init() {
}
