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

import feather from "../../ts/feather";

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { __ } from "../../ts/translation";

function update_config() {
    let config = API.get('info/display_name');

    // Remove the was-validated class to fix a visual bug
    // where saving the config triggers an update
    // that fills the elements, but clears the passphrase field.
    // An empty passphrase is invalid, so the input
    // field is marked as non-validated, confusing the user.
    let form = <HTMLFormElement>$('#device_name_form')[0];
    form.classList.remove('was-validated');

    $('#display_name').val(config.display_name);
    $('#display_name').parent().removeClass("input-group");
    $('#device_name_form_submit').prop('hidden', true);
    document.title = config.display_name + ' - ' + __("main.title");
}

function update_state() {

}

function save_config() {
    API.save('info/display_name',{
            display_name: $('#display_name').val().toString()
        },
        __("device_name.script.config_failed"));
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('info/name', update_state);
    source.addEventListener('info/display_name', update_config);
}

export function init() {
    // Use bootstrap form validation
    $('#device_name_form').on('submit', function (this: HTMLFormElement, event: Event) {
        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }
        save_config();
    });

    $('#display_name').on("input", () => {
        if (API.get('info/display_name').display_name == $('#display_name').val()) {
            $('#display_name').parent().removeClass("input-group");
            $('#device_name_form_submit').prop("hidden", true)
        } else {
            $('#display_name').parent().addClass("input-group");
            $('#device_name_form_submit').prop("hidden", false)
        }
    });
}

export function update_sidebar_state(module_init: any) {

}
