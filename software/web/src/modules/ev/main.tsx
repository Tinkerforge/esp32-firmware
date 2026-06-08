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
import { h, Fragment, Component } from "preact";
import { __ } from "../../ts/translation";
import { FormRow } from "../../ts/components/form_row";
import { InputText, InputTextPatterned } from "../../ts/components/input_text";
import { InputFloat } from "../../ts/components/input_float";
import { Switch } from "../../ts/components/switch";
import { SubPage } from "../../ts/components/sub_page";
import { Table } from "../../ts/components/table";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { DiscoveryResultItem, DiscoveryResultGroup } from "../../ts/components/discovery_result";
import { Plus } from "react-feather";

export function EvNavbar() {
    return <NavbarItem
        name="ev"
        module="ev"
        title={__("ev.navbar.ev")}
        symbol={
            <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                <path d="M19 17h2c.6 0 1-.4 1-1v-3c0-.9-.7-1.7-1.5-1.9C18.7 10.6 16 10 16 10s-1.3-1.4-2.2-2.3c-.5-.4-1.1-.7-1.8-.7H5c-.6 0-1.1.4-1.4.9l-1.4 2.9A3.7 3.7 0 0 0 2 12v4c0 .6.4 1 1 1h2"/>
                <circle cx="7" cy="17" r="2"/>
                <path d="M9 17h6"/>
                <circle cx="17" cy="17" r="2"/>
            </svg>
        }
    />;
}


const EV_MAX_EVS = 16;
const MAC_PATTERN = "^([0-9A-Fa-fXx]{2}:){5}[0-9A-Fa-fXx]{2}$";
const MAC_PREFIX_PATTERN = "^([0-9A-Fa-f]{2}:){2}[0-9A-Fa-f]{2}$";

function is_rotating_mac(mac: string): boolean {
    return mac.toUpperCase().endsWith(':XX:XX:XX');
}

function mac_to_prefix(mac: string): string {
    return mac.split(":").slice(0, 3).join(":");
}

// Check if two MACs match, treating "XX" as wildcards.
function mac_matches(pattern: string, mac: string): boolean {
    let p = pattern.toUpperCase().split(":");
    let m = mac.toUpperCase().split(":");
    if (p.length !== 6 || m.length !== 6)
        return false;
    for (let i = 0; i < 6; i++) {
        if (p[i] !== "XX" && m[i] !== "XX" && p[i] !== m[i])
            return false;
    }
    return true;
}

function find_matching_ev(evs: EvProfile[], mac: string, ignoreIndex?: number): number {
    for (let i = 0; i < evs.length; i++) {
        if (i !== ignoreIndex && mac_matches(evs[i].mac, mac))
            return i;
    }
    return -1;
}


type EvProfile = API.getType["ev/config"]["evs"][0];

// A seen EV MAC, optionally annotated with the name of the charger that saw it
// (when it was received through central user management).
export type EvSeenMac = API.getType['ev/seen_macs'][0] & { charger_name?: string | null };

// Get EV MACs seen locally, merged with the EV MACs received from other chargers
// through central user management. Returns all MACs deduplicated.
export async function get_all_seen_macs(): Promise<EvSeenMac[]> {
    let all_macs: EvSeenMac[] = (API.get('ev/seen_macs') ?? []).map(m => {
        const seen_mac: EvSeenMac = {...m, charger_name: null};
        return seen_mac;
    });

    if (API.hasModule("charge_manager") && API.get("charge_manager/config").enable_central_management) {
        let cm_state = API.get_unchecked("charge_manager/state");
        if (cm_state && cm_state.chargers) {
            try {
                const response = await fetch("/charge_manager/auth_info");
                if (response.ok) {
                    const auth_info_data: {
                        ti: string;
                        tt: number;
                        ts: number;
                        am?: number;
                    }[][] = await response.json();
                    for (let charger_index = 0; (charger_index < auth_info_data.length) && (charger_index < cm_state.chargers.length); charger_index++) {
                        const charger_name = cm_state.chargers[charger_index].n;
                        for (const auth_info of auth_info_data[charger_index]) {
                            if (!auth_info.ti || (auth_info.ti === "")) continue;
                            if ((auth_info.am !== 4) && (auth_info.am !== 5)) continue;

                            const mac = auth_info.ti;
                            const ts = Math.floor(Date.now() / 1000) - Math.floor(auth_info.ts / 1000);
                            const index = all_macs.findIndex((m) => m.mac === mac);
                            if (index !== -1) {
                                if (all_macs[index].last_seen < ts) {
                                    all_macs[index].charger_name = charger_name;
                                    all_macs[index].last_seen = ts;
                                }
                                continue;
                            }
                            all_macs.push({
                                mac: mac,
                                last_seen: ts,
                                charger_name: charger_name,
                            });
                        }
                    }
                }
            } catch (e) {
                console.error("Failed to fetch charge_manager/auth_info:", e);
            }
        }
    }

    return all_macs;
}

interface EvState {
    addEv: EvProfile;
    editEv: EvProfile;
    seenMacs: EvSeenMac[];
}

function EvFormContent(props: {
    ev: EvProfile;
    onEvChange: (changes: Partial<EvProfile>) => void;
    macInvalid?: boolean;
    macError?: string;
}) {
    let rotationActive = is_rotating_mac(props.ev.mac);

    return <>
        <FormRow label={__("ev.content.add_ev_name")} label_muted={__("ev.content.add_ev_name_desc")}>
            <InputText
                value={props.ev.name}
                onValue={(v) => props.onEvChange({name: v})}
                maxLength={16}
                required
            />
        </FormRow>
        <FormRow label={__("ev.content.add_ev_mac")} label_muted={__("ev.content.add_ev_mac_desc")}>
            {rotationActive
                ? <div class="input-group">
                    <InputTextPatterned
                        value={mac_to_prefix(props.ev.mac)}
                        onValue={(v) => props.onEvChange({mac: v.toUpperCase() + 'XX:XX:XX'})}
                        pattern={MAC_PREFIX_PATTERN}
                        required
                        invalidFeedback={props.macError || __("ev.content.add_ev_mac_invalid")}
                        class={props.macInvalid ? "is-invalid" : ""}
                    />
                    <span class="input-group-text font-monospace">:XX:XX:XX</span>
                  </div>
                : <InputTextPatterned
                    value={props.ev.mac}
                    onValue={(v) => props.onEvChange({mac: v.toUpperCase()})}
                    pattern={MAC_PATTERN}
                    required
                    invalidFeedback={props.macError || __("ev.content.add_ev_mac_invalid")}
                    class={props.macInvalid ? "is-invalid" : ""}
                />
            }
        </FormRow>
        <FormRow label={__("ev.content.add_ev_rotating_mac")} label_muted={__("ev.content.add_ev_rotating_mac_desc")}>
            <Switch
                checked={rotationActive}
                onClick={() => {
                    if (rotationActive) {
                        // Switch off: expand XX:XX:XX to 00:00:00 as placeholder
                        let prefix = mac_to_prefix(props.ev.mac);
                        props.onEvChange({mac: prefix + ":00:00:00"});
                    } else {
                        // Switch on: replace last 3 octets with XX:XX:XX
                        let prefix = mac_to_prefix(props.ev.mac);
                        props.onEvChange({mac: prefix + ":XX:XX:XX"});
                    }
                }}
            />
        </FormRow>
        <FormRow label={__("ev.content.add_ev_capacity")} label_muted={__("ev.content.add_ev_capacity_desc")}>
            <InputFloat
                value={props.ev.capacity * 10}
                onValue={(v) => props.onEvChange({capacity: v / 10})}
                digits={1}
                min={10}
                max={2000}
                unit="kWh"
            />
        </FormRow>
        <FormRow label={__("ev.content.add_ev_efficiency")} label_muted={__("ev.content.add_ev_efficiency_desc")}>
            <InputFloat
                value={props.ev.charging_efficiency * 100}
                onValue={(v) => props.onEvChange({charging_efficiency: v / 100})}
                digits={0}
                min={50}
                max={100}
                unit="%"
            />
        </FormRow>
    </>;
}

function saveConfig(evs: EvProfile[]) {
    return API.save(
        "ev/config",
        {evs: evs},
        () => __("ev.script.save_failed"),
    );
}

export class Ev extends Component<{}, EvState> {
    interval: number = null;

    constructor() {
        super();

        this.state = {
            addEv: {name: "", mac: "", capacity: 60, charging_efficiency: 0.92, user_id: 0},
            editEv: {name: "", mac: "", capacity: 60, charging_efficiency: 0.92, user_id: 0},
            seenMacs: [],
        };

        util.addApiEventListener('ev/config', () => {
            this.forceUpdate();
        });

        util.addApiEventListener('ev/state', () => {
            this.forceUpdate();
        });

        util.addApiEventListener('ev/seen_macs', () => {
            this.refreshSeenMacs();
        });
    }

    refreshSeenMacs = () => {
        get_all_seen_macs().then((macs) => this.setState({seenMacs: macs}));
    };

    override componentDidMount() {
        this.refreshSeenMacs();
        this.interval = window.setInterval(() => {
            this.refreshSeenMacs();
            this.forceUpdate();
        }, 1000);
    }

    override componentWillUnmount() {
        if (this.interval !== null) {
            clearInterval(this.interval);
        }
    }

    checkMacDuplicate(mac: string, ignoreIndex?: number): boolean {
        return find_matching_ev(API.get("ev/config").evs, mac, ignoreIndex) >= 0;
    }

    render(props: {}, state: EvState) {
        if (!util.render_allowed())
            return <SubPage name="ev" />;

        const ev_config = API.get("ev/config");
        const ev_state = API.get("ev/state");
        const seen_macs = state.seenMacs;
        const has_active_ev = ev_state.mac !== "";
        const known_ev_index = has_active_ev ? find_matching_ev(ev_config.evs, ev_state.mac) : -1;
        const is_known_ev = known_ev_index >= 0;

        return (
            <SubPage name="ev" title={__("ev.content.ev")}>
                <SubPage.Status collapsed={!has_active_ev}>
                    {has_active_ev ? <>
                        <FormRow label={__("ev.content.currently_connected")}>
                            <InputText value={is_known_ev ? ev_config.evs[known_ev_index].name : ev_state.mac} />
                        </FormRow>
                        {is_known_ev &&
                            <FormRow label={__("ev.content.active_ev_mac")}>
                                <InputText value={ev_state.mac} />
                            </FormRow>
                        }
                        {Number.isFinite(ev_state.soc) &&
                            <FormRow label={__("ev.content.active_ev_soc")}>
                                <InputText value={util.toLocaleFixed(ev_state.soc, 1) + " %"} />
                            </FormRow>
                        }
                        {is_known_ev && Number.isFinite(ev_state.capacity) &&
                            <FormRow label={__("ev.content.active_ev_capacity")}>
                                <InputText value={util.toLocaleFixed(ev_state.capacity, 1) + " kWh"} />
                            </FormRow>
                        }
                        {is_known_ev &&
                            <FormRow label={__("ev.content.active_ev_efficiency")}>
                                <InputText value={util.toLocaleFixed(ev_state.charging_efficiency * 100, 0) + " %"} />
                            </FormRow>
                        }
                    </> : <FormRow label={__("ev.status.ev")}>
                        <InputText value={__("ev.content.no_active_ev")} />
                    </FormRow>}
                </SubPage.Status>

                <FormRow label={__("ev.content.ev_profiles")} label_muted={__("ev.content.ev_profiles_desc")}>
                    <Table
                    columnNames={[
                        __("ev.content.table_name"),
                        __("ev.content.table_mac"),
                        __("ev.content.table_capacity"),
                        __("ev.content.table_efficiency"),
                    ]}
                    rows={ev_config.evs.map((ev, i) => {
                        return {
                            columnValues: [
                                ev.name,
                                ev.mac,
                                util.toLocaleFixed(ev.capacity, 1) + " kWh",
                                util.toLocaleFixed(ev.charging_efficiency * 100, 0) + " %",
                            ],
                            editTitle: __("ev.content.edit_ev_title"),
                            onEditShow: async () => {
                                this.setState({
                                    editEv: {
                                        name: ev.name,
                                        mac: ev.mac,
                                        capacity: ev.capacity,
                                        charging_efficiency: ev.charging_efficiency,
                                        user_id: ev.user_id,
                                    }
                                });
                            },
                            onEditGetChildren: () => [
                                <EvFormContent
                                    ev={state.editEv}
                                    onEvChange={(changes) => this.setState({editEv: {...state.editEv, ...changes}})}
                                    macInvalid={this.checkMacDuplicate(state.editEv.mac, i)}
                                    macError={this.checkMacDuplicate(state.editEv.mac, i) ? __("ev.content.mac_duplicate") : undefined}
                                />
                            ],
                            onEditCheck: async () => {
                                return !this.checkMacDuplicate(state.editEv.mac, i);
                            },
                            onEditSubmit: async () => {
                                let evs = ev_config.evs.slice();
                                evs[i] = state.editEv;
                                await saveConfig(evs);
                            },
                            onRemoveClick: async () => {
                                await saveConfig(ev_config.evs.filter((_, index) => index !== i));
                                return true;
                            },
                        };
                    })}
                    addEnabled={ev_config.evs.length < EV_MAX_EVS}
                    addTitle={__("ev.content.add_ev_title")}
                    addMessage={__("ev.content.add_ev_message")(ev_config.evs.length, EV_MAX_EVS)}
                    onAddShow={async () => this.setState({
                        addEv: {name: "", mac: "", capacity: 60, charging_efficiency: 0.92, user_id: 0},
                    })}
                    onAddGetChildren={() => {
                        const visible_macs = seen_macs.filter(m => m.mac !== "");
                        const unauth_macs = visible_macs.filter(m => find_matching_ev(ev_config.evs, m.mac) < 0);
                        const auth_macs = visible_macs.filter(m => find_matching_ev(ev_config.evs, m.mac) >= 0);

                        return [<>
                            <FormRow label={__("ev.content.add_ev_seen_macs")}>
                                {visible_macs.length > 0 ?
                                    <DiscoveryResultGroup>{
                                        unauth_macs.map(m => <DiscoveryResultItem
                                            key={m.mac}
                                            title={<h5 class="font-monospace">{m.mac}</h5>}
                                            labelAdd={<Plus />}
                                            onClick={() => this.setState({addEv: {...state.addEv, mac: m.mac}})}>
                                            <div>{__("ev.content.seen_mac_last_seen")(util.format_timespan(Math.floor(Date.now() / 1000) - m.last_seen))}</div>
                                            {m.charger_name ? <div class="text-muted small">{__("ev.content.seen_mac_charger")(m.charger_name)}</div> : null}
                                        </DiscoveryResultItem>).concat(
                                        auth_macs.map(m => {
                                            let index = find_matching_ev(ev_config.evs, m.mac);
                                            return <DiscoveryResultItem
                                                key={m.mac}
                                                title={<h5 class="font-monospace">{m.mac}</h5>}
                                                labelAdd={<Plus />}
                                                error={__("ev.content.mac_already_configured")(ev_config.evs[index].name)}
                                                onClick={() => this.setState({addEv: {...state.addEv, mac: m.mac}})}>
                                                <div>{__("ev.content.seen_mac_last_seen")(util.format_timespan(Math.floor(Date.now() / 1000) - m.last_seen))}</div>
                                                {m.charger_name ? <div class="text-muted small">{__("ev.content.seen_mac_charger")(m.charger_name)}</div> : null}
                                            </DiscoveryResultItem>;
                                        }))
                                    }</DiscoveryResultGroup>
                                    : <div class="form-control-plaintext text-center">{__("ev.content.no_seen_macs")}</div>}
                            </FormRow>
                            <EvFormContent
                                ev={state.addEv}
                                onEvChange={(changes) => this.setState({addEv: {...state.addEv, ...changes}})}
                                macInvalid={this.checkMacDuplicate(state.addEv.mac)}
                                macError={this.checkMacDuplicate(state.addEv.mac) ? __("ev.content.mac_duplicate") : undefined}
                            />
                        </>];
                    }}
                    onAddCheck={async () => {
                        return !this.checkMacDuplicate(state.addEv.mac);
                    }}
                    onAddSubmit={async () => {
                        await saveConfig(ev_config.evs.concat(state.addEv));
                    }}
                    />
                </FormRow>
            </SubPage>
        );
    }
}

export class EvStatus extends Component<{}, {}> {
    render() {
        return <StatusSection name="ev" />;
    }
}

export function pre_init() {
}

export function init() {
}
