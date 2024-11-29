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
import { h } from "preact";
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
import { FormSeparator } from "../../ts/components/form_separator";
import { MeterValueID } from "../meters/meter_value_id";
import { get_noninternal_meter_slots, NoninternalMeterSelector } from "../power_manager/main";
import { get_managed_chargers } from "../charge_manager/chargers";

const FRONT_PANEL_TILES = 6;

export function FrontPanelNavbar() {
    return (
        <NavbarItem
            name="front_panel"
            title={__("front_panel.navbar.front_panel")}
            hidden={false} // Module/Feature-Check
            symbol={<Monitor/>}
        />
    );
}

type FrontPanelConfig = API.getType["front_panel/config"];

export class FrontPanel extends ConfigComponent<"front_panel/config", {}> {
    static options_tile(): [string, string][] { return [
        ["0", __("front_panel.content.empty_tile")],
        ["1", __("front_panel.content.wallbox")],
        ["2", __("front_panel.content.charge_management")],
        ["3", __("front_panel.content.meter")],
        ["4", __("front_panel.content.day_ahead_price")],
        ["5", __("front_panel.content.solar_forecast")],
        ["6", __("front_panel.content.energy_manager_status")],
        ["7", __("front_panel.content.heating_status")],
    ]}

    static options_wallbox_unknown(): [string, string][] { return [...Array(32).keys()].map((i) => [
        i.toString(),
        __("front_panel.content.wallbox") + " " + i + " (" + __("front_panel.content.unconfigured") + ")"
    ])}

    static options_wallbox(): [string, string][] {
        const wallboxes = get_managed_chargers();
        let wallbox_slots = FrontPanel.options_wallbox_unknown();

        wallboxes.forEach((wallbox) => {
            const index = wallbox_slots.findIndex(([id]) => id === wallbox[0]);
            if (index !== -1) {
                wallbox_slots[index] = wallbox;
            }
        });

        return wallbox_slots;
    }


    static options_meter_unkown(): [string, string][] { return [...Array(7).keys()].map((i) => [
        i.toString(), __("front_panel.content.meter") + " " + i + " (" + __("front_panel.content.unconfigured") + ")"
    ])}

    static options_meter(): [string, string][] {
        const active_meter_slots = get_noninternal_meter_slots([MeterValueID.PowerActiveLSumImExDiff], NoninternalMeterSelector.AllValues, __("power_manager.content.meter_slot_grid_power_missing_value"));
        let meter_slots = FrontPanel.options_meter_unkown();

        active_meter_slots.forEach((slot) => {
            const index = meter_slots.findIndex(([id]) => id === slot[0]);
            if (index !== -1) {
                meter_slots[index] = slot;
            }
        });

        return meter_slots;
    }

    static options_day_ahead_prices(): [string, string][] { return [
        ["0", __("front_panel.content.current_electricity_price")],
        ["1", __("front_panel.content.average_price_today")],
        ["2", __("front_panel.content.average_price_tomorrow")],
    ]}

    static options_solar_forecast(): [string, string][] { return [
        ["0", __("front_panel.content.pv_yield_forecast_today")],
        ["1", __("front_panel.content.pv_yield_forecast_tomorrow")],
    ]}

    constructor() {
        super('front_panel/config',
              () => __("front_panel.script.save_failed"));
/*
        for (let tile_index = 0; tile_index < FRONT_PANEL_TILES; tile_index++) {
            util.addApiEventListener_unchecked(`front_panel/tiles/${tile_index}/config`, () => {
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
*/
    }

    /*override async sendSave(topic: "front_panel/config", config: FrontPanelConfig) {
        for (let tile_index = 0; tile_index < FRONT_PANEL_TILES; tile_index++) {
            await API.save_unchecked(
                `front_panel/tiles/${tile_index}/config`,
                this.state.tile_configs[tile_index],
                __("front_panel.script.save_failed"),
                tile_index == FRONT_PANEL_TILES - 1 ? this.reboot_string : undefined);
        }

        await super.sendSave(topic, config);
    }*/

/*
    override async sendReset(topic: "front_panel/config") {
        for (let tile_index = 0; tile_index < FRONT_PANEL_TILES; tile_index++) {
            await API.reset_unchecked(`front_panel/tiles/${tile_index}/config`, this.error_string, this.reboot_string);
        }

        await super.sendReset(topic);
    }
*/
/*
    override getIsModified(topic: "front_panel/config"): boolean {
        for (let tile_index = 0; tile_index < FRONT_PANEL_TILES; tile_index++) {
            if (API.is_modified_unchecked(`front_panel/tiles/${tile_index}/config`))
                return true;
        }

        return super.getIsModified(topic);
    }
*/
    get_tile_config(tile_index: number, tile_items: [string, string][]) {
        return <FormRow label="">
            <div class="row no-gutters">
                <div class="col-md-12">
                    <div class="input-group">
                        <div class="input-group-prepend">
                            <span class="input-group-text">{__("front_panel.content.setting")}</span>
                        </div>
                        <InputSelect
                            className="front-panel-input-group-prepend"
                            items={tile_items}
                            value={this.state.tiles[tile_index][1]}
                            onValue={(v) => {
                                let tiles = this.state.tiles;
                                tiles[tile_index][1] = parseInt(v);
                                this.setState({tiles: tiles})
                            }}
                        />
                    </div>
                </div>
            </div>
        </FormRow>
    }

    render(props: {}, state: FrontPanelConfig) {
        if (!util.render_allowed()) {
            return <SubPage name="front_panel" />;
        }

        function get_tile_symbol(tile_index: number) {
            return <svg xmlns="http://www.w3.org/2000/svg" width="38" height="35" viewBox="0 0 38 35" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-grid">
                <rect fill={tile_index == 0 ? "currentColor" : ""} y="3"  x="3"  width="14" height="7"></rect>
                <rect fill={tile_index == 1 ? "currentColor" : ""} y="14" x="3"  width="14" height="7"></rect>
                <rect fill={tile_index == 2 ? "currentColor" : ""} y="25" x="3"  width="14" height="7"></rect>
                <rect fill={tile_index == 3 ? "currentColor" : ""} y="3"  x="21" width="14" height="7"></rect>
                <rect fill={tile_index == 4 ? "currentColor" : ""} y="14" x="21" width="14" height="7"></rect>
                <rect fill={tile_index == 5 ? "currentColor" : ""} y="25" x="21" width="14" height="7"></rect>
            </svg>
        }

        return (
            <SubPage name="front_panel">
                <ConfigForm id="front_panel_config_form"
                            title={__("front_panel.content.front_panel")}
                            isModified={false}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("front_panel.content.front_panel")} label_muted={__("front_panel.content.front_panel_muted")}>
                        <Switch desc={__("front_panel.content.front_panel_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}
                        />
                    </FormRow>
                    <Collapse in={state.enable}>
                        <div>
                            {util.range(FRONT_PANEL_TILES).map((tile_index) => {
                                return <div>
                                    {tile_index != 0 && <FormSeparator first={true}/>}
                                    <FormRow symbol={get_tile_symbol(tile_index)} label={__("front_panel.content.tile") + " " + (tile_index+1)}>
                                        <InputSelect
                                            items={FrontPanel.options_tile()}
                                            value={state.tiles[tile_index][0]}
                                            onValue={(v) => {
                                                let tiles = state.tiles;
                                                tiles[tile_index] = [parseInt(v), state.tiles[tile_index][1]];
                                                this.setState({tiles: tiles})}
                                            }
                                        />
                                    </FormRow>
                                    {state.tiles[tile_index][0] === 1 && (this.get_tile_config(tile_index, FrontPanel.options_wallbox()))}
                                    {state.tiles[tile_index][0] === 3 && (this.get_tile_config(tile_index, FrontPanel.options_meter()))}
                                    {state.tiles[tile_index][0] === 4 && (this.get_tile_config(tile_index, FrontPanel.options_day_ahead_prices()))}
                                    {state.tiles[tile_index][0] === 5 && (this.get_tile_config(tile_index, FrontPanel.options_solar_forecast()))}
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
