/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "../../options";
let x = {
    "port_scanner": {
        "content": {
            "scan_title": "Host-Suche",
            "scan_title_muted": "erste IP-Adresse, letzte IP-Adresse, Port",
            "scan_start": "Suche starten",
            "scan_abort": "Suche abbrechen",
            "scan_start_error": "Fehler beim Starten der Suche",
            "scan_keep_alive_error": "Fehler beim Aufrechterhalten der Suche",
            "scan_abort_error": "Fehler beim Abbrechen der Suche",
            "scan_results": "Gefundene Hosts",
            "scan_log": "Log herunterladen",
            "scan_result_port": /*SFN*/(port: number) => `Port ${port}`/*NF*/,
            "scan_result_port_closed": "Port ist geschlossen",
            "scan_no_result": "Es wurden keine Hosts gefunden.",
            "scan_ip4_invalid": "Die IP-Adresse muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrennt durch einen Punkt, bestehen. Zum Beispiel 10.0.0.2."
        }
    }
}
