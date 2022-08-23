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

import { h, render } from "preact";
import { __, translate_unchecked } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";

render(<PageHeader title={__("firmware_update.content.firmware_update")} />, $('#firmware_update_header')[0]);

import bsCustomFileInput from "../../ts/bs-custom-file-input";

let last_version: string = null;

function update_version() {
    let version = API.get('info/version');

    if (last_version == null) {
        last_version = version.firmware;
    } else if (last_version != version.firmware) {
        window.location.reload();
    }

    $('#current_firmware').val(version.firmware);
    $('#current_spiffs').val(version.config);
}

function check_upload(type: string) {
    let file_select = $(`#${type}_file_select`)[0] as HTMLInputElement;

    $.ajax({
        timeout: 0,
        url: `/check_${type}`,
        type: 'POST',
        data: file_select.files[0].slice(0xd000 - 0x1000, 0xd000),
        contentType: false,
        processData: false,
        success: () => {
            upload(type);
        },
        error: (xhr, status, error) => {
            if (xhr.status == 423)
                util.add_alert("firmware_update_failed", "alert-danger", __("firmware_update.script.flash_fail"), __("firmware_update.script.vehicle_connected"));
            else {
                try {
                    let e = JSON.parse(xhr.responseText)
                    let error_message = translate_unchecked(e["error"])
                    if (e["error"] == "firmware_update.script.downgrade") {
                        error_message = error_message.replace("%fw%", e["fw"]).replace("%installed%", e["installed"]);
                        $('#downgrade_text').text(error_message);
                        $('#downgrade_modal').modal('show');
                    } else {
                        util.add_alert("firmware_update_failed","alert-danger", __("firmware_update.script.flash_fail"), error_message);
                    }
                } catch {
                    util.add_alert("firmware_update_failed","alert-danger", __("firmware_update.script.flash_fail"), error + ": " + xhr.responseText);
                }
            }
        }
    });
}

function upload(type: string) {
    util.pauseWebSockets();

    let file_select = $(`#${type}_file_select`)[0] as HTMLInputElement;
    let progress = $(`#${type}-progress`);
    let select = $(`#upload_${type}_form`);
    let progress_bar = $(`#${type}-progress-bar`);

    progress.prop("hidden", false);
    select.prop("hidden", true);

    $.ajax({
        timeout: 0,
        url: `/flash_${type}`,
        type: 'POST',
        data: file_select.files[0],
        contentType: false,
        processData: false,
        xhr: function () {
            let xhr = new window.XMLHttpRequest();
            xhr.upload.addEventListener('progress', function (evt) {
                if (evt.lengthComputable) {
                    let per = evt.loaded / evt.total;
                    progress_bar.prop('style', "width: " + (per * 100) + "%");
                    progress_bar.prop('aria-valuenow', (per * 100));
                }
            }, false);
            return xhr;
        },
        success: () => {
            progress.prop("hidden", true);
            select.prop("hidden", false);
            util.postReboot(__("firmware_update.script.flash_success"), __("util.reboot_text"));
        },
        error: (xhr, status, error) => {
            progress.prop("hidden", true);
            select.prop("hidden", false);
            if (xhr.status == 423)
                util.add_alert("firmware_update_failed", "alert-danger", __("firmware_update.script.flash_fail"), __("firmware_update.script.vehicle_connected"));
            else {
                let txt = xhr.responseText.startsWith("firmware_update.") ? translate_unchecked(xhr.responseText) : error + ": " + xhr.responseText;
                util.add_alert("firmware_update_failed","alert-danger", __("firmware_update.script.flash_fail"), txt);
            }
            util.resumeWebSockets();
        }
    });
}

function config_reset() {
    $.ajax({
        url: `/config_reset`,
        type: 'PUT',
        contentType: "application/json; charset=utf-8",
        data: JSON.stringify({"do_i_know_what_i_am_doing": true}),
        success: () => {
            $('#config_reset_modal').modal('hide');
            util.postReboot(__("firmware_update.script.config_reset_init"), __("util.reboot_text"));
        },
        error: (xhr, status, error) => {
            $('#config_reset_modal').modal('hide');
            util.add_alert("config_reset_failed", "alert-danger", __("firmware_update.script.config_reset_error"), error + ": " + xhr.responseText);
        }
    });
}

function factory_reset() {
    $.ajax({
        url: `/factory_reset`,
        type: 'PUT',
        contentType: "application/json; charset=utf-8",
        data: JSON.stringify({"do_i_know_what_i_am_doing": true}),
        success: () => {
            $('#factory_reset_modal').modal('hide');
            util.postReboot(__("firmware_update.script.factory_reset_init"), __("util.reboot_text"));
        },
        error: (xhr, status, error) => {
            $('#factory_reset_modal').modal('hide');
            util.add_alert("factory_reset_failed", "alert-danger", __("firmware_update.script.factory_reset_error"), error + ": " + xhr.responseText);
        }
    });
}

export function init() {
    // Firmware upload
    $('#upload_firmware_form').on("submit", function (e) {
        e.preventDefault();
        check_upload("firmware");
    });

    $('#downgrade_confirm').on("click", () => {
        $('#downgrade_modal').modal('hide');
        upload("firmware");
    });

    $('#firmware_file_select').on("change", () => $("#update_firmware_button").prop("disabled", false));

    $('#spiffs_config_reset').on("click", () => $('#config_reset_modal').modal('show'));
    $('#spiffs_factory_reset').on("click", () => $('#factory_reset_modal').modal('show'));
    $('#config_reset_confirm').on("click", config_reset);
    $('#factory_reset_confirm').on("click", factory_reset);
    $('#firmware_update_reboot').on("click", util.reboot);

    bsCustomFileInput.init();
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('info/version', update_version);
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-flash').prop('hidden', !module_init.firmware_update);
    $('#config_reset_row').prop('hidden', !module_init.users);
}
