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

interface EthernetConfig {
    enable_ethernet: boolean,
    hostname: string,
    ip: number[],
    gateway: number[],
    subnet: number[],
    dns: number[],
    dns2: number[],
}

function update_ethernet_config(config: EthernetConfig) {
    // Remove the was-validated class to fix a visual bug
    // where saving the config triggers an update
    // that fills the elements, but clears the passphrase field.
    // An empty passphrase is invalid, so the input
    // field is marked as non-validated, confusing the user.
    let form = <HTMLFormElement>$('#ethernet_form')[0];
    form.classList.remove('was-validated');

    $('#ethernet_enable').prop("checked", config.enable_ethernet);

    $('#ethernet_hostname').val(config.hostname);
    if(config.ip.join(".") == "0.0.0.0") {
        $('#ethernet_show_static').val("hide");
        ethernet_cfg_toggle_static_ip_collapse("hide");
    } else {
        $('#ethernet_show_static').val("show");
        ethernet_cfg_toggle_static_ip_collapse("show");
    }
    $('#ethernet_ip').val(config.ip.join("."));
    $('#ethernet_gateway').val(config.gateway.join("."));
    $('#ethernet_subnet').val(config.subnet.join("."));
    $('#ethernet_dns').val(config.dns.join("."));
    $('#ethernet_dns2').val(config.dns2.join("."));
}

interface EthernetState {
    connection_state: number,
    ip: number[],
    full_duplex: boolean,
    link_speed: number,
}

function update_ethernet_state(state: EthernetState) {
    util.update_button_group("btn_group_ethernet_state", state.connection_state);

    if (state.ip.join(".") != "0.0.0.0") {
        $('#status_ethernet_ip').html(state.ip.join("."));
    } else {
        $('#status_ethernet_ip').html("");
    }
}

function ethernet_cfg_toggle_static_ip_collapse(value: string) {
    if (value == "hide") {
        $('#ethernet_static_ip_cfg').collapse('hide');
        $('#ethernet_ip').prop('required', false);
        $('#ethernet_subnet').prop('required', false);
        $('#ethernet_gateway').prop('required', false);
    }
    else if (value == "show") {
        $('#ethernet_static_ip_cfg').collapse('show');
        $('#ethernet_ip').prop('required', true);
        $('#ethernet_subnet').prop('required', true);
        $('#ethernet_gateway').prop('required', true);
    }
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

function save_ethernet_config(continuation = function () { }) {
    let dhcp = $('#ethernet_show_static').val() != "show";

    let payload: EthernetConfig = {
        enable_ethernet: $('#ethernet_enable').is(':checked'),
        hostname: $('#ethernet_hostname').val().toString(),
        ip: dhcp ? [0, 0, 0, 0] : parse_ip($('#ethernet_ip').val().toString()),
        subnet: dhcp ? [0, 0, 0, 0] : parse_ip($('#ethernet_subnet').val().toString()),
        gateway: dhcp ? [0, 0, 0, 0] : parse_ip($('#ethernet_gateway').val().toString()),
        dns: dhcp ? [0, 0, 0, 0] : parse_ip($('#ethernet_dns').val().toString()),
        dns2: dhcp ? [0, 0, 0, 0] : parse_ip($('#ethernet_dns2').val().toString())
    };

    $.ajax({
        url: '/ethernet/config_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: continuation,
        error: (xhr, status, error) => util.add_alert("ethernet_config_failed", "alert-danger", __("ethernet.script.config_failed"), error + ": " + xhr.responseText)
    });
}

function ethernet_save_reboot() {
    $('#ethernet_reboot').modal('hide');
    util.reboot();
}

export function addEventListeners(source: EventSource) {
    source.addEventListener('ethernet/state', function (e: util.SSE) {
        update_ethernet_state(<EthernetState>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('ethernet/config', function (e: util.SSE) {
        update_ethernet_config(<EthernetConfig>(JSON.parse(e.data)));
    }, false);
}

export function init() {
    (<HTMLButtonElement>document.getElementById("ethernet_reboot_button")).addEventListener("click", ethernet_save_reboot);

    let ip_config = <HTMLInputElement>document.getElementById("ethernet_show_static");
    ip_config.addEventListener("change", () => ethernet_cfg_toggle_static_ip_collapse(ip_config.value));

    // Use bootstrap form validation
    let form = <HTMLFormElement>$('#ethernet_form')[0];
    form.addEventListener('submit', function (event: Event) {
        form.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (form.checkValidity() === false) {
            return;
        }
        save_ethernet_config(() => $('#ethernet_reboot').modal('show'));
    }, false);
}

export function updateLockState(module_init: any) {
    $('#sidebar-ethernet').prop('hidden', !module_init.ethernet);
}

export function getTranslation(lang: string) {
    const translations: {[index: string]:any} = {
        "de": {
            "ethernet": {
                "status": {
                    "ethernet_connection": "LAN-Verbindung",
                    "not_configured": "Unkonfiguriert",
                    "not_connected": "Getrennt",
                    "connecting": "Verbinde",
                    "connected": "Verbunden",
                },
                "navbar": {
                    "ethernet": "LAN-Verbindung",
                },
                "content": {
                    "settings": "LAN-Verbindungs&shy;einstellungen",
                    "enable": "Verbindung aktiviert",
                    "enable_desc": "Wenn aktiviert, verbindet sich das Gerät beim Start oder wenn ein Kabel eingesteckt wird, automatisch.",

                    "hostname": "Hostname",
                    "hostname_invalid": "Der Hostname darf nur aus den Groß- und Kleinbuchstaben A-Z und a-z, sowie den Ziffern 0-9 und Bindestrichen bestehen. Zum Beispiel warp-A1c",
                    "ip_configuration": "IP-Konfiguration",
                    "dhcp": "automatisch (DHCP)",
                    "static": "statisch",
                    "save": "Speichern",

                    "reboot_title": "Neu starten um Konfiguration anzuwenden",
                    "reboot_content": "Die geänderten LAN-Einstellungen werden nur nach einem Neustart angewendet. Jetzt neu starten?",
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
                },
                "script": {
                    "config_failed": "Speichern der Verbindungseinstellungen fehlgeschlagen.",
                }
            }
        },
        "en": {
            "ethernet": {
                "status": {
                    "ethernet_connection": "LAN connection",
                    "not_configured": "Not configured",
                    "not_connected": "Not connected",
                    "connecting": "Connecting",
                    "connected": "Connected",
                },
                "navbar": {
                    "ethernet": "LAN Connection",
                },
                "content": {
                    "settings": "LAN Connection Settings",
                    "enable": "Connection enabled",
                    "enable_desc": "Automatically connects to the configured network on start-up if activated.",
                    "hostname": "Hostname",
                    "hostname_invalid": "The hostname must contain only upper and lower case letters A-Z and a-z the digits 0-9 or dashes. For example warp-A1c",
                    "ip_configuration": "IP configuration",
                    "dhcp": "automatic (DHCP)",
                    "static": "static",
                    "save": "Save",

                    "reboot_title": "Restart to apply configuration",
                    "reboot_content": "The modified LAN configuration will only be applied after a restart. Reboot now?",
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
                },
                "script": {
                    "config_failed": "Failed to save connection configuration.",
                }
            }
        }
    };
    return translations[lang];
}
