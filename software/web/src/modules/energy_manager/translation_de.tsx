/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "energy_manager": {
        "status": {
            "status": "Energiemanager",
            "error_ok": "OK",
            "error_network": "Netzwerkfehler",
            "error_contactor": "Schützfehler",
            "error_config": "Unvollständige Konfiguration",
            "error_internal": "Interner Fehler",

            "no_bricklet": "Interner Fehler, Bricklet nicht gefunden"
        },
        "automation": {
            "input": /*SFN*/(input: number) => "Eingang " + input + " geschaltet"/*NF*/,
            "state": "",
            "closed": "Auf geschlossen",
            "open": "Auf offen",
            "automation_input_text": /*FFN*/(input: number, state: boolean) => {
                return <>Wenn <b>Eingang {input}</b> {state ? <b>geschlossen</b> : <b>geöffnet</b>} wird, </>
            }/*NF*/,

            "switch_relay": "Schalte Relais",
            "relay_index": "Relaisnummer",
            "relay_state": "Schalten auf",
            "relay_state_open": "Offen",
            "relay_state_closed": "Geschlossen",
            "relay_action_text": /*FFN*/(index: number, state: boolean) => {
                const index_text = index >= 0 ? " " + (index+1).toString() : "";
                let ret = state ? <><b>schließen</b></> : <><b>öffnen</b></>
                return <>Relais{index_text} {ret}.</>
            }/*NF*/
        }
    }
}
