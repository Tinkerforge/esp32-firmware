/* esp32-firmware
 * Copyright (C) 2024 Frederic Henrichs <frederic@tinkerforge.com>
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
import { Watch } from "react-feather";
import { ConfigComponent } from "src/ts/components/config_component";
import { ConfigForm } from "src/ts/components/config_form";
import { FormRow } from "src/ts/components/form_row";
import { InputPassword } from "src/ts/components/input_password";
import { InputText } from "src/ts/components/input_text";
import { NavbarItem } from "src/ts/components/navbar_item";
import { SubPage } from "src/ts/components/sub_page";
import { Switch } from "src/ts/components/switch";
import { __ } from "src/ts/translation";
import "./wireguard";
import { config, management_connection } from "./api";
import { InputNumber } from "src/ts/components/input_number";
import { InputSelect } from "src/ts/components/input_select";
import { ArgonType, hash } from "argon2-browser";

export function RemoteAccessNavbar() {
    return <NavbarItem name="remote_access" module="remote_access" title={__("remote_access.navbar.remote_access")} symbol={<Watch />} />;
}

interface RemoteAccessState {
    password: string
}

export class RemoteAccess extends ConfigComponent<"remote_access/config", {}, RemoteAccessState> {
    constructor() {
        super("remote_access/config",
            __("remote_access.script.save_failed"),
            __("remote_access.script.reboot_content_changed"));
    }

    async get_salt_for_user(username: string) {
        const resp = await fetch(`https://${this.state.relay_host}:${this.state.relay_host_port}/api/auth/get_login_salt?username=${username}`, {
            method: "GET"
        });
        if (resp.status !== 200) {
            throw `Failed to get login_salt for user ${username}: ${await resp.text()}`;
        }
        const json = await resp.text();
        const data = JSON.parse(json);

        return new Uint8Array(data);
    }

    async get_secret() {
        const resp = await fetch(`https://${this.state.relay_host}:${this.state.relay_host_port}/api/user/get_secret`, {
            method: "GET",
            credentials: "include",
        });
        if (resp.status !== 200) {
            throw `Failed to get secret: ${await resp.text()}`;
        }
        const json = await resp.text();
        const data = JSON.parse(json);
        return data
    }

    async registerCharger(cfg: config) {
        const charger_id = API.get("info/name").uid;
        const charger_name = API.get("info/display_name").display_name;
        const mg_charger_address = "10.123.123.2";
        const mg_server_address = "10.123.123.3";
        const mg_charger_keypair = (window as any).wireguard.generateKeypair();

        const keys = [];
        const connections = [];
        for (const i of util.range(0, 5)) {
            const charger_keypair = (window as any).wireguard.generateKeypair();
            const web_keypair = (window as any).wireguard.generateKeypair();
            const charger_address = "10.123." + i + ".2";
            const web_address = "10.123." + i + ".3";
            const port = 51825 + i;

            const iv = new Uint8Array(16);
            crypto.getRandomValues(iv);
            const iv_blob = new Blob([iv]);
            const iv_string = await util.blobToBase64(iv_blob);

            keys.push({
                charger_address: charger_address,
                web_address: web_address,
                charger_public: charger_keypair.publicKey,
                web_private: web_keypair.privateKey,
                web_private_iv: iv_string.replace("data:application/octet-stream;base64,", ""),
                connection_no: i
            });

            const connection: management_connection = {
                internal_ip: charger_address,
                internal_subnet: "255.255.255.0",
                internal_gateway: "10.123." + i + ".1",
                remote_internal_ip: web_address,
                remote_host: this.state.relay_host,
                remote_port: 51820,
                local_port: port,
                private_key: charger_keypair.privateKey,
                remote_public_key: web_keypair.publicKey,
            }
            connections.push(connection);
        }

        const {
            secret,
            secret_salt,
            secret_iv
        } = await this.get_secret();

        const secret_key = await hash({
            pass: this.state.password,
            salt: secret_salt,
            // Takes about 1.5 seconds on a Nexus 4
            time: 2, // the number of iterations
            mem: 19 * 1024, // used memory, in KiB
            hashLen: 16, // desired hash length
            parallelism: 1, // desired parallelism (it won't be computed in parallel, however)
            type: ArgonType.Argon2id,
        })

        const secret_blob = new Blob([new Uint8Array(secret)]);
        const secret_string = await util.blobToBase64(secret_blob);
        const secret_key_blob = new Blob([secret_key.hash]);
        const secret_key_string = await util.blobToBase64(secret_key_blob);
        const secret_iv_blob = new Blob([new Uint8Array(secret_iv)]);
        const secret_iv_string = await util.blobToBase64(secret_iv_blob);

        const registration_data = {
            charger_pub: mg_charger_keypair.publicKey,
            id: charger_id,
            name: charger_name,
            wg_charger_ip: mg_charger_address,
            wg_server_ip: mg_server_address,
            secret: secret_string.replace("data:application/octet-stream;base64,", ""),
            secret_key: secret_key_string.replace("data:application/octet-stream;base64,", ""),
            secret_iv: secret_iv_string.replace("data:application/octet-stream;base64,", ""),
            remote_host: this.state.relay_host,
            remote_port: this.state.relay_host_port,
            config: cfg,
            keys: keys
        };

        const resp = await API.call("remote_access/register", registration_data, __("remote_access.script.save_failed"));
        await API.save("remote_access/remote_connection_config", {
            connections: connections
        }, __("remote_access.script.save_failed"));
        const json = (JSON.parse(await resp.text()));
        const ret = {
            charger_pub: mg_charger_keypair.publicKey,
            wg_charger_ip: mg_charger_address,
            wg_server_ip: mg_server_address,
            charger_private: mg_charger_keypair.privateKey,
            remote_public: json.management_pub,
            password: json.charger_password,
        }

        return ret;
    }

    async login(): Promise<boolean> {
        const salt = await this.get_salt_for_user(this.state.username);
        const login_hash = await hash({
            pass: this.state.password,
            salt: salt,
            time: 2,
            mem: 19 * 1024,
            hashLen: 24,
            parallelism: 1,
            type: ArgonType.Argon2id
        });
        const login_key = login_hash.hash;

        const login_schema = {
            username: this.state.username,
            login_key: [].slice.call(login_key),
        };

        const resp = await fetch("https://" + this.state.relay_host + ":" + this.state.relay_host_port + "/api/auth/login", {
            method: "POST",
            credentials: "include",
            body: JSON.stringify(login_schema),
            headers: {
                "Content-Type": "application/json"
            }
        });

        const login_blob = new Blob([login_key]);
        const login_string = await util.blobToBase64(login_blob);

        this.setState({login_key: login_string.replace("data:application/octet-stream;base64,", "")});

        return resp.status === 200;
    }

    override async isSaveAllowed(cfg: config): Promise<boolean> {
        return await this.login();
    }

    override async sendSave(t: "remote_access/config", cfg: config): Promise<void> {
        cfg.login_key = this.state.login_key;
        const info = await this.registerCharger(cfg);
        cfg.password = info.password;

        await API.save("remote_access/management_connection", {
            internal_ip: info.wg_charger_ip,
            internal_gateway: "10.123.123.1",
            internal_subnet: "255.255.255.0",
            remote_internal_ip: info.wg_server_ip,
            remote_host: this.state.relay_host,
            remote_port: 51820,
            local_port: 51820,
            private_key: info.charger_private,
            remote_public_key: info.remote_public
        }, __("remote_access.script.save_failed"));
        await super.sendSave(t, cfg);
    }

    render() {
        if (!util.render_allowed())
            return <></>

        const cert_config = API.get("certs/state");
        const cert_items: [string, string][] = [["-1", __("remote_access.content.not_used")]];
        for (const cert of cert_config.certs) {
            cert_items.push([cert.id.toString(), cert.name]);
        }

        return <SubPage name="remote_access">
                    <ConfigForm id="remote_access_config_form"
                                title={__("remote_access.content.remote_access")}
                                isModified={this.isModified()}
                                isDirty={this.isDirty()}
                                onReset={this.reset}
                                onSave={this.save}
                                onDirtyChange={this.setDirty}>
                        <FormRow label={__("remote_access.content.enable")}>
                            <Switch checked={this.state.enable}
                                    onClick={() => {
                                        this.setState({enable: !this.state.enable});
                                    }} />
                        </FormRow>
                        <FormRow label={__("remote_access.content.username")}>
                            <InputText value={this.state.username}
                                       required
                                       maxLength={64}
                                       onValue={(v) => {
                                            this.setState({username: v});
                                       }} />
                        </FormRow>
                        <FormRow label={__("remote_access.content.password")}>
                            <InputPassword required
                                           maxLength={64}
                                           value={this.state.password}
                                           onValue={(v) => {
                                                this.setState({password: v});
                                           }} />
                        </FormRow>
                        <FormRow label={__("remote_access.content.relay_host")}
                                 label_muted={__("remote_access.content.relay_host_muted")}>
                            <InputText required
                                       maxLength={64}
                                       value={this.state.relay_host}
                                       onValue={(v) => {
                                            this.setState({relay_host: v})
                                       }} />
                        </FormRow>
                        <FormRow label={__("remote_access.content.relay_host_port")}>
                            <InputNumber required
                                         min={1}
                                         max={65565}
                                         value={this.state.relay_host_port}
                                         onValue={v => this.setState({relay_host_port: v})} />
                        </FormRow>
                        <FormRow label={__("remote_access.content.self_signed_cert")}>
                            <InputSelect items={cert_items} value={this.state.self_signed_cert_id} onValue={(v) => {
                                this.setState({self_signed_cert_id: parseInt(v)});
                            }}/>
                        </FormRow>
                    </ConfigForm>
                </SubPage>
    }
}


export function init() {}
