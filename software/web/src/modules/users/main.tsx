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

import $ from "../../ts/jq";

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import YaMD5 from "../../ts/yamd5";


import { h, render, Fragment } from "preact";
import { __ } from "../../ts/translation";

import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { FormGroup } from "../../ts/components/form_group";
import { InputText } from "../../ts/components/input_text";
import { Button, Card, Modal } from "react-bootstrap";
import { InputFloat } from "src/ts/components/input_float";
import { Switch } from "src/ts/components/switch";
import { InputPassword } from "src/ts/components/input_password";
import { Slash, User, UserPlus, UserX } from "react-feather";
import { EVSE_SLOT_USER } from "../evse_common/api";
import { ItemModal } from "src/ts/components/item_modal";

const MAX_ACTIVE_USERS = 16;

type User = (API.getType['users/config']['users'][0]) & {password: string, is_invalid: number};
type UsersConfig = Omit<API.getType['users/config'], 'users'> & {users: User[]};

interface UsersState {
    userSlotEnabled: boolean
    showModal: boolean
    newUser: User
}

 // This is a bit hacky: the user modification API can take some time because it writes the changed user/display name to flash
// The API will block up to five seconds, but just to be sure we try this twice.
function retry_once<T>(fn: () => Promise<T>, topic: string) {
    return fn().catch(() => {
        util.remove_alert(topic);
        return fn();
    });
}

function remove_user(id: number) {
    return retry_once(() => API.call("users/remove", {"id": id}, __("users.script.save_failed")), "users_remove_failed");
}

function modify_user(user: User) {
    let {password, is_invalid, ...u} = user;
    return retry_once(() => API.call("users/modify", u, __("users.script.save_failed")), "users_modify_failed");
}

function modify_unknown_user(name: string) {
    return retry_once(() => API.call("users/modify",
                                    {"id": 0,
                                     "display_name": name,
                                     "username": null,
                                     "current": null,
                                     "digest_hash": null,
                                     "roles": null},
                                    __("users.script.save_failed")),
                      "users_modify_failed");
}

function add_user(user: User) {
    let {password, is_invalid, ...u} = user;
    return retry_once(() => API.call("users/add", u, __("users.script.save_failed")), "users_add_failed");
}

export class Users extends ConfigComponent<'users/config', {}, UsersState> {
    constructor() {
        super('users/config',
              __("users.script.save_failed"),
              __("users.script.reboot_content_changed"));

        this.state = {userSlotEnabled: false, showModal: false, newUser: {id: 0, roles: 0xFFFF, username: "", display_name: "", current: 32000, digest_hash: "", password: ""}} as any;

        util.addApiEventListener('evse/slots', () => {
            this.setState({userSlotEnabled: API.get('evse/slots')[EVSE_SLOT_USER].active});
        });
    }

    isChangedUser(changed_user: User): boolean {
        let users = API.get("users/config").users;

        for (let user of users)
        {
            if (user.username == changed_user.username && user.id == changed_user.id)
                return false;
        }
        return true;
    }

    override async isSaveAllowed(cfg: UsersConfig): Promise<boolean> {
            let all_usernames = await getAllUsernames();
            let save_allowed = true;
            let new_users = cfg.users.slice();
            for (let i = 0; i < cfg.users.length; i++)
            {
                new_users[i].is_invalid = 0;
                for (let a = 0; a < cfg.users.length; a++)
                {
                    if (this.isChangedUser(cfg.users[i]) &&  cfg.users[i].username == cfg.users[a].username && a != i)
                    {
                        new_users[i].is_invalid = 1;
                        this.setState({users: new_users});
                        save_allowed = false;
                        break;
                    }
                    else if (this.isChangedUser(cfg.users[i]) && cfg.next_user_id == 0)
                    {
                        new_users[i].is_invalid = 3;
                        this.setState({users: new_users});
                        save_allowed = false;
                        break;
                    }
                }
                for (let user of all_usernames[0])
                {
                    if (this.isChangedUser(cfg.users[i]) && cfg.users[i].username == user &&
                            (cfg.users[i].is_invalid == undefined || cfg.users[i].is_invalid == 0))
                    {
                        new_users[i].is_invalid = 2;
                        this.setState({users: new_users});
                        save_allowed = false;
                        break;
                    }
                }
            }
            return save_allowed;
        }


    async save_authentication_config(enabled: boolean) {
        await API.save('users/http_auth', {
            "enabled": enabled
        },
        __("users.script.save_failed"),
        __("users.script.reboot_content_changed"));
    }

    http_auth_allowed() {
        return (this.state as Readonly<UsersState & UsersConfig>).users.some(u => (u.digest_hash == null && (u.password !== "")) ||
                                                                                  (u.digest_hash == "" && u.password !== undefined && u.password !== null && u.password !== ""))
    };

    override async sendSave(t: "users/config", new_config: UsersConfig) {
        let old_config = API.get('users/config');
        new_config.http_auth_enabled &&= this.http_auth_allowed();
        if (old_config.http_auth_enabled && !new_config.http_auth_enabled) {
            // If we want to disable authentication, do this first,
            // to make sure authentication is never enabled
            // while no user without password is configured.
            await this.save_authentication_config(new_config.http_auth_enabled);
        }

        if (new_config.users[0].display_name == __("charge_tracker.script.unknown_user"))
            new_config.users[0].display_name = "Anonymous"

        if (new_config.users[0].display_name === "")
            await modify_unknown_user("Anonymous");
        else if (new_config.users[0].display_name != old_config.users[0].display_name)
            await modify_unknown_user(new_config.users[0].display_name);

        let ids_to_remove = old_config.users.slice(1).filter(uOld => !new_config.users.slice(1).some(uNew => uOld.id == uNew.id)).map(uOld => uOld.id);
        let users_to_modify = new_config.users.slice(1).filter(uNew => old_config.users.slice(1).some(uOld => uNew.id == uOld.id));
        let users_to_add = new_config.users.slice(1).filter(uNew => !old_config.users.slice(1).some(uOld => uNew.id == uOld.id));

        for(let i of ids_to_remove) {
            await remove_user(i);
        }

        for(let u of users_to_modify) {
            u.digest_hash = (u.password != null && u.password != "") ? YaMD5.YaMD5.hashStr(u.username + ":esp32-lib:" + u.password) : u.password
            await modify_user(u);
        }

        let next_user_id = API.get('users/config').next_user_id;

        outer_loop:
        for(let u of users_to_add) {
            u.digest_hash = (u.password != null && u.password != "") ? YaMD5.YaMD5.hashStr(u.username + ":esp32-lib:" + u.password) : u.password
            u.id = next_user_id;
            await add_user(u);
            for(let i = 0; i < 20; ++i) {
                if (API.get('users/config').next_user_id != next_user_id) {
                    next_user_id = API.get('users/config').next_user_id;
                    continue outer_loop;
                }
                await util.wait(100)
            }
            // fallback: just assume the next id is free
            next_user_id = Math.max(1, (next_user_id + 1) % 256);
        }

        await this.save_authentication_config(new_config.http_auth_enabled);

        await API.save_maybe('evse/user_enabled', {"enabled": this.state.userSlotEnabled}, __("evse.script.save_failed"));
    }


    setUser(i: number, val: Partial<User>) {
        // We have to copy the users array here to make sure the change detection in sendSave works.
        let users = this.state.users.slice(0);

        users[i] = {...users[i], ...val};
        this.setState({users: users});
    }

    hackToAllowSave() {
        document.getElementById("users_config_form").dispatchEvent(new Event('input'));
    }

    override async sendReset(t: "users/config"){
        let new_users = this.state.users.slice(0);
        new_users = [new_users[0]];
        new_users[0].display_name = "";
        let new_state = {...this.state, users: [new_users[0]], userSlotEnabled: false, http_auth_enabled: false};

        this.setState(new_state, this.save);
    }

    override getIsModified(t: "users/config"): boolean {
        if (this.state.users.length > 1 || this.state.users[0].display_name != "Anonymous")
            return true;
        return false
    }

    errorMessage(user: User): string {
        switch (user.is_invalid) {
            case 1:
                return __("users.script.username_already_used");
            case 2:
                return __("users.script.username_already_tracked");
            case 3:
                return __("users.script.all_user_ids_in_use");
            default:
                return undefined;
        }
    }

    require_password(user: User): boolean {
        let lst = API.get("users/config").users.filter(u => u.id == user.id);
        if (lst.length != 1)
            return false;

        let configured = lst[0];
        if (configured.digest_hash === "")
            return false;

        return user.username != configured.username;
    }

    override render(props: {}, state: UsersConfig & UsersState) {
        if (!util.allow_render)
            return <></>

        let addUserCard = this.state.next_user_id != 0 ? <div class="col mb-4">
                <Card className="h-100" key={999}>
                <div class="card-header d-flex justify-content-between align-items-center">
                    <UserPlus/>
                    <Button variant="outline-dark" size="sm" style="visibility: hidden;">
                        <UserX/><span class="ml-2" style="font-size: 1rem; vertical-align: middle;">{__("users.script.delete")}</span>
                    </Button>
                </div>
                <Card.Body>
                    {state.users.length >= MAX_ACTIVE_USERS
                        ? <span>{__("users.script.add_user_disabled_prefix") + MAX_ACTIVE_USERS + __("users.script.add_user_disabled_suffix")}</span>
                        : <Button variant="light" size="lg" block style="height: 100%;" onClick={() => this.setState({showModal: true})}>{__("users.script.add_user")}</Button>}
                </Card.Body>
            </Card>
        </div> : <></>

        let auth_allowed = this.http_auth_allowed();

        // Only allow enabling the user slot if there are at least two users (anonymous counts as one)
        let user_slot_allowed = state.users.length > 1;

        return (
            <>
                <ConfigForm id="users_config_form" title={__("users.content.users")} isModified={this.isModified()} onSave={async () =>{this.save();}}
                    onReset={this.reset}
                    onDirtyChange={(d) => this.ignore_updates = d}>
                    <FormRow label={__("users.content.enable_authentication")}>
                        <Switch desc={__("users.content.enable_authentication_desc")}
                                checked={auth_allowed && state.http_auth_enabled}
                                onClick={this.toggle("http_auth_enabled")}
                                disabled={!auth_allowed}
                                className={!auth_allowed && state.http_auth_enabled ? "is-invalid" : ""}
                        />
                        <div class="invalid-feedback">{__("users.content.enable_authentication_invalid")}</div>
                    </FormRow>

                    <FormRow label={__("users.content.evse_user_description")} label_muted={__("users.content.evse_user_description_muted")}>
                        <Switch desc={__("users.content.evse_user_enable")}
                                checked={user_slot_allowed && state.userSlotEnabled}
                                disabled={!user_slot_allowed}
                                className={!user_slot_allowed && state.userSlotEnabled ? "is-invalid" : ""}
                                onClick={this.toggle("userSlotEnabled")}/>
                        <div class="invalid-feedback">{__("users.content.evse_user_enable_invalid")}</div>
                    </FormRow>

                    <FormRow label={__("users.content.unknown_username")}>
                        <InputPassword maxLength={32}
                                       value={state.users[0].display_name == "Anonymous" ? __("charge_tracker.script.unknown_user") : state.users[0].display_name}
                                       onValue={(v) => this.setUser(0, {display_name: v})}
                                       showAlways
                                       />
                    </FormRow>

                    <FormRow label={__("users.content.authorized_users")}>
                        <div class="row row-cols-1 row-cols-md-2">
                        {state.users.slice(1).map((user, i) => (
                            <div class="col mb-4">
                            <Card className="h-100" key={user.id}>
                                <div class="card-header d-flex justify-content-between align-items-center">
                                    <User/>
                                    <Button variant="outline-dark" size="sm"
                                            onClick={() => {
                                                this.setState({users: state.users.filter((v, user_i) => user_i != (i + 1))});
                                                this.hackToAllowSave();} }>
                                        <UserX/><span class="ml-2" style="font-size: 1rem; vertical-align: middle;">{__("users.script.delete")}</span>
                                    </Button>
                                </div>
                                <Card.Body>
                                    <FormGroup label={__("users.script.username")}>
                                        <InputText value={user.username}
                                                   onValue={(v) => {this.setUser(i+1, {username: v});}}
                                                   minLength={1} maxLength={32}
                                                   required
                                                   class={state.users[i + 1].is_invalid != undefined && state.users[i + 1].is_invalid != 0 ? "is-invalid" : ""}
                                                   invalidFeedback={this.errorMessage(state.users[i + 1])}/>
                                    </FormGroup>
                                    <FormGroup label={__("users.script.display_name")}>
                                        <InputText value={user.display_name}
                                                   onValue={(v) => this.setUser(i+1, {display_name: v})}
                                                   minLength={1} maxLength={32}
                                                   required/>
                                    </FormGroup>
                                    <FormGroup label={__("users.script.current")}>
                                        <InputFloat
                                            unit="A"
                                            value={user.current}
                                            onValue={(v) => this.setUser(i+1, {current: v})}
                                            digits={3}
                                            min={6000}
                                            max={32000}
                                            />
                                    </FormGroup>
                                    <FormGroup label={__("users.script.password")}>
                                        <InputPassword
                                            required={this.require_password(user)}
                                            maxLength={64}
                                            value={user.password === undefined ? user.digest_hash : user.password}
                                            onValue={(v) => this.setUser(i+1, {password: v})}
                                            clearPlaceholder={__("users.script.login_disabled")}
                                            clearSymbol={<Slash/>}
                                            allowAPIClear/>
                                    </FormGroup>
                                </Card.Body>
                            </Card>
                            </div>
                        )).concat(addUserCard)}
                        </div>
                    </FormRow>
                </ConfigForm>

                <ItemModal show={state.showModal}
                    onHide={() => this.setState({showModal: false})}
                    onSubmit={() => {this.setState({showModal: false,
                        users: state.users.concat({...state.newUser, id: -1, roles: 0xFFFF}),
                        newUser: {id: -1, roles: 0xFFFF, username: "", display_name: "", current: 32000, digest_hash: "", password: "", is_invalid: 0}});
                        this.hackToAllowSave();}}
                    title={__("users.content.add_user_modal_title")}
                    no_variant={"secondary"}
                    yes_variant={"primary"}
                    no_text={__("users.content.add_user_modal_abort")}
                    yes_text={__("users.content.add_user_modal_save")}>
                        <FormGroup label={__("users.content.add_user_modal_username")}>
                            <InputText
                                value={state.newUser.username}
                                onValue={(v) => this.setState({newUser: {...state.newUser, username: v}})}
                                required
                                maxLength={32}
                                placeholder={__("users.content.add_user_modal_username_desc")}
                                />
                        </FormGroup>
                        <FormGroup label={__("users.content.add_user_modal_display_name")}>
                            <InputText
                                value={state.newUser.display_name}
                                onValue={(v) => this.setState({newUser: {...state.newUser, display_name: v}})}
                                required
                                maxLength={32}
                                placeholder={__("users.content.add_user_modal_display_name_desc")}
                                />
                        </FormGroup>
                        <FormGroup label={__("users.content.add_user_modal_current")}>
                            <InputFloat
                                    unit="A"
                                    value={state.newUser.current}
                                    onValue={(v) => this.setState({newUser: {...state.newUser, current: v}})}
                                    digits={3}
                                    min={6000}
                                    max={32000}
                                    />
                        </FormGroup>
                        <FormGroup label={__("users.content.add_user_modal_password")}>
                            <InputPassword
                                maxLength={64}
                                value={state.newUser.password}
                                onValue={(v) => this.setState({newUser: {...state.newUser, password: v}})}
                                hideClear
                                placeholder={__("users.content.add_user_modal_password_desc")}
                                />
                        </FormGroup>
                </ItemModal>
            </>
        )
    }
}

render(<Users/>, $('#users')[0])

export function getAllUsernames() {
    return util.download('/users/all_usernames')
        .then(blob => blob.arrayBuffer())
        .then(buffer => {
            let usernames: string[] = [];
            let display_names: string[] = [];

            if (buffer.byteLength != 256 * 64) {
                console.log("Unexpected length of all_usernames!");
                return [null, null];
            }

            const decoder = new TextDecoder("utf-8");
            for(let i = 0; i < 256; ++i) {
                let view = new DataView(buffer, i * 64, 32);
                let username = decoder.decode(view).replace(/\0/g, "");

                view = new DataView(buffer, i * 64 + 32, 32);
                let display_name = decoder.decode(view).replace(/\0/g, "");

                usernames.push(username);
                display_names.push(display_name);
            }
            return [usernames, display_names];
        });
}

export function init() {

}

export function add_event_listeners(source: API.APIEventTarget) {
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-users').prop('hidden', !module_init.users);
}
