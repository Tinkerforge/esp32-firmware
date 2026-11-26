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
import { h } from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { FormRow } from "../../ts/components/form_row";
import { Switch } from "../../ts/components/switch";
import { InputSelect } from "../../ts/components/input_select";
import { InputNumber } from "../../ts/components/input_number";
import { InputText } from "../../ts/components/input_text";
import { register_status_provider, ModuleStatus } from "../../ts/status_registry";
import { P9EegSource } from "./generated/p9_eeg_source.enum";

const RELAY_UNUSED = 255;
const RELAY_LEVELS = [0, 30, 60];

export function P9EegNavbar() {
    return (
        <NavbarItem
            name="p9_eeg"
            module="p9_eeg"
            title={__("p9_eeg.navbar.p9_eeg")}
            symbol={
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="currentColor" stroke="none">
                    <text x="12" y="18" font-size="20" text-anchor="middle">&#167;</text>
                </svg>
            }
        />
    );
}

type P9EegConfig = API.getType["p9_eeg/config"];

interface P9EegState {
    p9_state: API.getType["p9_eeg/state"];
}

function available_levels(relays: number[]) {
    return [100, ...relays.filter((level) => level != RELAY_UNUSED)].sort((a, b) => b - a);
}

function level_power_w(level: number, inverter_power_w: number) {
    return Math.ceil(level * inverter_power_w / 100);
}

export class P9Eeg extends ConfigComponent<'p9_eeg/config', {}, P9EegState> {
    constructor() {
        super('p9_eeg/config',
              () => __("p9_eeg.script.save_failed"));

        util.addApiEventListener("p9_eeg/state", () => {
            this.setState({p9_state: API.get("p9_eeg/state")});
        });
    }

    set_relay(index: number, level: number) {
        const relays = this.state.relays.slice();
        relays[index] = level;

        let default_level_pct = this.state.default_level_pct;
        if (available_levels(relays).indexOf(default_level_pct) < 0) {
            default_level_pct = 100;
        }

        this.setState({relays: relays, default_level_pct: default_level_pct});
    }

    render(props: {}, state: P9EegState & P9EegConfig) {
        if (!util.render_allowed())
            return <SubPage name="p9_eeg" />;

        const level_text = (level: number) => state.inverter_power_w > 0
            ? __("p9_eeg.content.level_with_power")(level, level_power_w(level, state.inverter_power_w))
            : level + " %";

        const relay_items: [string, string][] = [
            [String(RELAY_UNUSED), __("p9_eeg.content.relay_unused")],
            ...RELAY_LEVELS.map((level): [string, string] => [String(level), level_text(level)]),
        ];

        const status_text = !state.p9_state ? ""
            : state.p9_state.default_active ? __("p9_eeg.status.default_active")(state.p9_state.level_pct)
            : state.p9_state.level_pct < 100 ? __("p9_eeg.status.limited")(state.p9_state.level_pct)
            : __("p9_eeg.status.not_limited");

        return (
            <SubPage name="p9_eeg" title={__("p9_eeg.content.p9_eeg")}>
                <SubPage.Status collapsed={!API.get("p9_eeg/config").enable}>
                    <FormRow label={__("p9_eeg.status.status")}
                             warning={__("p9_eeg.status.limit_not_reachable")}
                             show_warning={state.p9_state && !state.p9_state.limit_reachable}>
                        <InputText value={status_text} />
                    </FormRow>
                    {state.p9_state?.limit_active ?
                        <FormRow label={__("p9_eeg.status.requested_limit")}>
                            <InputText value={state.p9_state.limit_w + " W"} />
                        </FormRow>
                    : undefined}
                </SubPage.Status>

                <SubPage.Config id="p9_eeg_config_form"
                                isDirty={this.isDirty()}
                                onSave={this.save}
                                onDirtyChange={this.setDirty}>
                    <FormRow label={__("p9_eeg.content.enable_p9_eeg")} help={__("p9_eeg.content.enable_p9_eeg_help")}>
                        <Switch desc={__("p9_eeg.content.enable_p9_eeg_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}
                        />
                    </FormRow>

                    <FormRow label={__("p9_eeg.content.source")}
                             help={__("p9_eeg.content.source_help")}
                             error={__("p9_eeg.content.eebus_needs_activation")}
                             show_error={state.source == P9EegSource.EEBus && !API.get("eebus/config").enable}>
                        <InputSelect
                            items={[
                                [String(P9EegSource.EEBus), __("p9_eeg.content.source_eebus")],
                                [String(P9EegSource.API), __("p9_eeg.content.source_api")],
                            ]}
                            value={state.source}
                            onValue={(v) => this.setState({source: parseInt(v)})}
                        />
                    </FormRow>

                    <FormRow label={__("p9_eeg.content.inverter_power")}
                             label_muted={__("p9_eeg.content.inverter_power_muted")}
                             help={__("p9_eeg.content.inverter_power_help")}>
                        <InputNumber
                            required={state.enable}
                            value={state.inverter_power_w}
                            onValue={(v) => this.setState({inverter_power_w: v})}
                            min={state.enable ? 1 : 0}
                            max={10000000}
                            unit="W"
                        />
                    </FormRow>

                    {state.relays.map((level, i) =>
                        <FormRow key={i}
                                 label={state.relays.length > 1 ? __("p9_eeg.content.relay_n")(i + 1) : __("p9_eeg.content.relay")}
                                 label_muted={__("p9_eeg.content.relay_muted")}
                                 help={i == 0 ? __("p9_eeg.content.relay_help") : undefined}
                                 error={__("p9_eeg.content.relay_duplicate")}
                                 show_error={level != RELAY_UNUSED && state.relays.indexOf(level) != i}>
                            <InputSelect
                                items={relay_items}
                                value={level}
                                onValue={(v) => this.set_relay(i, parseInt(v))}
                            />
                        </FormRow>
                    )}

                    <FormRow label={__("p9_eeg.content.default_level")}
                             label_muted={__("p9_eeg.content.default_level_muted")}
                             help={__("p9_eeg.content.default_level_help")}
                             error={__("p9_eeg.content.no_relay_used")}
                             show_error={state.enable && available_levels(state.relays).length == 1}>
                        <InputSelect
                            items={available_levels(state.relays).map((level): [string, string] => [String(level), level_text(level)])}
                            value={state.default_level_pct}
                            onValue={(v) => this.setState({default_level_pct: parseInt(v)})}
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
    register_status_provider("p9_eeg", {
        name: () => __("p9_eeg.navbar.p9_eeg"),
        href: "#p9_eeg",
        get_status: () => {
            const config = API.get("p9_eeg/config");
            const state = API.get("p9_eeg/state");

            if (!config.enable) {
                return {status: ModuleStatus.Disabled};
            }

            if (!state.limit_reachable) {
                return {
                    status: ModuleStatus.Error,
                    text: () => __("p9_eeg.status.limit_not_reachable")
                };
            }

            if (state.default_active) {
                return {
                    status: ModuleStatus.Warning,
                    text: () => __("p9_eeg.status.default_active")(state.level_pct)
                };
            }

            if (state.level_pct < 100) {
                return {
                    status: ModuleStatus.Warning,
                    text: () => __("p9_eeg.status.limited")(state.level_pct)
                };
            }

            return {
                status: ModuleStatus.Ok,
                text: () => __("p9_eeg.status.not_limited")
            };
        }
    });
}
