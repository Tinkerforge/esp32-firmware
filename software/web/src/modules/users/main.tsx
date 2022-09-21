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

import feather from "../../ts/feather";

import YaMD5 from "../../ts/yamd5";

import { h, render } from "preact";
import { __ } from "../../ts/translation";
import { ConfigPageHeader } from "../../ts/components/config_page_header";

render(<ConfigPageHeader prefix="users" title={__("users.content.users")} />, $('#users_header')[0]);

const MAX_ACTIVE_USERS = 16;

type UsersConfig = API.getType['users/config'];
type User = UsersConfig['users'][0];

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

            const decoder = new TextDecoder();
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

function save_authentication_config() {
    return API.save('users/http_auth', {
            "enabled": $('#users_authentication_enable').is(':checked')
        },
        __("users.script.save_failed"),
        __("users.script.reboot_content_changed"));
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
    return retry_once(() => API.call("users/modify", user, __("users.script.save_failed")), "users_modify_failed");
}

function modify_unknown_user(name: string) {
    return retry_once(() => API.call("users/modify", {  "id": 0,
                                                        "display_name": name,
                                                        "username": null,
                                                        "current": null,
                                                        "digest_hash": null,
                                                        "roles": null}, __("users.script.save_failed")), "users_modify_failed");
}

let next_user_id = 0;

function add_user(user: User) {
    user.id = next_user_id;
    ++next_user_id;
    return retry_once(() => API.call("users/add", user, __("users.script.save_failed")), "users_add_failed");
}

let authorized_users_count = -1;

function user_unmodified(user: User) {
    let users_config = API.get('users/config');
    for(let old_user of users_config.users) {
        if (old_user.id != user.id)
            continue;
        return old_user.current == user.current
            && old_user.display_name == user.display_name
            && old_user.roles == user.roles
            && old_user.username == user.username
            // digest_hash of old user will only be "" if no password is set. Otherwise it's null.
            // If the hash of user is "" we clear the password, if it is null we don't modify it.
            // So checking for equality works.
            && (old_user.digest_hash == user.digest_hash);
    }

    return false;
}

async function save_users_config() {
    let users_config = API.get('users/config');
    if (users_config.http_auth_enabled && !$('#users_authentication_enable').is(':checked')) {
        // If we want to disable authentication, do this first,
        // to make sure authentication is never enabled
        // while no user without password is configured.
        await save_authentication_config();
    }

    let new_unknown_username = $('#users_unknown_username').val().toString();
    if (new_unknown_username == __("charge_tracker.script.unknown_user"))
        new_unknown_username = "Anonymous"

    if (util.passwordUpdate('#users_unknown_username') === "")
        await modify_unknown_user("Anonymous");
    else if (new_unknown_username != users_config.users[0].display_name)
        await modify_unknown_user(new_unknown_username);

    let have: User[] = [];

    let nums: number[] = $('.authorized-user-id').map(function() {return parseInt(this.id.replace("users_authorized_user_", "").replace("_id", ""));}).get();
    for (let i of nums) {
        let username = $(`#users_authorized_user_${i}_username`).val().toString();
        let password = util.passwordUpdate(`#users_authorized_user_${i}_password`);
        let digest = password !== null && password !== "" ? YaMD5.YaMD5.hashStr(username + ":esp32-lib:" + password) : password;
        have.push({
            id: parseInt($(`#users_authorized_user_${i}_id`).val().toString()),
            roles: parseInt($(`#users_authorized_user_${i}_roles`).val().toString()),
            current: Math.round(($(`#users_authorized_user_${i}_current`).val() as number) * 1000),
            display_name: $(`#users_authorized_user_${i}_display_name`).val().toString(),
            username: username,
            digest_hash: digest
        });
    }

    let have_ids = have.map(x => x.id);
    let old_ids = users_config.users.map(x => x.id);
    let to_remove = old_ids.filter(x => have_ids.indexOf(x) < 0);
    let to_add = have_ids.filter(x => old_ids.indexOf(x) < 0);
    let to_modify = old_ids.filter(x => have_ids.indexOf(x) >= 0);

    for(let i of to_remove) {
        await remove_user(i);
    }

    for(let i of have) {
        if (to_modify.indexOf(i.id) >= 0 && !user_unmodified(i))
            await modify_user(i);
    }

    for(let i of have) {
        if (to_add.indexOf(i.id) >= 0) {
            if (i.digest_hash == null)
                i.digest_hash = "";
            await add_user(i);
        }
    }

    await save_authentication_config();

    let nfc_auth_enable = $('#evse_user').is(":checked");
    await API.save_maybe('evse/user_enabled', {"enabled": nfc_auth_enable}, __("evse.script.save_failed"));
}

function generate_user_ui(user: User, password: string) {
    let i = authorized_users_count;
    let result = `<div class="col mb-4">
                    <div class="card h-100">
                        <div class="card-header d-flex justify-content-between align-items-center">
                            <span data-feather="user"></span>
                            <button type="button" class="btn btn-sm btn-outline-dark"
                                id="users_authorized_user_${i}_remove">
                                <span data-feather="user-x" class="mr-2"></span><span style="font-size: 1rem; vertical-align: middle;">${__("users.script.delete")}</span>
                            </button>
                        </div>

                        <div class="card-body">
                            <div class="form-group" hidden>
                                <label for="users_authorized_user_${i}_id" class="form-label">${__("users.script.id")}</label>
                                <input type="text" readonly class="form-control authorized-user-id" id="users_authorized_user_${i}_id" class="form-label">
                            </div>
                            <div class="form-group">
                                <label for="users_authorized_user_${i}_username" class="form-label">${__("users.script.username")}</label>
                                <input type="text" class="form-control" id="users_authorized_user_${i}_username" class="form-label">
                            </div>
                            <div class="form-group">
                                <label for="users_authorized_user_${i}_display_name" class="form-label">${__("users.script.display_name")}</label>
                                <input type="text" class="form-control" id="users_authorized_user_${i}_display_name" class="form-label">
                            </div>
                            <div class="form-group" hidden>
                                <label for="users_authorized_user_${i}_roles" class="form-label">${__("users.script.roles")}</label>
                                <input type="text" class="form-control" id="users_authorized_user_${i}_roles" class="form-label">
                            </div>
                            <div class="form-group">
                                <label for="users_authorized_user_${i}_current" class="form-label">${__("users.script.current")}</label>
                                <div class="input-group">
                                    <input id="users_authorized_user_${i}_current" class="form-control" type="number" min="6" max="32" step="any" required>
                                    <div class="input-group-append">
                                        <div class="input-group-text form-control">A</div>
                                    </div>
                                </div>
                            </div>
                            <div class="form-group">
                                <label class="form-label" for="users_authorized_user_${i}_password">${__("users.script.password")}</label>
                                <div class="input-group">
                                    <input type="password" id="users_authorized_user_${i}_password" class="form-control" value="" maxlength="64" placeholder="${__("users.script.unchanged")}"/>
                                    <div class="input-group-append">
                                        <div class="input-group-text custom-control custom-switch" style="padding-left: 2.5rem;">
                                            <input id="users_authorized_user_${i}_show_password" type="checkbox" class="custom-control-input" aria-label="Show password">
                                            <label class="custom-control-label" for="users_authorized_user_${i}_show_password" style="line-height: 20px;"><span data-feather="eye"></span></label>
                                        </div>
                                        <div class="input-group-text custom-control custom-switch" style="padding-left: 2.5rem;">
                                            <input id="users_authorized_user_${i}_clear_password" type="checkbox" class="custom-control-input user-disable-password" aria-label="Disable login">
                                            <label class="custom-control-label" for="users_authorized_user_${i}_clear_password" style="line-height: 20px;"><span data-feather="slash"></span></label>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>`;
    $('#users_add_user_card').before(result);
    $(`#users_authorized_user_${i}_id`).val(user.id);
    $(`#users_authorized_user_${i}_roles`).val(user.roles);
    util.setNumericInput(`users_authorized_user_${i}_current`, user.current / 1000, 3);
    $(`#users_authorized_user_${i}_display_name`).val(user.display_name);
    $(`#users_authorized_user_${i}_username`).val(user.username);
    $(`#users_authorized_user_${i}_password`).val(password)
    $(`#users_authorized_user_${i}_show_password`).on("change", util.toggle_password_fn(`#users_authorized_user_${i}_password`));
    $(`#users_authorized_user_${i}_clear_password`).on("change", util.clear_password_fn(`#users_authorized_user_${i}_password`, __("users.script.login_disabled")));
    $(`#users_authorized_user_${i}_clear_password`).prop("checked", (password === ""));
    $(`#users_authorized_user_${i}_clear_password`).trigger("change");
    ++authorized_users_count;
    feather.replace();
    $(`#users_authorized_user_${i}_remove`).on("click", () => {
        $(`#users_authorized_user_${i}_remove`).parent().parent().parent().remove();
        $('#users_config_save_button').prop("disabled", false);
        check_http_auth_allowed();
    });
}

// The first run of this function must always create the GUI,
// it will be empty after a web interface reboot otherwise.
let gui_created = false;
function update_users_config(force: boolean) {
    let cfg = API.get('users/config');

    next_user_id = cfg.next_user_id;

    $('#users_authentication_enable').prop("checked", cfg.http_auth_enabled);
    $('#users_unknown_username').val(cfg.users[0].display_name == "Anonymous" ? __("charge_tracker.script.unknown_user") : cfg.users[0].display_name);

    if (!force && !$('#users_config_save_button').prop('disabled') && gui_created)
        return;

    $('#users_config_save_button').prop('disabled', true);
    gui_created = true;

    if (cfg.users.length != authorized_users_count) {
        let authorized_users = "";
        for (let i = 0; i < cfg.users.length; i++) {
            authorized_users += `<div class="col mb-4" ${i == 0 ? "hidden" : ""}>
                    <div class="card h-100">
                        <div class="card-header d-flex justify-content-between align-items-center">
                            <span data-feather="user"></span>
                            <button type="button" class="btn btn-sm btn-outline-dark"
                                id="users_authorized_user_${i}_remove">
                                <span data-feather="user-x" class="mr-2"></span><span style="font-size: 1rem; vertical-align: middle;">${__("users.script.delete")}</span>
                            </button>
                        </div>
                        <div class="card-body">
                            <div class="form-group" hidden>
                                <label for="users_authorized_user_${i}_id" class="form-label">${__("users.script.id")}</label>
                                <input type="text" readonly class="form-control authorized-user-id" id="users_authorized_user_${i}_id" class="form-label">
                            </div>
                            <div class="form-group">
                                <label for="users_authorized_user_${i}_username" class="form-label">${__("users.script.username")}</label>
                                <input type="text" class="form-control" id="users_authorized_user_${i}_username" class="form-label">
                            </div>
                            <div class="form-group">
                                <label for="users_authorized_user_${i}_display_name" class="form-label">${__("users.script.display_name")}</label>
                                <input type="text" class="form-control" id="users_authorized_user_${i}_display_name" class="form-label">
                            </div>
                            <div class="form-group" hidden>
                                <label for="users_authorized_user_${i}_roles" class="form-label">${__("users.script.roles")}</label>
                                <input type="text" class="form-control" id="users_authorized_user_${i}_roles" class="form-label">
                            </div>
                            <div class="form-group">
                                <label for="users_authorized_user_${i}_current" class="form-label">${__("users.script.current")}</label>
                                <div class="input-group">
                                    <input id="users_authorized_user_${i}_current" class="form-control" type="number" min="6" max="32" step="any" required>
                                    <div class="input-group-append">
                                        <div class="input-group-text form-control">A</div>
                                    </div>
                                </div>
                            </div>
                            <div class="form-group">
                                <label class="form-label" for="users_authorized_user_${i}_password">${__("users.script.password")}</label>
                                <div class="input-group">
                                    <input type="password" id="users_authorized_user_${i}_password" class="form-control" value="" maxlength="64" placeholder="${__("users.script.unchanged")}"/>
                                    <div class="input-group-append">
                                        <div class="input-group-text custom-control custom-switch" style="padding-left: 2.5rem;">
                                            <input id="users_authorized_user_${i}_show_password" type="checkbox" class="custom-control-input" aria-label="Show password">
                                            <label class="custom-control-label" for="users_authorized_user_${i}_show_password" style="line-height: 20px;"><span data-feather="eye"></span></label>
                                        </div>
                                        <div class="input-group-text custom-control custom-switch" style="padding-left: 2.5rem;">
                                            <input id="users_authorized_user_${i}_clear_password" type="checkbox" class="custom-control-input user-disable-password" aria-label="Disable login">
                                            <label class="custom-control-label" for="users_authorized_user_${i}_clear_password" style="line-height: 20px;"><span data-feather="slash"></span></label>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>`;
        }
        authorized_users += `<div class="col mb-4" id="users_add_user_card">
        <div class="card h-100">
            <div class="card-header d-flex justify-content-between align-items-center">
                <span data-feather="user-plus"></span>
                <button type="button" class="btn btn-sm btn-outline-dark"
                                id="blah" disabled style="visibility: hidden;">
                                <span data-feather="user-x" class="mr-2"></span><span style="font-size: 1rem; vertical-align: middle;">${__("users.script.delete")}</span>
                            </button>
            </div>
            <div class="card-body">
                <button id="users_add_user" type="button" class="btn btn-light btn-lg btn-block" style="height: 100%;" data-toggle="modal" data-target="#users_add_user_modal">${__("users.script.add_user")}</button>
                <span id="users_add_user_disabled" hidden></span>
            </div>
        </div>
    </div>`;
        $('#users_authorized_users').html(authorized_users);
        authorized_users_count = cfg.users.length;
        feather.replace();
        for (let i = 0; i < cfg.users.length; i++) {
            $(`#users_authorized_user_${i}_remove`).on("click", () => {
                $(`#users_authorized_user_${i}_remove`).parent().parent().parent().remove();
                $('#users_config_save_button').prop("disabled", false);
                check_http_auth_allowed();
            });
        }
    }

    $('#users_add_user').prop("hidden", cfg.users.length >= MAX_ACTIVE_USERS || next_user_id == 0);
    $('#users_add_user_disabled').prop("hidden", cfg.users.length < MAX_ACTIVE_USERS && next_user_id != 0);
    if (cfg.users.length >= MAX_ACTIVE_USERS)
        $('#users_add_user_disabled').html(__("users.script.add_user_disabled_prefix") + (MAX_ACTIVE_USERS - 1 /* anonymous */) + __("users.script.add_user_disabled_suffix"));
    else if (next_user_id == 0)
        $('#users_add_user_disabled').html(__("users.script.add_user_disabled_user_ids_exhausted"));

    for (let i = 0; i < cfg.users.length; i++) {
        const s = cfg.users[i];
        $(`#users_authorized_user_${i}_id`).val(s.id);
        $(`#users_authorized_user_${i}_roles`).val(s.roles);
        util.setNumericInput(`users_authorized_user_${i}_current`, s.current / 1000, 3);
        $(`#users_authorized_user_${i}_display_name`).val(s.display_name);
        $(`#users_authorized_user_${i}_username`).val(s.username);
        $(`#users_authorized_user_${i}_show_password`).on("change", util.toggle_password_fn(`#users_authorized_user_${i}_password`));
        $(`#users_authorized_user_${i}_clear_password`).on("change", util.clear_password_fn(`#users_authorized_user_${i}_password`, __("users.script.login_disabled")));
        $(`#users_authorized_user_${i}_clear_password`).prop("checked", (s.digest_hash === ""));
        $(`#users_authorized_user_${i}_clear_password`).trigger("change");
    }

    check_http_auth_allowed();
}

function check_http_auth_allowed() {
    let disable_auth = $('.user-disable-password').filter(function() {
            if ($(this).prop("checked"))
                return false;

            let i_str = $(this).attr('id');
            let i = parseInt(i_str.replace("users_authorized_user_", "").split("_")[0]);
            let new_pw = $(`#users_authorized_user_${i}_password`).val();

            let id = $(`#users_authorized_user_${i}_id`).val();

            let filtered = API.get("users/config").users.filter(user => user.id == id);
            if (filtered.length == 0)
                return new_pw != "";
            else
                return filtered[0].digest_hash === null || new_pw != "";
        }).length == 0;

    if ($('#users_authentication_enable').prop("disabled") == disable_auth)
        return;

    $('#users_authentication_enable').prop("disabled", disable_auth);
    if (disable_auth) {
        if ($('#users_authentication_enable').prop("checked"))
            $('#users_authentication_enable').addClass("is-invalid");
        $('#users_authentication_enable').prop("checked", !disable_auth);
    } else {
        $('#users_authentication_enable').removeClass("is-invalid");
        $('#users_authentication_enable').prop("checked", API.get("users/config").http_auth_enabled);
    }
}


function update_evse_user() {
    let x = API.get_maybe('evse/user_enabled');
    $('#evse_user').prop("checked", x.enabled);
}


export function init() {
    $('#users_config_form').on('input', () => $('#users_config_save_button').prop("disabled", false));
    $('#users_config_form').on('input', check_http_auth_allowed);

    $('#users_clear_unknown_username').on("change", util.clear_password_fn("#users_unknown_username"));

    $('#users_config_form').on('submit', async function (event: Event) {
        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if ((this as HTMLFormElement).checkValidity() === false) {
            return;
        }

        $('#users_config_save_spinner').prop('hidden', false);
        let finally_fn = () => $('#users_config_save_spinner').prop('hidden', true);

        await save_users_config()
            .then(() => $('#users_config_save_button').prop("disabled", true))
            .then(util.getShowRebootModalFn(__("users.script.reboot_content_changed")))
            .then(finally_fn, finally_fn);
    });

    $('#users_add_user_form').on("input", () => {
        let username = $('#users_config_user_new_username').val().toString();

        if (API.get("users/config").users.some(u => u.username == username)) {
            $('#users_config_user_new_username').addClass("is-invalid");
            $('#users_config_user_new_username_feedback').html(__("users.content.add_user_modal_username_invalid"));
        }
        else
            $('#users_config_user_new_username').removeClass("is-invalid");
    })

    $('#users_add_user_form').on("submit", async (event: Event) => {
        event.preventDefault();
        event.stopPropagation();

        if ($('#users_config_user_new_username').hasClass("is-invalid"))
            return;

        const [usernames, _] = await getAllUsernames().catch(err => {
            util.add_alert("download-usernames", "danger", __("users.script.download_usernames_failed"), err);
            return [null, null] as [string[], string[]];
        });

        if (usernames == null)
            return;

        let username = $('#users_config_user_new_username').val().toString();

        if (usernames.some(x => x == username)) {
            $('#users_config_user_new_username_feedback').html(__("users.script.username_already_tracked"));
            $('#users_config_user_new_username').addClass("is-invalid");
            return;
        }

        let form = $('#users_add_user_form')[0] as HTMLFormElement;
        if (!form.checkValidity()) {
            form.classList.add('was-validated');
            return;
        } else {
            form.classList.remove('was-validated');
        }

        let current = $('#users_config_user_new_current').val();
        if (current == "")
            current = 32;

        generate_user_ui({
            id: -1,
            username: username,
            current: Math.round((current as number) * 1000),
            display_name: $('#users_config_user_new_display_name').val().toString(),
            roles: 0xFFFF,
            digest_hash: ""
        }, $('#users_config_user_new_password').val().toString());

        $('#users_add_user_modal').modal('hide');
        $('#users_config_save_button').prop("disabled", false);
        check_http_auth_allowed();
    });

    $('#users_add_user_modal').on("hidden.bs.modal", () => {
        let form = $('#users_add_user_form')[0] as HTMLFormElement;
        form.reset();
    })

    $(`#users_config_user_show_password`).on("change", util.toggle_password_fn(`#users_config_user_new_password`));
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('users/config', () => update_users_config(false));
    source.addEventListener("evse/user_enabled", update_evse_user);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-users').prop('hidden', !module_init.users);
}
