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

import "bootstrap";

import feather = require("feather-icons");

import * as util from "../util";

declare function __(s: string): string;

interface WifiInfo {
    ssid: string,
    bssid: string,
    rssi: number,
    channel: number,
    encryption: number
}

function wifi_symbol(rssi: number) {
    if(rssi >= -60)
        return '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><path d="M1.42 9a16 16 0 0 1 21.16 0"></path><path d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>';
    if(rssi >= -70)
        return '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><path d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>';
    if(rssi >= -80)
        return '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>';
    return '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>';
}

function update_wifi_scan_results(data: WifiInfo[]) {
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
        let button = document.getElementById(`wifi_scan_result_${i}`);
        button.addEventListener("click", () => connect_to_ap(v.ssid, v.bssid, v.encryption, <boolean>tups[2*i+1]));
    });

    feather.replace();
}

let scan_timeout: number = null;
function scan_wifi() {
    $.ajax({
        url: '/wifi/scan',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(null),
        error: (xhr, status, error) => util.add_alert("wifi_scan_failed", "alert-danger", __("wifi.script.scan_wifi_failed"), error + ": " + xhr.responseText),
        success: () => {
            scan_timeout = window.setTimeout(function () {
                    scan_timeout = null;
                    $.get("/wifi/scan_results").done(function (data: WifiInfo[]) {
                        update_wifi_scan_results(data);
                    });
                }, 10000);
        }
    });
}

interface WifiSTAConfig {
    enable_sta: boolean,
    ssid: string,
    bssid: number[],
    bssid_lock: boolean,
    passphrase: string,
    hostname: string,
    ip: number[],
    gateway: number[],
    subnet: number[],
    dns: number[],
    dns2: number[],
}

function update_wifi_sta_config(config: WifiSTAConfig) {
    // Remove the was-validated class to fix a visual bug
    // where saving the config triggers an update
    // that fills the elements, but clears the passphrase field.
    // An empty passphrase is invalid, so the input
    // field is marked as non-validated, confusing the user.
    let form = <HTMLFormElement>$('#wifi_sta_form')[0];
    form.classList.remove('was-validated');

    $('#wifi_sta_enable_sta').prop("checked", config.enable_sta);

    if (config.ssid != "") {
        $('#wifi_ap_configuration_state').val(config.ssid);
    }
    else {
        $('#wifi_ap_configuration_state').val(__("wifi.script.no_ap_configured"));
    }

    $('#wifi_sta_ssid').val(config.ssid);
    $('#wifi_sta_bssid').val(config.bssid.map((x)=> (x < 16 ? '0' : '') + x.toString(16).toUpperCase()).join(":"));
    $('#wifi_sta_bssid_lock').prop("checked", config.bssid_lock);
    $('#wifi_sta_passphrase').val(config.passphrase);
    $('#wifi_sta_hostname').val(config.hostname);
    if(config.ip.join(".") == "0.0.0.0") {
        $('#wifi_sta_show_static').val("hide");
        wifi_cfg_toggle_static_ip_collapse("hide");
    } else {
        $('#wifi_sta_show_static').val("show");
        wifi_cfg_toggle_static_ip_collapse("show");
    }
    $('#wifi_sta_ip').val(config.ip.join("."));
    $('#wifi_sta_gateway').val(config.gateway.join("."));
    $('#wifi_sta_subnet').val(config.subnet.join("."));
    $('#wifi_sta_dns').val(config.dns.join("."));
    $('#wifi_sta_dns2').val(config.dns2.join("."));
}

interface WifiAPConfig {
    enable_ap: boolean,
    ap_fallback_only: boolean,
    ssid: string,
    hide_ssid: boolean,
    passphrase: string,
    hostname: string,
    channel: number,
    ip: number[],
    gateway: number[],
    subnet: number[]
}

function update_wifi_ap_config(config: WifiAPConfig) {
    // Remove the was-validated class to fix a visual bug
    // where saving the config triggers an update
    // that fills the elements, but clears the passphrase field.
    // An empty passphrase is invalid, so the input
    // field is marked as non-validated, confusing the user.
    let form = <HTMLFormElement>$('#wifi_ap_form')[0];
    form.classList.remove('was-validated');

    $('#wifi_ap_ssid').val(config.ssid);
    $('#wifi_ap_hide_ssid').prop("checked", config.hide_ssid);
    $('#wifi_ap_passphrase').val(config.passphrase);
    $('#wifi_ap_hostname').val(config.hostname);
    $('#wifi_ap_channel').val(config.channel);
    $('#wifi_ap_ip').val(config.ip.join("."));
    $('#wifi_ap_gateway').val(config.gateway.join("."));
    $('#wifi_ap_subnet').val(config.subnet.join("."));

    if(config.enable_ap && config.ap_fallback_only)
        $('#wifi_ap_enable_ap').val(1);
    else if(config.enable_ap)
        $('#wifi_ap_enable_ap').val(0);
    else
        $('#wifi_ap_enable_ap').val(2);

    $('#wifi_save_enable_ap').prop("checked", config.enable_ap);
    $('#wifi_save_ap_fallback_only').prop("checked", config.ap_fallback_only);
}

interface WifiState {
    connection_state: number,
    ap_state: number,
    ap_bssid: string,
    sta_ip: number[],
    sta_rssi: number,
    sta_bssid: string
}

function update_wifi_state(state: WifiState) {
    let sta_button_dict: { [idx: number]: string } = {
        0: "#wifi_sta_not_configured",
        1: "#wifi_sta_not_connected",
        2: "#wifi_sta_connecting",
        3: "#wifi_sta_connected",
    };

    let sta_outline_dict: { [idx: number]: string } = {
        0: "btn-outline-primary",
        1: "btn-outline-danger",
        2: "btn-outline-warning",
        3: "btn-outline-success",
    };

    let sta_non_outline_dict: { [idx: number]: string } = {
        0: "btn-primary",
        1: "btn-danger",
        2: "btn-warning",
        3: "btn-success",
    };

    for (let i = 0; i < 4; ++i) {
        $(sta_button_dict[i]).removeClass(sta_non_outline_dict[i]);
        $(sta_button_dict[i]).addClass(sta_outline_dict[i]);
    }

    let button_to_highlight = state.connection_state;
    $(sta_button_dict[button_to_highlight]).removeClass(sta_outline_dict[button_to_highlight]);
    $(sta_button_dict[button_to_highlight]).addClass(sta_non_outline_dict[button_to_highlight]);


    let ap_button_dict: { [idx: number]: string } = {
        0: "#wifi_ap_disabled",
        1: "#wifi_ap_enabled",
        2: "#wifi_ap_fallback_inactive",
        3: "#wifi_ap_fallback_active",
    };

    let ap_outline_dict: { [idx: number]: string } = {
        0: "btn-outline-primary",
        1: "btn-outline-success",
        2: "btn-outline-success",
        3: "btn-outline-danger",
    };

    let ap_non_outline_dict: { [idx: number]: string } = {
        0: "btn-primary",
        1: "btn-success",
        2: "btn-success",
        3: "btn-danger",
    };

    for (let i = 0; i < 4; ++i) {
        $(ap_button_dict[i]).removeClass(ap_non_outline_dict[i]);
        $(ap_button_dict[i]).addClass(ap_outline_dict[i]);
    }

    button_to_highlight = state.ap_state
    $(ap_button_dict[button_to_highlight]).removeClass(ap_outline_dict[button_to_highlight]);
    $(ap_button_dict[button_to_highlight]).addClass(ap_non_outline_dict[button_to_highlight]);

    $('#wifi_ap_bssid').html(state.ap_bssid);

    if (state.sta_ip.join(".") != "0.0.0.0") {
        $('#status_wifi_sta_ip').html(state.sta_ip.join("."));
        $('#status_wifi_sta_rssi').html(wifi_symbol(state.sta_rssi));
    }
}

function wifi_cfg_toggle_static_ip_collapse(value: string) {
    if (value == "hide") {
        $('#wifi_sta_static_ip_cfg').collapse('hide');
        $('#wifi_sta_ip').prop('required', false);
        $('#wifi_sta_subnet').prop('required', false);
        $('#wifi_sta_gateway').prop('required', false);
    }
    else if (value == "show") {
        $('#wifi_sta_static_ip_cfg').collapse('show');
        $('#wifi_sta_ip').prop('required', true);
        $('#wifi_sta_subnet').prop('required', true);
        $('#wifi_sta_gateway').prop('required', true);
    }
}

function connect_to_ap(ssid: string, bssid: string, encryption: number, enable_bssid_lock: boolean) {
    $('#wifi_sta_ssid').val(ssid);
    $('#wifi_sta_bssid').val(bssid);
    $('#wifi_sta_passphrase').prop("required", encryption != 0);
    $('#wifi_sta_enable_sta').prop("checked", true);
    $('#wifi_sta_bssid_lock').prop("checked", enable_bssid_lock);
    return;
}

function parse_ip(ip_str: string) {
    let splt = ip_str.split('.');
    if (splt.length != 4)
        return [0, 0, 0, 0];

    let result: number[] = [];

    for (let i = 0; i < 4; ++i)
        result.push(parseInt(splt[i], 10));
    return result;
}

function save_wifi_sta_config(continuation = function () { }) {
    let dhcp = $('#wifi_sta_show_static').val() != "show";

    let payload: WifiSTAConfig = {
        enable_sta: $('#wifi_sta_enable_sta').is(':checked'),
        hostname: $('#wifi_sta_hostname').val().toString(),
        ssid: $('#wifi_sta_ssid').val().toString(),
        bssid: $('#wifi_sta_bssid').val().toString().split(':').map(x => parseInt(x, 16)),
        bssid_lock: $('#wifi_sta_bssid_lock').is(':checked'),
        passphrase: util.passwordUpdate('#wifi_sta_passphrase'),
        ip: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_sta_ip').val().toString()),
        subnet: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_sta_subnet').val().toString()),
        gateway: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_sta_gateway').val().toString()),
        dns: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_sta_dns').val().toString()),
        dns2: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_sta_dns2').val().toString())
    };

    $.ajax({
        url: '/wifi/sta_config_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: continuation,
        error: (xhr, status, error) => util.add_alert("wifi_sta_config_failed", "alert-danger", __("wifi.script.sta_config_failed"), error + ": " + xhr.responseText)
    });
}

function save_wifi_ap_config(continuation = function () { }) {

    let payload: WifiAPConfig = {
        enable_ap: $('#wifi_ap_enable_ap').val() != 2,
        ap_fallback_only: $('#wifi_ap_enable_ap').val() == 1,
        ssid: $('#wifi_ap_ssid').val().toString(),
        hide_ssid: $('#wifi_ap_hide_ssid').is(':checked'),
        passphrase: util.passwordUpdate('#wifi_ap_passphrase'),
        hostname: $('#wifi_ap_hostname').val().toString(),
        channel: parseInt($('#wifi_ap_channel').val().toString()),
        ip: parse_ip($('#wifi_ap_ip').val().toString()),
        subnet: parse_ip($('#wifi_ap_subnet').val().toString()),
        gateway: parse_ip($('#wifi_ap_gateway').val().toString()),
    };

    $.ajax({
        url: '/wifi/ap_config_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: continuation,
        error: (xhr, status, error) => util.add_alert("wifi_ap_config_failed", "alert-danger", __("wifi.script.ap_config_failed"), error + ": " + xhr.responseText),
    });
}

function wifi_save_reboot() {
    $('#wifi_reboot').modal('hide');
    util.reboot();
}

export function addEventListeners(source: EventSource) {
    source.addEventListener('wifi/state', function (e: util.SSE) {
        update_wifi_state(<WifiState>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('wifi/sta_config', function (e: util.SSE) {
        update_wifi_sta_config(<WifiSTAConfig>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('wifi/ap_config', function (e: util.SSE) {
        update_wifi_ap_config(<WifiAPConfig>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('wifi/scan_results', function (e: util.SSE) {
        if (e.data == "scan in progress")
            return;

        window.clearTimeout(scan_timeout);
        scan_timeout = null;

        if (e.data == "scan failed") {
            console.log("scan failed");
            update_wifi_scan_results(JSON.parse("[]"));
            return;
        }

        update_wifi_scan_results(JSON.parse(e.data));
    }, false);
}

export function init() {
    (<HTMLButtonElement>document.getElementById("scan_wifi_button")).addEventListener("click", scan_wifi);

    (<HTMLButtonElement>document.getElementById("wifi_reboot_button")).addEventListener("click", wifi_save_reboot);

    let sta_show_button = <HTMLButtonElement>document.getElementById("wifi_sta_show_passphrase");
    sta_show_button.addEventListener("change", util.toggle_password_fn("#wifi_sta_passphrase"));

    let sta_clear_button = <HTMLButtonElement>document.getElementById("wifi_sta_clear_passphrase");
    sta_clear_button.addEventListener("change", util.clear_password_fn("#wifi_sta_passphrase"));

    let ap_show_button = <HTMLButtonElement>document.getElementById("wifi_ap_show_passphrase");
    ap_show_button.addEventListener("change", util.toggle_password_fn("#wifi_ap_passphrase"));

    let ap_clear_button = <HTMLButtonElement>document.getElementById("wifi_ap_clear_passphrase");
    ap_clear_button.addEventListener("change", util.clear_password_fn("#wifi_ap_passphrase"));


    let ap_ip_config = <HTMLInputElement>document.getElementById("wifi_sta_show_static");
    ap_ip_config.addEventListener("change", () => wifi_cfg_toggle_static_ip_collapse(ap_ip_config.value));

    // Use bootstrap form validation
    let form = <HTMLFormElement>$('#wifi_sta_form')[0];
    form.addEventListener('submit', function (event: Event) {
        form.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (form.checkValidity() === false) {
            return;
        }
        save_wifi_sta_config(() => $('#wifi_reboot').modal('show'));
    }, false);

    let form2 = <HTMLFormElement>$('#wifi_ap_form')[0];
    form2.addEventListener('submit', function (event: Event) {
        form2.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (form2.checkValidity() === false) {
            return;
        }
        save_wifi_ap_config(() => $('#wifi_reboot').modal('show'));
    }, false);

    $('#scan_wifi_dropdown').on('hidden.bs.dropdown', function (e) {
        $("#wifi_scan_title").html(__("wifi.content.sta_scanning"));
        $("#wifi_config_scan_spinner").prop('hidden', false);
        $("#wifi_scan_results").prop('hidden', true);
        $("#scan_wifi_button").dropdown('update');
    });
}


export function updateLockState(module_init: any) {
    $('#sidebar-wifi-sta').prop('hidden', !module_init.wifi);
    $('#sidebar-wifi-ap').prop('hidden', !module_init.wifi);
}

export function getTranslation(lang: string) {
    const translations: {[index: string]:any} = {
        "de": {
            "wifi": {
                "status": {
                    "wifi_connection": "WLAN-Verbindung",
                    "not_configured": "Unkonfiguriert",
                    "not_connected": "Getrennt",
                    "connecting": "Verbinde",
                    "connected": "Verbunden",
                    "wifi_ap": "WLAN-Access-Point",
                    "deactivated": "Deaktiviert",
                    "activated": "Aktiviert",
                    "fallback_inactive": "Fallback inaktiv",
                    "fallback_active": "Fallback aktiv"
                },
                "navbar": {
                    "wifi_sta": "WLAN-Verbindung",
                    "wifi_ap": "WLAN-Access-Point",
                },
                "content": {
                    "sta_settings": "WLAN-Verbindungs&shy;einstellungen",
                    "sta_enable_sta": "Verbindung aktiviert",
                    "sta_enable_sta_desc": "Wenn aktiviert, verbindet sich das Gerät beim Start automatisch zum konfigurierten Netzwerk.",
                    "sta_ssid": "Netzwerkname (SSID)",
                    "sta_scan": "Netzwerksuche",
                    "sta_scanning": "Suche Netzwerke...",
                    "sta_bssid": "BSSID",
                    "sta_bssid_lock": "BSSID-Sperre",
                    "sta_bssid_invalid": "Die BSSID muss aus sechs Gruppen mit jeweils einer zweistelligen Hexadezimalzahl, getrennt durch einen Doppelpunkt, bestehen. Zum Beispiel 01:23:45:67:89:AB",
                    "sta_bssid_lock_desc": "Verbinde nur zur konfigurierten BSSID. Bei Einsatz mehrerer Access Points und/oder Repeater mit demselben Netzwerknamen wird so immer derselbe AP oder Repeater verwendet.",
                    "sta_passphrase": "Passphrase",
                    "sta_passphrase_invalid": "Die Passphrase muss zwischen 8 und 63 ASCII-Zeichen lang sein.",
                    "sta_hostname": "Hostname",
                    "sta_hostname_invalid": "Der Hostname darf nur aus den Groß- und Kleinbuchstaben A-Z und a-z, sowie den Ziffern 0-9 und Bindestrichen bestehen. Zum Beispiel warp-A1c",
                    "sta_ip_configuration": "IP-Konfiguration",
                    "sta_dhcp": "automatisch (DHCP)",
                    "sta_static": "statisch",
                    "save": "Speichern",

                    "reboot_title": "Neu starten um Konfiguration anzuwenden",
                    "reboot_content": "Die geänderten WLAN-Einstellungen werden nur nach einem Neustart angewendet. Jetzt neu starten?",
                    "abort": "Abbrechen",
                    "reboot": "Neu starten",

                    "static_ip": "IP-Adresse",
                    "subnet": "Subnetzmaske",
                    "gateway": "Gateway",
                    "dns": "<span class=\"form-label pr-2\">DNS-Server</span><span class=\"text-muted\">optional</span>",
                    "dns2": "<span class=\"form-label pr-2\">Alternativer DNS-Server</span><span class=\"text-muted\">optional</span>",
                    "static_ip_invalid": "Die IP-Adresse muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.2",
                    "subnet_invalid": "Die Subnetzmaske muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 255.255.255.0",
                    "gateway_invalid": "Die IP des Gateways muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1",
                    "dns_invalid": "Die IP des DNS-Servers muss entweder leer bleiben, oder aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1.",
                    "dns2_invalid": "Die IP des alternativen DNS-Servers muss entweder leer bleiben, oder aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1.",

                    "ap_settings": "WLAN-Access-Point-Einstellungen",
                    "ap_enable": "<span class=\"form-label pr-2\">Access Point</span><span class=\"text-muted\"><a href=\"https://www.warp-charger.com/#documents\">siehe Betriebsanleitung für Details</a></span></label>",
                    "ap_enabled": "aktiviert",
                    "ap_fallback_only": "nur als Fallback",
                    "ap_disabled": "deaktiviert",
                    "ap_ssid": "Netzwerkname (SSID)",
                    "ap_hide_ssid": "Netzwerknamen versteckt",
                    "ap_hide_ssid_desc_pre": "Versteckt den Netzwerknamen. Das Gerät ist unter der BSSID ",
                    "ap_hide_ssid_desc_post": " erreichbar.",
                    "ap_passphrase": "Passphrase",
                    "ap_passphrase_invalid": "Die Passphrase muss zwischen 8 und 63 ASCII-Zeichen lang sein.",
                    "ap_hostname": "Hostname",
                    "ap_hostname_invalid": "Der Hostname darf nur aus den Groß- und Kleinbuchstaben A-Z und a-z, sowie den Ziffern 0-9 und Bindestrichen bestehen. Zum Beispiel warp-A1c",
                    "ap_channel": "<span class=\"form-label pr-2\">Kanal</span><span class=\"text-muted\">ignoriert wenn WLAN-Verbindung aktiv ist</span>",
                    "ap_ip": "IP-Adresse",
                    "ap_subnet": "Subnetzmaske",
                    "ap_gateway": "Gateway",
                    "ap_ip_invalid": "Die IP-Adresse muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.2",
                    "ap_subnet_invalid": "Die Subnetzmaske muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 255.255.255.0",
                    "ap_gateway_invalid": "Die IP des Gateways muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1",

                    "unchanged": "unverändert"
                },
                "script": {
                    "scan_wifi_failed": "Suche nach Netzwerken fehlgeschlagen",
                    "select_ap": "Gefundene Netzwerke",
                    "no_ap_found": "Kein Netzwerk gefunden.",
                    "hidden_ap": "[Versteckter AP]",

                    "sta_config_failed": "Speichern der Verbindungseinstellungen fehlgeschlagen.",
                    "ap_config_failed": "Speichern der Access Point-Einstellungen fehlgeschlagen.",
                    "no_ap_configured": "Keine Verbindung konfiguriert"
                }
            }
        },
        "en": {
            "wifi": {
                "status": {
                    "wifi_connection": "WiFi connection",
                    "not_configured": "Not configured",
                    "not_connected": "Not connected",
                    "connecting": "Connecting",
                    "connected": "Connected",
                    "wifi_ap": "WiFi access point",
                    "deactivated": "Disabled",
                    "activated": "Enabled",
                    "fallback_inactive": "Fallback inactive",
                    "fallback_active": "Fallback active"
                },
                "navbar": {
                    "wifi_sta": "WiFi Connection",
                    "wifi_ap": "WiFi Access Point",
                },
                "content": {
                    "sta_settings": "WiFi Connection Settings",
                    "sta_enable_sta": "Connection enabled",
                    "sta_enable_sta_desc": "Automatically connects to the configured network on start-up if activated.",
                    "sta_ssid": "Network name (SSID)",
                    "sta_scan": "Search for networks",
                    "sta_scanning": "Searching for networks...",
                    "sta_bssid": "BSSID",
                    "sta_bssid_invalid": "The BSSID must have six groups separated by a colon, each with a two-digit hexadecimal number. For example 01:23:45:67:89:AB",
                    "sta_bssid_lock": "BSSID lock",
                    "sta_bssid_lock_desc": "Connect only to the network with the configured BSSID. Leave disabled if you use multiple access points or repeaters with with the same network name.",
                    "sta_passphrase": "Passphrase",
                    "sta_passphrase_invalid": "The passphrase must be 8-63 ASCII characters.",
                    "sta_hostname": "Hostname",
                    "sta_hostname_invalid": "The hostname must contain only upper and lower case letters A-Z and a-z the digits 0-9 or dashes. For example warp-A1c",
                    "sta_ip_configuration": "IP configuration",
                    "sta_dhcp": "automatic (DHCP)",
                    "sta_static": "static",
                    "save": "Save",

                    "reboot_title": "Restart to apply configuration",
                    "reboot_content": "The modified WiFi configuration will only be applied after a restart. Reboot now?",
                    "abort": "Abort",
                    "reboot": "Reboot",

                    "static_ip": "IP address",
                    "subnet": "Subnet mask",
                    "gateway": "Gateway",
                    "dns": "<span class=\"form-label pr-2\">DNS server</span><span class=\"text-muted\">optional</span>",
                    "dns2": "<span class=\"form-label pr-2\">Alternative DNS server</span><span class=\"text-muted\">optional</span>",
                    "static_ip_invalid": "The IP address must have four groups between 0 and 255 separated with a dot. For example 10.0.0.2",
                    "subnet_invalid": "The subnet mask must have four groups between 0 and 255 separated with a dot. For example 255.255.255.0",
                    "gateway_invalid": "The gateway IP must have four groups between 0 and 255 separated with a dot. For example 10.0.0.1",
                    "dns_invalid": "The DNS server IP must have four groups between 0 and 255 separated with a dot. For example 10.0.0.1, or be empty.",
                    "dns2_invalid": "The alternative DNS server IP must have four groups between 0 and 255 separated with a dot. For example 10.0.0.1, or be empty.",

                    "ap_settings": "WiFi Access Point Settings",
                    "ap_enable": "<span class=\"form-label pr-2\">Access point</span><span class=\"text-muted\"><a href=\"https://www.warp-charger.com/#documents\">see manual for details</a></span></label>",
                    "ap_enabled": "enabled",
                    "ap_fallback_only": "as fallback only",
                    "ap_disabled": "disabled",
                    "ap_ssid": "Network name (SSID)",
                    "ap_hide_ssid": "Network name hidden",
                    "ap_hide_ssid_desc_pre": "Hides the network name. The device is reachable under the BSSID ",
                    "ap_hide_ssid_desc_post": ".",
                    "ap_passphrase": "Passphrase",
                    "ap_passphrase_invalid": "The passphrase must be 8-63 ASCII characters.",
                    "ap_hostname": "Hostname",
                    "ap_hostname_invalid": "The hostname must contain only upper and lower case letters A-Z and a-z the digits 0-9 or dashes. For example warp-A1c",
                    "ap_channel": "<span class=\"form-label pr-2\">Channel</span><span class=\"text-muted\">ignored if WiFi connection is active</span>",
                    "ap_ip": "IP address",
                    "ap_subnet": "Subnet mask",
                    "ap_gateway": "Gateway",
                    "ap_ip_invalid": "The IP address must have four groups between 0 and 255 separated with a dot. For example 10.0.0.2",
                    "ap_subnet_invalid": "The subnet mask must have four groups between 0 and 255 separated with a dot. For example 255.255.255.0",
                    "ap_gateway_invalid": "The gateway IP must have four groups between 0 and 255 separated with a dot. For example 10.0.0.1",

                    "unchanged": "unchanged"
                },
                "script": {
                    "scan_wifi_failed": "Search for networks failed",
                    "select_ap": "Found networks",
                    "no_ap_found": "No network found",
                    "hidden_ap": "[Hidden AP]",

                    "sta_config_failed": "Failed to save connection configuration.",
                    "ap_config_failed": "Failed to save access point configuration.",
                    "no_ap_configured": "No connection configured"
                }
            }
        }
    };
    return translations[lang];
}
