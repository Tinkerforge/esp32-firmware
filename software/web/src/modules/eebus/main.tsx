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
import {Button} from "react-bootstrap";
import {Share2} from "react-feather";
import {Switch} from "../../ts/components/switch";
import {ShipDiscoveryState} from "./ship_discovery_state.enum";
import {NodeState} from "./node_state.enum";
import {LoadcontrolState} from "./loadcontrol_state.enum";
import {useState} from "preact/hooks";
import {Usecases} from "./usecases.enum";
import {OutputFloat} from "../../ts/components/output_float";
import {register_status_provider, ModuleStatus} from "../../ts/status_registry";

const loadcontrolStateMap: {[key: number]: string} = {
    [LoadcontrolState.Startup]: "Startup",
    [LoadcontrolState.Init]: "Init",
    [LoadcontrolState.UnlimitedControlled]: "UnlimitedControlled",
    [LoadcontrolState.Limited]: "Limited",
    [LoadcontrolState.Failsafe]: "Failsafe",
    [LoadcontrolState.UnlimitedAutonomous]: "UnlimitedAutonomous"
};

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

function PhaseRow(props: {label: string, label_muted: string, values: [number, number, number], unit: string, digits?: 0 | 1 | 2 | 3, scale?: number}) {
    const digits = props.digits ?? 0;
    const scale = props.scale ?? 0;
    return <FormRow label={props.label} label_muted={props.label_muted} small>
        <div class="row gx-2 gy-1">
            <div class="col-sm-4">
                <OutputFloat value={props.values[0]} digits={digits} scale={scale} unit={props.unit} small />
            </div>
            <div class="col-sm-4">
                <OutputFloat value={props.values[1]} digits={digits} scale={scale} unit={props.unit} small />
            </div>
            <div class="col-sm-4">
                <OutputFloat value={props.values[2]} digits={digits} scale={scale} unit={props.unit} small />
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
    const usecaseDescMap: {[key: number]: {name: string, desc: string}} = {
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

    // Get the list of supported use cases, excluding NMC (internal) and HEARTBEAT (internal)
    const supportedUsecases = (usecases?.usecases_supported || []).filter(
        uc => uc !== Usecases.NMC && uc !== Usecases.HEARTBEAT
    );

    // Build the list items only for enabled use cases
    const usecaseItems = supportedUsecases.map(uc => {
        const info = usecaseDescMap[uc];
        if (info) {
            return (
                <li key={uc}>
                    <b>{info.name}</b>: {info.desc}
                </li>
            );
        }
        return null;
    }).filter(item => item !== null);

    return (
        <>
            {__("eebus.content.enable_eebus_help_intro")}
            {usecaseItems.length > 0 && (
                <>
                    <p>{__("eebus.content.enable_eebus_help_usecases_intro")}</p>
                    <ul>{usecaseItems}</ul>
                </>
            )}
        </>
    );
}

export function EEBusNavbar() {
    return <NavbarItem name="eebus" module="eebus" title="EEBUS" symbol={<Share2/>}/>;
}

type EEBusConfig = API.getType["eebus/config"];
type EEBusScan = API.getType["eebus/scan"];
type EEBusAdd = API.getType["eebus/add"];
type EEBusRemove = API.getType["eebus/remove"];
type EEBusStateType = API.getType["eebus/state"];
type EEBusUsecases = API.getType["eebus/usecases"];

interface EEBusState {
    add: EEBusAdd;
    state: EEBusStateType;
    config: EEBusConfig;
    usecases: EEBusUsecases;
}

export class EEBus extends ConfigComponent<'eebus/config', {}, EEBusState> {
    constructor() {
        super('eebus/config', () => __("eebus.script.save_failed"));
        util.addApiEventListener('eebus/state', () => {
            this.setState({state: API.get('eebus/state')});
        });
        util.addApiEventListener('eebus/config', () => {
            this.setState({config: API.get('eebus/config')});
        });
        util.addApiEventListener('eebus/config', () => {
            this.setState({usecases: API.get('eebus/usecases')});
        });
    }

    override async sendSave(topic: "eebus/config", cfg: EEBusConfig) {
        // TODO: why is this unchecked? Currently cargo-culted from OCPP and Modbus TCP. Maybe add those API definitions to ocpp/api.ts (or modbus_tcp or eebus) with //APIPath?
        await API.save_unchecked('evse/eebus_enabled', {enabled: cfg.enable}, () => __("eebus.script.save_failed"));
        await super.sendSave(topic, cfg);
    }

    override async sendReset(topic: "eebus/config") {
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
                {state.enable &&
                    <SubPage.Status>
                    <FormRow label={__("eebus.content.ski")} label_muted={__("eebus.content.ski_muted")} help={__("eebus.content.ski_help")}>
                        <InputText value={ski}/>
                    </FormRow>

                    <CollapsedSection heading={__("eebus.content.usecase_details")}>
                    <Table
                        columnNames={[""]}
                        rows={(() => {
                            let rows: TableRow[] = [];

                            // Limitation of Power Consumption
                            if (state.usecases.usecases_supported && state.usecases.usecases_supported.lastIndexOf(Usecases.LPC) > -1) {
                                const lpc = state.usecases.power_consumption_limitation;
                                rows.push({
                                    hideRemoveButton: true,
                                    columnValues: ["LPC (Limitation of Power Consumption)"],
                                    extraValue: <>
                                        <FormRow label="Usecase State" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <InputText class="form-control-sm" value={loadcontrolStateMap[lpc.usecase_state] ?? lpc.usecase_state} />
                                            </div></div>
                                        </FormRow>
                                        <FormRow label="Limit Active" small>
                                            <div class="row gx-2 gy-1">
                                                <div class="col-sm-4">
                                                    <InputText class="form-control-sm"
                                                               value={lpc.limit_active ? __("eebus.content.yes") : __("eebus.content.no")}/>
                                                </div>
                                                {lpc.usecase_state === LoadcontrolState.Limited && lpc.outstanding_duration_s != null ?
                                                    <div class="col-sm-4">
                                                        <OutputFloat value={lpc.outstanding_duration_s} digits={0}
                                                                     scale={0} unit="s" small/>
                                                    </div> : undefined}
                                            </div>
                                        </FormRow>
                                        <FormRow label="Current Limit" small>
                                            <div class="row gx-2 gy-1">
                                                <div class="col-sm-4">
                                                    <OutputFloat value={lpc.current_limit} digits={0} scale={0} unit="W"
                                                                 small/>
                                                </div>
                                            </div>
                                        </FormRow>
                                        <FormRow label="Failsafe Limit Power" small>
                                            <div class="row gx-2 gy-1">
                                                <div class="col-sm-4">
                                                    <OutputFloat value={lpc.failsafe_limit_power_w} digits={0} scale={0}
                                                                 unit="W" small/>
                                                </div>
                                            </div>
                                        </FormRow>
                                        <FormRow label="Failsafe Limit Duration" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                    <OutputFloat value={lpc.failsafe_limit_duration_s} digits={0}
                                                                 scale={0} unit="s" small/>
                                            </div>
                                            {lpc.usecase_state === LoadcontrolState.Limited && lpc.outstanding_duration_s != null ?
                                                <div class="col-sm-4">
                                                        <OutputFloat value={lpc.outstanding_duration_s} digits={0}
                                                                     scale={0} unit="s" small/>
                                                </div> : undefined}
                                            </div>
                                        </FormRow>
                                        <FormRow label="Constraints Power Maximum" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <OutputFloat value={lpc.constraints_power_maximum} digits={0}
                                                                 scale={0} unit="W" small/>
                                            </div></div>
                                        </FormRow>
                                    </>
                                });
                            }

                            // Limitation of Power Production
                            if (state.usecases.usecases_supported && state.usecases.usecases_supported.lastIndexOf(Usecases.LPP) > -1) {
                                const lpp = state.usecases.power_production_limitation;
                                rows.push({
                                    hideRemoveButton: true,
                                    columnValues: ["LPP (Limitation of Power Production)"],
                                    extraValue: <>
                                        <FormRow label="Usecase State" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <InputText class="form-control-sm" value={loadcontrolStateMap[lpp.usecase_state] ?? lpp.usecase_state} />
                                            </div></div>
                                        </FormRow>
                                        <FormRow label="Limit Active" small>
                                            <div class="row gx-2 gy-1">
                                                <div class="col-sm-4">
                                                    <InputText class="form-control-sm"
                                                               value={lpp.limit_active ? __("eebus.content.yes") : __("eebus.content.no")}/>
                                                </div>
                                                {lpp.usecase_state === LoadcontrolState.Limited && lpp.outstanding_duration_s != null ?
                                                    <div class="col-sm-4">
                                                        <OutputFloat value={lpp.outstanding_duration_s} digits={0}
                                                                     scale={0} unit="s" small/>
                                                    </div> : undefined}
                                            </div>
                                        </FormRow>
                                        <FormRow label="Current Limit" small>
                                            <div class="row gx-2 gy-1">
                                                <div class="col-sm-4">
                                                    <OutputFloat value={lpp.current_limit} digits={0} scale={0} unit="W"
                                                                 small/>
                                                </div>
                                            </div>
                                        </FormRow>
                                        <FormRow label="Failsafe Limit Power" small>
                                            <div class="row gx-2 gy-1">
                                                <div class="col-sm-4">
                                                    <OutputFloat value={lpp.failsafe_limit_power_w} digits={0} scale={0}
                                                                 unit="W" small/>
                                                </div>
                                            </div>
                                        </FormRow>
                                        <FormRow label="Failsafe Limit Duration" small>
                                            <div class="row gx-2 gy-1">
                                                <div class="col-sm-4">
                                                    <OutputFloat value={lpp.failsafe_limit_duration_s} digits={0}
                                                                 scale={0} unit="s" small/>
                                                </div>
                                                {lpp.usecase_state === LoadcontrolState.Limited && lpp.outstanding_duration_s != null ?
                                                    <div class="col-sm-4">
                                                        <OutputFloat value={lpp.outstanding_duration_s} digits={0}
                                                                     scale={0} unit="s" small/>
                                                    </div> : undefined}
                                            </div>
                                        </FormRow>
                                        <FormRow label="Constraints Power Maximum" small>
                                            <div class="row gx-2 gy-1">
                                                <div class="col-sm-4">
                                                    <OutputFloat value={lpp.constraints_power_maximum} digits={0}
                                                                 scale={0} unit="W" small/>
                                                </div>
                                            </div>
                                        </FormRow>
                                    </>
                                });
                            }

                            // EV Commissioning and Configuration
                            if (state.usecases.usecases_supported && state.usecases.usecases_supported.lastIndexOf(Usecases.EVCC) > -1) {
                                const evcc = state.usecases.ev_commissioning_and_configuration;
                                rows.push({
                                    hideRemoveButton: true,
                                    columnValues: ["EVCC (EV Commissioning and Configuration)"],
                                    extraValue: <>
                                        <FormRow label="EV Connected" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <InputText class="form-control-sm" value={evcc.ev_connected ? __("eebus.content.yes") : __("eebus.content.no")} />
                                            </div></div>
                                        </FormRow>
                                        {evcc.ev_connected && <>
                                            <FormRow label="Communication Standard" small>
                                                <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                    <InputText class="form-control-sm" value={evcc.communication_standard} />
                                                </div></div>
                                            </FormRow>
                                            <FormRow label="Asymmetric Charging Supported" small>
                                                <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                    <InputText class="form-control-sm" value={evcc.asymmetric_charging_supported ? __("eebus.content.yes") : __("eebus.content.no")} />
                                                </div></div>
                                            </FormRow>
                                            <FormRow label="Vehicle MAC Address" small>
                                                <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                    <InputText class="form-control-sm" value={evcc.mac_address} />
                                                </div></div>
                                            </FormRow>
                                            <FormRow label="Minimum Power (reported by Vehicle)" small>
                                                <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                    <OutputFloat value={evcc.minimum_power} digits={0} scale={0} unit="W" small />
                                                </div></div>
                                            </FormRow>
                                            <FormRow label="Maximum Power (reported by Vehicle)" small>
                                                <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                    <OutputFloat value={evcc.maximum_power} digits={0} scale={0} unit="W" small />
                                                </div></div>
                                            </FormRow>
                                            <FormRow label="Standby Power (reported by Vehicle)" small>
                                                <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                    <OutputFloat value={evcc.standby_power} digits={0} scale={0} unit="W" small />
                                                </div></div>
                                            </FormRow>
                                            <FormRow label="Standby Mode Active" small>
                                                <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                    <InputText class="form-control-sm" value={evcc.standby_mode ? __("eebus.content.yes") : __("eebus.content.no")} />
                                                </div></div>
                                            </FormRow>
                                        </>}
                                    </>
                                });
                            }

                            // EV Charging Electricity Measurement
                            if (state.usecases.usecases_supported && state.usecases.usecases_supported.lastIndexOf(Usecases.EVCEM) > -1) {
                                const evcc = state.usecases.ev_commissioning_and_configuration;
                                const evcem = state.usecases.ev_charging_electricity_measurement;
                                rows.push({
                                    hideRemoveButton: true,
                                    columnValues: ["EVCEM (EV Charging Electricity Measurement)"],
                                    extraValue: <>
                                        <FormRow label="EV Connected" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <InputText class="form-control-sm" value={evcc.ev_connected ? __("eebus.content.yes") : __("eebus.content.no")} />
                                            </div></div>
                                        </FormRow>
                                        {evcc.ev_connected && <>
                                            <PhaseRow label="Amps" label_muted="L1, L2, L3" values={[
                                                evcem.amps_phase_1,
                                                evcem.amps_phase_2,
                                                evcem.amps_phase_3
                                            ]} unit="A" />
                                            <PhaseRow label="Power" label_muted="L1, L2, L3" values={[
                                                evcem.power_phase_1,
                                                evcem.power_phase_2,
                                                evcem.power_phase_3
                                            ]} unit="W" />
                                            <FormRow label="Charged" small>
                                                <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                    <OutputFloat value={evcem.charged_wh} digits={0} scale={0} unit="Wh" small />
                                                </div></div>
                                            </FormRow>
                                            <FormRow label="Method of Obtaining Charged Wh" small>
                                                <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                    <InputText class="form-control-sm" value={evcem.charged_valuesource_measured ? "Measured" : "Calculated"} />
                                                </div></div>
                                            </FormRow>
                                        </>}
                                    </>
                                });
                            }

                            // EVSE Commissioning and Configuration
                            if (state.usecases.usecases_supported && state.usecases.usecases_supported.lastIndexOf(Usecases.EVSECC) > -1) {
                                const evsecc = state.usecases.evse_commissioning_and_configuration;
                                rows.push({
                                    hideRemoveButton: true,
                                    columnValues: ["EVSECC (EVSE Commissioning and Configuration)"],
                                    extraValue: <>
                                        <FormRow label="EVSE in Failure State" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <InputText class="form-control-sm" value={evsecc.evse_failure ? __("eebus.content.yes") : __("eebus.content.no")} />
                                            </div></div>
                                        </FormRow>
                                        {evsecc.evse_failure && <>
                                            <FormRow label="Failure Message" small>
                                                <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                    <InputText class="form-control-sm" value={evsecc.evse_failure_description} />
                                                </div></div>
                                            </FormRow>
                                        </>}
                                    </>
                                });
                            }

                            // EV Charging Summary
                            if (state.usecases.usecases_supported && state.usecases.usecases_supported.lastIndexOf(Usecases.EVCS) > -1) {
                                const summary = state.usecases.charging_summary;
                                rows.push({
                                    hideRemoveButton: true,
                                    columnValues: ["EVCS (EV Charging Summary)"],
                                    extraValue: <>
                                        <FormRow label="Number of Charge Processes" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <OutputFloat value={summary.length} digits={0} scale={0} unit="" small />
                                            </div></div>
                                        </FormRow>
                                        {summary.map((item, idx) =>
                                            <FormRow label={`Process ${item.id}`} key={idx} small>
                                                <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                    <InputText class="form-control-sm" value={`${item.charged_kwh} kWh, Cost: ${item.cost}, Self-produced: ${item.percent_self_produced_energy}%`} />
                                                </div></div>
                                            </FormRow>
                                        )}
                                    </>
                                });
                            }

                            // Monitoring of Power Consumption
                            if (state.usecases.usecases_supported && state.usecases.usecases_supported.lastIndexOf(Usecases.MPC) > -1) {
                                const mpc = state.usecases.monitoring_of_power_consumption;
                                rows.push({
                                    hideRemoveButton: true,
                                    columnValues: ["MPC (Monitoring of Power Consumption)"],
                                    extraValue: <>
                                        <FormRow label="Total Power" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <OutputFloat value={mpc.total_power_w} digits={0} scale={0} unit="W" small />
                                            </div></div>
                                        </FormRow>
                                        <PhaseRow label="Power" label_muted="L1, L2, L3" values={[
                                            mpc.power_phase_1_w,
                                            mpc.power_phase_2_w,
                                            mpc.power_phase_3_w
                                        ]} unit="W" />
                                        <FormRow label="Energy Consumed" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <OutputFloat value={mpc.energy_consumed_wh} digits={0} scale={0} unit="Wh" small />
                                            </div></div>
                                        </FormRow>
                                        <FormRow label="Energy Produced" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <OutputFloat value={mpc.energy_produced_wh} digits={0} scale={0} unit="Wh" small />
                                            </div></div>
                                        </FormRow>
                                        <PhaseRow label="Current" label_muted="L1, L2, L3" values={[
                                            mpc.current_phase_1_ma,
                                            mpc.current_phase_2_ma,
                                            mpc.current_phase_3_ma
                                        ]} unit="mA" />
                                        <PhaseRow label="Voltage (L-N)" label_muted="L1-N, L2-N, L3-N" values={[
                                            mpc.voltage_phase_1_v,
                                            mpc.voltage_phase_2_v,
                                            mpc.voltage_phase_3_v
                                        ]} unit="V" />
                                        <PhaseRow label="Voltage (L-L)" label_muted="L1-L2, L2-L3, L3-L1" values={[
                                            mpc.voltage_phase_1_2_v,
                                            mpc.voltage_phase_2_3_v,
                                            mpc.voltage_phase_3_1_v
                                        ]} unit="V" />
                                        <FormRow label="Grid Frequency" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <OutputFloat value={mpc.frequency_mhz} digits={2} scale={3} unit="Hz" small />
                                            </div></div>
                                        </FormRow>
                                    </>
                                });
                            }

                            // Coordinated EV Charging
                            if (state.usecases.usecases_supported && state.usecases.usecases_supported.lastIndexOf(Usecases.CEVC) > -1) {
                                const cevc = state.usecases.coordinated_ev_charging;
                                rows.push({
                                    hideRemoveButton: true,
                                    columnValues: ["CEVC (Coordinated EV Charging)"],
                                    extraValue: <>
                                        <FormRow label="Energy Broker Connected" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <InputText class="form-control-sm" value={cevc.energy_broker_connected ? __("eebus.content.yes") : __("eebus.content.no")} />
                                            </div></div>
                                        </FormRow>
                                        <FormRow label="Energy Broker Heartbeat OK" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <InputText class="form-control-sm" value={cevc.energy_broker_heartbeat_ok ? __("eebus.content.yes") : __("eebus.content.no")} />
                                            </div></div>
                                        </FormRow>
                                        <FormRow label="Has Charging Plan" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <InputText class="form-control-sm" value={
                                                    (cevc.has_charging_plan ? __("eebus.content.yes") : __("eebus.content.no")) +
                                                    (cevc.has_charging_plan && cevc.charging_plan_start_time > 0
                                                        ? ` (from: ${new Date(cevc.charging_plan_start_time * 1000).toLocaleString()})`
                                                        : "")
                                                } />
                                            </div></div>
                                        </FormRow>
                                        {cevc.has_charging_plan && <>
                                            <FormRow label="Current Target Power" small>
                                                <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                    <OutputFloat value={cevc.target_power_w} digits={0} scale={0} unit="W" small />
                                                </div></div>
                                            </FormRow>
                                        </>}
                                        <FormRow label="Has Incentives" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <InputText class="form-control-sm" value={cevc.has_incentives ? __("eebus.content.yes") : __("eebus.content.no")} />
                                            </div></div>
                                        </FormRow>
                                    </>
                                });
                            }

                            // Monitoring of Grid Connection Point (MGCP)
                            if (state.usecases.usecases_supported && state.usecases.usecases_supported.lastIndexOf(Usecases.MGCP) > -1) {
                                const mgcp = state.usecases.monitoring_of_grid_connection_point;
                                rows.push({
                                    hideRemoveButton: true,
                                    columnValues: ["MGCP (Monitoring of Grid Connection Point)"],
                                    extraValue: <>
                                        <FormRow label="PV Curtailment Factor" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <OutputFloat value={mgcp.pv_curtailment_limit_factor_percent} digits={1} scale={0} unit="%" small />
                                            </div></div>
                                        </FormRow>
                                        <FormRow label="Total Power" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <OutputFloat value={mgcp.total_power_w} digits={0} scale={0} unit="W" small />
                                            </div></div>
                                        </FormRow>
                                        <FormRow label="Energy Feed-In" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <OutputFloat value={mgcp.energy_feed_in_wh} digits={0} scale={0} unit="Wh" small />
                                            </div></div>
                                        </FormRow>
                                        <FormRow label="Energy Consumed" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <OutputFloat value={mgcp.energy_consumed_wh} digits={0} scale={0} unit="Wh" small />
                                            </div></div>
                                        </FormRow>
                                        <PhaseRow label="Current" label_muted="L1, L2, L3" values={[
                                            mgcp.current_phase_1_ma,
                                            mgcp.current_phase_2_ma,
                                            mgcp.current_phase_3_ma
                                        ]} unit="mA" />
                                        <PhaseRow label="Voltage" label_muted="L1, L2, L3" values={[
                                            mgcp.voltage_phase_1_v,
                                            mgcp.voltage_phase_2_v,
                                            mgcp.voltage_phase_3_v
                                        ]} unit="V" />
                                        <FormRow label="Grid Frequency" small>
                                            <div class="row gx-2 gy-1"><div class="col-sm-4">
                                                <OutputFloat value={mgcp.frequency_mhz} digits={2} scale={3} unit="Hz" small />
                                            </div></div>
                                        </FormRow>
                                    </>
                                });
                            }

                            return rows;
                        })()}
                    />
                    </CollapsedSection>
                    </SubPage.Status>
                }

                <SubPage.Config id="eebus_config_form" isModified={this.isModified()} isDirty={this.isDirty()}
                                onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("eebus.content.enable_eebus")} help={buildEEBusHelpText(state.usecases)}>
                        <Switch desc={__("eebus.content.enable_eebus_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}
                        />
                    </FormRow>
                    <FormRow label={__("eebus.content.peer_info.peers")}>
                        <Table
                            columnNames={[
                                __("eebus.content.peer_info.model_model"),
                                __("eebus.content.peer_info.model_brand"),
                                __("eebus.content.peer_info.dns_name"),
                                __("eebus.content.peer_info.state")]}
                            rows={
                                state.state.peers
                                    .filter(peer => (peer.dns_name && peer.dns_name.length >= 1) || (peer.ip && peer.ip.length >= 1))
                                    .map((peer) => {
                                        return {
                                            columnValues: [
                                                peer.model_model,
                                                peer.model_brand,
                                                <ExpandableAddress dns={peer.dns_name} ip={peer.ip}/>,
                                                peer.state == NodeState.Disconnected ? __("eebus.content.peer_info.state_disconnected") : peer.state == NodeState.Discovered ? __("eebus.content.peer_info.state_discovered") : peer.state == NodeState.Connected ? __("eebus.content.peer_info.state_connected") : peer.state == NodeState.LoadedFromConfig ? __("eebus.content.peer_info.state_loaded_from_config") : __("eebus.content.peer_info.state_eebus_connected")],
                                            fieldValues: [
                                                peer.model_model,
                                                peer.model_brand,
                                                peer.dns_name,
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
                                                            required
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.device_trusted")}>
                                                        <InputSelect items={[
                                                            ["0", __("eebus.content.peer_info.trusted_no")],
                                                            ["1", __("eebus.content.peer_info.trusted_yes")]
                                                        ]}
                                                                     value={state.add.trusted ? "1" : "0"}
                                                                     onValue={(v) => this.setState({
                                                                         add: {
                                                                             ...state.add,
                                                                             trusted: v == "1"
                                                                         }
                                                                     })}/>
                                                    </FormRow>
                                                    <FormRow
                                                        label={"IP " + __("eebus.content.peer_info.device_ip") + "*"}>
                                                        <InputText
                                                            value={state.add.ip}
                                                            onValue={(v) => this.setState({
                                                                add: {
                                                                    ...state.add,
                                                                    ip: v
                                                                }
                                                            })}
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.device_port") + "*"}>
                                                        <InputText
                                                            value={state.add.port}
                                                            onValue={(v) => this.setState({
                                                                add: {
                                                                    ...state.add,
                                                                    port: parseInt(v)
                                                                }
                                                            })}
                                                        />
                                                    </FormRow>
                                                    <FormRow
                                                        label={__("eebus.content.peer_info.dns_name") + "*"}>
                                                        <InputText
                                                            value={state.add.dns_name}
                                                            onValue={(v) => this.setState({
                                                                add: {
                                                                    ...state.add,
                                                                    dns_name: v
                                                                }
                                                            })}
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.wss_path") + "*"}>
                                                        <InputText
                                                            value={state.add.wss_path}
                                                            onValue={(v) => this.setState({
                                                                add: {
                                                                    ...state.add,
                                                                    wss_path: v
                                                                }
                                                            })}
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.autoregister")}>
                                                        <InputSelect items={[
                                                            ["0", __("eebus.content.peer_info.trusted_no")],
                                                            ["1", __("eebus.content.peer_info.trusted_yes")]
                                                        ]}
                                                                     value={peer.autoregister ? "1" : "0"}
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
                                                        <InputSelect items={[
                                                            ["0", __("eebus.content.peer_info.state_disconnected")],
                                                            ["1", __("eebus.content.peer_info.state_discovered")],
                                                            ["2", __("eebus.content.peer_info.state_connected")],
                                                            ["3", __("eebus.content.peer_info.state_eebus_connected")],
                                                            ["5", __("eebus.content.peer_info.state_loaded_from_config")]
                                                        ]}
                                                                     value={peer.state.toString()}
                                                                     disabled
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.persistent")}>
                                                        <InputSelect items={[
                                                            ["0", __("eebus.content.peer_info.trusted_no")],
                                                            ["1", __("eebus.content.peer_info.trusted_yes")]
                                                        ]}
                                                                     value={state.add.persistent ? "1" : "0"}
                                                                     onValue={(v) => this.setState({
                                                                         add: {
                                                                             ...state.add,
                                                                             persistent: v == "1"
                                                                         }
                                                                     })}
                                                        />
                                                    </FormRow>
                                                    *{__("eebus.content.peer_info.overwrite_notice")}
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
                                                        port: 4815,
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
                            addTitle={__("eebus.content.add_peers")}
                            addMessage={__("eebus.content.add_peers")}
                            onAddShow={async () => {
                                this.setState({
                                    add: {
                                        ski: "",
                                        trusted: true,
                                        ip: "",
                                        port: 4815,
                                        dns_name: "",
                                        wss_path: "/ship/",
                                        persistent: true
                                    }
                                })
                            }}
                            onAddGetChildren={() => [<>
                                <FormRow label={__("eebus.content.ski")}>
                                    <InputText
                                        value={state.add.ski}
                                        onValue={(v) => this.setState({add: {...state.add, ski: v}})}
                                        required
                                    />
                                </FormRow>
                                <FormRow label={__("eebus.content.peer_info.device_trusted")}>
                                    <InputSelect items={[
                                        ["0", __("eebus.content.peer_info.trusted_no")],
                                        ["1", __("eebus.content.peer_info.trusted_yes")]
                                    ]}
                                                 value={state.add.trusted ? "1" : "0"}
                                                 required
                                    />
                                </FormRow>
                                <FormRow label={__("eebus.content.peer_info.device_ip")}>
                                    <InputText
                                        value={state.add.ip}
                                        maxlength={150}
                                        onValue={(v) => this.setState({add: {...state.add, ip: v}})}

                                    />
                                </FormRow>
                                <FormRow label={__("eebus.content.peer_info.device_port")}>
                                    <InputText
                                        value={state.add.port}
                                        onValue={(v) => this.setState({add: {...state.add, port: parseInt(v)}})}

                                    />
                                </FormRow>
                                <FormRow label={__("eebus.content.peer_info.dns_name")}>
                                    <InputText
                                        value={state.add.dns_name}
                                        onValue={(v) => this.setState({add: {...state.add, dns_name: v}})}

                                    />
                                </FormRow>
                                <FormRow label={__("eebus.content.peer_info.wss_path")}>
                                    <InputText
                                        value={state.add.wss_path}
                                        onValue={(v) => this.setState({add: {...state.add, wss_path: v}})}

                                    />
                                </FormRow>
                            </>]}
                            onAddSubmit={async () => {
                                let peer = state.add;
                                await API.call('eebus/add', peer);
                                this.setState({
                                    add: {
                                        ski: "",
                                        trusted: true,
                                        ip: "",
                                        port: 4815,
                                        dns_name: "",
                                        wss_path: "/ship/",
                                        persistent: true
                                    }
                                });
                            }}
                        />
                    </FormRow>
                    <FormRow>
                        <Button
                            className="form-control"
                            variant="primary"
                            onClick={async () => {
                                await API.call('eebus/scan', {});
                                state.state.discovery_state = ShipDiscoveryState.Scanning;

                            }}
                            disabled={!state.enable || (state.state.discovery_state === ShipDiscoveryState.Scanning)}>
                            {state.state.discovery_state === ShipDiscoveryState.Ready
                                ? __("eebus.content.search_peers")
                                : state.state.discovery_state === ShipDiscoveryState.Scanning
                                    ? __("eebus.content.searching_peers")
                                    : state.state.discovery_state === ShipDiscoveryState.ScanDone
                                        ? __("eebus.content.search_completed")
                                        : __("eebus.content.search_failed")}
                        </Button>
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

            if (!config?.enable) {
                return {status: ModuleStatus.Disabled};
            }

            // Count connected and discovered peers
            const connectedCount = state?.peers?.filter(p =>
                p.state >= NodeState.Connected && p.state <= NodeState.EEBUSDegraded
            ).length ?? 0;
            const discoveredCount = state?.peers?.length ?? 0;

            // Check for specific error conditions and return appropriate message
            if (state?.peers?.some(p => p.state === NodeState.EEBUSDegraded)) {
                return {
                    status: ModuleStatus.Error,
                    text: () => __("eebus.status.peer_degraded")
                };
            }


            if (usecases?.usecases_supported?.indexOf(Usecases.LPC) > -1 &&
                usecases?.power_consumption_limitation?.usecase_state === LoadcontrolState.Failsafe) {
                return {
                    status: ModuleStatus.Error,
                    text: () => __("eebus.status.lpc_failsafe")
                };
            }

            if (usecases?.evse_commissioning_and_configuration?.evse_failure) {
                return {
                    status: ModuleStatus.Error,
                    text: () => __("eebus.status.evse_failure")
                };
            }

            if (usecases?.coordinated_ev_charging?.energy_broker_connected &&
                !usecases?.coordinated_ev_charging?.energy_broker_heartbeat_ok) {
                return {
                    status: ModuleStatus.Error,
                    text: () => __("eebus.status.heartbeat_timeout")
                };
            }


            if (state?.peers?.some(p => p.state === NodeState.EEBUSActive)) {
                return {
                    status: ModuleStatus.Ok,
                    text: () => `${connectedCount}/${discoveredCount} ` + __("eebus.content.peer_info.state_eebus_connected")
                };
            }

            return {
                status: ModuleStatus.Ok,
                text: () => `${connectedCount}/${discoveredCount}`
            };
        }
    });
}
