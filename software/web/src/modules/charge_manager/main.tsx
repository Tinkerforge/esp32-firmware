/* esp32-firmware
 * Copyright (C) 2021 Erik Fleckstein <erik@tinkerforge.com>
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
import { h, Component } from "preact";
import { translate_unchecked, __ } from "../../ts/translation";
import { FormRow } from "../../ts/components/form_row";
import { InputFloat } from "../../ts/components/input_float";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { Server, Sliders } from "react-feather";

export { ChargeManagerChargers } from "./chargers";
export { ChargeManagerSettings } from "./settings";

export function ChargeManagerSettingsNavbar() {
    return <NavbarItem name="charge_manager_settings" module="charge_manager" title={__("charge_manager.navbar.charge_manager_settings")} symbol={<Sliders />} />;
}

export function ChargeManagerChargersNavbar() {
    return <NavbarItem name="charge_manager_chargers" module="charge_manager" title={__("charge_manager.navbar.charge_manager_chargers")} symbol={<Server {...{style: "transform: rotate(180deg);"} as any} />} />;
}

interface ChargeManagerStatusState {
    state: API.getType['charge_manager/state']
    available_current: API.getType['charge_manager/available_current']
    config: API.getType['charge_manager/config']
    uptime: number
}

export class ChargeManagerStatus extends Component<{}, ChargeManagerStatusState> {
    constructor() {
        super();

        util.addApiEventListener('charge_manager/state', () => {
            this.setState({state: API.get('charge_manager/state')})
        });

        util.addApiEventListener('charge_manager/available_current', () => {
            this.setState({available_current: API.get('charge_manager/available_current')})
        });

        util.addApiEventListener('charge_manager/config', () => {
            this.setState({config: API.get('charge_manager/config')})
        });

        util.addApiEventListener('info/keep_alive', () => {
            this.setState({uptime: API.get('info/keep_alive').uptime})
        });
    }

    render(props: {}, state: Readonly<ChargeManagerStatusState>) {
        if (!util.render_allowed()
         || !state.config.enable_charge_manager
         || state.config.chargers.length == 0)
            return <StatusSection name="charge_manager" />;

        let cards = state.state.chargers.map((c, i) => {
            let c_state = "";
            let c_info = "";
            let c_body_classes = "";

            let last_update = Math.floor((state.uptime - c.lu) / 1000);
            let c_status_text = `${util.toLocaleFixed(c.sc / 1000.0, 3)} A @ ${((c.sp & 4) != 0) ? "3p/1" : c.sp}p ${__("charge_manager.script.ampere_supported")}`;

            if (last_update >= 10)
                c_status_text += "; " + __("charge_manager.script.last_update_prefix") + " " + util.format_timespan(last_update) + (__("charge_manager.script.last_update_suffix"));

            if (c.s != 5) {
                if (state.state.state == 2) {
                    c_body_classes = "bg-warning bg-disabled";
                    c_state = __("charge_manager.script.charge_state_blocked_by_other_box");
                    c_info = __("charge_manager.script.charge_state_blocked_by_other_box_details");
                } else {
                    c_state = translate_unchecked(`charge_manager.script.charge_state_${c.s}`);
                    let cur = c.ac / 1000.0;
                    let p = c.ap;
                    let kw = util.toLocaleFixed(cur * p * 230 / 1000.0, 3);

                    c_info = `${util.toLocaleFixed(cur, 3)} A @ ${p}p = ${kw} kW ${__("charge_manager.script.ampere_allocated")}`
                }
            }
            else {
                if (c.e < 192)
                    c_state = __("charge_manager.script.charge_error_type_management");
                else
                    c_state = __("charge_manager.script.charge_error_type_client");

                c_body_classes = "bg-danger text-white bg-disabled";
                c_info = translate_unchecked(`charge_manager.script.charge_error_${c.e}`);
            }

            let charger_config = state.config.chargers[i];
            const name_link = (API.is_dirty("charge_manager/config") || !charger_config || util.remoteAccessMode) ? c.n :
                                <a target="_blank" rel="noopener noreferrer" href={(charger_config.host == '127.0.0.1' || charger_config.host == 'localhost') ? '/' : "http://" + charger_config.host}>{c.n}</a>

            return  <div class="card">
                        <h5 class="card-header">
                            {name_link}
                        </h5>
                        <div class={"card-body " + c_body_classes}>
                            <h5 class="card-title">{c_state}</h5>
                            <p class="card-text">{c_info}</p>
                        </div>
                        <div class="card-footer">
                            <span>{c_status_text}</span>
                        </div>
                    </div>
        });

        let controls_only_self = (state.config.chargers.length == 1
                       && (state.config.chargers[0].host == '127.0.0.1' || state.config.chargers[0].host == 'localhost'));

        return <StatusSection name="charge_manager">

            {controls_only_self && API.get_unchecked("power_manager/config")?.enabled ? null :
                <FormRow label={__("charge_manager.status.charge_manager")}>
                    <IndicatorGroup
                        style="width: 100%"
                        class="flex-wrap"
                        value={state.state.state}
                        items={[
                            ["primary", __("charge_manager.status.not_configured")],
                            ["success", __("charge_manager.status.manager")],
                            ["danger", __("charge_manager.status.error")]
                        ]}/>
                </FormRow>
            }

            {// The power manager controls the available current.
                API.get_unchecked("power_manager/config")?.enabled ? null:
                <FormRow label={__("charge_manager.status.available_current")}>
                    <InputFloat min={0} max={state.config.maximum_available_current} digits={3} unit="A"
                        value={state.available_current.current}
                        onValue={(v) => API.save("charge_manager/available_current", {"current": v}, () => __("charge_manager.script.set_available_current_failed"))}
                        showMinMax/>
                </FormRow>
            }

            {controls_only_self ? null :
                <FormRow label={__("charge_manager.status.managed_boxes")}>
                    {util.range(Math.ceil(cards.length / 2)).map(i =>
                        <div class="card-deck mb-4">
                            {cards[2 * i]}
                            {cards.length > (2 * i + 1) ? cards[2 * i + 1] : undefined}
                        </div>)
                    }
                </FormRow>
            }
        </StatusSection>;
    }
}

export function init() {
}
