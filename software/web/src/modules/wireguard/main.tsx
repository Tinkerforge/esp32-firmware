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
import { ConfigPageHeader } from "../../ts/config_page_header";

render(<ConfigPageHeader prefix="wireguard" title={__("wireguard.content.wireguard")} />, $('#wireguard_header')[0]);

function update_wireguard_state() {
    let state = API.get("wireguard/state").state;
    util.update_button_group("btn_group_wireguard_state", state);
}

function update_wireguard_config(force: boolean) {
    if (!force && !$('#wireguard_config_save_button').prop('disabled'))
        return;

    let cfg = API.get("wireguard/config");
    $('#wireguard_enable').prop("checked", cfg.enable);
    $('#wireguard_default_if').prop("checked", cfg.make_default_interface);
    $('#wireguard_internal_ip').val(cfg.internal_ip);
    $('#wireguard_internal_subnet').val(cfg.internal_subnet);
    $('#wireguard_internal_gateway').val(cfg.internal_gateway);
    $('#wireguard_remote_host').val(cfg.remote_host);
    $('#wireguard_remote_port').val(cfg.remote_port);

    $('#wireguard_clear_preshared_key').prop("checked", (cfg.preshared_key === ""));
    $('#wireguard_clear_preshared_key').trigger("change");

    $('#wireguard_allowed_ip').val(cfg.allowed_ip);
    $('#wireguard_allowed_subnet').val(cfg.allowed_subnet);
}

function save_wireguard_config() {
    API.save("wireguard/config", {
            enable: $('#wireguard_enable').prop("checked"),
            make_default_interface: $('#wireguard_default_if').prop("checked"),
            internal_ip: $('#wireguard_internal_ip').val().toString(),
            internal_subnet: $('#wireguard_internal_subnet').val().toString(),
            internal_gateway: $('#wireguard_internal_gateway').val().toString(),
            remote_host: $('#wireguard_remote_host').val().toString(),
            remote_port: parseInt($('#wireguard_remote_port').val().toString()),
            private_key: util.passwordUpdate('#wireguard_private_key'),
            remote_public_key: util.passwordUpdate('#wireguard_remote_public_key'),
            preshared_key:  util.passwordUpdate('#wireguard_preshared_key'),
            allowed_ip: $('#wireguard_allowed_ip').val().toString(),
            allowed_subnet: $('#wireguard_allowed_subnet').val().toString()
        },
        __("wireguard.script.save_failed"),
        __("wireguard.script.reboot_content_changed"));
}

export function init() {
    $('#wireguard_config_form').on('input', () => $('#wireguard_config_save_button').prop("disabled", false));
    $('#wireguard_config_form').on('submit', function (this: HTMLFormElement, event: Event) {
        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }

        save_wireguard_config();
    });

    $("#wireguard_show_private_key").on("change", util.toggle_password_fn("#wireguard_private_key"));
    $("#wireguard_clear_private_key").on("change", util.clear_password_fn("#wireguard_private_key"));

    $("#wireguard_show_remote_public_key").on("change", util.toggle_password_fn("#wireguard_remote_public_key"));
    $("#wireguard_clear_remote_public_key").on("change", util.clear_password_fn("#wireguard_remote_public_key"));

    $("#wireguard_show_preshared_key").on("change", util.toggle_password_fn("#wireguard_preshared_key"));
    $("#wireguard_clear_preshared_key").on("change", util.clear_password_fn("#wireguard_preshared_key", __("wireguard.script.preshared_key_unused")));
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('wireguard/state', () => update_wireguard_state());
    source.addEventListener('wireguard/config', () => update_wireguard_config(false));
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-wireguard').prop('hidden', !module_init.wireguard);
}
