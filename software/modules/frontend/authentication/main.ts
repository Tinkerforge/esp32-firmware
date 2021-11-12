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

interface AuthenticationConfig {
    enable_auth: boolean,
    username: string,
    password: string
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
    let payload: AuthenticationConfig = {
        enable_auth: $('#authentication_enable').is(':checked'),
        username: $('#authentication_username').val().toString(),
        password: util.passwordUpdate('#authentication_password'),
    };

    $.ajax({
        url: '/authentication/config_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: () => $('#authentication_reboot').modal('show'),
        error: (xhr, status, error) => util.add_alert("authentication_config_update_failed", "alert-danger", __("authentication.script.save_failed"), error + ": " + xhr.responseText)
    });
}


export function init() {
    let show_button = <HTMLButtonElement>document.getElementById("authentication_show_password");
    show_button.addEventListener("change", util.toggle_password_fn("#authentication_password"));

    let auth_button = <HTMLButtonElement>document.getElementById("authentication_enable");
    auth_button.addEventListener("change", (ev: Event) => {
        let x = <HTMLInputElement>ev.target;
        $('#authentication_username').prop("disabled", !x.checked);
        $('#authentication_password').prop("disabled", !x.checked);
        $('#authentication_show_password').prop("disabled", !x.checked);
        if (!x.checked) {
            $('#authentication_show_password').prop("checked", false);
        }

        $('#authentication_password').val('');
        let auth_placeholder = "";

        if (!$('#authentication_password').prop("required"))
            // If the field is not required, a password is currently stored.
            // if auth is to be enabled, the stored password can be used (-> unchanged if empty)
            // if auth is to be disabled, the stored password will be cleared
            auth_placeholder = x.checked ?  __("util.unchanged") : __("util.to_be_cleared");

        $('#authentication_password').attr("placeholder", auth_placeholder);
    });


    let form = <HTMLFormElement>$('#authentication_config_form')[0];
    form.addEventListener('submit', function (event: Event) {
        form.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (form.checkValidity() === false) {
            return;
        }

        save_authentication_config();
    }, false);

    (<HTMLButtonElement>document.getElementById("authentication_reboot_button")).addEventListener("click", () => {
        $('#authentication_reboot').modal('hide');
        util.reboot();
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

export function getTranslation(lang: string) {
    const translations: {[index: string]:any} = {
        "de": {
            "authentication": {
                "status": {

                },
                "navbar": {
                    "authentication": "Zugangsdaten"
                },
                "content": {
                    "authentication": "Zugangsdaten",
                    "enable_authentication": "Anmeldung aktiviert",
                    "enable_authentication_desc": "Wenn aktiviert, muss beim Aufrufen des Webinterfaces oder bei Verwendung der HTTP-API eine Anmeldung mit den konfigurierten Zugangsdaten durchgeführt werden.",
                    "username": "Benutzername",
                    "password": "Passwort",
                    "save": "Speichern",
                    "reboot_title": "Neu starten um Konfiguration anzuwenden",
                    "reboot_content": "Die geänderten Zugangsdaten werden nur nach einem Neustart angewendet. Jetzt neu starten?",
                    "abort": "Abbrechen",
                    "reboot": "Neu starten",
                    "username_invalid": "Der Benutzername darf nicht leer sein.",
                    "password_invalid": "Das Passwort darf nicht leer sein.",
                },
                "script": {
                    "save_failed": "Speichern der Zugangsdaten fehlgeschlagen.",
                }
            }
        },
        "en": {
            "authentication": {
                "status": {

                },
                "navbar": {
                    "authentication": "Credentials"
                },
                "content": {
                    "authentication": "Credentials",
                    "enable_authentication": "Enable authentication",
                    "enable_authentication_desc": "If activated, the configured credentials must be entered to open the web interface or use the HTTP API.",
                    "username": "Username",
                    "password": "Password",
                    "save": "Save",
                    "reboot_title": "Reboot to apply configuration",
                    "reboot_content": "The changed credentials will only be applied after rebooting. Reboot now?",
                    "abort": "Abort",
                    "reboot": "Reboot",
                    "username_invalid": "An empty username is not allowed.",
                    "password_invalid": "An empty password is not allowed.",
                },
                "script": {
                    "save_failed": "Failed to save the credentials.",
                }
            }
        }
    };
    return translations[lang];
}
