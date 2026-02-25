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
import { h, Fragment, Component, RefObject } from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import { InputSelect } from "../../ts/components/input_select";
import { InputNumber } from "../../ts/components/input_number";
import { InputText } from "../../ts/components/input_text";
import { register_status_provider, ModuleStatus } from "../../ts/status_registry";
import { P14aEnwgSource } from "./p14a_enwg_source.enum";

const SIMULTANEITY_FACTORS: {[key: number]: number} = {2: 0.80, 3: 0.75, 4: 0.70, 5: 0.65, 6: 0.60, 7: 0.55, 8: 0.50};

function calculateP14aLimit(deviceCount: number): number {
    if (deviceCount <= 1) return 4200;
    const factor = deviceCount >= 9 ? 0.45 : SIMULTANEITY_FACTORS[deviceCount];
    return Math.round(4200 + (deviceCount - 1) * factor * 4200);
}

function deviceCountFromLimit(limit: number): number {
    for (let n = 1; n <= 99; n++) {
        if (calculateP14aLimit(n) === limit) return n;
    }
    return 1;
}

export function P14aEnwgNavbar() {
    return (
        <NavbarItem
            name="p14a_enwg"
            module="p14a_enwg"
            title={__("p14a_enwg.navbar.p14a_enwg")}
            symbol={
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="currentColor" stroke="none">
                    <text x="12" y="18" font-size="20" text-anchor="middle">&#167;</text>
                </svg>
            }
        />
    );
}

type P14aEnwgConfig = API.getType["p14a_enwg/config"];

interface P14aEnwgState {
    is_charger: boolean;
    is_em: boolean;
    has_heating: boolean;
    config_enable: boolean;
    p14a_state: API.getType["p14a_enwg/state"];
}

export class P14aEnwg extends ConfigComponent<'p14a_enwg/config', {status_ref?: RefObject<P14aEnwgStatus>}, P14aEnwgState> {
    constructor() {
        super('p14a_enwg/config',
             () => __("p14a_enwg.script.save_failed"));

        util.addApiEventListener("info/modules", () => {
            this.setState({
                is_em: API.hasModule("em_common"),
                is_charger: API.hasModule("evse_common"),
                has_heating: API.hasModule("heating"),
            });
        });

        util.addApiEventListener("p14a_enwg/config", () => {
            let config = API.get("p14a_enwg/config");
            this.setState({config_enable: config.enable});
        });

        util.addApiEventListener("p14a_enwg/state", () => {
            this.setState({p14a_state: API.get("p14a_enwg/state")});
        });
    }

    override async sendSave(topic: "p14a_enwg/config", config: P14aEnwgConfig) {
        this.setState({config_enable: config.enable}); // avoid round trip time
        await super.sendSave(topic, config);
    }

    render(props: {}, state: P14aEnwgState & P14aEnwgConfig) {
        if (!util.render_allowed())
            return <SubPage name="p14a_enwg" />;

        const source_is_input = state.source === P14aEnwgSource.Input;
        const disabled = !state.config_enable;

        // Source dropdown: label differs between WARP and EM
        const source_items: [string, string][] = [
            ["0", state.is_charger ? __("p14a_enwg.content.source_shutdown_input") : __("p14a_enwg.content.source_input")(__("branding.device"))],
            ["1", __("p14a_enwg.content.source_eebus")],
            ["2", __("p14a_enwg.content.source_api")],
        ];

        return (
            <SubPage name="p14a_enwg" title={__("p14a_enwg.content.p14a_enwg")}>
                {state.config_enable &&
                    <SubPage.Status>
                        <FormRow label={__("p14a_enwg.status.status")}>
                            <InputText
                                value={state.p14a_state?.active ? __("p14a_enwg.status.active") : __("p14a_enwg.status.inactive")}
                            />
                        </FormRow>
                        {state.p14a_state?.active ?
                            <FormRow label={__("p14a_enwg.status.current_limit")}>
                                <InputText
                                    value={state.p14a_state.limit + " W"}
                                />
                            </FormRow>
                        : undefined}
                    </SubPage.Status>
                }

                <SubPage.Config id="p14a_enwg_config_form"
                                isModified={false}
                                isDirty={this.isDirty()}
                                onSave={this.save}
                                onReset={this.reset}
                                onDirtyChange={this.setDirty}>
                    <FormRow label={__("p14a_enwg.content.enable_p14a_enwg")} help={__("p14a_enwg.content.enable_p14a_enwg_help")}>
                        <Switch desc={__("p14a_enwg.content.enable_p14a_enwg_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}
                        />
                    </FormRow>

                    <FormRow label={__("p14a_enwg.content.source")}>
                        <InputSelect
                            items={source_items}
                            value={state.source}
                            onValue={(v) => this.setState({source: parseInt(v)})}
                            disabled={disabled}
                        />
                    </FormRow>

                    {source_is_input ?
                        <>
                            {state.is_em ?
                                <FormRow label={__("p14a_enwg.content.input")}>
                                    <InputSelect
                                        items={[
                                            ["0", __("p14a_enwg.content.input") + " 1"],
                                            ["1", __("p14a_enwg.content.input") + " 2"],
                                            ["2", __("p14a_enwg.content.input") + " 3"],
                                            ["3", __("p14a_enwg.content.input") + " 4"],
                                        ]}
                                        value={state.input_index}
                                        onValue={(v) => this.setState({input_index: parseInt(v)})}
                                        disabled={disabled}
                                    />
                                </FormRow>
                            : undefined}
                            <FormRow label={__("p14a_enwg.content.limit")} label_muted={__("p14a_enwg.content.limit_muted")} help={__("p14a_enwg.content.limit_help")}>
                                <InputNumber
                                    value={deviceCountFromLimit(state.limit)}
                                    onValue={(n) => this.setState({limit: calculateP14aLimit(n)})}
                                    min={1}
                                    max={99}
                                    unit={__("p14a_enwg.content.device_count_unit")(deviceCountFromLimit(state.limit), state.limit)}
                                    disabled={disabled}
                                />
                            </FormRow>
                            <FormRow label={__("p14a_enwg.content.active_on")}>
                                <InputSelect
                                    items={[
                                        ["1", __("p14a_enwg.content.active_on_close")],
                                        ["0", __("p14a_enwg.content.active_on_open")],
                                    ]}
                                    value={state.active_on_close ? "1" : "0"}
                                    onValue={(v) => this.setState({active_on_close: v === "1"})}
                                    disabled={disabled}
                                />
                            </FormRow>
                        </>
                    : undefined}

                    {state.is_charger ?
                        <FormRow label={__("p14a_enwg.content.this_charger")}>
                            <Switch desc={__("p14a_enwg.content.this_charger_desc")}
                                checked={state.this_charger}
                                onClick={this.toggle('this_charger')}
                                disabled={disabled}
                            />
                        </FormRow>
                    : undefined}

                    <FormRow label={__("p14a_enwg.content.managed_chargers")}>
                        <Switch desc={__("p14a_enwg.content.managed_chargers_desc")}
                            checked={state.managed_chargers}
                            onClick={this.toggle('managed_chargers')}
                            disabled={disabled}
                        />
                    </FormRow>

                    {state.has_heating ?
                        <>
                            <FormRow label={__("p14a_enwg.content.heating")}>
                                <Switch desc={__("p14a_enwg.content.heating_desc")}
                                    checked={state.heating}
                                    onClick={this.toggle('heating')}
                                    disabled={disabled}
                                />
                            </FormRow>
                            <FormRow label={__("p14a_enwg.content.heating_max_power")} label_muted={__("p14a_enwg.content.heating_max_power_muted")}>
                                <InputNumber
                                    value={state.heating_max_power}
                                    onValue={(v) => this.setState({heating_max_power: v})}
                                    min={0}
                                    max={50000}
                                    unit="W"
                                    disabled={disabled || !state.heating}
                                />
                            </FormRow>
                        </>
                    : undefined}
                </SubPage.Config>
            </SubPage>
        );
    }
}

interface P14aEnwgStatusState {
    state: API.getType["p14a_enwg/state"];
}

export class P14aEnwgStatus extends Component<{}, P14aEnwgStatusState> {
    constructor() {
        super();

        util.addApiEventListener('p14a_enwg/state', () => {
            this.setState({state: API.get('p14a_enwg/state')});
        });
    }

    render(props: {}, state: P14aEnwgStatusState) {
        if (!util.render_allowed()) {
            return <StatusSection name="p14a_enwg" />;
        }

        let config = API.get('p14a_enwg/config');
        if (!config.enable) {
            return <StatusSection name="p14a_enwg" />;
        }

        return <StatusSection name="p14a_enwg">
            <FormRow label={__("p14a_enwg.status.status")}>
                <InputText
                    value={state.state?.active ? __("p14a_enwg.status.active") : __("p14a_enwg.status.inactive")}
                />
            </FormRow>
            {state.state?.active ?
                <FormRow label={__("p14a_enwg.status.current_limit")}>
                    <InputText
                        value={state.state.limit + " W"}
                    />
                </FormRow>
            : undefined}
        </StatusSection>;
    }
}

export function pre_init() {
}

export function init() {
    register_status_provider("p14a_enwg", {
        name: () => __("p14a_enwg.navbar.p14a_enwg"),
        href: "#p14a_enwg",
        get_status: () => {
            const config = API.get("p14a_enwg/config");
            const state = API.get("p14a_enwg/state");

            if (!config.enable) {
                return {status: ModuleStatus.Disabled};
            }

            if (state.active) {
                return {
                    status: ModuleStatus.Warning,
                    text: () => __("p14a_enwg.status.active") + " (" + state.limit + " W)"
                };
            }

            return {
                status: ModuleStatus.Ok,
                text: () => __("p14a_enwg.status.inactive")
            };
        }
    });
}
