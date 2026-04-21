/* esp32-firmware
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
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

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import {h, Fragment, ComponentChild} from "preact";
import {__} from "../../ts/translation";
import {ConfigComponent} from "../../ts/components/config_component";
import {FormRow} from "../../ts/components/form_row";
import {InputSelect} from "../../ts/components/input_select";
import {InputText} from "../../ts/components/input_text";
import {SubPage} from "../../ts/components/sub_page";
import {NavbarItem} from "../../ts/components/navbar_item";
import {CollapsedSection} from "../../ts/components/collapsed_section";
import {Table, TableRow} from "../../ts/components/table";
import {Button, ListGroup, ListGroupItem} from "react-bootstrap";
import {Share2, Plus} from "react-feather";
import {Switch} from "../../ts/components/switch";
import {ShipDiscoveryState} from "./generated/ship_discovery_state.enum";
import {NodeState} from "./generated/node_state.enum";
import {LoadcontrolState} from "./generated/loadcontrol_state.enum";
import {useState} from "preact/hooks";
import {Usecases} from "./generated/usecases.enum";
import {OutputFloat} from "../../ts/components/output_float";
import {register_status_provider, ModuleStatus} from "../../ts/status_registry";
import {IPConfiguration} from "../../ts/components/ip_configuration";
import { DiscoveryResultGroup, DiscoveryResultItem } from "ts/components/discovery_result";

const EEBUS_NO_VALUE = -2147483648;

const loadcontrolStateMap: { [key: number]: string } = {
    [LoadcontrolState.Startup]: "Startup",
    [LoadcontrolState.Init]: "Init",
    [LoadcontrolState.UnlimitedControlled]: "UnlimitedControlled",
    [LoadcontrolState.Limited]: "Limited",
    [LoadcontrolState.Failsafe]: "Failsafe",
    [LoadcontrolState.UnlimitedAutonomous]: "UnlimitedAutonomous"
};

/** Convert a NodeState enum value to a localized display string. */
function nodeStateToString(state: number): string {
    switch (state) {
        case NodeState.Disconnected:     return __("eebus.content.peer_info.state_disconnected");
        case NodeState.Discovered:       return __("eebus.content.peer_info.state_discovered");
        case NodeState.Connected:        return __("eebus.content.peer_info.state_connected");
        case NodeState.EEBUSActive:      return __("eebus.content.peer_info.state_eebus_connected");
        case NodeState.EEBUSDegraded:    return __("eebus.content.peer_info.state_degraded");
        case NodeState.LoadedFromConfig: return __("eebus.content.peer_info.state_loaded_from_config");
        case NodeState.AwaitingApproval: return __("eebus.content.peer_info.state_awaiting_approval");
        case NodeState.Connecting:       return __("eebus.content.peer_info.state_connecting");
        default:                         return __("eebus.content.unknown");
    }
}

/** Get a human-readable display name for a peer, falling back to "Unknown Device". */
function peerDisplayName(peer: {model_model: string; model_brand: string; ip: string; dns_name: string}): string {
    if (peer.model_model) return peer.model_model;
    if (peer.dns_name)    return peer.dns_name;
    if (peer.ip)          return peer.ip;
    return __("eebus.content.peer_info.unknown_device");
}

function ExpandableAddress({dns, ip}: { dns: string; ip: string }) {
    const [expanded, setExpanded] = useState(false);
    const ipLines = (ip || "").split(";").map(s => s.trim()).filter(Boolean);
    const hasDns = (dns || "").length >= 2;
    const topLine = hasDns ? dns : (ipLines[0] ?? "");
    const lines = hasDns ? [dns, ...ipLines] : ipLines;

    return (
        <div
            role="button"
            tabIndex={0}
            onClick={() => setExpanded(!expanded)}
            onKeyDown={(e) => {
                if (e.key === "Enter" || e.key === " ") setExpanded(!expanded);
            }}
            style={{cursor: "pointer", whiteSpace: "pre-line"}}
        >
            {expanded ? lines.join("\n") : topLine + "..."}
        </div>
    );
}

function PhaseRow(props: {
    label: string,
    label_muted: string,
    values: [number, number, number],
    unit: string,
    digits?: 0 | 1 | 2 | 3,
    scale?: number
}) {
    if (props.values[0] === EEBUS_NO_VALUE && props.values[1] === EEBUS_NO_VALUE && props.values[2] === EEBUS_NO_VALUE) {
        return null;
    }
    const digits = props.digits ?? 0;
    const scale = props.scale ?? 0;
    return <FormRow label={props.label} label_muted={props.label_muted} small>
        <div class="row gx-2 gy-1">
            <div class="col-sm-4">
                <OutputFloat value={props.values[0]} digits={digits} scale={scale} unit={props.unit} small/>
            </div>
            <div class="col-sm-4">
                <OutputFloat value={props.values[1]} digits={digits} scale={scale} unit={props.unit} small/>
            </div>
            <div class="col-sm-4">
                <OutputFloat value={props.values[2]} digits={digits} scale={scale} unit={props.unit} small/>
            </div>
        </div>
    </FormRow>;
}

/**
 * Build the EEBUS help text dynamically based on enabled use cases.
 * Only includes descriptions for use cases that are actually supported.
 */
function buildEEBusHelpText(usecases: EEBusUsecases | undefined): ComponentChild {
    // Mapping of use case enum values to their descriptions (as ComponentChild)
    const usecaseDescMap: { [key: number]: { name: string, desc: string } } = {
        [Usecases.LPC]: {
            name: "LPC (Limitation of Power Consumption)",
            desc: __("eebus.content.usecase_lpc_desc")
        },
        [Usecases.LPP]: {
            name: "LPP (Limitation of Power Production)",
            desc: __("eebus.content.usecase_lpp_desc")
        },
        [Usecases.MPC]: {
            name: "MPC (Monitoring of Power Consumption)",
            desc: __("eebus.content.usecase_mpc_desc")
        },
        [Usecases.MGCP]: {
            name: "MGCP (Monitoring of Grid Connection Point)",
            desc: __("eebus.content.usecase_mgcp_desc")
        },
        [Usecases.CEVC]: {
            name: "CEVC (Coordinated EV Charging)",
            desc: __("eebus.content.usecase_cevc_desc")
        },
        [Usecases.EVCC]: {
            name: "EVCC (EV Commissioning and Configuration)",
            desc: __("eebus.content.usecase_evcc_desc")
        },
        [Usecases.EVCEM]: {
            name: "EVCEM (EV Charging Electricity Measurement)",
            desc: __("eebus.content.usecase_evcem_desc")
        },
        [Usecases.EVSECC]: {
            name: "EVSECC (EVSE Commissioning and Configuration)",
            desc: __("eebus.content.usecase_evsecc_desc")
        },
        [Usecases.EVCS]: {
            name: "EVCS (EV Charging Summary)",
            desc: __("eebus.content.usecase_evcs_desc")
        },
        [Usecases.OPEV]: {
            name: "OPEV (Overload Protection by EV Charging Current Curtailment)",
            desc: __("eebus.content.usecase_opev_desc")
        },
    };

    // Map use case enum values to the corresponding optional API property names
    const usecaseApiKeyMap: { [key: number]: keyof NonNullable<typeof usecases> } = {
        [Usecases.LPC]: "lpc",
        [Usecases.LPP]: "lpp",
        [Usecases.EVCC]: "evcc",
        [Usecases.EVCEM]: "evcem",
        [Usecases.EVSECC]: "evsecc",
        [Usecases.EVCS]: "evcs",
        [Usecases.MPC]: "mpc",
        [Usecases.CEVC]: "cevc",
        [Usecases.MGCP]: "mgcp",
        [Usecases.OPEV]: "opev",
    };

    // Build the list items only for use cases present in the API response
    const usecaseItems = Object.keys(usecaseDescMap).map(Number).map(uc => {
        const apiKey = usecaseApiKeyMap[uc];
        if (!apiKey || usecases?.[apiKey] == null) return null;
        const info = usecaseDescMap[uc];
        return (
            <li key={uc}>
                <b>{info.name}</b>: {info.desc}
            </li>
        );
    }).filter(item => item !== null);

    return (
        <>
            {__("eebus.content.enable_eebus_help_intro")}
            {usecaseItems.length > 0 && (
                <>
                    {__("eebus.content.enable_eebus_help_usecases_intro")}
                    <ul>{usecaseItems}</ul>
                </>
            )}
        </>
    );
}

const EEBusLogo = <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 6.35 6.35"><defs><clipPath id="a" clipPathUnits="userSpaceOnUse"><path d="M-45.28-91.6H88.44V30.23H-45.28Z"/></clipPath></defs><path d="M0 0c14.1 0 26.35-6.83 32.36-16.77a35.23 35.23 0 0 1-64.72 0C-26.34-6.83-14.1 0 0 0" clip-path="url(#a)" style="fill:currentColor;fill-opacity:1;fill-rule:nonzero;stroke:none;stroke-width:3.91262641;stroke-dasharray:none" transform="matrix(.09 0 0 -.09 3.18 .4)"/><path d="M3.18 5.94A3.4 3.4 0 0 1 .26 4.43a3.18 3.18 0 0 0 5.84 0 3.4 3.4 0 0 1-2.92 1.51" style="fill:currentColor;fill-opacity:1;fill-rule:nonzero;stroke:none;stroke-width:.352777"/><path d="M3.96 1.72q-.34.21-.4.68L3.3 3.97l-.03.25q.02.61.75.62h1.26l.07-.47H3.98a.3.3 0 0 1-.16-.09.3.3 0 0 1-.06-.17v-.03l.03-.2.07-.44h1.66l.08-.47H3.94l.08-.56c.04-.2.18-.35.35-.35h1.37l.08-.47h-1.2c-.34 0-.48.02-.66.13M1.21 1.72q-.34.21-.41.68L.55 3.97l-.02.25q.02.61.74.62h1.27l.07-.47H1.22a.3.3 0 0 1-.15-.09.3.3 0 0 1-.07-.17v-.03l.03-.2.07-.44h1.67l.07-.47H1.18l.09-.56c.03-.2.18-.35.35-.35h1.37l.08-.47h-1.2c-.35 0-.48.02-.66.13" style="fill:currentColor;fill-opacity:1;fill-rule:nonzero;stroke:none;stroke-width:.352778"/></svg>

export function EEBusNavbar() {
    return <NavbarItem name="eebus" module="eebus" title="EEBUS" symbol={EEBusLogo}/>;
}

type EEBusConfig = API.getType["eebus/config"];
type EEBusAdd = API.getType["eebus/add"];
type EEBusRemove = API.getType["eebus/remove"];
type EEBusStateType = API.getType["eebus/state"];
type EEBusUsecases = API.getType["eebus/usecases"];

interface EEBusState {
    add: EEBusAdd;
    state: EEBusStateType;
    config: EEBusConfig;
    config_enable: boolean;
    usecases: EEBusUsecases;
    addPeerSkiError: string;
    addPeerIpError: string;
}

export class EEBus extends ConfigComponent<'eebus/config', {}, EEBusState> {
    scan_interval_id: number = null;
    ipconfig_valid: boolean = true;  // IP configuration validation state

    constructor() {
        super('eebus/config', () => __("eebus.script.save_failed"));
        util.addApiEventListener('eebus/state', () => {
            this.setState({state: API.get('eebus/state')});
        });
        util.addApiEventListener('eebus/config', () => {
            let config = API.get('eebus/config');
            this.setState({config: config, config_enable: config.enable});
        });
        util.addApiEventListener('eebus/config', () => {
            this.setState({usecases: API.get('eebus/usecases')});
        });
    }

    async scan_services() {
        try {
            await API.call('eebus/scan', {});
        } catch {
            return;
        }
    }

    override async sendSave(topic: "eebus/config", cfg: EEBusConfig) {
        this.setState({config_enable: cfg.enable}); // avoid round trip time
        // TODO: why is this unchecked? Currently cargo-culted from OCPP and Modbus TCP. Maybe add those API definitions to ocpp/api.ts (or modbus_tcp or eebus) with //APIPath?
        await API.save_unchecked('evse/eebus_enabled', {enabled: cfg.enable}, () => __("eebus.script.save_failed"));
        await super.sendSave(topic, cfg);
    }

    override async sendReset(topic: "eebus/config") {
        this.setState({config_enable: false}); // avoid round trip time
        await API.save_unchecked('evse/eebus_enabled', {enabled: false}, () => __("eebus.script.save_failed"));
        await super.sendReset(topic);
    }

    override getIsModified(topic: "eebus/config"): boolean {
        let evse = API.get_unchecked("evse/eebus_enabled");
        if (evse != null && evse.enabled)
            return true;
        return super.getIsModified(topic);
    }

    render(props: {}, state: EEBusState & EEBusConfig) {
        if (!util.render_allowed())
            return <SubPage name="eebus"/>;

        let ski = state.state.ski;
        if (ski == "") {
            ski = __("eebus.content.unknown");
        }

        return (
            <SubPage name="eebus" title="EEBUS">
                <SubPage.Status collapsed={!state.config_enable}>
                        <FormRow label={__("eebus.content.ski")} label_muted={__("eebus.content.ski_muted")}
                                 help={__("eebus.content.ski_help")}>
                            <InputText value={ski}/>
                        </FormRow>

                        <CollapsedSection heading={__("eebus.content.usecase_details")}>
                            <Table
                                columnNames={[""]}
                                rows={(() => {
                                    let rows: TableRow[] = [];

                                    // Limitation of Power Consumption
                                    if (state.usecases.lpc != null) {
                                        const lpc = state.usecases.lpc;
                                        rows.push({
                                            hideRemoveButton: true,
                                            columnValues: ["LPC (Limitation of Power Consumption)"],
                                            extraValue: <>
                                                <FormRow label="Usecase State" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <InputText class="form-control-sm"
                                                                       value={loadcontrolStateMap[lpc.usecase_state] ?? lpc.usecase_state}/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                <FormRow label="Limit Active"
                                                         label_muted={__("eebus.content.usecase_value_writable")} small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <InputText class="form-control-sm"
                                                                       value={lpc.limit_active ? __("eebus.content.yes") : __("eebus.content.no")}/>
                                                        </div>
                                                        {lpc.usecase_state === LoadcontrolState.Limited && lpc.outstanding_duration_s != null ?
                                                            <div class="col-sm-4">
                                                                <OutputFloat value={lpc.outstanding_duration_s}
                                                                             digits={0}
                                                                             scale={0} unit="s" small/>
                                                            </div> : undefined}
                                                    </div>
                                                </FormRow>
                                                <FormRow label="Current Limit"
                                                         label_muted={__("eebus.content.usecase_value_writable")} small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={lpc.current_limit} digits={0} scale={0}
                                                                         unit="W"
                                                                         small/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                <FormRow label="Failsafe Limit Power"
                                                         label_muted={__("eebus.content.usecase_value_writable")} small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={lpc.failsafe_limit_power_w} digits={0}
                                                                         scale={0}
                                                                         unit="W" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                <FormRow label="Failsafe Limit Duration"
                                                         label_muted={__("eebus.content.usecase_value_writable")} small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={lpc.failsafe_limit_duration_s}
                                                                         digits={0}
                                                                         scale={0} unit="s" small/>
                                                        </div>
                                                        {lpc.usecase_state === LoadcontrolState.Limited && lpc.outstanding_duration_s != null ?
                                                            <div class="col-sm-4">
                                                                <OutputFloat value={lpc.outstanding_duration_s}
                                                                             digits={0}
                                                                             scale={0} unit="s" small/>
                                                            </div> : undefined}
                                                    </div>
                                                </FormRow>
                                                <FormRow label="Constraints Power Maximum" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={lpc.constraints_power_maximum}
                                                                         digits={0}
                                                                         scale={0} unit="W" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                            </>
                                        });
                                    }

                                    // Limitation of Power Production
                                    if (state.usecases.lpp != null) {
                                        const lpp = state.usecases.lpp;
                                        rows.push({
                                            hideRemoveButton: true,
                                            columnValues: ["LPP (Limitation of Power Production)"],
                                            extraValue: <>
                                                <FormRow label="Usecase State" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <InputText class="form-control-sm"
                                                                       value={loadcontrolStateMap[lpp.usecase_state] ?? lpp.usecase_state}/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                <FormRow label="Limit Active"
                                                         label_muted={__("eebus.content.usecase_value_writable")} small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <InputText class="form-control-sm"
                                                                       value={lpp.limit_active ? __("eebus.content.yes") : __("eebus.content.no")}/>
                                                        </div>
                                                        {lpp.usecase_state === LoadcontrolState.Limited && lpp.outstanding_duration_s != null ?
                                                            <div class="col-sm-4">
                                                                <OutputFloat value={lpp.outstanding_duration_s}
                                                                             digits={0}
                                                                             scale={0} unit="s" small/>
                                                            </div> : undefined}
                                                    </div>
                                                </FormRow>
                                                <FormRow label="Current Limit"
                                                         label_muted={__("eebus.content.usecase_value_writable")} small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={lpp.current_limit} digits={0} scale={0}
                                                                         unit="W"
                                                                         small/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                <FormRow label="Failsafe Limit Power"
                                                         label_muted={__("eebus.content.usecase_value_writable")} small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={lpp.failsafe_limit_power_w} digits={0}
                                                                         scale={0}
                                                                         unit="W" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                <FormRow label="Failsafe Limit Duration"
                                                         label_muted={__("eebus.content.usecase_value_writable")} small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={lpp.failsafe_limit_duration_s}
                                                                         digits={0}
                                                                         scale={0} unit="s" small/>
                                                        </div>
                                                        {lpp.usecase_state === LoadcontrolState.Limited && lpp.outstanding_duration_s != null ?
                                                            <div class="col-sm-4">
                                                                <OutputFloat value={lpp.outstanding_duration_s}
                                                                             digits={0}
                                                                             scale={0} unit="s" small/>
                                                            </div> : undefined}
                                                    </div>
                                                </FormRow>
                                                <FormRow label="Constraints Power Maximum" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={lpp.constraints_power_maximum}
                                                                         digits={0}
                                                                         scale={0} unit="W" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                            </>
                                        });
                                    }

                                    // EV Commissioning and Configuration
                                    if (state.usecases.evcc != null) {
                                        const evcc = state.usecases.evcc;
                                        rows.push({
                                            hideRemoveButton: true,
                                            columnValues: ["EVCC (EV Commissioning and Configuration)"],
                                            extraValue: <>
                                                <FormRow label="EV Connected" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <InputText class="form-control-sm"
                                                                       value={evcc.ev_connected ? __("eebus.content.yes") : __("eebus.content.no")}/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                {evcc.ev_connected && <>
                                                    <FormRow label="Communication Standard" small>
                                                        <div class="row gx-2 gy-1">
                                                            <div class="col-sm-4">
                                                                <InputText class="form-control-sm"
                                                                           value={evcc.communication_standard}/>
                                                            </div>
                                                        </div>
                                                    </FormRow>
                                                    <FormRow label="Asymmetric Charging Supported" small>
                                                        <div class="row gx-2 gy-1">
                                                            <div class="col-sm-4">
                                                                <InputText class="form-control-sm"
                                                                           value={evcc.asymmetric_charging_supported ? __("eebus.content.yes") : __("eebus.content.no")}/>
                                                            </div>
                                                        </div>
                                                    </FormRow>
                                                    <FormRow label="Vehicle MAC Address" small>
                                                        <div class="row gx-2 gy-1">
                                                            <div class="col-sm-4">
                                                                <InputText class="form-control-sm"
                                                                           value={evcc.mac_address}/>
                                                            </div>
                                                        </div>
                                                    </FormRow>
                                                    <FormRow label="Minimum Power (reported by Vehicle)" small>
                                                        <div class="row gx-2 gy-1">
                                                            <div class="col-sm-4">
                                                                <OutputFloat value={evcc.minimum_power} digits={0}
                                                                             scale={0} unit="W" small/>
                                                            </div>
                                                        </div>
                                                    </FormRow>
                                                    <FormRow label="Maximum Power (reported by Vehicle)" small>
                                                        <div class="row gx-2 gy-1">
                                                            <div class="col-sm-4">
                                                                <OutputFloat value={evcc.maximum_power} digits={0}
                                                                             scale={0} unit="W" small/>
                                                            </div>
                                                        </div>
                                                    </FormRow>
                                                    <FormRow label="Standby Power (reported by Vehicle)" small>
                                                        <div class="row gx-2 gy-1">
                                                            <div class="col-sm-4">
                                                                <OutputFloat value={evcc.standby_power} digits={0}
                                                                             scale={0} unit="W" small/>
                                                            </div>
                                                        </div>
                                                    </FormRow>
                                                    <FormRow label="Standby Mode Active" small>
                                                        <div class="row gx-2 gy-1">
                                                            <div class="col-sm-4">
                                                                <InputText class="form-control-sm"
                                                                           value={evcc.standby_mode ? __("eebus.content.yes") : __("eebus.content.no")}/>
                                                            </div>
                                                        </div>
                                                    </FormRow>
                                                </>}
                                            </>
                                        });
                                    }

                                    // EV Charging Electricity Measurement
                                    if (state.usecases.evcem != null) {
                                        const evcc = state.usecases.evcc;
                                        const evcem = state.usecases.evcem;
                                        rows.push({
                                            hideRemoveButton: true,
                                            columnValues: ["EVCEM (EV Charging Electricity Measurement)"],
                                            extraValue: <>
                                                {!evcem.active && <div class="alert alert-secondary mb-2 py-1 px-2" role="alert">
                                                    <small>{__("eebus.content.usecase_inactive")}</small>
                                                </div>}
                                                <div style={!evcem.active ? "opacity: 0.4; pointer-events: none;" : ""}>
                                                <FormRow label="EV Connected" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <InputText class="form-control-sm"
                                                                       value={evcc.ev_connected ? __("eebus.content.yes") : __("eebus.content.no")}/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                {evcc.ev_connected && <>
                                                    <PhaseRow label="Amps" label_muted="L1, L2, L3" values={[
                                                        evcem.amps_phase_1,
                                                        evcem.amps_phase_2,
                                                        evcem.amps_phase_3
                                                    ]} unit="A"/>
                                                    <PhaseRow label="Power" label_muted="L1, L2, L3" values={[
                                                        evcem.power_phase_1,
                                                        evcem.power_phase_2,
                                                        evcem.power_phase_3
                                                    ]} unit="W"/>
                                                    {evcem.charged_wh !== EEBUS_NO_VALUE &&
                                                    <FormRow label="Charged" small>
                                                        <div class="row gx-2 gy-1">
                                                            <div class="col-sm-4">
                                                                <OutputFloat value={evcem.charged_wh} digits={0}
                                                                             scale={0} unit="Wh" small/>
                                                            </div>
                                                        </div>
                                                    </FormRow>}
                                                    <FormRow label="Method of Obtaining Charged Wh" small>
                                                        <div class="row gx-2 gy-1">
                                                            <div class="col-sm-4">
                                                                <InputText class="form-control-sm"
                                                                           value={evcem.charged_valuesource_measured ? "Measured" : "Calculated"}/>
                                                            </div>
                                                        </div>
                                                    </FormRow>
                                                </>}
                                                </div>
                                            </>
                                        });
                                    }

                                    // EVSE Commissioning and Configuration
                                    if (state.usecases.evsecc != null) {
                                        const evsecc = state.usecases.evsecc;
                                        rows.push({
                                            hideRemoveButton: true,
                                            columnValues: ["EVSECC (EVSE Commissioning and Configuration)"],
                                            extraValue: <>
                                                <FormRow label="EVSE in Failure State" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <InputText class="form-control-sm"
                                                                       value={evsecc.evse_failure ? __("eebus.content.yes") : __("eebus.content.no")}/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                {evsecc.evse_failure && <>
                                                    <FormRow label="Failure Message" small>
                                                        <div class="row gx-2 gy-1">
                                                            <div class="col-sm-4">
                                                                <InputText class="form-control-sm"
                                                                           value={evsecc.evse_failure_description}/>
                                                            </div>
                                                        </div>
                                                    </FormRow>
                                                </>}
                                            </>
                                        });
                                    }

                                    // EV Charging Summary
                                    if (state.usecases.evcs != null) {
                                        const summary = state.usecases.evcs;
                                        rows.push({
                                            hideRemoveButton: true,
                                            columnValues: ["EVCS (EV Charging Summary)"],
                                            extraValue: <>
                                                <FormRow label="Number of Charge Processes" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={summary.length} digits={0} scale={0}
                                                                         unit="" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                {summary.map((item, idx) =>
                                                    <FormRow label={`Process ${item.id}`} key={idx} small>
                                                        <div class="row gx-2 gy-1">
                                                            <div class="col-sm-4">
                                                                <InputText class="form-control-sm"
                                                                           value={`${item.charged_kwh} kWh, Cost: ${item.cost}, Self-produced: ${item.percent_self_produced_energy}%`}/>
                                                            </div>
                                                        </div>
                                                    </FormRow>
                                                )}
                                            </>
                                        });
                                    }

                                    // Monitoring of Power Consumption
                                    if (state.usecases.mpc != null) {
                                        const mpc = state.usecases.mpc;
                                        rows.push({
                                            hideRemoveButton: true,
                                            columnValues: ["MPC (Monitoring of Power Consumption)"],
                                            extraValue: <>
                                                {!mpc.active && <div class="alert alert-secondary mb-2 py-1 px-2" role="alert">
                                                    <small>{__("eebus.content.usecase_inactive")}</small>
                                                </div>}
                                                <div style={!mpc.active ? "opacity: 0.4; pointer-events: none;" : ""}>
                                                {mpc.total_power_w !== EEBUS_NO_VALUE &&
                                                <FormRow label="Total Power" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={mpc.total_power_w} digits={0} scale={0}
                                                                         unit="W" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>}
                                                <PhaseRow label="Power" label_muted="L1, L2, L3" values={[
                                                    mpc.power_phase_1_w,
                                                    mpc.power_phase_2_w,
                                                    mpc.power_phase_3_w
                                                ]} unit="W"/>
                                                {mpc.energy_consumed_wh !== EEBUS_NO_VALUE &&
                                                <FormRow label="Energy Consumed" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={mpc.energy_consumed_wh} digits={0}
                                                                         scale={0} unit="Wh" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>}
                                                {mpc.energy_produced_wh !== EEBUS_NO_VALUE &&
                                                <FormRow label="Energy Produced" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={mpc.energy_produced_wh} digits={0}
                                                                         scale={0} unit="Wh" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>}
                                                <PhaseRow label="Current" label_muted="L1, L2, L3" values={[
                                                    mpc.current_phase_1_ma,
                                                    mpc.current_phase_2_ma,
                                                    mpc.current_phase_3_ma
                                                ]} unit="mA"/>
                                                <PhaseRow label="Voltage (L-N)" label_muted="L1-N, L2-N, L3-N" values={[
                                                    mpc.voltage_phase_1_v,
                                                    mpc.voltage_phase_2_v,
                                                    mpc.voltage_phase_3_v
                                                ]} unit="V"/>
                                                <PhaseRow label="Voltage (L-L)" label_muted="L1-L2, L2-L3, L3-L1"
                                                          values={[
                                                              mpc.voltage_phase_1_2_v,
                                                              mpc.voltage_phase_2_3_v,
                                                              mpc.voltage_phase_3_1_v
                                                          ]} unit="V"/>
                                                {mpc.frequency_mhz !== EEBUS_NO_VALUE &&
                                                <FormRow label="Grid Frequency" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={mpc.frequency_mhz} digits={2} scale={3}
                                                                         unit="Hz" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>}
                                                </div>
                                            </>
                                        });
                                    }

                                    // Coordinated EV Charging
                                    if (state.usecases.cevc != null) {
                                        const cevc = state.usecases.cevc;
                                        rows.push({
                                            hideRemoveButton: true,
                                            columnValues: ["CEVC (Coordinated EV Charging)"],
                                            extraValue: <>
                                                <FormRow label="Energy Broker Connected" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <InputText class="form-control-sm"
                                                                       value={cevc.energy_broker_connected ? __("eebus.content.yes") : __("eebus.content.no")}/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                <FormRow label="Energy Broker Heartbeat OK" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <InputText class="form-control-sm"
                                                                       value={cevc.energy_broker_heartbeat_ok ? __("eebus.content.yes") : __("eebus.content.no")}/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                <FormRow label="Has Charging Plan" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <InputText class="form-control-sm" value={
                                                                (cevc.has_charging_plan ? __("eebus.content.yes") : __("eebus.content.no")) +
                                                                (cevc.has_charging_plan && cevc.charging_plan_start_time > 0
                                                                    ? ` (from: ${new Date(cevc.charging_plan_start_time * 1000).toLocaleString()})`
                                                                    : "")
                                                            }/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                {cevc.has_charging_plan && <>
                                                    <FormRow label="Current Target Power" small>
                                                        <div class="row gx-2 gy-1">
                                                            <div class="col-sm-4">
                                                                <OutputFloat value={cevc.target_power_w} digits={0}
                                                                             scale={0} unit="W" small/>
                                                            </div>
                                                        </div>
                                                    </FormRow>
                                                </>}
                                                <FormRow label="Has Incentives" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <InputText class="form-control-sm"
                                                                       value={cevc.has_incentives ? __("eebus.content.yes") : __("eebus.content.no")}/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                            </>
                                        });
                                    }

                                    // Monitoring of Grid Connection Point (MGCP)
                                    if (state.usecases.mgcp != null) {
                                        const mgcp = state.usecases.mgcp;
                                        rows.push({
                                            hideRemoveButton: true,
                                            columnValues: ["MGCP (Monitoring of Grid Connection Point)"],
                                            extraValue: <>
                                                {!mgcp.active && <div class="alert alert-secondary mb-2 py-1 px-2" role="alert">
                                                    <small>{__("eebus.content.usecase_inactive")}</small>
                                                </div>}
                                                <div style={!mgcp.active ? "opacity: 0.4; pointer-events: none;" : ""}>
                                                <FormRow label="PV Curtailment Factor" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat
                                                                value={mgcp.pv_curtailment_limit_factor_percent}
                                                                digits={1} scale={0} unit="%" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                {mgcp.total_power_w !== EEBUS_NO_VALUE &&
                                                <FormRow label="Total Power" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={mgcp.total_power_w} digits={0} scale={0}
                                                                         unit="W" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>}
                                                {mgcp.energy_feed_in_wh > 0 &&
                                                <FormRow label="Energy Feed-In" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={mgcp.energy_feed_in_wh} digits={0}
                                                                         scale={0} unit="Wh" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>}
                                                {mgcp.energy_consumed_wh > 0 &&
                                                <FormRow label="Energy Consumed" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={mgcp.energy_consumed_wh} digits={0}
                                                                         scale={0} unit="Wh" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>}
                                                <PhaseRow label="Current" label_muted="L1, L2, L3" values={[
                                                    mgcp.current_phase_1_ma,
                                                    mgcp.current_phase_2_ma,
                                                    mgcp.current_phase_3_ma
                                                ]} unit="mA"/>
                                                <PhaseRow label="Voltage" label_muted="L1, L2, L3" values={[
                                                    mgcp.voltage_phase_1_v,
                                                    mgcp.voltage_phase_2_v,
                                                    mgcp.voltage_phase_3_v
                                                ]} unit="V"/>
                                                {mgcp.frequency_mhz !== EEBUS_NO_VALUE &&
                                                <FormRow label="Grid Frequency" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={mgcp.frequency_mhz} digits={2} scale={3}
                                                                         unit="Hz" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>}
                                                </div>
                                            </>
                                        });
                                    }

                                    // Overload Protection by EV Charging Current Curtailment (OPEV)
                                    if (state.usecases.opev != null) {
                                        const opev = state.usecases.opev;
                                        rows.push({
                                            hideRemoveButton: true,
                                            columnValues: ["OPEV (Overload Protection)"],
                                            extraValue: <>
                                                <FormRow label="Limit Active"
                                                         label_muted={__("eebus.content.usecase_value_writable")} small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <InputText class="form-control-sm"
                                                                       value={opev.limit_active ? __("eebus.content.yes") : __("eebus.content.no")}/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                <PhaseRow label="Current Limit"
                                                          label_muted={["L1", "L2", "L3", __("eebus.content.usecase_value_writable")].filter(Boolean).join(", ")}
                                                          values={[
                                                              opev.limit_phase_1_milliamps,
                                                              opev.limit_phase_2_milliamps,
                                                              opev.limit_phase_3_milliamps
                                                          ]} unit="mA"/>
                                                <FormRow label="Min Limit" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={opev.limit_milliamps_min} digits={0}
                                                                         scale={0} unit="mA" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                                <FormRow label="Max Limit" small>
                                                    <div class="row gx-2 gy-1">
                                                        <div class="col-sm-4">
                                                            <OutputFloat value={opev.limit_milliamps_max} digits={0}
                                                                         scale={0} unit="mA" small/>
                                                        </div>
                                                    </div>
                                                </FormRow>
                                            </>
                                        });
                                    }

                                    return rows;
                                })()}
                            />
                        </CollapsedSection>
                    </SubPage.Status>

                <SubPage.Config id="eebus_config_form" isModified={this.isModified()} isDirty={this.isDirty()}
                                onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("eebus.content.enable_eebus")} help={buildEEBusHelpText(state.usecases)}>
                        <Switch desc={__("eebus.content.enable_eebus_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}
                        />
                    </FormRow>
                    <FormRow label={__("eebus.content.peer_info.peers")}
                             help={__("eebus.content.peer_info.peers_desc")}>
                        <Table nestingDepth={1} // We are not nested, but this also reduces the modal's size to lg
                            columnNames={[
                                __("eebus.content.peer_info.model_model"),
                                __("eebus.content.peer_info.model_brand"),
                                __("eebus.content.peer_info.state")]}
                            rows={
                                state.state.peers
                                    .filter(peer => peer.trusted || peer.state == NodeState.AwaitingApproval)
                                    .map((peer) => {
                                        return {
                                            columnValues: [
                                                peerDisplayName(peer),
                                                peer.model_brand,
                                                nodeStateToString(peer.state)],
                                            fieldValues: [
                                                peerDisplayName(peer),
                                                peer.model_brand,
                                                peer.state
                                            ],
                                            editTitle: __("eebus.content.peer_info.edit_peer_title"),
                                            onEditShow: async () => this.setState({
                                                add: {
                                                    ski: peer.ski,
                                                    trusted: peer.trusted,
                                                    ip: peer.ip,
                                                    port: peer.port,
                                                    dns_name: peer.dns_name,
                                                    wss_path: peer.wss_path,
                                                    persistent: peer.persistent
                                                }
                                            }),
                                            onEditGetChildren: () => [
                                                <>
                                                    <FormRow label={__("eebus.content.ski")}>
                                                        <InputText
                                                            value={state.add.ski}
                                                            onValue={(v) => this.setState({
                                                                add: {
                                                                    ...state.add,
                                                                    ski: v
                                                                }
                                                            })}
                                                            disabled
                                                        />
                                                    </FormRow>
                                                    <FormRow
                                                        label={"IP " + __("eebus.content.peer_info.device_ip")}>
                                                        <InputText
                                                            value={state.add.ip}
                                                            onValue={(v) => this.setState({
                                                                add: {
                                                                    ...state.add,
                                                                    ip: v
                                                                }
                                                            })}
                                                            disabled
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.device_port")}>
                                                        <InputText
                                                            value={state.add.port}
                                                            onValue={(v) => this.setState({
                                                                add: {
                                                                    ...state.add,
                                                                    port: parseInt(v)
                                                                }
                                                            })}
                                                            disabled
                                                        />
                                                    </FormRow>
                                                    <FormRow
                                                        label={__("eebus.content.peer_info.dns_name")}>
                                                        <InputText
                                                            value={state.add.dns_name}
                                                            onValue={(v) => this.setState({
                                                                add: {
                                                                    ...state.add,
                                                                    dns_name: v
                                                                }
                                                            })}
                                                            disabled
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.wss_path")}>
                                                        <InputText
                                                            value={state.add.wss_path}
                                                            onValue={(v) => this.setState({
                                                                add: {
                                                                    ...state.add,
                                                                    wss_path: v
                                                                }
                                                            })}
                                                            disabled
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.autoregister")}>
                                                        <InputText
                                                            value={peer.autoregister ? __("eebus.content.peer_info.trusted_yes") : __("eebus.content.peer_info.trusted_no")}
                                                            disabled
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.model_type")}>
                                                        <InputText
                                                            value={peer.model_type}
                                                            disabled
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.model_model")}>
                                                        <InputText
                                                            value={peer.model_model}
                                                            disabled
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.model_brand")}>
                                                        <InputText
                                                            value={peer.model_brand}
                                                            disabled
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.state")}>
                                                        <InputText
                                                            value={nodeStateToString(peer.state)}
                                                            disabled
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.device_trusted")}>
                                                        <InputSelect
                                                            items={[
                                                                ["true", __("eebus.content.peer_info.trusted_yes")],
                                                                ["false", __("eebus.content.peer_info.trusted_no")],
                                                            ]}
                                                            value={state.add.trusted ? "true" : "false"}
                                                            onValue={(v) => {
                                                                const trusted = v === "true";
                                                                this.setState({
                                                                    add: {
                                                                        ...state.add,
                                                                        trusted: trusted,
                                                                        ski: peer.ski
                                                                    }
                                                                });
                                                            }}
                                                        />
                                                    </FormRow>
                                                </>
                                            ],
                                            onEditSubmit: async () => {
                                                let peer = state.add;
                                                await API.call('eebus/add', peer);
                                                this.setState({
                                                    add: {
                                                        ski: "",
                                                        trusted: true,
                                                        ip: "",
                                                        port: 4712,
                                                        dns_name: "",
                                                        wss_path: "/ship/",
                                                        persistent: true
                                                    }
                                                });
                                            },
                                            onRemoveClick: async () => {
                                                await API.call('eebus/remove', {ski: peer.ski});
                                                return true;
                                            }

                                        }
                                    })
                            }
                            addEnabled={state.enable}
                            addTitle={__("eebus.content.add_peer_title")}
                            addMessage={__("eebus.content.add_peer_message")}
                            onAddShow={async () => {
                                this.setState({
                                    add: {
                                        ski: "",
                                        trusted: true,
                                        ip: "",
                                        port: 4712,
                                        dns_name: "",
                                        wss_path: "/ship/",
                                        persistent: true
                                    },
                                    addPeerSkiError: "",
                                    addPeerIpError: ""
                                });
                                // Reset IP validation state
                                this.ipconfig_valid = true;
                                // Start mDNS discovery when modal opens
                                this.scan_services();
                                this.scan_interval_id = window.setInterval(() => this.scan_services(), 3000);
                            }}
                            onAddGetChildren={() => {
                                // Get all discovered peers (with valid SKI)
                                const discoveredPeers = state.state.peers.filter(p =>
                                    p.ski &&
                                    p.ski.length > 0
                                );

                                return [<>
                                    <FormRow label={__("eebus.content.discovered_peers")}>
                                            {state.state.discovery_state === ShipDiscoveryState.Scanning && discoveredPeers.length === 0 ? (
                                                <ListGroup>
                                                    <ListGroupItem>
                                                        {__("eebus.content.searching_peers")}
                                                    </ListGroupItem>
                                                </ListGroup>
                                            ) : discoveredPeers.length === 0 ? (
                                                 <ListGroup>
                                                    <ListGroupItem>
                                                        {__("eebus.content.no_peers_found")}
                                                    </ListGroupItem>
                                                </ListGroup>
                                            ) : (
                                                <DiscoveryResultGroup>
                                                {discoveredPeers.map(peer => {
                                                    const isTrusted = peer.trusted;
                                                    let title = __("eebus.content.unknown");
                                                    if (peer.model_model && peer.model_brand)
                                                        title = `${peer.model_brand} ${peer.model_model}`;
                                                    else if (peer.model_model)
                                                        title = peer.model_model;
                                                    else if (peer.model_brand)
                                                        title = peer.model_brand;

                                                    return (
                                                        <DiscoveryResultItem
                                                            key={peer.ski}
                                                            type="submit"
                                                            title={<h5>{title}</h5>}
                                                            labelAdd={<Plus />}
                                                            error={isTrusted ? __("component.discovery_result.already_added") : undefined}
                                                            onClick={() => {
                                                                this.setState({
                                                                    add: {
                                                                        ski: peer.ski,
                                                                        trusted: true,
                                                                        ip: peer.ip,
                                                                        port: peer.port,
                                                                        dns_name: peer.dns_name,
                                                                        wss_path: peer.wss_path,
                                                                        persistent: true
                                                                    }
                                                                });
                                                            }}
                                                        >
                                                            <div class="pb-1">{`${peer.dns_name || peer.ip || ""}:${peer.port}${peer.wss_path}`}</div>
                                                            <div>SKI: {peer.ski.replace(/(.{1,10})/g, "$1\u00AD")}</div>
                                                        </DiscoveryResultItem>
                                                    );
                                                })}
                                                </DiscoveryResultGroup>
                                            )}
                                    </FormRow>
                                    <hr/>
                                    <p class="text-muted">{__("eebus.content.add_peer_manual_desc")}</p>
                                    <FormRow label={__("eebus.content.ski")} label_muted={__("eebus.content.is_required")} small>
                                        <InputText
                                            value={state.add.ski}
                                            onValue={(v) => this.setState({
                                                add: {...state.add, ski: v},
                                                addPeerSkiError: ""
                                            })}
                                            required
                                            minlength={30}
                                            class={state.addPeerSkiError ? "is-invalid" : ""}
                                            invalidFeedback={state.addPeerSkiError}
                                        />
                                    </FormRow>
                                    <FormRow label={__("eebus.content.peer_info.device_ip")} label_muted={__("eebus.content.is_optional")}>
                                        <InputText
                                            value={state.add.ip}
                                            maxLength={46}
                                            onValue={(v) => this.setState({add: {...state.add, ip: v}})}
                                        />
                                    </FormRow>
                                    <FormRow label={__("eebus.content.peer_info.device_port")}  label_muted={__("eebus.content.is_optional")}>
                                        <InputText
                                            value={state.add.port}
                                            onValue={(v) => {
                                                const port = parseInt(v);
                                                this.setState({add: {...state.add, port: isNaN(port) ? 4712 : port}});
                                            }}
                                        />
                                    </FormRow>
                                    <FormRow label={__("eebus.content.peer_info.wss_path")}  label_muted={__("eebus.content.is_optional")}>
                                        <InputText
                                            value={state.add.wss_path}
                                            onValue={(v) => this.setState({add: {...state.add, wss_path: v}})}
                                        />
                                    </FormRow>
                                </>];
                            }}
                            onAddSubmit={async () => {
                                // Validate SKI
                                if (state.add.ski.trim().length === 0) {
                                    this.setState({addPeerSkiError: __("eebus.script.ski_required")});
                                    return;
                                }

                                // Validate IP using IPConfiguration component's validation (only if IP is provided)
                                if (state.add.ip.trim().length > 0 && !this.ipconfig_valid) {
                                    this.setState({addPeerIpError: __("eebus.script.ip_invalid")});
                                    return;
                                }

                                try {
                                    let peer = state.add;
                                    await API.call('eebus/add', peer);
                                    // Reset the form after successful submission
                                    this.setState({
                                        add: {
                                            ski: "",
                                            trusted: true,
                                            ip: "",
                                            port: 4712,
                                            dns_name: "",
                                            wss_path: "/ship/",
                                            persistent: true
                                        },
                                        addPeerSkiError: "",
                                        addPeerIpError: ""
                                    });
                                } catch (error) {
                                    // Display API error on SKI field
                                    this.setState({addPeerSkiError: __("eebus.script.add_peer_failed")});
                                }

                            }}
                            onAddHide={async () => {
                                // Stop mDNS discovery interval when modal closes
                                if (this.scan_interval_id !== null) {
                                    window.clearInterval(this.scan_interval_id);
                                    this.scan_interval_id = null;
                                }
                            }}
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
    register_status_provider("eebus", {
        name: () => __("eebus.navbar.eebus"),
        href: "#eebus",
        get_status: () => {
            const config = API.get("eebus/config");
            const state = API.get("eebus/state");
            const usecases = API.get("eebus/usecases");

            if (!config.enable) {
                return {status: ModuleStatus.Disabled};
            }


            // Check for specific error conditions and return appropriate message
            if (state.peers.some(p => p.state === NodeState.EEBUSDegraded)) {
                return {
                    status: ModuleStatus.Error,
                    text: () => __("eebus.status.peer_degraded")
                };
            }


            if (usecases.lpc?.usecase_state === LoadcontrolState.Failsafe) {
                return {
                    status: ModuleStatus.Warning,
                    text: () => __("eebus.status.lpc_failsafe")
                };
            }

            if (usecases.evsecc?.evse_failure) {
                return {
                    status: ModuleStatus.Error,
                    text: () => __("eebus.status.evse_failure")
                };
            }

            if (usecases.cevc?.energy_broker_connected &&
                !usecases.cevc.energy_broker_heartbeat_ok) {
                return {
                    status: ModuleStatus.Error,
                    text: () => __("eebus.status.heartbeat_timeout")
                };
            }

            // Count connected and discovered peers
            const connectedCount = state.peers.filter(p => p.state === NodeState.EEBUSActive).length;

            if (connectedCount > 0) {
                return {
                    status: ModuleStatus.Ok,
                    text: () => `${connectedCount} ` + __("eebus.status.n_devices_connected")
                };
            }

            return { status: ModuleStatus.Ok };
        }
    });
}
