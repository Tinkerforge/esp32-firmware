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

import $ from "jquery";

import * as util from "../util";

import YaMD5 from 'yamd5.js';

declare function __(s: string): string;

interface AuthenticationConfig {
    enable_auth: boolean,
    username: string,
    digest_hash: string
}

function update_authentication_config(config: AuthenticationConfig) {
    $('#authentication_enable').prop("checked", config.enable_auth);

    $('#authentication_username').val(config.username);
    $('#authentication_username').prop("disabled", !config.enable_auth);
    $('#authentication_username').prop("required", !config.enable_auth);

    $('#authentication_password').prop("disabled", !config.enable_auth);
    $('#authentication_password').prop("required", !config.enable_auth);
    $('#authentication_password').prop("placeholder", config.enable_auth ? __("util.unchanged") : "");

    $('#authentication_show_password').prop("disabled", !config.enable_auth);
    $('#authentication_show_password').prop("checked", false);
}

function save_authentication_config() {
    let username = $('#authentication_username').val().toString();
    let password = util.passwordUpdate('#authentication_password');

    let payload: AuthenticationConfig = {
        enable_auth: $('#authentication_enable').is(':checked'),
        username: username,
        digest_hash: password == null ? null : YaMD5.YaMD5.hashStr(username + ":esp32-lib:" + password)
    };

    $.ajax({
        url: '/authentication/config_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: util.getShowRebootModalFn(__("authentication.script.reboot_content_changed")),
        error: (xhr, status, error) => util.add_alert("authentication_config_update_failed", "alert-danger", __("authentication.script.save_failed"), error + ": " + xhr.responseText)
    });
}

export function init() {
    $("#authentication_show_password").on("change", util.toggle_password_fn("#authentication_password"));

    $("#authentication_enable").on("change", function(this: HTMLInputElement, ev: Event) {
        $('#authentication_username').prop("disabled", !this.checked);
        $('#authentication_password').prop("disabled", !this.checked);
        $('#authentication_show_password').prop("disabled", !this.checked);
        if (!this.checked) {
            $('#authentication_show_password').prop("checked", false);
        }

        $('#authentication_password').val('');
        let auth_placeholder = "";

        if (!$('#authentication_password').prop("required"))
            // If the field is not required, a password is currently stored.
            // if auth is to be enabled, the stored password can be used (-> unchanged if empty)
            // if auth is to be disabled, the stored password will be cleared
            auth_placeholder = this.checked ?  __("util.unchanged") : __("util.to_be_cleared");

        $('#authentication_password').attr("placeholder", auth_placeholder);
    });

    $('#authentication_config_form').on('submit', function (this: HTMLFormElement, event: Event) {
        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }

        if ($('#authentication_enable').is(':checked'))
            $('#authentication_confirm').modal('show');
        else
            save_authentication_config();
    });

    $('#authentication_confirm_button').on("click", () => {
        $('#authentication_confirm').modal('hide');
        save_authentication_config();
    });
}

export function addEventListeners(source: EventSource) {
    source.addEventListener('authentication/config', function (e: util.SSE) {
        update_authentication_config(<AuthenticationConfig>(JSON.parse(e.data)));
    }, false);
}

export function updateLockState(module_init: any) {
    $('#sidebar-authentication').prop('hidden', !module_init.authentication);
    $('#sidebar-system').prop('hidden',  $('#sidebar-system').prop('hidden') && !module_init.authentication);
}
