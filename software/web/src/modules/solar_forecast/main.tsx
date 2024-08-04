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
import { h, Fragment } from "preact";
import { __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Collapse } from "react-bootstrap";
import { InputFloat } from "../../ts/components/input_float";
import { Sunrise } from "react-feather";
import { Table } from "../../ts/components/table";
import { InputNumber } from "../../ts/components/input_number";

const SOLAR_FORECAST_PLANES = 6;

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

export class SolarForecast extends ConfigComponent<"solar_forecast/config", {}, SolarForecastState> {
    constructor() {
        super('solar_forecast/config',
              __("solar_forecast.script.save_failed"));

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
                __("solar_forecast.script.save_failed"),
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
                    value={this.state.plane_config_tmp.kwp}
                    onValue={(v) => this.setState({plane_config_tmp: {...this.state.plane_config_tmp, kwp: v}})}
                    digits={2}
                    min={0}
                    max={100000}
                />
            </FormRow>
        </>]
    }

    render(props: {}, state: SolarForecastState & SolarForecastConfig) {
        if (!util.render_allowed()) {
            return <SubPage name="solar_forecast" />;
        }

        function get_next_free_plane_index() {
            for (let i = 0; i < SOLAR_FORECAST_PLANES; i++) {
                if (!state.plane_configs[i].active) {
                    return i;
                }
            }

            return -1;
        }

        function get_active_planes() {
            let active_planes = [];
            for (let i = 0; i < SOLAR_FORECAST_PLANES; i++) {
                if (state.plane_configs[i].active) {
                    active_planes.push(i);
                }
            }
            return active_planes;
        }

        return (
            <SubPage name="solar_forecast">
                <ConfigForm
                    id="plane_configs_config_form"
                    title={__("solar_forecast.content.solar_forecast")}
                    isModified={this.isModified()}
                    isDirty={this.isDirty()}
                    onSave={this.save}
                    onReset={this.reset}
                    onDirtyChange={this.setDirty}>
                    <FormRow label={__("solar_forecast.content.solar_forecast")} label_muted={__("solar_forecast.content.solar_forecast_muted")}>
                        <Switch desc={__("solar_forecast.content.solar_forecast_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}
                        />
                    </FormRow>
                    <Collapse in={state.enable}>
                        <div>
                            <FormRow label={__("solar_forecast.content.planes")}>
                                <Table columnNames={[__("solar_forecast.content.table_latitude"), __("solar_forecast.content.table_longitude"), __("solar_forecast.content.table_declination"), __("solar_forecast.content.table_azimuth"), __("solar_forecast.content.table_kwp")]}
                                    rows={get_active_planes().map((active_plane_index) => {
                                        let plane_config = state.plane_configs[active_plane_index];
                                        return {
                                            columnValues: [
                                                (plane_config.latitude/10000).toFixed(4) + "°",
                                                (plane_config.longitude/10000).toFixed(4) + "°",
                                                plane_config.declination + "°",
                                                plane_config.azimuth + "°",
                                                (plane_config.kwp/100).toFixed(2) + "kWp",
                                            ],
                                            editTitle: __("solar_forecast.content.edit_plane_config_title"),
                                            onEditShow: async () => this.setState({plane_config_tmp: {...plane_config}}),
                                            onEditGetChildren: () => this.on_get_children(),
                                            onEditSubmit: async () => {
                                                this.setState({plane_configs: {...state.plane_configs, [active_plane_index]: state.plane_config_tmp}});
                                                this.setDirty(true);
                                            },
                                            onRemoveClick: async () => {
                                                this.setState({plane_configs: {...state.plane_configs, [active_plane_index]: {active: false, latitude: 0, longitude: 0, declination: 0, azimuth: 0, kwp: 0}}});
                                                this.setDirty(true);
                                            }}
                                        })
                                    }
                                    addEnabled={get_active_planes().length < SOLAR_FORECAST_PLANES}
                                    addTitle={__("solar_forecast.content.add_plane_config_title")}
                                    addMessage={__("solar_forecast.content.add_plane_config_message")}
                                    onAddShow={async () => {
                                        this.setState({plane_config_tmp: {active: true, latitude: 0, longitude: 0, declination: 0, azimuth: 0, kwp: 0}})
                                    }}
                                    onAddGetChildren={() => this.on_get_children()}
                                    onAddSubmit={async () => {
                                        this.setState({plane_configs: {...state.plane_configs, [get_next_free_plane_index()]: state.plane_config_tmp}});
                                        this.setDirty(true);
                                    }}
                                />
                            </FormRow>
                        </div>
                    </Collapse>
                </ConfigForm>
            </SubPage>
        );
    }
}

export function init() {
}
