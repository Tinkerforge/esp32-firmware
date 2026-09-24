/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "../../options";
let x = {
    "sun_spec": {
        "content": {
            "scan_title": "Gerätesuche",
            "scan_title_muted": "erste Geräteadresse, letzte Geräteadresse",
            "scan": "Suche starten",
            "scan_abort": "Suche abbrechen",
            "scan_error": /*FFN*/() => {
                let result = [<>Während der Gerätesuche ist ein Fehler aufgetreten.</>];

                if (options.SUPPORT_EMAIL.length > 0) {
                    result.push(<> Bitte das Log herunterladen und an <a href={`mailto:${options.SUPPORT_EMAIL}?subject=Fehler bei SunSpec-Gerätesuche`}>{options.SUPPORT_EMAIL}</a> schicken.</>);
                }

                return <>{result}</>;
            }/*NF*/,
            "scan_start_error": "Fehler beim Starten der Suche",
            "scan_continue_error": "Fehler beim Fortsetzen der Suche",
            "scan_abort_error": "Fehler beim Abbrechen der Suche",
            "scan_results": "Gefundene Geräte",
            "scan_log": "Log",
            "scan_log_file": "SunSpec-Gerätesuche",

            "scan_result_device_address": "Geräteadresse",
            "scan_result_serial_number": "Seriennummer",
            "scan_result_model_id": "Modell",
            "scan_result_model_no_supported": "Nicht unterstützt",
            "scan_result_model_other_preferred": /*SFN*/(slot: number|"?") => "Modell " + slot + " bevorzugt"/*NF*/,

            "scan_no_result": "Es wurden keine SunSpec-Geräte gefunden.",

            "model_101": "Wechselrichter",
            "model_102": "Wechselrichter",
            "model_103": "Wechselrichter",
            "model_111": "Wechselrichter",
            "model_112": "Wechselrichter",
            "model_113": "Wechselrichter",
            "model_122": "Wechselrichter Messwerte und Status",
            "model_124": "Wechselrichter Speichersteuerung",
            "model_160": "PV-Erzeugung",
            "model_201": "Zähler",
            "model_202": "Zähler",
            "model_203": "Zähler",
            "model_204": "Zähler",
            "model_211": "Zähler",
            "model_212": "Zähler",
            "model_213": "Zähler",
            "model_214": "Zähler",
            "model_220": "Zähler (signiert)",
            "model_701": "AC-Messwerte",
            "model_713": "Speicherkapazität",
            "model_714": "DC-Messwerte",
            "model_802": "Batterie"
        }
    }
}
