/* esp32-firmware
 * Copyright (C) 2021 Erik Fleckstein <erik@tinkerforge.com>
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

declare function __(s: string): string;

let charger_state_count = -1;

const MAX_CONTROLLED_CHARGERS = 10;

let charger_add_symbol = '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-server" style=""><rect x="2" y="14" width="20" height="8" rx="2" ry="2"></rect><line y1="18" y2="18" x1="18" x2="18.01"></line><line x1="19" x2="19" y1="3" y2="9"></line><line x1="22" x2="16" y1="6" y2="6"></line></svg>'
let charger_delete_symbol = '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-server mr-2" style=""><rect x="2" y="14" width="20" height="8" rx="2" ry="2"></rect><line y1="18" y2="18" x1="18" x2="18.01"></line><line x1="17" x2="22" y1="4" y2="9"></line><line x1="22" x2="17" y1="4" y2="9"></line></svg>'
let charger_symbol = '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-server" style=""><rect x="2" y="8" width="20" height="8" rx="2" ry="2"></rect><line y1="12" y2="12" x1="18" x2="18.01"></line></svg>'

function update_charge_manager_state() {
    let state = API.get('charge_manager/state');

    if (state.chargers.length != charger_state_count) {
        let charger_status = "";
        for (let i = 0; i < state.chargers.length; i++) {
            if (i % 2 == 0) {
                if (i != 0) {
                    charger_status += '</div>';
                }
                charger_status += '<div class="card-deck mb-4">';
            }

            charger_status += `
            <div class="card">
                <h5 id="charge_manager_status_charger_${i}_name" class="card-header"></h5>
                <div id="charge_manager_status_charger_${i}_body" class="card-body">
                    <h5 id="charge_manager_status_charger_${i}_state" class="card-title"></h5>
                    <p id="charge_manager_status_charger_${i}_info" class="card-text"></p>
                </div>
                <div class="card-footer">
                    <span id="charge_manager_status_charger_${i}_details"></span>
                </div>
            </div>
            `
        }
        charger_status += '</div>';
        $('#charge_manager_status_chargers').html(charger_status);
        charger_state_count = state.chargers.length;
        $('#charge_manager_status_controlled_chargers').prop('hidden', charger_state_count == 0);
        $('#charge_manager_status_available_current_form').prop('hidden', charger_state_count == 0);
    }
    for (let i = 0; i < state.chargers.length; i++) {
        const s = state.chargers[i];

        $(`#charge_manager_status_charger_${i}_name`).text(s.name);
        if (s.state != 5) {
            if (state.state == 2) {
                $(`#charge_manager_status_charger_${i}_body`).addClass("bg-danger text-white bg-disabled");
                $(`#charge_manager_status_charger_${i}_state`).text(__("charge_manager.script.charge_state_blocked_by_other_box"));
                $(`#charge_manager_status_charger_${i}_info`).text(__("charge_manager.script.charge_state_blocked_by_other_box_details"));
            } else {
                $(`#charge_manager_status_charger_${i}_body`).removeClass("bg-danger text-white bg-disabled");
                $(`#charge_manager_status_charger_${i}_state`).text(__(`charge_manager.script.charge_state_${s.state}`));
                $(`#charge_manager_status_charger_${i}_info`).text(util.toLocaleFixed(s.allocated_current / 1000.0, 3) + " " + __("charge_manager.script.ampere_allocated"));
            }
        }
        else {
            if (s.error < 192)
                $(`#charge_manager_status_charger_${i}_state`).text(__("charge_manager.script.charge_error_type_management"));
            else
                $(`#charge_manager_status_charger_${i}_state`).text(__("charge_manager.script.charge_error_type_client"));

            $(`#charge_manager_status_charger_${i}_body`).addClass("bg-danger text-white bg-disabled");
            $(`#charge_manager_status_charger_${i}_info`).text(__(`charge_manager.script.charge_error_${s.error}`));
        }

        let last_update = Math.floor((state.uptime - s.last_update) / 1000);
        let status_text = util.toLocaleFixed(s.supported_current / 1000.0, 3) + " " + __("charge_manager.script.ampere_supported");

        if (last_update >= 10)
            status_text += "; " + __("charge_manager.script.last_update_prefix") + util.format_timespan(last_update) + (__("charge_manager.script.last_update_suffix"));
        $(`#charge_manager_status_charger_${i}_details`).text(status_text);
    }

    util.update_button_group("btn_group_charge_manager_state", state.state);
}

let charger_config_count = -1;

function set_available_current(current: number) {
    $('#charge_manager_status_available_current_save').prop("disabled", true);
    API.save("charge_manager/available_current", {"current": current},__("charge_manager.script.set_available_current_failed"))
       .then(() => $('#charge_manager_status_available_current_save').html(feather.icons.check.toSvg()))
       .catch(error => $('#charge_manager_status_available_current_save').prop("disabled", false));
}

function update_available_current(current: number = API.get('charge_manager/available_current').current) {
    if($('#charge_manager_status_available_current_save').prop("disabled")) {
        util.setNumericInput("charge_manager_status_available_current", current / 1000, 3);
    }
}

function update_charge_manager_config(config: ChargeManagerConfig = API.get('charge_manager/config'), force: boolean) {
    $('#charge_manager_status_available_current').prop("max", config.maximum_available_current / 1000.0);
    $("#charge_manager_status_available_current_maximum").on("click", () => set_available_current(config.default_available_current));
    $('#charge_manager_status_available_current_maximum').html(util.toLocaleFixed(config.default_available_current / 1000.0, 0) + " A");

    update_available_current(config.default_available_current);

    if (!force && !$('#charge_manager_save_button').prop("disabled"))
        return;

    $('#charge_manager_enable').prop("checked", config.enable_charge_manager);
    $('#charge_manager_enable_watchdog').prop("checked", config.enable_watchdog);
    $('#charge_manager_verbose').prop("checked", config.verbose);
    util.setNumericInput("charge_manager_default_available_current", config.default_available_current / 1000, 3);
    util.setNumericInput("charge_manager_maximum_available_current", config.maximum_available_current / 1000, 3);
    util.setNumericInput("charge_manager_minimum_current", config.minimum_current / 1000, 3);

    if (config.chargers.length != charger_config_count) {
        let charger_configs = "";
        for (let i = 0; i < config.chargers.length; i++) {
            charger_configs += `<div class="col mb-4">
                    <div class="card h-100">
                        <div class="card-header d-flex justify-content-between align-items-center">
                            ${charger_symbol}
                            <button type="button" class="btn btn-sm btn-outline-dark"
                                id="charge_manager_content_${i}_remove">
                                ${charger_delete_symbol}<span style="font-size: 1rem; vertical-align: middle;">${__("charge_manager.script.delete")}</span>
                            </button>
                        </div>

                        <div class="card-body">
                        <div class="form-group">
                                <label class="form-label" for="charge_manager_config_charger_${i}_name">${__("charge_manager.script.display_name")}</label>
                                <input type="text" class="form-control" id="charge_manager_config_charger_${i}_name">
                            </div>
                            <div class="form-group">
                                <label class="form-label" for="charge_manager_config_charger_${i}_host">${__("charge_manager.script.host")}</label>
                                <input type="text" class="form-control" id="charge_manager_config_charger_${i}_host">
                            </div>
                        </div>
                    </div>
                </div>`;
        }
        charger_configs += `<div class="col mb-4">
        <div class="card h-100">
            <div class="card-header d-flex justify-content-between align-items-center">
                ${charger_add_symbol}
                <button type="button" class="btn btn-sm btn-outline-dark" style="visibility: hidden;">
                    ${charger_delete_symbol}
                </button>
            </div>
            <div class="card-body">
                <button id="charge_manager_add_charger" type="button" class="btn btn-light btn-lg btn-block" style="height: 100%;" data-toggle="modal" data-target="#charge_manager_add_charger_modal">${__("charge_manager.script.add_charger")}</button>
                <span id="charge_manager_add_charger_disabled" hidden>${__("charge_manager.script.add_charger_disabled_prefix") + MAX_CONTROLLED_CHARGERS + __("charge_manager.script.add_charger_disabled_suffix")}</span>
            </div>
        </div>
    </div>`;
        $('#charge_manager_configs').html(charger_configs);
        charger_config_count = config.chargers.length;
        feather.replace();
        for (let i = 0; i < config.chargers.length; i++) {
            $(`#charge_manager_content_${i}_remove`).on("click", () => {
                $('#charge_manager_save_button').prop("disabled", false);
                update_charge_manager_config(collect_charge_manager_config(null, i), true);
            });
        }
    }

    $('#charge_manager_add_charger').prop("hidden", config.chargers.length >= MAX_CONTROLLED_CHARGERS);
    $('#charge_manager_add_charger_disabled').prop("hidden", config.chargers.length < MAX_CONTROLLED_CHARGERS);

    for (let i = 0; i < config.chargers.length; i++) {
        const s = config.chargers[i];
        $(`#charge_manager_config_charger_${i}_name`).val(s.name);
        $(`#charge_manager_config_charger_${i}_host`).val(s.host);
    }
}

type ChargeManagerConfig = API.getType['charge_manager/config'];
type ChargerConfig = ChargeManagerConfig["chargers"][0];

function collect_charge_manager_config(new_charger: ChargerConfig = null, remove_charger: number = null) : ChargeManagerConfig {
    let chargers: ChargerConfig[] = [];
    for(let i = 0; i < charger_config_count; ++i) {
        if (remove_charger !== null && i == remove_charger)
            continue;
        let c: ChargerConfig = {
            host: $(`#charge_manager_config_charger_${i}_host`).val().toString(),
            name: $(`#charge_manager_config_charger_${i}_name`).val().toString(),
        }
        chargers.push(c);
    }
    if (new_charger != null)
        chargers.push(new_charger);

    return {
       enable_charge_manager: $('#charge_manager_enable').is(':checked'),
       enable_watchdog: $('#charge_manager_enable_watchdog').is(':checked'),
       verbose: $('#charge_manager_verbose').is(':checked'),
       default_available_current: Math.round(<number>$('#charge_manager_default_available_current').val() * 1000),
       maximum_available_current: Math.round(<number>$('#charge_manager_maximum_available_current').val() * 1000),
       minimum_current: Math.round(<number>$('#charge_manager_minimum_current').val() * 1000),
       chargers: chargers
    };
}

function save_charge_manager_config() {
    let payload = collect_charge_manager_config();
    API.save('charge_manager/config', payload, __("charge_manager.script.save_failed"), __("charge_manager.script.reboot_content_changed"))
       .then(() => $('#charge_manager_save_button').prop("disabled", true));
}

export function init() {
    $('#charge_manager_config_form').on('input', () => $('#charge_manager_save_button').prop("disabled", false));

    $('#charge_manager_config_form').on('submit', function (this: HTMLFormElement, event: Event) {
        $('#charge_manager_default_available_current').prop("max", $('#charge_manager_maximum_available_current').val());

        this.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }

        save_charge_manager_config();
    });

    $('#charge_manager_add_charger_form').on("submit", (event: Event) => {
        let form = <HTMLFormElement>$('#charge_manager_add_charger_form')[0];

        form.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (form.checkValidity() === false) {
            return;
        }

        $('#charge_manager_add_charger_modal').modal('hide');
        $('#charge_manager_save_button').prop("disabled", false);

        let new_config = collect_charge_manager_config({
            host: $(`#charge_manager_config_charger_new_host`).val().toString(),
            name: $(`#charge_manager_config_charger_new_name`).val().toString(),
        }, null);

        update_charge_manager_config(new_config, true);
    });

    $("#charge_manager_status_available_current_minimum").on("click", () => set_available_current(0));

    $('#charge_manager_status_available_current').on("input", () => {
        $('#charge_manager_status_available_current_save').html(feather.icons.save.toSvg());
        $('#charge_manager_status_available_current_save').prop("disabled", false);
    });

    $('#charge_manager_status_available_current_form').on('submit', function (this: HTMLFormElement, event: Event) {
        event.preventDefault();
        event.stopPropagation();

        if (this.checkValidity() === false) {
            return;
        }

        set_available_current(Math.round(<number>$('#charge_manager_status_available_current').val() * 1000));
    });
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('charge_manager/state', update_charge_manager_state);
    source.addEventListener('charge_manager/config', () => update_charge_manager_config(undefined, false));
    source.addEventListener('charge_manager/available_current', () => update_available_current());
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-charge_manager').prop('hidden', !module_init.charge_manager);
}
