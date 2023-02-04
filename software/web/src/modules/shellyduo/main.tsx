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

import * as API from "../../ts/api";

import { h, render } from "preact";
import { __ } from "../../ts/translation";
import { ConfigPageHeader } from "../../ts/components/config_page_header";

render(<ConfigPageHeader prefix="shellyduo" title={__("shellyduo.content.shellyduo")} />, $('#shellyduo_header')[0]);

function update_config(force: boolean = false) {
    if (!force && !$('#shellyduo_config_save_button').prop("disabled"))
        return;

    let cfg = API.get('shellyduo/config');
    $('#shellyduo_enabled').prop("checked", cfg.enable);
    $('#shellyduo_host').val(cfg.host);
    $('#shellyduo_intensity').val(cfg.intensity);
}

function update_state() {

}

function save_config() {
    API.save("shellyduo/config", {
                enable: $('#shellyduo_enabled').prop("checked"),
                host: $('#shellyduo_host').val().toString(),
                intensity: parseInt($('#shellyduo_intensity').val().toString())
            },
            __("shellyduo.script.save_failed"),
            __("shellyduo.script.reboot_content_changed"))
        .then(() => $('#shellyduo_config_save_button').prop("disabled", true));
}

export function init() {
    $('#shellyduo_config_form').on('input', () => $('#shellyduo_config_save_button').prop("disabled", false));

    $('#shellyduo_config_form').on('submit', function (this: HTMLFormElement, event: Event) {
        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }

        save_config();
    });
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('shellyduo/config', () => update_config());
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-shellyduo').prop('hidden', !module_init.shellyduo);
}
