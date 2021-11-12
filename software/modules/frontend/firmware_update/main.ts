/* esp32-brick
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

declare function __(s: string): string;

import bsCustomFileInput from "bs-custom-file-input";

interface Version {
    firmware: string,
    spiffs: string
}

let last_version: string = null;

function update_version(version: Version) {
    if (last_version == null) {
        last_version = version.firmware;
    } else if (last_version != version.firmware) {
        window.location.reload();
    }

    $('#current_firmware').val(version.firmware);
    $('#current_spiffs').val(version.spiffs);
}

function upload(e: JQuery.SubmitEvent, type: string) {
    util.pauseWebSockets();
    e.preventDefault();

    let file_select = <HTMLInputElement>$(`#${type}_file_select`)[0];
    let progress = $(`#${type}-progress`);
    let select = $(`#${type}_select`);
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
                let txt = xhr.responseText.startsWith("firmware_update.") ? __(xhr.responseText) : error + ": " + xhr.responseText;
                util.add_alert("firmware_update_failed","alert-danger", __("firmware_update.script.flash_fail"), txt);
            }
            util.resumeWebSockets();
        }
    });
}

function factory_reset_modal() {
    $('#factory_reset_modal').modal('show');
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
    //Firmware upload
    $('#upload_firmware_form').on("submit", function (e) {
        upload(e, "firmware");
    });

    $('#firmware_file_select').on("change", () => $("#update_firmware_button").prop("disabled", false));

    $('#spiffs_factory_reset').on("click", factory_reset_modal);
    $('#factory_reset_confirm').on("click", factory_reset);
    $('#reboot').on("click", util.reboot);

    bsCustomFileInput.init();
}

export function addEventListeners(source: EventSource) {
    source.addEventListener('version', function (e: util.SSE) {
        update_version(<Version>(JSON.parse(e.data)));
    }, false);
}


export function updateLockState(module_init: any) {
    $('#sidebar-flash').prop('hidden', !module_init.firmware_update);
    $('#sidebar-system').prop('hidden',  $('#sidebar-system').prop('hidden') && !module_init.firmware_update);
}

export function getTranslation(lang: string) {
    const translations: {[index: string]:any} = {
        "de": {
            "firmware_update": {
                "status": {

                },
                "navbar": {
                    "flash": "Firmware-Aktualisierung"
                },
                "content": {
                    "current_firmware": "Firmware-Version",
                    "firmware_update": "Firmware-Aktualisierung",
                    "browse": "Durchsuchen",
                    "select_file": "Firmware-Datei",
                    "update": "Hochladen",
                    "uploading": "Firmware wird hochgeladen...",
                    "current_spiffs": "Konfigurationsversion",
                    "factory_reset_desc": "Setzt die gesamte Konfiguration auf den Auslieferungs&shy;zustand zurück",
                    "factory_reset": "Zurücksetzen",
                    "factory_reset_modal_text": "Hiermit wird die gesamte Konfiguration auf den Auslieferungszustand zurückgesetzt. <b>Diese Aktion kann nicht rückgängig gemacht werden!</b> Die Wallbox öffnet dann wieder einen eigenen WLAN-Access-Point mit dem Netzwerknamen (SSID) und der Passphrase aus dem Handbuch. Fortfahren?",
                    "abort_reset": "Abbrechen",
                    "confirm_reset": "Zurücksetzen auf Auslieferungszustand",
                    "reboot": "Neu starten",
                    "reboot_desc": "Startet das Webinterface neu. Eine laufende Ladung wird dabei nicht unterbrochen"
                },
                "script": {
                    "flash_success": "Erfolgreich aktualisiert; starte neu...",
                    "flash_fail": "Aktualisierung fehlgeschlagen",
                    "factory_reset_init": "Formatiere Konfigurationspartition und starte neu...",
                    "factory_reset_error": "Zurücksetzen auf Auslieferungszustand fehlgeschlagen",
                    "vehicle_connected": "Es kann keine Aktualisierung vorgenommen werden, während ein Fahrzeug verbunden ist.",
                    "no_info_page": "Firmware-Datei ist beschädigt oder für WARP 1 (Firmware-Info-Seite fehlt)",
                    "info_page_corrupted": "Firmware-Datei ist beschädigt (Checksummenfehler)",
                    "wrong_firmware_type": "Firmware-Datei ist für anderen Wallbox-Typ",
                }
            }
        },
        "en": {
            "firmware_update": {
                "status": {

                },
                "navbar": {
                    "flash": "Firmware Update"
                },
                "content": {
                    "current_firmware": "Firmware version",
                    "firmware_update": "Firmware Update",
                    "browse": "Browse",
                    "select_file": "Firmware file",
                    "update": "Upload",
                    "uploading": "Uploading firmware...",
                    "current_spiffs": "Configuration version",
                    "factory_reset_desc": "Restores all configuration to factory defaults",
                    "factory_reset": "Factory reset",
                    "factory_reset_modal_text": "A factory reset will reset all configuration to their default values. <b>This action cannot be undone!</b> The wallbox will re-open the WiFi access point with the default SSID and passphrase. Are you sure?",
                    "abort_reset": "Abort",
                    "confirm_reset": "Restore",
                    "reboot": "Reboot",
                    "reboot_desc": "Restarts the web interface. Charging will not be interrupted."
                },
                "script": {
                    "flash_success": "Successfully updated; restarting...",
                    "flash_fail": "Failed to update",
                    "factory_reset_init": "Formating configuration partition and restarting...",
                    "factory_reset_error": "Initiating factory reset failed",
                    "vehicle_connected": "Can't update the firmware while a vehicle is connected.",
                    "no_info_page": "Firmware file corrupted or for WARP 1 (info page missing)",
                    "info_page_corrupted": "Firmware file corrupted (checksum error)",
                    "wrong_firmware_type": "Firmware file is for another type of wallbox",
                }
            }
        }
    };
    return translations[lang];
}
