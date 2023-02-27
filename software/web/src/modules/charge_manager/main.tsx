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

import feather from "../../ts/feather";

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, render, Fragment, Component, Attributes, ComponentChild, ComponentChildren, Ref } from "preact";
import { translate_unchecked, __ } from "../../ts/translation";

import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { FormGroup } from "../../ts/components/form_group";
import { InputText } from "../../ts/components/input_text";
import { Button, Card, Collapse, ListGroup, Modal } from "react-bootstrap";
import { InputSelect } from "src/ts/components/input_select";
import { InputFloat } from "src/ts/components/input_float";
import { ItemModal } from "src/ts/components/item_modal";
import { Switch } from "src/ts/components/switch";
import { config } from "./api";
import { IndicatorGroup } from "src/ts/components/indicator_group";

type ChargeManagerConfig = API.getType['charge_manager/config'];
type ChargerConfig = ChargeManagerConfig["chargers"][0];
type ScanCharger = Exclude<API.getType['charge_manager/scan_result'], string>[0];

const MAX_CONTROLLED_CHARGERS = 10;

let charger_add_symbol = <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-server" style=""><rect x="2" y="14" width="20" height="8" rx="2" ry="2"></rect><line y1="18" y2="18" x1="18" x2="18.01"></line><line x1="19" x2="19" y1="3" y2="9"></line><line x1="22" x2="16" y1="6" y2="6"></line></svg>
let charger_delete_symbol = <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-server mr-2" style=""><rect x="2" y="14" width="20" height="8" rx="2" ry="2"></rect><line y1="18" y2="18" x1="18" x2="18.01"></line><line x1="17" x2="22" y1="4" y2="9"></line><line x1="22" x2="17" y1="4" y2="9"></line></svg>
let charger_symbol = <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-server" style=""><rect x="2" y="8" width="20" height="8" rx="2" ry="2"></rect><line y1="12" y2="12" x1="18" x2="18.01"></line></svg>


interface ChargeManagerState {
    showModal: boolean
    newCharger: ChargerConfig
    managementEnabled: boolean
    showExpert: boolean
    scanResult: Readonly<ScanCharger[]>
    energyManagerMode: boolean
}


export class ChargeManager extends ConfigComponent<'charge_manager/config', {}, ChargeManagerState> {
    intervalID: number = null;

    constructor() {
        super('charge_manager/config',
              __("charge_manager.script.save_failed"),
              __("charge_manager.script.reboot_content_changed"));

        this.state = {
            showModal: false,
            newCharger: {host: "", name: ""},
            managementEnabled: false,
            showExpert: false,
            scanResult: []
        } as any;

        // Does not check if the event exists, in case the evse module is not compiled in.
        util.eventTarget.addEventListener_unchecked('evse/management_enabled', () => {
            let evse_enabled = API.get_maybe('evse/management_enabled');
            if (evse_enabled != null) {
                this.setState({managementEnabled: evse_enabled.enabled});
            }
        });

        util.eventTarget.addEventListener('charge_manager/scan_result', () => {
            this.addScanResults( API.get('charge_manager/scan_result') as ScanCharger[]);
        });

        util.eventTarget.addEventListener('info/modules', () => {
            this.setState({energyManagerMode: !!((API.get('info/modules') as any).energy_manager)})
        });
    }

    addScanResults(result: ScanCharger[]) {
        let copy = [...this.state.scanResult];
        outer_loop:
        for (let newC of result) {
            for (let oldIdx in copy) {
                let oldC = copy[oldIdx];
                if (newC.hostname != oldC.hostname)
                    continue;

                if (oldC.ip == "[no_address]")
                    copy[oldIdx].ip = newC.ip;

                continue outer_loop;
            }
            copy.push(newC);
        }
        this.setState({scanResult: copy});
    }

    setCharger (i: number, val: Partial<ChargerConfig>){
        let chargers = this.state.chargers;
        chargers[i] = {...chargers[i], ...val};
        this.setState({chargers: chargers});
    }

    hackToAllowSave() {
        document.getElementById("charge_manager_config_form").dispatchEvent(new Event('input'));
    }

    override async sendSave(t: "charge_manager/config", cfg: config) {
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
        await API.save_maybe('evse/management_enabled', {"enabled": this.state.managementEnabled}, translate_unchecked("charge_manager.script.save_failed"));
        await super.sendSave(t, cfg);
    }

    override async sendReset(t: "charge_manager/config"){
        await API.save_maybe('evse/management_enabled', {"enabled": false}, translate_unchecked("charge_manager.script.save_failed"));
        await super.sendReset(t);
    }

    override getIsModified(t: "charge_manager/config"): boolean {
        let evse_enabled = API.get_maybe("evse/management_enabled");
        if (evse_enabled != null)
            if (evse_enabled.enabled)
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


    scan_timeout: number = null;
    async scan_services()
    {
        try {
            await API.call('charge_manager/scan', {}, __("charge_manager.script.scan_failed"))
        } catch {
            return;
        }

        if (this.scan_timeout != null)
            window.clearTimeout(this.scan_timeout);

        this.scan_timeout = window.setTimeout(async function () {
            this.scan_timeout = null;

            let result = "";
            try {
                result = await util.download("/charge_manager/scan_result").then(blob => blob.text());
                let parsed = JSON.parse(result);
                this.addScanResults(parsed);
            } catch {
            }
        }, 3000);
    }

    intToIP(int: number) {
        var part1 = int & 255;
        var part2 = ((int >> 8) & 255);
        var part3 = ((int >> 16) & 255);
        var part4 = ((int >> 24) & 255);

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
            const eth_subnet = util.parseIP(API.get_maybe("ethernet/config").subnet);
            const eth_ip = util.parseIP(API.get_maybe("ethernet/config").ip);
            const eth_network = eth_ip & eth_subnet;
            const eth_broadcast = (~eth_subnet) | eth_network;
            if (API.get_maybe("ethernet/config")?.subnet != "255.255.255.254" && (v == this.intToIP(eth_broadcast) || v == this.intToIP(eth_network)))
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
        if (!util.allow_render)
            return <></>

        let addChargerCard = <div class="col mb-4">
                <Card className="h-100" key={999}>
                <div class="card-header d-flex justify-content-between align-items-center">
                    {charger_add_symbol}
                    <Button variant="outline-dark" size="sm" style="visibility: hidden;">
                        {charger_delete_symbol}<span style="font-size: 1rem; vertical-align: middle;">{__("charge_manager.script.delete")}</span>
                    </Button>
                </div>
                <Card.Body>
                    {state.chargers.length >= MAX_CONTROLLED_CHARGERS
                        ? <span>{__("charge_manager.script.add_charger_disabled_prefix") + MAX_CONTROLLED_CHARGERS + __("charge_manager.script.add_charger_disabled_suffix")}</span>
                        : <Button variant="light" size="lg" block style="height: 100%;" onClick={() => this.setState({showModal: true})}>{__("charge_manager.script.add_charger")}</Button>}
                </Card.Body>
            </Card>
        </div>

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

        let minimum_current = <FormRow label={__("charge_manager.content.minimum_current")} label_muted={__("charge_manager.content.minimum_current_muted")}>
                <InputFloat
                    unit="A"
                    value={state.minimum_current}
                    onValue={(v) => this.setState({
                        minimum_current: v,
                        maximum_available_current: Math.max(v, state.maximum_available_current)
                    })}
                    digits={3}
                    min={6000}
                    max={32000}
                    />
            </FormRow>;

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

        let chargers = <FormRow label={__("charge_manager.content.managed_boxes")}>
                <div class="row row-cols-1 row-cols-md-2">
                {state.chargers.map((c, i) => (
                    <div class="col mb-4">
                    <Card className="h-100" key={i}>
                        <div class="card-header d-flex justify-content-between align-items-center">
                            {charger_symbol}
                            <Button variant="outline-dark" size="sm"
                                    style={c.host == "127.0.0.1" ? "visibility: hidden;" : ""}
                                    onClick={() => {
                                        this.setState({chargers: state.chargers.filter((v, idx) => idx != i)});
                                        this.hackToAllowSave();} }>
                                {charger_delete_symbol}<span style="font-size: 1rem; vertical-align: middle;">{__("charge_manager.script.delete")}</span>
                            </Button>
                        </div>
                        <Card.Body>
                            <FormGroup label={__("charge_manager.script.display_name")}>
                                <InputText value={c.name}
                                        onValue={(v) => this.setCharger(i, {name: v})}
                                        maxLength={32}
                                        required/>
                            </FormGroup>
                            <FormGroup label={__("charge_manager.script.host")}>
                                <InputText value={c.host}
                                        onValue={(v) => this.setCharger(i, {host: v})}
                                        maxLength={64}
                                        required/>
                            </FormGroup>
                        </Card.Body>
                    </Card>
                    </div>
                )).concat(addChargerCard)}
                </div>
            </FormRow>

        let modal = <ItemModal onSubmit={() => {
                this.setState({showModal: false,
                    chargers: state.chargers.concat(state.newCharger),
                    newCharger: {name: "", host: ""}});
                this.hackToAllowSave();}}
            onHide={() => this.setState({showModal: false})}
            onEnter={() => {this.scan_services(); this.intervalID = setInterval(this.scan_services, 3000)}}
            onExited={() => {this.setState({scanResult: []}); window.clearInterval(this.intervalID)}}
            show={state.showModal}
            no_variant="secondary"
            yes_variant="primary"
            title={__("charge_manager.content.add_charger_modal_title")}
            no_text={__("charge_manager.content.add_charger_modal_abort")}
            yes_text={__("charge_manager.content.add_charger_modal_save")}>
                    <FormGroup label={__("charge_manager.content.add_charger_modal_name")}>
                                <InputText value={state.newCharger.name}
                                        onValue={(v) => this.setState({newCharger: {...state.newCharger, name: v}})}
                                        maxLength={32}
                                        required/>
                            </FormGroup>
                            <FormGroup label={__("charge_manager.content.add_charger_modal_host")}>
                                <InputText value={state.newCharger.host}
                                        onValue={(v) => this.setState({newCharger: {...state.newCharger, host: v}})}
                                        maxLength={64}
                                        required/>
                            </FormGroup>
                            <FormGroup label={__("charge_manager.content.add_charger_modal_found")}>
                                <ListGroup>
                                {
                                    state.scanResult.filter(c => !state.chargers.some(c1 => c1.host == c.hostname + ".local"))
                                        .map(c => (
                                            <ListGroup.Item action type="button"
                                                        onClick={c.error != 0 ? () => {} : () => {
                                                            this.setState({newCharger: {host: c.hostname + ".local", name: c.display_name}})
                                                        }}
                                                        style={c.error == 0 ? "" : "background-color: #eeeeee !important;"}>
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
                                            </ListGroup.Item>))
                                }
                                </ListGroup>
                            </FormGroup>
                </ItemModal>

        return (
            <>
                <ConfigForm id="charge_manager_config_form" title={__("charge_manager.content.charge_manager")} isModified={this.isModified()} onSave={() => this.save()} onReset={this.reset} onDirtyChange={(d) => this.ignore_updates = d}>
                    {state.energyManagerMode ? null:
                        charge_manager_mode
                    }

                    {state.energyManagerMode ?
                        <>
                            {maximum_available_current}
                            {minimum_current}
                            {chargers}
                        </>
                    :
                    <Collapse in={state.enable_charge_manager}>
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
                                    {minimum_current}
                                </div>
                            </Collapse>

                            <Collapse in={!state.showExpert}>
                                <div>
                                    {available_current}
                                </div>
                            </Collapse>

                            {chargers}
                        </div>
                    </Collapse>
                    }
                </ConfigForm>
                {modal}
            </>
        )
    }
}

render(<ChargeManager/>, $('#charge_manager')[0]);

interface ChargeManagerStatusState {
    state: API.getType['charge_manager/state']
    available_current: API.getType['charge_manager/available_current']
    config: API.getType['charge_manager/config']
    energyManagerMode: boolean
    uptime: number
}

export class ChargeManagerStatus extends Component<{}, ChargeManagerStatusState> {
    constructor() {
        super();

        util.eventTarget.addEventListener('charge_manager/state', () => {
            this.setState({state: API.get_maybe('charge_manager/state')})
        });

        util.eventTarget.addEventListener('charge_manager/available_current', () => {
            this.setState({available_current: API.get_maybe('charge_manager/available_current')})
        });

        util.eventTarget.addEventListener('charge_manager/config', () => {
            this.setState({config: API.get_maybe('charge_manager/config')})
        });

        util.eventTarget.addEventListener('info/modules', () => {
            this.setState({energyManagerMode: !!((API.get('info/modules') as any).energy_manager)})
        });

        util.eventTarget.addEventListener('info/keep_alive', () => {
            this.setState({uptime: API.get('info/keep_alive').uptime})
        });
    }

    render(props: {}, state: Readonly<ChargeManagerStatusState>) {
        if (!util.allow_render || !state.config.enable_charge_manager)
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

            return  <div class="card">
                        <h5 class="card-header">{c.name}</h5>
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

            {state.energyManagerMode ? null:
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

render(<ChargeManagerStatus/>, $('#status-charge_manager')[0]);

export function init() {
}

export function add_event_listeners(source: API.APIEventTarget) {
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-charge_manager').prop('hidden', !module_init.charge_manager);
}
