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

declare function __(s: string): string;

const MAX_ACTIVE_USERS = 8;

type UsersConfig = API.getType['users/config'];
type User = UsersConfig['users'][0];

function save_authentication_config() {
    return $.ajax({
        url: '/users/http_auth_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify({"enabled": $('#users_authentication_enable').is(':checked')}),
        success: util.getShowRebootModalFn(__("users.script.reboot_content_changed")),
        error: (xhr, status, error) => util.add_alert("authentication_config_update_failed", "alert-danger", __("users.script.save_failed"), error + ": " + xhr.responseText)
    });
}

function delete_user(id: number) {
    return $.ajax({
        url: '/users/delete',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify({"id": id}),
        success: () => {
        },
        error: (xhr, status, error) => {
            //util.add_alert("charge_manager_set_available_current_failed", "alert-danger", __("charge_manager.script.set_available_current_failed"), error + ": " + xhr.responseText);
        }
    });
}

function modify_user(user: User) {
    return $.ajax({
        url: '/users/modify',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(user),
        success: () => {
        },
        error: (xhr, status, error) => {
            //util.add_alert("charge_manager_set_available_current_failed", "alert-danger", __("charge_manager.script.set_available_current_failed"), error + ": " + xhr.responseText);
        }
    });
}

let next_user_id = 0;

function add_user(user: User) {
    user.id = next_user_id;
    ++next_user_id;
    return $.ajax({
        url: '/users/add',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(user),
        success: () => {
        },
        error: (xhr, status, error) => {
            //util.add_alert("charge_manager_set_available_current_failed", "alert-danger", __("charge_manager.script.set_available_current_failed"), error + ": " + xhr.responseText);
        }
    });
}

let authorized_users_count = -1;

async function save_users_config() {
    let users_config = API.get('users/config');

    let have: User[] = [];

    let nums: number[] = $('.authorized-user-id').map(function() {return parseInt(this.id.replace("users_authorized_user_", "").replace("_id", ""));}).get();
    for (let i of nums) {
        let username = $(`#users_authorized_user_${i}_username`).html();
        let password = util.passwordUpdate(`#users_authorized_user_${i}_password`);
        let digest = password == null ? null : password == "" ? "" : YaMD5.YaMD5.hashStr(username + ":esp32-lib:" + password);
        have.push({
            id: parseInt($(`#users_authorized_user_${i}_id`).val().toString()),
            roles: parseInt($(`#users_authorized_user_${i}_roles`).val().toString()),
            current: Math.round(<number>$(`#users_authorized_user_${i}_current`).val() * 1000),
            display_name: $(`#users_authorized_user_${i}_display_name`).val().toString(),
            username: username,
            digest_hash: digest
        });
    }

    let have_ids = have.map(x => x.id);
    let old_ids = users_config.users.map(x => x.id);
    let to_remove = old_ids.filter(x => !have_ids.includes(x));
    let to_add = have_ids.filter(x => !old_ids.includes(x));
    let to_modify = old_ids.filter(x => have_ids.includes(x));

    for(let i of to_remove) {
        await delete_user(i);
    }

    for(let i of have) {
        if (to_modify.includes(i.id))
            await modify_user(i);
    }

    for(let i of have) {
        if (to_add.includes(i.id)) {
            if (i.digest_hash == null)
                i.digest_hash = "";
            await add_user(i);
        }
    }

    await save_authentication_config();

    //TODO show reboot modal
}

function generate_user_ui(user: User, password: string) {
    let i = authorized_users_count;
    let result = `<div class="col mb-4">
                    <div class="card h-100">
                        <div class="card-header d-flex justify-content-between align-items-center">
                            <span class="h5" id="users_authorized_user_${i}_username" style="margin-bottom: 0"></span>
                            <button type="button" class="btn btn-sm btn-outline-secondary"
                                id="users_authorized_user_${i}_remove">
                                <span data-feather="user-x"></span>
                            </button>
                        </div>

                        <div class="card-body">
                            <div class="form-group" hidden>
                                <label for="users_authorized_user_${i}_id" class="form-label">${__("users.script.id")}</label>
                                <input type="text" readonly class="form-control authorized-user-id" id="users_authorized_user_${i}_id" class="form-label">
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
                                            <input id="users_authorized_user_${i}_clear_password" type="checkbox" class="custom-control-input" aria-label="Disable login">
                                            <label class="custom-control-label" for="users_authorized_user_${i}_clear_password" style="line-height: 20px;"><span data-feather="slash"></span></label>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                        <div class="card-footer">
                            <small id="users_authorized_user_${i}_last_seen" style="visibility: hidden;">${__("users.script.last_seen_unknown")}</small>
                        </div>
                    </div>
                </div>`;
    $('#users_add_user_card').before(result);
    $(`#users_authorized_user_${i}_id`).val(user.id);
    $(`#users_authorized_user_${i}_roles`).val(user.roles);
    util.setNumericInput(`users_authorized_user_${i}_current`, user.current / 1000, 3);
    $(`#users_authorized_user_${i}_display_name`).val(user.display_name);
    $(`#users_authorized_user_${i}_username`).html(user.username);
    $(`#users_authorized_user_${i}_password`).val(password)
    $(`#users_authorized_user_${i}_show_password`).on("change", util.toggle_password_fn(`#users_authorized_user_${i}_password`));
    $(`#users_authorized_user_${i}_clear_password`).on("change", util.clear_password_fn(`#users_authorized_user_${i}_password`, __("users.script.login_disabled")));
    $(`#users_authorized_user_${i}_clear_password`).prop("checked", (password === ""));
    $(`#users_authorized_user_${i}_clear_password`).trigger("change");
    ++authorized_users_count;
    feather.replace();
    $(`#users_authorized_user_${i}_remove`).on("click", () => {
        $(`#users_authorized_user_${i}_remove`).parent().parent().parent().remove();
        $('#users_save_button').prop("disabled", false);
    });
}

// The first run of this function must always create the GUI,
// it will be empty after a web interface reboot otherwise.
let gui_created = false;
function update_users_config(force: boolean) {
    let cfg = API.get('users/config');

    next_user_id = cfg.next_user_id;

    $('#users_next_user_id').val(cfg.next_user_id);
    $('#users_authentication_enable').prop("checked", cfg.http_auth_enabled);

    if (!force && !$('#users_save_button').prop('disabled') && gui_created)
        return;

    $('#users_save_button').prop('disabled', true);
    gui_created = true;

    if (cfg.users.length != authorized_users_count) {
        let authorized_users = "";
        for (let i = 0; i < cfg.users.length; i++) {
            authorized_users += `<div class="col mb-4" ${i == 0 ? "hidden" : ""}>
                    <div class="card h-100">
                        <div class="card-header d-flex justify-content-between align-items-center">
                            <span class="h5" id="users_authorized_user_${i}_username" style="margin-bottom: 0"></span>
                            <button type="button" class="btn btn-sm btn-outline-secondary"
                                id="users_authorized_user_${i}_remove">
                                <span data-feather="user-x"></span>
                            </button>
                        </div>

                        <div class="card-body">
                            <div class="form-group" hidden>
                                <label for="users_authorized_user_${i}_id" class="form-label">${__("users.script.id")}</label>
                                <input type="text" readonly class="form-control authorized-user-id" id="users_authorized_user_${i}_id" class="form-label">
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
                                            <input id="users_authorized_user_${i}_clear_password" type="checkbox" class="custom-control-input" aria-label="Disable login">
                                            <label class="custom-control-label" for="users_authorized_user_${i}_clear_password" style="line-height: 20px;"><span data-feather="slash"></span></label>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                        <div class="card-footer">
                            <small id="users_authorized_user_${i}_last_seen" style="visibility: hidden;">${__("users.script.last_seen_unknown")}</small>
                        </div>
                    </div>
                </div>`;
        }
        authorized_users += `<div class="col mb-4" id="users_add_user_card">
        <div class="card h-100">
            <div class="card-header d-flex justify-content-between align-items-center">
                <span class="h5" style="margin-bottom: 0">${__("users.script.add_user")}</span>
                <button type="button" class="btn btn-sm btn-outline-secondary" style="visibility: hidden;">
                        <span data-feather="trash-2"></span>
                </button>
            </div>
            <div class="card-body">
                <button id="users_add_user" type="button" class="btn btn-light btn-lg btn-block" style="height: 100%;" data-toggle="modal" data-target="#users_add_user_modal"><span data-feather="user-plus"></span></button>
                <span id="users_add_user_disabled" hidden>${__("users.script.add_user_disabled")}</span>
            </div>
            <div class="card-footer">
                <small style="visibility: hidden;">${__("users.script.last_seen_unknown")}</small>
            </div>
        </div>
    </div>`;
        $('#users_authorized_users').html(authorized_users);
        authorized_users_count = cfg.users.length;
        feather.replace();
        for (let i = 0; i < cfg.users.length; i++) {
            $(`#users_authorized_user_${i}_remove`).on("click", () => {
                $(`#users_authorized_user_${i}_remove`).parent().parent().parent().remove();
                $('#users_save_button').prop("disabled", false);
            });
        }
    }

    $('#users_add_user').prop("hidden", cfg.users.length >= MAX_ACTIVE_USERS);
    $('#users_add_user_disabled').prop("hidden", cfg.users.length < MAX_ACTIVE_USERS);

    for (let i = 0; i < cfg.users.length; i++) {
        const s = cfg.users[i];
        $(`#users_authorized_user_${i}_id`).val(s.id);
        $(`#users_authorized_user_${i}_roles`).val(s.roles);
        util.setNumericInput(`users_authorized_user_${i}_current`, s.current / 1000, 3);
        $(`#users_authorized_user_${i}_display_name`).val(s.display_name);
        $(`#users_authorized_user_${i}_username`).html(s.username);
        $(`#users_authorized_user_${i}_show_password`).on("change", util.toggle_password_fn(`#users_authorized_user_${i}_password`));
        $(`#users_authorized_user_${i}_clear_password`).on("change", util.clear_password_fn(`#users_authorized_user_${i}_password`, __("users.script.login_disabled")));
        $(`#users_authorized_user_${i}_clear_password`).prop("checked", (s.digest_hash === ""));
        $(`#users_authorized_user_${i}_clear_password`).trigger("change");
    }
}

export function init() {
    $('#users_config_form').on('input', () => $('#users_save_button').prop("disabled", false));

    $('#users_config_form').on('submit', async function (event: Event) {
        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if ((<HTMLFormElement>this).checkValidity() === false) {
            return;
        }

        $('#users_save_spinner').prop('hidden', false);
        await save_users_config().finally(() => $('#users_save_spinner').prop('hidden', true));
    });

    $('#users_add_user_form').on("submit", (event: Event) => {

        let form = <HTMLFormElement>$('#users_add_user_form')[0];
        form.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (form.checkValidity() === false) {
            return;
        }

        generate_user_ui({
            id: -1,
            username: $('#users_config_user_new_username').val().toString(),
            current: Math.round(<number>$('#users_config_user_new_current').val() * 1000),
            display_name: $('#users_config_user_new_display_name').val().toString(),
            roles: 0xFFFF,
            digest_hash: ""
        }, $('#users_config_user_new_password').val().toString());

        $('#users_add_user_modal').modal('hide');
        $('#users_save_button').prop("disabled", false);

/*
        let new_config = collect_nfc_config({
            tag_name: $('#users_config_tag_new_name').val().toString(),
            tag_id: fromHexBytes($('#users_config_tag_new_tag_id').val().toString()),
            tag_type: parseInt($('#users_config_tag_new_tag_type').val().toString())
        }, null);

        update_nfc_config(new_config, true);
*/
    });

    $('#users_add_user_modal').on("hidden.bs.modal", () => {
        let form = <HTMLFormElement>$('#users_add_user_form')[0];
        form.reset();
    })

   $(`#users_config_user_show_password`).on("change", util.toggle_password_fn(`#users_config_user_new_password`));
}

export function addEventListeners(source: API.ApiEventTarget) {
    source.addEventListener('users/config', () => update_users_config(false));
}

export function updateLockState(module_init: any) {
    $('#sidebar-users').prop('hidden', !module_init.users);
}
