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

declare function __(s: string): string;

const MAX_AUTHORIZED_TAGS = 16;

let authorized_tag_count = -1;

type NFCConfig = API.getType['nfc/config'];
type AuthorizedTag = NFCConfig['authorized_tags'][0];

function update_nfc_config(cfg: NFCConfig = API.get('nfc/config'), force: boolean) {
    if (!force && !$('#nfc_save_button').prop('disabled'))
        return;

    if (cfg.authorized_tags.length != authorized_tag_count) {
        let authorized_tags = "";
        for (let i = 0; i < cfg.authorized_tags.length; i++) {
            authorized_tags += `<div class="col mb-4">
                    <div class="card h-100">
                        <div class="card-header d-flex justify-content-between align-items-center">
                            <span class="h5" id="nfc_authorized_tag_${i}_tag_id" style="margin-bottom: 0"></span>
                            <button type="button" class="btn btn-sm btn-outline-secondary"
                                id="nfc_authorized_tag_${i}_remove">
                                <span data-feather="trash-2"></span>
                            </button>
                        </div>

                        <div class="card-body">
                            <div class="form-group">
                                <label for="nfc_authorized_tag_${i}_user_id" class="form-label">${__("nfc.script.user_id")}</label>
                                <select id="nfc_authorized_tag_${i}_user_id" class="form-control custom-select nfc-user-select">

                                </select>
                            </div>
                            <div class="form-group">
                                <label for="nfc_authorized_tag_${i}_tag_type" class="form-label">${__("nfc.script.tag_type")}</label>
                                <select id="nfc_authorized_tag_${i}_tag_type" class="form-control custom-select">
                                    <option value="0">${__("nfc.content.type_0")}</option>
                                    <option value="1">${__("nfc.content.type_1")}</option>
                                    <option value="2">${__("nfc.content.type_2")}</option>
                                    <option value="3">${__("nfc.content.type_3")}</option>
                                    <option value="4">${__("nfc.content.type_4")}</option>
                                </select>
                            </div>
                        </div>
                        <div class="card-footer">
                            <small id="nfc_authorized_tag_${i}_last_seen" style="visibility: hidden;">${__("nfc.script.last_seen_unknown")}</small>
                        </div>
                    </div>
                </div>`;
        }
        authorized_tags += `<div class="col mb-4">
        <div class="card h-100">
            <div class="card-header d-flex justify-content-between align-items-center">
                <span class="h5" style="margin-bottom: 0">${__("nfc.script.add_tag")}</span>
                <button type="button" class="btn btn-sm btn-outline-secondary" style="visibility: hidden;">
                        <span data-feather="trash-2"></span>
                </button>
            </div>
            <div class="card-body">
                <button id="nfc_add_tag" type="button" class="btn btn-light btn-lg btn-block" style="height: 100%;" data-toggle="modal" data-target="#nfc_add_tag_modal"><span data-feather="plus-circle"></span></button>
                <span id="nfc_add_tag_disabled" hidden>${__("nfc.script.add_tag_disabled_prefix") + MAX_AUTHORIZED_TAGS + __("nfc.script.add_tag_disabled_suffix")}</span>
            </div>
            <div class="card-footer">
                <small style="visibility: hidden;">${__("nfc.script.last_seen_unknown")}</small>
            </div>
        </div>
    </div>`;
        $('#nfc_authorized_tags').html(authorized_tags);
        authorized_tag_count = cfg.authorized_tags.length;
        feather.replace();
        for (let i = 0; i < cfg.authorized_tags.length; i++) {
            $(`#nfc_authorized_tag_${i}_remove`).on("click", () => {
                $('#nfc_save_button').prop("disabled", false);
                update_nfc_config(collect_nfc_config(null, i), true)
            });
        }
        update_users_config();
    }

    $('#nfc_add_tag').prop("hidden", cfg.authorized_tags.length >= MAX_AUTHORIZED_TAGS);
    $('#nfc_add_tag_disabled').prop("hidden", cfg.authorized_tags.length < MAX_AUTHORIZED_TAGS);

    for (let i = 0; i < cfg.authorized_tags.length; i++) {
        const s = cfg.authorized_tags[i];
        $(`#nfc_authorized_tag_${i}_user_id`).val(s.user_id);
        $(`#nfc_authorized_tag_${i}_tag_type`).val(s.tag_type);
        $(`#nfc_authorized_tag_${i}_tag_id`).html(s.tag_id);
    }
}

function collect_nfc_config(new_tag: AuthorizedTag = null, remove_tag: number = null) : NFCConfig {
    let tags: AuthorizedTag[] = [];
    for(let i = 0; i < authorized_tag_count; ++i) {
        if (remove_tag !== null && i == remove_tag)
            continue;
        let c: AuthorizedTag = {
            tag_type: parseInt($(`#nfc_authorized_tag_${i}_tag_type`).val().toString()),
            user_id: parseInt($(`#nfc_authorized_tag_${i}_user_id`).val().toString()),
            tag_id: $(`#nfc_authorized_tag_${i}_tag_id`).html().toString()
        }
        tags.push(c);
    }
    if (new_tag != null)
        tags.push(new_tag);

    return {
        authorized_tags: tags
    };
}

function save_nfc_config() {
    let payload = collect_nfc_config();

    $.ajax({
        url: '/nfc/config_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: () => {
            $('#nfc_save_button').prop("disabled", true);
            util.getShowRebootModalFn(__("nfc.script.reboot_content_changed"))();
        },
        error: (xhr, status, error) => util.add_alert("nfc_config_update_failed", "alert-danger", __("nfc.script.save_failed"), error + ": " + xhr.responseText)
    });
}


let unauth_tag_list_length = 0;
function update_nfc_seen_tags() {
    let seen_tags = API.get('nfc/seen_tags');
    let current_nfc_config = collect_nfc_config(null, null);

    type NFCSeenTag = API.getType['nfc/seen_tags'][0];

    let unauth_seen_tags: NFCSeenTag[] = [];

    let auth_seen_tags: NFCSeenTag[] = [];
    let auth_seen_ids: number[] = [];

outer_loop:
    for(let i = 0; i < seen_tags.length; ++i) {
        if (seen_tags[i].tag_id == "")
            continue;

        for (let auth_tag_idx = 0; auth_tag_idx < current_nfc_config.authorized_tags.length; ++auth_tag_idx) {
            let auth_tag = current_nfc_config.authorized_tags[auth_tag_idx];
            if (auth_tag.tag_type != seen_tags[i].tag_type)
                continue;

            if (auth_tag.tag_id != seen_tags[i].tag_id)
                continue;

            auth_seen_tags.push(seen_tags[i]);
            auth_seen_ids.push(auth_tag_idx);

            continue outer_loop;
        }

        unauth_seen_tags.push(seen_tags[i]);
    }

    for(let i = 0; i < auth_seen_tags.length; ++i) {
        $(`#nfc_authorized_tag_${auth_seen_ids[i]}_last_seen`).prop("style",  "");
        $(`#nfc_authorized_tag_${auth_seen_ids[i]}_last_seen`).text(__("nfc.content.last_seen") + util.format_timespan(Math.floor(auth_seen_tags[i].last_seen / 1000)) + __("nfc.content.last_seen_suffix"));
    }

    for (let i = 0; i < current_nfc_config.authorized_tags.length; ++i) {
        if (auth_seen_ids.includes(i))
            continue;
        $(`#nfc_authorized_tag_${i}_last_seen`).prop("style",  "visibility: hidden;");
    }

    if (unauth_seen_tags.length < unauth_tag_list_length) {
        $('#nfc_seen_tags > button').slice(unauth_seen_tags.length).remove();
        unauth_tag_list_length = unauth_seen_tags.length;
    }

    while (unauth_seen_tags.length > unauth_tag_list_length) {
        $('#nfc_seen_tags').append(`<button type="button" id="nfc_seen_tag_${unauth_tag_list_length}" class="list-group-item list-group-item-action">
        <h5 id="nfc_seen_tag_${unauth_tag_list_length}_id" class="mb-1 pr-2"></h5>
        <div class="d-flex w-100 justify-content-between">
            <span id="nfc_seen_tag_${unauth_tag_list_length}_last_seen"></span>
            <span id="nfc_seen_tag_${unauth_tag_list_length}_type"></span>
        </div>
         </button>`)
        ++unauth_tag_list_length;
    }

    for(let i = 0; i < unauth_tag_list_length; ++i) {
        $(`#nfc_seen_tag_${i}_id`).text(unauth_seen_tags[i].tag_id);
        $(`#nfc_seen_tag_${i}_type`).text(__(`nfc.content.type_${unauth_seen_tags[i].tag_type}`));
        $(`#nfc_seen_tag_${i}_last_seen`).text(__("nfc.content.last_seen") + util.format_timespan(Math.floor(unauth_seen_tags[i].last_seen / 1000)) + __("nfc.content.last_seen_suffix"));

        $(`#nfc_seen_tag_${i}`).on("click", () => {
            $(`#nfc_config_tag_new_tag_id`).val(unauth_seen_tags[i].tag_id);
            $(`#nfc_config_tag_new_tag_type`).val(unauth_seen_tags[i].tag_type);
        });
    }

    $('#nfc_config_tag_no_seen').prop("hidden", unauth_tag_list_length != 0);
}

function update_users_config() {
    let cfg = API.get('users/config');
    let nfc_config = API.get('nfc/config');

    let options = cfg.users.map((x) => `<option value=${x.id}>${x.id == 0 ? __("nfc.script.not_assigned") : x.display_name}</option>`).join("");
    $('.nfc-user-select').empty().append(options);
    for (let i = 0; i < nfc_config.authorized_tags.length; i++) {
        const s = nfc_config.authorized_tags[i];
        $(`#nfc_authorized_tag_${i}_user_id`).val(s.user_id);
    }
}

export function init() {

    $('#nfc_config_form').on('input', (event: Event) => $('#nfc_save_button').prop("disabled", false));

    $('#nfc_config_form').on('submit', function (this: HTMLFormElement, event: Event) {
        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }

        save_nfc_config();
    });

    $('#nfc_add_tag_form').on("submit", (event: Event) => {
        let btns = $('#nfc_seen_tags > button');
        if ($('#nfc_config_tag_new_tag_id').val().toString() === "" && btns.length == 1) {
            btns.first().trigger("click");
        }

        let form = <HTMLFormElement>$('#nfc_add_tag_form')[0];
        form.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (form.checkValidity() === false) {
            return;
        }

        $('#nfc_add_tag_modal').modal('hide');
        $('#nfc_save_button').prop("disabled", false);

        let new_config = collect_nfc_config({
            user_id: parseInt($('#nfc_config_tag_new_user_id').val().toString()),
            tag_id: $('#nfc_config_tag_new_tag_id').val().toString(),
            tag_type: parseInt($('#nfc_config_tag_new_tag_type').val().toString())
        }, null);

        update_nfc_config(new_config, true);
    });

    $('#nfc_add_tag_modal').on("hidden.bs.modal", () => {
        let form = <HTMLFormElement>$('#nfc_add_tag_form')[0];
        form.reset();
    })
}

export function addEventListeners(source: API.ApiEventTarget) {
    source.addEventListener('nfc/config', () => update_nfc_config(undefined, false));

    source.addEventListener('nfc/seen_tags', update_nfc_seen_tags);

    source.addEventListener('nfc/config', update_users_config);
    source.addEventListener('users/config', update_users_config);
}

export function updateLockState(module_init: any) {
    $('#sidebar-nfc').prop('hidden', !module_init.nfc);
}
