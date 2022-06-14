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

declare function __(s: string): string;

function wifi_symbol(rssi: number) {
    if(rssi >= -60)
        return `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><title>RSSI: ${rssi}</title><path d="M1.42 9a16 16 0 0 1 21.16 0"></path><path d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>`;
    if(rssi >= -70)
        return `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><title>RSSI: ${rssi}</title><path stroke="#cccccc" d="M1.42 9a16 16 0 0 1 21.16 0"></path><path d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>`;
    if(rssi >= -80)
        return `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><title>RSSI: ${rssi}</title><path stroke="#cccccc" d="M1.42 9a16 16 0 0 1 21.16 0"></path><path stroke="#cccccc" d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>`;

    return `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><title>RSSI: ${rssi}</title><path stroke="#cccccc" d="M1.42 9a16 16 0 0 1 21.16 0"></path><path stroke="#cccccc" d="M5 12.55a11 11 0 0 1 14.08 0"></path><path stroke="#cccccc" d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>`;
}

type WifiInfo = Exclude<API.getType['wifi/scan_results'], string>[0];

function update_wifi_scan_results(data: Readonly<WifiInfo[]>) {
    $("#wifi_config_scan_spinner").prop('hidden', true);
    $("#wifi_scan_results").prop('hidden', false);
    $("#wifi_scan_title").html(__("wifi.script.select_ap"));

    if (data.length == 0) {
        $("#wifi_scan_results").html(__("wifi.script.no_ap_found"));
        return;
    }
    let result = ``;

    // We want to show the BSSID if this is a hidden AP or we have multiple APs with the same SSID
    let tups = $.map(data, (v: WifiInfo, i) => {
        if (v.ssid == "")
            return [__("wifi.script.hidden_ap") + ` (${v.bssid})`, true];

        let result = [v.ssid, false];
        $.each(data, (i2, v2: WifiInfo) => {
            if (i != i2 && v2.ssid == v.ssid)
                result = [v.ssid + ` (${v.bssid})`, v.rssi - v2.rssi > 15];
        });
        return result;
    });

    $.each(data, (i, v: WifiInfo) => {
        let line = `<a id="wifi_scan_result_${i}" class="dropdown-item" href="#">${wifi_symbol(v.rssi)}<span class="ml-2" data-feather='${v.encryption == 0 ? 'unlock' : 'lock'}'></span><span class="pl-2">${tups[2*i]}</span></a>`;
        result += line;
    });

    $("#wifi_scan_results").html(result);
    $("#scan_wifi_button").dropdown('update')

    $.each(data, (i, v: WifiInfo) => {
        $(`#wifi_scan_result_${i}`).on("click", (event) => {
            event.preventDefault();
            connect_to_ap(v.ssid, v.bssid, v.encryption, <boolean>tups[2*i+1]);
        });
    });

    feather.replace();
}

let scan_timeout: number = null;
function scan_wifi() {
    $("#wifi_config_scan_spinner").prop('hidden', false);
    $("#wifi_scan_results").prop('hidden', true);

    API.call('wifi/scan', {}, __("wifi.script.scan_wifi_init_failed"))
       .catch(() => $('#scan_wifi_dropdown').dropdown('hide'))
       .then(() => {
            if (scan_timeout != null)
                window.clearTimeout(scan_timeout);

            scan_timeout = window.setTimeout(function () {
                    scan_timeout = null;
                    $.get("/wifi/scan_results").done(function (data: WifiInfo[]) {
                        update_wifi_scan_results(data);
                    }).fail((xhr, status, error) => {
                        util.add_alert("wifi_scan_failed", "alert-danger", __("wifi.script.scan_wifi_results_failed"), error + ": " + xhr.responseText);
                        $('#scan_wifi_dropdown').dropdown('hide');
                    });
                }, 12000);
        });
}


function update_wifi_sta_config() {
    let config = API.default_updater('wifi/sta_config', ['bssid']);

    $('#wifi_sta_config_bssid').val(config.bssid.map((x)=> (x < 16 ? '0' : '') + x.toString(16).toUpperCase()).join(":"));

    if(config.ip == "0.0.0.0") {
        $('#wifi_sta_config_show_static').val("hide");
        wifi_cfg_toggle_static_ip_collapse("hide");
    } else {
        $('#wifi_sta_config_show_static').val("show");
        wifi_cfg_toggle_static_ip_collapse("show");
    }
}

function set_wifi_ap_enable_select() {
    let config = API.get('wifi/ap_config');

    if(config.enable_ap && config.ap_fallback_only)
        $('#wifi_ap_config_enable_ap').val(1);
    else if(config.enable_ap)
        $('#wifi_ap_config_enable_ap').val(0);
    else
        $('#wifi_ap_config_enable_ap').val(2);
}

function update_wifi_ap_config() {
    API.default_updater('wifi/ap_config', ['ap_fallback_only', 'enable_ap']);
    set_wifi_ap_enable_select();
}

function update_wifi_state() {
    let state = API.default_updater('wifi/state', ['sta_ip', 'sta_rssi', 'sta_bssid'], false);

    if (state.sta_ip != "0.0.0.0") {
        $('#wifi_state_sta_ip').html(state.sta_ip);
        $('#wifi_state_sta_rssi').html(wifi_symbol(state.sta_rssi));
    }
}

function wifi_cfg_toggle_static_ip_collapse(value: string) {
    if (value == "hide") {
        $('#wifi_sta_config_static_ip_cfg').collapse('hide');
        $('#wifi_sta_config_ip').prop('required', false);
        $('#wifi_sta_config_subnet').prop('required', false);
        $('#wifi_sta_config_gateway').prop('required', false);
    }
    else if (value == "show") {
        $('#wifi_sta_config_static_ip_cfg').collapse('show');
        $('#wifi_sta_config_ip').prop('required', true);
        $('#wifi_sta_config_subnet').prop('required', true);
        $('#wifi_sta_config_gateway').prop('required', true);
    }
}

function connect_to_ap(ssid: string, bssid: string, encryption: number, enable_bssid_lock: boolean) {
    $('#wifi_sta_config_ssid').val(ssid);
    $('#wifi_sta_config_bssid').val(bssid);
    let passphrase_required = API.get("wifi/sta_config").ssid != ssid && encryption != 0;
    $('#wifi_sta_config_passphrase').prop("required", passphrase_required);
    $('#wifi_sta_config_passphrase').prop("placeholder", passphrase_required ? __("wifi.content.required") : __("wifi.content.unchanged"))
    $('#wifi_sta_config_enable_sta').prop("checked", true);
    $('#wifi_sta_config_bssid_lock').prop("checked", enable_bssid_lock);
    $('#wifi_sta_config_ssid').trigger("input");
    return;
}

export function add_event_listeners(source: API.APIEventTarget) {
    source.addEventListener('wifi/state', update_wifi_state);
    source.addEventListener('wifi/sta_config', update_wifi_sta_config);
    source.addEventListener('wifi/ap_config', update_wifi_ap_config);

    source.addEventListener('wifi/scan_results', (e) => {
        if (e.data == "scan in progress")
            return;

        window.clearTimeout(scan_timeout);
        scan_timeout = null;

        if (e.data == "scan failed") {
            console.log("scan failed");
            update_wifi_scan_results(JSON.parse("[]"));
            return;
        }

        if (typeof e.data !== "string")
            update_wifi_scan_results(e.data);
    }, false);
}

export function init() {
    $("#scan_wifi_button").on("click", scan_wifi);
    $("#wifi_sta_config_show_passphrase").on("change", util.toggle_password_fn("#wifi_sta_config_passphrase"));
    $("#wifi_sta_config_clear_passphrase").on("change", util.clear_password_fn("#wifi_sta_config_passphrase"));
    $("#wifi_ap_config_show_passphrase").on("change", util.toggle_password_fn("#wifi_ap_passphrase"));
    $("#wifi_ap_config_clear_passphrase").on("change", util.clear_password_fn("#wifi_ap_passphrase"));
    $("#wifi_sta_config_show_static").on("change", function(this: HTMLInputElement) {wifi_cfg_toggle_static_ip_collapse(this.value);});

    API.register_config_form('wifi/sta_config', () => {
            let dhcp = $('#wifi_sta_config_show_static').val() != "show";
            return {
                bssid: $('#wifi_sta_config_bssid').val().toString().split(':').map(x => parseInt(x, 16)),
                passphrase: util.passwordUpdate('#wifi_sta_config_passphrase'),
                ip: dhcp ? "0.0.0.0": $('#wifi_sta_config_ip').val().toString(),
                subnet: dhcp ? "0.0.0.0": $('#wifi_sta_config_subnet').val().toString(),
                gateway: dhcp ? "0.0.0.0": $('#wifi_sta_config_gateway').val().toString(),
                dns: dhcp ? "0.0.0.0": $('#wifi_sta_config_dns').val().toString(),
                dns2: dhcp ? "0.0.0.0": $('#wifi_sta_config_dns2').val().toString()
            }
        },
        () => $('#wifi_sta_config_ssid').prop("required", $('#wifi_sta_config_enable_sta').is(':checked')),
        __("wifi.script.sta_config_failed"),
        __("wifi.script.sta_reboot_content_changed")
    );


    $('#wifi_ap_config_enable_ap').on('change', () => {
        if ($('#wifi_ap_config_enable_ap').val() == 2) {
            $('#wifi_ap_disable').modal('show');
        }
    });

    $('#wifi_ap_not_disable_button').on('click', () => {
        set_wifi_ap_enable_select();
    });


    API.register_config_form('wifi/ap_config', () => ({
            enable_ap: $('#wifi_ap_config_enable_ap').val() != 2,
            ap_fallback_only: $('#wifi_ap_config_enable_ap').val() == 1,
            passphrase: util.passwordUpdate('#wifi_ap_config_passphrase'),
        }),
        undefined,
        __("wifi.script.ap_config_failed"),
        __("wifi.script.ap_reboot_content_changed"));

    $('#scan_wifi_dropdown').on('hidden.bs.dropdown', function (e) {
        $("#wifi_scan_title").html(__("wifi.content.sta_scanning"));
        $("#wifi_config_scan_spinner").prop('hidden', false);
        $("#wifi_scan_results").prop('hidden', true);
        $("#scan_wifi_button").dropdown('update');
    });
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-wifi-sta').prop('hidden', !module_init.wifi);
    $('#sidebar-wifi-ap').prop('hidden', !module_init.wifi);
}
