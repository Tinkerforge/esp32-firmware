/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "../../options";
let x = {
    "port_scanner": {
        "content": {
            "scan_title": "Host search",
            "scan_title_muted": "first IP address, last IP address, port",
            "scan_start": "Start search",
            "scan_abort": "Abort search",
            "scan_start_error": "Error while starting search",
            "scan_keep_alive_error": "Error while keeping search running",
            "scan_abort_error": "Error while aborting search",
            "scan_results": "Found hosts",
            "scan_log": "Download log",
            "scan_result_port": /*SFN*/(port: number) => `Port ${port}`/*NF*/,
            "scan_result_port_closed": "Port is closed",
            "scan_no_result": "No hosts found.",
            "scan_ip4_invalid": "The IP address must have four groups between 0 and 255 separated with a dot. For example 10.0.0.2."
        }
    }
}
