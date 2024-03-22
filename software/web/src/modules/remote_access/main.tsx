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
import { Button } from "react-bootstrap";
import { InputNumber } from "src/ts/components/input_number";
import { InputSelect } from "src/ts/components/input_select";


export function RemoteAccessNavbar() {
    return <NavbarItem name="remote_access" module="remote_access" title={__("remote_access.navbar.remote_access")} symbol={<Watch />} />;
}

export class RemoteAccess extends ConfigComponent<"remote_access/config", {}> {
    constructor() {
        super("remote_access/config",
            __("remote_access.script.save_failed"),
            __("remote_access.script.reboot_content_changed"));
    }

    async registerCharger() {
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
            keys.push({
                charger_address: charger_address,
                web_address: web_address,
                charger_public: charger_keypair.publicKey,
                web_private: web_keypair.privateKey,
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

        await API.save("remote_access/remote_connection_config", {
            connections: connections
        }, __("remote_access.script.save_failed"));

        const charger = {
            charger: {
                charger_pub: mg_charger_keypair.publicKey,
                id: charger_id,
                name: charger_name,
                wg_charger_ip: mg_charger_address,
                wg_server_ip: mg_server_address
            },
            keys: keys
        };

        const resp = await fetch("https://" + this.state.relay_host + ":" + this.state.relay_host_port + "/api/charger/add", {
            method: "POST",
            credentials: "include",
            body: JSON.stringify(charger),
            headers: {
                "Content-Type": "application/json"
            }
        });

        const pub_key = (JSON.parse(await resp.text()));

        const ret = {
            charger_pub: mg_charger_keypair.publicKey,
            wg_charger_ip: mg_charger_address,
            wg_server_ip: mg_server_address,
            charger_private: mg_charger_keypair.privateKey,
            remote_public: pub_key.management_pub
        }

        return ret;
    }

    async login(): Promise<boolean> {
        const login_schema = {
            email: this.state.email,
            password: this.state.password
        };

        const resp = await fetch("https://" + this.state.relay_host + ":" + this.state.relay_host_port + "/api/auth/login", {
            method: "POST",
            credentials: "include",
            body: JSON.stringify(login_schema),
            headers: {
                "Content-Type": "application/json"
            }
        });

        return resp.status === 200;
    }

    override async isSaveAllowed(cfg: config): Promise<boolean> {
        return await this.login();
    }

    override async sendSave(t: "remote_access/config", cfg: config): Promise<void> {
        const info = await this.registerCharger();

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
                        <FormRow label={__("remote_access.content.email")}>
                            <InputText value={this.state.email}
                                       required
                                       maxLength={64}
                                       onValue={(v) => {
                                            this.setState({email: v});
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
