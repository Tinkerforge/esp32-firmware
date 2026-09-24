/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "../../options";
let x = {
    "sun_spec": {
        "content": {
            "scan_title": "Device search",
            "scan_title_muted": "first device address, last device address",
            "scan": "Start search",
            "scan_abort": "Abort search",
            "scan_error": /*FFN*/() => {
                let result = [<>An error occurred during the device search.</>];

                if (options.SUPPORT_EMAIL.length > 0) {
                    result.push(<> Please download the log and send it to <a href={`mailto:${options.SUPPORT_EMAIL}?subject=Error during SunSpec device search`}>{options.SUPPORT_EMAIL}</a>.</>);
                }

                return <>{result}</>;
            }/*NF*/,
            "scan_start_error": "Error while starting scan",
            "scan_continue_error": "Error while keeping scan running",
            "scan_abort_error": "Error while aborting scan",
            "scan_results": "Discovered devices",
            "scan_log": "Log",
            "scan_log_file": "SunSpec-scan-log",

            "scan_result_device_address": "Device address",
            "scan_result_serial_number": "Serial number",
            "scan_result_model_id": "Model",
            "scan_result_model_no_supported": "Not supported",
            "scan_result_model_other_preferred": /*SFN*/(slot: number|"?") => "Model " + slot + " preferred"/*NF*/,

            "scan_no_result": "No SunSpec devices found.",

            "model_101": "Inverter",
            "model_102": "Inverter",
            "model_103": "Inverter",
            "model_111": "Inverter",
            "model_112": "Inverter",
            "model_113": "Inverter",
            "model_122": "Inverter measurements and status",
            "model_124": "Inverter storage controls",
            "model_160": "PV generation",
            "model_201": "Meter",
            "model_202": "Meter",
            "model_203": "Meter",
            "model_204": "Meter",
            "model_211": "Meter",
            "model_212": "Meter",
            "model_213": "Meter",
            "model_214": "Meter",
            "model_220": "Meter (secure)",
            "model_701": "AC measurements",
            "model_713": "Storage capacity",
            "model_714": "DC measurements",
            "model_802": "Battery"
        }
    }
}
