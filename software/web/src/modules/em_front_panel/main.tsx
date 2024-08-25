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
import { createRef, h } from "preact";
import { __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Monitor } from "react-feather";
import { Collapse } from "react-bootstrap";
import { InputSelect } from "../../ts/components/input_select";
import { FormSeparator } from "src/ts/components/form_separator";

const FRONT_PANEL_TILES = 6;

export function EMFrontPanelNavbar() {
    return (
        <NavbarItem
            name="front_panel"
            title={__("em_front_panel.navbar.em_front_panel")}
            hidden={false}
            symbol={<Monitor/>
            }
        />
    );
}

type TileConfig = API.getType['tiles/0/config'];
type EMFrontPanelConfig = API.getType["front_panel/config"];

interface EMFrontPanelState {
    tile_configs: {[tile_index: number]: TileConfig};
}

export class EMFrontPanel extends ConfigComponent<"front_panel/config", {}, EMFrontPanelState> {
    static options_tile: [string, string][] = [
        ["0", "Leere Kachel"],
        ["1", "Wallbox"],
        ["2", "Lastmanagement"],
        ["3", "Stromzähler"],
        ["4", "Dynamischer Strompreis"],
        ["5", "Solarprognose"],
        ["6", "Status des Energy Managers"],
    ]

    static options_wallbox: [string, string][] = [
        ["0", "Wallbox 1"],
        ["1", "Wallbox 2"],
        ["2", "Wallbox 3"],
        ["3", "Wallbox 4"],
        ["4", "Wallbox 5"],
        ["5", "Wallbox 6"],
    ]

    static options_meter: [string, string][] = [
        ["0", "Stromzähler 1"],
        ["1", "Stromzähler 2"],
        ["2", "Stromzähler 3"],
        ["3", "Stromzähler 4"],
        ["4", "Stromzähler 5"],
        ["5", "Stromzähler 6"],
    ]

    static options_day_ahead_prices: [string, string][] = [
        ["0", "Aktueller Strompreis"],
        ["1", "Durchschnittspreis heute"],
        ["2", "Durchschnittspreis morgen"],
    ]

    static options_solar_forecast: [string, string][] = [
        ["0", "PV-Ertragsprognose heute"],
        ["1", "PV-Ertragsprognose morgen"],
    ]

    constructor() {
        super('front_panel/config',
              __("em_front_panel.script.save_failed"));

        for (let tile_index = 0; tile_index < FRONT_PANEL_TILES; tile_index++) {
            util.addApiEventListener_unchecked(`front_panel/tiles/${tile_index}/config`, () => {
                console.log("tile_index", tile_index);
                let config = API.get_unchecked(`front_panel/tiles/${tile_index}/config`);

                this.setState((prevState) => ({
                    tile_configs: {
                        ...prevState.tile_configs,
                        [tile_index]: config
                    }
                }));

                if (!this.isDirty()) {
                    this.setState((prevState) => ({
                        tile_configs: {
                            ...prevState.tile_configs,
                            [tile_index]: config
                        }
                    }));
                }
            });
        }
    }

    override async sendSave(topic: "front_panel/config", config: EMFrontPanelConfig) {
        for (let tile_index = 0; tile_index < FRONT_PANEL_TILES; tile_index++) {
            await API.save_unchecked(
                `front_panel/tiles/${tile_index}/config`,
                this.state.tile_configs[tile_index],
                __("em_front_panel.script.save_failed"),
                tile_index == FRONT_PANEL_TILES - 1 ? this.reboot_string : undefined);
        }

        await super.sendSave(topic, config);
    }

    override async sendReset(topic: "front_panel/config") {
        for (let tile_index = 0; tile_index < FRONT_PANEL_TILES; tile_index++) {
            await API.reset_unchecked(`front_panel/tiles/${tile_index}/config`, this.error_string, this.reboot_string);
        }

        await super.sendReset(topic);
    }

    override getIsModified(topic: "front_panel/config"): boolean {
        for (let tile_index = 0; tile_index < FRONT_PANEL_TILES; tile_index++) {
            if (API.is_modified_unchecked(`front_panel/tiles/${tile_index}/config`))
                return true;
        }

        return super.getIsModified(topic);
    }
    render(props: {}, state: EMFrontPanelState & EMFrontPanelConfig) {
        if (!util.render_allowed()) {
            return <SubPage name="front_panel" />;
        }

        function get_tile_symbol(tile_index: number) {
            return <svg xmlns="http://www.w3.org/2000/svg" width="35" height="24" viewBox="0 0 35 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-grid">
                <rect fill={tile_index == 0 ? "currentColor" : ""} x="3"  y="3"  width="7" height="7"></rect>
                <rect fill={tile_index == 1 ? "currentColor" : ""} x="14" y="3"  width="7" height="7"></rect>
                <rect fill={tile_index == 2 ? "currentColor" : ""} x="25" y="3"  width="7" height="7"></rect>
                <rect fill={tile_index == 3 ? "currentColor" : ""} x="3"  y="14" width="7" height="7"></rect>
                <rect fill={tile_index == 4 ? "currentColor" : ""} x="14" y="14" width="7" height="7"></rect>
                <rect fill={tile_index == 5 ? "currentColor" : ""} x="25" y="14" width="7" height="7"></rect>
            </svg>
        }

        function get_tile_config(tile_index: number, tile_items: [string, string][]) {
            return <FormRow label="">
                <div class="row no-gutters">
                    <div class="col-md-12">
                        <div class="input-group">
                            <div class="input-group-prepend heating-input-group-append">
                                <span class="heating-fixed-size input-group-text">Einstellung</span>
                            </div>
                            <InputSelect
                                items={tile_items}
                                value={state.tile_configs[tile_index].parameter}
                                onValue={(v) => this.setState({tile_configs: {...state.tile_configs, [tile_index]: {parameter: parseInt(v), "type": state.tile_configs[tile_index].type}}})}
                            />
                        </div>
                    </div>
                </div>
            </FormRow>
        }

        return (
            <SubPage name="front_panel">
                <ConfigForm id="em_front_panel_config_form"
                            title={__("em_front_panel.content.em_front_panel")}
                            isModified={this.isModified()}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={undefined}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("em_front_panel.content.em_front_panel")} label_muted={__("em_front_panel.content.em_front_panel_muted")}>
                        <Switch desc={__("em_front_panel.content.em_front_panel_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}
                        />
                    </FormRow>
                    <Collapse in={state.enable}>
                        <div>
                            {[...Array(FRONT_PANEL_TILES).keys()].map((tile_index) => {
                                return <div>
                                    {tile_index != 0 && <FormSeparator first={true}/>}
                                    <FormRow symbol={get_tile_symbol(tile_index)} label={"Kachel " + (tile_index+1)}>
                                        <InputSelect
                                            items={EMFrontPanel.options_tile}
                                            value={state.tile_configs[tile_index].type}
                                            onValue={(v) => this.setState({tile_configs: {...state.tile_configs, [tile_index]: {parameter: state.tile_configs[tile_index].parameter, "type": parseInt(v)}}})}
                                        />
                                    </FormRow>
                                    {state.tile_configs[tile_index].type === 1 && (get_tile_config(tile_index, EMFrontPanel.options_wallbox))}
                                    {state.tile_configs[tile_index].type === 3 && (get_tile_config(tile_index, EMFrontPanel.options_meter))}
                                    {state.tile_configs[tile_index].type === 4 && (get_tile_config(tile_index, EMFrontPanel.options_day_ahead_prices))}
                                    {state.tile_configs[tile_index].type === 5 && (get_tile_config(tile_index, EMFrontPanel.options_solar_forecast))}
                                </div>
                            })}
                        </div>
                    </Collapse>
                </ConfigForm>
        	</SubPage>
        );
    }
}

export function init() {
}
