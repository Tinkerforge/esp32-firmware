import $ from "jquery";

import * as util from "../util";

declare function __(s: string): string;

interface Device {
    uid: string,
    port: string,
    name: string
}

function update_devices(devices: Device[]) {
    if (devices.length == 0) {
        $("#bricklets_content").html("No bricklets found.");
        return;
    }

    let result = "";

    for (let device of devices) {
        let line = "<tr>";
        line += "<td>" + device.port + "</td>";
        line += "<td>" + device.uid + "</td>";
        line += "<td>" + (device.name == "unknown device" ? __("proxy.script.unknown_device") : device.name) + "</td>";
        line += "</tr>"

        result += line;
    }

    $("#bricklets_content").html(result);
}

interface ErrorCounter {
    SpiTfpChecksum: number,
    SpiTfpFrame: number,
    TfpFrame: number,
    TfpUnexpected: number
}

function update_error_counters(error_counters: {[index:string]: ErrorCounter}) {
    if (Object.keys(error_counters).length == 0) {
        $("#bricklets_error_counters").html("");
        return;
    }

    let result = "";

    for (let port in error_counters) {
        let counters = error_counters[port];

        let line = "<tr>";
        line += "<td>" + port + "</td>";
        line += "<td>" + counters.SpiTfpChecksum + "</td>";
        line += "<td>" + counters.SpiTfpFrame + "</td>";
        line += "<td>" + counters.TfpFrame + "</td>";
        line += "<td>" + counters.TfpUnexpected + "</td>";
        line += "</tr>"

        result += line;
    }

    $("#bricklets_error_counters").html(result);
}

export function addEventListeners(source: EventSource) {
    source.addEventListener('proxy/devices', function (e: util.SSE) {
        update_devices(<Device[]>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('proxy/error_counters', function (e: util.SSE) {
        update_error_counters(JSON.parse(e.data));
    }, false);
}

export function init() {

}


export function updateLockState(module_init: any) {
    $('#sidebar-bricklets').prop('hidden', !module_init.proxy);
}

export function getTranslation(lang: string) {
    const translations: {[index: string]:any} = {
        "de": {
            "proxy": {
                "status": {

                },
                "navbar": {
                    "bricklets": "Bricklets"
                },
                "content": {
                    "bricklets": "Bricklets",
                    "port": "Port",
                    "UID": "UID",
                    "device_type": "Gerätetyp",
                    "error_counters": "Fehlerzähler",
                    "spitfp_checksum": "SPI-TFP-Prüfsumme",
                    "spitfp_frame": "SPI-TFP-Frame",
                    "tfp_frame": "TFP-Frame",
                    "tfp_unexpected": "TFP unerwartet",
                },
                "script": {
                    "unknown_device": "unbekanntes Gerät"
                }
            }
        },
        "en": {
            "proxy": {
                "status": {

                },
                "navbar": {
                    "bricklets": "Bricklets"
                },
                "content": {
                    "bricklets": "Bricklets",
                    "port": "Port",
                    "UID": "UID",
                    "device_type": "Device Type",
                    "error_counters": "Error Counters",
                    "spitfp_checksum": "SPI-TFP Checksum",
                    "spitfp_frame": "SPI-TFP Frame",
                    "tfp_frame": "TFP Frame",
                    "tfp_unexpected": "TFP Unexpected",
                },
                "script": {
                    "unknown_device": "unknown device"
                }
            }
        }
    };
    return translations[lang];
}
