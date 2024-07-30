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
import { Collapse, Button, Spinner, ListGroup, ListGroupItem, Alert } from "react-bootstrap";
import { InputText } from "../../ts/components/input_text";
import { InputPassword } from "../../ts/components/input_password";
import { Lock, Unlock } from "react-feather";
import { SubPage } from "../../ts/components/sub_page";
import { EapConfigID, EapConfigPEAPTTLS, EapConfigTLS } from "./api";
import { InputSelect } from "../../ts/components/input_select";
import { ItemModal } from "../../ts/components/item_modal";

type STAConfig = API.getType["wifi/sta_config"];

type WifiSTAState = {
    scan_show: boolean;
    scan_running: boolean;
    scan_error: string;
    scan_results: Readonly<WifiInfo[]>;
    passphrase_required: boolean;
    passphrase_placeholder: string;
};

type WifiInfo = Exclude<API.getType['wifi/scan_results'], string>[0];

export function wifi_symbol(rssi: number) {
    if (rssi >= -60)
        return <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><title>RSSI: {rssi}</title><path d="M1.42 9a16 16 0 0 1 21.16 0"></path><path d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>;
    if (rssi >= -70)
        return <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><title>RSSI: {rssi}</title><path stroke="#cccccc" d="M1.42 9a16 16 0 0 1 21.16 0"></path><path d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>;
    if (rssi >= -80)
        return <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><title>RSSI: {rssi}</title><path stroke="#cccccc" d="M1.42 9a16 16 0 0 1 21.16 0"></path><path stroke="#cccccc" d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>;
    if (rssi > -100)
        return <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><title>RSSI: {rssi}</title><path stroke="#cccccc" d="M1.42 9a16 16 0 0 1 21.16 0"></path><path stroke="#cccccc" d="M5 12.55a11 11 0 0 1 14.08 0"></path><path stroke="#cccccc" d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>;

    return <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><title>RSSI: {rssi}</title><path stroke="#cccccc" d="M1.42 9a16 16 0 0 1 21.16 0"></path><path stroke="#cccccc" d="M5 12.55a11 11 0 0 1 14.08 0"></path><path stroke="#cccccc" d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line stroke="#cccccc" x1="12" y1="20" x2="12.01" y2="20"></line></svg>;
}

export class WifiSTA extends ConfigComponent<'wifi/sta_config', {}, WifiSTAState> {
    ipconfig_valid: boolean = true;
    scan_timeout: number = null;

    constructor() {
        super('wifi/sta_config',
              __("wifi.script.sta_save_failed"),
              __("wifi.script.sta_reboot_content_changed"), {
                passphrase_placeholder: __("component.input_password.unchanged"),
                passphrase_required: false,
                scan_show: false,
                scan_running: false,
            });

        util.addApiEventListener('wifi/scan_results', (e) => {
            if (e.data == "scan in progress")
                return;

            window.clearTimeout(this.scan_timeout);
            this.scan_timeout = null;

            if (e.data == "scan failed" || e.data == "scan out of memory") {
                this.setState({scan_running: false, scan_error: __("wifi.script.scan_wifi_init_failed")});
                return;
            }

            if (typeof e.data === "string") {
                this.setState({scan_running: false, scan_error: e.data});
                return;
            }

            this.setState({scan_running: false, scan_error: undefined, scan_results: e.data});
        });
    }

    override async isSaveAllowed(cfg: STAConfig) {
        return this.ipconfig_valid;
    }

    override async transformSave(cfg: STAConfig) {
        cfg.dns = cfg.dns == "" ? "0.0.0.0" : cfg.dns;
        cfg.dns2 = cfg.dns2 == "" ? "0.0.0.0" : cfg.dns2;
        return cfg;
    }

    async start_scan() {
        if (this.scan_timeout != null)
            window.clearTimeout(this.scan_timeout);

        this.setState({scan_show: true, scan_running: true});
        try {
            await API.call('wifi/scan', {});
        } catch {
            this.setState({scan_running: false, scan_error: __("wifi.script.scan_wifi_init_failed")});
            return;
        }

        this.scan_timeout = window.setTimeout(async () => {
            this.scan_timeout = null;

            let result = "";
            try {
                result = await util.download("/wifi/scan_results").then(blob => blob.text())
            } catch (e) {
                this.setState({scan_running: false, scan_error: __("wifi.script.scan_wifi_results_failed") + ": " + e.message});
                return;
            }

            let data: WifiInfo[] = JSON.parse(result);
            this.setState({scan_running: false, scan_error: undefined, scan_results: data});
        }, 12000);
    }

    get_scan_results() {
        if (this.state.scan_running) {
            return <div class="form-group text-center">
                <Spinner animation="border" style="width: 3rem; height: 3rem;"/>
            </div>;
        }

        if (this.state.scan_error) {
            return <Alert variant="danger">
                {this.state.scan_error}
            </Alert>;
        }

        if (this.state.scan_results == undefined) {
            return <></>;
        }

        if (this.state.scan_results.length == 0) {
            return <Alert variant="secondary">
                {__("wifi.script.no_ap_found")}
            </Alert>;
        }

        let result = this.state.scan_results.map(ap => {
            let display_name = ap.ssid;
            let enable_bssid_lock = false;

            if (ap.ssid == "") {
                display_name = __("wifi.script.hidden_ap");
                enable_bssid_lock = true;
            } else {
                for (const inner_ap of this.state.scan_results) {
                    if (ap == inner_ap)
                        continue;
                    if (ap.ssid != inner_ap.ssid)
                        continue;

                    display_name = ap.ssid;
                    enable_bssid_lock ||= (ap.rssi - inner_ap.rssi) > 15;
                }
            }

            let passphrase_required = API.get("wifi/sta_config").ssid != ap.ssid && ap.encryption != 0;

            return <ListGroupItem
                        action
                        type="submit"
                        onClick={() => {
                            this.setState({
                                ssid: ap.ssid,
                                bssid: this.string_to_bssid(ap.bssid),
                                passphrase_required: passphrase_required,
                                passphrase_placeholder: passphrase_required ? __("component.input_password.required") : __("component.input_password.unchanged"),
                                enable_sta: true,
                                bssid_lock: enable_bssid_lock
                            });
                            this.setDirty(true);
                        }}
                        key={ap.bssid}>
                        <div class="row align-items-center mb-2">
                            <div class="col">{wifi_symbol(ap.rssi)}</div>
                            <div class="col-auto text-wrap" style="font-size: 1.1rem">{display_name}</div>
                        </div>
                        <div class="row align-items-center">
                            <div class="col">{ap.encryption == 0 ? <Unlock {...{class: "mr-2"} as any}/> : <Lock {...{class: "mr-2"} as any}/>}</div>
                            <div class="col-auto">{ap.bssid}</div>
                        </div>
                </ListGroupItem>
        });

        return <div class="form-group">
            <ListGroup>
            {result}
            </ListGroup>
        </div>;
    }

    last_peap_ttls_state: EapConfigPEAPTTLS[1] = {
        identity: "",
        ca_cert_id: -1,
        username: "",
        password: "",
        client_cert_id: -1,
        client_key_id: -1,
    };

    eap_peap_ttls(state: Readonly<WifiSTAState & STAConfig>) {
        const certs_config = API.get("certs/state");
        const cert_items: [string, string][] = [
            ["-1", __("wifi.content.eap_cert_placeholder")],
        ];
        const key_items: [string, string][] = [
            ["-1", __("wifi.content.eap_key_placeholder")],
        ];
        for (const cert of certs_config.certs) {
            cert_items.push([cert.id.toString(), cert.name]);
            key_items.push([cert.id.toString(), cert.name]);
        }

        if (state.wpa_eap_config[0] === EapConfigID.PEAP_TTLS) {
            this.last_peap_ttls_state = state.wpa_eap_config[1];
        }

        return <>
            <FormRow label={__("wifi.content.eap_username")}>
                <InputText required={state.wpa_eap_config[0] == EapConfigID.PEAP_TTLS}
                    maxLength={64}
                    value={this.last_peap_ttls_state.username}
                    onValue={(v) => {
                        (state.wpa_eap_config as EapConfigPEAPTTLS)[1].username = v;
                        this.setState({wpa_eap_config: state.wpa_eap_config});
                }}/>
            </FormRow>

            <FormRow label={__("wifi.content.eap_password")}>
                <InputPassword
                    maxLength={64}
                    required={state.wpa_eap_config[0] == EapConfigID.PEAP_TTLS && this.last_peap_ttls_state.password === ""}
                    value={this.last_peap_ttls_state.password}
                    onValue={(v) => {
                        (state.wpa_eap_config as EapConfigPEAPTTLS)[1].password = v;
                        this.setState({wpa_eap_config: state.wpa_eap_config});
                    }}/>
            </FormRow>

            <FormRow label={__("wifi.content.eap_identity")} label_muted={__("wifi.content.optional")}>
                <InputText
                    maxLength={64}
                    value={this.last_peap_ttls_state.identity}
                    onValue={(v) => {
                        (state.wpa_eap_config as EapConfigTLS | EapConfigPEAPTTLS)[1].identity = v;
                        this.setState({wpa_eap_config: state.wpa_eap_config})
                    }}
                    />
            </FormRow>

            <FormRow label={__("wifi.content.eap_ca_cert")} label_muted={__("wifi.content.optional_eap_cert_muted")(false)}>
                <InputSelect
                    items={cert_items}
                    value={this.last_peap_ttls_state.ca_cert_id.toString()}
                    onValue={(v) => {
                        (state.wpa_eap_config as EapConfigTLS | EapConfigPEAPTTLS)[1].ca_cert_id = parseInt(v);
                        this.setState({wpa_eap_config: state.wpa_eap_config});
                    }}/>
            </FormRow>

            <FormRow label={__("wifi.content.eap_client_cert")} label_muted={__("wifi.content.optional_eap_cert_muted")(false)}>
                <InputSelect
                    items={cert_items}
                    value={this.last_peap_ttls_state.client_cert_id.toString()}
                    onValue={(v) => {
                        (state.wpa_eap_config as EapConfigPEAPTTLS)[1].client_cert_id = parseInt(v);
                        this.setState({wpa_eap_config: state.wpa_eap_config});
                    }}/>
            </FormRow>

            <FormRow label={__("wifi.content.eap_client_key")} label_muted={__("wifi.content.optional_eap_cert_muted")(true)}>
                <InputSelect
                    items={key_items}
                    value={this.last_peap_ttls_state.client_key_id.toString()}
                    onValue={(v) => {
                        (state.wpa_eap_config as EapConfigPEAPTTLS)[1].client_key_id = parseInt(v);
                        this.setState({wpa_eap_config: state.wpa_eap_config});
                    }}/>
            </FormRow>
        </>
    }

    last_state: EapConfigTLS[1] = {
        identity: "",
        ca_cert_id: -1,
        client_cert_id: -1,
        client_key_id: -1,
    };

    eap_tls(state: Readonly<WifiSTAState & STAConfig>) {
        const certs_config = API.get("certs/state");
        const cert_items: [string, string][] = [
            ["-1", __("wifi.content.eap_cert_placeholder")],
        ];
        for (const cert of certs_config.certs) {
            cert_items.push([cert.id.toString(), cert.name]);
        }

        if (state.wpa_eap_config[0] == EapConfigID.TLS) {
            this.last_state = state.wpa_eap_config[1];
        }

        return <>
            <FormRow label={__("wifi.content.eap_client_cert")} label_muted={__("wifi.content.eap_cert_muted")(false)}>
                <InputSelect
                    required={state.wpa_eap_config[0] == EapConfigID.TLS}
                    items={cert_items.filter((v) => v[0] != "-1")}
                    placeholder={__("wifi.content.eap_cert_placeholder")}
                    value={this.last_state.client_cert_id.toString()}
                    onValue={(v) => {
                        (state.wpa_eap_config as EapConfigTLS)[1].client_cert_id = parseInt(v);
                        this.setState({wpa_eap_config: state.wpa_eap_config});
                    }}/>
            </FormRow>

            <FormRow label={__("wifi.content.eap_client_key")} label_muted={__("wifi.content.eap_cert_muted")(true)}>
                <InputSelect
                    required={state.wpa_eap_config[0] == EapConfigID.TLS}
                    items={cert_items.filter((v) => v[0] != "-1")}
                    placeholder={__("wifi.content.eap_key_placeholder")}
                    value={this.last_state.client_key_id.toString()}
                    onValue={(v) => {
                        (state.wpa_eap_config as EapConfigTLS)[1].client_key_id = parseInt(v);
                        this.setState({wpa_eap_config: state.wpa_eap_config});
                    }}/>
            </FormRow>

            <FormRow label={__("wifi.content.eap_identity")} label_muted={__("wifi.content.optional")}>
                <InputText
                    value={this.last_state.identity}
                    onValue={(v) => {
                        (state.wpa_eap_config as EapConfigTLS | EapConfigPEAPTTLS)[1].identity = v;
                        this.setState({wpa_eap_config: state.wpa_eap_config})
                    }}
                    />
            </FormRow>

            <FormRow label={__("wifi.content.eap_ca_cert")} label_muted={__("wifi.content.optional_eap_cert_muted")(false)}>
                <InputSelect
                    items={cert_items}
                    value={this.last_state.ca_cert_id.toString()}
                    onValue={(v) => {
                        (state.wpa_eap_config as EapConfigTLS | EapConfigPEAPTTLS)[1].ca_cert_id = parseInt(v);
                        this.setState({wpa_eap_config: state.wpa_eap_config});
                    }}/>
            </FormRow>
        </>
    }

    bssid_to_string = (bssid?: number[]) => bssid?.map((x)=> (x < 16 ? '0' : '') + x.toString(16).toUpperCase()).join(":");
    string_to_bssid = (s: string) => s.split(':').map(x => parseInt(x, 16));

    render(props: {}, state: Readonly<STAConfig & WifiSTAState>) {
        if (!util.render_allowed())
            return <SubPage name="wifi_sta" />;

        return (
            <SubPage name="wifi_sta">
                <ConfigForm id="wifi_sta_config_form"
                            title={__("wifi.content.sta_settings")}
                            isModified={this.isModified()}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
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
                                <div class="input-group-append">
                                    <Button className="form-control rounded-right" variant="primary" onClick={() => this.start_scan()}>{__("wifi.content.sta_scan")}</Button>
                                </div>
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

                    <FormRow label={__("wifi.content.wpa_auth_type")}>
                        <InputSelect
                            items={[
                                ["0", __("wifi.content.wpa_personal")],
                                ["1", __("wifi.content.eap_tls")],
                                ["2", __("wifi.content.eap_peap_ttls")],
                            ]}
                            value={state.wpa_eap_config[0].toString()}
                            onValue={(v) => {
                                switch (v) {
                                    case "0":
                                        this.setState({wpa_eap_config: [EapConfigID.None, null]});
                                        break;

                                    case "1":
                                        const eap_config = {
                                            identity: "",
                                            ca_cert_id: -1,
                                            client_cert_id: -1,
                                            client_key_id: -1,
                                        }
                                        this.setState({wpa_eap_config: [EapConfigID.TLS, eap_config]});
                                        break;

                                    case "2":
                                        const eap_config2 = {
                                            identity: "",
                                            ca_cert_id: -1,
                                            username: "",
                                            password: "",
                                            client_cert_id: -1,
                                            client_key_id: -1
                                        }
                                        this.setState({wpa_eap_config: [EapConfigID.PEAP_TTLS, eap_config2]});
                                        break;
                                }
                        }}/>
                    </FormRow>

                    <Collapse in={state.wpa_eap_config[0] === EapConfigID.PEAP_TTLS}>
                        <div>
                            {this.eap_peap_ttls(state)}
                        </div>
                    </Collapse>

                    <Collapse in={state.wpa_eap_config[0] === EapConfigID.TLS}>
                        <div>
                            {this.eap_tls(state)}
                        </div>
                    </Collapse>

                    <Collapse in={state.wpa_eap_config[0] === EapConfigID.None}>
                        <div>
                            <FormRow label={__("wifi.content.sta_passphrase")}>
                                <InputPassword minLength={8} maxLength={63}
                                            required={state.passphrase_required && state.wpa_eap_config[0] == EapConfigID.None}
                                            onValue={this.set("passphrase")}
                                            value={state.passphrase}
                                            placeholder={state.passphrase_placeholder} />
                            </FormRow>
                        </div>
                    </Collapse>

                    <IPConfiguration
                        showAnyAddress
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
                                !API.hasModule("wireguard") || API.get_unchecked("wireguard/config").internal_ip == "0.0.0.0" ? [] :
                                [{ip: util.parseIP(API.get_unchecked("wireguard/config").internal_ip),
                                subnet: util.parseIP(API.get_unchecked("wireguard/config").internal_subnet),
                                name: __("component.ip_configuration.wireguard")}]
                            )
                        }
                        />

                    <FormRow label={__("wifi.content.sta_enable_11b")}>
                        <Switch desc={__("wifi.content.sta_enable_11b_desc")}
                                checked={state.enable_11b}
                                onClick={this.toggle("enable_11b")}
                        />
                    </FormRow>

                </ConfigForm>

                <ItemModal
                    size="sm"
                    onCheck={async () => {
                        return true;
                    }}
                    onSubmit={async () => {
                    }}
                    onHide={async () => {
                        this.setState({scan_show: false});
                    }}
                    show={state.scan_show}
                    title={__("wifi.content.sta_scan")}
                    dialogClassName="modal-width-auto"
                >
                    {this.get_scan_results()}
                </ItemModal>
            </SubPage>
        );
    }
}
