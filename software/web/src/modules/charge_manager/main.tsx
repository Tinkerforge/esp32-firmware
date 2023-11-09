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

import $ from "../../ts/jq";

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, render, Fragment, Component } from "preact";
import { translate_unchecked, __ } from "../../ts/translation";

import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { Button, Collapse, ListGroup, ListGroupItem } from "react-bootstrap";
import { InputSelect } from "../../ts/components/input_select";
import { InputFloat } from "../../ts/components/input_float";
import { Switch } from "../../ts/components/switch";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { InputNumber } from "../../ts/components/input_number";
import { SubPage } from "../../ts/components/sub_page";
import { Table } from "../../ts/components/table";

type ChargeManagerConfig = API.getType["charge_manager/config"];
type ChargerConfig = ChargeManagerConfig["chargers"][0];
type ScanCharger = Exclude<API.getType['charge_manager/scan_result'], string>[0];

interface ChargeManagerState {
    addCharger: ChargerConfig
    editCharger: ChargerConfig
    managementEnabled: boolean
    showExpert: boolean
    scanResult: Readonly<ScanCharger[]>
}

export class ChargeManager extends ConfigComponent<'charge_manager/config', {}, ChargeManagerState> {
    intervalID: number = null;

    constructor() {
        super('charge_manager/config',
              __("charge_manager.script.save_failed"),
              __("charge_manager.script.reboot_content_changed"), {
                  addCharger: {host: "", name: ""},
                  editCharger: {host: "", name: ""},
                  managementEnabled: false,
                  showExpert: false,
                  scanResult: []
              });

        // Does not check if the event exists, in case the evse module is not compiled in.
        util.addApiEventListener_unchecked('evse/management_enabled', () => {
            let evse_enabled = API.get_unchecked('evse/management_enabled');
            if (evse_enabled != null) {
                this.setState({managementEnabled: evse_enabled.enabled});
            }
        });

        util.addApiEventListener('charge_manager/scan_result', () => {
            this.addScanResults(API.get('charge_manager/scan_result') as ScanCharger[]);
        });
    }

    addScanResults(result: ScanCharger[]) {
        // Copy to remove signals.
        let newResult: ScanCharger[] = result.filter(c => c).map(c => ({
            display_name: c.display_name,
            error: c.error,
            hostname: c.hostname,
            ip: c.ip,
        }));

        for (let oldC of this.state.scanResult) {
            let i = newResult.findIndex(c => c.hostname == oldC.hostname);
            if (i == -1)
                newResult.push(oldC);
            else if (newResult[i].ip == "[no_address]")
                newResult[i].ip = oldC.ip;
        }

        newResult.sort((a, b) => {
            if (a.error == 0 && b.error != 0)
                return -1;
            if (a.error != 0 && b.error == 0)
                return 1;
            return a.display_name.localeCompare(b.display_name);
        });

        this.setState({scanResult: newResult});
    }

    setCharger (i: number, val: Partial<ChargerConfig>){
        let chargers = this.state.chargers;
        chargers[i] = {...chargers[i], ...val};
        this.setState({chargers: chargers});
    }

    override async isSaveAllowed(cfg: ChargeManagerConfig): Promise<boolean> {
        for (let i = 0; i < cfg.chargers.length; i++)
            for (let a = i + 1; a < cfg.chargers.length; a++)
                if (cfg.chargers[a].host == cfg.chargers[i].host)
                    return false;
        return true;
    }

    override async sendSave(t: "charge_manager/config", cfg: ChargeManagerConfig) {
        const modal = util.async_modal_ref.current;
        let illegal_chargers = "";
        for (let i = 0; i < cfg.chargers.length; i++)
        {
            if (this.isMultiOrBroadcastIp(cfg.chargers[i].host))
                illegal_chargers += cfg.chargers[i].name + ": " + cfg.chargers[i].host + "<br>";
        }

        if (illegal_chargers != "" && !await modal.show({
            title: __("charge_manager.content.multi_broadcast_modal_title"),
            body: __("charge_manager.content.multi_broadcast_modal_body") + "<br><br>" + illegal_chargers + "<br>" + __("charge_manager.content.multi_broadcast_modal_body_end"),
            no_text: __("charge_manager.content.multi_broadcast_modal_cancel"),
            yes_text: __("charge_manager.content.multi_broadcast_modal_save"),
            no_variant: "secondary",
            yes_variant: "danger"
        }))
            return;

        await API.save_unchecked('evse/management_enabled', {"enabled": this.state.managementEnabled}, translate_unchecked("charge_manager.script.save_failed"));
        await super.sendSave(t, cfg);
    }

    override async sendReset(t: "charge_manager/config"){
        await API.save_unchecked('evse/management_enabled', {"enabled": false}, translate_unchecked("charge_manager.script.save_failed"));
        await super.sendReset(t);
    }

    override getIsModified(t: "charge_manager/config"): boolean {
        let evse_enabled = API.get_unchecked("evse/management_enabled");
        if (evse_enabled != null && evse_enabled.enabled)
            return true;
        return super.getIsModified(t);
    }

    insertLocalHost() {
        if (this.state.chargers.some(v => v.host == "127.0.0.1"))
            return;

        let name = API.get("info/display_name");
        let c = this.state.chargers;
        c.unshift({
            host: "127.0.0.1",
            name: name.display_name,
        });
        this.setState({chargers: c})
    }

    async scan_services()
    {
        try {
            await API.call('charge_manager/scan', {}, __("charge_manager.script.scan_failed"))
        } catch {
            return;
        }
    }

    intToIP(int: number) {
        let part1 = int & 255;
        let part2 = ((int >> 8) & 255);
        let part3 = ((int >> 16) & 255);
        let part4 = ((int >> 24) & 255);

        return part4 + "." + part3 + "." + part2 + "." + part1;
    }

    isMultiOrBroadcastIp(v: string): boolean
    {
        const ip = util.parseIP(v);
        if (isNaN(ip))
            return false;

        const wifi_subnet = util.parseIP(API.get("wifi/sta_config").subnet);
        const wifi_ip = util.parseIP(API.get("wifi/sta_config").ip);
        const wifi_network = wifi_subnet & wifi_ip;
        const wifi_broadcast = (~wifi_subnet) | wifi_network;

        if (API.get("wifi/sta_config").subnet != "255.255.255.254" && (v == this.intToIP(wifi_broadcast) || v == this.intToIP(wifi_network)))
            return true;

        if (API.hasFeature("ethernet")) {
            const eth_subnet = util.parseIP(API.get_unchecked("ethernet/config").subnet);
            const eth_ip = util.parseIP(API.get_unchecked("ethernet/config").ip);
            const eth_network = eth_ip & eth_subnet;
            const eth_broadcast = (~eth_subnet) | eth_network;
            if (API.get_unchecked("ethernet/config")?.subnet != "255.255.255.254" && (v == this.intToIP(eth_broadcast) || v == this.intToIP(eth_network)))
                return true;
        }

        const start_multicast = util.parseIP("224.0.0.0");
        const end_multicast = util.parseIP("239.255.255.255");
        if (ip >= start_multicast && ip <= end_multicast)
            return true;

        const ap_ip = util.parseIP(API.get("wifi/ap_config").ip);
        const ap_subnet = util.parseIP(API.get("wifi/ap_config").subnet);
        const ap_network = ap_ip & ap_subnet;
        const ap_broadcast =  (~ap_subnet) | ap_network;
        if (API.get("wifi/ap_config").subnet != "255.255.255.254" && (v == this.intToIP(ap_network) || v == this.intToIP(ap_broadcast)))
            return true;

    }

    render(props: {}, state: ChargeManagerConfig & ChargeManagerState) {
        if (!util.render_allowed())
            return <></>

        const MAX_CONTROLLED_CHARGERS = API.hasModule("esp32_ethernet_brick") ? 32 : 10;

        let energyManagerMode = API.hasModule("energy_manager") && !(API.hasModule("evse_v2") || API.hasModule("evse"));
        let warpUltimateMode  = API.hasModule("energy_manager") &&  (API.hasModule("evse_v2") || API.hasModule("evse"));

        let charge_manager_mode = <FormRow label={__("charge_manager.content.enable_charge_manager")} label_muted={__("charge_manager.content.enable_charge_manager_muted")}>
             <InputSelect
                    items={[
                        ["0",__("charge_manager.content.mode_disabled")],
                        ["1",__("charge_manager.content.mode_managed")],
                        ["2",__("charge_manager.content.mode_manager")],
                    ]}
                    value={state.enable_charge_manager ? "2" : state.managementEnabled ? "1" : "0"}
                    onValue={(v) => {
                        if (v == "2")
                            this.insertLocalHost();
                        this.setState({enable_charge_manager: v == "2", managementEnabled: v != "0"})
                    }}
                />
                <div class="pt-3 pb-4">
                    {translate_unchecked(`charge_manager.script.mode_explainer_${state.enable_charge_manager ? "2" : state.managementEnabled ? "1" : "0"}`)}
                </div>
            </FormRow>;


        let verbose = <FormRow label={__("charge_manager.content.verbose")}>
                <Switch desc={__("charge_manager.content.verbose_desc")}
                        checked={state.verbose}
                        onClick={this.toggle("verbose")}/>
            </FormRow>;

        let watchdog = <FormRow label={__("charge_manager.content.enable_watchdog")} label_muted={__("charge_manager.content.enable_watchdog_muted")}>
                <Switch desc={__("charge_manager.content.enable_watchdog_desc")}
                        checked={state.enable_watchdog}
                        onClick={this.toggle("enable_watchdog")}/>
            </FormRow>;

        let default_available_current = <FormRow label={__("charge_manager.content.default_available_current")} label_muted={__("charge_manager.content.default_available_current_muted")}>
                <InputFloat
                    unit="A"
                    value={state.default_available_current}
                    onValue={this.set("default_available_current")}
                    digits={3}
                    min={0}
                    max={state.maximum_available_current}
                    />
                <div class="invalid-feedback">{__("charge_manager.content.default_available_current_invalid")}</div>
            </FormRow>;

        let maximum_available_current = <FormRow label={__("charge_manager.content.maximum_available_current")} label_muted={__("charge_manager.content.maximum_available_current_muted")}>
                <InputFloat
                    unit="A"
                    value={state.maximum_available_current}
                    onValue={(v) => this.setState({
                        maximum_available_current: v,
                        default_available_current: Math.min(v, state.default_available_current)
                    })}
                    digits={3}
                    min={state.minimum_current}
                    max={1000000}
                    />
            </FormRow>;

        let requested_current_margin = <FormRow label={__("charge_manager.content.requested_current_margin")} label_muted={__("charge_manager.content.requested_current_margin_muted")}>
                <InputFloat
                    unit="A"
                    value={state.requested_current_margin}
                    onValue={this.set("requested_current_margin")}
                    digits={3}
                    min={1000}
                    max={10000}
                    />
            </FormRow>;

        let requested_current_threshold = <FormRow label={__("charge_manager.content.requested_current_threshold")} label_muted={__("charge_manager.content.requested_current_threshold_muted")}>
                <InputNumber
                    unit="s"
                    value={state.requested_current_threshold}
                    onValue={this.set("requested_current_threshold")}
                    min={20}
                    max={3600}
                    />
            </FormRow>;

        let minimum_current = <>
            <FormRow label={__("charge_manager.content.minimum_current_auto")}>
                <Switch desc={__("charge_manager.content.minimum_current_auto_desc")}
                    checked={state.minimum_current_auto}
                    onClick={this.toggle('minimum_current_auto')}
                />
            </FormRow>

            <Collapse in={state.minimum_current_auto}>
                <div>
                    <FormRow label={__("charge_manager.content.minimum_current_vehicle_type")}>
                        <InputSelect items={[
                                ["0", __("charge_manager.content.minimum_current_vehicle_type_other")],
                                ["1", __("charge_manager.content.minimum_current_vehicle_type_zoe")],
                            ]}
                            value={state.minimum_current_vehicle_type}
                            onValue={(v) => this.setState({minimum_current_vehicle_type: parseInt(v)})}
                            />
                    </FormRow>
                </div>
            </Collapse>

            <Collapse in={!state.minimum_current_auto}>
                <div>
                    <FormRow label={      energyManagerMode ? __("charge_manager.content.minimum_current_3p")       : __("charge_manager.content.minimum_current")}
                             label_muted={energyManagerMode ? __("charge_manager.content.minimum_current_3p_muted") : __("charge_manager.content.minimum_current_muted")}>
                        <InputFloat
                            unit="A"
                            value={state.minimum_current}
                            onValue={(v) => this.setState({
                                minimum_current: v,
                                maximum_available_current: Math.max(v, state.maximum_available_current) // TODO Is this a good idea? Should warn instead?
                            })}
                            digits={3}
                            min={6000}
                            max={32000}
                        />
                    </FormRow>

                    {energyManagerMode || warpUltimateMode ?
                        <FormRow label={__("charge_manager.content.minimum_current_1p")} label_muted={__("charge_manager.content.minimum_current_1p_muted")}>
                            <InputFloat
                                unit="A"
                                value={state.minimum_current_1p}
                                onValue={(v) => this.setState({minimum_current_1p: v})}
                                digits={3}
                                min={6000}
                                max={32000}
                            />
                        </FormRow>
                    :
                        null
                    }
                </div>
            </Collapse>
        </>

        let available_current = <FormRow label={__("charge_manager.content.maximum_available_current")}>
                <InputFloat
                    unit="A"
                    value={state.maximum_available_current}
                    onValue={(v) => this.setState({maximum_available_current: v, default_available_current: v})}
                    digits={3}
                    min={0}
                    max={1000000}
                    />
            </FormRow>

        const check_host = (host: string, idx: number): string => {
            let ret: string;

            if (!energyManagerMode && idx != 0 && (host.toLowerCase() == 'localhost' || host == '127.0.0.1')) {
                return __("charge_manager.content.host_exists");
            }

            state.chargers.forEach((charger, i) => {
                if (charger.host.toLowerCase() == host.toLowerCase() && idx != i) {
                    ret = __("charge_manager.content.host_exists");
                    return;
                }
            });

            return ret;
        }

        let chargers = <FormRow label={__("charge_manager.content.managed_boxes")}>
                    <Table
                        columnNames={[__("charge_manager.content.table_charger_name"), __("charge_manager.content.table_charger_host")]}
                        rows={state.chargers.map((charger, i) =>
                            { return {
                                columnValues: [
                                    charger.name,
                                    <a target="_blank" rel="noopener noreferrer" href={(charger.host == '127.0.0.1' || charger.host == 'localhost') ? '/' : "http://" + charger.host}>{charger.host}</a>
                                ],
                                editTitle: __("charge_manager.content.edit_charger_title"),
                                onEditShow: async () => this.setState({editCharger: {name: charger.name.trim(), host: charger.host.trim()}}),
                                onEditGetRows: () => [
                                    {
                                        name: __("charge_manager.content.edit_charger_name"),
                                        value: <InputText value={state.editCharger.name}
                                                        onValue={(v) => this.setState({editCharger: {...state.editCharger, name: v}})}
                                                        maxLength={32}
                                                        required/>
                                    },
                                    {
                                        name: __("charge_manager.content.edit_charger_host"),
                                        value: <InputText value={state.editCharger.host}
                                                        onValue={(v) => this.setState({editCharger: {...state.editCharger, host: v}})}
                                                        maxLength={64}
                                                        pattern="^[a-zA-Z0-9\-\.]+$"
                                                        required
                                                        disabled={!energyManagerMode && (charger.host == '127.0.0.1' || charger.host == 'localhost')}
                                                        class={check_host(state.editCharger.host, i) != undefined ? "is-invalid" : ""}
                                                        invalidFeedback={check_host(state.editCharger.host, i)}/>
                                    }
                                ],
                                onEditSubmit: async () => {
                                    this.setState({chargers: state.chargers.map((charger, k) => i === k ? state.editCharger : charger)});
                                    this.setDirty(true);
                                },
                                onRemoveClick: !energyManagerMode && (charger.host == '127.0.0.1' || charger.host == 'localhost') ? undefined : async () => {
                                    this.setState({chargers: state.chargers.filter((v, idx) => idx != i)});
                                    this.setDirty(true);
                                }
                            }})
                        }
                        addEnabled={state.chargers.length < MAX_CONTROLLED_CHARGERS}
                        addTitle={__("charge_manager.content.add_charger_title")}
                        addMessage={__("charge_manager.content.add_charger_count")(state.chargers.length, MAX_CONTROLLED_CHARGERS)}
                        onAddShow={async () => {
                            this.setState({addCharger: {name: "", host: ""}});
                            this.scan_services();
                            this.intervalID = window.setInterval(this.scan_services, 3000);
                        }}
                        onAddGetRows={() => [
                            {
                                name: __("charge_manager.content.add_charger_name"),
                                value: <InputText value={state.addCharger.name}
                                                onValue={(v) => this.setState({addCharger: {...state.addCharger, name: v}})}
                                                maxLength={32}
                                                required/>
                            },
                            {
                                name: __("charge_manager.content.add_charger_host"),
                                value: <InputText value={state.addCharger.host}
                                                onValue={(v) => this.setState({addCharger: {...state.addCharger, host: v}})}
                                                maxLength={64}
                                                pattern="^[a-zA-Z0-9\-\.]+$"
                                                required
                                                class={check_host(state.addCharger.host, -1) != undefined ? "is-invalid" : ""}
                                                invalidFeedback={check_host(state.addCharger.host, -1)}/>
                            },
                            {
                                name: __("charge_manager.content.add_charger_found"),
                                value:
                                    <ListGroup>
                                    {
                                        state.scanResult.filter(c => !state.chargers.some(c1 => c1.host == c.hostname + ".local" || c1.host == c.ip))
                                            .map(c => (
                                                <ListGroupItem key={c.hostname}
                                                            action type="button"
                                                            onClick={c.error != 0 ? undefined : () => {
                                                                this.setState({addCharger: {host: c.hostname + ".local", name: c.display_name}})
                                                            }}
                                                            style={c.error == 0 ? "" : "cursor: default; background-color: #eeeeee !important;"}>
                                                    <div class="d-flex w-100 justify-content-between">
                                                        <span class="h5 text-left">{c.display_name}</span>
                                                        {c.error == 0 ? null :
                                                            <span class="text-right" style="color:red">{translate_unchecked(`charge_manager.content.scan_error_${c.error}`)}</span>
                                                        }
                                                    </div>
                                                    <div class="d-flex w-100 justify-content-between">
                                                        <a target="_blank" rel="noopener noreferrer" href={"http://" + c.hostname + ".local"}>{c.hostname + ".local"}</a>
                                                        <a target="_blank" rel="noopener noreferrer" href={"http://" + c.ip}>{c.ip}</a>
                                                    </div>
                                                </ListGroupItem>))
                                    }
                                    </ListGroup>
                            }
                        ]}
                        onAddSubmit={async () => {
                            this.setState({chargers: state.chargers.concat({name: state.addCharger.name.trim(), host: state.addCharger.host.trim()})});
                            this.setDirty(true);
                        }}
                        onAddHide={async () => {
                            window.clearInterval(this.intervalID);
                        }} />
            </FormRow>

        return (
            <SubPage>
                <ConfigForm id="charge_manager_config_form" title={__("charge_manager.content.charge_manager")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    {!energyManagerMode || warpUltimateMode ?
                        charge_manager_mode
                    :
                        null
                    }

                    {energyManagerMode ?
                        <>
                            {maximum_available_current}
                            {minimum_current}
                            {chargers}
                        </>
                        : <Collapse in={state.enable_charge_manager}>
                            <div>
                                <FormRow label={__("charge_manager.content.configuration_mode")} label_muted={__("charge_manager.content.configuration_mode_muted")}>
                                    <Button className="form-control" onClick={() => this.setState({showExpert: !state.showExpert})}>
                                        {state.showExpert ? __("component.collapsed_section.hide") : __("component.collapsed_section.show")}
                                    </Button>
                                </FormRow>

                                <Collapse in={state.showExpert}>
                                    <div>
                                        {verbose}
                                        {watchdog}
                                        {maximum_available_current}
                                        {default_available_current}
                                        {requested_current_threshold}
                                        {requested_current_margin}
                                    </div>
                                </Collapse>

                                <Collapse in={!state.showExpert}>
                                    <div>
                                        {available_current}
                                    </div>
                                </Collapse>

                                {minimum_current}
                                {chargers}
                            </div>
                        </Collapse>
                    }
                </ConfigForm>
            </SubPage>
        )
    }
}

render(<ChargeManager/>, $('#charge_manager')[0]);

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
        if (!util.render_allowed() || !state.config.enable_charge_manager || state.config.chargers.length == 0)
            return <></>;

        let cards = state.state.chargers.map((c, i) => {
            let c_state = "";
            let c_info = "";
            let c_body_classes = "";

            let last_update = Math.floor((state.uptime - c.last_update) / 1000);
            let c_status_text = util.toLocaleFixed(c.supported_current / 1000.0, 3) + " " + __("charge_manager.script.ampere_supported");

            if (last_update >= 10)
                c_status_text += "; " + __("charge_manager.script.last_update_prefix") + " " + util.format_timespan(last_update) + (__("charge_manager.script.last_update_suffix"));

            if (c.state != 5) {
                if (state.state.state == 2) {
                    c_body_classes = "bg-danger text-white bg-disabled";
                    c_state = __("charge_manager.script.charge_state_blocked_by_other_box");
                    c_info = __("charge_manager.script.charge_state_blocked_by_other_box_details");
                } else {
                    c_state = translate_unchecked(`charge_manager.script.charge_state_${c.state}`);
                    c_info = util.toLocaleFixed(c.allocated_current / 1000.0, 3) + " " + __("charge_manager.script.ampere_allocated");
                }
            }
            else {
                if (c.error < 192)
                    c_state = __("charge_manager.script.charge_error_type_management");
                else
                    c_state = __("charge_manager.script.charge_error_type_client");

                c_body_classes = "bg-danger text-white bg-disabled";
                c_info = translate_unchecked(`charge_manager.script.charge_error_${c.error}`);
            }

            let charger_config = state.config.chargers[i];
            const name_link = (API.is_dirty("charge_manager/config") || !charger_config) ? c.name :
                                <a target="_blank" rel="noopener noreferrer" href={(charger_config.host == '127.0.0.1' || charger_config.host == 'localhost') ? '/' : "http://" + charger_config.host}>{c.name}</a>

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

        return <>
            <FormRow label={__("charge_manager.status.charge_manager")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
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

            {// The energy manager controls the available current.
                API.hasModule("energy_manager") ? null:
                <FormRow label={__("charge_manager.status.available_current")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                    <InputFloat min={0} max={state.config.maximum_available_current} digits={3} unit="A"
                        value={state.available_current.current}
                        onValue={(v) => API.save("charge_manager/available_current", {"current": v}, __("charge_manager.script.set_available_current_failed"))}
                        showMinMax/>
                </FormRow>
            }

            <FormRow label={__("charge_manager.status.managed_boxes")} labelColClasses="col-lg-4" contentColClasses="col-lg-8 col-xl-4">
                {util.range(Math.ceil(cards.length / 2)).map(i =>
                    <div class="card-deck mb-4">
                        {cards[2 * i]}
                        {cards.length > (2 * i + 1) ? cards[2 * i + 1] : undefined}
                    </div>)
                }
            </FormRow>
        </>
    }
}

render(<ChargeManagerStatus />, $("#status-charge_manager")[0]);

export function init() {
}

export function add_event_listeners(source: API.APIEventTarget) {
}

export function update_sidebar_state(module_init: any) {
    $("#sidebar-charge_manager").prop("hidden", !module_init.charge_manager);
}
