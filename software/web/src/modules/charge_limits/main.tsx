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
import { h, Fragment, Component } from "preact";
import { __ } from "../../ts/translation";
import { FormRow } from "../../ts/components/form_row";
import { InputSelect } from "../../ts/components/input_select";
import { StatusSection } from "../../ts/components/status_section";
import { InputFloat } from "ts/components/input_float";
import { Button, Modal } from "react-bootstrap";
import { ItemModal } from "ts/components/item_modal";
import { useState } from "preact/hooks";

export function CustomLimitModal(props: {show: boolean, onHide: () => void, onSubmit: (custom_limit: number) => void}) {
    const [customLimit, setCustomLimit] = useState(0);

    const electricity_price = API.get("charge_tracker/config").electricity_price;

    const hideFn = async () => {setCustomLimit(0); props.onHide()};

    return <ItemModal
        size="lg"
        show={props.show}
        onHide={hideFn}
        onSubmit={async () => {
            props.onSubmit(customLimit);
            //API.call("charge_limits/override_energy", {energy_wh: customLimit}, () => __("charge_limits.script.override_failed"));
            hideFn()
        }}
        title={__("charge_limits.content.custom_energy_modal_title")}
        no_variant="secondary" no_text={__("main.abort")}
        yes_variant="primary" yes_text={__("charge_limits.content.custom_energy_modal_apply")}
        >
        <FormRow label={__("charge_limits.content.custom_energy_limit_label")}>
        <InputFloat
                value={customLimit}
                onValue={setCustomLimit}
                min={1}
                max={999999}
                digits={3}
                unit="kWh"/>
        </FormRow>

        {electricity_price > 0 ?
            <FormRow label={__("charge_limits.content.custom_energy_limit_cost_label")}>
            <InputFloat
                    disabled
                    value={electricity_price / 100 * customLimit / 1000}
                    digits={2}
                    unit="€"/>
            </FormRow>
            : undefined}
    </ItemModal>
}

export class ChargeLimitsStatus extends Component<{}, {last_custom_energy: number, show_custom_energy_modal: boolean}> {
    constructor() {
        super();
        this.state = {last_custom_energy: 0, show_custom_energy_modal: false};
    }

    render() {
        if (!util.render_allowed() || !API.hasFeature("evse"))
            return <StatusSection name="charge_limits" />;

        let config_in_use = API.get("charge_limits/active_limits");
        let config = API.get("charge_limits/default_limits");
        let state = API.get("charge_limits/state");

        let evse_uptime = API.get("evse/low_level_state").uptime;
        let electricity_price = API.get("charge_tracker/config").electricity_price;

        const get_duration_left = () => {
            let ret = "";
            if (config_in_use.duration == 0)
                ret = __("charge_limits.content.unlimited");
            else {
                let duration: number;
                duration = state.target_timestamp_ms;
                if (state.start_timestamp_ms != 0)
                    duration = state.target_timestamp_ms - evse_uptime;
                if (state.target_timestamp_ms - evse_uptime < 0 && state.start_timestamp_ms != 0)
                    duration = 0;

                ret = util.format_timespan_ms(duration);
            }

            return ret;
        };

        let duration_items: [string, string][] = [
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
            ["10", __("charge_limits.content.h12")],
        ];

        let duration_placeholder = duration_items[config_in_use.duration][1];

        if (config_in_use.duration != 0)
            duration_placeholder += " | " + get_duration_left() + __("charge_limits.content.left");

        duration_items[config.duration][1] += " " + __("charge_limits.content.configured");
        if (config.duration != config_in_use.duration)
            duration_items[config_in_use.duration][1] += " " + __("charge_limits.content.overridden");

        let energy_row = <></>;

        if (API.hasFeature("meter")) {
            let meter_abs = API.get("meter/values").energy_abs;

            const get_energy_left = () => {
                let ret = "";
                if (config_in_use.energy_wh == 0)
                    ret = __("charge_limits.content.unlimited");
                else {
                    let energy = state.target_energy_kwh - meter_abs;
                    if (state.start_energy_kwh == null)
                        energy = config_in_use.energy_wh / 1000.0;
                    else if (energy < 0)
                        energy = 0;

                    ret = util.toLocaleFixed(energy, 3) + " kWh";
                }

                return ret;
            };

            let energy_items: [string, string][] = [
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
                ["100000", util.toLocaleFixed(100, 0) + " kWh"],
                ["custom", __("charge_limits.content.custom_energy_limit")]
            ];

            if (electricity_price > 0) {
                for (let i = 1; i < energy_items.length; ++i) {
                    if (energy_items[i][0] == "custom")
                        continue;
                    energy_items[i][1] += ` (~ ${util.toLocaleFixed(electricity_price / 10000 * parseFloat(energy_items[i][0]) / 1000, 2)} €)`
                }
            }

            let energy_placeholder = __("charge_limits.content.unlimited");

            if (config_in_use.energy_wh != 0)
                energy_placeholder = (config_in_use.energy_wh % 1000 == 0 ? util.toLocaleFixed(config_in_use.energy_wh / 1000, 0) : util.toLocaleFixed(config_in_use.energy_wh / 1000.0, 3)) + " kWh | " + get_energy_left() + __("charge_limits.content.left");

            let conf_idx = energy_items.findIndex(x => x[0] == config.energy_wh.toString());
            if (conf_idx == -1)
                energy_items = [[config.energy_wh.toString(),
                                    util.toLocaleFixed(config.energy_wh / 1000.0, 3)
                                    + " kWh "
                                    + (electricity_price > 0 ? ` (~ ${util.toLocaleFixed(electricity_price / 10000 * config.energy_wh / 1000, 2)} €)` : "")
                                    + __("charge_limits.content.configured")],
                                ...energy_items];
            else
                energy_items[conf_idx][1] += " " + __("charge_limits.content.configured");

            if (config.energy_wh != config_in_use.energy_wh) {
                let conf_idx = energy_items.findIndex(x => x[0] == config_in_use.energy_wh.toString())
                if (conf_idx >= 0)
                    energy_items[conf_idx][1] += " " + __("charge_limits.content.overridden");
            }

            energy_row = <FormRow label={__("charge_limits.content.override_energy")}>
                <InputSelect items={energy_items}
                    placeholder={energy_placeholder}
                    value={""}
                    onValue={(v) => {
                        if (v == "custom") {
                            this.setState({show_custom_energy_modal: true});
                            return;
                        }

                        API.call("charge_limits/override_energy", {energy_wh: parseInt(v)}, () => __("charge_limits.script.override_failed"));
                }}/>

                <CustomLimitModal
                    show={this.state.show_custom_energy_modal}
                    onHide={() => this.setState({show_custom_energy_modal: false})}
                    onSubmit={limit => API.call("charge_limits/override_energy", {energy_wh: limit}, () => __("charge_limits.script.override_failed"))}/>
            </FormRow>
        }

        return <StatusSection name="charge_limits">
                <FormRow label={__("charge_limits.content.override_duration")}>
                    <InputSelect items={duration_items}
                        placeholder={duration_placeholder}
                        value={""}
                        onValue={(v) => {
                            API.call("charge_limits/override_duration", {duration: parseInt(v)}, () => __("charge_limits.script.override_failed"));
                    }}/>
                </FormRow>
                {energy_row}
            </StatusSection>;
    }
}

export function init() {
}
