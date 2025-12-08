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
import {h, Fragment} from "preact";
import {__} from "../../ts/translation";
import {ConfigComponent} from "../../ts/components/config_component";
import {ConfigForm} from "../../ts/components/config_form";
import {FormRow} from "../../ts/components/form_row";
import {InputSelect} from "../../ts/components/input_select";
import {InputText} from "../../ts/components/input_text";
import {SubPage} from "../../ts/components/sub_page";
import {NavbarItem} from "../../ts/components/navbar_item";
import {Table} from "../../ts/components/table";
import {Button} from "react-bootstrap";
import {Share2} from "react-feather";
import {Switch} from "../../ts/components/switch";
import {ShipDiscoveryState} from "./ship_discovery_state.enum";
import {ShipConnectionState} from "./ship_connection_state.enum";
import {NodeState} from "./node_state.enum";
import {CollapsedSection} from "../../ts/components/collapsed_section";
import {FormSeparator} from "../../ts/components/form_separator";
import {LPCState} from "./lpc_state.enum";
import {usecases} from "./api";
import {useState} from "preact/hooks";

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


    render(props: {}, state: EEBusState & EEBusConfig) {
        if (!util.render_allowed())
            return <SubPage name="eebus"/>;

        let ski = state.state.ski;
        if (ski == "") {
            ski = __("eebus.content.unknown");
        }

        return (
            <SubPage name="eebus">
                <ConfigForm id="eebus_config_form" title="EEBUS" isModified={this.isModified()} isDirty={this.isDirty()}
                            onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("eebus.content.enable_eebus")} help={__("eebus.content.enable_eebus_help")}>
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
                                state.config.peers
                                    .filter(peer => (peer.dns_name && peer.dns_name.length >= 1) || (peer.ip && peer.ip.length >= 1))
                                    .map((peer) => {
                                        return {
                                            columnValues: [
                                                peer.model_model,
                                                peer.model_brand,
                                                <ExpandableAddress dns={peer.dns_name} ip={peer.ip}/>,
                                                peer.state == NodeState.Disconnected ? __("eebus.content.peer_info.state_disconnected") : peer.state == NodeState.Discovered ? __("eebus.content.peer_info.state_discovered") : peer.state == NodeState.Connected ? __("eebus.content.peer_info.state_connected") : __("eebus.content.peer_info.state_eebus_connected")],
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
                                                    wss_path: peer.wss_path
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
                                                                     disabled={true}
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.model_type")}>
                                                        <InputText
                                                            value={peer.model_type}
                                                            disabled={true}
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.model_model")}>
                                                        <InputText
                                                            value={peer.model_model}
                                                            disabled={true}
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.model_brand")}>
                                                        <InputText
                                                            value={peer.model_brand}
                                                            disabled={true}
                                                        />
                                                    </FormRow>
                                                    <FormRow label={__("eebus.content.peer_info.state")}>
                                                        <InputSelect items={[
                                                            ["0", __("eebus.content.peer_info.state_disconnected")],
                                                            ["1", __("eebus.content.peer_info.state_discovered")],
                                                            ["2", __("eebus.content.peer_info.state_connected")],
                                                            ["3", __("eebus.content.peer_info.state_eebus_connected")]
                                                        ]}
                                                                     value={peer.state.toString()}
                                                                     disabled={true}
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
                                                        wss_path: "/ship/"
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
                                        wss_path: "/ship/"
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
                                        wss_path: "/ship/"
                                    }
                                });
                            }}
                        />
                        <Button
                            className="form-control rounded-right"
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
                    <FormRow label={__("eebus.content.ski")} label_muted={__("eebus.content.ski_muted")}
                             help={__("eebus.content.ski_help")}>
                        <InputText value={ski}/>
                    </FormRow>
                </ConfigForm>

                <CollapsedSection heading={__("eebus.content.show_usecase_details")}>
                    <p> This shows the current State of the EEBUS usecases as they are presented to other EEBUS Devices.
                        For understanding the details of the usecases please refer to the EEBUS documentation.
                        Some knowledge about the implemented usecases is recommended.</p>

                    <table class="table table-bordered table-sm">
                        <thead class="thead-light">
                        <tr>
                            <th>Usecase</th>
                            <th>Details</th>
                        </tr>
                        </thead>
                        <tbody>
                        <tr>
                            <td>General EEBUS State</td>
                            <td>
                                <table class="table table-bordered table-sm mb-0">
                                    <tbody>
                                    <tr>
                                        <td>Commands Received</td>
                                        <td>
                                            {state.usecases.commands_received}
                                        </td>
                                    </tr>
                                    <tr>
                                        <td>Responses/Commands sent</td>
                                        <td>
                                            {state.usecases.commands_sent}
                                        </td>
                                    </tr>
                                    </tbody>
                                </table>
                            </td>
                        </tr>
                        <tr>
                            <td>Limitation of Power Consumption</td>
                            <td>
                                <table class="table table-bordered table-sm mb-0">
                                    <tbody>
                                    <tr>
                                        <td>Usecase State</td>
                                        <td>
                                            {{
                                                [LPCState.Startup]: "Startup",
                                                [LPCState.Init]: "Init",
                                                [LPCState.UnlimitedControlled]: "UnlimitedControlled",
                                                [LPCState.Limited]: "Limited",
                                                [LPCState.Failsafe]: "Failsafe",
                                                [LPCState.UnlimitedAutonomous]: "UnlimitedAutonomous"
                                            }[state.usecases.power_consumption_limitation.usecase_state] ?? state.usecases.power_consumption_limitation.usecase_state}
                                        </td>
                                    </tr>
                                    <tr>
                                        <td>Limit Active</td>
                                        <td>{state.usecases.power_consumption_limitation.limit_active ? __("eebus.content.yes") : __("eebus.content.no")}</td>
                                    </tr>
                                    <tr>
                                        <td>Current Limit (W)</td>
                                        <td>{state.usecases.power_consumption_limitation.current_limit} W</td>
                                    </tr>
                                    <tr>
                                        <td>Failsafe Limit Power (W)</td>
                                        <td>{state.usecases.power_consumption_limitation.failsafe_limit_power_w} W</td>
                                    </tr>
                                    <tr>
                                        <td>Failsafe Limit Duration (s)</td>
                                        <td>{state.usecases.power_consumption_limitation.failsafe_limit_duration_s} s</td>
                                    </tr>
                                    <tr>
                                        <td>Constraints Power Maximum (W)</td>
                                        <td>{state.usecases.power_consumption_limitation.constraints_power_maximum} W</td>
                                    </tr>
                                    </tbody>
                                </table>
                            </td>
                        </tr>
                        <tr>
                            <td>EV Commissioning and Configuration</td>
                            <td>
                                <table class="table table-bordered table-sm mb-0">
                                    <tbody>
                                    <tr>
                                        <td>EV Connected</td>
                                        <td>
                                            {state.usecases.ev_commissioning_and_configuration.ev_connected
                                                ? __("eebus.content.yes")
                                                : __("eebus.content.no")}
                                        </td>
                                    </tr>
                                    {state.usecases.ev_commissioning_and_configuration.ev_connected && (
                                        <>
                                            <tr>
                                                <td>Communication Standard</td>
                                                <td>{state.usecases.ev_commissioning_and_configuration.communication_standard}</td>
                                            </tr>
                                            <tr>
                                                <td>Asymmetric Charging supported</td>
                                                <td>
                                                    {state.usecases.ev_commissioning_and_configuration.asymmetric_charging_supported
                                                        ? __("eebus.content.yes")
                                                        : __("eebus.content.no")}
                                                </td>
                                            </tr>
                                            <tr>
                                                <td>Vehicle MAC Address</td>
                                                <td>{state.usecases.ev_commissioning_and_configuration.mac_address}</td>
                                            </tr>
                                            <tr>
                                                <td>Minimum Power Consumption (reported by Vehicle)</td>
                                                <td>{state.usecases.ev_commissioning_and_configuration.minimum_power} W</td>
                                            </tr>
                                            <tr>
                                                <td>Maximum Power Consumption (reported by Vehicle)</td>
                                                <td>{state.usecases.ev_commissioning_and_configuration.maximum_power} W</td>
                                            </tr>
                                            <tr>
                                                <td>Standby Power Consumption (reported by Vehicle)</td>
                                                <td>{state.usecases.ev_commissioning_and_configuration.standby_power} W</td>
                                            </tr>
                                            <tr>
                                                <td>Standby Mode Active</td>
                                                <td>  {state.usecases.ev_commissioning_and_configuration.standby_mode
                                                    ? __("eebus.content.yes")
                                                    : __("eebus.content.no")}</td>
                                            </tr>
                                        </>
                                    )}
                                    </tbody>
                                </table>
                            </td>
                        </tr>
                        <tr>
                            <td>EV Charging Electricity Measurement</td>
                            <td>
                                <table class="table table-bordered table-sm mb-0">
                                    <tbody>
                                    <tr>
                                        <td>EV Connected</td>
                                        <td>
                                            {state.usecases.ev_commissioning_and_configuration.ev_connected
                                                ? __("eebus.content.yes")
                                                : __("eebus.content.no")}
                                        </td>
                                    </tr>
                                    {state.usecases.ev_commissioning_and_configuration.ev_connected && (
                                        <>
                                            <tr>
                                                <td>Amps Phase 1</td>
                                                <td>{state.usecases.ev_charging_electricity_measurement.amps_phase_1} A</td>
                                            </tr>
                                            <tr>
                                                <td>Amps Phase 2</td>
                                                <td>{state.usecases.ev_charging_electricity_measurement.amps_phase_2} A</td>
                                            </tr>
                                            <tr>
                                                <td>Amps Phase 3</td>
                                                <td>{state.usecases.ev_charging_electricity_measurement.amps_phase_3} A</td>
                                            </tr>
                                            <tr>
                                                <td>Power Phase 1</td>
                                                <td>{state.usecases.ev_charging_electricity_measurement.power_phase_1} W</td>
                                            </tr>
                                            <tr>
                                                <td>Power Phase 2</td>
                                                <td>{state.usecases.ev_charging_electricity_measurement.power_phase_2} W</td>
                                            </tr>
                                            <tr>
                                                <td>Power Phase 3</td>
                                                <td>{state.usecases.ev_charging_electricity_measurement.power_phase_3} W</td>
                                            </tr>
                                            <tr>
                                                <td>Charged Wh</td>
                                                <td>{state.usecases.ev_charging_electricity_measurement.charged_wh} Wh</td>
                                            </tr>
                                            <tr>
                                                <td>Method of obtaining charged Wh</td>
                                                <td>{state.usecases.ev_charging_electricity_measurement.charged_valuesource_measured ? "Measured" : "Calculated"}</td>
                                            </tr>
                                        </>
                                    )}
                                    </tbody>
                                </table>
                            </td>
                        </tr>
                        <tr>
                            <td>EVSE Commissioning and Configuration</td>
                            <td>
                                <table class="table table-bordered table-sm mb-0">
                                    <tbody>
                                    <tr>
                                        <td>EVSE in failure state</td>
                                        <td>
                                            {state.usecases.evse_commissioning_and_configuration.evse_failure
                                                ? __("eebus.content.yes")
                                                : __("eebus.content.no")}
                                        </td>
                                    </tr>
                                    {state.usecases.evse_commissioning_and_configuration.evse_failure && (
                                        <>
                                            <tr>
                                                <td>Failure Message</td>
                                                <td>{state.usecases.evse_commissioning_and_configuration.evse_failure_description}</td>
                                            </tr>
                                        </>
                                    )}
                                    </tbody>
                                </table>
                            </td>
                        </tr>
                        <tr>
                            <td>Charging Summary</td>
                            <td>
                                <table class="table table-bordered table-sm mb-0">
                                    <tbody>
                                    <tr>
                                        <td colSpan={2}>
                                            <CollapsedSection
                                                heading={"Number of Charge Processes: " + state.usecases.charging_summary.length}>
                                                <table class="table table-bordered table-sm mb-0">
                                                    <thead>
                                                    <tr>
                                                        <th>ID</th>
                                                        <th>Charged Kwh</th>
                                                        <th>Cost</th>
                                                        <th>Percentage Self Produced Energy</th>
                                                    </tr>
                                                    </thead>
                                                    <tbody>
                                                    {state.usecases.charging_summary.map((item, idx) => (
                                                        <tr key={idx}>
                                                            <td>{item.id}</td>
                                                            <td>{item.charged_kwh}</td>
                                                            <td>{item.cost}</td>
                                                            <td>{item.percent_self_produced_energy}</td>
                                                        </tr>
                                                    ))}
                                                    </tbody>
                                                </table>
                                            </CollapsedSection>
                                        </td>
                                    </tr>
                                    </tbody>
                                </table>
                            </td>
                        </tr>

                        </tbody>
                    </table>
                </CollapsedSection>
            </SubPage>
        )
            ;
    }
}

export function init() {
}
