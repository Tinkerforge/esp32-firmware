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
import { Smartphone } from "react-feather";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputPassword } from "../../ts/components/input_password";
import { InputText } from "../../ts/components/input_text";
import { NavbarItem } from "../../ts/components/navbar_item";
import { SubPage } from "../../ts/components/sub_page";
import { Switch } from "../../ts/components/switch";
import { __ } from "../../ts/translation";
import "./wireguard";
import { add_user, config, RegistrationState, update_config } from "./api";
import { InputNumber } from "../../ts/components/input_number";
import { InputSelect } from "../../ts/components/input_select";
import { ArgonType, hash } from "argon2-browser";
import { CollapsedSection } from "../../ts/components/collapsed_section";
import { Container, Modal, Row, Spinner } from "react-bootstrap";
import { Table, TableRow } from "ts/components/table";

export function RemoteAccessNavbar() {
    return <NavbarItem name="remote_access" module="remote_access" title={__("remote_access.navbar.remote_access")} symbol={<Smartphone />} />;
}

interface RemoteAccessState {
    login_key: string,
    status_modal_string: string,
    addUser: {
        email: string,
        password: string,
        note: string,
    },
    removeUsers: number[],
}

export class RemoteAccess extends ConfigComponent<"remote_access/config", {}, RemoteAccessState> {

    resolve: ((arg0?: any) => void);
    reject: (arg0?: any) => void;
    constructor() {
        super("remote_access/config",
              () => __("remote_access.script.save_failed"),
              () => __("remote_access.script.reboot_content_changed"));

        this.resolve = undefined;
        this.reject = undefined;
        util.addApiEventListener("remote_access/registration_state", () => {
            if (!this.resolve || !this.reject) {
                return;
            }
            const state = API.get("remote_access/registration_state");
            if (state.state === RegistrationState.Success) {
                if (state.message !== "") {
                    this.resolve(state.message);
                } else {
                    this.resolve();
                }
                this.resolve = undefined;
                this.reject = undefined;
            } else if (state.state === RegistrationState.Error) {
                this.reject(state.message);
                this.resolve = undefined;
                this.reject = undefined;
            }
        });
        this.setState({status_modal_string: "", removeUsers: []});
    }

    async get_login_salt(cfg: util.NoExtraProperties<API.getType["remote_access/register"]["config"]>) {
        this.setState({status_modal_string: __("remote_access.content.prepare_login")});
        const getLoginSaltPromise: Promise<string> = new Promise((resolve, reject) => {
            this.resolve = resolve;
            this.reject = reject;
        });
        await API.call("remote_access/get_login_salt", cfg, () => __("remote_access.script.save_failed"));

        const bs64LoginSalt = await getLoginSaltPromise;
        const encodedString = "data:application/octet-stream;base64," + bs64LoginSalt;
        const res = await fetch(encodedString);

        return new Uint8Array(await res.arrayBuffer());
    }

    async get_secret_salt(cfg: util.NoExtraProperties<API.getType["remote_access/register"]["config"]>) {
        this.setState({status_modal_string: __("remote_access.content.prepare_encryption")});
        const getSecretPromise: Promise<string> = new Promise((resolve, reject) => {
            this.resolve = resolve;
            this.reject = reject;
        });
        await API.call("remote_access/get_secret_salt", cfg, () => __("remote_access.script.save_failed"));
        const bs64Secret = await getSecretPromise;
        const encodedString = "data:application/octet-stream;base64," + bs64Secret;
        const res = await fetch(encodedString);

        return new Uint8Array(await res.arrayBuffer());
    }

    login(data: util.NoExtraProperties<API.getType["remote_access/register"]>) {
        this.setState({status_modal_string: __("remote_access.content.logging_in")});
        const loginPromise: Promise<void> = new Promise((resolve, reject) => {
            this.resolve = resolve;
            this.reject = reject;
        });

        API.call("remote_access/login", data, () => __("remote_access.script.save_failed"));
        return loginPromise;
    }

    async runRegistration(cfg: util.NoExtraProperties<API.getType["remote_access/register"]>) {
        this.setState({status_modal_string: __("remote_access.content.registration")});
        const registrationPromise: Promise<void> = new Promise((resolve, reject) => {
            this.resolve = resolve;
            this.reject = reject;
        });

        await API.call("remote_access/register", cfg, () => __("remote_access.script.save_failed"));
        await registrationPromise;

        this.setState({status_modal_string: ""});
        const modal = util.async_modal_ref.current;
        if(!await modal.show({
                title: () => __("main.reboot_title"),
                body: () => __("main.reboot_content")(__("remote_access.script.reboot_content_changed")),
                no_text: () => __("main.abort"),
                yes_text: () => __("main.reboot"),
                no_variant: "secondary",
                yes_variant: "danger",
                nestingDepth: 1,
            }))
            return;

        util.reboot();
    }

    async registerCharger(cfg: util.NoExtraProperties<API.getType["remote_access/register"]["config"]>) {
        if (!cfg.enable) {
            const registration_data: util.NoExtraProperties<API.getType["remote_access/register"]> = {
                config: cfg,
                note: "",
                login_key: "",
                secret_key: "",
                mgmt_charger_public: "",
                mgmt_charger_private: "",
                mgmt_psk: "",
                keys: []
            };

            await API.call("remote_access/register", registration_data, () => __("remote_access.script.save_failed"), () => __("remote_access.script.reboot_content_changed"), 10000);
            return;
        }

        let loginSalt: Uint8Array;
        try {
            loginSalt = await this.get_login_salt(cfg);
        } catch (err) {
            console.error(err);
            util.add_alert("registration", "danger", () => "Failed to login:", () => "Wrong user or password");
            this.setState({status_modal_string: ""});
            return;
        }

        const loginHash = await hash({
            pass: this.state.addUser.password,
            salt: loginSalt,
            time: 2,
            mem: 19 * 1024,
            hashLen: 24,
            parallelism: 1,
            type: ArgonType.Argon2id
        });
        const loginKey = (await util.blobToBase64(new Blob([loginHash.hash]))).replace("data:application/octet-stream;base64,", "");

        try {
            await this.login({
                config: cfg,
                note: "",
                login_key: loginKey,
                secret_key: "",
                mgmt_charger_public: "",
                mgmt_charger_private: "",
                mgmt_psk: "",
                keys: []
            });
        } catch (err) {
            console.error(`Failed to login: ${err}`);

            // scrape the status code from the error message and only display wrong username or password in case it
            // really is an authorization error.
            const errString = err as string;
            const errCode = errString.substring(errString.lastIndexOf(" ") + 1);
            if (errCode === "401") {
                util.add_alert("registration", "danger", () => __("remote_access.content.login_failed"), () => __("remote_access.content.wrong_credentials"));
            } else {
                util.add_alert("registration", "danger", () => __("remote_access.content.login_failed"), () => errString);
            }
            this.setState({status_modal_string: ""});
            return;
        }

        let secretSalt: Uint8Array;
        try {
            secretSalt = await this.get_secret_salt(cfg);
        } catch (err) {
            console.error(`Failed to get secret salt: ${err}`);
            util.add_alert("registration", "danger", () => "Failed to get secret-salt:", () => err);
            this.setState({status_modal_string: ""});
            return;
        }

        const mg_charger_keypair = (window as any).wireguard.generateKeypair();

        const keys: util.NoExtraProperties<API.getType["remote_access/register"]["keys"]> = [];

        for (const i of util.range(0, 5)) {
            const charger_keypair = (window as any).wireguard.generateKeypair();
            const web_keypair = (window as any).wireguard.generateKeypair();

            const psk: string = (window as any).wireguard.generatePresharedKey();

            keys.push({
                charger_public: charger_keypair.publicKey,
                charger_private: charger_keypair.privateKey,
                web_public: web_keypair.publicKey,
                web_private: web_keypair.privateKey,
                psk: psk,
            });
        }

        const secret_key = await hash({
            pass: this.state.addUser.password,
            salt: secretSalt,
            // Takes about 1.5 seconds on a Nexus 4
            time: 2, // the number of iterations
            mem: 19 * 1024, // used memory, in KiB
            hashLen: 32, // desired hash length
            parallelism: 1, // desired parallelism (it won't be computed in parallel, however)
            type: ArgonType.Argon2id,
        })

        const secret_key_blob = new Blob([secret_key.hash]);
        const secret_key_string = await util.blobToBase64(secret_key_blob);

        const psk: string = (window as any).wireguard.generatePresharedKey();

        const registration_data: util.NoExtraProperties<API.getType["remote_access/register"]> = {
            config: cfg,
            note: this.state.addUser.note,
            login_key: this.state.login_key,
            secret_key: secret_key_string.replace("data:application/octet-stream;base64,", ""),
            mgmt_charger_public: mg_charger_keypair.publicKey,
            mgmt_charger_private: mg_charger_keypair.privateKey,
            mgmt_psk: psk,
            keys: keys
        };

        try {
            await this.runRegistration(registration_data);
        } catch (err) {
            console.error(`Failed to register charger: ${err}`);
            util.add_alert("registration", "danger", () => "Failed to register", () => err);
            this.setState({status_modal_string: ""});
        }
    }
    async runAddUser(user: add_user) {
        this.setState({status_modal_string: __("remote_access.content.registration")});
        const registrationPromise: Promise<void> = new Promise((resolve, reject) => {
            this.resolve = resolve;
            this.reject = reject;
        });

        await API.call("remote_access/add_user", user, () => __("remote_access.script.save_failed"));
        await registrationPromise;

        this.setState({status_modal_string: ""});
    }

    async addUser() {
        const cfg : util.NoExtraProperties<API.getType["remote_access/register"]["config"]> = {
            enable: this.state.enable,
            relay_host: this.state.relay_host,
            relay_port: this.state.relay_port,
            email: this.state.addUser.email,
            cert_id: this.state.cert_id,
        }

        let loginSalt: Uint8Array;
        try {
            loginSalt = await this.get_login_salt(cfg);
        } catch (err) {
            console.error(err);
            util.add_alert("registration", "danger", () => "Failed to login:", () => "Wrong user or password");
            this.setState({status_modal_string: ""});
            return;
        }

        const loginHash = await hash({
            pass: this.state.addUser.password,
            salt: loginSalt,
            time: 2,
            mem: 19 * 1024,
            hashLen: 24,
            parallelism: 1,
            type: ArgonType.Argon2id
        });
        const loginKey = (await util.blobToBase64(new Blob([loginHash.hash]))).replace("data:application/octet-stream;base64,", "");

        try {
            await this.login({
                config: cfg,
                login_key: loginKey,
                note: "",
                secret_key: "",
                mgmt_charger_public: "",
                mgmt_charger_private: "",
                mgmt_psk: "",
                keys: []
            });
        } catch (err) {
            console.error(`Failed to login: ${err}`);

            // scrape the status code from the error message and only display wrong username or password in case it
            // really is an authorization error.
            const errString = err as string;
            const errCode = errString.substring(errString.lastIndexOf(" ") + 1);
            if (errCode === "401") {
                util.add_alert("registration", "danger", () => __("remote_access.content.login_failed"), () => __("remote_access.content.wrong_credentials"));
            } else {
                util.add_alert("registration", "danger", () => __("remote_access.content.login_failed"), () => errString);
            }
            this.setState({status_modal_string: ""});
            return;
        }

        let secretSalt: Uint8Array;
        try {
            secretSalt = await this.get_secret_salt(cfg);
        } catch (err) {
            console.error(`Failed to get secret salt: ${err}`);
            util.add_alert("registration", "danger", () => "Failed to get secret-salt:", err);
            this.setState({status_modal_string: ""});
            return;
        }

        const keys: util.NoExtraProperties<API.getType["remote_access/register"]["keys"]> = [];

        for (const i of util.range(0, 5)) {
            const charger_keypair = (window as any).wireguard.generateKeypair();
            const web_keypair = (window as any).wireguard.generateKeypair();

            const psk: string = (window as any).wireguard.generatePresharedKey();

            keys.push({
                charger_public: charger_keypair.publicKey,
                charger_private: charger_keypair.privateKey,
                web_public: web_keypair.publicKey,
                web_private: web_keypair.privateKey,
                psk: psk,
            });
        }

        const secret_key = await hash({
            pass: this.state.addUser.password,
            salt: secretSalt,
            // Takes about 1.5 seconds on a Nexus 4
            time: 2, // the number of iterations
            mem: 19 * 1024, // used memory, in KiB
            hashLen: 32, // desired hash length
            parallelism: 1, // desired parallelism (it won't be computed in parallel, however)
            type: ArgonType.Argon2id,
        })

        const secret_key_blob = new Blob([secret_key.hash]);
        const secret_key_string = await util.blobToBase64(secret_key_blob);

        const add_user_data: add_user = {
            secret_key: secret_key_string.replace("data:application/octet-stream;base64,", ""),
            login_key: loginKey,
            note: this.state.addUser.note,
            email: this.state.addUser.email,
            wg_keys: keys,
        }

        try {
            await this.runAddUser(add_user_data);
        } catch (err) {
            console.error(`Failed to register charger: ${err}`);
            util.add_alert("registration", "danger", () => "Failed to register", () => err);
            this.setState({status_modal_string: ""});
        }
    }

    override async sendSave(t: "remote_access/config", cfg: config): Promise<void> {
        let enable = cfg.enable;
        for (const id of this.state.removeUsers) {
            API.call("remote_access/remove_user", {
                id: id,
            }, () => __("remote_access.script.save_failed"));
        }
        if (this.state.users.length === 0) {
            enable = false;
        }
        this.setState({removeUsers: []});
        const config: update_config = {
            enable: enable,
            relay_host: this.state.relay_host,
            relay_port: this.state.relay_port,
            email: "",
            cert_id: this.state.cert_id,
        }
        API.call("remote_access/update_config", config, () => __("remote_access.script.save_failed"), () => __("remote_access.script.reboot_content_changed"));
    }

    override getIsModified(topic: "remote_access/config"): boolean {
        return API.get(topic).enable;
    }

    override async sendReset(topic: "remote_access/config") {
        for (const user of this.state.users) {
            API.call("remote_access/remove_user", {
                id: user.id
            }, () => __("remote_access.script.save_failed"));
        }

        this.setState({removeUsers: []});
        const config: update_config = {
                enable: false,
            relay_host: this.state.relay_host,
            relay_port: this.state.relay_port,
                email: "",
                cert_id: this.state.cert_id,
        }
        API.call("remote_access/update_config", config, () => __("remote_access.script.save_failed"), () => __("remote_access.script.reboot_content_changed"));
    }

    checkUserExisting() {
        for (const user of this.state.users) {
            if (user.email.toLowerCase() === this.state.addUser.email.toLowerCase()) {
                return true;
            }
        }
        return false;
    }

    render() {
        if (!util.render_allowed())
            return <></>

        const cert_config = API.get("certs/state");
        const cert_items: [string, string][] = [["-1", __("remote_access.content.not_used")]];
        for (const cert of cert_config.certs) {
            cert_items.push([cert.id.toString(), cert.name]);
        }

        let users: TableRow[] = [];
        for (const user of this.state.users) {
            const row: TableRow = {
                columnValues: [user.email],
                onRemoveClick: async () => {
                    this.setState({removeUsers: this.state.removeUsers.concat(user.id)});
                    this.setState({users: this.state.users.filter((u) => u.id != user.id)});
                    this.setDirty(true);
                },
            }
            users.push(row);
        }

        return <>
            <Modal centered className="modal-1" backdropClassName="modal-backdrop-1" show={this.state.status_modal_string != ""}>
                <Modal.Header {...{closeButton: false} as any}>
                    <Modal.Title>
                        {__("remote_access.content.status_modal_header")}
                    </Modal.Title>
                </Modal.Header>
                <Modal.Body>
                    <Container fluid>
                        <Row className="justify-content-center mb-2">
                            <Spinner animation="border" variant="primary"/>
                        </Row>
                        <Row className="justify-content-center">
                            <div>{this.state.status_modal_string}</div>
                        </Row>
                    </Container>
                </Modal.Body>
            </Modal>
            <SubPage name="remote_access">
                <ConfigForm id="remote_access_config_form"
                            title={__("remote_access.content.remote_access")}
                            isModified={this.isModified()}
                            isDirty={this.isDirty()}
                            onReset={this.reset}
                            onSave={this.save}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("remote_access.content.enable")}>
                        <Switch checked={this.state.enable}
                                desc={__("remote_access.content.enable_desc")(this.state.relay_host)}
                                onClick={() => {
                                    this.setState({enable: !this.state.enable});
                                }} />
                    </FormRow>
                    <FormRow label={__("remote_access.content.user")}>
                        <Table columnNames={[__("remote_access.content.email")]}
                            rows={users}
                            addEnabled={users.length < 5 && this.state.enable}
                            addTitle={__("remote_access.content.add_user")}
                            onAddShow={async () => this.setState({addUser: {email: "", password: "", note: ""}})}
                            addMessage={users.length < 5 ? __("remote_access.content.user_add_message")(users.length, 5) : __("remote_access.content.all_users_in_use")}
                            onAddGetChildren={() => {
                                return <>
                                    <FormRow label={__("remote_access.content.email")}>
                                        <InputText value={this.state.addUser.email} required
                                            maxLength={64}
                                            class={this.checkUserExisting() ? "is-invalid" : undefined}
                                            invalidFeedback={__("remote_access.content.user_exists")}
                                            onValue={(v) => {
                                                        this.setState({addUser: {...this.state.addUser, email: v}})
                                            }} />
                                    </FormRow>
                                    <FormRow label={__("remote_access.content.password")} label_muted={__("remote_access.content.password_muted")}>
                                        <InputPassword required
                                            value={this.state.addUser.password}
                                            maxLength={64}
                                            onValue={(v) => {
                                                    this.setState({addUser: {...this.state.addUser, password: v}})
                                            }}
                                            hideClear
                                            placeholder="" />
                                    </FormRow>
                                    <FormRow label={__("remote_access.content.note")} label_muted={__("remote_access.content.note_muted")(this.state.relay_host)}>
                                        <InputText value={this.state.addUser.note} onValue={(v) => this.setState({addUser: {...this.state.addUser, note: v}})}/>
                                    </FormRow>
                                </>
                            }}
                            onAddSubmit={async () => {
                                if (users.length === 0) {
                                    const config: util.NoExtraProperties<API.getType["remote_access/register"]["config"]> = {
                                        enable: this.state.enable,
                                        email: this.state.addUser.email,
                                        relay_host: this.state.relay_host,
                                        relay_port: this.state.relay_port,
                                        cert_id: this.state.cert_id,
                                    };
                                    await this.registerCharger(config);
                                } else {
                                    await this.addUser();
                                }
                            }}
                        />
                    </FormRow>
                    <CollapsedSection heading={__("remote_access.content.advanced_settings")}>
                        <FormRow label={__("remote_access.content.relay_host")} label_muted={__("remote_access.content.relay_host_muted")}>
                            <InputText required
                                    maxLength={64}
                                    value={this.state.relay_host}
                                    onValue={(v) => {
                                            this.setState({relay_host: v})
                                    }} />
                        </FormRow>
                        <FormRow label={__("remote_access.content.relay_port")}>
                            <InputNumber required
                                        min={1}
                                        max={65565}
                                        value={this.state.relay_port}
                                        onValue={v => this.setState({relay_port: v})} />
                        </FormRow>
                        <FormRow label={__("remote_access.content.cert")}>
                            <InputSelect items={cert_items} value={this.state.cert_id} onValue={(v) => {
                                this.setState({cert_id: parseInt(v)});
                            }}/>
                        </FormRow>
                    </CollapsedSection>
                </ConfigForm>
            </SubPage>
        </>
    }
}


export function init() {}
