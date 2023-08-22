/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

import { h, Fragment } from "preact";
import { __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { IPConfiguration } from "../../ts/components/ip_configuration";
import { Dropdown, Spinner } from "react-bootstrap";
import { InputText } from "../../ts/components/input_text";
import { InputPassword } from "../../ts/components/input_password";
import { Lock, Unlock } from "react-feather";
import { SubPage } from "../../ts/components/sub_page";

type STAConfig = API.getType['wifi/sta_config'];

type WifiSTAState = {
    scan_running: boolean
    scan_results: Readonly<WifiInfo[]>
    passphrase_required: boolean,
    passphrase_placeholder: string
}

type WifiInfo = Exclude<API.getType['wifi/scan_results'], string>[0];

export function wifi_symbol(rssi: number) {
    if(rssi >= -60)
        return <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><title>RSSI: {rssi}</title><path d="M1.42 9a16 16 0 0 1 21.16 0"></path><path d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>;
    if(rssi >= -70)
        return <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><title>RSSI: {rssi}</title><path stroke="#cccccc" d="M1.42 9a16 16 0 0 1 21.16 0"></path><path d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>;
    if(rssi >= -80)
        return <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><title>RSSI: {rssi}</title><path stroke="#cccccc" d="M1.42 9a16 16 0 0 1 21.16 0"></path><path stroke="#cccccc" d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>;

    return <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><title>RSSI: {rssi}</title><path stroke="#cccccc" d="M1.42 9a16 16 0 0 1 21.16 0"></path><path stroke="#cccccc" d="M5 12.55a11 11 0 0 1 14.08 0"></path><path stroke="#cccccc" d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>;
}

export class WifiSTA extends ConfigComponent<'wifi/sta_config', {}, WifiSTAState> {
    ipconfig_valid: boolean = true;
    scan_timeout: number = null;

    constructor() {
        super('wifi/sta_config',
              __("wifi.script.sta_config_failed"),
              __("wifi.script.sta_reboot_content_changed"));

        util.addApiEventListener('wifi/scan_results', (e) => {
            if (e.data == "scan in progress")
                return;

            window.clearTimeout(this.scan_timeout);
            this.scan_timeout = null;

            if (e.data == "scan failed") {
                console.log("scan failed");
                this.setState({scan_results: JSON.parse("[]")});
                return;
            }

            if (typeof e.data !== "string")
                this.setState({scan_running: false, scan_results: e.data});
        }, false);

        this.state = {passphrase_placeholder: __("component.input_password.unchanged"),
                      passphrase_required: false,
                      scan_running: false} as any;
    }

    override async isSaveAllowed(cfg: STAConfig) {
        return this.ipconfig_valid;
    }

    override async transformSave(cfg: STAConfig) {
        cfg.dns = cfg.dns == "" ? "0.0.0.0" : cfg.dns;
        cfg.dns2 = cfg.dns2 == "" ? "0.0.0.0" : cfg.dns;
        return cfg;
    }

    toggleDropdown = async (isOpen: boolean, event: Event, metadata: {source: 'select' | 'click' | 'rootClose' | 'keydown'}) => {
        if (!isOpen)
            return;

        if (this.scan_timeout != null)
            window.clearTimeout(this.scan_timeout);

        this.setState({scan_running: true});
        try {
            await API.call('wifi/scan', {}, __("wifi.script.scan_wifi_init_failed"));
        } catch {
            this.setState({scan_running: false});
            return;
        }

        this.scan_timeout = window.setTimeout(async () => {
            this.scan_timeout = null;

            let result = ""
            try {
                result = await util.download("/wifi/scan_results").then(blob => blob.text())
            } catch (e) {
                util.add_alert("wifi_scan_failed", "alert-danger", __("wifi.script.scan_wifi_results_failed"), e.message);
                this.setState({scan_running: false});
                return;
            }

            let data: WifiInfo[] = JSON.parse(result);
            this.setState({scan_running: false, scan_results: data});
        }, 12000);
    }

    hackToAllowSave() {
        document.getElementById("wifi_sta_config_form").dispatchEvent(new Event('input'));
    }

    get_scan_results(state: Readonly<STAConfig & WifiSTAState>) {
        if (state.scan_running) {
            return <>
                <Dropdown.Header>{__("wifi.content.sta_scanning")}</Dropdown.Header>
                <div class="text-center">
                    <Spinner animation="border" style="width: 3rem; height: 3rem;"/>
                </div>
            </>;
        }

        if (state.scan_results == undefined)
            return <></>;

        if (state.scan_results.length == 0)
            return <Dropdown.Header>{__("wifi.script.no_ap_found")}</Dropdown.Header>;

        let result = state.scan_results.map(ap => {
            let display_name = ap.ssid;
            let enable_bssid_lock = false;

            if (ap.ssid == "") {
                display_name = `${__("wifi.script.hidden_ap")} (${ap.bssid})`;
                enable_bssid_lock = true;
            } else {
                for (const inner_ap of state.scan_results) {
                    if (ap == inner_ap)
                        continue;
                    if (ap.ssid != inner_ap.ssid)
                        continue;

                    display_name = `${ap.ssid} (${ap.bssid})`;
                    enable_bssid_lock ||= (ap.rssi - inner_ap.rssi) > 15;
                }
            }

            let passphrase_required = API.get("wifi/sta_config").ssid != ap.ssid && ap.encryption != 0;

            return <Dropdown.Item
                        as="button"
                        type="button"
                        onClick={() => {
                            this.setState({
                                ssid: ap.ssid,
                                bssid: this.string_to_bssid(ap.bssid),
                                passphrase_required: passphrase_required,
                                passphrase_placeholder: passphrase_required ? __("component.input_password.required") : __("component.input_password.unchanged"),
                                enable_sta: true,
                                bssid_lock: enable_bssid_lock
                            });
                            this.hackToAllowSave();
                        }}
                        key={ap.bssid}>
                    {wifi_symbol(ap.rssi)}
                    {ap.encryption == 0 ? <Unlock/> : <Lock/>}
                    <span class="pl-2">{display_name}</span>
                </Dropdown.Item>
        });
        return <>
            <Dropdown.Header>{__("wifi.script.select_ap")}</Dropdown.Header>
            {result}
        </>;
    }

    bssid_to_string = (bssid?: number[]) => bssid?.map((x)=> (x < 16 ? '0' : '') + x.toString(16).toUpperCase()).join(":");
    string_to_bssid = (s: string) => s.split(':').map(x => parseInt(x, 16));

    render(props: {}, state: Readonly<STAConfig & WifiSTAState>) {
        if (!util.render_allowed())
            return <></>

        return (
            <SubPage>
                <ConfigForm id="wifi_sta_config_form"
                            title={__("wifi.content.sta_settings")}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={(d) => this.ignore_updates = d}
                            isModified={this.isModified()}>
                    <FormRow label={__("wifi.content.sta_enable_sta")}>
                        <Switch desc={__("wifi.content.sta_enable_sta_desc")}
                                checked={state.enable_sta}
                                onClick={this.toggle("enable_sta")}
                        />
                    </FormRow>

                    <FormRow label={__("wifi.content.sta_ssid")}>
                            <InputText required={state.enable_sta}
                                    maxLength={32}
                                    value={state.ssid}
                                    onValue={this.set("ssid")}>
                                <Dropdown className="input-group-append" onToggle={this.toggleDropdown}>
                                    <Dropdown.Toggle className="form-control rounded-right">{__("wifi.content.sta_scan")}</Dropdown.Toggle>
                                    <Dropdown.Menu align="right">
                                        {this.get_scan_results(state)}
                                    </Dropdown.Menu>
                                </Dropdown>
                            </InputText>
                    </FormRow>

                    <FormRow label={__("wifi.content.sta_bssid")}>
                        <InputText pattern="([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}"
                                   value={this.bssid_to_string(state.bssid)}
                                   onValue={(v) => this.setState({bssid: this.string_to_bssid(v)})}
                                   invalidFeedback={__("wifi.content.sta_bssid_invalid")}
                                   />
                    </FormRow>

                    <FormRow label={__("wifi.content.sta_bssid_lock")}>
                        <Switch desc={__("wifi.content.sta_bssid_lock_desc")}
                                checked={state.bssid_lock}
                                onClick={this.toggle("bssid_lock")}
                        />
                    </FormRow>

                    <FormRow label={__("wifi.content.sta_passphrase")}>
                        <InputPassword minLength={8} maxLength={63}
                                       required={state.passphrase_required}
                                       onValue={this.set("passphrase")}
                                       value={state.passphrase}
                                       placeholder={state.passphrase_placeholder} />
                    </FormRow>

                    <IPConfiguration
                        showDhcp
                        showDns
                        onValue={(v) => this.setState(v)}
                        value={state}
                        setValid={(v) => this.ipconfig_valid = v}
                        forbidNetwork={[
                                {ip: util.parseIP("127.0.0.1"), subnet: util.parseIP("255.0.0.0"), name: "localhost"}
                            ].concat(
                                [{ip: util.parseIP(API.get("wifi/ap_config").ip),
                                subnet: util.parseIP(API.get("wifi/ap_config").subnet),
                                name: __("component.ip_configuration.wifi_ap")}]
                            ).concat(
                                !API.hasModule("wireguard") || API.get_maybe("wireguard/config").internal_ip == "0.0.0.0" ? [] :
                                [{ip: util.parseIP(API.get_maybe("wireguard/config").internal_ip),
                                subnet: util.parseIP(API.get_maybe("wireguard/config").internal_subnet),
                                name: __("component.ip_configuration.wireguard")}]
                            )
                        }
                        />

                </ConfigForm>
            </SubPage>
        );
    }
}
