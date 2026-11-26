/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "p9_eeg": {
        "status": {
            "status": "Status",
            "not_limited": "Nicht begrenzt (100 %)",
            "limited": /*SFN*/(level: number) => "Einspeisung auf " + level + " % begrenzt"/*NF*/,
            "default_active": /*SFN*/(level: number) => "Standardstufe aktiv (" + level + " %)"/*NF*/,
            "requested_limit": "Angefordertes Limit",
            "limit_not_reachable": "Das angeforderte Limit liegt unter der niedrigsten verfügbaren Stufe."
        },
        "navbar": {
            "p9_eeg": "9 EEG"
        },
        "content": {
            "p9_eeg": "§9 EEG",
            "enable_p9_eeg": "§9 EEG aktiviert",
            "enable_p9_eeg_desc": "Leistungsbegrenzungen des Netzbetreibers werden über die Relais an den Wechselrichter weitergegeben.",
            "enable_p9_eeg_help": <>
                <p>§9 des Erneuerbare-Energien-Gesetzes (EEG) verpflichtet Betreiber von PV-Anlagen, dem Netzbetreiber die Reduzierung der Einspeiseleistung zu ermöglichen.</p>
                <p>Das Limit des Netzbetreibers wird empfangen und in eine der Stufen 0 %, 30 %, 60 % oder 100 % der Nennleistung des Wechselrichters umgesetzt. Die Stufe wird über die Relais an den Eingang des Wechselrichters für Rundsteuerempfänger (oft DRM oder RSE genannt) gegeben. Es wird immer die höchste Stufe gewählt, die das Limit einhält.</p>
            </>,
            "source": "Signalquelle",
            "source_help": <>
                <p><strong>EEBUS</strong>: Das Limit wird über den EEBUS-Anwendungsfall „Limitation of Power Production“ (LPP) von der Steuerbox des Netzbetreibers empfangen. Die Steuerbox kann den Ausfallsicherheitswert und die Ausfallsicherheitsdauer ändern.</p>
                <p><strong>API</strong>: Das Limit wird über die API (p9_eeg/control_update) gesetzt.</p>
            </>,
            "source_eebus": "EEBUS",
            "source_api": "API",
            "eebus_needs_activation": <>Um EEBUS als Signalquelle nutzen zu können, muss <a href="#eebus">EEBUS</a> aktiviert werden.</>,
            "inverter_power": "Nennleistung des Wechselrichters",
            "inverter_power_muted": "AC-Nennleistung",
            "inverter_power_help": <>
                <p>Die Stufen des Wechselrichters beziehen sich auf seine AC-Nennleistung, nicht auf die installierte Modulleistung (kWp). Die Nennleistung steht im Datenblatt des Wechselrichters.</p>
            </>,
            "relay": "Relais",
            "relay_n": /*SFN*/(n: number) => "Relais " + n/*NF*/,
            "relay_muted": "Stufe bei geschlossenem Relais",
            "relay_help": <>
                <p>Jedes Relais wird mit dem Eingang des Wechselrichters für eine Stufe verbunden (zum Beispiel K3 für 30 % oder K4 für 0 %). Bei einer Begrenzung wird höchstens ein Relais geschlossen.</p>
                <p>Ist kein Relais geschlossen, gilt immer 100 %. Der Wechselrichter muss so konfiguriert sein, dass er ohne geschlossenen Kontakt mit voller Leistung einspeist.</p>
                <p>Relais, die von §9 EEG verwendet werden, können nicht mehr über Automatisierungsregeln geschaltet werden.</p>
            </>,
            "relay_unused": "Nicht verwendet",
            "relay_duplicate": "Diese Stufe ist bereits einem anderen Relais zugeordnet.",
            "level_with_power": /*SFN*/(level: number, power_w: number) => level + " % (" + power_w + " W)"/*NF*/,
            "default_level": "Standardstufe",
            "default_level_muted": "Solange kein Limit von der Signalquelle vorliegt",
            "default_level_help": <>
                <p>Bei der API gilt die Standardstufe nach einem Neustart bis zum ersten Aufruf von p9_eeg/control_update.</p>
                <p>Bei EEBUS gilt die Standardstufe, solange EEBUS deaktiviert ist oder noch startet. Zusätzlich ist sie der voreingestellte Ausfallsicherheitswert des Anwendungsfalls LPP. Dieser gilt nach einem Neustart und wenn die Verbindung zur Steuerbox abbricht. Hat die Steuerbox selbst einen Ausfallsicherheitswert geschrieben, hat dieser Vorrang.</p>
            </>,
            "no_relay_used": "Es muss mindestens ein Relais verwendet werden."
        },
        "script": {
            "save_failed": "Speichern der §9-EEG-Einstellungen fehlgeschlagen"
        },
        "automation": {
            "automation_trigger": "§9 EEG (Einspeisebegrenzung)",
            "state": "Zustand",
            "limited": "Begrenzt",
            "not_limited": "Nicht begrenzt",
            "trigger_disabled": "§9 EEG ist deaktiviert",
            "automation_trigger_text": /*FFN*/(limited: boolean) => limited
                ? <>Wenn <b>§9 EEG</b> die Einspeisung <b>begrenzt</b>,{" "}</>
                : <>Wenn <b>§9 EEG</b> die Einspeisung <b>nicht mehr begrenzt</b>,{" "}</>/*NF*/
        }
    }
}
