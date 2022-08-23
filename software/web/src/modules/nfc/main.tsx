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

import { h, render } from "preact";
import { __, translate_unchecked } from "../../ts/translation";
import { ConfigPageHeader } from "../../ts/components/config_page_header";

render(<ConfigPageHeader prefix="nfc" title={__("nfc.content.nfc")} />, $('#nfc_header')[0]);

const MAX_AUTHORIZED_TAGS = 16;

let authorized_tag_count = -1;

type NFCConfig = API.getType['nfc/config'];
type AuthorizedTag = NFCConfig['authorized_tags'][0];

let nfc_card_symbol = '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="currentColor" stroke="currentColor" class="feather feather-nfc"><g transform="matrix(1.33 0 0 -1.33 -46.7 105)"><g transform="matrix(.0118 0 0 .0118 39.3 61.8)" fill="currentColor"><path d="m5.51 1345-2.79-149c-.457-23.2-5.91-570 .043-726 9.85-253 24.2-393 122-470h227c-10.9 2.55-21.6 5.48-32.4 9.28-150 52.7-176 187-194 474-2.98 49.9-4.48 145-4.48 250 0 108 1.14 225 2.33 315l456-456v179l-573 573"/><path d="m798 55.1 2.79 149c.461 23.2 5.91 570-.046 725-9.84 252-24.2 393-122 470h-227c10.9-2.55 21.7-5.48 32.4-9.27 150-52.7 176-187 194-474 2.98-49.9 4.49-145 4.49-250 0-108-1.15-225-2.34-315l-456 456v-179l573-573"/></g></g></svg>';
let nfc_card_delete_symbol = '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-user-x mr-2"><g transform="matrix(1.33 0 0 -1.33 -51.7 105) translate(0 0)"><g transform="matrix(.0118 0 0 .0118 39.3 61.8)" fill="currentColor"><path d="m5.51 1345-2.79-149c-.457-23.2-5.91-570 .043-726 9.85-253 24.2-393 122-470h227c-10.9 2.55-21.6 5.48-32.4 9.28-150 52.7-176 187-194 474-2.98 49.9-4.48 145-4.48 250 0 108 1.14 225 2.33 315l456-456v179l-573 573"></path><path d="m798 55.1 2.79 149c.461 23.2 5.91 570-.046 725-9.84 252-24.2 393-122 470h-227c10.9-2.55 21.7-5.48 32.4-9.27 150-52.7 176-187 194-474 2.98-49.9 4.49-145 4.49-250 0-108-1.15-225-2.34-315l-456 456v-179l573-573"></path></g></g><line x1="18" x2="23" y1="10" y2="15"></line><line x1="23" x2="18" y1="10" y2="15"></line></svg>';
let nfc_card_add_symbol = '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-user-x mr-2"><g transform="matrix(1.33 0 0 -1.33 -51.7 105) translate(0 0)"><g transform="matrix(.0118 0 0 .0118 39.3 61.8)" fill="currentColor"><path d="m5.51 1345-2.79-149c-.457-23.2-5.91-570 .043-726 9.85-253 24.2-393 122-470h227c-10.9 2.55-21.6 5.48-32.4 9.28-150 52.7-176 187-194 474-2.98 49.9-4.48 145-4.48 250 0 108 1.14 225 2.33 315l456-456v179l-573 573"></path><path d="m798 55.1 2.79 149c.461 23.2 5.91 570-.046 725-9.84 252-24.2 393-122 470h-227c10.9-2.55 21.7-5.48 32.4-9.27 150-52.7 176-187 194-474 2.98-49.9 4.49-145 4.49-250 0-108-1.15-225-2.34-315l-456 456v-179l573-573"></path></g></g><line x1="20" y1="9" x2="20" y2="15"></line><line x1="23" y1="12" x2="17" y2="12"></line></svg>';

function update_nfc_config(cfg: NFCConfig = API.get('nfc/config'), force: boolean) {
    if (!force && !$('#nfc_config_save_button').prop('disabled'))
        return;

    if (cfg.authorized_tags.length != authorized_tag_count) {
        let authorized_tags = "";
        for (let i = 0; i < cfg.authorized_tags.length; i++) {
            authorized_tags += `<div class="col mb-4">
                    <div class="card h-100">
                        <div class="card-header d-flex justify-content-between align-items-center">
                            ${nfc_card_symbol}
                            <button type="button" class="btn btn-sm btn-outline-dark"
                                id="nfc_authorized_tag_${i}_remove">
                                ${nfc_card_delete_symbol}<span style="font-size: 1rem; vertical-align: middle;">${__("nfc.script.delete")}</span>
                            </button>
                        </div>

                        <div class="card-body">
                            <div class="form-group">
                                <label for="nfc_authorized_tag_${i}_tag_id" class="form-label">${__("nfc.script.tag_id")}</label>
                                <input type="text" id="nfc_authorized_tag_${i}_tag_id" class="form-control">
                            </div>
                            <div class="form-group">
                                <label for="nfc_authorized_tag_${i}_user_id" class="form-label">${__("nfc.script.user_id")}</label>
                                <select id="nfc_authorized_tag_${i}_user_id" class="form-control custom-select nfc-user-select">

                                </select>
                            </div>
                            <div class="">
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
                            <span id="nfc_authorized_tag_${i}_last_seen"></span>
                        </div>
                    </div>
                </div>`;
        }
        authorized_tags += `<div class="col mb-4">
        <div class="card h-100">
            <div class="card-header d-flex justify-content-between align-items-center">
                ${nfc_card_add_symbol}
                <button type="button" class="btn btn-sm btn-outline-dark" style="visibility: hidden;">
                    ${nfc_card_delete_symbol}
                </button>
            </div>
            <div class="card-body">
                <button id="nfc_add_tag" type="button" class="btn btn-light btn-lg btn-block" style="height: 100%;" data-toggle="modal" data-target="#nfc_add_tag_modal">${__("nfc.script.add_tag")}</button>
                <span id="nfc_add_tag_disabled" hidden>${__("nfc.script.add_tag_disabled_prefix") + MAX_AUTHORIZED_TAGS + __("nfc.script.add_tag_disabled_suffix")}</span>
            </div>
        </div>
    </div>`;
        $('#nfc_authorized_tags').html(authorized_tags);
        authorized_tag_count = cfg.authorized_tags.length;
        feather.replace();
        for (let i = 0; i < cfg.authorized_tags.length; i++) {
            $(`#nfc_authorized_tag_${i}_remove`).on("click", () => {
                $('#nfc_config_save_button').prop("disabled", false);
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
        $(`#nfc_authorized_tag_${i}_tag_id`).val(s.tag_id);
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
            tag_id: $(`#nfc_authorized_tag_${i}_tag_id`).val().toString()
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

    API.save('nfc/config', payload, __("nfc.script.save_failed"), __("nfc.script.reboot_content_changed"))
       .then(() => $('#nfc_config_save_button').prop("disabled", true));
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

    let cfg = API.get("nfc/config");
    for(let i = 0; i < current_nfc_config.authorized_tags.length; ++i) {
        let tag = current_nfc_config.authorized_tags[i];
        let auth_seen_idx = auth_seen_ids.indexOf(i);

        if (auth_seen_idx >= 0) {
            $(`#nfc_authorized_tag_${i}_last_seen`).html(__("nfc.content.last_seen") + util.format_timespan(Math.floor(auth_seen_tags[auth_seen_idx].last_seen / 1000)) + __("nfc.content.last_seen_suffix"));
        } else {
            $(`#nfc_authorized_tag_${i}_last_seen`).html(__("nfc.script.not_seen"));
        }
    }

    for (let i = 0; i < current_nfc_config.authorized_tags.length; ++i) {
        if (auth_seen_ids.indexOf(i) >= 0)
            continue;
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
        $(`#nfc_seen_tag_${i}_type`).text(translate_unchecked(`nfc.content.type_${unauth_seen_tags[i].tag_type}`));
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
    if (nfc_config == null)
        return;

    let options = cfg.users.map((x) => `<option value=${x.id}>${x.id == 0 ? __("nfc.script.not_assigned") : x.display_name}</option>`).join("");
    $('.nfc-user-select').empty().append(options);
    for (let i = 0; i < nfc_config.authorized_tags.length; i++) {
        const s = nfc_config.authorized_tags[i];
        $(`#nfc_authorized_tag_${i}_user_id`).val(s.user_id);
    }
}

export function init() {

    $('#nfc_config_form').on('input', (event: Event) => $('#nfc_config_save_button').prop("disabled", false));

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

        let form = $('#nfc_add_tag_form')[0] as HTMLFormElement;
        form.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (form.checkValidity() === false) {
            return;
        }

        $('#nfc_add_tag_modal').modal('hide');
        $('#nfc_config_save_button').prop("disabled", false);

        let new_config = collect_nfc_config({
            user_id: parseInt($('#nfc_config_tag_new_user_id').val().toString()),
            tag_id: $('#nfc_config_tag_new_tag_id').val().toString(),
            tag_type: parseInt($('#nfc_config_tag_new_tag_type').val().toString())
        }, null);

        update_nfc_config(new_config, true);
    });

    $('#nfc_add_tag_modal').on("hidden.bs.modal", () => {
        let form = $('#nfc_add_tag_form')[0] as HTMLFormElement;
        form.reset();
    })
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('nfc/config', () => update_nfc_config(undefined, false));
    source.addEventListener('nfc/seen_tags', update_nfc_seen_tags);
    source.addEventListener('nfc/config', update_nfc_seen_tags);
    source.addEventListener('nfc/config', update_users_config);
    source.addEventListener('users/config', update_users_config);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-nfc').prop('hidden', !module_init.nfc);
}
