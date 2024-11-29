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
import YaMD5 from "../../ts/yamd5";
import { h, Fragment } from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent, ConfigComponentState } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputFloat } from "../../ts/components/input_float";
import { Switch } from "../../ts/components/switch";
import { InputPassword } from "../../ts/components/input_password";
import { EVSE_SLOT_USER } from "../evse_common/api";
import { SubPage } from "../../ts/components/sub_page";
import { Table } from "../../ts/components/table";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Slash, Check, Users as UsersSymbol } from "react-feather";

export function UsersNavbar() {
    return <NavbarItem name="users" module="users" title={__("users.navbar.users")} symbol={<UsersSymbol />} />;
}

type User = (API.getType['users/config']['users'][0]) & {password: string, is_invalid: number};
type UsersConfig = Omit<API.getType['users/config'], 'users'> & {users: User[]};

interface UsersState {
    userSlotEnabled: boolean;
    addUser: User;
    editUser: User;
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
    return retry_once(() => API.call("users/remove", {"id": id}, () => __("users.script.save_failed")), "users_remove_failed");
}

function modify_user(user: User) {
    let {password, is_invalid, ...u} = user;
    return retry_once(() => API.call("users/modify", u, () => __("users.script.save_failed")), "users_modify_failed");
}

function modify_unknown_user(name: string) {
    return retry_once(() => API.call("users/modify",
                                    {"id": 0,
                                     "display_name": name,
                                     "username": null,
                                     "current": null,
                                     "digest_hash": null,
                                     "roles": null},
                                     () => __("users.script.save_failed")),
                                     "users_modify_failed");
}

function add_user(user: User) {
    let {password, is_invalid, ...u} = user;
    return retry_once(() => API.call("users/add", u, () => __("users.script.save_failed")), "users_add_failed");
}

export class Users extends ConfigComponent<'users/config', {}, UsersState> {
    constructor() {
        super('users/config',
              () => __("users.script.save_failed"),
              () => __("users.script.reboot_content_changed"), {
                userSlotEnabled: false,
                addUser: {
                    id: -1,
                    roles: 0xFFFF,
                    username: "",
                    display_name: "",
                    current: 32000,
                    digest_hash: "",
                    password: "",
                    is_invalid: 0,
                },
                editUser: {
                    id: -1,
                    roles: 0xFFFF,
                    username: "",
                    display_name: "",
                    current: 32000,
                    digest_hash: "",
                    password: "",
                    is_invalid: 0,
                },
            },
        );

        util.addApiEventListener('evse/slots', () => {
            this.setState({userSlotEnabled: API.get('evse/slots')[EVSE_SLOT_USER].active});
        });
    }

    isChangedUser(changed_user: User): boolean {
        let users = API.get("users/config").users;

        for (let user of users) {
            if (user.username == changed_user.username && user.id == changed_user.id)
                return false;
        }

        return true;
    }

    override async isSaveAllowed(cfg: UsersConfig): Promise<boolean> {
        let all_usernames = await getAllUsernames();
        let save_allowed = true;
        let new_users = cfg.users.slice();

        for (let i = 0; i < cfg.users.length; i++) {
            new_users[i].is_invalid = 0;

            for (let a = 0; a < cfg.users.length; a++) {
                if (this.isChangedUser(cfg.users[i]) &&  cfg.users[i].username == cfg.users[a].username && a != i) {
                    new_users[i].is_invalid = 1;
                    this.setState({users: new_users});
                    save_allowed = false;
                    break;
                }
                else if (this.isChangedUser(cfg.users[i]) && cfg.next_user_id == 0) {
                    new_users[i].is_invalid = 3;
                    this.setState({users: new_users});
                    save_allowed = false;
                    break;
                }
            }

            for (let user of all_usernames[0]) {
                if (this.isChangedUser(cfg.users[i]) && cfg.users[i].username == user &&
                        (cfg.users[i].is_invalid == undefined || cfg.users[i].is_invalid == 0)) {
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
        await API.call_unchecked('users/http_auth_update', {
            "enabled": enabled
        },
        () => __("users.script.save_failed"));
    }

    user_has_password(u: User) {
        return (u.digest_hash == null &&    // user is known and has password set, which is censored
                u.password !== "") ||       // password will not be removed, an empty string as password would mean to remove the password
               (u.digest_hash == "" &&      // user is new and/or has currently no password set
                u.password !== undefined &&
                u.password !== null &&      // password will be changed/set
                u.password !== "")          // password will not be removed, an empty string as password would mean to remove the password
    }

    get_password_replacement(u: User) {
        if (!this.user_has_password(u)) {
            return '';
        }

        if (u.password !== undefined && u.password !== null) {
            return '\u2022'.repeat(u.password.length);
        }

        return <span style="color: rgb(85,85,85);">{__("component.input_password.unchanged")}</span>;
    }

    http_auth_allowed() {
        return (this.state as Readonly<UsersState & UsersConfig & ConfigComponentState>).users.some(u => this.user_has_password(u))
    }

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

        for (let i of ids_to_remove) {
            await remove_user(i);
        }

        for (let u of users_to_modify) {
            // Don't hash if u.password is falsy, i.e. null, undefined or the empty string
            u.digest_hash = u.password ? YaMD5.YaMD5.hashStr(u.username + ":esp32-lib:" + u.password) : u.password
            // Always send digest_hash, but as null if we don't want to change it.
            // digest_hash can be undefined if this user was not modified.
            if (u.digest_hash === undefined)
                u.digest_hash = null;
            await modify_user(u);
        }

        let next_user_id = API.get('users/config').next_user_id;

        outer_loop:
        for (let u of users_to_add) {
            // Don't hash if u.password is falsy, i.e. null, or the empty string.
            // u.password can't be undefined (as is handled above when modifying users),
            // because adding a user sets password to "" if nothing was entered.
            u.digest_hash = u.password ? YaMD5.YaMD5.hashStr(u.username + ":esp32-lib:" + u.password) : u.password
            u.id = next_user_id;
            await add_user(u);
            for (let i = 0; i < 20; ++i) {
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

        await API.save('evse/user_enabled', {"enabled": this.state.userSlotEnabled}, () => __("evse.script.save_failed"), () => __("users.script.reboot_content_changed"));
    }

    setUser(i: number, val: Partial<User>) {
        // We have to copy the users array here to make sure the change detection in sendSave works.
        let users = this.state.users.slice(0);

        users[i] = {...users[i], ...val};
        this.setState({users: users});
    }

    override async sendReset(t: "users/config") {
        let new_users = this.state.users.slice(0);
        new_users = [new_users[0]];
        new_users[0].display_name = "";
        let new_state = {...this.state, users: [new_users[0]], userSlotEnabled: false, http_auth_enabled: false};

        this.setState(new_state, this.save);
    }

    override getIsModified(t: "users/config"): boolean {
        if (this.state.users.length > 1 || this.state.users[0].display_name != "Anonymous")
            return true;
        return false;
    }

    async checkUsername(user: User, ignore_i: number): Promise<number> {
        for (let i = 0; i < this.state.users.length; ++i) {
            if (i != ignore_i && this.state.users[i].username.trim() == user.username.trim()) {
                return 1;
            }
        }

        if (API.get('users/config').next_user_id == 0) {
            return 3;
        }

        if (this.isChangedUser(user)) {
            let all_usernames = await getAllUsernames();

            for (let i = 0; i < all_usernames[0].length; ++i) {
                if (all_usernames[0][i].trim() == user.username.trim()) {
                    return 2;
                }
            }
        }

        return 0;
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

    override render(props: {}, state: UsersConfig & UsersState & ConfigComponentState) {
        if (!util.render_allowed())
            return <SubPage name="users" />;

        const MAX_ACTIVE_USERS = API.hasModule("esp32_ethernet_brick") ? 33 : 17;

        let auth_allowed = this.http_auth_allowed();

        // Only allow enabling the user slot if there are at least two users (anonymous counts as one)
        let user_slot_allowed = state.users.length > 1;

        return (
            <SubPage name="users">
                <ConfigForm id="users_config_form" title={__("users.content.users")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save}
                    onReset={this.reset}
                    onDirtyChange={this.setDirty}>
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

                        <Table columnNames={[__("users.content.table_username"), __("users.content.table_display_name"), __("users.content.table_current"), __("users.content.table_password")]}
                            rows={state.users.slice(1).map((user, i) =>
                                { return {
                                    columnValues: [
                                        user.username,
                                        user.display_name,
                                        util.toLocaleFixed(user.current / 1000, 3) + ' A',
                                        this.user_has_password(user) ? <Check/> : ''
                                    ],
                                    fieldValues: [
                                        user.username,
                                        user.display_name,
                                        util.toLocaleFixed(user.current / 1000, 3) + ' A',
                                        this.user_has_password(user) ? this.get_password_replacement(user) : <span style="color: rgb(85,85,85);">{__("users.script.login_disabled")}</span>
                                    ],
                                    editTitle: __("users.content.edit_user_title"),
                                    onEditShow: async () => this.setState({editUser: {id: user.id, roles: user.roles, username: user.username, display_name: user.display_name, current: user.current, digest_hash: user.digest_hash, password: user.password, is_invalid: user.is_invalid}}),
                                    onEditGetChildren: () => [<>
                                        <FormRow label={__("users.content.edit_user_username")}>
                                            <InputText
                                                value={state.editUser.username}
                                                onValue={(v) => this.setState({editUser: {...state.editUser, username: v}})}
                                                minLength={1}
                                                maxLength={32}
                                                required
                                                class={state.editUser.is_invalid != undefined && state.editUser.is_invalid != 0 ? "is-invalid" : ""}
                                                invalidFeedback={this.errorMessage(state.editUser)} />
                                        </FormRow>
                                        <FormRow label={__("users.content.edit_user_display_name")}>
                                            <InputText
                                                value={state.editUser.display_name}
                                                onValue={(v) => this.setState({editUser: {...state.editUser, display_name: v}})}
                                                minLength={1}
                                                maxLength={32}
                                                required />
                                        </FormRow>
                                        <FormRow label={__("users.content.edit_user_current")}>
                                            <InputFloat
                                                unit="A"
                                                value={state.editUser.current}
                                                onValue={(v) => this.setState({editUser: {...state.editUser, current: v}})}
                                                digits={3}
                                                min={6000}
                                                max={32000} />
                                        </FormRow>
                                        <FormRow label={__("users.content.edit_user_password")}>
                                            <InputPassword
                                                required={this.require_password(state.editUser)}
                                                maxLength={64}
                                                value={state.editUser.password === undefined ? state.editUser.digest_hash : state.editUser.password}
                                                onValue={(v) => this.setState({editUser: {...state.editUser, password: v}})}
                                                clearPlaceholder={__("users.script.login_disabled")}
                                                clearSymbol={<Slash/>}
                                                allowAPIClear />
                                        </FormRow>
                                    </>],
                                    onEditCheck: async () => {
                                        let is_invalid = await this.checkUsername(state.editUser, i + 1);

                                        return new Promise<boolean>((resolve) => {
                                            this.setState({editUser: {...state.editUser, is_invalid: is_invalid}}, () => resolve(is_invalid == undefined || is_invalid == 0));
                                        });
                                    },
                                    onEditSubmit: async () => {
                                        this.setUser(i + 1, state.editUser);
                                        this.setDirty(true);
                                    },
                                    onRemoveClick: async () => {
                                        this.setState({users: state.users.filter((v, idx) => idx != i + 1)});
                                        this.setDirty(true);
                                    }}
                                })
                            }
                            addEnabled={API.get('users/config').next_user_id != 0 && state.users.length < MAX_ACTIVE_USERS}
                            addTitle={__("users.content.add_user_title")}
                            // One user slot is always taken by the unknown user, so display MAX_ACTIVE_USERS - 1 as the maximum number of users that can be added.
                            addMessage={API.get('users/config').next_user_id == 0 ? __("users.content.add_user_user_ids_exhausted") : __("users.content.add_user_prefix") + (state.users.length - 1) + __("users.content.add_user_infix") + (MAX_ACTIVE_USERS - 1) + __("users.content.add_user_suffix")}
                            onAddShow={async () => this.setState({addUser: {id: -1, roles: 0xFFFF, username: "", display_name: "", current: 32000, digest_hash: "", password: "", is_invalid: 0}})}
                            onAddGetChildren={() => [<>
                                <FormRow label={__("users.content.add_user_username")}>
                                    <InputText
                                        value={state.addUser.username}
                                        onValue={(v) => this.setState({addUser: {...state.addUser, username: v}})}
                                        required
                                        minLength={1}
                                        maxLength={32}
                                        placeholder={__("users.content.add_user_username_desc")}
                                        class={state.addUser.is_invalid != undefined && state.addUser.is_invalid != 0 ? "is-invalid" : ""}
                                        invalidFeedback={this.errorMessage(state.addUser)} />
                                </FormRow>
                                <FormRow label={__("users.content.add_user_display_name")}>
                                    <InputText
                                        value={state.addUser.display_name}
                                        onValue={(v) => this.setState({addUser: {...state.addUser, display_name: v}})}
                                        required
                                        minLength={1}
                                        maxLength={32}
                                        placeholder={__("users.content.add_user_display_name_desc")} />
                                </FormRow>
                                <FormRow label={__("users.content.add_user_current")}>
                                    <InputFloat
                                        unit="A"
                                        value={state.addUser.current}
                                        onValue={(v) => this.setState({addUser: {...state.addUser, current: v}})}
                                        digits={3}
                                        min={6000}
                                        max={32000} />
                                </FormRow>
                                <FormRow label={__("users.content.add_user_password")}>
                                    <InputPassword
                                        maxLength={64}
                                        value={state.addUser.password}
                                        onValue={(v) => this.setState({addUser: {...state.addUser, password: v}})}
                                        hideClear
                                        placeholder={__("users.content.add_user_password_desc")} />
                                </FormRow>
                            </>]}
                            onAddCheck={async () => {
                                let is_invalid = await this.checkUsername(state.addUser, undefined);

                                return new Promise<boolean>((resolve) => {
                                    this.setState({addUser: {...state.addUser, is_invalid: is_invalid}}, () => resolve(is_invalid == undefined || is_invalid == 0));
                                });
                            }}
                            onAddSubmit={async () => {
                                this.setState({users: state.users.concat({...state.addUser, id: -1, roles: 0xFFFF})});
                                this.setDirty(true);
                            }}
                        />
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

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
            for (let i = 0; i < 256; ++i) {
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
